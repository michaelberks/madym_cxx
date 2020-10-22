#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_calculateT1_lite.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_T1Voxel.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_calculateT1_lite::mdm_RunTools_calculateT1_lite(mdm_InputOptions &options, mdm_OptionsParser &options_parser)
	: mdm_RunTools(options, options_parser)
{
}


MDM_API mdm_RunTools_calculateT1_lite::~mdm_RunTools_calculateT1_lite()
{

}

//
MDM_API int mdm_RunTools_calculateT1_lite::run()
{
	if (options_.inputDataFile().empty())
	{
		mdm_progAbort("input data file (option -s) must be provided");
	}
	if (!options_.nT1Inputs())
	{
		mdm_progAbort("number of signals (option -n) must be provided");
	}
	if (!options_.TR())
	{
		mdm_progAbort("TR (option -TR) must be provided");
	}
	if (options_.outputDir().empty())
	{
		mdm_progAbort("output directory (option -o) must be provided");
	}

	//Set which type of model we're using - throws error if method not recognised
	setT1Method(options_.T1method());

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

	double fa;
	int col_counter = 0;
	int row_counter = 0;

	const int &nSignals = options_.nT1Inputs();
	const int &col_length = 2 * nSignals;

	std::vector<double> signals(nSignals, 0);
	std::vector<double> FAs(nSignals, 0);
	const auto PI = acos(-1.0);

	//Make T1 calculator
	mdm_T1Voxel T1Calculator;
	T1Calculator.setTR(options_.TR());

	//Loop through the file, reading in each line
	while (true)
	{
		if (col_counter == col_length)
		{
			double T1, M0;
			T1Calculator.setFAs(FAs);
			T1Calculator.setSignals(signals);
			int errCode = T1Calculator.fitT1_VFA(T1, M0);
			col_counter = 0;

			//Now write the output
			outputData <<
				T1 << " " <<
				M0 << " " <<
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

//*******************************************************************************
// Private:
//*******************************************************************************