#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <fstream>

#include <madym/tests/mdm_test_utils.h>
#include <madym/t1_methods/mdm_T1FitterBase.h>
#include <mdm_version.h>
#include <madym/mdm_AIF.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_lite) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym_DCE_lite =======");
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
	BOOST_REQUIRE_MESSAGE(ifs.is_open(), "Failed to write out Ct values for madym_DCE_lite");

	for (const auto c : Ct)
		ifs << c << " ";
	ifs.close();

	//Writeout the dynamic time
	std::string dynTimesFile = test_dir + "/dyn_times.dat";
	std::ofstream dfs(dynTimesFile, std::ios::out);
	BOOST_REQUIRE_MESSAGE(dfs.is_open(), "Failed to write out dyn times values for madym_DCE_lite");
	
	for (const auto t : dynTimes)
		dfs << t << " ";
	dfs.close();

	//Call madym_lite to fit ETM
	std::string Ct_output_dir = test_dir + "/madym_DCE_lite/";
	std::string outputName = "madym_analysis.dat";
	std::stringstream cmd;
	cmd << mdm_test_utils::tools_exe_dir() << "madym_DCE_lite"
		<< " -m ETM"
		<< " --data " << inputDataFile
		<< " -n " << nTimes
		<< " -I " << IAUC_str
		<< " -i " << injectionImage
		<< " -o " << Ct_output_dir
		<< " -O " << outputName
		<< " --Ct"
		<< " -t " << dynTimesFile;

	BOOST_TEST_MESSAGE("Command to run: " + cmd.str());

	int error;
	try
	{
		error = std::system(cmd.str().c_str());
	}
	catch (...)
	{
		BOOST_CHECK_MESSAGE(false, "Running madym_DCE_lite failed");
		return;
	}

	BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DCE_lite tool");

	//Load in the fitted parameters from the output file
	std::string outputDataFile = Ct_output_dir + "ETM_" + outputName;
	std::ifstream ofs(outputDataFile, std::ios::in);
	BOOST_REQUIRE_MESSAGE(ofs.is_open(), "Failed to read in fitted values for ETM");
	
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
	double tol = 0.1;
	BOOST_TEST_MESSAGE(boost::format("Fitted ktrans (%1.2f, %2.2f)")
		% ktrans_fit % trueParams[0]);
	BOOST_CHECK_CLOSE(ktrans_fit, trueParams[0], tol);
	BOOST_TEST_MESSAGE(boost::format("Fitted Ve (%1.2f, %2.2f)")
		% ve_fit % trueParams[1]);
	BOOST_CHECK_CLOSE(ve_fit, trueParams[1], tol);
	BOOST_TEST_MESSAGE(boost::format("Fitted Vp (%1.2f, %2.2f)")
		% vp_fit % trueParams[2]);
	BOOST_CHECK_CLOSE(vp_fit, trueParams[2], tol);
	BOOST_TEST_MESSAGE(boost::format("Fitted tau (%1.2f, %2.2f)")
		% tau_fit % trueParams[3]);
	BOOST_CHECK_CLOSE(tau_fit, trueParams[3], tol);

	//Check model fit, error codes and enhancing
	BOOST_TEST_MESSAGE(boost::format("Model residuals = %1%") 
		% model_fit);
	BOOST_CHECK_SMALL(model_fit, 0.01);
	BOOST_TEST_MESSAGE("No error code");
	BOOST_CHECK(!fit_errors);
	BOOST_TEST_MESSAGE("Enhancing");
	BOOST_CHECK(enhancing);

	
	for (int i = 0; i < nIAUC; i++)
	{
		BOOST_TEST_MESSAGE("IAUC " + std::to_string(IAUCTimes[i]));
		BOOST_CHECK_CLOSE(IAUC_fit[i], IAUCVals[i], tol);
	}
	

	//Tidy up
	fs::remove(inputDataFile);
	fs::remove(dynTimesFile);
	fs::remove_all(Ct_output_dir);
}

BOOST_AUTO_TEST_SUITE_END() //
