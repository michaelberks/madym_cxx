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
#include <madym/run/mdm_RunTools_madym_T1.h>
#include <madym/mdm_OptionsParser.h>

mdm_OptionsParser options_parser_;
mdm_InputOptions options_;

/******************************************************************************************
*       Model fitting                                                                    *
******************************************************************************************/

/**
* Main program based on command-line input.
*/
int main(int argc, char *argv[])
{
	//Instantiate new madym_exe object
	mdm_RunTools_madym_T1 madym_exe(options_, options_parser_);

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run_catch();
}
