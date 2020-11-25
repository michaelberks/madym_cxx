#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <iomanip>
#include <string>

#include <madym/mdm_AIF.h>
#include <madym/tests/mdm_test_utils.h>
#include <madym/mdm_AIF.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>
#include <madym/mdm_DCEVoxel.h>
#include <madym/mdm_DCEModelFitter.h>


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
	BOOST_TEST_MESSAGE(boost::format(
		"Read time series for %1% from binary calibration file") % modelName);

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
		BOOST_TEST_MESSAGE(boost::format(
			"Read IAUC data for %1% from binary calibration file") % modelName);
	}

	//Now create the model and compute model time series
	auto modelType = mdm_DCEModelGenerator::ParseModelName(modelName);
	BOOST_REQUIRE_MESSAGE(modelType != mdm_DCEModelGenerator::UNDEFINED,
		"Model name " << modelName << " is undefined");

  AIF.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
  AIF.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
	auto model = mdm_DCEModelGenerator::createModel(AIF,
		modelType, {},
		{}, fixedParams, {}, {}, {});

  mdm_DCEModelFitter fitter(
    *model,
    0,
    nTimes,
    {}
  );

	mdm_DCEVoxel vox(
		{},
		CtCalibration,
		AIF.prebolus(),
    AIF.AIFTimes(),
    IAUCTimes);

	fitter.initialiseModelFit(vox.CtData());
  fitter.fitModel(vox.status(), vox.enhancing());

	//Check params match (should be within 0.01)
	BOOST_TEST_MESSAGE("Actual vs fitted params: ");
	
	for (int i = 0; i < nParams; i++)
	{
		BOOST_TEST_MESSAGE(boost::format("( %1$.2f, %2$.2f )")
			% trueParams[i] % model->params()[i]);
	}

	BOOST_TEST_MESSAGE(boost::format("Model SSE = %1$.4f") % fitter.modelFitError());
	BOOST_TEST_MESSAGE("Test DCE models, values match: " + modelName);
	BOOST_CHECK(mdm_test_utils::vectors_near_equal_rel(
		model->params(), trueParams, paramTol));

	BOOST_TEST_MESSAGE("Test DCE models, SSE < tol: " + modelName);
	BOOST_CHECK_SMALL(fitter.modelFitError(), sseTol);

	if (test_IAUC)
	{
		vox.computeIAUC();
		std::vector<double> computedIAUC(nIAUC);
		for (int i = 0; i < nIAUC; i++)
			computedIAUC[i] = vox.IAUC_val(i);

		BOOST_TEST_MESSAGE("Test IAUC values for " + modelName);
		BOOST_CHECK(mdm_test_utils::vectors_near_equal(
			computedIAUC, IAUCVals, 0.01));
	}
}

BOOST_AUTO_TEST_SUITE(test_mdm)

BOOST_AUTO_TEST_CASE(test_DCE_fit) {
	BOOST_TEST_MESSAGE("======= Testing DCE model optimisation =======");
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
BOOST_AUTO_TEST_SUITE_END() //