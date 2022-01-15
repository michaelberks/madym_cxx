#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/t1/mdm_T1FitterVFA.h>
#include <madym/t1/mdm_T1FitterIR.h>
#include <madym/tests/mdm_test_utils.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_T1_VFA) {
  BOOST_TEST_MESSAGE("======= Testing T1 VFA mapping =======");

  //Read in T1 calibration file
  int nFAs;
  double T1;
  double M0;
  double TR;
  std::string T1FileName(mdm_test_utils::calibration_dir() + "T1.dat");
  std::ifstream T1FileStream(T1FileName, std::ios::in | std::ios::binary);
  T1FileStream.read(reinterpret_cast<char*>(&nFAs), sizeof(int));

  std::vector<double> FAs(nFAs);
  std::vector<double> signalsCalibration(nFAs);
  for (double &fa : FAs)
    T1FileStream.read(reinterpret_cast<char*>(&fa), sizeof(double));
  for (double &s : signalsCalibration)
    T1FileStream.read(reinterpret_cast<char*>(&s), sizeof(double));

  T1FileStream.read(reinterpret_cast<char*>(&T1), sizeof(double));
  T1FileStream.read(reinterpret_cast<char*>(&M0), sizeof(double));
  T1FileStream.read(reinterpret_cast<char*>(&TR), sizeof(double));

  T1FileStream.close();

  BOOST_TEST_MESSAGE(boost::format(
    "Read T1 calibration file, T1 = %1% , M0 =%2%, TR = %3%")
    % T1 % M0 % TR);

  //Now compute signals
  std::vector<double> signals(nFAs);
  BOOST_TEST_MESSAGE("Testing signals from VFA (calibration, computed): ");
  for (int i = 0; i < nFAs; i++)
  {
    signals[i] = mdm_T1FitterVFA::T1toSignal(T1, M0, FAs[i], TR);
    BOOST_TEST_MESSAGE(boost::format("(%1%, %2%)")
      % signalsCalibration[i] % signals[i]);
  }
  BOOST_CHECK_VECTORS(signals, signalsCalibration);

  //Next fit the signals to recover M0 and T1
  double T1fit, M0fit;
  mdm_T1FitterVFA T1CalculatorVFA(FAs, TR, false);
  T1CalculatorVFA.setInputs(signalsCalibration);
  auto errCode = T1CalculatorVFA.fitT1(T1fit, M0fit);
  BOOST_CHECK_MESSAGE(!errCode, "T1 fit returned error " << errCode);

  BOOST_TEST_MESSAGE("Testing fitted T1 match using VFA");
  BOOST_CHECK_CLOSE(T1fit, T1, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted M0 match using VFA");
  BOOST_CHECK_CLOSE(M0fit, M0, 0.01);

  //Repeat the test using VFA B1 correction
  auto signalsCalibrationB1 = signalsCalibration;
  auto FAsB1 = FAs;
  double B1 = 0.9;
  for (auto &fa : FAsB1)
    fa /= B1;
  signalsCalibrationB1.push_back(B1);

  mdm_T1FitterVFA T1CalculatorB1(FAsB1, TR, true);
  T1CalculatorB1.setInputs(signalsCalibrationB1);
  errCode = T1CalculatorB1.fitT1(T1fit, M0fit);
  BOOST_CHECK_MESSAGE(!errCode, "T1 fit returned error " << errCode);

  BOOST_TEST_MESSAGE("Testing fitted T1 match using VFA B1 correction");
  BOOST_CHECK_CLOSE(T1fit, T1, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted M0 match using VFA B1 correction");
  BOOST_CHECK_CLOSE(M0fit, M0, 0.01);
}

BOOST_AUTO_TEST_CASE(test_T1_IR) {
  BOOST_TEST_MESSAGE("======= Testing T1 inversion recovery mapping =======");
  //Test inversion recovery method
 
  //Read in T1 calibration file
  int nTIs;
  double T1;
  double M0;
  double TR;
  std::string T1FileName(mdm_test_utils::calibration_dir() + "T1_IR.dat");
  std::ifstream T1FileStream(T1FileName, std::ios::in | std::ios::binary);
  T1FileStream.read(reinterpret_cast<char*>(&nTIs), sizeof(int));

  std::vector<double> TIs(nTIs);
  std::vector<double> signalsCalibration(nTIs);
  for (double &ti : TIs)
    T1FileStream.read(reinterpret_cast<char*>(&ti), sizeof(double));
  for (double &s : signalsCalibration)
    T1FileStream.read(reinterpret_cast<char*>(&s), sizeof(double));

  T1FileStream.read(reinterpret_cast<char*>(&T1), sizeof(double));
  T1FileStream.read(reinterpret_cast<char*>(&M0), sizeof(double));
  T1FileStream.read(reinterpret_cast<char*>(&TR), sizeof(double));

  T1FileStream.close();

  BOOST_TEST_MESSAGE(boost::format(
    "Read T1 calibration file, T1 = %1% , M0 =%2%, TR = %3%")
    % T1 % M0 % TR);
 
  std::vector<double> signals(nTIs);
  BOOST_TEST_MESSAGE("Testing signals from IR (calibration, computed): ");
  for (int i = 0; i < nTIs; i++)
  {
    signals[i] = mdm_T1FitterIR::T1toSignal(T1, M0, TIs[i], TR);
    BOOST_TEST_MESSAGE(boost::format("(%1%, %2%)")
      % signalsCalibration[i] % signals[i]);
  }

  mdm_T1FitterIR T1CalculatorIR(TIs, TR);
  T1CalculatorIR.setInputs(signalsCalibration);

  double T1fit, M0fit;
  auto errCode = T1CalculatorIR.fitT1(T1fit, M0fit);
  BOOST_CHECK_MESSAGE(!errCode, "T1 fit returned error " << errCode);

  BOOST_TEST_MESSAGE("Testing fitted T1 match using inversion recovery");
  BOOST_CHECK_CLOSE(T1fit, T1, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted M0 match using inversion recovery");
  BOOST_CHECK_CLOSE(M0fit, M0, 0.01);

}

BOOST_AUTO_TEST_SUITE_END() //
