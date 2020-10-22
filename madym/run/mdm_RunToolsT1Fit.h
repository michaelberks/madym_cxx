/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_T1_FIT_HDR
#define MDM_RUNTOOLS_T1_FIT_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunToolsT1Fit : public virtual mdm_RunTools {

public:

		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunToolsT1Fit();
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_RunToolsT1Fit();

protected:
	//Methods:
	bool setT1Method(const std::string &method);

	//Variables:

private:

};

#endif
