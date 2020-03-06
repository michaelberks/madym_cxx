/**
*
* Standalone T1 calculator, reads in analyze format image volumes
* and computes a volumetric map of T1 values. The aim is to
* support all commonly used methods for calculating T1, however
* currently only the variable flip-angle method is implemented.
*
* @author   MA Berks (c) Copyright QBI Lab, University of Manchester 2020
* @brief    Standalone T1 mapper, takes input images from disk
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
*/
int main(int argc, char *argv[])
{
  /* If cmd-line looks ok save for log otherwise advise user */
  vul_arg_base::set_help_option("-h");

	vul_arg<std::string> configFile("-config", madym_options_.configFileText.c_str(), madym_options_.configFile);
	vul_arg<std::string> dataDir("-cwd", madym_options_.dataDirText.c_str(), madym_options_.dataDir);

  vul_arg<std::vector<std::string>> mapNames("-maps", madym_options_.T1inputNamesText.c_str(), madym_options_.T1inputNames);
	
	vul_arg<std::string> outputDir("-o", madym_options_.outputDirText.c_str(), madym_options_.outputDir);

  vul_arg<std::string> method("-m", madym_options_.T1methodText.c_str(), madym_options_.T1method);
	vul_arg<double> noiseThresh("-noise", madym_options_.T1noiseThreshText.c_str(), madym_options_.T1noiseThresh);

	vul_arg<std::string> roiName("-roi", madym_options_.roiNameText.c_str(), madym_options_.roiName);

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

  vul_arg_parse(argc, argv);

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
	if (outputDir.set())
		madym_options_.outputDir = outputDir();
	if (mapNames.set())
		madym_options_.T1inputNames = mapNames();

	if (method.set())
		madym_options_.T1method = method();
	if (noiseThresh.set())
		madym_options_.T1noiseThresh = noiseThresh();
	if (roiName.set())
		madym_options_.roiName = roiName();

	if (programLogName.set())
		madym_options_.programLogName = programLogName();
	if (outputConfigFileName.set())
		madym_options_.outputConfigFileName = outputConfigFileName();
	if (auditLogBaseName.set())
		madym_options_.auditLogBaseName = auditLogBaseName();
	madym_options_.auditLogDir = auditLogDir();
	if (errorCodesName.set())
		madym_options_.errorCodesName = errorCodesName();

	if (overwrite.set())
		madym_options_.overwrite = overwrite();

  //Instantiate new madym_exe object with these options and run
  mdm_RunTools madym_exe(madym_options_);

  return madym_exe.run_CalculateT1(exeArgs, "calculate_T1");
}
