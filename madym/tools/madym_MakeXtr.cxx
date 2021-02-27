/**
* @file madym_DicomConvert.cxx
* Tool for converting DICOM series into NIFTI/Analyze format for further processing.
*
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Tool for sorting and converting DICOM data to separate 3d image volumes
*/


#include <madym/run/mdm_RunTools_madym_MakeXtr.h>


//---------------------------------------------------------------------
//! Launch the command line tool
int main(int argc, char *argv[])
{
  

  //Instantiate new madym_exe object
  mdm_RunTools_madym_MakeXtr madym_exe;

  //Parse inputs
  auto parse_error = madym_exe.parseInputs(argc, (const char **)argv);
  if (parse_error == mdm_OptionsParser::HELP || parse_error == mdm_OptionsParser::VERSION)
    return 0;
  else if (parse_error != mdm_OptionsParser::OK)
    return parse_error;

  //If inputs ok, then run
  return madym_exe.run_catch();
  
}
