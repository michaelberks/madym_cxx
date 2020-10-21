#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <madym/mdm_AIF.h>
#include <madym/tests/mdm_test_utils.h>

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_AIF) {
	BOOST_TEST_MESSAGE("======= Testing AIF generation, reading and writing =======");

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

	BOOST_TEST_MESSAGE(boost::format("Calibration data read, nTimes = %1%"
		", injectionImage = %2%"
		", Hct = %3%"
		", dose = %4%")
		% nTimes % injectionImage % hct % dose);

	//Now we can test an AIF we create against these values
	mdm_AIF AIF_pop;
	AIF_pop.setAIFflag(mdm_AIF::AIF_POP);
	AIF_pop.setPrebolus(injectionImage);
	AIF_pop.setHct(hct);
	AIF_pop.setDose(dose);
	AIF_pop.setAIFTimes(dynTimes);
	AIF_pop.resample_AIF(nTimes, 0);

	BOOST_TEST_MESSAGE("Testing population AIF values match");
	BOOST_CHECK_VECTORS(aifVals, AIF_pop.AIF());

	//Now test writing this AIF out...
	std::string aif_name = mdm_test_utils::temp_dir() + "/auto_AIF.txt";
	BOOST_TEST_MESSAGE("Testing writing AIF to file");
	BOOST_CHECK(AIF_pop.writeAIF(aif_name));

	//Then reading it back in...
	mdm_AIF AIF_auto;
	AIF_auto.readAIF(aif_name, nTimes);
	AIF_auto.setHct(0); //So we don't correct twice
	AIF_auto.resample_AIF(nTimes, 0);

	BOOST_TEST_MESSAGE("Testing reading AIF values from file");
	BOOST_CHECK_EQUAL(AIF_auto.AIFflag(), mdm_AIF::AIF_FILE);//"Reading AIF: flag"
	BOOST_CHECK(mdm_test_utils::vectors_near_equal(
		AIF_auto.AIFTimes(), dynTimes, 1e-4));//"Reading AIF: times"

	BOOST_CHECK(mdm_test_utils::vectors_near_equal(
		AIF_auto.AIF(), aifVals, 1e-4));//"Reading AIF: values"

	//Repeat tests for PIF
	AIF_pop.setPIFflag(mdm_AIF::PIF_POP);
	AIF_pop.resample_PIF(nTimes, 0);

	BOOST_TEST_MESSAGE("Testing population PIF values match");
	BOOST_CHECK_VECTORS(AIF_pop.PIF(), pifVals);

	BOOST_TEST_MESSAGE("Testing writing PIF to file");
	std::string pif_name = mdm_test_utils::temp_dir() + "/auto_PIF.txt";
	BOOST_CHECK(AIF_pop.writePIF(pif_name));//"Writing PIF"

	AIF_auto.readPIF(pif_name, nTimes);
	AIF_auto.resample_PIF(nTimes, 0);

	BOOST_TEST_MESSAGE("Testing reading PIF from file");
	BOOST_CHECK_EQUAL(AIF_auto.PIFflag(), mdm_AIF::PIF_FILE);//"Reading PIF: flag"
	BOOST_CHECK(mdm_test_utils::vectors_near_equal(
		AIF_auto.AIFTimes(), dynTimes, 1e-4));//"Reading PIF: times"
	BOOST_CHECK(mdm_test_utils::vectors_near_equal(
		AIF_auto.PIF(), pifVals, 1e-4));//"Reading PIF: values"

}

BOOST_AUTO_TEST_SUITE_END() //