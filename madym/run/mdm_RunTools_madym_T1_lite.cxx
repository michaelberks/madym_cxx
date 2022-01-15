/**
*  @file    mdm_RunTools_madym_T1_lite.cxx
*  @brief   Implementation of mdm_RunTools_madym_T1_lite class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_T1_lite.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/t1/mdm_T1MethodGenerator.h>
#include <madym/mdm_exception.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_T1_lite::mdm_RunTools_madym_T1_lite()
{
}


MDM_API mdm_RunTools_madym_T1_lite::~mdm_RunTools_madym_T1_lite()
{
}

//
MDM_API void mdm_RunTools_madym_T1_lite::run()
{
	//Check required fields are set
	if (options_.inputDataFile().empty())
    throw mdm_exception(__func__, "input data file (option --data) must be provided");
	
	const int &nSignals = options_.nT1Inputs();
	if (!nSignals)
    throw mdm_exception(__func__, "number of signals (option --n_T1) must be provided");
	
	if (!options_.TR())
    throw mdm_exception(__func__, "TR (option --TR) must be provided");

  //Set curent working dir
  set_up_cwd();

	//Parse T1 method from string, will abort if method type not recognised
	auto methodType = mdm_T1MethodGenerator::parseMethodName(options_.T1method(), options_.B1Correction());

	//Instantiate T1 fitter of desired type
	auto T1Fitter = mdm_T1MethodGenerator::createFitter(methodType, options_);

	//Check number of inputs is valid
	checkNumInputs(methodType, nSignals);

	//Set up output path and output file
	set_up_output_folder();

	std::string outputDataFile = outputPath_.string() + "/" +
		options_.T1method() + "_" + options_.outputName();

	//Open the input data (FA and signals) file
	std::ifstream inputData(options_.inputDataFile(), std::ios::in);
	if (!inputData.is_open())
    throw mdm_exception(__func__, "Error opening input data file, Check it exists");
	
	//Open up an output file
	std::ofstream outputData(outputDataFile, std::ios::out);
	if (!outputData.is_open())
    throw mdm_exception(__func__, "Error opening ouput data file");

	int row_counter = 0;
	
	//Loop through the file, reading in each line
	while (!inputData.eof())
	{
		//Get fitter to munch line of inputs from datastream, if EOF reached break
		if (!T1Fitter->setInputsFromStream(inputData, nSignals))
			break;

		//If valid inputs, fit T1 and write to output stream
		double T1, M0;
		int errCode = T1Fitter->fitT1(T1, M0);

		outputData <<
			T1 << " " <<
			M0 << " " <<
			errCode << std::endl;

		row_counter++;
		if (!options_.quiet() && !fmod(row_counter, 1000))
			std::cout << "Processed sample " << row_counter << std::endl;

	}

	//Close the input and output file
	inputData.close();
	outputData.close();

  if (!options_.quiet())
  {
    std::cout << "Finished processing! " << std::endl;
    std::cout << "Processed " << row_counter << " samples in total." << std::endl;
  }
}

//
MDM_API int mdm_RunTools_madym_T1_lite::parseInputs(int argc, const char *argv[])
{
	po::options_description config_options("calculate_T1_lite config options_");
	
	options_parser_.add_option(config_options, options_.dataDir);
	options_parser_.add_option(config_options, options_.inputDataFile);
	options_parser_.add_option(config_options, options_.T1method);
	options_parser_.add_option(config_options, options_.FA);
	options_parser_.add_option(config_options, options_.TR);
  options_parser_.add_option(config_options, options_.B1Correction);
	options_parser_.add_option(config_options, options_.T1noiseThresh);
	options_parser_.add_option(config_options, options_.nT1Inputs);
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.outputName);
  options_parser_.add_option(config_options, options_.quiet);

	//Always set overwrite true for lite methods
	options_.overwrite.set(true);

	return options_parser_.parseInputs(
		config_options,
		argc, argv);
}

MDM_API std::string mdm_RunTools_madym_T1_lite::who() const
{
	return "madym_T1_lite";
}
//*******************************************************************************
// Private:
//*******************************************************************************
