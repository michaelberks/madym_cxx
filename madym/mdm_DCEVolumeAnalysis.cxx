/**
 *  @file    mdm_DCEVolumeAnalysis.cxx
 *  @brief   

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

#include "mdm_version.h"
#include "mdm_ProgramLogger.h"

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

MDM_API mdm_DCEVolumeAnalysis::mdm_DCEVolumeAnalysis(mdm_ErrorTracker &errorTracker, mdm_T1VolumeAnalysis &T1_mapper)
	:errorTracker_(errorTracker),
	T1_mapper_(T1_mapper),
	testEnhancement_(false),
	useRatio_(true),
  outputCt_sig_(false),
  outputCt_modod_(false),
  useNoise_(false),
  dynamicTimes_(0),
  noiseVar_(0),
  firstImage_(0),
  lastImage_(-1),
  model_(NULL)
{
	setIAUCtimes({ 60.0, 90.0, 120.0 }, true);
}

MDM_API mdm_DCEVolumeAnalysis::~mdm_DCEVolumeAnalysis()
{

}

MDM_API void mdm_DCEVolumeAnalysis::setROIimage(const mdm_Image3D ROI)
{
	/*MB TODO find out where we actually do this*/
	ROI_image_ = ROI;
}

MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::ROIimage() const
{
	return ROI_image_;
}

MDM_API void mdm_DCEVolumeAnalysis::addStDataMap(const mdm_Image3D dynImg)
{
	//Add the image to the list
	StDataMaps_.push_back(dynImg);

	//Extract the time from the header
	double t = dynImg.getTimeStamp();
	dynamicTimes_.push_back(timeFromTimeStamp(t));

  if (useNoise_)
  {
    double noise = dynImg.info_.noiseSigma.value();
    if (noise != NAN)
      noiseVar_.push_back(noise);
  }
  
	if (outputCt_sig_ && (CtDataMaps_.size() == StDataMaps_.size()-1))
	{
		mdm_Image3D catMap;
		catMap.copyFields(dynImg);
    catMap.setTimeStamp(t);
		catMap.setType(mdm_Image3D::imageType::TYPE_CAMAP);
		CtDataMaps_.push_back(catMap);
	}
  if (outputCt_modod_ && (CtModelMaps_.size() == StDataMaps_.size() - 1))
  {
    mdm_Image3D cModMap;
    cModMap.copyFields(dynImg);
    cModMap.setType(mdm_Image3D::imageType::TYPE_CAMAP);
    CtModelMaps_.push_back(cModMap);
  }
  
}

MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::StDataMap(int i) const
{
	//assert(i >= 0 && i < getNDyns());
	return StDataMaps_[i];
}

MDM_API const std::vector<mdm_Image3D> & mdm_DCEVolumeAnalysis::StDataMaps() const
{
  return StDataMaps_;
}

MDM_API int  mdm_DCEVolumeAnalysis::getNDyns() const
{
	if (StDataMaps_.empty())
		return CtDataMaps_.size();

  return StDataMaps_.size();
}

MDM_API int   mdm_DCEVolumeAnalysis::getNCs_t() const
{
	return CtDataMaps_.size();
}

MDM_API int   mdm_DCEVolumeAnalysis::getNCm_t() const
{
  return CtModelMaps_.size();
}

MDM_API void mdm_DCEVolumeAnalysis::addCtDataMap(const mdm_Image3D ctMap)
{
	//Add the image to the list
	CtDataMaps_.push_back(ctMap);

	//Extract the time from the header
	double t = timeFromTimeStamp(ctMap.getTimeStamp());
	dynamicTimes_.push_back(t);

  //Check if there is a noise variance associated with the volume
  if (useNoise_)
  {
    double noise = ctMap.info_.noiseSigma.value();
    if (!isnan(noise))
      noiseVar_.push_back(noise);
  }
}

MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::CtDataMap(int i) const
{
	return CtDataMaps_[i];
}

MDM_API const std::vector<mdm_Image3D> & mdm_DCEVolumeAnalysis::CtDataMaps() const
{
  return CtDataMaps_;
}

MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::CtModelMap(int i) const
{
  return CtModelMaps_[i];
}

