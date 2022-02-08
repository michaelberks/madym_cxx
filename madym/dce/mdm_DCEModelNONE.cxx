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

MDM_API mdm_DCEModelNONE::mdm_DCEModelNONE(
  mdm_AIF &AIF)
	:mdm_DCEModelBase(AIF)
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

MDM_API void mdm_DCEModelNONE::computeCtModel(size_t nTimes)
{
  return;
}

MDM_API void mdm_DCEModelNONE::checkParams()
{ 
}

