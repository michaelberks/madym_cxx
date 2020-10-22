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
#include <madym/run/mdm_RunTools_calculateT1.h>
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
	int parse_error = options_parser_.calculate_T1_inputs(argc, (const char **)argv, options_);

	if (parse_error)
		return parse_error;

	//Instantiate new madym_exe object with these options and run
	mdm_RunTools_calculateT1 madym_exe(options_, options_parser_);
  return madym_exe.run();
}
