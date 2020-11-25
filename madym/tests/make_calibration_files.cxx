#include <iostream>
#include <ostream>

#include <madym/mdm_AIF.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>
#include <madym/t1_methods/mdm_T1FitterVFA.h>
#include "mdm_test_utils.h"

void write_series_to_binary(const std::string filename, 
	const std::vector<double> ts, const std::vector<double> params)
{
	int nTimes = ts.size();
	int nParams = params.size();
	std::ofstream modelFileStream(filename, std::ios::out | std::ios::binary);
	modelFileStream.write(reinterpret_cast<const char*>(
		&nParams), sizeof(int));
	modelFileStream.write(reinterpret_cast<const char*>(
		&params[0]), sizeof(double)*nParams);
	modelFileStream.write(reinterpret_cast<const char*>(
		&ts[0]), sizeof(double)*nTimes);
	modelFileStream.close();
}

void make_model_time_series(
	const std::string &outputDir,
	const std::string &modelName,
	const std::vector<double> &initialParams,
	mdm_AIF &AIF,
	bool makeIAUC)
{
	//generate specific instance of model
	auto modelType = mdm_DCEModelGenerator::ParseModelName(modelName);
	if (modelType == mdm_DCEModelGenerator::UNDEFINED)
	{
		std::cout << "Model name " << modelName << " is not defined " << std::endl;
		return;
	}

  AIF.setAIFType(mdm_AIF::AIF_TYPE::AIF_POP);
  AIF.setPIFType(mdm_AIF::PIF_TYPE::PIF_POP);
	auto model = mdm_DCEModelGenerator::createModel(AIF,
		modelType, {},
		initialParams, {}, {}, {}, {});
	

	//Compute model Ct
	int nTimes = AIF.AIF().size();
	model->computeCtModel(nTimes);
	std::vector<double> Ct = model->CtModel();

	//Write to binary file
	std::string modelFileName = outputDir + "" + modelName + ".dat";
	write_series_to_binary(modelFileName,
		Ct, initialParams);
	std::cout << "Wrote time series for "<< modelName << " to binary calibration file" << std::endl;

	//Add noise
	mdm_test_utils::add_noise(Ct, 0.001);
	modelFileName = outputDir + "" + modelName + "_noise.dat";
	write_series_to_binary(modelFileName,
		Ct, initialParams);
	std::cout << "Wrote time series with added noise for " << modelName << " to binary calibration file" << std::endl;

	if (makeIAUC)
	{

		int nIAUC = 3;
		std::vector<double> IAUCVals_(nIAUC);
		std::vector<double> IAUCTimes_ = { 60, 90, 120 };

		const auto &dynamicTimings_ = AIF.AIFTimes();
		double cumulativeCt = 0;
		const int &bolusImage = AIF.prebolus();
		const double bolusTime = dynamicTimings_[bolusImage];

		//This relies on IAUC times being sorted, which we enforce externally to save
		//time, but for robustness could do so here?
		int currIAUCt = 0;
		for (int i_t = bolusImage; i_t < nTimes; i_t++)
		{
			double elapsedTime = dynamicTimings_[i_t] - bolusTime;

			//If we exceed time for any IAUC time, set the val
			if (elapsedTime > IAUCTimes_[currIAUCt]/60)
			{
				IAUCVals_[currIAUCt] = cumulativeCt;

				//If this was the last time point, we can break
				if (currIAUCt == nIAUC - 1)
					break;
				else
					currIAUCt++;;
			}

			//Add to the cumulative Ct
			cumulativeCt += (Ct[i_t] + Ct[i_t - 1]) *
				(dynamicTimings_[i_t] - dynamicTimings_[i_t - 1]) / 2.0;
		}

		std::string iaucFileName = outputDir + "" + modelName + "_IAUC.dat";
		int nParams = initialParams.size();
		std::ofstream iaucFileStream(iaucFileName, std::ios::out | std::ios::binary);
		iaucFileStream.write(reinterpret_cast<const char*>(
			&nIAUC), sizeof(int));
		iaucFileStream.write(reinterpret_cast<const char*>(
			&IAUCTimes_[0]), sizeof(double)*nIAUC);
		iaucFileStream.write(reinterpret_cast<const char*>(
			&IAUCVals_[0]), sizeof(double)*nIAUC);
		iaucFileStream.close();
		std::cout << "Wrote IAUC values for " << modelName << " to binary calibration file" << std::endl;
	}
}

