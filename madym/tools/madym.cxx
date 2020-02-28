/**
* Main program based on command-line input.
*
* @brief    madym's command-line main program*  @details More info...
* @author MA Berks(c) Copyright QBI Lab, University of Manchester 2020
*/

#include "mdm_version.h"

#include <mdm_RunTools.h>
#include <vul/vul_arg.h>
#include <madym/mdm_vul_arg.h>

mdm_ToolsOptions madym_options_;

/******************************************************************************************
*       Model fitting                                                                    *
******************************************************************************************/

/**
* Main program based on command-line input.
*
* @brief    madym's command-line main program
* @version  madym 1.22
* @param    argc   Number of command-line parameters
* @param    argv   String array of command-line parameters
* @return   int    Standard C return status flag
*/
int main(int argc, char *argv[])
{
	/* If cmd-line looks ok save for log otherwise advise user */
	vul_arg_base::set_help_option("-h");

	vul_arg<std::string> model("-m", madym_options_.modelText.c_str(), madym_options_.model);
	vul_arg<std::string> outputDir("-o", madym_options_.outputDirText.c_str(), madym_options_.outputDir);
  vul_arg<bool> noOptimise("-no_opt", madym_options_.noOptimiseText.c_str(), madym_options_.noOptimise);

	vul_arg<std::vector<std::string>> FANames("-vfa", madym_options_.T1inputNamesText.c_str(), madym_options_.T1inputNames);
	vul_arg<std::string> dynName("-dyn", madym_options_.dynNameText.c_str(), madym_options_.dynName);
	vul_arg<bool> inputCt("-Cin", madym_options_.inputCtText.c_str(), madym_options_.inputCt);
	vul_arg<bool> outputCt("-Cout", madym_options_.outputCtText.c_str(), madym_options_.outputCt);
  vul_arg<bool> outputCm("-Cmod", madym_options_.outputCmText.c_str(), madym_options_.outputCm);

	vul_arg<std::string> T1Name("-T1", madym_options_.T1NameText.c_str(), madym_options_.T1Name);
	vul_arg<std::string> S0Name("-S0", madym_options_.S0NameText.c_str(), madym_options_.S0Name);
	vul_arg<bool> useRatio("-useRatio", madym_options_.useRatioText.c_str(), madym_options_.useRatio);
  vul_arg<double> T1noiseThresh("-T1noise", madym_options_.T1noiseThreshText.c_str(), madym_options_.T1noiseThresh);
	
	vul_arg<double> hct("-H", madym_options_.hctText.c_str(), madym_options_.hct);
	vul_arg<double> r1Const("-r1", madym_options_.r1ConstText.c_str(), madym_options_.r1Const);
	vul_arg<double> dose("-D", madym_options_.doseText.c_str(), madym_options_.dose);
	
  vul_arg<int> injectionImage("-i", madym_options_.injectionImageText.c_str(), madym_options_.injectionImage);
  vul_arg<int> firstImage("-first", madym_options_.firstImageText.c_str(), madym_options_.firstImage);
  vul_arg<int> lastImage("-last", madym_options_.lastImageText.c_str(), madym_options_.lastImage);

	vul_arg<std::string> roiName("-roi", madym_options_.roiNameText.c_str(), madym_options_.roiName);
	vul_arg<std::string> aifName("-aif", madym_options_.aifNameText.c_str(), madym_options_.aifName);
	vul_arg<std::string> pifName("-pif", madym_options_.pifNameText.c_str(), madym_options_.pifName);

  vul_arg<std::string> programLogBaseName("-log", madym_options_.programLogBaseNameText.c_str(), 
		madym_options_.programLogBaseName);
  vul_arg<std::string> auditLogBaseName("-audit", madym_options_.auditLogBaseNameText.c_str(), 
		madym_options_.auditLogBaseName);
	vul_arg<std::string> auditLogDir("-audit_dir", madym_options_.auditLogDirText.c_str(),
		madym_options_.auditLogDir);
  vul_arg<std::string> errorBaseName("-E", madym_options_.errorBaseNameText.c_str(), 
		madym_options_.errorBaseName);

	vul_arg<std::vector<double>> IAUCTimes("-iauc", madym_options_.IAUCTimesText.c_str(), madym_options_.IAUCTimes);
	vul_arg<std::vector<double>> initParams("-init_params", madym_options_.initParamsText.c_str(), madym_options_.initParams);
  vul_arg<std::string> initMapsDir("-init_maps", madym_options_.initMapsDirText.c_str(), madym_options_.initMapsDir);
	vul_arg<std::vector<std::string>> paramNames("-param_names", madym_options_.paramNamesText.c_str(), madym_options_.paramNames);
	vul_arg<std::vector<int>> fixedParams("-fixed_params", madym_options_.fixedParamsText.c_str(), madym_options_.fixedParams);
  vul_arg<std::vector<double>> fixedValues("-fixed_values", madym_options_.fixedValuesText.c_str(), madym_options_.fixedValues);

  vul_arg<bool> dynNoise("-dyn_noise", madym_options_.dynNoiseText.c_str(), madym_options_.dynNoise);
	vul_arg<bool> enhFlag("-enh", madym_options_.enhFlagText.c_str(), madym_options_.enhFlag);
	vul_arg<bool> overwrite("-overwrite", madym_options_.overwriteText.c_str(), madym_options_.overwrite);
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

  //Parse arguments
	vul_arg_parse(argc, argv);

  //Check for help or version flags
  if (help())
    vul_arg_display_usage_and_exit();

  if (version())
  {
    std::cout << MDM_VERSION;
    return 0;
  }

  //Try and run main madym executable - set options object from vul_arg inputs
  madym_options_.model = model();
  madym_options_.outputDir = outputDir();
  madym_options_.noOptimise = noOptimise();

  madym_options_.T1inputNames = FANames();
  madym_options_.dynName = dynName();
  madym_options_.inputCt = inputCt();
  madym_options_.outputCt = outputCt();
  madym_options_.outputCm = outputCm();

  madym_options_.T1Name = T1Name();
  madym_options_.S0Name = S0Name();
  madym_options_.useRatio = useRatio();

  madym_options_.hct = hct();
  madym_options_.r1Const = r1Const();
  madym_options_.dose = dose();
  madym_options_.T1noiseThresh = T1noiseThresh();

  madym_options_.injectionImage = injectionImage();
  madym_options_.firstImage = firstImage();
  madym_options_.lastImage = lastImage();

  madym_options_.roiName = roiName();
  madym_options_.aifName = aifName();
  madym_options_.pifName = pifName();

  madym_options_.programLogBaseName = programLogBaseName();
  madym_options_.auditLogBaseName = auditLogBaseName();
	madym_options_.auditLogDir = auditLogDir();
  madym_options_.errorBaseName = errorBaseName();

  madym_options_.IAUCTimes = IAUCTimes();
  madym_options_.initParams = initParams();
  madym_options_.initMapsDir = initMapsDir();
  madym_options_.paramNames = paramNames();
  madym_options_.fixedParams = fixedParams();
  madym_options_.fixedValues = fixedValues();

  madym_options_.dynNoise = dynNoise();
  madym_options_.enhFlag = enhFlag();
  madym_options_.overwrite = overwrite();

  //Instantiate new madym_exe object with these options and run
  mdm_RunTools madym_exe(madym_options_);
  return madym_exe.run_DCEFit(exeArgs, "madym");
}
