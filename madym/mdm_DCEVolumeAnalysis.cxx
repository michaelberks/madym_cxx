/**
*  @file    mdm_DCEVolumeAnalysis.cxx
*  @brief   Implementation of mdm_DCEVolumeAnalysis class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS


#include "mdm_DCEVolumeAnalysis.h"

#include <cmath>
#include <cassert>
#include <chrono>  // chrono::system_clock
#include <sstream> // stringstream
#include <algorithm>
#include <boost/format.hpp>

#include <mdm_version.h>
#include <mdm_exception.h>
#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_AIF.h>

//Names of output maps
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_IAUC = "IAUC"; //Appended with IAUC time
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_RESDIUALS = "residuals";
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_ENHANCING = "enhVox";
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_ROI = "ROI";
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_T1 = "T1";
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_M0 = "M0";
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_CT_SIG = "Ct_sig"; //Signal derived concentration - appended with volume number
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_CT_MOD = "Ct_mod"; //Model estimated concentration - appended with volume number
const std::string mdm_DCEVolumeAnalysis::MAP_NAME_ERROR_CODE = "error_codes";

MDM_API mdm_DCEVolumeAnalysis::mdm_DCEVolumeAnalysis()
	:
	T1_mapper_(errorTracker_),
	testEnhancement_(false),
	useM0Ratio_(true),
  outputCt_sig_(false),
  outputCt_mod_(false),
  useNoise_(false),
  dynamicTimes_(0),
  noiseVar_(0),
  firstImage_(0),
  lastImage_(0),
	maxIterations_(0),
  model_(NULL)
{
	setIAUCtimes({ 60.0, 90.0, 120.0 }, true);
}

MDM_API mdm_DCEVolumeAnalysis::~mdm_DCEVolumeAnalysis()
{

}

//
MDM_API mdm_ErrorTracker& mdm_DCEVolumeAnalysis::errorTracker()
{
	return errorTracker_;
}

//
MDM_API mdm_T1VolumeAnalysis& mdm_DCEVolumeAnalysis::T1Mapper()
{
	return T1_mapper_;
}

//
MDM_API const mdm_T1VolumeAnalysis& mdm_DCEVolumeAnalysis::T1Mapper() const
{
  return T1_mapper_;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setROI(const mdm_Image3D ROI)
{
	ROI_ = ROI;
}

//
MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::ROI() const
{
	return ROI_;
}

MDM_API void mdm_DCEVolumeAnalysis::setAIFmap(
  const mdm_Image3D map)
{
  if (map.type() != mdm_Image3D::ImageType::TYPE_AIFVOXELMAP)
  {
    AIFmap_.copy(map);
    AIFmap_.setType(mdm_Image3D::ImageType::TYPE_AIFVOXELMAP);
    for (size_t idx = 0; idx < map.numVoxels(); idx++)
      if (map.voxel(idx) > 0)
        AIFmap_.setVoxel(idx, mdm_AIF::AIFmapVoxel::SELECTED);
  }
  else
    AIFmap_ = map;
}

//
MDM_API std::vector<double> mdm_DCEVolumeAnalysis::AIFfromMap()
{
  if (!referenceDynamicImg_.numVoxels())
    throw mdm_exception(__func__, "Dynamic maps not loaded.");

  std::vector<size_t> badVoxels;
  std::vector<double>baseAIF;
  computeMeanCt(AIFmap_, mdm_AIF::AIFmapVoxel::SELECTED, baseAIF, badVoxels);
  for (const auto vox : badVoxels)
    AIFmap_.setVoxel(vox, mdm_AIF::AIFmapVoxel::INVALID_CT);

  return baseAIF;
};

//
MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::AIFmap() const
{
  return AIFmap_;
}

MDM_API void mdm_DCEVolumeAnalysis::addStDataMap(const mdm_Image3D dynImg)
{
	//Add the image to the list
	StDataMaps_.push_back(dynImg);

  //First map we add, set the reference image
  if (!referenceDynamicImg_.numVoxels())
    referenceDynamicImg_.copy(dynImg);

	//Extract the time from the header, converted to minutes
	dynamicTimes_.push_back(dynImg.minutesFromTimeStamp());

  if (useNoise_)
  {
    double noise = dynImg.info().noiseSigma.value();
    if (noise != NAN)
      noiseVar_.push_back(noise);
  }
  
	if (outputCt_sig_ && (CtDataMaps_.size() == numSt()-1))
	{
		mdm_Image3D ctMap;
		ctMap.copy(dynImg);
		ctMap.setTimeStampFromDoubleStr(dynImg.timeStamp());
		ctMap.setType(mdm_Image3D::ImageType::TYPE_CAMAP);
		CtDataMaps_.push_back(ctMap);
	}
  if (outputCt_mod_ && (CtModelMaps_.size() == numSt() - 1))
  {
    mdm_Image3D cModMap;
    cModMap.copy(dynImg);
		cModMap.setTimeStampFromDoubleStr(dynImg.timeStamp());
    cModMap.setType(mdm_Image3D::ImageType::TYPE_CAMAP);
    CtModelMaps_.push_back(cModMap);
  }
  
}

//
MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::StDataMap(size_t i) const
{
  try { return StDataMaps_[i]; }
  catch (std::out_of_range &e)
  {
    mdm_exception em(__func__, e.what());
    em.append(boost::format(
      "Attempting to access St map %1% when there are %2% S(t) maps")
      % i % StDataMaps_.size());
    throw em;
  }
}

//
MDM_API const std::vector<mdm_Image3D> & mdm_DCEVolumeAnalysis::StDataMaps() const
{
  return StDataMaps_;
}

//
MDM_API size_t  mdm_DCEVolumeAnalysis::numDynamics() const
{
	if (StDataMaps_.empty())
		return numCtSignal();

  return numSt();
}

//
MDM_API void mdm_DCEVolumeAnalysis::computeMeanCt(
  const mdm_Image3D &map, double map_val,
  std::vector<double> &meanCt, std::vector<size_t> &badVoxels) const
{
  auto nTimes = numDynamics();
  
  if (!nTimes)
    throw mdm_exception(__func__, "Trying to compute mean C(t) when no dynamic maps set");

  if (!referenceDynamicImg_.dimensionsMatch(map))
    throw mdm_exception(__func__, "Dimensions of map do not match dimensions of dynamic maps");

  meanCt.resize(nTimes, 0);
  badVoxels.clear();
  double numVox = 0;
  for (size_t idx = 0; idx < map.numVoxels(); idx++)
  {
    if (map.voxel(idx) == map_val)
    {
      std::vector<double> Ct;
      if (computeCt_)
      {
        mdm_DCEVoxel vox(setUpVoxel(idx));
        if (vox.status() != mdm_DCEVoxel::OK)
        {
          badVoxels.push_back(idx);
          continue;
        }
          

        Ct = vox.CtData();
      }
      else
        voxelCtData(idx, Ct);

      for (size_t t = 0; t < nTimes; t++)
        meanCt[t] += Ct[t];
      
      numVox++;
    }
  }

  if (numVox)
    for (auto &v : meanCt)
      v /= numVox;

  return;
}

//
MDM_API void mdm_DCEVolumeAnalysis::addCtDataMap(const mdm_Image3D ctMap)
{

	//Add the image to the list
	CtDataMaps_.push_back(ctMap);

  //First map we add, set the reference image
  if (!referenceDynamicImg_.numVoxels())
    referenceDynamicImg_.copy(ctMap);

	//Extract the time from the header, converted to minutes
	dynamicTimes_.push_back(ctMap.minutesFromTimeStamp());

  //Check if there is a noise variance associated with the volume
  if (useNoise_)
  {
    double noise = ctMap.info().noiseSigma.value();
    if (!isnan(noise))
      noiseVar_.push_back(noise);
  }
}

//
MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::CtDataMap(size_t i) const
{
  try { return CtDataMaps_[i]; }
  catch (std::out_of_range &e)
  {
    mdm_exception em(__func__, e.what());
    em.append(boost::format(
      "Attempting to access C(t) map %1% when there are %2% C(t) maps")
      % i % CtDataMaps_.size());
    throw em;
  }
}

//
MDM_API const std::vector<mdm_Image3D> & mdm_DCEVolumeAnalysis::CtDataMaps() const
{
  return CtDataMaps_;
}

//
MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::CtModelMap(size_t i) const
{
  try { return CtModelMaps_[i]; }
  catch (std::out_of_range &e)
  {
    mdm_exception em(__func__, e.what());
    em.append(boost::format(
      "Attempting to access C_m(t) map %1% when there are %2% C_m(t) maps")
      % i % CtModelMaps_.size());
    throw em;
  }
}

//
MDM_API const std::vector<mdm_Image3D> & mdm_DCEVolumeAnalysis::CtModelMaps() const
{
  return CtModelMaps_;
}

//
MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::DCEMap(const std::string &mapName) const
{
  if (model_)
  {
    for (int i = 0; i < model_->numParams(); i++)
    {
      if (mapName == model_->paramName(i))
        return pkParamMaps_[i];
    }
  }

	for (size_t i = 0; i < IAUCTimes_.size(); i++)
	{
		if (mapName == (MAP_NAME_IAUC + std::to_string(int(IAUCTimes_[i]))))
			return IAUCMaps_[i];
	}	

	if (mapName == MAP_NAME_RESDIUALS)
		return modelResidualsMap_;

	if (mapName == MAP_NAME_ENHANCING)
		return enhVoxMap_;
	
	//Error map name not recognised
	throw mdm_exception(__func__, boost::format("Map name %1% not recognised") % mapName);

}

//
MDM_API void mdm_DCEVolumeAnalysis::setDCEMap(const std::string &mapName, const mdm_Image3D &map)
{
  if (pkParamMaps_.size() != model_->numParams())
    pkParamMaps_.resize(model_->numParams());

  for (int i = 0; i < model_->numParams(); i++)
  {
    if (mapName == model_->paramName(i))
    {
      pkParamMaps_[i] = map;
      return;
    }  
  }

  for (size_t i = 0; i < IAUCTimes_.size(); i++)
  {
    if (mapName == (MAP_NAME_IAUC + std::to_string(int(IAUCTimes_[i]))))
    {
      IAUCMaps_[i] = map;
      return;
    }    
  }

  if (mapName == MAP_NAME_RESDIUALS)
  {
    modelResidualsMap_ = map;
    return;
  }
  if (mapName == MAP_NAME_ENHANCING)
  {
    enhVoxMap_ = map;
    return;
  }

  //Error map name not recognised
  throw mdm_exception(__func__, boost::format("Map name %1% not recognised") % mapName);
}

//
MDM_API std::string mdm_DCEVolumeAnalysis::modelType() const
{
  if (model_)
    return model_->modelType();
  else
    return "";
}

//
MDM_API std::vector<double> mdm_DCEVolumeAnalysis::dynamicTimes() const
{
	return dynamicTimes_;
}

//
MDM_API double mdm_DCEVolumeAnalysis::dynamicTime(size_t i) const
{
  try { return dynamicTimes_[i]; }
  catch (std::out_of_range &e)
  {
    mdm_exception em(__func__, e.what());
    em.append(boost::format(
      "Attempting to access timepoint %1% when there are only %2% timepoints")
      % i % dynamicTimes_.size());
    throw em;
  }
}

//
MDM_API std::vector<std::string> mdm_DCEVolumeAnalysis::paramNames() const
{
	return model_->paramNames();
}

//
MDM_API std::vector<double> mdm_DCEVolumeAnalysis::IAUCtimes() const
{
	return IAUCTimes_;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setR1Const(double rc)
{
	r1Const_ = rc;
}

MDM_API void mdm_DCEVolumeAnalysis::setPrebolusImage(int prebolus)
{
  prebolusImage_ = prebolus;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setModel(std::shared_ptr<mdm_DCEModelBase> model)
{
	model_ = model;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setTestEnhancement(bool flag)
{
	testEnhancement_ = flag;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setM0Ratio(bool flag)
{
	useM0Ratio_ = flag;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setComputeCt(bool flag)
{
	computeCt_ = flag;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setOutputCt(bool flag)
{
	outputCt_sig_ = flag;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setOutputCmod(bool flag)
{
  outputCt_mod_ = flag;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setIAUCtimes(const std::vector<double> &times, bool convertToMins)
{
	IAUCTimes_ = times;
	std::sort(IAUCTimes_.begin(), IAUCTimes_.end());

	IAUCTMinutes_ = times;
	if (convertToMins)
	{
		for (auto &t : IAUCTMinutes_)
			t /= 60;
	}
		
}

//
MDM_API void mdm_DCEVolumeAnalysis::setUseNoise(bool b)
{
  useNoise_ = b;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setFirstImage(size_t t)
{
  firstImage_ = t;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setLastImage(size_t t)
{
  lastImage_ = t;
}

//
MDM_API void mdm_DCEVolumeAnalysis::setMaxIterations(int maxItr)
{
	maxIterations_ = maxItr;
}

//
MDM_API void mdm_DCEVolumeAnalysis::initialiseParameterMaps()
{
  //Model parameter maps may already have been loaded
  if (pkParamMaps_.size() != model_->numParams())
  {
    pkParamMaps_.resize(model_->numParams());
    for (auto &map : pkParamMaps_)
      createMap(map);
  }

  IAUCMaps_.resize(IAUCTimes_.size());
  for (auto &map : IAUCMaps_)
    createMap(map);

  createMap(modelResidualsMap_);
  createMap(enhVoxMap_);

  //
  if (outputCt_mod_)
  {
    CtModelMaps_.resize(numDynamics());
    for (auto &map : CtModelMaps_)
      createMap(map);
  }
}

//
MDM_API void  mdm_DCEVolumeAnalysis::fitDCEModel(bool paramMapsInitialised, bool optimiseModel, const std::vector<int> initMapParams)
{
  // Check we have the input files we need, either concentration maps
  //or dynamic images
  if (computeCt_)
  {
    if (!numDynamics() || !StDataMaps_[0].numVoxels())
      throw mdm_exception(__func__, "No input dynamic images - nothing to fit");
      
  }
  else if (!numCtSignal() || !CtDataMaps_[0].numVoxels())
    throw mdm_exception(__func__, "No input concentration maps - nothing to fit");
    

  //
  initialiseParameterMaps();
    

  //Fit the model
  fitModel(paramMapsInitialised, optimiseModel, initMapParams);

}

//------------------------------------------------------------------
// Private
//------------------------------------------------------------------

//
size_t mdm_DCEVolumeAnalysis::numSt() const
{
  return StDataMaps_.size();
}

//
size_t mdm_DCEVolumeAnalysis::numCtSignal() const
{
  return CtDataMaps_.size();
}

//
size_t mdm_DCEVolumeAnalysis::numCtModel() const
{
  return CtModelMaps_.size();
}

//
mdm_DCEVoxel mdm_DCEVolumeAnalysis::setUpVoxel(size_t voxelIndex) const
{
  

  std::vector<double> St, Ct;
  if (computeCt_)
    voxelStData(voxelIndex, St);

  else
    voxelCtData(voxelIndex, Ct);

  mdm_DCEVoxel vox(
    St,//dynSignals
    Ct,//dynConc
    prebolusImage_,//bolus_time
    dynamicTimes_,//dynamicTimings
    IAUCTMinutes_);//IAUC_times

  if (computeCt_)
  {
    auto TR = referenceDynamicImg_.info().TR.value();
    auto FA = referenceDynamicImg_.info().flipAngle.value();

    auto T1 = T1_mapper_.T1(voxelIndex);
    auto M0 = useM0Ratio_ ? 0.0 : T1_mapper_.M0(voxelIndex);

    //Convert signal (if already C(t) does nothing so can call regardless)
    vox.computeCtFromSignal(T1, FA, TR, r1Const_, M0, firstImage_);
  }
    
  return vox;
}

//
void  mdm_DCEVolumeAnalysis::voxelStData(size_t voxelIndex, std::vector<double> &data) const
{
	size_t n = numSt();

	data.resize(n);
  for (size_t k = 0; k < n; k++)
    data[k] = StDataMaps_[k].voxel(voxelIndex);
}

//
void  mdm_DCEVolumeAnalysis::voxelCtData(size_t voxelIndex, std::vector<double> &data) const
{
	size_t n = CtDataMaps_.size();

	data.resize(n);
	for (size_t k = 0; k < n; k++)
		data[k] = CtDataMaps_[k].voxel(voxelIndex);
}

//
void  mdm_DCEVolumeAnalysis::voxelCtModel(size_t voxelIndex, std::vector<double> &data) const
{
  size_t n = CtModelMaps_.size();

  data.resize(n);
  for (size_t k = 0; k < n; k++)
    data[k] = CtModelMaps_[k].voxel(voxelIndex);
}

//
void mdm_DCEVolumeAnalysis::setVoxelErrors(size_t voxelIndex, const mdm_DCEVoxel &vox)
{
	//
  size_t voxelOk = vox.status();
  if (voxelOk == mdm_DCEVoxel::CA_NAN)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::CA_IS_NAN);
  
  else if (voxelOk == mdm_DCEVoxel::DYN_T1_BAD)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::DYNT1_NEGATIVE);
  
  else if (voxelOk == mdm_DCEVoxel::M0_BAD)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::M0_NEGATIVE);
}

//
void mdm_DCEVolumeAnalysis::setVoxelInAllMaps(size_t voxelIndex, 
  const mdm_DCEVoxel  &vox, const mdm_DCEModelFitter &fitter)
{
	for (size_t i = 0; i < pkParamMaps_.size(); i++)
		pkParamMaps_[i].setVoxel(voxelIndex, model_->params(int(i)));

	for (size_t i = 0; i < IAUCMaps_.size(); i++)
		IAUCMaps_[i].setVoxel(voxelIndex, vox.IAUC_val(i));
  
  setVoxelModelError(voxelIndex, fitter);
  enhVoxMap_.setVoxel(voxelIndex,  vox.enhancing());

  //
  if (outputCt_sig_)
    for (size_t i = 0; i < numDynamics(); i++)
      CtDataMaps_[i].setVoxel(voxelIndex, vox.CtData()[i]);
    
  if (outputCt_mod_)
    for (size_t i = 0; i < numDynamics(); i++)
      CtModelMaps_[i].setVoxel(voxelIndex, fitter.CtModel()[i]);
    
}

//
void mdm_DCEVolumeAnalysis::setVoxelModelError(size_t voxelIndex, const mdm_DCEModelFitter  &fitter)
{
  modelResidualsMap_.setVoxel(voxelIndex, fitter.modelFitError());
}

//
void mdm_DCEVolumeAnalysis::setVoxelInAllMaps(size_t voxelIndex, double value)
{
  for (auto &map : pkParamMaps_)
		map.setVoxel(voxelIndex, value);

	for (auto &map : IAUCMaps_)
		map.setVoxel(voxelIndex, value);

  modelResidualsMap_.setVoxel(voxelIndex, value);
  enhVoxMap_.setVoxel(voxelIndex, value);

  /** 1.22 */
  if (outputCt_sig_)
    for (auto &map : CtDataMaps_)
      map.setVoxel(voxelIndex, value);
    

  if (outputCt_mod_)
    for (auto &map : CtModelMaps_)
      map.setVoxel(voxelIndex, value);
    
}

