/**
 @file madym_AIF.cxx

 @brief  Madym's command-line tool for auto detecting AIFs
 @author MA Berks(c) Copyright QBI Lab, University of Manchester 2020
*/

#include <madym/run/mdm_RunTools_madym_AIF.h>

//! Launch the command line tool
int main(int argc, char *argv[])
{

  //Instantiate new madym_exe object
  mdm_RunTools_madym_AIF madym_exe;

  //Parse inputs
  auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
	if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
		return 0;
	else if (parse_error != mdm_OptionsParser::OK)
		return parse_error;

  //If inputs ok, then run
  return madym_exe.run_catch();

}