int main(int argc, char *argv[])
{
	std::string outputDir;
	if (argc > 1)
		outputDir = std::string(argv[1]);

	//Create dynamic times vector
	int nTimes = 100;
	std::vector<double> dynTimes(nTimes);
	for (int i_t = 0; i_t < nTimes; i_t++)
		dynTimes[i_t] = 5 * double(i_t) / 60;

	//Write it out to a binary file
	std::string timesFileName(outputDir + "dyn_times.dat");
	std::ofstream timesFileStream(timesFileName, std::ios::out | std::ios::binary);
	timesFileStream.write(reinterpret_cast<const char*>(
		&nTimes), sizeof(int));
	timesFileStream.write(reinterpret_cast<const char*>(
		&dynTimes[0]), sizeof(double)*nTimes);
	timesFileStream.close();
	std::cout << "Wrote dynamic times to binary calibration file" << std::endl;

	//Create population AIF
	int injectionImage = 8;
	double hct = 0.42;
	double dose = 0.1;
	mdm_AIF AIF;
	AIF.setAIFType(mdm_AIF::AIF_POP);
	AIF.setPrebolus(injectionImage);
	AIF.setHct(hct);
	AIF.setDose(dose);
	AIF.setAIFTimes(dynTimes);
	AIF.resample_AIF( 0);
	std::vector<double> aifVals = AIF.AIF();

	//Write it out to binary file
	std::string aifFileName(outputDir + "aif.dat");
	std::ofstream aifFileStream(aifFileName, std::ios::out | std::ios::binary);
	aifFileStream.write(reinterpret_cast<const char*>(
		&injectionImage), sizeof(int));
	aifFileStream.write(reinterpret_cast<const char*>(
		&hct), sizeof(double));
	aifFileStream.write(reinterpret_cast<const char*>(
		&dose), sizeof(double));
	aifFileStream.write(reinterpret_cast<const char*>(
		&aifVals[0]), sizeof(double)*nTimes);
	aifFileStream.close();
	std::cout << "Wrote AIF to binary calibration file" << std::endl;

	//Write out PIF too
	AIF.setPIFType(mdm_AIF::PIF_POP);
	AIF.resample_PIF( 0);
	std::vector<double> pifVals = AIF.PIF();
	std::string pifFileName(outputDir + "pif.dat");
	std::ofstream pifFileStream(pifFileName, std::ios::out | std::ios::binary);
	pifFileStream.write(reinterpret_cast<const char*>(
		&pifVals[0]), sizeof(double)*pifVals.size());
	pifFileStream.close();
	std::cout << "Wrote PIF to binary calibration file" << std::endl;

	//Create model concentrations for each model type
	make_model_time_series(
		outputDir,
		"ETM",
		{ 0.25, 0.2, 0.1, 0.1 },
		AIF, true);
	make_model_time_series(
		outputDir,
		"DIETM",
		{ 0.25, 0.2, 0.1, 0.8, 0.1, 0.0 },
		AIF, false);
	make_model_time_series(
		outputDir,
		"AUEM",
		{ 0.6, 0.2, 0.2, 0.1, 0.2, 0.1, 0.0 },
		AIF, false);
	make_model_time_series(
		outputDir,
		"DISCM",
		{ 0.6, 1.0, 0.2,  0.1, 0.0 },
		AIF, false);
	make_model_time_series(
		outputDir,
		"2CXM",
		{ 0.6, 0.2, 0.2, 0.2, 0.1 },
		AIF, false);
	make_model_time_series(
		outputDir,
		"DI2CXM",
		{ 0.6, 0.2, 0.2, 0.2, 0.8, 0.1, 0.0 },
		AIF, false);
	make_model_time_series(
		outputDir,
		"DIBEM",
		{ 0.2, 0.2, 0.5, 4.0, 0.5, 0.1, 0.0 },
		AIF, false);

	//Create signals from T1 and S0
	const auto PI = acos(-1.0);
	std::vector<double> FAs = { PI*2.0/180, PI*10.0 / 180, PI*18.0 / 180 };
	int nFAs = FAs.size();
	double T1 = 1500;
	double S0 = 1000;
	double TR = 3.5;
	std::vector<double> signals(nFAs);
	for (int i = 0; i < nFAs; i++)
		signals[i] = mdm_T1FitterVFA::T1toSignal(T1, S0, FAs[i], TR);

	std::string T1FileName(outputDir + "T1.dat");
	std::ofstream T1FileStream(T1FileName, std::ios::out | std::ios::binary);
	T1FileStream.write(reinterpret_cast<const char*>(
		&nFAs), sizeof(int));
	T1FileStream.write(reinterpret_cast<const char*>(
		&FAs[0]), sizeof(double)*nFAs);
	T1FileStream.write(reinterpret_cast<const char*>(
		&signals[0]), sizeof(double)*nFAs);

	T1FileStream.write(reinterpret_cast<const char*>(
		&T1), sizeof(double));
	T1FileStream.write(reinterpret_cast<const char*>(
		&S0), sizeof(double));
	T1FileStream.write(reinterpret_cast<const char*>(
		&TR), sizeof(double));

	T1FileStream.close();
	std::cout << "Wrote T1 data to binary calibration file" << std::endl;
}