#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools.h"

#include <iostream>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time

#include <mdm_version.h>
#include <madym/mdm_T1Voxel.h>
#include <madym/mdm_DCEModelGenerator.h>

#include <madym/mdm_ProgramLogger.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

MDM_API mdm_RunTools::mdm_RunTools(mdm_ToolsOptions &options)
  :
  T1Mapper_(errorTracker_),
  volumeAnalysis_(errorTracker_, T1Mapper_),
  model_(NULL),
  fileManager_(AIF_, T1Mapper_, volumeAnalysis_, errorTracker_),
  options_(options)
{

}
MDM_API mdm_RunTools::~mdm_RunTools()
{

}

/**
* @author   M Berks
* @brief    Exit and print message
* @version  madym 2.2+
* @param    msg   String message to print on exit
*/
MDM_API int mdm_RunTools::run_DCEFit(const std::string &exe_args, const std::string &exeString)
{
  exeString_ = exeString;

  if (options_.model.empty())
  {
    mdm_progAbort("model (option -m) must be provided");
  }

  if (options_.outputDir.empty())
  {
    mdm_progAbort("output directory (option -o) must be provided");
  }

  if (!options_.T1Name.empty() && options_.T1Name.at(0) == '-')
    mdm_progAbort("Error no value associated with T1 map name from command-line");

  if (!options_.S0Name.empty() && options_.S0Name.at(0) == '-')
    mdm_progAbort("Error no value associated with S0 map name from command-line");

  if (!options_.dynName.empty() && options_.dynName.at(0) == '-')
    mdm_progAbort("Error no value associated with dynamic series file name from command-line");

  //Set parameters in data objects from user input
  fileManager_.setWriteCtDataMaps(options_.outputCt);
  fileManager_.setWriteCtModelMaps(options_.outputCm);
  AIF_.setPrebolus(options_.injectionImage);
  AIF_.setHct(options_.hct);
  AIF_.setDose(options_.dose);

  //Set which type of model we're using
  setModel(options_.model, !options_.aifName.empty(), !options_.pifName.empty(),
    options_.paramNames, options_.initParams, options_.fixedParams, options_.fixedValues);

  volumeAnalysis_.setComputeCt(!options_.inputCt);
  volumeAnalysis_.setOutputCt(options_.outputCt);
  volumeAnalysis_.setOutputCmod(options_.outputCm);
  volumeAnalysis_.setRelaxCoeff(options_.r1Const);
  volumeAnalysis_.setTestEnhancement(!options_.enhFlag);
  volumeAnalysis_.setUseNoise(options_.dynNoise);
  volumeAnalysis_.setUseRatio(options_.useRatio);
  if (options_.firstImage)
    volumeAnalysis_.setFirstImage(options_.firstImage - 1);
  if (options_.lastImage > 0)
    volumeAnalysis_.setLastImage(options_.lastImage);

  T1Mapper_.setNoiseThreshold(options_.T1noiseThresh);

  //If these are empty, the defaults 60, 90, 120 will be set in volumeAnalysis
  if (!options_.IAUCTimes.empty())
    volumeAnalysis_.setIAUCtimes(options_.IAUCTimes);

  //Using boost filesyetm, can call one line to make absolute path from input
  //regardless of whether relative or absolute path has been given
  fs::path outputPath = fs::absolute(options_.outputDir);

  //We probably don't need to check if directory exists, just call create... regardless
  //but we do so anyway
  if (!is_directory(outputPath))
    create_directories(outputPath);

  /* If we've got this file already warn user of previous analysis and quit */
  if (!options_.overwrite && !is_empty(outputPath))
  {
    mdm_progAbort("Output directory is not empty (use option -O to overwrite existing data)");
  }

  //Set up paths to error image and audit logs, using default names if not user supplied
  // and using boost::filesystem to make absolute paths
  fs::path errorBasePath = outputPath / options_.errorBaseName;

  fs::path mdmLogBasePath = outputPath / options_.programLogBaseName;

  //Note the default audit path doesn't use the output directory (unless user specifically set so)
	fs::path auditPath = fs::absolute(options_.auditLogDir);
	if (!is_directory(auditPath))
		create_directories(auditPath);
	std::string auditName = exeString_ + timeNow() + options_.auditLogBaseName;
  fs::path auditBasePath = auditPath / auditName;

	std::string caller = exeString_ + " " + MDM_VERSION;
  mdm_ProgramLogger::openAuditLog(auditBasePath.string(), caller);
	mdm_ProgramLogger::logAuditMessage("Command args: " + exe_args);
	mdm_ProgramLogger::openProgramLog(mdmLogBasePath.string(), caller);
	mdm_ProgramLogger::logProgramMessage("Command args: " + exe_args);

  //Log location of program log in audit log
	mdm_ProgramLogger::logAuditMessage(
    "Program log saved to " + mdmLogBasePath.string() + "\n");

  /*
  *  Now it's time for the fun ...
  *
  *  1.  Set parameter values
  *  2.  Load files
  *  3.  Do T1 map
  *  4.  Read dyn, AIF and ROI
  *  5.  Fit model (unless user has requested no fit)
  */

  if (!options_.roiName.empty())
  {
    std::string roiPath = fs::absolute(options_.roiName).string();
    if (fileManager_.loadROI(roiPath))
    {
      mdm_progAbort("error loading ROI");
    }
  }

  /*Load in all the required images for madym processing. The user has 4 options:
  1) Process everything from scratch:
  - Need paths to variable flip angle images to compute T1 and S0
  - Need path to dynamic images folder and the prefix with which the dynamic images are labelled
  to compute concentration images
  2) Load existing T1 and S0, use baseline S0 to scale signals
  - Need dynamic images
  3) Load existing T1, use ratio method to scale signals
  4) Load existing concentration images
  - Just need folder the concentration images are stored in and the prefix with which they're labelled
  */
	
	//Before we start, try and load an errorImage, this allows us to add
	//to any existing errors in a re-analysis
	fileManager_.loadErrorImage(errorBasePath.string());

  //Check for case 4: load pre-computed concentration maps
  if (options_.inputCt)
  {
    fs::path catPath = fs::absolute(fs::path(options_.dynDir) / options_.dynName);
    std::string catPrefix = catPath.filename().string();
    std::string catBasePath = catPath.remove_filename().string();
    if (!catBasePath.empty() && !catPrefix.empty())
    {
      if (!fileManager_.loadCtDataMaps(catBasePath, catPrefix))
      {
        mdm_progAbort("error loading catMaps");
      }
    }
    else
    {
      mdm_progAbort("caMapFlag set to true, but paths and/or prefix to cat maps not set");
    }
  }
  else
  {
    fs::path dynPath = fs::absolute(fs::path(options_.dynDir) / options_.dynName);
    std::string dynPrefix = dynPath.filename().string();
    std::string dynBasePath = dynPath.remove_filename().string();

    if (!volumeAnalysis_.modelType().empty())
    {
      //If not case 4, and we want to fit a model we *must* have dynamic images
      if (dynBasePath.empty() && dynPrefix.empty())
      {
        mdm_progAbort("paths and/or prefix to dynamic images not set");
      }

      //Load the dynamic image
      if (!fileManager_.loadStDataMaps(dynBasePath, dynPrefix))
      {
        mdm_progAbort("error loading dynamic images");
      }
    }

    //Now check if we've suuplied an existing T1 map
    if (!options_.T1Name.empty())
    {
      fs::path T1Path = fs::absolute(options_.T1Name);
      if (!fileManager_.loadT1Image(T1Path.string()))
      {
        mdm_progAbort("error loading T1 map");
      }

      //Now check for cases 2 and 3, if useBaselineS0 is true
      //we need both S0 and T1, otherwise we can just use T1
      if (!options_.useRatio)
      {

        if (options_.S0Name.empty())
        {
          mdm_progAbort("S0MapFlag set to true, but path to S0 not set");
        }
        //Otherwise load S0 and return
        fs::path S0Path = fs::absolute(options_.S0Name);
        if (!fileManager_.loadS0Image(S0Path.string()))
        {
          mdm_progAbort("error loading S0 map");
        }
      }
    }
    else
    {
      //We need to load FA images
      if (options_.T1inputNames.size() < mdm_T1Voxel::MINIMUM_FAS)
      {
        mdm_progAbort("Not enough variable flip angle file names");
      }
      else if (options_.T1inputNames.size() > mdm_T1Voxel::MAXIMUM_FAS)
      {
        mdm_progAbort("Too many variable flip angle file names");
      }

      std::vector<std::string> T1inputPaths(0);
      for (std::string fa : options_.T1inputNames)
        T1inputPaths.push_back(fs::absolute(fa).string());

      if (!fileManager_.loadFAImages(T1inputPaths))
      {
        mdm_progAbort("error loading input images for baseline T1 calculation");
      }
      //FA images loaded, try computing T1 and S0 maps
      T1Mapper_.T1_mapVarFlipAngle();
    }
  }

  //Only do this stuff when we're fitting a model (ie NOT T1 only)
  if (!volumeAnalysis_.modelType().empty())
  {
    /*Load the AIF from file if using model VP auto
    *must do this *after* loading either dynamic signals or concentration maps
    */
    if (!options_.aifName.empty())
    {
      std::string aifPath = fs::absolute(options_.aifName).string();
      if (aifPath.empty())
      {
        mdm_progAbort(options_.model + " chosen as model but no AIF filename set");
      }
      if (fileManager_.loadAIF(aifPath))
      {
        mdm_progAbort("error loading AIF for model " + options_.model);
      }
    }
    if (!options_.pifName.empty())
    {
      std::string pifPath = fs::absolute(options_.pifName).string();
      if (pifPath.empty())
      {
        mdm_progAbort(options_.model + " chosen as model but no AIF filename set");
      }
      if (!fileManager_.loadPIF(pifPath))
      {
        mdm_progAbort("error loading AIF for model " + options_.model);
      }
    }

    //If supplied with initial maps, load these now
    bool paramMapsInitialised = false;
    if (!options_.initMapsDir.empty())
    {
      fs::path initMapsPath = fs::absolute(options_.initMapsDir);
      if (!fileManager_.loadParameterMaps(initMapsPath.string()))
      {
        mdm_progAbort("error loading parameter maps");
      }
      paramMapsInitialised = true;
    }

    /*If we're here, then we should have either:
    1) A set of concentration images in volumeAnalysis OR
    2) A set of dynamic images and a T1 map and either an S0 map or the use baseline S0 flag set to false*/

    /* Finally, we can do the actual model fitting inside volumeAnalysis */
    bool modelsFitted =
      volumeAnalysis_.fitDCEModel(paramMapsInitialised, !options_.noOptimise, options_.initMapParams);

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
  fileManager_.writeErrorMap(errorBasePath.string());

  return mdm_progExit();
}

//
MDM_API int mdm_RunTools::run_DCEFit_lite(const std::string &exe_args, const std::string &exeString)
{
	exeString_ = exeString;

	if (options_.model.empty())
	{
		mdm_progAbort("model (option -m) must be provided");
	}
	if (options_.inputDataFile.empty())
	{
		mdm_progAbort("input data file (option -i) must be provided");
	}
	if (!options_.nDyns)
	{
		mdm_progAbort("number of dynamics (option -n) must be provided");
	}
	if (options_.outputDir.empty())
	{
		mdm_progAbort("output directory (option -o) must be provided");
	}

	//Using boost filesyetm, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	fs::path outPath = fs::absolute(options_.outputDir);

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	if (!is_directory(outPath))
		create_directories(outPath);

	std::string outputDataFile = outPath.string() + "/" + 
		options_.model + "_" + options_.outputName;

	/*
	 *  Now it's time for the fun ...
	 *
	 *  - Load in the data, can either be dynamic signals or concentrations
	 *	- Fit the models
	 *	- Write the output file
	 *
	 */

	//Set which type of model we're using
	setModel(options_.model, 
		!options_.aifName.empty(), !options_.pifName.empty(),
		options_.paramNames, options_.initParams, 
		options_.fixedParams, options_.fixedValues);
	AIF_.setPrebolus(options_.injectionImage);
	AIF_.setHct(options_.hct);
	AIF_.setDose(options_.dose);

	//If we're using an auto AIF read from file, it will use the times encoded in the file
	//but if we're using a population AIF we need to read these times and set them in the AIF object
	if (options_.aifName.empty())
	{
		//Using population AIF
		if (options_.dynTimesFile.empty())
		{
			mdm_progAbort("if not using an auto-AIF, a dynamic times file must be provided");
		}
		//Try and open the file and read in the times
		std::ifstream dynTimesStream(options_.dynTimesFile, std::ios::in);
		if (!dynTimesStream.is_open())
		{
			mdm_progAbort("error opening dynamic times file, Check it exists");
		}
		std::vector<double> dynamicTimes;
		for (int i = 0; i < options_.nDyns; i++)
		{
			double t;
			dynTimesStream >> t;
			dynamicTimes.push_back(t);
		}
		AIF_.setAIFTimes(dynamicTimes);
	}
	else //If we're using an auto-generated AIF, read it from file now
	{
		std::string aifPath = fs::absolute(options_.aifName).string();
		if (aifPath.empty())
		{
			mdm_progAbort(options_.model + " chosen as model but no AIF filename set");
		}
		if (!AIF_.readAIF(aifPath, options_.nDyns))
		{
			mdm_progAbort("error loading AIF for model " + options_.model);
		}
	}

	//If we're using an auto-generated PIF, read it from file now
	if (!options_.pifName.empty())
	{
		std::string pifPath = fs::absolute(options_.pifName).string();
		if (pifPath.empty())
		{
			mdm_progAbort(options_.model + " chosen as model but no AIF filename set");
		}
		if (!AIF_.readPIF(pifPath, options_.nDyns))
		{
			mdm_progAbort("error loading AIF for model " + options_.model);
		}
	}

	//If we're converting from signal to concentration, make sure we've been supplied TR and FA values
	if (!options_.inputCt && (!options_.TR || !options_.FA || !options_.r1Const))
	{
		mdm_progAbort("TR, FA, r1 must be set to convert from signal concentration");
	}

	//Set-up default IAUC times
	if (options_.IAUCTimes.empty())
		options_.IAUCTimes = { 60.0, 90.0, 120.0 };

	//Open the input data file
	std::ifstream inputData(options_.inputDataFile, std::ios::in);
	if (!inputData.is_open())
	{
		mdm_progAbort("error opening input data file, Check it exists");
	}

	//Open up an output file
	std::ofstream outputData(outputDataFile, std::ios::out);
	if (!outputData.is_open())
	{
		mdm_progAbort("error opening ouput data file");
	}

	//If we've been given a initial parameters for every time-series open stream to file
	//containing these
	std::ifstream inputParams;
	bool load_params = false;
	if (!options_.initParamsFile.empty())
	{
		inputParams.open(options_.initParamsFile, std::ios::in);
		if (!inputParams.is_open())
		{
			mdm_progAbort("error opening input parameter file, Check it exists");
		}
		load_params = true;
	}

	//Check if we've been given a file defining varying dynamic noise
	std::vector<double> noiseVar;
	if (!options_.dynNoiseFile.empty())
	{
		//Try and open the file and read in the noise values
		std::ifstream dynNoiseStream(options_.dynNoiseFile, std::ios::in);
		if (!dynNoiseStream.is_open())
		{
			mdm_progAbort("error opening dynamic times file, Check it exists");
		}
		for (int i = 0; i < options_.nDyns; i++)
		{
			double sigma;
			dynNoiseStream >> sigma;
			noiseVar.push_back(sigma);
		}
	}

	//Set valid first image
	if (options_.lastImage <= 0)
		options_.lastImage = options_.nDyns;

	std::vector<double> ts(options_.nDyns, 0);

	double d;
	double t10 = 0.0;
	double s0 = 0.0;
	int col_counter = 0;
	int row_counter = 0;

	int col_length = options_.nDyns;
	if (!options_.inputCt)
	{
		col_length++;
		if (!options_.useRatio)
			col_length++;
	}

	//Loop through the file, reading in each line
	while (true)
	{
		if (col_counter == col_length)
		{
			//Check if input parameters to read
			if (load_params)
			{
				int n = model_->num_dims();
				std::vector<double> initParams(n, 0);
				double vox;
				for (int i = 0; i < n; i++)
				{
					inputParams >> vox;
					initParams[i] = vox;
				}
				model_->setPkInitParams(initParams);
			}

			//Fit the series
			fit_series(outputData, ts, options_.inputCt,
				noiseVar,
				t10, s0,
				options_.r1Const,
				options_.TR,
				options_.FA,
				options_.firstImage,
				options_.lastImage,
				options_.enhFlag,
				options_.useRatio,
				options_.IAUCTimes,
				options_.outputCm,
				options_.outputCt,
				!options_.noOptimise);

			col_counter = 0;

			row_counter++;
			if (!fmod(row_counter, 1000))
				std::cout << "Processed time-series " << row_counter << std::endl;
		}
		else
		{
			inputData >> d;

			if (col_counter == options_.nDyns)
				t10 = d;

			else if (col_counter == options_.nDyns + 1)
				s0 = d;

			else //col_counter < nDyns()
				ts[col_counter] = d;

			col_counter++;

			if (inputData.eof())
				break;
		}
	}

	//Close the input and output file
	inputData.close();
	outputData.close();
	if (load_params)
		inputParams.close();

	std::cout << "Finished processing! " << std::endl;
	std::cout << "Processed " << row_counter << " time-series in total." << std::endl;

	//Tidy up the logging objects
	return mdm_progExit();
}

//
MDM_API int mdm_RunTools::run_CalculateT1(const std::string &exe_args, const std::string &exeString)
{
  exeString_ = exeString;

  if (options_.T1inputNames.empty())
  {
    mdm_progAbort("input map names (option -maps) must be provided");
  }

  if (options_.outputDir.empty())
  {
    mdm_progAbort("output directory (option -o) must be provided");
  }

  //Set which type of model we're using
  if (!setT1Method(options_.T1method))
    return 1;

  T1Mapper_.setNoiseThreshold(options_.T1noiseThresh);

  //Using boost filesystem, can call one line to make absolute path from input
  //regardless of whether relative or absolute path has been given
  fs::path outputPath = fs::absolute(options_.outputDir);

  //We probably don't need to check if directory exists, just call create... regardless
  //but we do so anyway
  if (!is_directory(outputPath))
    create_directories(outputPath);

  // If we've got this file already warn user of previous analysis and quit
  if (!options_.overwrite && !is_empty(outputPath))
  {
    mdm_progAbort("output directory is not empty (use option O to overwrite existing data)");
  }

  //Set up paths to error image and audit logs, using default names if not user supplied
  // and using boost::filesystem to make absolute paths
  fs::path errorBasePath = outputPath / options_.errorBaseName;
  fs::path mdmLogBasePath = outputPath / options_.programLogBaseName;

	//Note the default audit path doesn't use the output directory (unless user specifically set so)
	fs::path auditPath = fs::absolute(options_.auditLogDir);
	if (!is_directory(auditPath))
		create_directories(auditPath);
	std::string auditName = exeString_ + timeNow() + options_.auditLogBaseName;
	fs::path auditBasePath = auditPath / auditName;

  std::string caller = exeString_ + " " + MDM_VERSION;

  mdm_ProgramLogger::openAuditLog(auditBasePath.string(), caller);
	mdm_ProgramLogger::logAuditMessage(exe_args);
	mdm_ProgramLogger::openProgramLog(mdmLogBasePath.string(), caller);
	mdm_ProgramLogger::logProgramMessage(exe_args);

  //Log path to program log in audit log
	mdm_ProgramLogger::logAuditMessage(
    "Program log saved to " + mdmLogBasePath.string() + "\n");

  /*
  *  Now it's time for the fun ...
  *
  *  1.  Set parameter values
  *  2.  Load files
  *  3.  Do T1 map
  *  4.  Read dyn, AIF and ROI
  *  5.  Fit model (unless user has requested no fit)
  */

	//Before we start, try and load an errorImage, this allows us to add
	//to any existing errors in a re-analysis
	fileManager_.loadErrorImage(errorBasePath.string());

  if (!options_.roiName.empty())
  {
    std::string roiPath = fs::absolute(options_.roiName).string();
    if (!fileManager_.loadROI(roiPath))
    {
      mdm_progAbort("error loading ROI");
    }
  }

  /*Load in all the required images for madym processing. The user has 4 options:
  1) Process everything from scratch:
  - Need paths to variable flip angle images to compute T1 and S0
  - Need path to dynamic images folder and the prefix with which the dynamic images are labelled
  to compute concentration images
  2) Load existing T1 and S0, use baseline S0 to scale signals
  - Need dynamic images
  3) Load existing T1, use ratio method to scale signals
  4) Load existing concentration images
  - Just need folder the concentration images are stored in and the prefix with which they're labelled
  */

  //We need to load FA images
  if (options_.T1inputNames.size() < mdm_T1Voxel::MINIMUM_FAS)
  {
    mdm_progAbort("not enough variable flip angle file names");
  }
  else if (options_.T1inputNames.size() > mdm_T1Voxel::MAXIMUM_FAS)
  {
    mdm_progAbort("too many variable flip angle file names");
  }

  std::vector<std::string> T1inputPaths(0);
  for (std::string mapName : options_.T1inputNames)
    T1inputPaths.push_back(fs::absolute(mapName).string());

  if (!fileManager_.loadFAImages(T1inputPaths))
  {
    mdm_progAbort("error loading FA images");
  }

  //FA images loaded, try computing T1 and S0 maps
  T1Mapper_.T1_mapVarFlipAngle();

  if (!fileManager_.writeOutputMaps(outputPath.string()))
  {
    std::cerr << exeString_ << ": error saving maps" << std::endl;
    //Don't quit here, we may as well try and save the error image anyway
    //nothing else depends on the success of the save
  }

  //Write out the error image
  fileManager_.writeErrorMap(errorBasePath.string());

  //Tidy up the logging objects
	return mdm_progExit();
}

//
MDM_API int mdm_RunTools::run_CalculateT1_lite(const std::string &exe_args, const std::string &exeString,
	const int nSignals)
{
	exeString_ = exeString;

	if (options_.inputDataFile.empty())
	{
		mdm_progAbort("input data file (option -s) must be provided");
	}
	if (!nSignals)
	{
		mdm_progAbort("number of signals (option -n) must be provided");
	}
	if (!options_.TR)
	{
		mdm_progAbort("TR (option -TR) must be provided");
	}
	if (options_.outputDir.empty())
	{
		mdm_progAbort("output directory (option -o) must be provided");
	}

	//Set which type of model we're using - throws error if method not recognised
	setT1Method(options_.T1method);

	//Using boost filesyetm, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	fs::path outPath = fs::absolute(options_.outputDir);

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	if (!fs::is_directory(outPath))
		fs::create_directories(outPath);

	std::string outputDataFile = outPath.string() + "/" + 
		options_.T1method + "_" + options_.outputName;

	//Open the input data (FA and signals) file
	std::ifstream inputData(options_.inputDataFile, std::ios::in);
	if (!inputData.is_open())
	{
		mdm_progAbort("error opening input data file, Check it exists");
	}

	//Open up an output file
	std::ofstream outputData(outputDataFile, std::ios::out);
	if (!outputData.is_open())
	{
		mdm_progAbort("error opening ouput data file");
	}

	double fa;
	int col_counter = 0;
	int row_counter = 0;

	const int &col_length = 2*nSignals;

	std::vector<double> signals(nSignals, 0);
	std::vector<double> FAs(nSignals, 0);
	const auto PI = acos(-1.0);

	//Make T1 calculator
	mdm_T1Voxel T1Calculator;
	T1Calculator.setTR(options_.TR);

	//Loop through the file, reading in each line
	while (true)
	{
		if (col_counter == col_length)
		{
			double T1, S0;
			T1Calculator.setFAs(FAs);
			T1Calculator.setSignals(signals);
			int errCode = T1Calculator.fitT1_VFA(T1, S0);
			col_counter = 0;

			//Now write the output
			outputData <<
				T1 << " " <<
				S0 << " " <<
				errCode << std::endl;

			row_counter++;
			if (!fmod(row_counter, 1000))
				std::cout << "Processed sample " << row_counter << std::endl;
		}

		else
		{
			if (col_counter < nSignals)
			{
				inputData >> fa;
				//Convert to radians
				FAs[col_counter] = fa * PI / 180;
			}
			else
				inputData >> signals[col_counter - nSignals];

			if (inputData.eof())
				break;
			col_counter++;
		}
	}

	//Close the input and output file
	inputData.close();
	outputData.close();

	std::cout << "Finished processing! " << std::endl;
	std::cout << "Processed " << row_counter << " samples in total." << std::endl;

	return mdm_progExit();
}

//
//Fit auto AIF
MDM_API int mdm_RunTools::run_AIFFit(const std::string &exe_args, const std::string &exeString)
{
  exeString_ = exeString;

  if (options_.outputDir.empty())
  {
    mdm_progAbort("output directory (option -o) must be provided");
  }

  if (!options_.T1Name.empty() && options_.T1Name.at(0) == '-')
    mdm_progAbort("Error no value associated with T1 map name from command-line");

  if (!options_.S0Name.empty() && options_.S0Name.at(0) == '-')
    mdm_progAbort("Error no value associated with S0 map name from command-line");

  if (!options_.dynName.empty() && options_.dynName.at(0) == '-')
    mdm_progAbort("Error no value associated with dynamic series file name from command-line");

  //Set parameters in data objects from user input
  AIF_.setPrebolus(options_.injectionImage);
  AIF_.setHct(options_.hct);
  AIF_.setDose(options_.dose);

  volumeAnalysis_.setComputeCt(!options_.inputCt);
  volumeAnalysis_.setRelaxCoeff(options_.r1Const);
  volumeAnalysis_.setUseRatio(options_.useRatio);
  if (options_.firstImage)
    volumeAnalysis_.setFirstImage(options_.firstImage - 1);
  if (options_.lastImage)
    volumeAnalysis_.setLastImage(options_.lastImage);

  T1Mapper_.setNoiseThreshold(options_.T1noiseThresh);

  //Using boost filesyetm, can call one line to make absolute path from input
  //regardless of whether relative or absolute path has been given
  fs::path outputPath = fs::absolute(options_.outputDir);

  //We probably don't need to check if directory exists, just call create... regardless
  //but we do so anyway
  if (!is_directory(outputPath))
    create_directories(outputPath);

  /* If we've got this file already warn user of previous analysis and quit */
  if (!options_.overwrite && !is_empty(outputPath))
  {
    mdm_progAbort("Output directory is not empty (use option O to overwrite existing data)");
  }

  //Set up paths to error image and audit logs, using default names if not user supplied
  // and using boost::filesystem to make absolute paths
  fs::path errorBasePath = outputPath / options_.errorBaseName;

  fs::path mdmLogBasePath = outputPath / options_.programLogBaseName;

	//Note the default audit path doesn't use the output directory (unless user specifically set so)
	fs::path auditPath = fs::absolute(options_.auditLogDir);
	if (!is_directory(auditPath))
		create_directories(auditPath);
	std::string auditName = exeString_ + timeNow() + options_.auditLogBaseName;
	fs::path auditBasePath = auditPath / auditName;

  std::string caller = exeString_ + " " + MDM_VERSION;
  mdm_ProgramLogger::openAuditLog(auditBasePath.string(), caller);
	mdm_ProgramLogger::logAuditMessage(exe_args);
	mdm_ProgramLogger::openProgramLog(mdmLogBasePath.string(), caller);
	mdm_ProgramLogger::logProgramMessage(exe_args);

  //Log program log location in audit log
	mdm_ProgramLogger::logAuditMessage(
    "Program log saved to " + mdmLogBasePath.string() + "\n");

  /*
  *  Now it's time for the fun ...
  *
  *  1.  Set parameter values
  *  2.  Load files
  *  3.  Do T1 map
  *  4.  Read dyn, AIF and ROI
  *  5.  Fit AIF
  */

  /*Load in all the required images for madym processing. The user has 4 options:
  1) Process everything from scratch:
  - Need paths to variable flip angle images to compute T1 and S0
  - Need path to dynamic images folder and the prefix with which the dynamic images are labelled
  to compute concentration images
  2) Load existing T1 and S0, use baseline S0 to scale signals
  - Need dynamic images
  3) Load existing T1, use ratio method to scale signals
  4) Load existing concentration images
  - Just need folder the concentration images are stored in and the prefix with which they're labelled
  */

	//Before we start, try and load an errorImage, this allows us to add
	//to any existing errors in a re-analysis
	fileManager_.loadErrorImage(errorBasePath.string());

  //Check for case 4: load pre-computed concentration maps
  if (options_.inputCt)
  {
    fs::path catPath = fs::absolute(options_.dynDir) / options_.dynName;
    std::string catPrefix = catPath.filename().string();
    std::string catBasePath = catPath.remove_filename().string();
    if (!catBasePath.empty() && !catPrefix.empty())
    {
      if (!fileManager_.loadCtDataMaps(catBasePath, catPrefix))
      {
        mdm_progAbort("error loading catMaps");
      }
    }
    else
    {
      mdm_progAbort("caMapFlag set to true, but paths and/or prefix to cat maps not set");
    }
  }
  else
  {
    fs::path dynPath = fs::absolute(options_.dynName);
    std::string dynPrefix = dynPath.filename().string();
    std::string dynBasePath = dynPath.remove_filename().string();

    //If not case 4 we *must* have dynamic images
    if (dynBasePath.empty() && dynPrefix.empty())
    {
      mdm_progAbort("paths and/or prefix to dynamic images not set");
    }

    //Load the dynamic image
    if (!fileManager_.loadStDataMaps(dynBasePath, dynPrefix))
    {
      mdm_progAbort("error loading dynamic images");
    }

    //Now check if we've suuplied an existing T1 map
    if (!options_.T1Name.empty())
    {
      fs::path T1Path = fs::absolute(options_.T1Name);
      if (!fileManager_.loadT1Image(T1Path.string()))
      {
        mdm_progAbort("error loading T1 map");
      }

      //Now check for cases 2 and 3, if useBaselineS0 is true
      //we need both S0 and T1, otherwise we can just use T1
      if (!options_.useRatio)
      {

        if (options_.S0Name.empty())
        {
          mdm_progAbort("S0MapFlag set to true, but path to S0 not set");
        }
        //Otherwise load S0 and return
        fs::path S0Path = fs::absolute(options_.S0Name);
        if (!fileManager_.loadS0Image(S0Path.string()))
        {
          mdm_progAbort("error loading S0 map");
        }
      }
    }
    else
    {
      //We need to load FA images
      if (options_.T1inputNames.size() < mdm_T1Voxel::MINIMUM_FAS)
      {
        mdm_progAbort("Not enough variable flip angle file names");
      }
      else if (options_.T1inputNames.size() > mdm_T1Voxel::MAXIMUM_FAS)
      {
        mdm_progAbort("Too many variable flip angle file names");
      }

      std::vector<std::string> T1inputPaths(0);
      for (std::string fa : options_.T1inputNames)
        T1inputPaths.push_back(fs::absolute(fa).string());

      if (!fileManager_.loadFAImages(T1inputPaths))
      {
        mdm_progAbort("error loading input images for baseline T1 calculation");
      }

      //FA images loaded, try computing T1 and S0 maps
      T1Mapper_.T1_mapVarFlipAngle();
    }
  }

  /*If we're here, then we should have either:
  1) A set of concentration images in volumeAnalysis OR
  2) A set of dynamic images and a T1 map and either an S0 map or the use baseline S0 flag set to false*/
  const std::vector<mdm_Image3D> &maps = options_.inputCt ?
    volumeAnalysis_.CtDataMaps() : volumeAnalysis_.StDataMaps();
  AIF_.computeAutoAIF(maps, T1Mapper_.T1Map(), options_.aifSlice, options_.r1Const, options_.inputCt);


  /*Call the file manager to write out the AIF*/
  std::string AIFname = "slice_" + std::to_string(options_.aifSlice) + "_Auto_AIF.txt";
  fs::path AIFpath = outputPath / AIFname;

  if (!fileManager_.saveAIF(AIFpath.string()))
  {
    std::cerr << exeString_ << ": error saving AIF\n";
  }

	//Write out the error image
	fileManager_.writeErrorMap(errorBasePath.string());

  //Tidy up the logging objects
  return mdm_progExit();
}

//

//-----------------------------------------------------------
//-----------------------------------------------------------
// Private methods:
//-----------------------------------------------------------
int mdm_RunTools::mdm_progExit()
{
	std::string success_msg = exeString_ + " completed successfully.\n";
	mdm_ProgramLogger::logProgramMessage(success_msg);
	mdm_ProgramLogger::logAuditMessage(success_msg);
	mdm_ProgramLogger::closeAuditLog();
	mdm_ProgramLogger::closeProgramLog();
	return 0;
}

void mdm_RunTools::mdm_progAbort(const std::string &err_str)
{

  std::string error_msg = exeString_ + " ABORTING: " + err_str + "\n";

	mdm_ProgramLogger::logProgramMessage(error_msg);
	mdm_ProgramLogger::logAuditMessage(error_msg);
	mdm_ProgramLogger::closeAuditLog();
	mdm_ProgramLogger::closeProgramLog();
  exit(1);
}

std::string mdm_RunTools::timeNow()
{
	
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "_%Y%m%d_%H%M%S_");
	return ss.str();
}

