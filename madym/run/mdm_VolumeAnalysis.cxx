/**
*  @file    mdm_VolumeAnalysis.cxx
*  @brief   Implementation of mdm_VolumeAnalysis class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS


#include "mdm_VolumeAnalysis.h"

#include <cmath>
#include <chrono>  // chrono::system_clock
#include <sstream> // stringstream
#include <algorithm>
#include <numeric>
#include <boost/format.hpp>

#include <madym/utils/mdm_exception.h>
#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/dce/mdm_AIF.h>

//Names of output maps
const std::string mdm_VolumeAnalysis::MAP_NAME_IAUC = "IAUC"; //Appended with IAUC time
const std::string mdm_VolumeAnalysis::MAP_NAME_RESIDUALS = "residuals";
const std::string mdm_VolumeAnalysis::MAP_NAME_ENHANCING = "enhVox";
const std::string mdm_VolumeAnalysis::MAP_NAME_ROI = "ROI";
const std::string mdm_VolumeAnalysis::MAP_NAME_ERROR_TRACKER = "error_tracker";
const std::string mdm_VolumeAnalysis::MAP_NAME_T1 = "T1";
const std::string mdm_VolumeAnalysis::MAP_NAME_M0 = "M0";
const std::string mdm_VolumeAnalysis::MAP_NAME_EFFICIENCY = "efficiency";

MDM_API mdm_VolumeAnalysis::mdm_VolumeAnalysis()
	:
	T1Mapper_(errorTracker_, ROI_),
  DWIMapper_(errorTracker_, ROI_),
	testEnhancement_(false),
	useM0Ratio_(true),
  useB1correction_(false),
  outputCt_sig_(false),
  outputCt_mod_(false),
  useNoise_(false),
  StDataMaps_(0),
  CtDataMaps_(0),
  CtModelMaps_(0),
  dynamicTimes_(0),
  noiseVar_(0),
  firstImage_(0),
  lastImage_(0),
	maxIterations_(0),
  model_(NULL)
{
	setIAUCtimes({ 60.0, 90.0, 120.0 }, true, false);
}

MDM_API mdm_VolumeAnalysis::~mdm_VolumeAnalysis()
{

}

MDM_API void mdm_VolumeAnalysis::reset()
{
  ROI_.reset();
  AIFmap_.reset();
  StDataMaps_.clear();
  CtDataMaps_.clear();
  CtModelMaps_.clear();
  dynamicTimes_.clear();
  noiseVar_.clear();
  dynamicMetaData_.reset();

  T1Mapper_.reset();
  errorTracker_.resetErrorImage();

  /* Images for inputs and output */
  pkParamMaps_.clear();
  IAUCMaps_.clear();
  modelResidualsMap_.reset();
  enhVoxMap_.reset();
  initMapParams_.clear();
}

//
MDM_API mdm_ErrorTracker& mdm_VolumeAnalysis::errorTracker()
{
	return errorTracker_;
}

//
MDM_API mdm_T1Mapper& mdm_VolumeAnalysis::T1Mapper()
{
	return T1Mapper_;
}

//
MDM_API const mdm_T1Mapper& mdm_VolumeAnalysis::T1Mapper() const
{
  return T1Mapper_;
}

//
MDM_API mdm_DWIMapper& mdm_VolumeAnalysis::DWIMapper()
{
  return DWIMapper_;
}

//
MDM_API const mdm_DWIMapper& mdm_VolumeAnalysis::DWIMapper() const
{
  return DWIMapper_;
}

//
MDM_API void mdm_VolumeAnalysis::setROI(const mdm_Image3D ROI)
{
  errorTracker_.checkOrSetDimension(ROI, "ROI");
  ROI_ = ROI;
	}

//
MDM_API mdm_Image3D mdm_VolumeAnalysis::ROI() const
{
	return ROI_;
}

