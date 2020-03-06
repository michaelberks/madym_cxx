#include <testlib/testlib_test.h>
#include "../../tests/mdm_test_utils.h"
#include <fstream>
#include <madym/mdm_T1Voxel.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

void run_test_calculate_T1_lite()
{

	std::cout << "======= Testing tool: calculate T1 =======" << std::endl;

	//Generate some signals from sample FA, TR, T1 and S0 values
	double T1 = 1000;
	double S0 = 2000;
	double TR = 3.5;
	const auto PI = acos(-1.0);
	std::vector<double>	FAs = { 2 , 10, 18 };
	
	std::string test_dir = mdm_test_utils::temp_dir();
	std::string inputDataFile = test_dir + "/T1_input.dat";
	std::ofstream ifs(inputDataFile, std::ios::out);

	if (!ifs.is_open())
	{
		TEST("Failed to write out test values for T1", 0, 1);
		return;
	}

	//Write out FAs
	for (const auto FA : FAs)
		ifs << FA << " ";

	//Compute signal for each FA and write out value
	for (int i_fa = 0; i_fa < 3; i_fa++)
		ifs << mdm_T1Voxel::T1toSignal(T1, S0, PI*FAs[i_fa]/180, TR) << " ";
	ifs.close();

	//Call calculate_T1 to fit T1 and S0
	std::string T1_output_dir = test_dir + "/calculate_T1_lite/";
	std::string outputName = "madym_analysis.dat";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "calculate_T1_lite"
		<< " -m VFA "
		<< " -d " << inputDataFile
		<< " -n 3 "
		<< " -TR " << TR
		<< " -o " << T1_output_dir
		<< " -O " << outputName;
		;

	std::cout << "Command to run: " << cmd.str() << std::endl;

	int error; 
	try
	{
		error = std::system(cmd.str().c_str());
	}	
	catch (...)
	{
		TEST("Running calculate_T1_lite failed", 0, 1);
		return;
	}

	TEST("calculate_T1_lite tool ran without error", error, 0);

	//Load in the fitted parameters from the output file
	std::string outputDataFile = T1_output_dir + "VFA_" + outputName;
	std::ifstream ofs(outputDataFile, std::ios::in);
	if (!ofs.is_open())
	{
		TEST("Failed to read in fitted values for T1", 0, 1);
		return;
	}
	double T1_fit, S0_fit;
	int fit_errors;
	ofs >> T1_fit;
	ofs >> S0_fit;
	ofs >> fit_errors;
	ofs.close();

	//Check the model parameters have fitted correctly
	double tol = 0.1;
	TEST_NEAR("Fitted T1", T1_fit, T1, tol);
	TEST_NEAR("Fitted S0", S0_fit, S0, tol);
	TEST("Error codes zero", fit_errors, 0);

	//Tidy up
	fs::remove(inputDataFile);
	fs::remove_all(T1_output_dir);
}

void test_calculate_T1_lite()
{
	run_test_calculate_T1_lite();
}

TESTMAIN(test_calculate_T1_lite);
