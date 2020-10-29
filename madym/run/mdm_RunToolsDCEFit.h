/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_DCE_FIT_HDR
#define MDM_RUNTOOLS_DCE_FIT_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>
#include <memory>

/**
*  @brief   Called by command line/GUI tools to run DCE-analysis or T1 mapper
*  @details More info...
*/
class mdm_RunToolsDCEFit : public virtual mdm_RunTools {

public:

		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_RunToolsDCEFit();
		
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual ~mdm_RunToolsDCEFit();

protected:
	//Methods:
	void setModel(const std::string &model_name, bool auto_aif, bool auto_pif,
		const std::vector<std::string> &paramNames,
		const std::vector<double> &initialParams,
		const std::vector<int> fixedParams,
		const std::vector<double> fixedValues,
		const std::vector<int> relativeLimitParams,
		const std::vector<double> relativeLimitValues);

	//Variables:
	std::shared_ptr<mdm_DCEModelBase> model_;
	mdm_AIF AIF_;

private:

};

#endif
