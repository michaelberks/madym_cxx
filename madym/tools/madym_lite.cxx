/**
 *  @file    madym_lite.cxx
 *  @brief   Main program for lite-weight madym version
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#include <mdm_RunTools.h>
#include <mdm_InputOptions.h>

mdm_InputOptions options_parser_;
mdm_DefaultValues options_;

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
	int parse_error = options_parser_.madym_lite_inputs(argc, (const char **)argv, options_);

	if (parse_error)
		return parse_error;

	//Instantiate new madym_exe object with these options and run
	mdm_RunTools madym_exe(options_, options_parser_);
	return madym_exe.run_DCEFit_lite();
}
