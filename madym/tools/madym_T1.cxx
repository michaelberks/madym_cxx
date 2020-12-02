/**
* @file madym_T1.cxx
* Standalone T1 calculator, reads in analyze format image volumes
* and computes a volumetric map of T1 values. The aim is to
* support all commonly used methods for calculating T1, however
* currently only the variable flip-angle method is implemented.
*
* @author   MA Berks (c) Copyright QBI Lab, University of Manchester 2020
* @brief    Madym's command-line program for mapping T1 from inout images
*/
#include <madym/run/mdm_RunTools_madym_T1.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{
	//Instantiate new madym_exe object
	mdm_RunTools_madym_T1 madym_exe;

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run_catch();
}