MDM_API void mdm_VolumeAnalysis::setAIFmap(
  const mdm_Image3D map)
{
  errorTracker_.checkOrSetDimension(map, "AIF map");

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
MDM_API std::vector<double> mdm_VolumeAnalysis::AIFfromMap()
{
  if (!AIFmap_)
    throw mdm_exception(__func__, "AIF map not set.");

  checkDynamicsSet();

  std::vector<size_t> badVoxels;
  std::vector<double>baseAIF;
  computeMeanCt(AIFmap_, mdm_AIF::AIFmapVoxel::SELECTED, baseAIF, badVoxels);
  for (const auto vox : badVoxels)
    AIFmap_.setVoxel(vox, mdm_AIF::AIFmapVoxel::INVALID_CT);

  return baseAIF;
};

//
MDM_API mdm_Image3D mdm_VolumeAnalysis::AIFmap() const
{
  return AIFmap_;
}

MDM_API void mdm_VolumeAnalysis::addStDataMap(const mdm_Image3D dynImg)
{
  //Check the image dimension match
  errorTracker_.checkOrSetDimension(dynImg, 
    "dynamic image " + std::to_string(StDataMaps_.size()+1));

	//Add the image to the list
	StDataMaps_.push_back(dynImg);

  //First map we add, set the reference image
  if (!dynamicMetaData_)
    setDynamicMetaData(dynImg);

	//Extract the time from the header, converted to minutes
  setDynamicTime(dynImg);

  if (useNoise_)
  {
    double noise = dynImg.info().noiseSigma.value();
    if (noise != NAN)
      noiseVar_.push_back(noise);
  }
  
	if (outputCt_sig_ && (CtDataMaps_.size() == numSt() - 1))
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
MDM_API mdm_Image3D mdm_VolumeAnalysis::StDataMap(size_t i) const
{
  if (i >= StDataMaps_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access S(t) map at index %1% when there are only %2% S(t) maps")
      % i % StDataMaps_.size());

  return StDataMaps_[i];
}

//
MDM_API const std::vector<mdm_Image3D> & mdm_VolumeAnalysis::StDataMaps() const
{
  return StDataMaps_;
}

//
MDM_API size_t  mdm_VolumeAnalysis::numDynamics() const
{
	if (StDataMaps_.empty())
		return numCtSignal();

  return numSt();
}

//
MDM_API void mdm_VolumeAnalysis::computeMeanCt(
  const mdm_Image3D &map, double map_val,
  std::vector<double> &meanCt, std::vector<size_t> &badVoxels) const
{
  errorTracker_.checkDimension(map, "Ct ROI");

  auto nTimes = numDynamics();
  
  if (!nTimes)
    throw mdm_exception(__func__, "Trying to compute mean C(t) when no dynamic maps set");


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
MDM_API void mdm_VolumeAnalysis::addCtDataMap(const mdm_Image3D ctMap)
{
  //Check the image dimension match
  errorTracker_.checkOrSetDimension(ctMap,
    "concentration image " + std::to_string(CtDataMaps_.size() + 1));

  //We don't allow mixed setting of Ct and St maps - so if St already set, throw error
  if (!StDataMaps_.empty())
    throw mdm_exception(__func__, "Attempting to add C(t) when S(t) maps already set");

	//Add the image to the list
	CtDataMaps_.push_back(ctMap);

  //First map we add, set the reference image
  if (!dynamicMetaData_)
    setDynamicMetaData(ctMap);


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
MDM_API mdm_Image3D mdm_VolumeAnalysis::CtDataMap(size_t i) const
{
  if (i >= CtDataMaps_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access C(t) map at index %1% when there are only %2% C(t) maps")
      % i % CtDataMaps_.size());

  return CtDataMaps_[i];
}

//
MDM_API const std::vector<mdm_Image3D> & mdm_VolumeAnalysis::CtDataMaps() const
{
  return CtDataMaps_;
}

//
MDM_API mdm_Image3D mdm_VolumeAnalysis::CtModelMap(size_t i) const
{
  if (i >= CtModelMaps_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access Cm(t) map at index %1% when there are only %2% Cm(t) maps")
      % i % CtDataMaps_.size());

  return CtModelMaps_[i];
}

//
MDM_API const std::vector<mdm_Image3D> & mdm_VolumeAnalysis::CtModelMaps() const
{
  return CtModelMaps_;
}

//
MDM_API mdm_Image3D mdm_VolumeAnalysis::DCEMap(const std::string &mapName) const
{
  checkModelSet();

  for (int i = 0; i < model_->numParams(); i++)
  {
    if (mapName == model_->paramName(i))
      return pkParamMaps_[i];
  }

	for (size_t i = 0; i < IAUCTimes_.size(); i++)
	{
		if (mapName == (MAP_NAME_IAUC + std::to_string(int(IAUCTimes_[i]))))
			return IAUCMaps_[i];
	}	

  if (mapName == (MAP_NAME_IAUC + "_peak"))
    return IAUCMaps_.back();

	if (mapName == MAP_NAME_RESIDUALS)
		return modelResidualsMap_;

	if (mapName == MAP_NAME_ENHANCING)
		return enhVoxMap_;
	
	//Error map name not recognised
	throw mdm_exception(__func__, boost::format("Map name %1% not recognised") % mapName);

}

//
MDM_API void mdm_VolumeAnalysis::setDCEMap(const std::string &mapName, const mdm_Image3D &map)
{
  errorTracker_.checkOrSetDimension(map, "param map " + mapName);

  checkModelSet();

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

  if (mapName == (MAP_NAME_IAUC + "_peak"))
  {
    IAUCMaps_[IAUCMaps_.size()-1] = map;
    return;
  }

  if (mapName == MAP_NAME_RESIDUALS)
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
MDM_API std::string mdm_VolumeAnalysis::modelType() const
{
  if (model_)
    return model_->modelType();
  else
    return "";
}

//
MDM_API std::vector<double> mdm_VolumeAnalysis::dynamicTimes() const
{
	return dynamicTimes_;
}

//
MDM_API double mdm_VolumeAnalysis::dynamicTime(size_t i) const
{
  if (i >= dynamicTimes_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access timepoint %1% when there are only %2% timepoints")
      % i % dynamicTimes_.size());

  return dynamicTimes_[i];
}

//
MDM_API std::vector<std::string> mdm_VolumeAnalysis::paramNames() const
{
  checkModelSet();
	return model_->paramNames();
}

//
MDM_API std::vector<double> mdm_VolumeAnalysis::IAUCtimes() const
{
	return IAUCTimes_;
}

//
MDM_API bool mdm_VolumeAnalysis::IAUCAtpeak() const
{
  return IAUCAtPeak_;
}

//
MDM_API void mdm_VolumeAnalysis::setR1Const(double rc)
{
	r1Const_ = rc;
}

MDM_API void mdm_VolumeAnalysis::setPrebolusImage(int prebolus)
{
  prebolusImage_ = prebolus;
}

//
MDM_API void mdm_VolumeAnalysis::setModel(std::shared_ptr<mdm_DCEModelBase> model)
{
	model_ = model;
}

//
MDM_API void mdm_VolumeAnalysis::setTestEnhancement(bool flag)
{
	testEnhancement_ = flag;
}

//
MDM_API void mdm_VolumeAnalysis::setM0Ratio(bool flag)
{
	useM0Ratio_ = flag;
}

//
MDM_API void mdm_VolumeAnalysis::setB1correction(bool flag)
{
  useB1correction_ = flag;
}

//
MDM_API void mdm_VolumeAnalysis::setComputeCt(bool flag)
{
	computeCt_ = flag;
}

//
MDM_API void mdm_VolumeAnalysis::setOutputCtSig(bool flag)
{
	outputCt_sig_ = flag;
}

//
MDM_API void mdm_VolumeAnalysis::setOutputCtMod(bool flag)
{
  outputCt_mod_ = flag;
}

//
MDM_API void mdm_VolumeAnalysis::setIAUCtimes(
  const std::vector<double> &times, bool convertToMins, bool IAUCAtPeak)
{
	IAUCTimes_ = times;
	std::sort(IAUCTimes_.begin(), IAUCTimes_.end());

	IAUCTMinutes_ = times;
	if (convertToMins)
	{
		for (auto &t : IAUCTMinutes_)
			t /= 60;
	}
	IAUCAtPeak_ = IAUCAtPeak;	
}

//
MDM_API void mdm_VolumeAnalysis::setUseNoise(bool b)
{
  useNoise_ = b;
}

//
MDM_API void mdm_VolumeAnalysis::setFirstImage(size_t t)
{
  firstImage_ = t;
}

//
MDM_API void mdm_VolumeAnalysis::setLastImage(size_t t)
{
  lastImage_ = t;
}

//
MDM_API void mdm_VolumeAnalysis::setOptimisationType(const std::string& type)
{
  optimisationType_ = type;
}

//
MDM_API void mdm_VolumeAnalysis::setMaxIterations(int maxItr)
{
	maxIterations_ = maxItr;
}

//
MDM_API void mdm_VolumeAnalysis::setInitMapParams(const std::vector<int> &params)
{
  initMapParams_ = params;
}

//
MDM_API void  mdm_VolumeAnalysis::fitDCEModel(
  bool optimiseModel, const std::vector<int> initMapParams)
{
  checkDynamicsSet();
  checkModelSet();

  //
  initialiseParameterMaps(*model_);

  //Fit the model
  fitModel(*model_, optimiseModel);

}

//------------------------------------------------------------------
// Private
//------------------------------------------------------------------

//
size_t mdm_VolumeAnalysis::numSt() const
{
  return StDataMaps_.size();
}

//
size_t mdm_VolumeAnalysis::numCtSignal() const
{
  return CtDataMaps_.size();
}

//
size_t mdm_VolumeAnalysis::numCtModel() const
{
  return CtModelMaps_.size();
}

//
void mdm_VolumeAnalysis::checkModelSet() const
{
  if (!model_)
    throw mdm_exception(__func__, "Model not set");
}

void mdm_VolumeAnalysis::checkDynamicsSet() const
{
  if (!numDynamics())
    throw mdm_exception(__func__, "Dynamic maps not loaded.");
}

//
void mdm_VolumeAnalysis::setDynamicMetaData(const mdm_Image3D& img)
{
  dynamicMetaData_ = std::make_unique<mdm_Image3D::MetaData>(img.info());
  auto msg = boost::format(
    "Acquisition parameters for dynamic series set from %1%: \n"
    "    TR = %2% ms\n"
    "    FA = %3% deg")
    % dynamicMetaData_->xtrSource 
    % dynamicMetaData_->TR.value() 
    % dynamicMetaData_->flipAngle.value();

  mdm_ProgramLogger::logProgramMessage(msg.str());
}

//
void mdm_VolumeAnalysis::setDynamicTime(const mdm_Image3D& img)
{
  dynamicTimes_.push_back(img.minutesFromTimeStamp());
  auto msg = boost::format(
    "Time t(%1%) = %2% mins set from %3%")
    % dynamicTimes_.size()
    % dynamicTimes_.back()
    % dynamicMetaData_->xtrSource;

  mdm_ProgramLogger::logProgramMessage(msg.str());
}

//
void mdm_VolumeAnalysis::initialiseParameterMaps(
  const mdm_DCEModelBase &model)
{
  //Model parameter maps may already have been loaded
  if (pkParamMaps_.size() != model.numParams())
    pkParamMaps_.resize(model.numParams());

  //For each map not already created, create it
  for (auto &map : pkParamMaps_)
    if (!map)
      createMap(map);

  //Create IAUC maps
  IAUCMaps_.resize(IAUCTimes_.size() + int(IAUCAtPeak_));
  for (auto &map : IAUCMaps_)
    createMap(map);

  //Model residuals may already have been loaded
  if (!modelResidualsMap_ && model.numParams())
    createMap(modelResidualsMap_);

  //Create enhancing map
  createMap(enhVoxMap_);

  //Create modelled Ct maps
  if (outputCt_mod_)
  {
    CtModelMaps_.resize(numDynamics());
    for (auto &map : CtModelMaps_)
      createMap(map);
  }
}

//
mdm_DCEVoxel mdm_VolumeAnalysis::setUpVoxel(size_t voxelIndex) const
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
    IAUCTMinutes_,
    IAUCAtPeak_);//IAUC_times

  if (computeCt_)
  {
    if (!dynamicMetaData_)
      throw mdm_exception(__func__, 
        "Attempting to convert to signal with no dynamic meta data set (eg TR, FA)");

    auto TR = dynamicMetaData_->TR.value();
    auto FA = dynamicMetaData_->flipAngle.value();
    
    auto T1 = T1Mapper_.T1(voxelIndex);
    auto M0 = useM0Ratio_ ? 0.0 : T1Mapper_.M0(voxelIndex);
    auto B1 = useB1correction_ ? T1Mapper_.B1(voxelIndex) : 1.0;

    //Convert signal (if already C(t) does nothing so can call regardless)
    vox.computeCtFromSignal(T1, FA, TR, r1Const_, M0, B1, firstImage_);
  }
    
  return vox;
}

//
void  mdm_VolumeAnalysis::voxelStData(size_t voxelIndex, std::vector<double> &data) const
{
	size_t n = numSt();

	data.resize(n);
  for (size_t k = 0; k < n; k++)
    data[k] = StDataMaps_[k].voxel(voxelIndex);
}

//
void  mdm_VolumeAnalysis::voxelCtData(size_t voxelIndex, std::vector<double> &data) const
{
	size_t n = CtDataMaps_.size();

	data.resize(n);
	for (size_t k = 0; k < n; k++)
		data[k] = CtDataMaps_[k].voxel(voxelIndex);
}

//
void  mdm_VolumeAnalysis::voxelCtModel(size_t voxelIndex, std::vector<double> &data) const
{
  size_t n = CtModelMaps_.size();

  data.resize(n);
  for (size_t k = 0; k < n; k++)
    data[k] = CtModelMaps_[k].voxel(voxelIndex);
}

//
void mdm_VolumeAnalysis::setVoxelErrors(size_t voxelIndex, const mdm_DCEVoxel &vox)
{
	//
  auto status = vox.status();
  if (status == mdm_DCEVoxel::CA_NAN)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::CA_IS_NAN);

  else if (status == mdm_DCEVoxel::DYN_T1_BAD)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::DYNT1_NEGATIVE);

  else if (status == mdm_DCEVoxel::M0_BAD)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::M0_NEGATIVE);

  else if (status == mdm_DCEVoxel::NON_ENHANCING)
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::NON_ENH_IAUC);
}

