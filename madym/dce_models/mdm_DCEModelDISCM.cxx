/**
*  @file    mdm_DCEModelDISCM.cxx
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelDISCM.h"
#include <cmath>

MDM_API mdm_DCEModelDISCM::mdm_DCEModelDISCM(
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
  if (pkParamNames_.empty())
    pkParamNames_ = { "F_p", "k_2", "f_a", "tau_a", "tau_v" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.6,   1.0,  0.5,  0.025,      0.0 };
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0, 0.0, -0.5 };
  if (upperBounds_.empty())
    upperBounds_ = { 10.0, 10.0, 1.0, 0.5, 0.5 };//, 0.1

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelDISCM::~mdm_DCEModelDISCM()
{

}

MDM_API std::string mdm_DCEModelDISCM::modelType() const
{
  return "mdm_DCEModelDISCM";
}

MDM_API void mdm_DCEModelDISCM::computeCtModel(int nTimes)
{
  //Reset all the model concentrations to 0
  for (int i_t = 0; i_t < nTimes; i_t++)
    CtModel_[i_t] = 0;

  for (const double& param : pkParams_)
  {
    if (std::isnan(param))
      return;
  }

  //Rename parameters
  const double &F_p = pkParams_[0];// arterial flow rate constant
  const double &k2 = pkParams_[1];//outflow rate constant
  const double &f_a = pkParams_[2];//modified portal flow rate constant
  const double &tau_a = pkParams_[3];//arterial delay (Tau_a in paper)
  const double &tau_v = pkParams_[4];//portal vein delay (Tau_hpv in paper)
  const double KMAX = 1e9;

  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  //Resample AIF and get AIF times
  AIF_.resample_AIF( tau_a);
  AIF_.resample_PIF( tau_v, false, true);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> Cv_t = AIF_.PIF();
  const std::vector<double> &t = AIF_.AIFTimes();

  double f_v = 1 - f_a; // estimate of hepatic portal venous fraction
  double k1a = f_a * F_p;
  double k1hpv = f_v * F_p;
  
  double Cp_t0 = (k1a*Ca_t[0] + k1hpv * Cv_t[0]);

  for (int i_t = 1; i_t < nTimes; i_t++)
  {
    //Make combined input function
    double Cp_t1 = (k1a * Ca_t[i_t] + k1hpv * Cv_t[i_t]);

    //Update the integral sum terms
    double delta_t = t[i_t] - t[i_t-1];
    double e_delta = std::exp(-delta_t * k2);

    double A = (k2 > KMAX) ? 0 :
      delta_t * 0.5 * (Cp_t1 + Cp_t0*e_delta);
    double C_t = e_delta * CtModel_[i_t - 1] + A;
    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;
    Cp_t0 = Cp_t1;
  }
}

MDM_API double mdm_DCEModelDISCM::checkParams()
{
	/*TODO: define parameter limits - for now, just check all finite and not NAN*/
	for (double p : pkParams_)
	{
		if (std::isnan(p) || !std::isfinite(p))
		{
			errorCode_ = mdm_ErrorTracker::DCE_FIT_FAIL;
			return BAD_FIT_SSD;
		}
	}
	errorCode_ = mdm_ErrorTracker::OK;
	return 0;
}

MDM_API void mdm_DCEModelDISCM::resetRerun()
{
  //Reset selected parameters to their initial value then rerun the optimisation
  std::vector<int> fixedParams = { 3, 4 };
  for (int i = 0; i < fixedParams.size(); i++)
    pkParams_[fixedParams[i]] = pkInitParams_[fixedParams[i]];
}
