/**
*  @file    mdm_DCEModelAUEM.cxx
*  @brief   Implementation of mdm_DCEModelAUEM class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelAUEM.h"
#include <madym/mdm_Exponentials.h>
#include <cmath>

MDM_API mdm_DCEModelAUEM::mdm_DCEModelAUEM(
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
    pkParamNames_ = { "F_p", "v_ecs", "k_i", "k_ef", "f_a", "tau_a", "tau_v" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.6,  0.2,  0.2,   0.1,  0.5,      0.025,     0.00 };//
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5 };
  if (upperBounds_.empty())
    upperBounds_ = { 10.0, 1.0, 10.0, 10.0, 1.0, 0.5, 0.5 };//, 0.1

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelAUEM::~mdm_DCEModelAUEM()
{

}

MDM_API std::string mdm_DCEModelAUEM::modelType() const
{
  return "mdm_DCEModelAUEM";
}

MDM_API void mdm_DCEModelAUEM::computeCtModel(size_t nTimes)
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
  const double &F_p = pkParams_[0];// flow plasma rate
  const double &v_ecs = pkParams_[1];//extravascular, extracellular space
  const double &k_i = pkParams_[2];//transport constant to liver tissue
  const double &k_ef = pkParams_[3];//efflux to bile duct transfer constant
  const double &f_a = pkParams_[4];//the arterial fraction
  const double &tau_a = pkParams_[5];//the arterial fraction
  const double &tau_v = pkParams_[6];//the arterial fraction

  //Compute derived parameters from input parameters
  auto T_e = v_ecs / (F_p + k_i);// extracellular mean transit time
  auto v_i = 1 - v_ecs; // estimate of intracellular volume
  auto T_i = v_i  / k_ef;// intracellular mean transit time
  auto E_i = k_i  / (F_p + k_i);// the hepatic uptake fraction

  // This can also be precomputed
  auto E_pos = E_i  / (1 - T_e / T_i);

  auto K_neg = 1  / T_e;
  auto F_neg = F_p*(1 - E_pos);

  auto K_pos = 1  / T_i;
  auto F_pos = F_p*E_pos;

  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  const auto& t = AIF_.AIFTimes();
  auto Cp_t = mdm_Exponentials::mix_vifs(AIF_, f_a, tau_a, tau_v);

  mdm_Exponentials::biexponential(
    F_pos, F_neg, K_pos, K_neg, Cp_t, t,
    CtModel_);
}

MDM_API void mdm_DCEModelAUEM::checkParams()
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
}
