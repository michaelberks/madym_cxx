/**
* @file madym_T1_lite.cxx
* Standalone T1 calculator, takes simple text files of input data.
* Suitable for easy use with python/Matlab wrappers. Aim is to 
* support all commonly used methods for calculating T1, however
* currently only the variable flip-angle method is implemented.
*
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Lite version of standalone T1 calculator, takes simple text files of input data.
*/

#include <madym/run/mdm_RunTools_madym_T1_lite.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{
  
	//Instantiate new madym_exe object
	mdm_RunTools_madym_T1_lite madym_exe;

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run_catch();
}
