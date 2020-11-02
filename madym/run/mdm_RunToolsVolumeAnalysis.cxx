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
MDM_API mdm_RunToolsVolumeAnalysis::mdm_RunToolsVolumeAnalysis()
	: fileManager_(volumeAnalysis_)
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

MDM_API void mdm_RunToolsVolumeAnalysis::writeOuput()
{
	if (!fileManager_.writeOutputMaps(outputPath_.string()))
	{
		std::cerr << options_parser_.exe_cmd() << ": error saving maps" << std::endl;
		//Don't quit here, we may as well try and save the error image anyway
		//nothing else depends on the success of the save
	}

	//Write out the error image
	fileManager_.writeErrorMap(errorMapPath_.string());
}

//



