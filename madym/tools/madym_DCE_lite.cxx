/**
 *  @file    madym_lite.cxx
 *  @brief   Main program for lite-weight madym version
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#include <madym/run/mdm_RunTools_madym_DCE_lite.h>
#include <madym/mdm_OptionsParser.h>

mdm_OptionsParser options_parser_;
mdm_InputOptions options_;

/******************************************************************************************
 *       Model fitting                                                                    *
 ******************************************************************************************/

/**
 * Main program based on command-line input.
 *
 * @brief    madym's command-line main program
 * @param    argc   Number of command-line parameters
 * @param    argv   String array of command-line parameters
 * @return   int    Standard C++ return status flag
 */
int main(int argc, char *argv[])
{
	//Instantiate new madym_exe object
	mdm_RunTools_madym_DCE_lite madym_exe(options_, options_parser_);

	//Parse inputs
	int parse_error = madym_exe.parse_inputs(argc, (const char **)argv);
	if (parse_error)
		return parse_error;

	//If inputs ok, then run
	return madym_exe.run();
}
