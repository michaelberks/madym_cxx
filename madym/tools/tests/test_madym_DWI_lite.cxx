#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>

#include <fstream>
#include <madym/dwi/mdm_DWIFitterADC.h>
#include <madym/dwi/mdm_DWIFitterIVIM.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

void run_DWI_lite_ADC_test()
{
  //Generate some signals from sample S0 and ADC values
  std::vector<double> Bvals = { 0, 150, 500, 800 };
  int nBvals = (int)Bvals.size();
  double S0 = 100;
  double ADC = 0.8e-3;
  auto signals = mdm_DWIFitterADC::modelToSignals({ S0, ADC }, Bvals);

  std::string test_dir = mdm_test_utils::temp_dir();
  std::string inputDataFile = test_dir + "/DWI_input.dat";
  std::ofstream ifs(inputDataFile, std::ios::out);

  BOOST_REQUIRE_MESSAGE(ifs.is_open(), "Failed to write out test values for DWI");

  //Write out FAs
  for (const auto b : Bvals)
    ifs << b << " ";
  

  //Compute signal for each FA and write out value
  for (const auto s : signals)
    ifs << s << " ";

  ifs.close();

  //Call madym_DWI
  std::string DWI_output_dir = test_dir + "/madym_DWI_lite/";
  std::string outputName = "madym_analysis.dat";
  std::stringstream cmd;
  cmd << mdm_test_utils::tools_exe_dir() << "madym_DWI_lite"
    << " --DWI_method ADC "
    << " --data " << inputDataFile
    << " --n_DWI " << nBvals
    << " -o " << DWI_output_dir
    << " -O " << outputName;

  BOOST_TEST_MESSAGE("Command to run: " + cmd.str());

  int error;
  try
  {
    error = std::system(cmd.str().c_str());
  }
  catch (...)
  {
    BOOST_CHECK_MESSAGE(false, "Running madym_DWI_lite failed");
    return;
  }

  BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DWI_lite tool");

  //Load in the fitted parameters from the output file
  std::string outputDataFile = DWI_output_dir + "ADC_" + outputName;
  std::ifstream ofs(outputDataFile, std::ios::in);
  BOOST_REQUIRE_MESSAGE(ofs.is_open(), "Failed to read in fitted DWI model parameters");

  double S0_fit, ADC_fit, ssr;
  int fit_errors;
  ofs >> S0_fit;
  ofs >> ADC_fit;
  ofs >> ssr;
  ofs >> fit_errors;
  ofs.close();

  //Check the model parameters have fitted correctly
  double tol = 0.1;
  BOOST_TEST_MESSAGE("Testing fitted S0");
  BOOST_CHECK_CLOSE(S0_fit, S0, tol);
  BOOST_TEST_MESSAGE("Testing fitted ADC");
  BOOST_CHECK_CLOSE(ADC_fit, ADC, tol);
  BOOST_TEST_MESSAGE("Checking zero error-codes");
  BOOST_CHECK(!fit_errors);

  //Tidy up
  fs::remove(inputDataFile);
  fs::remove_all(DWI_output_dir);
}

void run_DWI_lite_IVIM_test()
{
  //Generate some signals from sample S0 and ADC values
  std::vector<double> Bvals = { 0.0, 20.0, 40.0, 60.0, 80.0, 100.0, 300.0, 500.0, 800.0 };
  int nBvals = (int)Bvals.size();
  double S0 = 100;
  double d = 0.8e-3;
  double f = 0.2;
  double dstar = 15e-3;
  auto signals = mdm_DWIFitterIVIM::modelToSignals({ S0, d, f, dstar }, Bvals);

  std::string test_dir = mdm_test_utils::temp_dir();
  std::string inputDataFile = test_dir + "/DWI_input.dat";
  std::ofstream ifs(inputDataFile, std::ios::out);

  BOOST_REQUIRE_MESSAGE(ifs.is_open(), "Failed to write out test values for DWI");

  //Write out FAs
  for (const auto b : Bvals)
    ifs << b << " ";


  //Compute signal for each FA and write out value
  for (const auto s : signals)
    ifs << s << " ";

  ifs.close();

  //Call madym_DWI
  std::string DWI_output_dir = test_dir + "/madym_DWI_lite/";
  std::string outputName = "madym_analysis.dat";
  std::stringstream cmd;
  cmd << mdm_test_utils::tools_exe_dir() << "madym_DWI_lite"
    << " --DWI_method IVIM "
    << " --data " << inputDataFile
    << " --n_DWI " << nBvals
    << " --Bvals_thresh 40.0,60.0,100.0,150.0"
    << " -o " << DWI_output_dir
    << " -O " << outputName;

  BOOST_TEST_MESSAGE("Command to run: " + cmd.str());

  int error;
  try
  {
    error = std::system(cmd.str().c_str());
  }
  catch (...)
  {
    BOOST_CHECK_MESSAGE(false, "Running madym_DWI_lite failed");
    return;
  }

  BOOST_CHECK_MESSAGE(!error, "Error returned from madym_DWI_lite tool");

  //Load in the fitted parameters from the output file
  std::string outputDataFile = DWI_output_dir + "IVIM_" + outputName;
  std::ifstream ofs(outputDataFile, std::ios::in);
  BOOST_REQUIRE_MESSAGE(ofs.is_open(), "Failed to read in fitted DWI model parameters");

  double S0_fit, d_fit, f_fit, dstar_fit, ssr;
  int fit_errors;
  ofs >> S0_fit;
  ofs >> d_fit;
  ofs >> f_fit;
  ofs >> dstar_fit;
  ofs >> ssr;
  ofs >> fit_errors;
  ofs.close();

  //Check the model parameters have fitted correctly
  double tol = 0.5;
  BOOST_TEST_MESSAGE("Testing fitted S0");
  BOOST_CHECK_CLOSE(S0_fit, S0, tol);
  BOOST_TEST_MESSAGE("Testing fitted d");
  BOOST_CHECK_CLOSE(d_fit, d, tol);
  BOOST_TEST_MESSAGE("Testing fitted f");
  BOOST_CHECK_CLOSE(f_fit, f, tol);
  BOOST_TEST_MESSAGE("Testing fitted dstar");
  BOOST_CHECK_CLOSE(dstar_fit, dstar, tol);
  BOOST_TEST_MESSAGE("Checking zero error-codes");
  BOOST_CHECK(!fit_errors);

  //Tidy up
  fs::remove(inputDataFile);
  //fs::remove_all(DWI_output_dir);
}

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_DWI_lite) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym DWI lite =======");
  
  //Run ADC fit
  run_DWI_lite_ADC_test();

  //Run IVIM fit
  run_DWI_lite_IVIM_test();
	
}

BOOST_AUTO_TEST_SUITE_END() //
