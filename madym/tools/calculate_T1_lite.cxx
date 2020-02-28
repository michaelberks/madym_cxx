/**
*
* Standalone T1 calculator, takes simple text files of input data.
* Suitable for easy use with python/Matlab wrappers. Aim is to 
* support all commonly used methods for calculating T1, however
* currently only the variable flip-angle method is implemented.
*
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Lite version of standalone T1 calculator, takes simple text files of input data.
* @param
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
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Main program based on command-line input.
* @version  2.?
* @param    
*/
int main(int argc, char *argv[])
{
  /* If cmd-line looks ok save for log otherwise advise user */
  vul_arg_base::set_help_option("-h");

  vul_arg<std::string> inputDataFile("-d", "input data filename, see notes for options", "");
  vul_arg<int> nSignals("-n", "Number of signals in data", 0);
  vul_arg<double> TR("-TR", madym_options_.TRText.c_str(), madym_options_.TR);

	vul_arg<std::string> outputDir("-o", madym_options_.outputDirText.c_str(), madym_options_.outputDir);
	vul_arg<std::string> outputName("-O", madym_options_.outputNameText.c_str(), madym_options_.outputName);

	vul_arg<std::string> method("-m", madym_options_.T1methodText.c_str(), madym_options_.T1method);

	vul_arg<bool> help("-help", madym_options_.helpText.c_str(), madym_options_.help);
	vul_arg<bool> version("-version", madym_options_.versionText.c_str(), madym_options_.version);
  //////////////////////////////////////////////////////////////////

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

	//Copy command line options back into options object
	madym_options_.inputDataFile = inputDataFile();
	madym_options_.TR = TR();
	madym_options_.outputDir = outputDir();
	madym_options_.outputName = outputName();
	madym_options_.T1method = method();

	//Instantiate new madym_exe object with these options and run
	mdm_RunTools madym_exe(madym_options_);
	return madym_exe.run_CalculateT1_lite(exeArgs, "calculate_T1_lite", nSignals());
}
