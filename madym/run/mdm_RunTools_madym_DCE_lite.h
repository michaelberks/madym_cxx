/*!
*  @file    mdm_RunTools_madym_DCE_lite.h
*  @brief   Defines class mdm_RunTools_madym_DCE_lite to run the lite version of the DCE analysis tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DCE_LITE_HDR
#define MDM_RUNTOOLS_MADYM_DCE_LITE_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsDCEFit.h>

//! Class to run the lite version of the DCE analysis tool
/*!
*/
class mdm_RunTools_madym_DCE_lite : public mdm_RunToolsDCEFit {

public:

		
	//! Constructor
	/*!
	\param options set of analysis options
	\param options_parser_ object for parsing input options
	\return
	*/
	MDM_API mdm_RunTools_madym_DCE_lite(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_DCE_lite();
  	
	//! Runs the lite version of DCE analysis
	/*!
	1. Parses and validates input options
	2. Sets specified tracer-kinetic model
	3. Opens input data file
	4. Processes each line in input data file, fitting tracer-kineti model to input signals/concentrations, 
	writing fited parameters and IAUC measurements to output file
	5. Closes input/output file and reports the number of samples processed.
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
	void fit_series(std::ostream &outputData,
    mdm_DCEModelFitter &fitter,
		const std::vector<double> &timeSeries, 
    const bool &inputCt,
		const double &T1, 
    const double &M0,
		const double &r1,
		const double &TR,
		const double & FA,
		const bool&testEnhancement,
		const bool&useM0Ratio,
		const std::vector<double> &IAUCTimes,
		const bool &outputCt_mod,
		const bool &outputCt_sig,
		const bool &optimiseModel);

	//Variables:
};

#endif
