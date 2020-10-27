#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>

#include <fstream>
#include <madym/mdm_T1Voxel.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_calculate_T1_lite) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym T1 lite =======");

	//Generate some signals from sample FA, TR, T1 and M0 values
	double T1 = 1000;
	double M0 = 2000;
	double TR = 3.5;
	const auto PI = acos(-1.0);
	std::vector<double>	FAs = { 2 , 10, 18 };
	
	std::string test_dir = mdm_test_utils::temp_dir();
	std::string inputDataFile = test_dir + "/T1_input.dat";
	std::ofstream ifs(inputDataFile, std::ios::out);

	BOOST_REQUIRE_MESSAGE(ifs.is_open(), "Failed to write out test values for T1");

	//Write out FAs
	for (const auto FA : FAs)
		ifs << FA << " ";

	//Compute signal for each FA and write out value
	for (int i_fa = 0; i_fa < 3; i_fa++)
		ifs << mdm_T1Voxel::T1toSignal(T1, M0, PI*FAs[i_fa]/180, TR) << " ";
	ifs.close();

	//Call calculate_T1 to fit T1 and M0
	std::string T1_output_dir = test_dir + "/madym_T1_lite/";
	std::string outputName = "madym_analysis.dat";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_T1_lite"
		<< " -T VFA "
		<< " --data " << inputDataFile
		<< " --n_T1 " << 3
		<< " --tr " << TR
		<< " -o " << T1_output_dir
		<< " -O " << outputName;
		;

	BOOST_TEST_MESSAGE("Command to run: " + cmd.str() );

	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		BOOST_CHECK_MESSAGE(false, "Running madym_T1_lite failed");
		return;
	}

	BOOST_CHECK_MESSAGE(!error, "Error returned from madym_T1_lite tool");

	//Load in the fitted parameters from the output file
	std::string outputDataFile = T1_output_dir + "VFA_" + outputName;
	std::ifstream ofs(outputDataFile, std::ios::in);
	BOOST_REQUIRE_MESSAGE(ofs.is_open(), "Failed to read in fitted values for T1");
		
	double T1_fit, M0_fit;
	int fit_errors;
	ofs >> T1_fit;
	ofs >> M0_fit;
	ofs >> fit_errors;
	ofs.close();

	//Check the model parameters have fitted correctly
	double tol = 0.1;
	BOOST_TEST_MESSAGE("Testing fitted T1");
	BOOST_CHECK_CLOSE(T1_fit, T1, tol);
	BOOST_TEST_MESSAGE("Testing fitted M0");
	BOOST_CHECK_CLOSE(M0_fit, M0, tol);
	BOOST_TEST_MESSAGE("Checking zero error-codes");
	BOOST_CHECK(!fit_errors);

	//Tidy up
	fs::remove(inputDataFile);
	fs::remove_all(T1_output_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
