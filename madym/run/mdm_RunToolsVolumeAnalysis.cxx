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

#include <madym/mdm_ProgramLogger.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsVolumeAnalysis::mdm_RunToolsVolumeAnalysis(mdm_InputOptions &options, mdm_OptionsParser &options_parser)
	: mdm_RunToolsT1Fit(options, options_parser),
  mdm_RunTools(options, options_parser),
	fileManager_(volumeAnalysis_)
{
}


MDM_API mdm_RunToolsVolumeAnalysis::~mdm_RunToolsVolumeAnalysis()
{

}

MDM_API void mdm_RunToolsVolumeAnalysis::loadErrorMap()
{
	//Before we start, try and load an errorImage, this allows us to add
	//to any existing errors in a re-analysis
	errorMapPath_ = outputPath_ / options_.errorCodesName();
	fileManager_.loadErrorMap(errorMapPath_.string());
}

MDM_API void mdm_RunToolsVolumeAnalysis::loadROI()
{
	if (!options_.roiName().empty())
	{
		std::string roiPath = fs::absolute(options_.roiName()).string();
		if (!fileManager_.loadROI(roiPath))
		{
			mdm_progAbort("error loading ROI");
		}
	}
}

//
void mdm_RunToolsVolumeAnalysis::loadSt()
{
  fs::path dynPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
  std::string dynPrefix = dynPath.filename().string();
  std::string dynBasePath = dynPath.remove_filename().string();

  //If not case 4, and we want to fit a model we *must* have dynamic images
  if (dynBasePath.empty() && dynPrefix.empty())
    mdm_progAbort("paths and/or prefix to dynamic images not set");

  //Load the dynamic images
  if (!fileManager_.loadStDataMaps(dynBasePath, dynPrefix, options_.nDyns(), options_.dynFormat()))
    mdm_progAbort("error loading dynamic images");

}

//
void mdm_RunToolsVolumeAnalysis::loadCt()
{
  fs::path CtPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
  std::string catPrefix = CtPath.filename().string();
  std::string catBasePath = CtPath.remove_filename().string();
  if (catBasePath.empty() && catPrefix.empty())
    mdm_progAbort("Ct flag set to true, but paths and/or prefix to Ct maps not set");

  if (!fileManager_.loadCtDataMaps(catBasePath, catPrefix, options_.nDyns(), options_.dynFormat()))
    mdm_progAbort("error loading catMaps");
}

//
void mdm_RunToolsVolumeAnalysis::loadT1()
{
  fs::path T1Path = fs::absolute(options_.T1Name());
  if (!fileManager_.loadT1Map(T1Path.string()))
    mdm_progAbort("error loading T1 map");

  //Now check for cases 2 and 3, if useBaselineM0 is true
  //we need both M0 and T1, otherwise we can just use T1
  if (!options_.M0Ratio())
  {

    if (options_.M0Name().empty())
      mdm_progAbort("M0MapFlag set to true, but path to M0 not set");

    //Otherwise load M0 and return
    fs::path M0Path = fs::absolute(options_.M0Name());
    if (!fileManager_.loadM0Map(M0Path.string()))
      mdm_progAbort("error loading M0 map");

  }
}

//
MDM_API void mdm_RunToolsVolumeAnalysis::loadT1Inputs()
{
	std::vector<std::string> T1inputPaths(0);
	for (std::string mapName : options_.T1inputNames())
		T1inputPaths.push_back(fs::absolute(mapName).string());

	if (!fileManager_.loadT1MappingInputImages(T1inputPaths))
	{
		mdm_progAbort("error loading FA images");
	}
}

//
void mdm_RunToolsVolumeAnalysis::mapT1()
{
  //Check inputs set by user
  if (options_.T1inputNames().empty())
    mdm_progAbort("input map names (option --T1_vols) must be provided");

  //Parse T1 method from string, will abort if method type not recognised
  auto methodType = parseMethod(options_.T1method());

  //Check number of signal inputs, will abort if too many/too few
  checkNumInputs(methodType, options_.T1inputNames().size());

  //Load T1 inputs
  loadT1Inputs();

  //FA images loaded, try computing T1 and M0 maps
  volumeAnalysis_.T1Mapper().setMethod(methodType);
  volumeAnalysis_.T1Mapper().setNoiseThreshold(options_.T1noiseThresh());
  volumeAnalysis_.T1Mapper().mapT1();
}

MDM_API void mdm_RunToolsVolumeAnalysis::writeOutput()
{
	if (!fileManager_.saveOutputMaps(outputPath_.string()))
	{
		std::cerr << options_parser_.exe_cmd() << ": error saving maps" << std::endl;
		//Don't quit here, we may as well try and save the error image anyway
		//nothing else depends on the success of the save
	}

	//Write out the error image
	fileManager_.saveErrorMap(errorMapPath_.string());
}

//