MDM_API const std::vector<mdm_Image3D> & mdm_DCEVolumeAnalysis::CtModelMaps() const
{
  return CtModelMaps_;
}

MDM_API mdm_Image3D mdm_DCEVolumeAnalysis::modelMap(const std::string &mapName) const
{
  if (model_)
  {
    for (int i = 0; i < model_->num_dims(); i++)
    {
      if (mapName == model_->pkParamName(i))
        return pkParamMaps_[i];
    }
  }

	for (int i = 0; i < IAUCTimes_.size(); i++)
	{
		if (mapName == (MAP_NAME_IAUC + std::to_string(int(IAUCTimes_[i]))))
			return IAUCMaps_[i];
	}	

	if (mapName == MAP_NAME_RESDIUALS)
		return modelResidualsMap_;

	if (mapName == MAP_NAME_ENHANCING)
		return enhVoxMap_;
	
	//Error map name not recognised
	std::cerr << "Map name " << mapName << " not recognised" << std::endl;
	std::abort();
}

MDM_API void mdm_DCEVolumeAnalysis::setModelMap(const std::string &mapName, const mdm_Image3D &map)
{
  if (pkParamMaps_.size() != model_->num_dims())
    pkParamMaps_.resize(model_->num_dims());

  for (int i = 0; i < model_->num_dims(); i++)
  {
    if (mapName == model_->pkParamName(i))
    {
      pkParamMaps_[i] = map;
      return;
    }  
  }

  for (int i = 0; i < IAUCTimes_.size(); i++)
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
  std::cerr << "Map name " << mapName << " not recognised" << std::endl;
  std::abort();
}

MDM_API std::string mdm_DCEVolumeAnalysis::modelType() const
{
  if (model_)
    return model_->modelType();
  else
    return "";
}

MDM_API std::vector<double> mdm_DCEVolumeAnalysis::dynamicTimes() const
{
	return dynamicTimes_;
}

MDM_API double mdm_DCEVolumeAnalysis::dynamicTime(int i) const
{
	return dynamicTimes_[i];
}

MDM_API std::vector<std::string> mdm_DCEVolumeAnalysis::paramNames() const
{
	return model_->pkParamNames();
}

MDM_API std::vector<double> mdm_DCEVolumeAnalysis::IAUCtimes() const
{
	return IAUCTimes_;
}

MDM_API void mdm_DCEVolumeAnalysis::setRelaxCoeff(double rc)
{
	r1Const_ = rc;
}

//Flag for which model we're using - MB TODO make this an Enum
MDM_API void mdm_DCEVolumeAnalysis::setModel(mdm_DCEModelBase *model)
{
	model_ = model;
}

//Flag to check if we're testing for enhancment
MDM_API void mdm_DCEVolumeAnalysis::setTestEnhancement(bool flag)
{
	testEnhancement_ = flag;
}

//Flag to check if we're using ratio method for converting to concentration
MDM_API void mdm_DCEVolumeAnalysis::setUseRatio(bool flag)
{
	useRatio_ = flag;
}

//Flag to see if we need to compute concentration
MDM_API void mdm_DCEVolumeAnalysis::setComputeCt(bool flag)
{
	computeCt_ = flag;
}

//Flag to see if we need to output computed concentration
MDM_API void mdm_DCEVolumeAnalysis::setOutputCt(bool flag)
{
	outputCt_sig_ = flag;
}

//Flag to see if we need to output modelled concentration
MDM_API void mdm_DCEVolumeAnalysis::setOutputCmod(bool flag)
{
  outputCt_modod_ = flag;
}

//Set the time points at which we calculate IAUC
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

//Set whether we're using temporal varying noise if it's econded in the dynamic series xtr headers
MDM_API void mdm_DCEVolumeAnalysis::setUseNoise(bool b)
{
  useNoise_ = b;
}

//Set time ranges for computing model fit
MDM_API void mdm_DCEVolumeAnalysis::setFirstImage(int t)
{
  firstImage_ = t;
}
MDM_API void mdm_DCEVolumeAnalysis::setLastImage(int t)
{
  lastImage_ = t;
}

