#include <testlib/testlib_test.h>
#include <iostream>
#include <iomanip>
#include <madym/mdm_AIF.h>
#include <madym/mdm_DCEModelGenerator.h>
#include <madym/mdm_DCEVoxel.h>

#include "mdm_test_utils.h"

void test_model_time_fit(
	const std::string &modelName,
	const std::vector<int> fixedParams,
	mdm_AIF &AIF,
	const double paramTol,
	const double sseTol,
	bool test_IAUC = false)
{
	//Read in the model calibration file - this has noise added to it
	int nTimes = AIF.AIFTimes().size();
	int nParams;
	std::vector<double> CtCalibration(nTimes);
	std::string modelFileName = mdm_test_utils::calibration_dir() + modelName + "_noise.dat";

	std::ifstream modelFileStream(modelFileName, std::ios::in | std::ios::binary);
	modelFileStream.read(reinterpret_cast<char*>(&nParams), sizeof(int));

	std::vector<double> trueParams(nParams);
	for (double &p : trueParams)
		modelFileStream.read(reinterpret_cast<char*>(&p), sizeof(double));
	for (double &c : CtCalibration)
		modelFileStream.read(reinterpret_cast<char*>(&c), sizeof(double));
	modelFileStream.close();
	std::cout << "Read time series for " << modelName << " from binary calibration file" << std::endl;

	int nIAUC;
	std::vector<double> IAUCTimes;
	std::vector<double> IAUCVals;
	if (test_IAUC)
	{
		
		std::string iaucFileName = mdm_test_utils::calibration_dir() + modelName + "_IAUC.dat";

		std::ifstream iaucFileStream(iaucFileName, std::ios::in | std::ios::binary);
		iaucFileStream.read(reinterpret_cast<char*>(&nIAUC), sizeof(int));

		IAUCTimes.resize(nIAUC);
		IAUCVals.resize(nIAUC);
		for (double &itm : IAUCTimes)
		{
			double its;
			iaucFileStream.read(reinterpret_cast<char*>(&its), sizeof(double));
			itm = its / 60;
		}
			
		for (double &iv : IAUCVals)
			iaucFileStream.read(reinterpret_cast<char*>(&iv), sizeof(double));
		iaucFileStream.close();
		std::cout << "Read time series for " << modelName << " from binary calibration file" << std::endl;
	}

	//Now create the model and compute model time series
	mdm_DCEModelBase *model = NULL;
	bool model_set = mdm_DCEModelGenerator::setModel(model, AIF,
		modelName, false, false, {},
		{}, fixedParams, {});

	if (!model_set)
	{
		std::cout << "Unable to compute time series for " << modelName << std::endl;
		return;
	}

	mdm_DCEVoxel vox(
		{},
		CtCalibration,
		{},
		0,
		0,
		0,
		AIF.prebolus(),
		AIF.AIFTimes(),
		0,
		0,
		0,
		nTimes,
		false,
		true,
		IAUCTimes);
	vox.initialiseModelFit(*model);
	vox.fitModel();

	//Check params match (should be within 0.01)
	std::cout << "Actual vs fitted params: ";
	std::cout << std::fixed << std::showpoint << std::setprecision(2);

	for (int i = 0; i < nParams; i++)
	{
		std::cout << "(" << trueParams[i] << ", " <<
			model->pkParams()[i] << ") ";
	}
	std::cout << std::endl;
	std::cout << std::fixed << std::showpoint << std::setprecision(4);

	std::cout << "Model SSE = " << vox.modelFitError() << std::endl;
	std::string test_str = "Test DCE models, values match: " + modelName;
	TEST(test_str.c_str(), mdm_test_utils::vectors_near_equal_rel(
		model->pkParams(), trueParams, paramTol), true);

	test_str = "Test DCE models, SSE < tol: " + modelName;
	TEST_NEAR(test_str.c_str(), vox.modelFitError(), 0, sseTol);

	if (test_IAUC)
	{
		vox.calculateIAUC();
		std::vector<double> computedIAUC(nIAUC);
		for (int i = 0; i < nIAUC; i++)
			computedIAUC[i] = vox.IAUC_val(i);

		test_str = "Test IAUC values for " + modelName;
		TEST(test_str.c_str(), mdm_test_utils::vectors_near_equal(
			computedIAUC, IAUCVals, 0.01), true);
	}

	delete model;
}

void run_test_DCE_fit()
{
	std::cout << "======= Testing DCE model optimisation =======" << std::endl;
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
	test_model_time_fit(
		"ETM", {},
		AIF, 0.1, 0.0005, true);
	test_model_time_fit(
		"DIETM", {6},
		AIF, 0.5, 0.0005);
	test_model_time_fit(
		"AUEM", {7},
		AIF, 0.5, 0.0005);
	test_model_time_fit(
		"DISCM", {},
		AIF, 0.5, 0.0005);
	test_model_time_fit(
		"2CXM", {},
		AIF, 0.2, 0.0005);
	test_model_time_fit(
		"DI2CXM", {7},
		AIF, 0.5, 0.0005);
	test_model_time_fit(
		"DIBEM", {7},
		AIF, 0.5, 0.0005);
}

void test_DCE_fit()
{
  run_test_DCE_fit();
}

TESTMAIN(test_DCE_fit);
