/**
*  @file    mdm_RunToolsVolumeAnalysis.cxx
*  @brief   Implementation of mdm_RunToolsVolumeAnalysis class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunToolsVolumeAnalysis.h"

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsVolumeAnalysis::mdm_RunToolsVolumeAnalysis()
	: fileManager_(volumeAnalysis_)
{
}

//
MDM_API mdm_RunToolsVolumeAnalysis::~mdm_RunToolsVolumeAnalysis()
{

}

//
//! Set-up general file manager options
void mdm_RunToolsVolumeAnalysis::setFileManagerParams()
{
  fileManager_.setImageReadFormat(options_.imageReadFormat());
  fileManager_.setImageWriteFormat(options_.imageWriteFormat());
  fileManager_.setApplyNiftiScaling(options_.niftiScaling());
  fileManager_.setXtrType(options_.useBIDS());
}

//
MDM_API void mdm_RunToolsVolumeAnalysis::loadErrorTracker()
{
  if (!options_.errorTrackerName().empty())
  {
    std::string errorTrackerPath = fs::absolute(options_.errorTrackerName()).string();
    fileManager_.loadErrorTracker(errorTrackerPath);
  }
  volumeAnalysis_.errorTracker().setVoxelSizeWarnOnly(options_.voxelSizeWarnOnly());
}

//
MDM_API void mdm_RunToolsVolumeAnalysis::loadROI()
{
	if (!options_.roiName().empty())
	{
		std::string roiPath = fs::absolute(options_.roiName()).string();
		fileManager_.loadROI(roiPath);
	}
}

//
void mdm_RunToolsVolumeAnalysis::loadDynamicTimeSeries(bool Ct)
{
  fs::path dynPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
  std::string dynPrefix = dynPath.filename().string();
  std::string dynBasePath = dynPath.remove_filename().string();

  //If not case 4, and we want to fit a model we *must* have dynamic images
  if (dynBasePath.empty() && dynPrefix.empty())
    throw mdm_exception(__func__, "paths and/or prefix to dynamic images not set");

  //Load the dynamic images
  if (options_.nifti4D())
    fileManager_.loadDynamicTimeseries(dynBasePath, dynPrefix, Ct);
  else
    fileManager_.loadDynamicTimeseries(dynBasePath, dynPrefix, options_.nDyns(),
      options_.sequenceFormat(), options_.sequenceStart(), options_.sequenceStep(), Ct);

}

//
void mdm_RunToolsVolumeAnalysis::loadT1()
{
  fs::path T1Path = fs::absolute(fs::path(options_.T1Dir()) / options_.T1Name());
  fileManager_.loadT1Map(T1Path.string());

  //Now check for cases 2 and 3, if useBaselineM0 is true
  //we need both M0 and T1, otherwise we can just use T1
  if (!options_.M0Ratio())
  {

    if (options_.M0Name().empty())
      throw mdm_exception(__func__, "If M0_ratio is false, path to M0 map must be set");

    //Otherwise load M0 and return
    fs::path M0Path = fs::absolute(fs::path(options_.T1Dir()) / options_.M0Name());
    fileManager_.loadM0Map(M0Path.string());

  }
  volumeAnalysis_.setM0Ratio(options_.M0Ratio());
}

//
MDM_API void mdm_RunToolsVolumeAnalysis::loadT1Inputs()
{
	std::vector<std::string> T1inputPaths(0);
	for (std::string mapName : options_.T1inputNames())
		T1inputPaths.push_back(fs::absolute(fs::path(options_.T1Dir()) / mapName).string());
  
  fileManager_.loadT1MappingInputImages(T1inputPaths, options_.nifti4D());
}

//! Load B1 correction map
MDM_API void mdm_RunToolsVolumeAnalysis::loadB1(bool required)
{
  //May already have been loaded in Map T1, in which case just return
  if (volumeAnalysis_.T1Mapper().B1())
    return;

  if (required)
  {

    if (options_.B1Name().empty())
      throw mdm_exception(__func__, "If using B1 correction, a path to a B1 map must be set");

    //Otherwise load M0 and return
    fs::path B1Path = fs::absolute(options_.B1Name());
    fileManager_.loadB1Map(B1Path.string(), options_.B1Scaling());
  }
  else if (!options_.B1Name().empty())
    mdm_ProgramLogger::logProgramWarning(__func__,
      "B1 map supplied, B1Correction is not set and T1 method is not VFA_B1. Map will be ignored in T1 fitting");

  volumeAnalysis_.setB1correction(required);
}

//
void mdm_RunToolsVolumeAnalysis::mapT1()
{
  //Check inputs set by user
  if (options_.T1inputNames().empty())
    throw mdm_exception(__func__, "input map names (option --T1_vols) must be provided");

  //Parse T1 method from string, will abort if method type not recognised
  auto methodType = mdm_T1MethodGenerator::parseMethodName(
    options_.T1method(), options_.B1Correction());

  //Check number of signal inputs, will abort if too many/too few
  checkNumInputs(methodType, (int)options_.T1inputNames().size());

  //Load T1 inputs
  loadT1Inputs();

  //See if B1 correction map to load
  loadB1(methodType == mdm_T1MethodGenerator::VFA_B1);

  //For inversion recovery, override TR
  if (options_.TR())
    volumeAnalysis_.T1Mapper().setBigTR(options_.TR());

  //FA images loaded, try computing T1 and M0 maps
  volumeAnalysis_.T1Mapper().setMethod(methodType);
  volumeAnalysis_.T1Mapper().setNoiseThreshold(options_.T1noiseThresh());
  volumeAnalysis_.T1Mapper().mapT1();
}

//



