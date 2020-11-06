/**
*  @file    mdm_RunTools_madym_DCE_lite.cxx
*  @brief   Implementation of mdm_RunTools_madym_DCE_lite class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_DCE_lite.h"

#include <madym/mdm_ProgramLogger.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_DCE_lite::mdm_RunTools_madym_DCE_lite(mdm_InputOptions &options_, mdm_OptionsParser &options_parser)
	: 
	mdm_RunToolsDCEFit(), 
	mdm_RunTools(options_, options_parser)
{
}


MDM_API mdm_RunTools_madym_DCE_lite::~mdm_RunTools_madym_DCE_lite()
{
}

//
MDM_API int mdm_RunTools_madym_DCE_lite::run()
{
	//Check required inputs
	if (options_.model().empty())
	{
		mdm_progAbort("model (option -m) must be provided");
	}
	if (options_.inputDataFile().empty())
	{
		mdm_progAbort("input data file (option -i) must be provided");
	}
	if (!options_.nDyns())
	{
		mdm_progAbort("number of dynamics (option -n) must be provided");
	}

	//Set-up output folder and output file
	set_up_output_folder();
	std::string outputDataFile = outputPath_.string() + "/" +
		options_.model() + "_" + options_.outputName();

	
 //Set which type of model we're using
	setModel(options_.model(),
		!options_.aifName().empty(), !options_.pifName().empty(),
		options_.paramNames(), options_.initialParams(),
		options_.fixedParams(), options_.fixedValues(),
		options_.relativeLimitParams(), options_.relativeLimitValues());
	AIF_.setPrebolus(options_.injectionImage());
	AIF_.setHct(options_.hct());
	AIF_.setDose(options_.dose());

	//If we're using an auto AIF read from file, it will use the times encoded in the file
	//but if we're using a population AIF we need to read these times and set them in the AIF object
	if (options_.aifName().empty())
	{
		//Using population AIF
		if (options_.dynTimesFile().empty())
		{
			mdm_progAbort("if not using an auto-AIF, a dynamic times file must be provided");
		}
		//Try and open the file and read in the times
		std::ifstream dynTimesStream(options_.dynTimesFile(), std::ios::in);
		if (!dynTimesStream.is_open())
		{
			mdm_progAbort("error opening dynamic times file, Check it exists");
		}
		std::vector<double> dynamicTimes;
		for (int i = 0; i < options_.nDyns(); i++)
		{
			double t;
			dynTimesStream >> t;
			dynamicTimes.push_back(t);
		}
		AIF_.setAIFTimes(dynamicTimes);
	}
	else //If we're using an auto-generated AIF, read it from file now
	{
		std::string aifPath = fs::absolute(options_.aifName()).string();
		if (aifPath.empty())
		{
			mdm_progAbort(options_.model() + " chosen as model but no AIF filename set");
		}
		if (!AIF_.readAIF(aifPath, options_.nDyns()))
		{
			mdm_progAbort("error loading AIF for model " + options_.model());
		}
	}

	//If we're using an auto-generated PIF, read it from file now
	if (!options_.pifName().empty())
	{
		std::string pifPath = fs::absolute(options_.pifName()).string();
		if (pifPath.empty())
		{
			mdm_progAbort(options_.model() + " chosen as model but no AIF filename set");
		}
		if (!AIF_.readPIF(pifPath, options_.nDyns()))
		{
			mdm_progAbort("error loading AIF for model " + options_.model());
		}
	}

	//If we're converting from signal to concentration, make sure we've been supplied TR and FA values
	if (!options_.inputCt() && (!options_.TR() || !options_.FA() || !options_.r1Const()))
	{
		mdm_progAbort("TR, FA, r1 must be set to convert from signal concentration");
	}

	//Open the input data file
	std::ifstream inputData(options_.inputDataFile(), std::ios::in);
	if (!inputData.is_open())
	{
		mdm_progAbort("error opening input data file, Check it exists");
	}

	//Open up an output file
	std::ofstream outputData(outputDataFile, std::ios::out);
	if (!outputData.is_open())
	{
		mdm_progAbort("error opening ouput data file");
	}

	//If we've been given a initial parameters for every time-series open stream to file
	//containing these
	std::ifstream inputParams;
	bool load_params = false;
	if (!options_.initParamsFile().empty())
	{
		inputParams.open(options_.initParamsFile(), std::ios::in);
		if (!inputParams.is_open())
		{
			mdm_progAbort("error opening input parameter file, Check it exists");
		}
		load_params = true;
	}

	//Check if we've been given a file defining varying dynamic noise
	std::vector<double> noiseVar;
	if (!options_.dynNoiseFile().empty())
	{
		//Try and open the file and read in the noise values
		std::ifstream dynNoiseStream(options_.dynNoiseFile(), std::ios::in);
		if (!dynNoiseStream.is_open())
		{
			mdm_progAbort("error opening dynamic times file, Check it exists");
		}
		for (int i = 0; i < options_.nDyns(); i++)
		{
			double sigma;
			dynNoiseStream >> sigma;
			noiseVar.push_back(sigma);
		}
	}

	//Set valid first image
	if (options_.lastImage() <= 0)
		options_.lastImage.set(options_.nDyns());

	//Convert IAUC times to minutes
	auto iauc_t = options_.IAUCTimes();
	std::sort(iauc_t.begin(), iauc_t.end());
	for (auto &t : iauc_t)
		t /= 60;

	std::vector<double> ts(options_.nDyns(), 0);

	double d;
	double t10 = 0.0;
	double s0 = 0.0;
	int col_counter = 0;
	int row_counter = 0;

	int col_length = options_.nDyns();
	if (!options_.inputCt())
	{
		col_length++;
		if (!options_.M0Ratio())
			col_length++;
	}

	//Loop through the file, reading in each line
	while (true)
	{
		if (col_counter == col_length)
		{
			//Check if input parameters to read
			if (load_params)
			{
				int n = model_->num_params();
				std::vector<double> initialParams(n, 0);
				double vox;
				for (int i = 0; i < n; i++)
				{
					inputParams >> vox;
					initialParams[i] = vox;
				}
				model_->setInitialParams(initialParams);
			}

			//Fit the series
			fit_series(outputData, ts, options_.inputCt(),
				noiseVar,
				t10, s0,
				options_.r1Const(),
				options_.TR(),
				options_.FA(),
				options_.firstImage(),
				options_.lastImage(),
				options_.testEnhancement(),
				options_.M0Ratio(),
				iauc_t,
				options_.outputCt_mod(),
				options_.outputCt_sig(),
				!options_.noOptimise(),
				options_.maxIterations());

			col_counter = 0;

			row_counter++;
			if (!fmod(row_counter, 1000))
				std::cout << "Processed time-series " << row_counter << std::endl;
		}
		else
		{
			inputData >> d;

			if (col_counter == options_.nDyns())
				t10 = d;

			else if (col_counter == options_.nDyns() + 1)
				s0 = d;

			else //col_counter < nDyns()
				ts[col_counter] = d;

			col_counter++;

			if (inputData.eof())
				break;
		}
	}

	//Close the input and output file
	inputData.close();
	outputData.close();
	if (load_params)
		inputParams.close();

	std::cout << "Finished processing! " << std::endl;
	std::cout << "Processed " << row_counter << " time-series in total." << std::endl;

	//Tidy up the logging objects
	return mdm_progExit();
}

//
MDM_API int mdm_RunTools_madym_DCE_lite::parseInputs(int argc, const char *argv[])
{
	po::options_description config_options("madym-lite config options_");
	
	options_parser_.add_option(config_options, options_.dataDir);

		//DCE input options_
	options_parser_.add_option(config_options, options_.inputDataFile);
	options_parser_.add_option(config_options, options_.inputCt);
	options_parser_.add_option(config_options, options_.dynTimesFile);
	options_parser_.add_option(config_options, options_.nDyns);
	options_parser_.add_option(config_options, options_.injectionImage);

		//Signal to concentration options_
	options_parser_.add_option(config_options, options_.M0Ratio);
	options_parser_.add_option(config_options, options_.r1Const);
	options_parser_.add_option(config_options, options_.FA);
	options_parser_.add_option(config_options, options_.TR);

		//AIF options_
	options_parser_.add_option(config_options, options_.aifName);
	options_parser_.add_option(config_options, options_.pifName);
	options_parser_.add_option(config_options, options_.dose);
	options_parser_.add_option(config_options, options_.hct);

		//Model options_
	options_parser_.add_option(config_options, options_.model);
	options_parser_.add_option(config_options, options_.initialParams);
	options_parser_.add_option(config_options, options_.initParamsFile);
	options_parser_.add_option(config_options, options_.paramNames);
	options_parser_.add_option(config_options, options_.fixedParams);
	options_parser_.add_option(config_options, options_.fixedValues);
	options_parser_.add_option(config_options, options_.relativeLimitParams);
	options_parser_.add_option(config_options, options_.relativeLimitValues);
	options_parser_.add_option(config_options, options_.firstImage);
	options_parser_.add_option(config_options, options_.lastImage);

	options_parser_.add_option(config_options, options_.noOptimise);
	options_parser_.add_option(config_options, options_.dynNoiseFile);
	options_parser_.add_option(config_options, options_.testEnhancement);
	options_parser_.add_option(config_options, options_.maxIterations);

		//DCE only output options_
	options_parser_.add_option(config_options, options_.outputCt_sig);
	options_parser_.add_option(config_options, options_.outputCt_mod);
	options_parser_.add_option(config_options, options_.IAUCTimes);

		//General output options_
	options_parser_.add_option(config_options, options_.outputName);
	options_parser_.add_option(config_options, options_.outputDir);

	//Set overwrite default to true for lite version
	options_.overwrite.set(true);

	return options_parser_.parseInputs(
		config_options,
		argc, argv);
}

//*******************************************************************************
// Private:
//*******************************************************************************
void mdm_RunTools_madym_DCE_lite::fit_series(std::ostream &outputData,
	const std::vector<double> &ts, const bool &inputCt,
	const std::vector<double> &noiseVar,
	const double &t10, const double &s0,
	const double &r1,
	const double &TR,
	const double & FA,
	const int &firstImage,
	const int &lastImage,
	const bool&testEnhancement,
	const bool&useM0Ratio,
	const std::vector<double> &IAUCTimes,
	const bool &outputCt_mod,
	const bool &outputCt_sig,
	const bool &optimiseModel,
	const int &maxIterations)
{
	std::vector<double> signalData;
	std::vector<double> CtData;

	if (inputCt)
		CtData = ts;
	else
		signalData = ts;

	const int nDyns = ts.size();

	//Create a perm object
	mdm_DCEVoxel vox(
		*model_,
		signalData,
		CtData,
		noiseVar,
		t10,
		s0,
		r1,
		model_->AIF().prebolus(),
		AIF_.AIFTimes(),
		TR,
		FA,
		firstImage,
		lastImage,
		testEnhancement,
		useM0Ratio,
		IAUCTimes,
		maxIterations);
	vox.initialiseModelFit();

	vox.computeIAUC();

	if (optimiseModel)
		vox.fitModel();

	//Now write the output
	outputData <<
		vox.status() << " " <<
		vox.enhancing() << " " <<
		vox.modelFitError() << " ";

	for (int i = 0; i < IAUCTimes.size(); i++)
		outputData << " " << vox.IAUC_val(i);

	for (int i = 0; i < model_->num_params(); i++)
		outputData << " " << model_->params(i);


	if (outputCt_mod)
	{
		const std::vector<double> &Cm_t = vox.CtModel();
		for (int i = 0; i < nDyns; i++)
			outputData << " " << Cm_t[i];
	}

	if (outputCt_sig)
	{
		const std::vector<double> &Cs_t = vox.CtData();
		for (int i = 0; i < nDyns; i++)
			outputData << " " << Cs_t[i];
	}

	outputData << std::endl;
}