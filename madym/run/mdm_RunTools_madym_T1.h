/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_T1_HDR
#define MDM_RUNTOOLS_MADYM_T1_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsT1Fit.h>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunTools_madym_T1 : public mdm_RunToolsT1Fit {

public:

		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunTools_madym_T1(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_RunTools_madym_T1();
  	
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

	//Variables:
	mdm_DCEVolumeAnalysis volumeAnalysis_;
	mdm_FileManager fileManager_;
};

#endif
