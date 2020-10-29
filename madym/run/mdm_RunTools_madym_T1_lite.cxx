#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_T1_lite.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_T1MethodGenerator.h>

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
	if (options_.inputDataFile().empty())
	{
		mdm_progAbort("input data file (option --data) must be provided");
	}
	if (!options_.nT1Inputs())
	{
		mdm_progAbort("number of signals (option --n_T1) must be provided");
	}
	if (!options_.TR())
	{
		mdm_progAbort("TR (option --TR) must be provided");
	}

	//Set which type of model we're using - throws error if method not recognised
	if (!setT1Method(options_.T1method()))
		mdm_progAbort("T1 method not recognised");

	//Using boost filesyetm, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	fs::path outPath = fs::absolute(options_.outputDir());

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	if (!fs::is_directory(outPath))
		fs::create_directories(outPath);

	std::string outputDataFile = outPath.string() + "/" +
		options_.T1method() + "_" + options_.outputName();

	//Open the input data (FA and signals) file
	std::ifstream inputData(options_.inputDataFile(), std::ios::in);
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

	int row_counter = 0;
	const int &nSignals = options_.nT1Inputs();

	//Instantiate T1 fitter of desired type
	auto methodType = mdm_T1MethodGenerator::ParseMethodName(
		options_.T1method());
	if (methodType == mdm_T1MethodGenerator::UNDEFINED)
		mdm_progAbort("T1 method not recognised");

	auto T1Fitter = mdm_T1MethodGenerator::createFitter(methodType, options_);

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

	return options_parser_.parse_inputs(
		config_options,
		argc, argv);
}
//*******************************************************************************
// Private:
//*******************************************************************************
