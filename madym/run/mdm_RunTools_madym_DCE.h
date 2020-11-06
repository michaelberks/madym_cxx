/*!
*  @file    mdm_RunTools_madym_DCE.h
*  @brief   Defines class mdm_RunTools_madym_DCE to run the DCE analysis tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DCE_HDR
#define MDM_RUNTOOLS_MADYM_DCE_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsDCEFit.h>
#include <madym/run/mdm_RunToolsT1Fit.h>
#include <madym/run/mdm_RunToolsVolumeAnalysis.h>

//! Class to run the DCE analysis tool
/*!
*/
class mdm_RunTools_madym_DCE : public mdm_RunToolsDCEFit, mdm_RunToolsT1Fit, mdm_RunToolsVolumeAnalysis {

public:

		
	//! Constructor
	/*!
	\param options set of analysis options
	\param options_parser_ object for parsing input options
	\return
	*/
	MDM_API mdm_RunTools_madym_DCE(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_DCE();
  	
	//! Runs the T1 mapping pipeline
	/*!
	1. Parses and validates input options
	2. Sets specified tracer-kinetic model
	3. Loads in signal/concentration input volumes (and ROI if given)
	4. Processes all voxels (in ROI mask if given), fitting tracer-kinetic model to input signals, saving fitted paremeters and IAUC measures to output maps
	5. Saves output maps.
	\return 0 on success, non-zero otherwise
	*/
	MDM_API int run();

	//! parse user inputs specific to DCE analysis
	/*!
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	using mdm_RunTools::parseInputs;
	MDM_API int parseInputs(int argc, const char *argv[]);

protected:
  

private:
  //Methods:
	void checkRequiredInputs();

	void setFileManagerParams();

	void setAIFParams();

	void setVolumeAnalysisParams();

	void loadSt();

	void loadCt();

	void loadT1();

	void mapT1();

	void loadAIF();

	void loadInitParamMaps();

	void fitModel();

	//Variables:
	bool paramMapsInitialised_;
};

#endif