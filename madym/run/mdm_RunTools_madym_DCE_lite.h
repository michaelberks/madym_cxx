/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DCE_LITE_HDR
#define MDM_RUNTOOLS_MADYM_DCE_LITE_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsDCEFit.h>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunTools_madym_DCE_lite : public mdm_RunToolsDCEFit {

public:

		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunTools_madym_DCE_lite(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_RunTools_madym_DCE_lite();
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int run();

	/**
	* @brief

	* @param
	* @return
	*/
	using mdm_RunTools::parse_inputs;
	MDM_API int parse_inputs(int argc, const char *argv[]);

protected:
  

private:
  //Methods:
	void fit_series(std::ostream &outputData,
		const std::vector<double> &ts, const bool &inputCt,
		const std::vector<double> &noiseVar,
		const double &t10, const double &s0,
		const double &r1,
		const double &TR,
		const double & FA,
		const int &firstImage,
		const int &lastImage,
		const bool&testEnhancement,
		const bool&useM0Ratio,
		const std::vector<double> &IAUCTimes,
		const bool &outputCt_mod,
		const bool &outputCt_sig,
		const bool &optimiseModel);

	//Variables:
};

#endif
