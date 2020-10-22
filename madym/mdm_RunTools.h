/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_HDR
#define MDM_RUNTOOLS_HDR
#include "mdm_api.h"
#include <mdm_version.h>
#include <mdm_OptionsParser.h>
#include <madym/mdm_FileManager.h>
#include <madym/mdm_DCEVolumeAnalysis.h>
#include <madym/mdm_AIF.h>
#include <madym/mdm_T1VolumeAnalysis.h>
#include <madym/mdm_ErrorTracker.h>

#include <madym/dce_models/mdm_DCEModelBase.h>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunTools {

public:
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunTools(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_RunTools();
  	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual int run() = 0;

protected:
	int mdm_progExit();
	void mdm_progAbort(const std::string &err_str);

	std::string timeNow();

	void set_up_logging(boost::filesystem::path outputPath);

	//Variables:
	mdm_InputOptions options_;
	mdm_OptionsParser options_parser_;
	

private:
  //Methods:
	
};

#endif /* mdm_RunTools_HDR */