/**  
 * Pre-conditions:
 * -  Dynamic series loaded and holding valid data
 *
 * Note:  NOT a stand-alone fn - see pre-conditions
 *
 * @author   GJM Parker
 * @brief    Fill the array "data" with the value at voxelIndex from dynamic images "start" to "end"
 * @version  madym 1.21.alpha
 * @param    voxelIndex  Linear index of current voxel
 * @param    start       Integer index of first req'd image from dynamic series
 * @param    end         Integer index of last req'd image from dynamic series
 * @param    data        Pointer to double array to hold time series signal intensities
 * @return   double *     Pointer to array with time-series of signal intensities
 */
void  mdm_DCEVolumeAnalysis::getSignalsFromVoxel(int voxelIndex, std::vector<double> &data)
{
	size_t n = StDataMaps_.size();

	data.resize(n);
  for (int k = 0; k < n; k++)
    data[k] = StDataMaps_[k].getVoxel(voxelIndex);

}

void  mdm_DCEVolumeAnalysis::getCs_tFromVoxel(int voxelIndex, std::vector<double> &data)
{
	size_t n = CtDataMaps_.size();

	data.resize(n);
	for (int k = 0; k < n; k++)
		data[k] = CtDataMaps_[k].getVoxel(voxelIndex);
}

void  mdm_DCEVolumeAnalysis::getCm_tFromVoxel(int voxelIndex, std::vector<double> &data)
{
  size_t n = CtModelMaps_.size();

  data.resize(n);
  for (int k = 0; k < n; k++)
    data[k] = CtModelMaps_[k].getVoxel(voxelIndex);
}

void mdm_DCEVolumeAnalysis::setVoxelErrors(int voxelIndex, const mdm_DCEVoxel &vox)
{
	/*
   * Check array for NaNs and log in error code map
   * Also check for CA(t) < -101, indicating negative T1(t)
   * Lower level functions don't carry voxel knowledge so we must check here
   * so we can map to the correct voxel
   */
  int voxelOk = vox.status();
  if (voxelOk == mdm_DCEVoxel::CA_NAN)
  {
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::CA_IS_NAN);
  }
  else if (voxelOk == mdm_DCEVoxel::DYN_T1_BAD)
  {
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::DYNT1_NEGATIVE);
  }
  else if (voxelOk == mdm_DCEVoxel::M0_BAD)
  {
    errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::M0_NEGATIVE);
  }
}

/**
 * Pre-conditions:
 * -  All parameter maps malloced
 * -  Valid values in Permeability struct fields
 *
 * Post-conditions
 * -  Voxel values set in parameter maps
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker (moved to function by GA Buonaccorsi)
 * @brief    Set voxels in all parameter maps from Permeability struct
 * @version  madym 1.22
 * @param    voxelIndex   Linear index of current voxel
 * @param    perm         Pointer to Permeability struct with values to set
 */
void mdm_DCEVolumeAnalysis::setVoxelInAllMaps(int voxelIndex, const mdm_DCEVoxel  &vox)
{
	for (int i = 0; i < pkParamMaps_.size(); i++)
		pkParamMaps_[i].setVoxel(voxelIndex, model_->pkParams(i));

	for (int i = 0; i < IAUCMaps_.size(); i++)
		IAUCMaps_[i].setVoxel(voxelIndex, vox.IAUC_val(i));
  
  setVoxelModelError(voxelIndex, vox);
  enhVoxMap_.setVoxel(voxelIndex,  vox.enhancing());

  /** 2.0 */
  if (outputCt_sig_)
  {
    for (int i = 0; i < getNDyns(); i++)
    {
      CtDataMaps_[i].setVoxel(voxelIndex, vox.CtData()[i]);
    }
  }
  if (outputCt_modod_)
  {
    for (int i = 0; i < getNDyns(); i++)
    {
      CtModelMaps_[i].setVoxel(voxelIndex, vox.CtModel()[i]);
    }
  }
}

void mdm_DCEVolumeAnalysis::setVoxelModelError(int voxelIndex, const mdm_DCEVoxel  &v)
{
  modelResidualsMap_.setVoxel(voxelIndex, v.modelFitError());
}

/**
 * Pre-conditions:
 * -  All parameter maps malloced
 *
 * Post-conditions
 * -  Selected voxel values set to NaN in all parameter maps
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GA Buonaccorsi
 * @brief    NaN pixels in all parameter maps
 * @version  madym 1.22
 * @param    voxelIndex   Linear index of current voxel
 */

