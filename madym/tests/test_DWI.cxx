#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/dwi/mdm_DWIFitterADC.h>
#include <madym/dwi/mdm_DWIFitterIVIM.h>
#include <madym/tests/mdm_test_utils.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_DWI_ADC) {
  BOOST_TEST_MESSAGE("======= Testing ADC mapping =======");

  //Read in ADC calibration file
  int nBvals;
  double S0;
  double ADC;
  
  std::string DWIFileName(mdm_test_utils::calibration_dir() + "DWI_ADC.dat");
  std::ifstream DWIFileStream(DWIFileName, std::ios::in | std::ios::binary);
  DWIFileStream.read(reinterpret_cast<char*>(&nBvals), sizeof(int));

  std::vector<double> Bvals(nBvals);
  std::vector<double> signalsCalibration(nBvals);
  for (double &b : Bvals)
    DWIFileStream.read(reinterpret_cast<char*>(&b), sizeof(double));
  for (double &s : signalsCalibration)
    DWIFileStream.read(reinterpret_cast<char*>(&s), sizeof(double));

  DWIFileStream.read(reinterpret_cast<char*>(&S0), sizeof(double));
  DWIFileStream.read(reinterpret_cast<char*>(&ADC), sizeof(double));

  DWIFileStream.close();

  BOOST_TEST_MESSAGE(boost::format(
    "Read ADC calibration file, S0 = %1% , ADC =%2%")
    % S0 % ADC);

  //Now compute signals
  auto signals = mdm_DWIFitterADC::modelToSignals({ S0, ADC }, Bvals);
  BOOST_CHECK_VECTORS(signals, signalsCalibration);

  //Next fit the signals to recover S0 and ADC
  std::vector<double> ADCfit;
  double ssr;
  mdm_DWIFitterADC DWIFitterADC(Bvals, false);
  DWIFitterADC.setSignals(signalsCalibration);
  auto errCode = DWIFitterADC.fitModel(ADCfit, ssr);
  BOOST_CHECK_MESSAGE(!errCode, "ADC fit returned error " << errCode);

  BOOST_TEST_MESSAGE("Testing fitted S0 match");
  BOOST_CHECK_CLOSE(ADCfit[0], S0, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted ADC match");
  BOOST_CHECK_CLOSE(ADCfit[1], ADC, 0.01);
}

BOOST_AUTO_TEST_CASE(test_DWI_IVIM) {
  BOOST_TEST_MESSAGE("======= Testing IVIM model fitting =======");
  //Read in IVIM calibration file
  int nBvals;
  double S0;
  double d;
  double f;
  double dstar;

  std::string DWIFileName(mdm_test_utils::calibration_dir() + "DWI_IVIM.dat");
  std::ifstream DWIFileStream(DWIFileName, std::ios::in | std::ios::binary);
  DWIFileStream.read(reinterpret_cast<char*>(&nBvals), sizeof(int));

  std::vector<double> Bvals(nBvals);
  std::vector<double> signalsCalibration(nBvals);
  for (double& b : Bvals)
    DWIFileStream.read(reinterpret_cast<char*>(&b), sizeof(double));
  for (double& s : signalsCalibration)
    DWIFileStream.read(reinterpret_cast<char*>(&s), sizeof(double));

  DWIFileStream.read(reinterpret_cast<char*>(&S0), sizeof(double));
  DWIFileStream.read(reinterpret_cast<char*>(&d), sizeof(double));
  DWIFileStream.read(reinterpret_cast<char*>(&f), sizeof(double));
  DWIFileStream.read(reinterpret_cast<char*>(&dstar), sizeof(double));

  DWIFileStream.close();

  BOOST_TEST_MESSAGE(boost::format(
    "Read IVIM calibration file, S0 = %1% , d = %2%, f = %3%, d* = %4% ")
    % S0 % d % f % dstar);

  //Now compute signals
  auto signals = mdm_DWIFitterIVIM::modelToSignals({ S0, d, f, dstar }, Bvals);
  BOOST_CHECK_VECTORS(signals, signalsCalibration);

  //Next fit the signals to recover IVIM params
  std::vector<double> IVIMfit;
  double ssr;
  std::vector<double> BvalsThresh = { 40, 60, 100, 150 };
  mdm_DWIFitterIVIM DWIFitterIVIM(Bvals, true, BvalsThresh);
  DWIFitterIVIM.setSignals(signalsCalibration);
  auto errCode = DWIFitterIVIM.fitModel(IVIMfit, ssr);
  BOOST_CHECK_MESSAGE(!errCode, "IVIM fit returned error " << errCode);

  BOOST_TEST_MESSAGE("Testing fitted S0 match");
  BOOST_CHECK_CLOSE(IVIMfit[0], S0, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted d match");
  BOOST_CHECK_CLOSE(IVIMfit[1], d, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted d match");
  BOOST_CHECK_CLOSE(IVIMfit[2], f, 0.01);
  BOOST_TEST_MESSAGE("Testing fitted d match");
  BOOST_CHECK_CLOSE(IVIMfit[3], dstar, 0.01);

}

BOOST_AUTO_TEST_SUITE_END() //
