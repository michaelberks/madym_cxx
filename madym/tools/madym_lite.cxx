/**
 *  @file    madym_lite.cxx
 *  @brief   Main program for lite-weight madym version
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#include "mdm_version.h"

#include <mdm_RunTools.h>
#include <vul/vul_arg.h>
#include <madym/mdm_vul_arg.h>
#include <mdm_AnalyzeFormat.h>
mdm_ToolsOptions madym_options_;

/******************************************************************************************
 *       Model fitting                                                                    *
 ******************************************************************************************/

/**
 * Main program based on command-line input.
 *
 * @brief    madym's command-line main program
 * @param    argc   Number of command-line parameters
 * @param    argv   String array of command-line parameters
 * @return   int    Standard C++ return status flag
 */
int main(int argc, char *argv[])
{
	/* If cmd-line looks ok save for log otherwise advise user */
  vul_arg_base::set_help_option("-h");

	vul_arg<std::string> dataDir("-cwd", madym_options_.dataDirText.c_str(), madym_options_.dataDir);

	vul_arg<std::string> model("-m", madym_options_.modelText.c_str(), madym_options_.model);
  vul_arg<std::string> inputDataFile("-d", madym_options_.inputDataFileText.c_str(), madym_options_.inputDataFile);
  vul_arg<int> nDyns("-n", madym_options_.nDynsText.c_str(), madym_options_.nDyns);
	vul_arg<std::string> outputDir("-o", madym_options_.outputDirText.c_str(), madym_options_.outputDir);
  vul_arg<std::string> outputName("-O", madym_options_.outputNameText.c_str(), madym_options_.outputName);

  vul_arg<std::string> dynTimesFile("-t", madym_options_.dynTimesFileText.c_str(), madym_options_.dynTimesFile);

	vul_arg<bool> inputCt("-Cin", madym_options_.inputCtText.c_str(), madym_options_.inputCt);
	vul_arg<bool> outputCt("-Cout", madym_options_.outputCtText.c_str(), madym_options_.outputCt);
	vul_arg<bool> outputCm("-Cmod", madym_options_.outputCmText.c_str(), madym_options_.outputCm);
	vul_arg<bool> noOptimise("-no_opt", madym_options_.noOptimiseText.c_str(), madym_options_.noOptimise);

	vul_arg<bool> useRatio("-useRatio", madym_options_.useRatioText.c_str(), madym_options_.useRatio);
	vul_arg<double> hct("-H", madym_options_.hctText.c_str(), madym_options_.hct);
	vul_arg<double> dose("-D", madym_options_.doseText.c_str(), madym_options_.dose);

  vul_arg<double> TR("-TR", madym_options_.TRText.c_str(), madym_options_.TR);
  vul_arg<double> FA("-FA", madym_options_.FAText.c_str(), madym_options_.FA);
	vul_arg<double> r1Const("-r1", madym_options_.r1ConstText.c_str(), madym_options_.r1Const);

	vul_arg<int> injectionImage("-i", madym_options_.injectionImageText.c_str(), madym_options_.injectionImage);
	vul_arg<int> firstImage("-first", madym_options_.firstImageText.c_str(), madym_options_.firstImage);
	vul_arg<int> lastImage("-last", madym_options_.lastImageText.c_str(), madym_options_.lastImage);

	vul_arg<std::string> aifName("-aif", madym_options_.aifNameText.c_str(), madym_options_.aifName);
	vul_arg<std::string> pifName("-pif", madym_options_.pifNameText.c_str(), madym_options_.pifName);

  vul_arg<std::vector<double>> IAUCTimes("-iauc", madym_options_.IAUCTimesText.c_str(), madym_options_.IAUCTimes);
  vul_arg<std::vector<double>> initParams("-init_params", madym_options_.initParamsText.c_str(), madym_options_.initParams);
  vul_arg<std::string> initParamsFile("-init_file", madym_options_.initParamsFileText.c_str(),
		madym_options_.initParamsFile);
	vul_arg<std::vector<std::string>> paramNames("-param_names", madym_options_.paramNamesText.c_str(), madym_options_.paramNames);
	vul_arg<std::vector<int>> fixedParams("-fixed_params", madym_options_.fixedParamsText.c_str(), madym_options_.fixedParams);
	vul_arg<std::vector<double>> fixedValues("-fixed_values", madym_options_.fixedValuesText.c_str(), madym_options_.fixedValues);
	vul_arg<std::vector<int>> relativeLimitParams("-relative_limit_params", madym_options_.relativeLimitParamsText.c_str(), madym_options_.fixedParams);
	vul_arg<std::vector<double>> relativeLimitValues("-relative_limit_values", madym_options_.relativeLimitValuesText.c_str(), madym_options_.fixedValues);

  vul_arg<std::string> dynNoiseFile("-dyn_noise", madym_options_.dynNoiseFileText.c_str(), madym_options_.dynNoiseFile);
	vul_arg<bool> noEnhFlag("-no_enh", madym_options_.noEnhFlagText.c_str(), madym_options_.noEnhFlag);
	vul_arg<bool> help("-help", madym_options_.helpText.c_str(), madym_options_.help);
	vul_arg<bool> version("-version", madym_options_.versionText.c_str(), madym_options_.version);

  //Check if we have args to process
	std::string exeArgs;
	if (argc > 1)
	{
		exeArgs = std::string(argv[0]);
		for (int i = 1; i < argc; i++)
		{
			exeArgs += " " + std::string(argv[i]);
		}
		exeArgs += "\n";
	}
	else
		vul_arg_display_usage_and_exit();

  vul_arg_parse(argc, argv);

  if (help())
    vul_arg_display_usage_and_exit();

  if (version())
  {
    std::cout << MDM_VERSION;
    return 0;
  }

	//Set options from command line inputs
	madym_options_.dataDir = dataDir();
	madym_options_.model = model();
	madym_options_.inputDataFile = inputDataFile();
	madym_options_.nDyns = nDyns();
	madym_options_.outputDir = outputDir();
	madym_options_.outputName = outputName();
	madym_options_.dynTimesFile = dynTimesFile();
	madym_options_.inputCt = inputCt();
	madym_options_.outputCt = outputCt();
	madym_options_.outputCm = outputCm();
	madym_options_.noOptimise = noOptimise();
	madym_options_.useRatio = useRatio();
	madym_options_.hct = hct();
	madym_options_.dose = dose();
	madym_options_.TR = TR();
	madym_options_.FA = FA();
	madym_options_.r1Const = r1Const();
	madym_options_.injectionImage = injectionImage();
	madym_options_.firstImage = firstImage();
	madym_options_.lastImage = lastImage();
	madym_options_.aifName = aifName();
	madym_options_.pifName = pifName();
	madym_options_.IAUCTimes = IAUCTimes();
	madym_options_.initParams = initParams();
	madym_options_.initParamsFile = initParamsFile();
	madym_options_.paramNames = paramNames();
	madym_options_.fixedParams = fixedParams();
	madym_options_.fixedValues = fixedValues();
	madym_options_.relativeLimitParams = relativeLimitParams();
	madym_options_.relativeLimitValues = relativeLimitValues();
	madym_options_.dynNoiseFile = dynNoiseFile();
	madym_options_.noEnhFlag = noEnhFlag();

	//Instantiate new madym_exe object with these options and run
	mdm_RunTools madym_exe(madym_options_);
	return madym_exe.run_DCEFit_lite(exeArgs, "madym_lite");
}
