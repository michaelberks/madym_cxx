#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_calculateT1.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_T1Voxel.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_calculateT1::mdm_RunTools_calculateT1(mdm_InputOptions &options, mdm_OptionsParser &options_parser)
	: 
	mdm_RunToolsT1Fit(),
	mdm_RunTools(options, options_parser),
	T1Mapper_(errorTracker_),
	volumeAnalysis_(errorTracker_, T1Mapper_),
	fileManager_(AIF_, T1Mapper_, volumeAnalysis_, errorTracker_)
{
}


MDM_API mdm_RunTools_calculateT1::~mdm_RunTools_calculateT1()
{

}

MDM_API int mdm_RunTools_calculateT1::run()
{
	if (options_.T1inputNames().empty())
	{
		mdm_progAbort("input map names (option -maps) must be provided");
	}

	if (options_.outputDir().empty())
	{
		mdm_progAbort("output directory (option -o) must be provided");
	}

	//Set which type of model we're using
	if (!setT1Method(options_.T1method()))
		return 1;

	T1Mapper_.setNoiseThreshold(options_.T1noiseThresh());

	//Using boost filesystem, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	fs::path outputPath = fs::absolute(options_.outputDir());

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	if (!is_directory(outputPath))
		create_directories(outputPath);

	// If we've got this file already warn user of previous analysis and quit
	if (!options_.overwrite() && !is_empty(outputPath))
	{
		mdm_progAbort("output directory is not empty (use option O to overwrite existing data)");
	}

	//Set up logging and audit trail
	set_up_logging(outputPath);

	//  Now it's time for the fun ...
	//  1.  Set parameter values
	//  2.  Load files
	//  3.  Do T1 map

	//Before we start, try and load an errorImage, this allows us to add
	//to any existing errors in a re-analysis
	fs::path errorCodesPath = outputPath / options_.errorCodesName();
	fileManager_.loadErrorImage(errorCodesPath.string());

	if (!options_.roiName().empty())
	{
		std::string roiPath = fs::absolute(options_.roiName()).string();
		if (!fileManager_.loadROI(roiPath))
		{
			mdm_progAbort("error loading ROI");
		}
	}

	//Load in all the required images for madym processing. The user has 4 options:
	//1) Process everything from scratch:
	//- Need paths to variable flip angle images to compute T1 and M0
	//- Need path to dynamic images folder and the prefix with which the dynamic images are labelled
	//to compute concentration images
	//2) Load existing T1 and M0, use baseline M0 to scale signals
	//- Need dynamic images
	//3) Load existing T1, use ratio method to scale signals
	//4) Load existing concentration images
	//- Just need folder the concentration images are stored in and the prefix with which they're labelled

	//We need to load FA images
	if (options_.T1inputNames().size() < mdm_T1Voxel::MINIMUM_FAS)
	{
		mdm_progAbort("not enough variable flip angle file names");
	}
	else if (options_.T1inputNames().size() > mdm_T1Voxel::MAXIMUM_FAS)
	{
		mdm_progAbort("too many variable flip angle file names");
	}

	std::vector<std::string> T1inputPaths(0);
	for (std::string mapName : options_.T1inputNames())
		T1inputPaths.push_back(fs::absolute(mapName).string());

	if (!fileManager_.loadFAImages(T1inputPaths))
	{
		mdm_progAbort("error loading FA images");
	}

	//FA images loaded, try computing T1 and M0 maps
	T1Mapper_.T1_mapVarFlipAngle();

	if (!fileManager_.writeOutputMaps(outputPath.string()))
	{
		std::cerr << options_parser_.exe_cmd() << ": error saving maps" << std::endl;
		//Don't quit here, we may as well try and save the error image anyway
		//nothing else depends on the success of the save
	}

	//Write out the error image
	fileManager_.writeErrorMap(errorCodesPath.string());

	//Tidy up the logging objects
	return mdm_progExit();
}

//*******************************************************************************
// Private:
//*******************************************************************************