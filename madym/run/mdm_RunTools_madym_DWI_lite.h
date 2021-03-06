/*!
*  @file    mdm_RunTools_madym_DWI_lite.h
*  @brief   Defines class mdm_RunTools_madym_DWI_lite to run the lite version of the DWI modelling tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DWI_LITE_HDR
#define MDM_RUNTOOLS_MADYM_DWI_LITE_HDR
#include <madym/utils/mdm_api.h>
#include <madym/run/mdm_RunTools.h>

//! Class to run the lite version of the DWI models tool
/*!
*/
class mdm_RunTools_madym_DWI_lite : public mdm_RunTools {

public:

		
	//! Constructor
	/*!
	*/
	MDM_API mdm_RunTools_madym_DWI_lite();
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_DWI_lite();

	//! parse user inputs specific to DWI tool
	/*!
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	using mdm_RunTools::parseInputs;
	MDM_API int parseInputs(int argc, const char *argv[]);

	//! Return name of the tool
	/*!
  Must be implemented by the derived classes that will be instantiated into run tools objects.
	\return name of the tool 
  */
  MDM_API std::string who() const;
	
protected:
  //! Runs the lite version of DWI modelling
  /*!
  1. Parses and validates input options
  2. Sets specified DWI method
  3. Opens input data file
  4. Processes each line in input data file, fitting DWI model to input signals, writing fitted parameters values to output file
  5. Closes input/output file and reports the number of samples processed.
  
  Throws mdm_exception if errors encountered
  */
  MDM_API void run();

private:

};

#endif
