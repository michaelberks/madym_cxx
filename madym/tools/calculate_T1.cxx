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

  vul_arg<std::vector<std::string>> mapNames("-maps", madym_options_.T1inputNamesText.c_str(), madym_options_.T1inputNames);
	
	vul_arg<std::string> outputDir("-o", madym_options_.outputDirText.c_str(), madym_options_.outputDir);

  vul_arg<std::string> method("-m", madym_options_.T1methodText.c_str(), madym_options_.T1method);
	vul_arg<double> noiseThresh("-noise", madym_options_.T1noiseThreshText.c_str(), madym_options_.T1noiseThresh);

	vul_arg<std::string> roiName("-roi", madym_options_.roiNameText.c_str(), madym_options_.roiName);

	vul_arg<std::string> programLogBaseName("-log", madym_options_.programLogBaseNameText.c_str(),
		madym_options_.programLogBaseName);
	vul_arg<std::string> auditLogBaseName("-audit", madym_options_.auditLogBaseNameText.c_str(),
		madym_options_.auditLogBaseName);
	vul_arg<std::string> auditLogDir("-audit_dir", madym_options_.auditLogDirText.c_str(),
		madym_options_.auditLogDir);
	vul_arg<std::string> errorBaseName("-E", madym_options_.errorBaseNameText.c_str(),
		madym_options_.errorBaseName);

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

  madym_options_.outputDir = outputDir();
  madym_options_.T1inputNames = mapNames();

  madym_options_.T1method = method();
  madym_options_.T1noiseThresh = noiseThresh();
  madym_options_.roiName = roiName();

	madym_options_.programLogBaseName = programLogBaseName();
	madym_options_.auditLogBaseName = auditLogBaseName();
	madym_options_.auditLogDir = auditLogDir();
	madym_options_.errorBaseName = errorBaseName();

  madym_options_.overwrite = overwrite();
  madym_options_.help = help();
  madym_options_.version = version();

  //Instantiate new madym_exe object with these options and run
  mdm_RunTools madym_exe(madym_options_);

  return madym_exe.run_CalculateT1(exeArgs, "calculate_T1");
}
