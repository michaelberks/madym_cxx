#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_T1.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_T1Voxel.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_T1::mdm_RunTools_madym_T1(mdm_InputOptions &options_, mdm_OptionsParser &options_parser)
	: 
	mdm_RunToolsT1Fit(),
	mdm_RunTools(options_, options_parser),
	T1Mapper_(errorTracker_),
	volumeAnalysis_(errorTracker_, T1Mapper_),
	fileManager_(T1Mapper_, volumeAnalysis_, errorTracker_)
{}


MDM_API mdm_RunTools_madym_T1::~mdm_RunTools_madym_T1()
{}

MDM_API int mdm_RunTools_madym_T1::run()
{
	if (options_.T1inputNames().empty())
	{
		mdm_progAbort("input map names (option --T1_vols) must be provided");
	}

	//Set which type of model we're using
	if (!setT1Method(options_.T1method()))
		mdm_progAbort("T1 method not recognised");

	T1Mapper_.setNoiseThreshold(options_.T1noiseThresh());

	//Create output folder/check overwrite
	fs::path outputPath = set_up_output_folder();

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

	/*Load in all the required images for madym processing. The user has 4 options_:
		1) Process everything from scratch:
		- Need paths to variable flip angle images to compute T1 and M0
		- Need path to dynamic images folder and the prefix with which the dynamic images are labelled
		to compute concentration images
		2) Load existing T1 and M0, use baseline M0 to scale signals
		- Need dynamic images
		3) Load existing T1, use ratio method to scale signals
		4) Load existing concentration images
		- Just need folder the concentration images are stored in and the prefix with which they're labelled
	*/

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

/**
	* @brief

	* @param
	* @return
	*/
MDM_API int mdm_RunTools_madym_T1::parse_inputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("calculate_T1 options_");
	po::options_description config_options("calculate_T1 config options_");
	
	
		//Generic input options_ applied to all command-line tools
	options_parser_.add_option(cmdline_options, options_.configFile);
	options_parser_.add_option(cmdline_options, options_.dataDir);

		//ROI options_
	options_parser_.add_option(config_options, options_.roiName);

		//T1 calculation options_
	options_parser_.add_option(config_options, options_.T1method);
	options_parser_.add_option(config_options, options_.T1inputNames);
	options_parser_.add_option(config_options, options_.T1noiseThresh);
	options_parser_.add_option(config_options, options_.nT1Inputs);

		//General output options_
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.sparseWrite);
	options_parser_.add_option(config_options, options_.overwrite);

		//Logging options_
	options_parser_.add_option(config_options, options_.errorCodesName);
	options_parser_.add_option(config_options, options_.programLogName);
	options_parser_.add_option(config_options, options_.outputConfigFileName);
	options_parser_.add_option(config_options, options_.auditLogBaseName);
	options_parser_.add_option(config_options, options_.auditLogDir);

	return options_parser_.parse_inputs(
		cmdline_options,
		config_options,
		options_.configFile(),
		argc, argv);
}
//*******************************************************************************
// Private:
//*******************************************************************************