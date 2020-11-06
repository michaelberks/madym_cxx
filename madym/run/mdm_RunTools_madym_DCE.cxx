/**
*  @file    mdm_RunTools_madym_DCE.cxx
*  @brief   Implementation of mdm_RunTools_madym_DCE class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_DCE.h"

#include <madym/mdm_ProgramLogger.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

//
MDM_API mdm_RunTools_madym_DCE::mdm_RunTools_madym_DCE(mdm_InputOptions &options_, mdm_OptionsParser &options_parser)
	: 
	mdm_RunToolsDCEFit(),
	mdm_RunToolsVolumeAnalysis(),
	mdm_RunTools(options_, options_parser)
{
}


MDM_API mdm_RunTools_madym_DCE::~mdm_RunTools_madym_DCE()
{
}

//
MDM_API int mdm_RunTools_madym_DCE::run()
{
	//Check required inputs
	checkRequiredInputs();

	//Set parameters from user inputs
	setFileManagerParams();
	setAIFParams();
	setVolumeAnalysisParams();

	//Set which type of model we're using
	setModel(options_.model(), !options_.aifName().empty(), !options_.pifName().empty(),
		options_.paramNames(), options_.initialParams(),
		options_.fixedParams(), options_.fixedValues(),
		options_.relativeLimitParams(), options_.relativeLimitValues());
	volumeAnalysis_.setModel(model_);

	//Create output folder/check overwrite
	set_up_output_folder();

	//Set up logging trail
	set_up_logging();

	//Load ROI
	loadROI();

	//Load error map if it already exists
	loadErrorMap();

	//If supplied with initial maps, load these now
	loadInitParamMaps();

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
	if (options_.inputCt())
		//Case 4: load pre-computed concentration maps
		loadCt(); 

	else 
	{
		//Cases 1 - 3: Load signal
		loadSt();
		
		if (!options_.T1Name().empty())
			//Case 2- 3: supplied an existing T1 map
			loadT1();
		
		else 
			//Case 1: mapping T1 from input signal volumes
			mapT1();
	}

	//Load the AIF/PIF from file if filename given - must do this *after* 
	//loading either dynamic signals or concentration maps
	loadAIF();

	/*If we're here, then we should have either:
	1) A set of concentration images in volumeAnalysis OR
	2) A set of dynamic images and a T1 map and either an M0 map or the use baseline M0 flag set to false*/

	//Finally, we can do the actual model fitting
	fitModel();

	//Write output
	writeOuput();

	return mdm_progExit();
}

//
MDM_API int mdm_RunTools_madym_DCE::parseInputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("madym options_");
	po::options_description config_options("madym config options_");

	options_parser_.add_option(cmdline_options, options_.configFile);
	options_parser_.add_option(cmdline_options, options_.dataDir);

	options_parser_.add_option(config_options, options_.inputCt);
	options_parser_.add_option(config_options, options_.dynName);
	options_parser_.add_option(config_options, options_.dynDir);
	options_parser_.add_option(config_options, options_.dynFormat);
	options_parser_.add_option(config_options, options_.nDyns);
	options_parser_.add_option(config_options, options_.injectionImage);

	//ROI options_
	options_parser_.add_option(config_options, options_.roiName);
	options_parser_.add_option(config_options, options_.T1method);
	options_parser_.add_option(config_options, options_.T1inputNames);
	options_parser_.add_option(config_options, options_.T1noiseThresh);
	options_parser_.add_option(config_options, options_.nT1Inputs);

		//Signal to concentration options_
	options_parser_.add_option(config_options, options_.M0Ratio);
	options_parser_.add_option(config_options, options_.T1Name);
	options_parser_.add_option(config_options, options_.M0Name);
	options_parser_.add_option(config_options, options_.r1Const);

		//AIF options_
	options_parser_.add_option(config_options, options_.aifName);
	options_parser_.add_option(config_options, options_.pifName);
	options_parser_.add_option(config_options, options_.aifSlice);
	options_parser_.add_option(config_options, options_.dose);
	options_parser_.add_option(config_options, options_.hct);

		//Model options_
	options_parser_.add_option(config_options, options_.model);
	options_parser_.add_option(config_options, options_.initialParams);
	options_parser_.add_option(config_options, options_.initMapsDir);
	options_parser_.add_option(config_options, options_.initMapParams);
	options_parser_.add_option(config_options, options_.paramNames);
	options_parser_.add_option(config_options, options_.fixedParams);
	options_parser_.add_option(config_options, options_.fixedValues);
	options_parser_.add_option(config_options, options_.relativeLimitParams);
	options_parser_.add_option(config_options, options_.relativeLimitValues);
	options_parser_.add_option(config_options, options_.firstImage);
	options_parser_.add_option(config_options, options_.lastImage);

	options_parser_.add_option(config_options, options_.noOptimise);
	options_parser_.add_option(config_options, options_.dynNoise);
	options_parser_.add_option(config_options, options_.testEnhancement);
	options_parser_.add_option(config_options, options_.maxIterations);

		//DCE only output options_
	options_parser_.add_option(config_options, options_.outputCt_sig);
	options_parser_.add_option(config_options, options_.outputCt_mod);
	options_parser_.add_option(config_options, options_.IAUCTimes);

		//General output options_
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.overwrite);
	options_parser_.add_option(config_options, options_.sparseWrite);

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

