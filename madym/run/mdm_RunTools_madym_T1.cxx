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

#include <madym/mdm_ProgramLogger.h>
#include <madym/t1_methods/mdm_T1FitterBase.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_T1::mdm_RunTools_madym_T1(mdm_InputOptions &options_, mdm_OptionsParser &options_parser)
	: 
	mdm_RunToolsT1Fit(options_, options_parser),
	mdm_RunToolsVolumeAnalysis(options_, options_parser),
	mdm_RunTools(options_, options_parser)	
{}


MDM_API mdm_RunTools_madym_T1::~mdm_RunTools_madym_T1()
{}

MDM_API int mdm_RunTools_madym_T1::run()
{
	//Check inputs set by user
	if (options_.T1inputNames().empty())
		mdm_progAbort("input map names (option --T1_vols) must be provided");

	//Parse T1 method from string, will abort if method type not recognised
	auto methodType = parseMethod(options_.T1method());

	//Check number of signal inputs, will abort if too many/too few
	checkNumInputs(methodType, options_.T1inputNames().size());

	//Create output folder/check overwrite
	set_up_output_folder();

	//Set up logging and audit trail
	set_up_logging();

	//Load existing error image if it exists
	loadErrorMap();

	//Load ROI
	loadROI();

	//Load T1 inputs
	loadT1Inputs();

	//FA images loaded, try computing T1 and M0 maps
	volumeAnalysis_.T1Mapper().setMethod(methodType);
	volumeAnalysis_.T1Mapper().setNoiseThreshold(options_.T1noiseThresh());
	volumeAnalysis_.T1Mapper().mapT1();

	//Write output
	writeOuput();

	//Tidy up the logging objects
	return mdm_progExit();
}

//
MDM_API int mdm_RunTools_madym_T1::parseInputs(int argc, const char *argv[])
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
	options_parser_.add_option(config_options, options_.outputRoot);
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.sparseWrite);
	options_parser_.add_option(config_options, options_.overwrite);

		//Logging options_
	options_parser_.add_option(config_options, options_.errorCodesName);
	options_parser_.add_option(config_options, options_.programLogName);
	options_parser_.add_option(config_options, options_.outputConfigFileName);
	options_parser_.add_option(config_options, options_.auditLogBaseName);
	options_parser_.add_option(config_options, options_.auditLogDir);

	return options_parser_.parseInputs(
		cmdline_options,
		config_options,
		options_.configFile(),
		argc, argv);
}
//*******************************************************************************
// Private:
//*******************************************************************************