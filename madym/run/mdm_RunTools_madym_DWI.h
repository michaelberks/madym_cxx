/*!
*  @file    mdm_RunTools_madym_T1.h
*  @brief   Defines class mdm_RunTools_madym_T1 to run the T1 mapping tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DWI_HDR
#define MDM_RUNTOOLS_MADYM_DWI_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsT1Fit.h>
#include <madym/run/mdm_RunToolsVolumeAnalysis.h>

//! Class to run the DWI mapping tool
/*!
*/
class mdm_RunTools_madym_DWI : public mdm_RunToolsVolumeAnalysis {

public:

		
	//! Constructor
	/*!
	*/
	MDM_API mdm_RunTools_madym_DWI();
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_DWI();

	//! parse user inputs specific to DWI mapping
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
  2. 
  3. Loads in signal input volumes (and ROI if given)
  4. Processes all voxels (in ROI mask if given)...
  5. Saves output maps.
  Throws mdm_exception if errors encountered
  */
  MDM_API void run();

private:
  //Methods:

	//! Map DWI from input images using method specified in options
	void mapDWI();

	//! Check there are a valid number of signal inputs for a given DWI modelling method
	/*!
	Throws mdm_exception if number of inputs not valid
	\param methodType T1 method
	\param numInputs number of input signals specified by user
	*/
	void checkNumInputs(mdm_DWImodelGenerator::DWImodels methodType, const int& numInputs);

	//! Check if there are T1 signal inputs to load, and if so, load them
	void loadDWIInputs();


};

#endif