void mdm_DCEVolumeAnalysis::nanVoxelInAllMaps(int voxelIndex)
{
  int i;
	for (int i = 0; i < pkParamMaps_.size(); i++)
		pkParamMaps_[i].setVoxel(voxelIndex, NAN);

	for (int i = 0; i < IAUCMaps_.size(); i++)
		IAUCMaps_[i].setVoxel(voxelIndex, NAN);

  modelResidualsMap_.setVoxel(voxelIndex, NAN);
  enhVoxMap_.setVoxel(voxelIndex, NAN);

  /** 1.22 */
  if (outputCt_sig_)
  {
    for (i = 0; i < getNDyns(); i++)
    {
      CtDataMaps_[i].setVoxel(voxelIndex, NAN);
    }
  }
  if (outputCt_modod_)
  {
    for (i = 0; i < getNDyns(); i++)
    {
      CtModelMaps_[i].setVoxel(voxelIndex, NAN);
    }
  }
}

/**
 * Pre-conditions:
 * -  All parameter maps malloced
 *
 * Post-conditions
 * -  Selected voxel values set to zero in all parameter maps
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker (moved to function by GA Buonaccorsi)
 * @brief    Zero pixels in all parameter maps
 * @version  madym 1.22
 * @param    voxelIndex   Linear index of current voxel
 */
void mdm_DCEVolumeAnalysis::zeroVoxelInAllMaps(int voxelIndex)
{
  int i;

	for (int i = 0; i < pkParamMaps_.size(); i++)
		pkParamMaps_[i].setVoxel(voxelIndex, 0);

	for (int i = 0; i < IAUCMaps_.size(); i++)
		IAUCMaps_[i].setVoxel(voxelIndex, 0);

  modelResidualsMap_.setVoxel(voxelIndex, 0.0);
  enhVoxMap_.setVoxel(voxelIndex,   0.0);

  /** 1.22 */
  if (outputCt_sig_)
  {
    for (i = 0; i < getNDyns(); i++)
    {
      CtDataMaps_[i].setVoxel(voxelIndex, 0.0);
    }
  }

  if (outputCt_modod_)
  {
    for (i = 0; i < getNDyns(); i++)
    {
      CtModelMaps_[i].setVoxel(voxelIndex, 0.0);
    }
  }
}

/**
 * Pre-conditions:
 * -  Dynamic series loaded and number_of_dynamics set
 * -  T1 map already generated - if not using input [CA](t) images
 * -  ROI_image already loaded and mdmCfg.ROI_flag set
 *
 * Post-conditions:
 * -  Kinetic analysis run and all output parameter maps populated
 *
 * Uses madym.h globals:
 * - mdmCfg.caMapFlag, mdmCfg.ROI_flag             (input only)
 *
 * Uses mdm_T1VolumeAnalysis.h globals:
 * - T1                                            (input only)
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker (mods by GA Buonaccorsi)
 * @brief    Fit model at all voxels and fill model parameter maps
 * @version  madym 1.21.alpha
 * @return   Integer 0 on success or 1 if no fitting was done
 */