//
void  mdm_DCEVolumeAnalysis::fitModel(bool paramMapsInitialised, 
  bool optimiseModel, const std::vector<int> initMapParams)
{
  if (!referenceDynamicImg_.numVoxels())
    throw mdm_exception(__func__, "Dynamic maps not loaded.");

  //Create a new fitter object
  mdm_DCEModelFitter modelFitter(
    *model_,
    firstImage_,
    lastImage_ ? lastImage_ : numDynamics(),
    noiseVar_,
    maxIterations_
  );

  /* Loop through images having fun ... */
	bool useROI = ROI_.numVoxels() > 0;
	int numProcessed = 0;
	int numErrors = 0;
	auto fit_start = std::chrono::system_clock::now();
  for(size_t voxelIndex = 0; voxelIndex < referenceDynamicImg_.numVoxels(); voxelIndex++)
  {
        
    if ((!computeCt_ || T1_mapper_.T1(voxelIndex) > 0.0)
          && (!useROI || ROI_.voxel(voxelIndex) > 0.0))
    {
      //Check if we've got parameter maps with values to initialise each voxel
      //if not the existing values set in the model will be used
      if (paramMapsInitialised)
      {
        int n = model_->numParams();
			  std::vector<double> initialParams = model_->initialParams();

			  if (initMapParams.empty())
				  for (int i = 0; i < n; i++)
					  initialParams[i] = pkParamMaps_[i].voxel(voxelIndex);
			  else
				  for (int i : initMapParams)
					  initialParams[i-1] = pkParamMaps_[i-1].voxel(voxelIndex); //Need -1 because user indexing starts at 1

        model_->setInitialParams(initialParams);
      }
          
      //Set up the DCE voxel object
      mdm_DCEVoxel vox(setUpVoxel(voxelIndex));

      //Compute IAUC
      vox.computeIAUC();

      //Run an initial fit (does not optimise parameters, but
      //sets bounds on model parameters, and compute the model residual
      //for the initial model parameters
      modelFitter.initialiseModelFit(vox.CtData());

      //Set any error codes returned from setting up the voxel in the error codes map
      setVoxelErrors(voxelIndex, vox);

      //Test enhancement
      if (testEnhancement_)
      {
        vox.testEnhancing();
        if (!vox.enhancing())
          errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::NON_ENH_IAUC);
      }

      //The main event: If optimising the model fit, do so now
      if (optimiseModel)
      {
        modelFitter.fitModel(vox.status(), vox.enhancing());
            
        //Check if any model fitting error codes generated
        mdm_ErrorTracker::ErrorCode errorCode = model_->getModelErrorCode();
        if (errorCode != mdm_ErrorTracker::OK)
        {
          errorTracker_.updateVoxel(voxelIndex, errorCode);
          numErrors++;
        }
      }

      //Set all the necessary values in the output maps
      setVoxelInAllMaps(voxelIndex, vox, modelFitter);

		  numProcessed++;
    }
  }

	// Get end time and log results
	auto fit_end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = fit_end - fit_start;
	
	std::stringstream ss;
	ss << "mdm_DCEVolumeAnalysis: Processed " << 
		numProcessed << " voxels in " << elapsed_seconds.count() << "s.\n" << 
		numErrors << " voxels returned fit errors\n";
	mdm_ProgramLogger::logProgramMessage(ss.str());
}

//
void mdm_DCEVolumeAnalysis::createMap(mdm_Image3D& img)
{
  if (!referenceDynamicImg_.numVoxels())
    throw mdm_exception(__func__, boost::format("Error allocating parameter maps, "
			"at least one of dynamic signal (StDataMaps_) or concentration series "
			"(CtDataMaps_) should be non-empty"));
		
  img.copy(referenceDynamicImg_);
	img.setType(mdm_Image3D::ImageType::TYPE_KINETICMAP);
}
