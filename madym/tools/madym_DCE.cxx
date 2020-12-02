/**
* @file madym_DCE.cxx
* Main program based on command-line input.
*
* @brief    Madym's main command-line tool for DCE analysis
* @author MA Berks(c) Copyright QBI Lab, University of Manchester 2020
*/

#include <madym/run/mdm_RunTools_madym_DCE.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{
	
	//Instantiate new madym_exe object
	mdm_RunTools_madym_DCE madym_exe;

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run_catch();

}