//
void mdm_VolumeAnalysis::setVoxelPreFit(size_t voxelIndex,
  const mdm_DCEVoxel  &vox, const mdm_DCEModelFitter &fitter)
{
  //Set any error codes returned from setting up the voxel in the error codes map
  setVoxelErrors(voxelIndex, vox);

  //Set any IAUC values
  for (size_t i = 0; i < IAUCMaps_.size(); i++)
    IAUCMaps_[i].setVoxel(voxelIndex, vox.IAUCVal(i));

  //Set output C(t) maps
  if (outputCt_sig_)
    for (size_t i = 0; i < numDynamics(); i++)
      CtDataMaps_[i].setVoxel(voxelIndex, vox.CtData()[i]);

  if (outputCt_mod_)
    for (size_t i = 0; i < numDynamics(); i++)
      CtModelMaps_[i].setVoxel(voxelIndex, fitter.CtModel()[i]);

  //Set enhancing status
  enhVoxMap_.setVoxel(voxelIndex, vox.enhancing());
}

//
void mdm_VolumeAnalysis::setVoxelPostFit(size_t voxelIndex,
  const mdm_DCEModelBase &model, const mdm_DCEVoxel  &vox, const mdm_DCEModelFitter &fitter,
  int &numErrors)
{
  if (!model.numParams())
    return; //When model type is NONE, do nothing

  //Check if any model fitting error codes generated
  auto errorCode = model.getModelErrorCode();
  if (errorCode != mdm_ErrorTracker::OK)
  {
    errorTracker_.updateVoxel(voxelIndex, errorCode);
    numErrors++;
  }

  //Check if we have a target model residual to match
  auto residual = fitter.modelFitError();
  auto targetResidual = modelResidualsMap_.voxel(voxelIndex);
  if (targetResidual && targetResidual < residual)
    return; //Don't update parameter maps or model residuals

  //Otherwise, residual is accepted, set parameter maps, modelled C(t) residuals
  for (size_t i = 0; i < pkParamMaps_.size(); i++)
		pkParamMaps_[i].setVoxel(voxelIndex, model.params(int(i)));
  
  if (outputCt_mod_)
    for (size_t i = 0; i < numDynamics(); i++)
      CtModelMaps_[i].setVoxel(voxelIndex, fitter.CtModel()[i]);

  modelResidualsMap_.setVoxel(voxelIndex, residual); 
    
}

