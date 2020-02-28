#include <testlib/testlib_test.h>
#include <testlib/testlib_root_dir.h>
#include <iostream>
#include <madym/mdm_AIF.h>
#include "mdm_test_utils.h"

void run_test_AIF()
{
	std::cout<<"======= Testing AIF generation, reading and writing ======="<<std::endl;

	//Read in the calibration files
	
	//For dynamic times, open file, read in nTimes, then read in vector of times
	int nTimes;
	std::string timesFileName(mdm_test_utils::calibration_dir() + "dyn_times.dat");
	std::ifstream timesFileStream(timesFileName, std::ios::in | std::ios::binary);
	timesFileStream.read(reinterpret_cast<char*>(&nTimes), sizeof(int));

	std::vector<double> dynTimes(nTimes);
	for (double &t : dynTimes)
		timesFileStream.read(reinterpret_cast<char*>(&t), sizeof(double));
	timesFileStream.close();
	
	//For the AIF, read in the injection image, haematocrit correction and dose
	//then the values
	int injectionImage;
	double hct;
	double dose;
	std::vector<double> aifVals(nTimes);
	std::vector<double> pifVals(nTimes);

	std::string aifFileName(mdm_test_utils::calibration_dir() + "aif.dat");
	std::ifstream aifFileStream(aifFileName, std::ios::in | std::ios::binary);
	aifFileStream.read(reinterpret_cast<char*>(&injectionImage), sizeof(int));
	aifFileStream.read(reinterpret_cast<char*>(&hct), sizeof(double));
	aifFileStream.read(reinterpret_cast<char*>(&dose), sizeof(double));
	for (double &a : aifVals)
		aifFileStream.read(reinterpret_cast<char*>(&a), sizeof(double));
	aifFileStream.close();

	std::string pifFileName(mdm_test_utils::calibration_dir() + "pif.dat");
	std::ifstream pifFileStream(pifFileName, std::ios::in | std::ios::binary);
	for (double &a : pifVals)
		pifFileStream.read(reinterpret_cast<char*>(&a), sizeof(double));
	pifFileStream.close();

	std::cout << "Calibration data read, nTimes = " << nTimes <<
		", injectionImage = " << injectionImage <<
		", Hct = " << hct <<
		", dose = " << dose << std::endl;

	//Now we can test an AIF we create against these values
	mdm_AIF AIF_pop;
	AIF_pop.setAIFflag(mdm_AIF::AIF_POP);
	AIF_pop.setPrebolus(injectionImage);
	AIF_pop.setHct(hct);
	AIF_pop.setDose(dose);
	AIF_pop.setAIFTimes(dynTimes);
	AIF_pop.resample_AIF(nTimes, 0);

	TEST("Generating population AIF, values match", aifVals, AIF_pop.AIF());

	//Now test writing this AIF out...
	std::string aif_name = mdm_test_utils::temp_dir() + "/auto_AIF.txt";
	bool success = AIF_pop.writeAIF(aif_name);
	TEST("Writing AIF", success, true);

	//Then reading it back in...
	mdm_AIF AIF_auto;
	AIF_auto.readAIF(aif_name, nTimes);
	AIF_auto.setHct(0); //So we don't correct twice
	AIF_auto.resample_AIF(nTimes, 0);
	TEST("Reading AIF: flag", AIF_auto.AIFflag(), mdm_AIF::AIF_FILE);
	TEST("Reading AIF: times", mdm_test_utils::vectors_near_equal(
		AIF_auto.AIFTimes(), dynTimes, 1e-4), true);
	TEST("Reading AIF: values", mdm_test_utils::vectors_near_equal(
		AIF_auto.AIF(), aifVals, 1e-4), true);

	//Repeat tests for PIF
	AIF_pop.setPIFflag(mdm_AIF::PIF_POP);
	AIF_pop.resample_PIF(nTimes, 0);
	TEST("PIF pop values match", AIF_pop.PIF(), pifVals);

	std::string pif_name = mdm_test_utils::temp_dir() + "/auto_PIF.txt";
	success = AIF_pop.writePIF(pif_name);
	TEST("Writing PIF", success, true);

	AIF_auto.readPIF(pif_name, nTimes);
	AIF_auto.resample_PIF(nTimes, 0);
	TEST("Reading PIF: flag", AIF_auto.PIFflag(), mdm_AIF::PIF_FILE);
	TEST("Reading PIF: times", mdm_test_utils::vectors_near_equal(
		AIF_auto.AIFTimes(), dynTimes, 1e-4), true);
	TEST("Reading PIF: values", mdm_test_utils::vectors_near_equal(
		AIF_auto.PIF(), pifVals, 1e-4), true);
}

void test_AIF()
{
  run_test_AIF();
}

TESTMAIN(test_AIF);