/**
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Fit selected dynamic contrast agent concentration model
* @version  madym 1.22
*/
void mdm_RunTools::setModel(const std::string &model_name, bool auto_aif, bool auto_pif,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initParams,
  const std::vector<int> fixedParams,
  const std::vector<double> fixedValues)
{
  if (model_name == "T1_ONLY")
  {
    AIF_.setAIFflag(mdm_AIF::AIF_INVALID);
  }
  else
  {
    bool model_set = mdm_DCEModelGenerator::setModel(model_, AIF_,
      model_name, auto_aif, auto_pif, paramNames,
      initParams, fixedParams, fixedValues);

    if (!model_set)
      mdm_progAbort("Invalid or unsupported model (from command-line)");

    volumeAnalysis_.setModel(model_);
  }
}

bool mdm_RunTools::setT1Method(const std::string &method)
{
  //TODO currently only variable flip-angle method implemented
  //when other options implemented, set method for choosing between them in T1 map
  if (method == "VFA")
  {
    std::cout << "Using variable flip angle method" << std::endl;
  }
  else if (method == "IR")
  {
    std::cout << "Using inversion recovery method" << std::endl;
  }
  else
  {
    mdm_progAbort("method " + method + " not recognised");
  }
  return true;
}

void mdm_RunTools::fit_series(std::ostream &outputData, 
	const std::vector<double> &ts, const bool &inputCt,
	const std::vector<double> &noiseVar,
	const double &t10, const double &s0,
	const double &r1,
	const double &TR,
	const double & FA,
	const int &firstImage,
	const int &lastImage,
	const bool&enhFlag,
	const bool&useRatio,
	const std::vector<double> &IAUCTimes,
	const bool &outputCm,
	const bool &outputCt,
	const bool &optimiseModel)
{
	std::vector<double> signalData;
	std::vector<double> CtData;

	if (inputCt)
		CtData = ts;
	else
		signalData = ts;

	const int nDyns = ts.size();

	//Create a perm object
	mdm_DCEVoxel vox(
		signalData,
		CtData,
		noiseVar,
		t10,
		s0,
		r1,
		model_->AIF().prebolus(),
		AIF_.AIFTimes(),
		TR,
		FA,
		firstImage,
		lastImage,
		enhFlag,
		useRatio,
		IAUCTimes);
	vox.initialiseModelFit(*model_);

	vox.IAUC_calc();

	if (optimiseModel)
		vox.fitModel();

	//Now write the output
	outputData <<
		vox.status() << " " <<
		vox.enhancing() << " " <<
		vox.modelFitError() << " ";

	for (int i = 0; i < IAUCTimes.size(); i++)
		outputData << " " << vox.IAUC_val(i);

	for (int i = 0; i < model_->num_dims(); i++)
		outputData << " " << model_->pkParams(i);


	if (outputCm)
	{
		const std::vector<double> &Cm_t = vox.CtModel();
		for (int i = 0; i < nDyns; i++)
			outputData << " " << Cm_t[i];
	}

	if (outputCt)
	{
		const std::vector<double> &Cs_t = vox.CtData();
		for (int i = 0; i < nDyns; i++)
			outputData << " " << Cs_t[i];
	}

	outputData << std::endl;
}