//
std::vector <size_t> mdm_VolumeAnalysis::getVoxelsToFit() const
{
  std::vector<size_t> selectedVoxels;
  if (ROI_)
  {
    for (size_t idx = 0; idx < ROI_.numVoxels(); idx++)
    {
      if (ROI_.voxel(idx))
        selectedVoxels.push_back(idx);
    }
  }
  else
  {
    selectedVoxels.resize(errorTracker_.errorImage().numVoxels());
    std::iota(selectedVoxels.begin(), selectedVoxels.end(), 0);
  }
  return selectedVoxels;
}

//
void mdm_VolumeAnalysis::initialiseModelParams(
  const size_t voxelIndex,
  mdm_DCEModelBase &model)
{
  
  int n = model.numParams();
  std::vector<double> initialParams = model.initialParams();

  for (int i : initMapParams_)
    initialParams[i] = pkParamMaps_[i].voxel(voxelIndex); 

  model.setInitialParams(initialParams);
}

//
void mdm_VolumeAnalysis::logProgress(
  double &numProcessed, const double numVoxels)
{
  //Increments the processed count, and logs a message at every 10th % complete
  numProcessed++;
  double pctComplete = 100.0*numProcessed / numVoxels;
  if (pctComplete >= pctTarget_)
  {
    mdm_ProgramLogger::logProgramMessage(std::to_string(int(pctComplete)) + "% voxels fitted.");
    pctTarget_ += 10;
  }
    
}

