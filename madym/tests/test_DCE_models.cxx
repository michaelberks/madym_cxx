#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>

#include <madym/tests/mdm_test_utils.h>
#include <madym/mdm_AIF.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>

void test_model_time_series(
	const std::string &modelName,
	mdm_AIF &AIF)
{
	//Read in the model calibration file
	auto nTimes = AIF.AIFTimes().size();
	int nParams;
	std::vector<double> CtCalibration(nTimes);
	std::string modelFileName = mdm_test_utils::calibration_dir() + modelName + ".dat";
	
	std::ifstream modelFileStream(modelFileName, std::ios::in | std::ios::binary);
	modelFileStream.read(reinterpret_cast<char*>(&nParams), sizeof(int));
	
	std::vector<double> initialParams(nParams);
	for (double &p : initialParams)
		modelFileStream.read(reinterpret_cast<char*>(&p), sizeof(double));
	for (double &c : CtCalibration)
		modelFileStream.read(reinterpret_cast<char*>(&c), sizeof(double));
	modelFileStream.close();
	BOOST_TEST_MESSAGE(boost::format(
		"Read time series for %1% from binary calibration file") % modelName);

	//Now create the model and compute model time series
	auto modelType = mdm_DCEModelGenerator::ParseModelName(modelName);
	BOOST_REQUIRE_MESSAGE(modelType != mdm_DCEModelGenerator::UNDEFINED,
		"Model name " << modelName << "Is undefined ");

  AIF.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
  AIF.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
	auto model = mdm_DCEModelGenerator::createModel(AIF,
		modelType, {},
		initialParams, {}, {}, {}, {});
	
	model->computeCtModel(nTimes);
	std::vector<double> Ct = model->CtModel();
	BOOST_TEST_MESSAGE("Test DCE models, values match: " + modelName);
	BOOST_CHECK(mdm_test_utils::vectors_near_equal(
        Ct, CtCalibration, 0.0001));
}

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_DCE_models) {
	BOOST_TEST_MESSAGE("======= Testing DCE models implemented in mdm library =======");
	
	//Read in dynamic times from calibration file
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
	aifFileStream.close();

	//Create AIF
	mdm_AIF AIF;
	AIF.setAIFTimes(dynTimes);
	AIF.setPrebolus(injectionImage);
	AIF.setHct(hct);
	AIF.setDose(dose);

	//Create model concentrations for each model type
	test_model_time_series(
		"ETM",
		AIF);
	test_model_time_series(
		"DIETM",
		AIF);
	test_model_time_series(
		"AUEM",
		AIF);
	test_model_time_series(
		"DISCM",
		AIF);
	test_model_time_series(
		"2CXM",
		AIF);
	test_model_time_series(
		"DI2CXM",
		AIF);
	test_model_time_series(
		"DIBEM",
		AIF);
  test_model_time_series(
    "PATLAK",
    AIF);
}

BOOST_AUTO_TEST_SUITE_END() //
