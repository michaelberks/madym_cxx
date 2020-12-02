/*!
*  @file    mdm_RunTools_madym_T1.h
*  @brief   Defines class mdm_RunTools_madym_T1 to run the T1 mapping tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_T1_HDR
#define MDM_RUNTOOLS_MADYM_T1_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsT1Fit.h>
#include <madym/run/mdm_RunToolsVolumeAnalysis.h>

//! Class to run the T1 mapping tool
/*!
*/
class mdm_RunTools_madym_T1 : public mdm_RunToolsVolumeAnalysis {

public:

		
	//! Constructor
	/*!
	*/
	MDM_API mdm_RunTools_madym_T1();
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_T1();

	//! parse user inputs specific to T1 mapping
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
  //! Runs the T1 mapping pipeline
  /*!
  1. Parses and validates input options
  2. Sets specified T1 method
  3. Loads in signal input volumes (and ROI if given)
  4. Processes all voxels (in ROI mask if given), mapping T1 from input signals, saving T1 and M0 values to output maps
  5. Saves output maps.
  Throws mdm_exception if errors encountered
  */
  MDM_API void run();

private:
  //Methods:

};

#endif