//
void  mdm_VolumeAnalysis::fitModel(
  mdm_DCEModelBase &model,
  bool optimiseModel)
{
  //Create a new fitter object
  mdm_DCEModelFitter modelFitter(
    model,
    firstImage_,
    lastImage_ ? lastImage_ : numDynamics(),
    noiseVar_,
    optimisationType_,
    maxIterations_
  );

  // Get list of voxels to fit
  std::vector<size_t> selectedVoxels = getVoxelsToFit();
  auto numVoxels = selectedVoxels.size();
	double numProcessed = 0;
	int numErrors = 0;
  pctTarget_ = 10;
  bool paramMapsInitialised = !initMapParams_.empty();

  //Away we go...
  mdm_ProgramLogger::logProgramMessage(
    "Fitting " + modelType() + " to " + std::to_string(numVoxels) + " voxels");
	auto fit_start = std::chrono::system_clock::now();
  for(const auto voxelIndex : selectedVoxels)
  {
    //If compute Ct from signal, skip voxels with invalid T1    
    if (computeCt_ && T1Mapper_.T1(voxelIndex) <= 0.0)
      continue;
    
    //Check if we've got parameter maps with values to initialise each voxel
    //if not the existing values set in the model will be used
    if (paramMapsInitialised)
      initialiseModelParams(voxelIndex, model);
          
    //Set up the DCE voxel object
    mdm_DCEVoxel vox(setUpVoxel(voxelIndex));

    //Compute IAUC
    vox.computeIAUC();

    //Run an initial fit (does not optimise parameters, but
    //sets bounds on model parameters, and compute the model residual
    //for the initial model parameters
    modelFitter.initialiseModelFit(vox.CtData());

    //Test enhancement
    if (testEnhancement_)
      vox.testEnhancing();
      
    //Set values that don't depend on model fitting
    setVoxelPreFit(voxelIndex, vox, modelFitter);

    //The main event: If optimising the model fit, do so now
    if (optimiseModel)
      modelFitter.fitModel(vox.status());

    //Set all the necessary values in the output maps
    setVoxelPostFit(voxelIndex, model, vox, modelFitter, numErrors);

		logProgress(numProcessed, double(numVoxels));
  }

	// Get end time and log results
	auto fit_end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = fit_end - fit_start;
	
	std::stringstream ss;
	ss << "mdm_VolumeAnalysis: Processed " << 
		numProcessed << " voxels in " << elapsed_seconds.count() << "s.\n" << 
		numErrors << " voxels returned fit errors\n";
	mdm_ProgramLogger::logProgramMessage(ss.str());
}

//
void mdm_VolumeAnalysis::createMap(mdm_Image3D& img)
{
  if (!errorTracker_.errorImage())
    throw mdm_exception(__func__,
      "Attempting to create parameter maps before any other images have been set to"
      " to determine reference dimensions.");

  img.copy(errorTracker_.errorImage());
	img.setType(mdm_Image3D::ImageType::TYPE_KINETICMAP);
}
