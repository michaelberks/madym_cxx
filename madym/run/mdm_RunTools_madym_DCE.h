/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DCE_HDR
#define MDM_RUNTOOLS_MADYM_DCE_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsDCEFit.h>
#include <madym/run/mdm_RunToolsT1Fit.h>
#include <madym/run/mdm_RunToolsVolumeAnalysis.h>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunTools_madym_DCE : public mdm_RunToolsDCEFit, mdm_RunToolsT1Fit, mdm_RunToolsVolumeAnalysis {

public:

		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunTools_madym_DCE(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_RunTools_madym_DCE();
  	
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
