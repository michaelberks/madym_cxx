#include <testlib/testlib_test.h>
#include <iostream>
#include <fstream>
#include "mdm_test_utils.h"
#include <madym/mdm_T1Voxel.h>

void run_test_T1()
{

	std::cout << "======= Testing T1 mapping =======" << std::endl;
	
	//Read in T1 calibration file
	int nFAs;
	double T1;
	double S0;
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
	T1FileStream.read(reinterpret_cast<char*>(&S0), sizeof(double));
	T1FileStream.read(reinterpret_cast<char*>(&TR), sizeof(double));
	
	T1FileStream.close();

	std::cout << "Read T1 calibration file, T1 = " << T1
		<< ", S0 = " << S0 << ", TR = " << TR << std::endl;

	//Now compute signals
	std::vector<double> signals(nFAs);
	std::cout << "Signals from VFA (calibration, computed): ";
	for (int i = 0; i < nFAs; i++)
	{
		signals[i] = mdm_T1Voxel::T1toSignal(T1, S0, FAs[i], TR);
		std::cout << "(" << signalsCalibration[i] << ", " <<
			signals[i] << ") ";
	}
	std::cout << std::endl;
	TEST("VFA signals match", signals, signalsCalibration);

	//Next fit the signals to recover S0 and T1
	double T1fit, S0fit;
	mdm_T1Voxel T1Calculator(FAs, TR);
	T1Calculator.setSignals(signalsCalibration);
	int errCode = T1Calculator.fitT1_VFA(T1fit, S0fit);

	if (errCode)
		std::cout << "T1 fit returned error " << errCode << std::endl;

	TEST_NEAR_REL("Fitted T1 match", T1fit, T1, 0.01);
	TEST_NEAR_REL("Fitted S0 match", S0fit, S0, 0.01);
}

void test_T1()
{
  run_test_T1();
}

TESTMAIN(test_T1);
