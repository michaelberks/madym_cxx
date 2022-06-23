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

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>
#include <madym/dwi/mdm_DWIFitterBase.h>

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
	fileManager_.saveGeneralOutputMaps(outputPath_.string());
	fileManager_.saveDWIOutputMaps(outputPath_.string());

	//Reset the volume analysis
	volumeAnalysis_.reset();
}

//
MDM_API int mdm_RunTools_madym_DWI::parseInputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("madym_DWI options_");
	po::options_description config_options("madym_DWI config options_");
	
	
	//Generic input options_ applied to all command-line tools
	options_parser_.add_option(cmdline_options, options_.help);
	options_parser_.add_option(cmdline_options, options_.version);
	options_parser_.add_option(cmdline_options, options_.configFile);
	options_parser_.add_option(cmdline_options, options_.dataDir);

	//ROI options_
	options_parser_.add_option(config_options, options_.roiName);
	options_parser_.add_option(config_options, options_.errorTrackerName);

	//DWI input options_
	options_parser_.add_option(config_options, options_.DWImodel);
	options_parser_.add_option(config_options, options_.DWIDir);
	options_parser_.add_option(config_options, options_.DWIinputNames);
	options_parser_.add_option(config_options, options_.BvalsThresh);
  

	//General output options_
	options_parser_.add_option(config_options, options_.outputRoot);
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.overwrite);

	//Image format options
	options_parser_.add_option(config_options, options_.imageReadFormat);
	options_parser_.add_option(config_options, options_.imageWriteFormat);
	options_parser_.add_option(config_options, options_.niftiScaling);

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

//
void mdm_RunTools_madym_DWI::checkNumInputs(mdm_DWIModelGenerator::DWImodels methodType,
	const int& numInputs)
{
	//This is a bit rubbish - instantiating a whole new object just to get
	//some limits returned. But we want limits defined by the derived DWI method class,
	//and want to check these to parse user input before the actual fitting objects
	//get created.
	auto DWIfitter = mdm_DWIModelGenerator::createFitter(methodType);

	if (numInputs < DWIfitter->minimumInputs())
		throw mdm_exception(__func__, "not enough DWI inputs");

	else if (numInputs > DWIfitter->maximumInputs())
		throw mdm_exception(__func__, "too many DWI inputs");
}

//
void mdm_RunTools_madym_DWI::mapDWI()
{
	//Check inputs set by user
	if (options_.DWIinputNames().empty())
		throw mdm_exception(__func__, "input map names (option --DWI_vols) must be provided");

	//Parse DWI method from string, will abort if method type not recognised
	auto model = mdm_DWIModelGenerator::parseModelName(
		options_.DWImodel());

	//Check number of signal inputs, will abort if too many/too few
	checkNumInputs(model, (int)options_.DWIinputNames().size());

	//Set B-vals thresh - only needed for ivim but negligible cost to set for all methods
	volumeAnalysis_.DWIMapper().setBvalsThresh(options_.BvalsThresh());

	//Load DWI inputs
	loadDWIInputs();

	//FA images loaded, try computing DWI maps
	volumeAnalysis_.DWIMapper().setModel(model);

	volumeAnalysis_.DWIMapper().mapDWI();
}

//
void mdm_RunTools_madym_DWI::loadDWIInputs()
{
	std::vector<std::string> DWIinputPaths(0);
	for (std::string mapName : options_.DWIinputNames())
		DWIinputPaths.push_back(fs::absolute(fs::path(options_.DWIDir()) / mapName).string());

	fileManager_.loadDWIMappingInputImages(DWIinputPaths);
}