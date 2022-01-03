/**
* @file madym_T1.cxx
* Standalone tool for working with DWI, reads in analyze format image volumes
* and computes a volumetric maps of ADC or IVIM parameters.
*
* @author   MA Berks (c) Copyright QBI Lab, University of Manchester 2020
* @brief    Madym's command-line program for mapping T1 from inout images
*/
#include <madym/run/mdm_RunTools_madym_DWI.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{
	//Instantiate new madym_exe object
	mdm_RunTools_madym_DWI madym_exe;

	//Parse inputs
	auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run_catch();
}
