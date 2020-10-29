#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <vector>
#include <madym/t1_methods/mdm_T1FitterVFA.h>
#include <madym/tests/mdm_test_utils.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_T1) {
	BOOST_TEST_MESSAGE("======= Testing T1 mapping =======");
	
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
	mdm_T1FitterVFA T1Calculator(FAs, TR);
	T1Calculator.setInputSignals(signalsCalibration);
	int errCode = T1Calculator.fitT1(T1fit, M0fit);
	BOOST_CHECK_MESSAGE(!errCode, "T1 fit returned error " << errCode);

	BOOST_TEST_MESSAGE("Testing fitted T1 match");
	BOOST_CHECK_CLOSE(T1fit, T1, 0.01);
	BOOST_TEST_MESSAGE("Testing fitted M0 match");
	BOOST_CHECK_CLOSE(M0fit, M0, 0.01);
}

BOOST_AUTO_TEST_SUITE_END() //
