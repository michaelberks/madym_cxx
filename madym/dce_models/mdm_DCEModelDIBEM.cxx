/**
*  @file    mdm_DCEModelDIBEM.cxx
*  @brief   Implementation of mdm_DCEModelDIBEM class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelDIBEM.h"
#include <madym/mdm_Exponentials.h>

#include <cmath>

MDM_API mdm_DCEModelDIBEM::mdm_DCEModelDIBEM(
  mdm_AIF &AIF,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initialParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
  const std::vector<double>& lowerBounds,
  const std::vector<double>& upperBounds,
  const std::vector<int>& relativeLimitParams,
  const std::vector<double>& relativeLimitValues)
  :mdm_DCEModelBase(
    AIF, paramNames, initialParams,
    fixedParams, fixedValues,
    lowerBounds, upperBounds,
    relativeLimitParams,
    relativeLimitValues)
{
  if (pkParamNames_.empty())
    pkParamNames_ = { "Fpos", "Fneg", "Kpos", "Kneg", "f_a", "tau_a", "tau_v" };
  if (pkInitParams_.empty())
    pkInitParams_ = {    0.2,    0.2,    0.5,    4.0,  0.25,       0.025,    0.0};
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0, 0.0, -0.1, 0.0, -0.5 };
  if (upperBounds_.empty())
    upperBounds_ = { 100.0, 100.0, 100, 100, 1.1, 0.5, 0.5 };//, 0.1

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelDIBEM::~mdm_DCEModelDIBEM()
{

}

MDM_API std::string mdm_DCEModelDIBEM::modelType() const
{
  return "mdm_DCEModelDIBEM";
}

MDM_API void mdm_DCEModelDIBEM::computeCtModel(size_t nTimes)
{
  //Reset all the model concentrations to 0
  for (size_t i_t = 0; i_t < nTimes; i_t++)
    CtModel_[i_t] = 0;

  for (const double& param : pkParams_)
  {
    if (std::isnan(param))
      return;
  }

  //Rename parameters
  const double &F_pos = pkParams_[0];// flow plasma rate
  const double &F_neg = pkParams_[1];// efflux flow
  const double &K_pos = pkParams_[2];//extravascular, extracellular space
  const double &K_neg = pkParams_[3];//plasma volume*/
  const double &f_a = pkParams_[4];//the arterial fraction
  const double &tau_a = pkParams_[5];//AIF delay
  const double &tau_v = pkParams_[6];//the arterial fraction

  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  const auto& t = AIF_.AIFTimes();
  auto Cp_t = mdm_Exponentials::mix_vifs(AIF_, f_a, tau_a, tau_v);

  mdm_Exponentials::biexponential(
    F_pos, F_neg, K_pos, K_neg, Cp_t, t,
    CtModel_);
}

MDM_API void mdm_DCEModelDIBEM::checkParams()
{
	/*TODO: define parameter limits - for now, just check all finite and not NAN*/
	for (double p : pkParams_)
	{
		if (std::isnan(p) || !std::isfinite(p))
		{
			errorCode_ = mdm_ErrorTracker::DCE_FIT_FAIL;
			return;
		}
	}
	errorCode_ = mdm_ErrorTracker::OK;
	return;
}
