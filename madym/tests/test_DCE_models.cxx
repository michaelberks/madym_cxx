#include <testlib/testlib_test.h>
#include <iostream>
#include "mdm_test_utils.h"
#include <madym/mdm_AIF.h>
#include <madym/mdm_DCEModelGenerator.h>

void test_model_time_series(
	const std::string &modelName,
	mdm_AIF &AIF)
{
	//Read in the model calibration file
	int nTimes = AIF.AIFTimes().size();
	int nParams;
	std::vector<double> CtCalibration(nTimes);
	std::string modelFileName = mdm_test_utils::calibration_dir() + modelName + ".dat";
	
	std::ifstream modelFileStream(modelFileName, std::ios::in | std::ios::binary);
	modelFileStream.read(reinterpret_cast<char*>(&nParams), sizeof(int));
	
	std::vector<double> initParams(nParams);
	for (double &p : initParams)
		modelFileStream.read(reinterpret_cast<char*>(&p), sizeof(double));
	for (double &c : CtCalibration)
		modelFileStream.read(reinterpret_cast<char*>(&c), sizeof(double));
	modelFileStream.close();
	std::cout << "Read time series for " << modelName << " from binary calibration file" << std::endl;

	//Now create the model and compute model time series
	mdm_DCEModelBase *model = NULL;
	bool model_set = mdm_DCEModelGenerator::setModel(model, AIF,
		modelName, false, false, {},
		initParams, {}, {});

	if (!model_set)
	{
		std::cout << "Unable to compute time series for " << modelName << std::endl;
		return;
	}

	model->computeCtModel(nTimes);
	std::vector<double> Ct = model->CtModel();
	std::string test_str = "Test DCE models, values match: " + modelName;
	TEST(test_str.c_str(), mdm_test_utils::vectors_near_equal(
        Ct, CtCalibration, 0.0001), true);
	
	delete model;
}

void run_test_DCE_models()
{

	std::cout << "======= Testing DCE models implemented in mdm library =======" << std::endl;
	
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
}

void test_DCE_models()
{
  run_test_DCE_models();
}

TESTMAIN(test_DCE_models);