void mdm_RunTools_madym_DCE::checkRequiredInputs()
{
	if (options_.model().empty())
		mdm_progAbort("model (option -m) must be provided");

	if (!options_.T1Name().empty() && options_.T1Name().at(0) == '-')
		mdm_progAbort("Error no value associated with T1 map name from command-line");

	if (!options_.M0Name().empty() && options_.M0Name().at(0) == '-')
		mdm_progAbort("Error no value associated with M0 map name from command-line");

	if (!options_.dynName().empty() && options_.dynName().at(0) == '-')
		mdm_progAbort("Error no value associated with dynamic series file name from command-line");
}

void mdm_RunTools_madym_DCE::setFileManagerParams()
{
	fileManager_.setWriteCtDataMaps(options_.outputCt_sig());
	fileManager_.setWriteCtModelMaps(options_.outputCt_mod());
	fileManager_.setSparseWrite(options_.sparseWrite());
}

void mdm_RunTools_madym_DCE::setAIFParams()
{
	AIF_.setPrebolus(options_.injectionImage());
	AIF_.setHct(options_.hct());
	AIF_.setDose(options_.dose());
}

void mdm_RunTools_madym_DCE::setVolumeAnalysisParams()
{
	volumeAnalysis_.setComputeCt(!options_.inputCt());
	volumeAnalysis_.setOutputCt(options_.outputCt_sig());
	volumeAnalysis_.setOutputCmod(options_.outputCt_mod());
	volumeAnalysis_.setRelaxCoeff(options_.r1Const());
	volumeAnalysis_.setTestEnhancement(options_.testEnhancement());
	volumeAnalysis_.setUseNoise(options_.dynNoise());
	volumeAnalysis_.setM0Ratio(options_.M0Ratio());
	if (options_.firstImage())
		volumeAnalysis_.setFirstImage(options_.firstImage() - 1);
	if (options_.lastImage() > 0)
		volumeAnalysis_.setLastImage(options_.lastImage());
	volumeAnalysis_.setIAUCtimes(options_.IAUCTimes());
	volumeAnalysis_.setMaxIterations(options_.maxIterations());
}

//
void mdm_RunTools_madym_DCE::loadSt()
{
	fs::path dynPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
	std::string dynPrefix = dynPath.filename().string();
	std::string dynBasePath = dynPath.remove_filename().string();

	if (!volumeAnalysis_.modelType().empty())
	{
		//If not case 4, and we want to fit a model we *must* have dynamic images
		if (dynBasePath.empty() && dynPrefix.empty())
			mdm_progAbort("paths and/or prefix to dynamic images not set");

		//Load the dynamic images
		if (!fileManager_.loadStDataMaps(dynBasePath, dynPrefix, options_.nDyns(), options_.dynFormat()))
			mdm_progAbort("error loading dynamic images");
		
	}
}

//
void mdm_RunTools_madym_DCE::loadCt()
{
	fs::path CtPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
	std::string catPrefix = CtPath.filename().string();
	std::string catBasePath = CtPath.remove_filename().string();
	if (!catBasePath.empty() && !catPrefix.empty())
	{
		if (!fileManager_.loadCtDataMaps(catBasePath, catPrefix, options_.nDyns(), options_.dynFormat()))
			mdm_progAbort("error loading catMaps");

		//Set the times in the AIF from the dynamic times
		AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());
	}
	else
		mdm_progAbort("Ct flag set to true, but paths and/or prefix to Ct maps not set");
}

//
void mdm_RunTools_madym_DCE::loadT1()
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
void mdm_RunTools_madym_DCE::mapT1()
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

//
void mdm_RunTools_madym_DCE::loadAIF()
{
	//Set the times in the AIF from the dynamic times
	AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());

	//Load AIF
	if (!options_.aifName().empty())
	{
		std::string aifPath = fs::absolute(options_.aifName()).string();
		if (aifPath.empty())
		{
			mdm_progAbort(options_.model() + " chosen as model but no AIF filename set");
		}
		if (!AIF_.readAIF(aifPath, volumeAnalysis_.numDynamics()))
		{
			mdm_progAbort("error loading AIF for model " + options_.model());
		}
	}

	//Load PIF
	if (!options_.pifName().empty())
	{
		std::string pifPath = fs::absolute(options_.pifName()).string();
		if (pifPath.empty())
		{
			mdm_progAbort(options_.model() + " chosen as model but no AIF filename set");
		}
		if (!AIF_.readPIF(pifPath, volumeAnalysis_.numDynamics()))
		{
			mdm_progAbort("error loading PIF for model " + options_.model());
		}
	}
}

//
void mdm_RunTools_madym_DCE::loadInitParamMaps()
{
	paramMapsInitialised_ = false;
	if (!options_.initMapsDir().empty())
	{
		fs::path initMapsPath = fs::absolute(options_.initMapsDir());
		if (!fileManager_.loadParameterMaps(initMapsPath.string()))
		{
			mdm_progAbort("error loading parameter maps");
		}
		paramMapsInitialised_ = true;
	}
}

//
void mdm_RunTools_madym_DCE::fitModel()
{
	bool modelsFitted =
		volumeAnalysis_.fitDCEModel(
			paramMapsInitialised_,
			!options_.noOptimise(),
			options_.initMapParams());

	if (!modelsFitted)
		mdm_progAbort("error fitting models");

}