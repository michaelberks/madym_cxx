/**
*  @file    mdm_RunTools_madym_DWI.cxx
*  @brief   Implementation of mdm_RunTools_madym_DWI class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_DWI.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>
#include <madym/dwi_methods/mdm_DWIFitterBase.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_DWI::mdm_RunTools_madym_DWI()
{}


MDM_API mdm_RunTools_madym_DWI::~mdm_RunTools_madym_DWI()
{}

MDM_API void mdm_RunTools_madym_DWI::run()
{
	//Check inputs set by user
	if (options_.DWIinputNames().empty())
	throw mdm_exception(__func__, "Input map names (option --DWI_vols) must be provided");

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

	//Do the diffusion model mapping
	mapDWI();

	//Write output
	writeOutput();

	//Reset the volume analysis
	volumeAnalysis_.reset();
}

//
MDM_API int mdm_RunTools_madym_DWI::parseInputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("madym_DWI options_");
	po::options_description config_options("madym_DWI config options_");
	
	
	//Generic input options_ applied to all command-line tools
	options_parser_.add_option(cmdline_options, options_.configFile);
	options_parser_.add_option(cmdline_options, options_.dataDir);

	//ROI options_
	options_parser_.add_option(config_options, options_.roiName);
	options_parser_.add_option(config_options, options_.errorTrackerName);

	//DWI input options_
	options_parser_.add_option(config_options, options_.DWImethod);
	options_parser_.add_option(config_options, options_.DWIinputNames);
	options_parser_.add_option(config_options, options_.DWInoiseThresh);
  

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

MDM_API std::string mdm_RunTools_madym_DWI::who() const
{
	return "madym_DWI";
}
//*******************************************************************************
// Private:
//*******************************************************************************