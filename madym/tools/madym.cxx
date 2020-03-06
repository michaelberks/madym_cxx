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
	//Set up command line options using vul_args
	vul_arg_base::set_help_option("-h");

	vul_arg<std::string> configFile("-config", madym_options_.configFileText.c_str(), madym_options_.configFile);
	vul_arg<std::string> dataDir("-cwd", madym_options_.dataDirText.c_str(), madym_options_.dataDir);

	vul_arg<std::string> model("-m", madym_options_.modelText.c_str(), madym_options_.model);
	vul_arg<std::string> outputDir("-o", madym_options_.outputDirText.c_str(), madym_options_.outputDir);
	vul_arg<bool> noOptimise("-no_opt", madym_options_.noOptimiseText.c_str(), madym_options_.noOptimise);

	vul_arg<std::vector<std::string>> FANames("-vfa", madym_options_.T1inputNamesText.c_str(), madym_options_.T1inputNames);
	vul_arg<std::string> dynName("-dyn", madym_options_.dynNameText.c_str(), madym_options_.dynName);
	vul_arg<int> nDyns("-n", madym_options_.nDynsText.c_str(), madym_options_.nDyns);
	vul_arg<bool> inputCt("-Cin", madym_options_.inputCtText.c_str(), madym_options_.inputCt);
	vul_arg<bool> outputCt("-Cout", madym_options_.outputCtText.c_str(), madym_options_.outputCt);
	vul_arg<bool> outputCm("-Cmod", madym_options_.outputCmText.c_str(), madym_options_.outputCm);
	vul_arg<bool> sparseWrite("-sparse", madym_options_.sparseWriteText.c_str(), madym_options_.sparseWrite);

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

	vul_arg<std::string> programLogName("-log", madym_options_.programLogNameText.c_str(),
		madym_options_.programLogName);
	vul_arg<std::string> outputConfigFileName("-config_name", madym_options_.outputConfigFileNameText.c_str(),
		madym_options_.outputConfigFileName);
	vul_arg<std::string> auditLogBaseName("-audit", madym_options_.auditLogBaseNameText.c_str(),
		madym_options_.auditLogBaseName);
	vul_arg<std::string> auditLogDir("-audit_dir", madym_options_.auditLogDirText.c_str(),
		madym_options_.auditLogDir);
	vul_arg<std::string> errorCodesName("-E", madym_options_.errorCodesNameText.c_str(),
		madym_options_.errorCodesName);

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

	//Check if any input, if not, then display usage and quit, otherwise
	//take copy of input command line for our records
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

	//Now we can parse args
	vul_arg_parse(argc, argv);

	//Check for help or version flags
	if (help())
		vul_arg_display_usage_and_exit();

	if (version())
	{
		std::cout << MDM_VERSION;
		return 0;
	}

	//Check if configFile supplied, if so, load it to fill madym_options
	if (!configFile().empty())
	{
		if (!madym_options_.from_file(configFile()))
		{
			std::cerr << "Unable to read config file " << configFile() << std::endl;
			return 1;
		}
	}

	//Now check which, if any, command line args were set, and use this to override the config
	if (dataDir.set())
		madym_options_.dataDir = dataDir();
	if (model.set())
		madym_options_.model = model();
	if (outputDir.set())
		madym_options_.outputDir = outputDir();
	if (noOptimise.set())
		madym_options_.noOptimise = noOptimise();

	if (FANames.set())
		madym_options_.T1inputNames = FANames();
	if (dynName.set())
		madym_options_.dynName = dynName();
	if (nDyns.set())
		madym_options_.nDyns = nDyns();
	if (inputCt.set())
		madym_options_.inputCt = inputCt();
	if (outputCt.set())
		madym_options_.outputCt = outputCt();
	if (outputCm.set())
		madym_options_.outputCm = outputCm();
	if (sparseWrite.set())
		madym_options_.sparseWrite = sparseWrite();

	if (T1Name.set())
		madym_options_.T1Name = T1Name();
	if (S0Name.set())
		madym_options_.S0Name = S0Name();
	if (useRatio.set())
		madym_options_.useRatio = useRatio();

	if (hct.set())
		madym_options_.hct = hct();
	if (r1Const.set())
		madym_options_.r1Const = r1Const();
	if (dose.set())
		madym_options_.dose = dose();
	if (T1noiseThresh.set())
		madym_options_.T1noiseThresh = T1noiseThresh();

	if (injectionImage.set())
		madym_options_.injectionImage = injectionImage();
	if (firstImage.set())
		madym_options_.firstImage = firstImage();
	if (lastImage.set())
		madym_options_.lastImage = lastImage();

	if (roiName.set())
		madym_options_.roiName = roiName();
	if (aifName.set())
		madym_options_.aifName = aifName();
	if (pifName.set())
		madym_options_.pifName = pifName();

	if (programLogName.set())
		madym_options_.programLogName = programLogName();
	if (outputConfigFileName.set())
		madym_options_.outputConfigFileName = outputConfigFileName();
	if (auditLogBaseName.set())
		madym_options_.auditLogBaseName = auditLogBaseName();
	if (auditLogDir.set())
		madym_options_.auditLogDir = auditLogDir();
	if (errorCodesName.set())
		madym_options_.errorCodesName = errorCodesName();

	if (IAUCTimes.set())
		madym_options_.IAUCTimes = IAUCTimes();
	if (initParams.set())
		madym_options_.initParams = initParams();
	if (initMapsDir.set())
		madym_options_.initMapsDir = initMapsDir();
	if (paramNames.set())
		madym_options_.paramNames = paramNames();
	if (fixedParams.set())
		madym_options_.fixedParams = fixedParams();
	if (fixedValues.set())
		madym_options_.fixedValues = fixedValues();

	if (dynNoise.set())
		madym_options_.dynNoise = dynNoise();
	if (enhFlag.set())
		madym_options_.enhFlag = enhFlag();
	if (overwrite.set())
		madym_options_.overwrite = overwrite();

	
	//Instantiate new madym_exe object with these options and run
	mdm_RunTools madym_exe(madym_options_);
	return madym_exe.run_DCEFit(exeArgs, "madym");

}