bool  mdm_DCEVolumeAnalysis::fitModel(bool paramMapsInitialised, bool optimiseModel, const std::vector<int> initMapParams)
{
  std::string msg, errString;

  int nX, nY, nZ;
  double tr;
  double fa;

  if (!StDataMaps_.empty())
  {
    tr = StDataMaps_[0].info_.TR.value();
    fa = StDataMaps_[0].info_.flipAngle.value();
    StDataMaps_[0].getMatrixDims(nX, nY, nZ);
  }
  else
  {
    tr = CtDataMaps_[0].info_.TR.value();
    fa = CtDataMaps_[0].info_.flipAngle.value();
    CtDataMaps_[0].getMatrixDims(nX, nY, nZ);
  }
  

  /* Loop through images having fun ... */
	bool useROI = ROI_image_.getNvoxels() > 0;
	int numFitted = 0;
	int numErrors = 0;
	auto fit_start = std::chrono::system_clock::now();
  for(int ix = 0; ix < nX; ix++)
  {
		std::cout << "Processing x = " << ix << std::endl;

    for(int iy = 0; iy < nY; iy++)
    {
      for (int iz = 0; iz < nZ; iz++)
      {
        int voxelIndex = ix + (iy * nX) + (iz * nX * nY);
        
        if ((!computeCt_ || T1_mapper_.T1atVoxel(voxelIndex) > 0.0)
             && (!useROI || ROI_image_.getVoxel(voxelIndex) > 0.0))
        {
          //Check if we've got parameter maps with values to initialise each voxel
          //if not the existing values set in the model will be used
          if (paramMapsInitialised)
          {
            int n = model_->num_dims();
						std::vector<double> initParams = model_->pkInitParams();

						if (initMapParams.empty())
							for (int i = 0; i < n; i++)
								initParams[i] = pkParamMaps_[i].getVoxel(voxelIndex);
						else
							for (int i : initMapParams)
								initParams[i-1] = pkParamMaps_[i-1].getVoxel(voxelIndex); //Need -1 because user indexing starts at 1

            model_->setPkInitParams(initParams);
          }
          
          //Set up the DCE voxel object
          double t10 = 0.0;
          double s0 = 0.0;
          double r1Const = 0.0;
          if (lastImage_ < 0)
            lastImage_ = getNDyns();

          //Get signals
          std::vector<double> signalData;
          std::vector<double> CtData;

          if (computeCt_)
          {
            r1Const = r1Const_;
            t10 = T1_mapper_.T1atVoxel(voxelIndex);
            if (!useRatio_)
              s0 = T1_mapper_.M0atVoxel(voxelIndex);
            getSignalsFromVoxel(voxelIndex, signalData);
          }
          else
            getCs_tFromVoxel(voxelIndex, CtData);

          mdm_DCEVoxel vox(signalData,//dynSignals
            CtData,//dynConc
            noiseVar_,//noiseVar
            t10,//T10
            s0,//M0
            r1Const,//r1Const
            model_->AIF().prebolus(),//bolus_time
            dynamicTimes_,//dynamicTimings
            tr,//TR
            fa,//FA
            firstImage_,//n1
            lastImage_,//n2
            testEnhancement_,//testEnhancement
            useRatio_,//useRatio
						IAUCTMinutes_);//IAUC_times

          //Run an initial fit (does not optimise parameters, but ensures
          //concentration has been derived from signal if not already done,
          //sets bounds on model parameters, and compute the model residual
          //for the initial model parameters
          vox.initialiseModelFit(*model_);

          //Set any error codes returned from setting up the voxel in the error codes map
          setVoxelErrors(voxelIndex, vox);

          //Compute IAUC, this is cheap enough to do regardless, although
          //we may consider setting a flag?
          vox.calculateIAUC();

          //The main event: If optimising the model fit, do so now
          if (optimiseModel)
          {
            vox.fitModel();
            if (!vox.enhancing())
              errorTracker_.updateVoxel(voxelIndex, mdm_ErrorTracker::NON_ENH_IAUC);
          }

          //Check if any model fitting error codes generated
          int errorCode = model_->getModelErrorCode();
					if (errorCode != mdm_ErrorTracker::OK)
					{
						errorTracker_.updateVoxel(voxelIndex, errorCode);
						numErrors++;
					}
            

          //Set all the necessary values in the output maps
          setVoxelInAllMaps(voxelIndex, vox);

					numFitted++;
        }

      }
    }
  }

	// Get end time and log results
	auto fit_end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = fit_end - fit_start;
	
	std::stringstream ss;
	ss << "mdm_DCEVolumeAnalysis: Fitted " << 
		numFitted << " voxels in " << elapsed_seconds.count() << "s.\n" << 
		numErrors << " voxels returned fit errors\n";
	mdm_ProgramLogger::logProgramMessage(ss.str());
  return true;
}


/**
 * Pre-conditions:
 * -  Dynamic series loaded - used in createMap()
 *
 * Post-conditions
 * -  Memory allocated and voxel values zeroed in all parameter maps
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker (moved to function by GA Buonaccorsi)
 * @brief    Allocate memory for all output maps and zero their pixels
 * @version  madym 1.22
 */
