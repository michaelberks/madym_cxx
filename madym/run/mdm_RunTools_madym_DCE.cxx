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
#include <madym/mdm_exception.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

//
MDM_API mdm_RunTools_madym_DCE::mdm_RunTools_madym_DCE()
{
}


MDM_API mdm_RunTools_madym_DCE::~mdm_RunTools_madym_DCE()
{
}

//
MDM_API void mdm_RunTools_madym_DCE::run()
{
	//Check required inputs
	checkRequiredInputs();

  //Set curent working dir
  set_up_cwd();

	//Set parameters from user inputs
	setFileManagerParams();
	setAIFParams();
	setVolumeAnalysisParams();

  //Set AIF
  setAIF();

	//Set which type of model we're using, must do this after defining AIF
	setModel(options_.model(),
		options_.paramNames(), options_.initialParams(),
		options_.fixedParams(), options_.fixedValues(),
		options_.relativeLimitParams(), options_.relativeLimitValues());
	volumeAnalysis_.setModel(model_);

	//Create output folder/check overwrite
	set_up_output_folder();

	//Set up logging trail
	set_up_logging();

	//Load error map if it already exists
	loadErrorTracker();
	
	//Load ROI
	loadROI();

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

  //Load B1 map
  loadB1(options_.B1Correction());

	//Load the AIF/PIF from file if filename given - must do this *after* 
	//loading either dynamic signals or concentration maps
	loadAIF();

	/*If we're here, then we should have either:
	1) A set of concentration images in volumeAnalysis OR
	2) A set of dynamic images and a T1 map and either an M0 map or the use baseline M0 flag set to false*/

	//Finally, we can do the actual model fitting
	fitModel();

	//Write output
	writeOutput();

  //Reset the volume analysis
  volumeAnalysis_.reset();
}

//
MDM_API int mdm_RunTools_madym_DCE::parseInputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("madym_DCE options");
	po::options_description config_options("madym_DCE config options");

	options_parser_.add_option(cmdline_options, options_.configFile);
	options_parser_.add_option(cmdline_options, options_.dataDir);

	options_parser_.add_option(config_options, options_.inputCt);
	options_parser_.add_option(config_options, options_.dynName);
	options_parser_.add_option(config_options, options_.dynDir);
	options_parser_.add_option(config_options, options_.sequenceFormat);
	options_parser_.add_option(config_options, options_.nDyns);
	options_parser_.add_option(config_options, options_.injectionImage);
  options_parser_.add_option(config_options, options_.roiName);
  options_parser_.add_option(config_options, options_.errorTrackerName);

  //T1 mapping options
	options_parser_.add_option(config_options, options_.T1method);
	options_parser_.add_option(config_options, options_.T1inputNames);
	options_parser_.add_option(config_options, options_.T1noiseThresh);
  options_parser_.add_option(config_options, options_.B1Scaling);
  options_parser_.add_option(config_options, options_.B1Name);
  options_parser_.add_option(config_options, options_.TR);

	//Signal to concentration options_
	options_parser_.add_option(config_options, options_.M0Ratio);
	options_parser_.add_option(config_options, options_.T1Name);
	options_parser_.add_option(config_options, options_.M0Name);
	options_parser_.add_option(config_options, options_.r1Const);
  options_parser_.add_option(config_options, options_.B1Correction);

		//AIF options_
  options_parser_.add_option(config_options, options_.aifName);
  options_parser_.add_option(config_options, options_.aifMap);
  options_parser_.add_option(config_options, options_.pifName);
	options_parser_.add_option(config_options, options_.dose);
	options_parser_.add_option(config_options, options_.hct);

		//Model options_
	options_parser_.add_option(config_options, options_.model);
	options_parser_.add_option(config_options, options_.initialParams);
	options_parser_.add_option(config_options, options_.initMapsDir);
	options_parser_.add_option(config_options, options_.initMapParams);
  options_parser_.add_option(config_options, options_.modelResiduals);
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
	options_parser_.add_option(config_options, options_.outputRoot);
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.overwrite);

  //Image format options
  options_parser_.add_option(config_options, options_.imageReadFormat);
  options_parser_.add_option(config_options, options_.imageWriteFormat);

		//Logging options_
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

