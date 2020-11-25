/*!
*  @file    mdm_RunTools_madym_T1_lite.h
*  @brief   Defines class mdm_RunTools_madym_T1_lite to run the lite version of the T1 mapping tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_T1_LITE_HDR
#define MDM_RUNTOOLS_MADYM_T1_LITE_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsT1Fit.h>

//! Class to run the lite version of the T1 mapping tool
/*!
*/
class mdm_RunTools_madym_T1_lite : public mdm_RunToolsT1Fit {

public:

		
	//! Constructor
	/*!
	\param options set of analysis options
	\param options_parser_ object for parsing input options
	\return
	*/
	MDM_API mdm_RunTools_madym_T1_lite(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_T1_lite();

	//! parse user inputs specific to T1 mapping
	/*!
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	using mdm_RunTools::parseInputs;
	MDM_API int parseInputs(int argc, const char *argv[]);

protected:
  //! Runs the lite version of T1 mapping
  /*!
  1. Parses and validates input options
  2. Sets specified T1 method
  3. Opens input data file
  4. Processes each line in input data file, mapping T1 from input signals, writing T1 and M0 values to output file
  5. Closes input/output file and reports the number of samples processed.
  \return 0 on success, non-zero otherwise
  */
  MDM_API int run();

private:

};

#endif
