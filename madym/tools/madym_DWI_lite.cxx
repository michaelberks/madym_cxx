/**
* @file madym_T1_lite.cxx
* Standalone DWI tool, takes simple text files of input data.
* Suitable for easy use with python/Matlab wrappers.
*
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Lite version of standalone DWI tool, takes simple text files of input data.
*/

#include <madym/run/mdm_RunTools_madym_DWI_lite.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{
  
	//Instantiate new madym_exe object
	mdm_RunTools_madym_DWI_lite madym_exe;

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run_catch();
}
