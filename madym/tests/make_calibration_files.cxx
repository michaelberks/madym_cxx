#include <iostream>
#include <ostream>

#include <madym/mdm_AIF.h>
#include <madym/mdm_DCEModelGenerator.h>
#include <madym/mdm_T1Voxel.h>

void make_model_time_series(
	const std::string &modelName,
	const std::vector<double> &initParams,
	mdm_AIF &AIF)
{
	mdm_DCEModelBase *model = NULL;
	bool model_set = mdm_DCEModelGenerator::setModel(model, AIF,
		modelName, false, false, {},
		initParams, {}, {});

	if (!model_set)
	{
		std::cout << "Unable to write time series for " << modelName << std::endl;
		return;
	}

	int nTimes = AIF.AIF().size();
	model->computeCtModel(nTimes);
	std::vector<double> Ct = model->CtModel();

	std::string modelFileName = "./" + modelName + ".dat";
	int nParams = initParams.size();
	std::ofstream modelFileStream(modelFileName, std::ios::out | std::ios::binary);
	modelFileStream.write(reinterpret_cast<const char*>(
		&nParams), sizeof(int));
	modelFileStream.write(reinterpret_cast<const char*>(
		&initParams[0]), sizeof(double)*nParams);
	modelFileStream.write(reinterpret_cast<const char*>(
		&Ct[0]), sizeof(double)*nTimes);
	modelFileStream.close();
	std::cout << "Wrote time series for "<< modelName << " to binary calibration file" << std::endl;
	delete model;
}

int main(int argc, char *argv[])
{
	//Create dynamic times vector
	int nTimes = 100;
	std::vector<double> dynTimes(nTimes);
	for (int i_t = 0; i_t < nTimes; i_t++)
		dynTimes[i_t] = 5 * double(i_t) / 60;

	//Write it out to a binary file
	std::string timesFileName("./dyn_times.dat");
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
	AIF.setAIFflag(mdm_AIF::AIF_POP);
	AIF.setPrebolus(injectionImage);
	AIF.setHct(hct);
	AIF.setDose(dose);
	AIF.setAIFTimes(dynTimes);
	AIF.resample_AIF(nTimes, 0);
	std::vector<double> aifVals = AIF.AIF();

	//Write it out to binary file
	std::string aifFileName("./aif.dat");
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
	AIF.setPIFflag(mdm_AIF::PIF_POP);
	AIF.resample_PIF(nTimes, 0);
	std::vector<double> pifVals = AIF.PIF();
	std::string pifFileName("./pif.dat");
	std::ofstream pifFileStream(pifFileName, std::ios::out | std::ios::binary);
	pifFileStream.write(reinterpret_cast<const char*>(
		&pifVals[0]), sizeof(double)*pifVals.size());
	pifFileStream.close();
	std::cout << "Wrote PIF to binary calibration file" << std::endl;

	//Create model concentrations for each model type
	make_model_time_series(
		"ETM",
		{ 0.25, 0.2, 0.1, 0.1 },
		AIF);
	make_model_time_series(
		"DIETM",
		{ 0.25, 0.2, 0.1, 0.8, 0.1, 0.0 },
		AIF);
	make_model_time_series(
		"AUEM",
		{ 0.6, 0.2, 0.2, 0.1, 0.2, 0.1, 0.0 },
		AIF);
	make_model_time_series(
		"DISCM",
		{ 0.6, 1.0, 0.2,  0.1, 0.0 },
		AIF);
	make_model_time_series(
		"2CXM",
		{ 0.6, 0.2, 0.2, 0.2, 0.1 },
		AIF);
	make_model_time_series(
		"DI2CXM",
		{ 0.6, 0.2, 0.2, 0.2, 0.8, 0.1, 0.0 },
		AIF);
	make_model_time_series(
		"DIBEM",
		{ 0.2, 0.2, 0.5, 4.0, 0.5, 0.1, 0.0 },
		AIF);

	//Create signals from T1 and S0
	const auto PI = acos(-1.0);
	std::vector<double> FAs = { PI*2.0/180, PI*10.0 / 180, PI*18.0 / 180 };
	int nFAs = FAs.size();
	double T1 = 1500;
	double S0 = 1000;
	double TR = 3.5;
	std::vector<double> signals(nFAs);
	for (int i = 0; i < nFAs; i++)
		signals[i] = mdm_T1Voxel::T1toSignal(T1, S0, FAs[i], TR);

	std::string T1FileName("./T1.dat");
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