MDM_API std::string mdm_RunTools_madym_DCE::who() const
{
	return "madym_DCE";
}

//*******************************************************************************
// Private:
//*******************************************************************************

void mdm_RunTools_madym_DCE::checkRequiredInputs()
{
	if (options_.model().empty())
    throw mdm_exception(__func__, "model (option -m) must be provided");

	if (!options_.T1Name().empty() && options_.T1Name().at(0) == '-')
    throw mdm_exception(__func__, "Error no value associated with T1 map name from command-line");

	if (!options_.M0Name().empty() && options_.M0Name().at(0) == '-')
    throw mdm_exception(__func__, "Error no value associated with M0 map name from command-line");

	if (!options_.dynName().empty() && options_.dynName().at(0) == '-')
    throw mdm_exception(__func__, "Error no value associated with dynamic series file name from command-line");
}

void mdm_RunTools_madym_DCE::setFileManagerParams()
{
  //Call the base class method to set general options
  mdm_RunToolsVolumeAnalysis::setFileManagerParams();

  //Add the DCE fit specific options
	fileManager_.setSaveCtDataMaps(options_.outputCt_sig());
	fileManager_.setSaveCtModelMaps(options_.outputCt_mod());
	
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
	volumeAnalysis_.setOutputCtSig(options_.outputCt_sig());
	volumeAnalysis_.setOutputCtMod(options_.outputCt_mod());
	volumeAnalysis_.setR1Const(options_.r1Const());
  volumeAnalysis_.setPrebolusImage(options_.injectionImage());
	volumeAnalysis_.setTestEnhancement(options_.testEnhancement());
	volumeAnalysis_.setUseNoise(options_.dynNoise());
	if (options_.firstImage() > 0)
		volumeAnalysis_.setFirstImage(options_.firstImage() - 1);
	if (options_.lastImage() > 0)
		volumeAnalysis_.setLastImage(options_.lastImage() - 1);
	volumeAnalysis_.setIAUCtimes(options_.IAUCTimes());
	volumeAnalysis_.setMaxIterations(options_.maxIterations());
}

//
void mdm_RunTools_madym_DCE::loadAIF()
{
	//Set the times in the AIF from the dynamic times
	AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());

	//Load AIF from file
	if (AIF_.AIFType() == mdm_AIF::AIF_TYPE::AIF_FILE)
	{
		auto aifPath = fs::absolute(options_.aifName()).string();
    AIF_.readAIF(aifPath, volumeAnalysis_.numDynamics());
	}

  //Load AIF from voxel map
  else if (AIF_.AIFType() == mdm_AIF::AIF_TYPE::AIF_MAP)
  {
    auto aifPath = fs::absolute(options_.aifMap()).string();
    fileManager_.loadAIFmap(aifPath);

    std::vector<double> baseAIF(volumeAnalysis_.AIFfromMap());
    AIF_.setBaseAIF(baseAIF);
  }

	//Load PIF
	if (AIF_.PIFType() == mdm_AIF::PIF_TYPE::PIF_FILE)
	{
		std::string pifPath = fs::absolute(options_.pifName()).string();
		AIF_.readPIF(pifPath, volumeAnalysis_.numDynamics());
	}
}

//
void mdm_RunTools_madym_DCE::loadInitParamMaps()
{
	if (!options_.initMapsDir().empty())
	  fileManager_.loadParameterMaps(
      fs::absolute(options_.initMapsDir()).string(), options_.initMapParams());
	
  if (!options_.modelResiduals().empty())
    fileManager_.loadModelResiduals(
      fs::absolute(options_.modelResiduals()).string());
}

//
void mdm_RunTools_madym_DCE::fitModel()
{
	volumeAnalysis_.fitDCEModel(
			!options_.noOptimise(),
			options_.initMapParams());
}

void mdm_RunTools_madym_DCE::writeOutput()
{
  if (model_->numParams())
    AIF_.writeAIF(outputPath_.string() + "/AIF.txt");

  mdm_RunToolsVolumeAnalysis::writeOutput();
}