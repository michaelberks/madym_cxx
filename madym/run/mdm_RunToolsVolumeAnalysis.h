/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_VOLUMEANALYSIS_HDR
#define MDM_RUNTOOLS_VOLUMEANALYSIS_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>

namespace fs = boost::filesystem;

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunToolsVolumeAnalysis : public virtual mdm_RunTools {

public:

		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunToolsVolumeAnalysis();
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual ~mdm_RunToolsVolumeAnalysis();

protected:
	//Methods:
	MDM_API void loadErrorMap();

	MDM_API void loadROI();

	MDM_API void loadT1Inputs();

	MDM_API void writeOuput();

	//Variables:
	mdm_FileManager fileManager_;
	mdm_DCEVolumeAnalysis volumeAnalysis_;

private:
	fs::path errorMapPath_;
};

#endif
