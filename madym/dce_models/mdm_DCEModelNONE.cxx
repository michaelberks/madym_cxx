/**
*  @file    mdm_DCEModelNONE.cxx
*  @brief   Implementation of mdm_DCEModelNONE class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelNONE.h"
#include <cmath>

const double mdm_DCEModelNONE::ETM_KEPMAX = 42.0;

MDM_API mdm_DCEModelNONE::mdm_DCEModelNONE(
  mdm_AIF &AIF,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initialParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
	const std::vector<int> &relativeLimitParams,
	const std::vector<double> &relativeLimitValues)
	:mdm_DCEModelBase(
    AIF, {}, {},
    {}, {},
    {},
    {})
{
  //Could be stricter and raise exception if any inputs are non-empty?
}

MDM_API mdm_DCEModelNONE::~mdm_DCEModelNONE()
{

}

MDM_API std::string mdm_DCEModelNONE::modelType() const
{
  return "mdm_DCEModelNONE";
}

MDM_API void mdm_DCEModelNONE::computeCtModel(int nTimes)
{
  return;
}

MDM_API double mdm_DCEModelNONE::checkParams()
{ 
  //Check params should never be called, so if it does, make sense to return a NULL value
	return BAD_FIT_SSD;
}

MDM_API void mdm_DCEModelNONE::resetRerun()
{
  return;
}

