/**
*  @file    mdm_RunTools_madym_DWI_lite.cxx
*  @brief   Implementation of mdm_RunTools_madym_DWI_lite class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_DWI_lite.h"

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/dwi/mdm_DWImodelGenerator.h>
#include <madym/utils/mdm_exception.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_DWI_lite::mdm_RunTools_madym_DWI_lite()
{
}


MDM_API mdm_RunTools_madym_DWI_lite::~mdm_RunTools_madym_DWI_lite()
{
}

//
MDM_API void mdm_RunTools_madym_DWI_lite::run()
{
	//Check required fields are set
	if (options_.inputDataFile().empty())
    throw mdm_exception(__func__, "input data file (option --data) must be provided");
	
	const int &nSignals = options_.nDWIInputs();
	if (!nSignals)
    throw mdm_exception(__func__, "number of signals (option --n_DWI) must be provided");

  //Set curent working dir
  set_up_cwd();

	//Parse DWI model from string, will abort if model type not recognised
	auto modelType = mdm_DWImodelGenerator::parseModelName(options_.DWImodel());

	//Instantiate DWI fitter of desired type
	auto DWIFitter = mdm_DWImodelGenerator::createFitter(modelType, options_.BvalsThresh());

	//Check number of inputs is valid
	if (nSignals < DWIFitter->minimumInputs())
		throw mdm_exception(__func__, "not enough signal inputs for DWI model " + options_.DWImodel());

	else if (nSignals > DWIFitter->maximumInputs())
		throw mdm_exception(__func__, "too many signal inputs for DWI model " + options_.DWImodel());

	//Set up output path and output file
	set_up_output_folder();

	std::string outputDataFile = outputPath_.string() + "/" +
		options_.DWImodel() + "_" + options_.outputName();

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
		if (!DWIFitter->setInputsFromStream(inputData, nSignals))
			break;

		//If valid inputs, fit DWI model and write to output stream
		std::vector<double> params;
		double ssr;
		int errCode = DWIFitter->fitModel(params, ssr);

		for (auto p : params)
			outputData << p << " ";
		outputData << ssr << " " << errCode << std::endl;

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
MDM_API int mdm_RunTools_madym_DWI_lite::parseInputs(int argc, const char *argv[])
{
	po::options_description config_options("madym_DWI_lite config options_");
	
	options_parser_.add_option(config_options, options_.dataDir);
	options_parser_.add_option(config_options, options_.inputDataFile);
	options_parser_.add_option(config_options, options_.DWImodel);
	options_parser_.add_option(config_options, options_.nDWIInputs);
	options_parser_.add_option(config_options, options_.BvalsThresh);
	
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.outputName);
  options_parser_.add_option(config_options, options_.quiet);

	//Always set overwrite true for lite tools
	options_.overwrite.set(true);

	return options_parser_.parseInputs(
		config_options,
		argc, argv);
}

MDM_API std::string mdm_RunTools_madym_DWI_lite::who() const
{
	return "madym_DWI_lite";
}
//*******************************************************************************
// Private:
//*******************************************************************************