MDM_API bool mdm_DCEVolumeAnalysis::createParameterMaps()
{
  //Model parameter maps may already have been loaded
	if (pkParamMaps_.size() != model_->num_dims())
	{
		pkParamMaps_.resize(model_->num_dims());
		for (int i = 0; i < pkParamMaps_.size(); i++)
		{
			if (!createMap(pkParamMaps_[i]))
			{
				mdm_ProgramLogger::logProgramMessage(
					"ERROR: mdm_DCEVolumeAnalysis::createParameterMaps: "
					"Could not create PK model maps\n");
				return false;
			}
		}
	}

	IAUCMaps_.resize(IAUCTimes_.size());
	for (int i = 0; i < IAUCMaps_.size(); i++)
	{
		if (!createMap(IAUCMaps_[i]))
		{
			mdm_ProgramLogger::logProgramMessage(
				"ERROR: mdm_DCEVolumeAnalysis::createParameterMaps: "
				"Could not create IAUC maps\n");
			return false;
		}
	}

	if (!createMap(modelResidualsMap_))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_DCEVolumeAnalysis::createParameterMaps: "
			"Could not create model residuals map\n");
		return false;
	}
	if (!createMap(enhVoxMap_))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_DCEVolumeAnalysis::createParameterMaps: "
			"Could not create enhancing map\n");
		return false;
	}

  //
  if (outputCt_modod_)
  {
    const int &nDyns = getNDyns();
    CtModelMaps_.resize(nDyns);
		for (int i = 0; i < nDyns; i++)
		{
			if (!createMap(CtModelMaps_[i]))
			{
				mdm_ProgramLogger::logProgramMessage(
					"ERROR: mdm_DCEVolumeAnalysis::createParameterMaps: "
					"Could not model concentration maps\n");
				return false;
			}
		}
  }
	return true;
}

//
bool mdm_DCEVolumeAnalysis::createMap(mdm_Image3D& img)
{
	if (!StDataMaps_.empty())
    img.copyFields(StDataMaps_[0]);
  else if (!CtDataMaps_.empty())
    img.copyFields(CtDataMaps_[0]);
  else
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_DCEVolumeAnalysis::createMap: Error allocating parameter maps, "
			"at least one of dynamic signal (StDataMaps_) or concentration series "
			"(CtDataMaps_) should be non-empty");
		return false;
  }
    

	img.setType(mdm_Image3D::imageType::TYPE_KINETICMAP);
	return true;
}

double mdm_DCEVolumeAnalysis::timeFromTimeStamp(double timeStamp)
{
	int hours = (timeStamp / 10000);
	int minutes = (timeStamp - 10000 * hours) / 100;
	double seconds = (timeStamp
		- 10000 * hours
		- 100 * minutes);
	double timeInSecs = double(hours) * 60 * 60
		+ double(minutes) * 60
		+ seconds;
  return timeInSecs / 60.0; //time in minutes as used as standard throughout DCE analysis
}

/**
 * @author   GJM Parker (mods by GA Buonaccorsi)
 * @brief    Do volume permeability analysis
 * @version  madym 1.21.alpha
 */
bool  mdm_DCEVolumeAnalysis::fitDCEModel(bool paramMapsInitialised, bool optimiseModel, const std::vector<int> initMapParams)
{
  /* Check we have the input files we need, either concentration maps
	or dynamic images*/
	if (computeCt_)
	{
		if (!getNDyns() || !StDataMaps_[0].getNvoxels())
		{
			mdm_ProgramLogger::logProgramMessage(
				"ERROR: mdm_DCEVolumeAnalysis::fitDCEModel: No input dynamic images - nothing to fit\n");
			return false;
		}
	}
	else if (!getNCs_t() || !CtDataMaps_[0].getNvoxels())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_DCEVolumeAnalysis::fitDCEModel: No input concentration maps - nothing to fit\n");
		return false;
	}

	/* Allocate all the output maps */
	if (!createParameterMaps())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_DCEVolumeAnalysis::fitDCEModel: Could not create parameter maps\n");
		return false;
	}

	/*Fit the models*/
	bool models_fitted = fitModel(paramMapsInitialised, optimiseModel, initMapParams);

	if (models_fitted)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_DCEVolumeAnalysis::fitDCEModel: Error fitting models, check logs\n");
	}
	return models_fitted;
	
}
