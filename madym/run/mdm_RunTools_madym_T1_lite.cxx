#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_T1_lite.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/t1_methods/mdm_T1MethodGenerator.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_T1_lite::mdm_RunTools_madym_T1_lite(mdm_InputOptions &options_, mdm_OptionsParser &options_parser)
	: mdm_RunTools(options_, options_parser)
{
}


MDM_API mdm_RunTools_madym_T1_lite::~mdm_RunTools_madym_T1_lite()
{
}

//
MDM_API int mdm_RunTools_madym_T1_lite::run()
{
	//Check required fields are set
	if (options_.inputDataFile().empty())
		mdm_progAbort("input data file (option --data) must be provided");
	
	const int &nSignals = options_.nT1Inputs();
	if (!nSignals)
		mdm_progAbort("number of signals (option --n_T1) must be provided");
	
	if (!options_.TR())
		mdm_progAbort("TR (option --TR) must be provided");

	//Parse T1 method from string, will abort if method type not recognised
	auto methodType = parseMethod(options_.T1method());

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
		mdm_progAbort("error opening input data file, Check it exists");
	
	//Open up an output file
	std::ofstream outputData(outputDataFile, std::ios::out);
	if (!outputData.is_open())
		mdm_progAbort("error opening ouput data file");

	int row_counter = 0;
	
	//Loop through the file, reading in each line
	while (!inputData.eof())
	{
		bool eof;
		double T1, M0;
		int errCode = T1Fitter->fitT1(inputData, nSignals, T1, M0, eof);

		if (eof)
			break;

		//Now write the output
		outputData <<
			T1 << " " <<
			M0 << " " <<
			errCode << std::endl;

		row_counter++;
		if (!fmod(row_counter, 1000))
			std::cout << "Processed sample " << row_counter << std::endl;

	}

	//Close the input and output file
	inputData.close();
	outputData.close();

	std::cout << "Finished processing! " << std::endl;
	std::cout << "Processed " << row_counter << " samples in total." << std::endl;
	
	return mdm_progExit();
}

/**
	* @brief

	* @param
	* @return
	*/
MDM_API int mdm_RunTools_madym_T1_lite::parse_inputs(int argc, const char *argv[])
{
	po::options_description config_options("calculate_T1_lite config options_");
	
	options_parser_.add_option(config_options, options_.dataDir);
	options_parser_.add_option(config_options, options_.inputDataFile);
	options_parser_.add_option(config_options, options_.T1method);
	options_parser_.add_option(config_options, options_.FA);
	options_parser_.add_option(config_options, options_.TR);
	options_parser_.add_option(config_options, options_.T1noiseThresh);
	options_parser_.add_option(config_options, options_.nT1Inputs);
	options_parser_.add_option(config_options, options_.outputDir);
	options_parser_.add_option(config_options, options_.outputName);

	//Always set overwrite true for lite methods
	options_.overwrite.set(true);

	return options_parser_.parse_inputs(
		config_options,
		argc, argv);
}
//*******************************************************************************
// Private:
//*******************************************************************************
