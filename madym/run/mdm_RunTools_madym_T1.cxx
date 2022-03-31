/**
*  @file    mdm_RunTools_madym_T1.cxx
*  @brief   Implementation of mdm_RunTools_madym_T1 class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_T1.h"

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>
#include <madym/t1/mdm_T1FitterBase.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_T1::mdm_RunTools_madym_T1()
{}


MDM_API mdm_RunTools_madym_T1::~mdm_RunTools_madym_T1()
{}

MDM_API void mdm_RunTools_madym_T1::run()
{
	//Check inputs set by user
	if (options_.T1inputNames().empty())
    throw mdm_exception(__func__, "Input map names (option --T1_vols) must be provided");

  //Set curent working dir
  set_up_cwd();

  //Set file manager options
  setFileManagerParams();

	//Create output folder/check overwrite
	set_up_output_folder();

	//Set up logging and audit trail
	set_up_logging();

	//Load existing error image if it exists
	loadErrorTracker();

	//Load ROI
	loadROI();

  //Map T1
  mapT1();

	//Write output
	fileManager_.saveGeneralOutputMaps(outputPath_.string());
	fileManager_.saveT1OutputMaps(outputPath_.string());

  //Reset the volume analysis
  volumeAnalysis_.reset();
}

//
MDM_API int mdm_RunTools_madym_T1::parseInputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("madym_T1 options_");
	po::options_description config_options("madym_T1 config options_");
	
	
	//Generic input options_ applied to all command-line tools
	options_parser_.add_option(cmdline_options, options_.help);
	options_parser_.add_option(cmdline_options, options_.version);
	options_parser_.add_option(cmdline_options, options_.configFile);
	options_parser_.add_option(cmdline_options, options_.dataDir);

	//ROI options_
	options_parser_.add_option(config_options, options_.roiName);
	options_parser_.add_option(config_options, options_.errorTrackerName);

	//T1 calculation options_
	options_parser_.add_option(config_options, options_.T1method);
	options_parser_.add_option(config_options, options_.T1Dir);
	options_parser_.add_option(config_options, options_.T1inputNames);
	options_parser_.add_option(config_options, options_.T1noiseThresh);
	options_parser_.add_option(config_options, options_.B1Scaling);
	options_parser_.add_option(config_options, options_.B1Name);
	options_parser_.add_option(config_options, options_.TR);

	//General output options_
	options_parser_.add_option(config_options, options_.outputRoot);
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.overwrite);

	//Image format options
	options_parser_.add_option(config_options, options_.imageReadFormat);
	options_parser_.add_option(config_options, options_.imageWriteFormat);

	//Logging options_
	options_parser_.add_option(config_options, options_.voxelSizeWarnOnly);
	options_parser_.add_option(config_options, options_.noLog);
	options_parser_.add_option(config_options, options_.noAudit);
	options_parser_.add_option(config_options, options_.quiet);
	options_parser_.add_option(config_options, options_.programLogName);
	options_parser_.add_option(config_options, options_.outputConfigFileName);
	options_parser_.add_option(config_options, options_.auditLogBaseName);
	options_parser_.add_option(config_options, options_.auditLogDir);

	return options_parser_.parseInputs(
		cmdline_options,
		config_options,
		options_.configFile(),
		who(),
		argc, argv);
}

MDM_API std::string mdm_RunTools_madym_T1::who() const
{
	return "madym_T1";
}
//*******************************************************************************
// Private:
//*******************************************************************************