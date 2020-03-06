#include <testlib/testlib_test.h>
#include "../../tests/mdm_test_utils.h"
#include <fstream>
#include <madym/mdm_T1Voxel.h>
#include <mdm_version.h>
#include <madym/mdm_AIF.h>
#include <madym/mdm_DCEModelGenerator.h>

namespace fs = boost::filesystem;

void run_test_madym_lite()
{
	std::cout << "======= Testing tool: madym =======" << std::endl;
	//Need to generate input data files. To do this, load in calibration
	//data

	//Read in dyn times
	int nTimes;
	std::string timesFileName(mdm_test_utils::calibration_dir() + "dyn_times.dat");
	std::ifstream timesFileStream(timesFileName, std::ios::in | std::ios::binary);
	timesFileStream.read(reinterpret_cast<char*>(&nTimes), sizeof(int));
	std::vector<double> dynTimes(nTimes);
	for (double &t : dynTimes)
		timesFileStream.read(reinterpret_cast<char*>(&t), sizeof(double));
	timesFileStream.close();

	//Read in AIF parameters
	int injectionImage;
	double hct;
	double dose;

	std::string aifFileName(mdm_test_utils::calibration_dir() + "aif.dat");
	std::ifstream aifFileStream(aifFileName, std::ios::in | std::ios::binary);
	aifFileStream.read(reinterpret_cast<char*>(&injectionImage), sizeof(int));
	aifFileStream.read(reinterpret_cast<char*>(&hct), sizeof(double));
	aifFileStream.read(reinterpret_cast<char*>(&dose), sizeof(double));
	aifFileStream.close();

	//Read (noisy) ETM times series from calibration data
	int nParams;
	std::vector<double> Ct(nTimes);
	std::string modelFileName = mdm_test_utils::calibration_dir() + "ETM_noise.dat";

	std::ifstream modelFileStream(modelFileName, std::ios::in | std::ios::binary);
	modelFileStream.read(reinterpret_cast<char*>(&nParams), sizeof(int));

	std::vector<double> trueParams(nParams);
	for (double &p : trueParams)
		modelFileStream.read(reinterpret_cast<char*>(&p), sizeof(double));
	for (double &c : Ct)
		modelFileStream.read(reinterpret_cast<char*>(&c), sizeof(double));
	modelFileStream.close();

	//Read IAUC values
	int nIAUC;
	std::string iaucFileName = mdm_test_utils::calibration_dir() + "ETM_IAUC.dat";
	std::ifstream iaucFileStream(iaucFileName, std::ios::in | std::ios::binary);
	iaucFileStream.read(reinterpret_cast<char*>(&nIAUC), sizeof(int));
	std::vector<double> IAUCTimes(nIAUC);
	std::vector<double> IAUCVals(nIAUC);
	for (double &it : IAUCTimes)
		iaucFileStream.read(reinterpret_cast<char*>(&it), sizeof(double));

	for (double &iv : IAUCVals)
		iaucFileStream.read(reinterpret_cast<char*>(&iv), sizeof(double));
	iaucFileStream.close();

	std::string IAUC_str = std::to_string((int)IAUCTimes[0]);
	for (int i = 1; i < nIAUC; i++)
		IAUC_str += ("," + std::to_string((int)IAUCTimes[i]));

	//Create a temporary directory where we'll run these tests, which we can then cleanup
	//easily at the end
	std::string test_dir = mdm_test_utils::temp_dir();

	//Write out the concentration times
	std::string inputDataFile = test_dir + "/Ct_input.dat";
	std::ofstream ifs(inputDataFile, std::ios::out);
	if (!ifs.is_open())
	{
		TEST("Failed to write out Ct values for madym_lite", 0, 1);
		return;
	}
	for (const auto c : Ct)
		ifs << c << " ";
	ifs.close();

	//Writeout the dynamic time
	std::string dynTimesFile = test_dir + "/dyn_times.dat";
	std::ofstream dfs(dynTimesFile, std::ios::out);
	if (!dfs.is_open())
	{
		TEST("Failed to write out dyn times for madym_lite", 0, 1);
		return;
	}
	for (const auto t : dynTimes)
		dfs << t << " ";
	dfs.close();

	//Call madym_lite to fit ETM
	std::string Ct_output_dir = test_dir + "/madym_lite/";
	std::string outputName = "madym_analysis.dat";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_lite"
		<< " -m ETM"
		<< " -d " << inputDataFile
		<< " -n " << nTimes
		<< " -iauc " << IAUC_str
		<< " -i " << injectionImage
		<< " -o " << Ct_output_dir
		<< " -O " << outputName
		<< " -Cin"
		<< " -t " << dynTimesFile;

	std::cout << "Command to run: " << cmd.str() << std::endl;

	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		TEST("Running madym_lite failed", 0, 1);
		return;
	}

	TEST("madym_lite tool ran without error", error, 0);

	//Load in the fitted parameters from the output file
	std::string outputDataFile = Ct_output_dir + "ETM_" + outputName;
	std::ifstream ofs(outputDataFile, std::ios::in);
	if (!ofs.is_open())
	{
		TEST("Failed to read in fitted values for ETM", 0, 1);
		return;
	}
	int fit_errors, enhancing;
	double model_fit, ktrans_fit, ve_fit, vp_fit, tau_fit;
	std::vector<double> IAUC_fit(nIAUC);
	ofs >> fit_errors;
	ofs >> enhancing;
	ofs >> model_fit;
	for (auto &i : IAUC_fit)
		ofs >> i;
	ofs >> ktrans_fit;
	ofs >> ve_fit;
	ofs >> vp_fit;
	ofs >> tau_fit;
	ofs.close();

	//Check the model parameters have fitted correctly
	double tol = 0.01;
	TEST_NEAR("Fitted ktrans", ktrans_fit, trueParams[0], tol);
	TEST_NEAR("Fitted Ve", ve_fit, trueParams[1], tol);
	TEST_NEAR("Fitted Vp", vp_fit, trueParams[2], tol);
	TEST_NEAR("Fitted tau", tau_fit, trueParams[3], tol);
	TEST_NEAR("Model", model_fit, 0, tol);
	TEST("Error codes zero", fit_errors, 0);
	TEST("Enhancing", enhancing, 1);
	for (int i = 0; i < nIAUC; i++)
		TEST_NEAR(("IAUC " + std::to_string(IAUCTimes[i])).c_str(), IAUC_fit[i], IAUCVals[i], tol);

	//Tidy up
	fs::remove(inputDataFile);
	fs::remove(dynTimesFile);
	fs::remove_all(Ct_output_dir);
}

void test_madym_lite()
{
	run_test_madym_lite();
}

TESTMAIN(test_madym_lite);
