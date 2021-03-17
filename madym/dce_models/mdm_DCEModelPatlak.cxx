/**
*  @file    mdm_DCEModelPatlak.cxx
*  @brief   Implementation of mdm_DCEModelPatlak class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelPatlak.h"
#include <cmath>

MDM_API mdm_DCEModelPatlak::mdm_DCEModelPatlak(
  mdm_AIF &AIF,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initialParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
	const std::vector<int> &relativeLimitParams,
	const std::vector<double> &relativeLimitValues)
	:mdm_DCEModelBase(
		AIF, paramNames, initialParams,
		fixedParams, fixedValues,
		relativeLimitParams,
		relativeLimitValues)
{
  //Default values specific to tofts model
  if (pkParamNames_.empty())
    pkParamNames_ = { "Ktrans", "v_p", "tau_a" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.2, 0.2, 0.0}; //
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0 };
  if (upperBounds_.empty())
    upperBounds_ = { 10.0, 1.0, 0.5 };

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelPatlak::~mdm_DCEModelPatlak()
{

}

MDM_API std::string mdm_DCEModelPatlak::modelType() const
{
  return "mdm_DCEModelPatlak";
}

MDM_API void mdm_DCEModelPatlak::computeCtModel(size_t nTimes)
{
  //Reset all the model concentrations to 0
  for (size_t i_t = 0; i_t < nTimes; i_t++)
    CtModel_[i_t] = 0;

  for (const double& param : pkParams_)
  {
    if (std::isnan(param))
      return;
  }

  const double &kTrans =   pkParams_[0];
  const double &vp =       pkParams_[1];
  const double &tau_a =  pkParams_[2];

  //Resample AIF and get AIF times (I don't usually like single letter variables
  //but to be consistent with paper formulae, use AIF times = t)
  AIF_.resample_AIF( tau_a);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> &t = AIF_.AIFTimes();

  if (kTrans == 0.0)
  {
    for (size_t i = 0; i < nTimes; i++)
      CtModel_[i] = vp * Ca_t[i];
    return;
  }

  double  integral = 0.0;
  CtModel_[0] = vp * Ca_t[0];

  for (size_t i_t = 1; i_t < nTimes; i_t++)
  {
    double delta_t = t[i_t] - t[i_t - 1];
    double Ca_2 = (Ca_t[i_t-1] + Ca_t[i_t]) / 2;
    integral += delta_t*Ca_2;

    double C_t = vp * Ca_t[i_t] + kTrans * integral;

    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;
  }
}

MDM_API void mdm_DCEModelPatlak::checkParams()
{ 
	//First check all finite and not NaN
	for (double p : pkParams_)
	{
		if (std::isnan(p) || !std::isfinite(p))
		{
			errorCode_ = mdm_ErrorTracker::DCE_FIT_FAIL;
			return;
		}
	}
	errorCode_ = mdm_ErrorTracker::OK;
}

