/**
 *  @file    madym_DCE_lite.cxx
 *  @brief   Lite version of DCE analysis tool, takes simple text files of input data.
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#include <madym/run/mdm_RunTools_madym_DCE_lite.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{
	//Instantiate new madym_exe object
	mdm_RunTools_madym_DCE_lite madym_exe;

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;
		
	//If inputs ok, then run
	return madym_exe.run_catch();
}
