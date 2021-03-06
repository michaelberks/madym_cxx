#include <boost/test/unit_test.hpp>

#include <madym/tests/mdm_test_utils.h>

#include <fstream>
#include <madym/t1/mdm_T1FitterVFA.h>
#include <madym/t1/mdm_T1FitterIR.h>
#include <mdm_version.h>

namespace fs = boost::filesystem;

void run_T1_lite_VFA_test(double B1)
{
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

  if (B1)
  {
    //Write out FAs
    for (const auto FA : FAs)
      ifs << FA/B1 << " ";
  }
  else
  {
    //Write out FAs
    for (const auto FA : FAs)
      ifs << FA << " ";
  }
  

  //Compute signal for each FA and write out value
  for (int i_fa = 0; i_fa < 3; i_fa++)
    ifs << mdm_T1FitterVFA::T1toSignal(T1, M0, PI*FAs[i_fa] / 180, TR) << " ";

  if (B1)
    ifs << B1;

  ifs.close();

  std::string method;
  if (B1)
  {
    BOOST_TEST_MESSAGE("Testing VFA with B1 correction");
    method = "VFA_B1";
  }
    
  else
  {
    BOOST_TEST_MESSAGE("Testing VFA");
    method = "VFA";
  } 

  //Call madym_T1 to fit T1 and M0
  std::string T1_output_dir = test_dir + "/madym_T1_lite/";
  std::string outputName = "madym_analysis.dat";
  std::stringstream cmd;
  cmd << mdm_test_utils::tools_exe_dir() << "madym_T1_lite"
    << " -T " << method
    << " --data " << inputDataFile
    << " --n_T1 " << 3
    << " --TR " << TR
    << " -o " << T1_output_dir
    << " -O " << outputName;

  BOOST_TEST_MESSAGE("Command to run: " + cmd.str());

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
  std::string outputDataFile = T1_output_dir + method + "_" + outputName;
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

void run_T1_lite_IR_test()
{
  //Generate some signals from sample FA, TR, T1 and M0 values
 //Generate some signals from sample FA, TR, T1 and S0 values
  std::vector<double> TIs = { 50, 300, 800, 1000, 2000, 4000 };
  int nTIs = (int)TIs.size();
  double T1 = 1200;
  double M0 = 1200;
  double TR = 1e5;

  std::string test_dir = mdm_test_utils::temp_dir();
  std::string inputDataFile = test_dir + "/T1_input.dat";
  std::ofstream ifs(inputDataFile, std::ios::out);

  BOOST_REQUIRE_MESSAGE(ifs.is_open(), "Failed to write out test values for T1");

  //Write out TIs
  for (const auto ti : TIs)
    ifs << ti << " ";

  //Compute signal for each IR and write out value
  for (int i = 0; i < nTIs; i++)
    ifs << mdm_T1FitterIR::T1toSignal(T1, M0, TIs[i], TR) << " ";

  ifs.close();

  std::string method;
  BOOST_TEST_MESSAGE("Testing IR");
  method = "IR";

  //Call madym_T1 to fit T1 and M0
  std::string T1_output_dir = test_dir + "/madym_T1_lite/";
  std::string outputName = "madym_analysis.dat";
  std::stringstream cmd;
  cmd << mdm_test_utils::tools_exe_dir() << "madym_T1_lite"
    << " -T " << method
    << " --data " << inputDataFile
    << " --n_T1 " << nTIs
    << " --TR " << TR
    << " -o " << T1_output_dir
    << " -O " << outputName;

  BOOST_TEST_MESSAGE("Command to run: " + cmd.str());

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
  std::string outputDataFile = T1_output_dir + method + "_" + outputName;
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

BOOST_AUTO_TEST_SUITE(test_mdm_tools)

BOOST_AUTO_TEST_CASE(test_madym_T1_lite) {
	BOOST_TEST_MESSAGE("======= Testing tool: madym T1 lite =======");
  
  //Run VFA fit with no B1 correction
  run_T1_lite_VFA_test(0);

  //Run VFA fit with B1 correction
  run_T1_lite_VFA_test(0.9);

  //Run IR fit
  run_T1_lite_IR_test();
	
}

BOOST_AUTO_TEST_SUITE_END() //
