#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunToolsDCEFit.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsDCEFit::mdm_RunToolsDCEFit()
	:
	model_(NULL)
{
}


MDM_API mdm_RunToolsDCEFit::~mdm_RunToolsDCEFit()
{

}

//*******************************************************************************
/**
* @author   Mike Berks modifying GA Buonaccorsi, based on GJM Parker
* @brief    Fit selected dynamic contrast agent concentration model
* @version  madym 1.22
*/
void mdm_RunToolsDCEFit::setModel(const std::string &modelName, bool auto_aif, bool auto_pif,
	const std::vector<std::string> &paramNames,
	const std::vector<double> &initialParams,
	const std::vector<int> fixedParams,
	const std::vector<double> fixedValues,
	const std::vector<int> relativeLimitParams,
	const std::vector<double> relativeLimitValues)
{
	auto modelType = mdm_DCEModelGenerator::ParseModelName(modelName);
	if (modelType == mdm_DCEModelGenerator::UNDEFINED)
		mdm_progAbort("Invalid or unsupported model (from command-line)");

	model_ = mdm_DCEModelGenerator::createModel(AIF_,
		modelType, auto_aif, auto_pif, paramNames,
		initialParams, fixedParams, fixedValues,
		relativeLimitParams, relativeLimitValues);

	
}