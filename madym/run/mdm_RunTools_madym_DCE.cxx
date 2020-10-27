#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_DCE.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_T1Voxel.h>

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
	mdm_RunTools(options_, options_parser),
	T1Mapper_(errorTracker_),
	volumeAnalysis_(errorTracker_, T1Mapper_),
	fileManager_(T1Mapper_, volumeAnalysis_, errorTracker_)
{
}


MDM_API mdm_RunTools_madym_DCE::~mdm_RunTools_madym_DCE()
{

}

//
MDM_API int mdm_RunTools_madym_DCE::run()
{
	if (options_.model().empty())
		mdm_progAbort("model (option -m) must be provided");

	if (!options_.T1Name().empty() && options_.T1Name().at(0) == '-')
		mdm_progAbort("Error no value associated with T1 map name from command-line");

	if (!options_.M0Name().empty() && options_.M0Name().at(0) == '-')
		mdm_progAbort("Error no value associated with M0 map name from command-line");

	if (!options_.dynName().empty() && options_.dynName().at(0) == '-')
		mdm_progAbort("Error no value associated with dynamic series file name from command-line");

	//Set parameters in data objects from user input
	fileManager_.setWriteCtDataMaps(options_.outputCt_sig());
	fileManager_.setWriteCtModelMaps(options_.outputCt_mod());
	fileManager_.setSparseWrite(options_.sparseWrite());

	AIF_.setPrebolus(options_.injectionImage());
	AIF_.setHct(options_.hct());
	AIF_.setDose(options_.dose());

	//Set which type of model we're using
	setModel(options_.model(), !options_.aifName().empty(), !options_.pifName().empty(),
		options_.paramNames(), options_.initialParams(),
		options_.fixedParams(), options_.fixedValues(),
		options_.relativeLimitParams(), options_.relativeLimitValues());

	volumeAnalysis_.setModel(model_);
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

	T1Mapper_.setNoiseThreshold(options_.T1noiseThresh());

	//If these are empty, the defaults 60, 90, 120 will be set in volumeAnalysis
	if (!options_.IAUCTimes().empty())
		volumeAnalysis_.setIAUCtimes(options_.IAUCTimes());

	//Create output folder/check overwrite
	fs::path outputPath = set_up_output_folder();

	//Set up logging trail
	set_up_logging(outputPath);

	/*
	*  Now it's time for the fun ...
	*
	*  1.  Set parameter values
	*  2.  Load files
	*  3.  Do T1 map
	*  4.  Read dyn, AIF and ROI
	*  5.  Fit model (unless user has requested no fit)
	*/

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

	//Before we start, try and load an errorImage, this allows us to add
	//to any existing errors in a re-analysis
	fs::path errorCodesPath = outputPath / options_.errorCodesName();
	fileManager_.loadErrorImage(errorCodesPath.string());

	//Check for case 4: load pre-computed concentration maps
	if (options_.inputCt())
	{
		fs::path catPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
		std::string catPrefix = catPath.filename().string();
		std::string catBasePath = catPath.remove_filename().string();
		if (!catBasePath.empty() && !catPrefix.empty())
		{
			if (!fileManager_.loadCtDataMaps(catBasePath, catPrefix, options_.nDyns()))
			{
				mdm_progAbort("error loading catMaps");
			}

			//Set the times in the AIF from the dynamic times
			AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());
		}
		else
		{
			mdm_progAbort("Ct flag set to true, but paths and/or prefix to Ct maps not set");
		}
	}
	else
	{
		fs::path dynPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
		std::string dynPrefix = dynPath.filename().string();
		std::string dynBasePath = dynPath.remove_filename().string();

		if (!volumeAnalysis_.modelType().empty())
		{
			//If not case 4, and we want to fit a model we *must* have dynamic images
			if (dynBasePath.empty() && dynPrefix.empty())
			{
				mdm_progAbort("paths and/or prefix to dynamic images not set");
			}

			//Load the dynamic images
			if (!fileManager_.loadStDataMaps(dynBasePath, dynPrefix, options_.nDyns()))
			{
				mdm_progAbort("error loading dynamic images");
			}

			//Set the times in the AIF from the dynamic times
			AIF_.setAIFTimes(volumeAnalysis_.dynamicTimes());
		}

		//Now check if we've suuplied an existing T1 map
		if (!options_.T1Name().empty())
		{
			fs::path T1Path = fs::absolute(options_.T1Name());
			if (!fileManager_.loadT1Image(T1Path.string()))
			{
				mdm_progAbort("error loading T1 map");
			}

			//Now check for cases 2 and 3, if useBaselineM0 is true
			//we need both M0 and T1, otherwise we can just use T1
			if (!options_.M0Ratio())
			{

				if (options_.M0Name().empty())
				{
					mdm_progAbort("M0MapFlag set to true, but path to M0 not set");
				}
				//Otherwise load M0 and return
				fs::path M0Path = fs::absolute(options_.M0Name());
				if (!fileManager_.loadM0Image(M0Path.string()))
				{
					mdm_progAbort("error loading M0 map");
				}
			}
		}
		else
		{
			//Set which type of model we're using
			if (!setT1Method(options_.T1method()))
				return 1;

			//We need to load FA images
			if (options_.T1inputNames().size() < mdm_T1Voxel::MINIMUM_FAS)
			{
				mdm_progAbort("Not enough variable flip angle file names");
			}
			else if (options_.T1inputNames().size() > mdm_T1Voxel::MAXIMUM_FAS)
			{
				mdm_progAbort("Too many variable flip angle file names");
			}

			std::vector<std::string> T1inputPaths(0);
			for (std::string fa : options_.T1inputNames())
				T1inputPaths.push_back(fs::absolute(fa).string());

			if (!fileManager_.loadFAImages(T1inputPaths))
			{
				mdm_progAbort("error loading input images for baseline T1 calculation");
			}
			//FA images loaded, try computing T1 and M0 maps
			T1Mapper_.T1_mapVarFlipAngle();
		}
	}

	//Only do this stuff when we're fitting a model (ie NOT T1 only)
	if (!volumeAnalysis_.modelType().empty())
	{
		/*Load the AIF from file if using model VP auto
		*must do this *after* loading either dynamic signals or concentration maps
		*/
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

		//If supplied with initial maps, load these now
		bool paramMapsInitialised = false;
		if (!options_.initMapsDir().empty())
		{
			fs::path initMapsPath = fs::absolute(options_.initMapsDir());
			if (!fileManager_.loadParameterMaps(initMapsPath.string()))
			{
				mdm_progAbort("error loading parameter maps");
			}
			paramMapsInitialised = true;
		}

		/*If we're here, then we should have either:
		1) A set of concentration images in volumeAnalysis OR
		2) A set of dynamic images and a T1 map and either an M0 map or the use baseline M0 flag set to false*/

		/* Finally, we can do the actual model fitting inside volumeAnalysis */
		bool modelsFitted =
			volumeAnalysis_.fitDCEModel(paramMapsInitialised, !options_.noOptimise(), options_.initMapParams());

		if (!modelsFitted)
		{
			mdm_progAbort("error fitting models");
		}
	}

	/*Call the file manager to write out the output maps*/
	if (!fileManager_.writeOutputMaps(outputPath.string()))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_RunTools::run_DCEFit: error saving maps\n");
		//Don't quit here, we may as well try and save the error image anyway
		//nothing else depends on the success of the save
	}

	/*Write out the error image*/
	fileManager_.writeErrorMap(errorCodesPath.string());

	return mdm_progExit();
}

/**
	* @brief

	* @param
	* @return
	*/
MDM_API int mdm_RunTools_madym_DCE::parse_inputs(int argc, const char *argv[])
{
	po::options_description cmdline_options("madym options_");
	po::options_description config_options("madym config options_");

	options_.configFile;
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

	return options_parser_.parse_inputs(
		cmdline_options,
		config_options,
		options_.configFile(),
		argc, argv);
}

//*******************************************************************************
// Private:
//*******************************************************************************