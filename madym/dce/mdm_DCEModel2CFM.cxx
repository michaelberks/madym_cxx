
/**
*  @file    mdm_DCEModel2CFM.cxx
*  @brief   Implementation of mdm_DCEModel2CFM class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModel2CFM.h"
#include <madym/dce/mdm_Exponentials.h>
#include <cmath>

MDM_API mdm_DCEModel2CFM::mdm_DCEModel2CFM(
  mdm_AIF &AIF,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initialParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
  const std::vector<double>& lowerBounds,
  const std::vector<double>& upperBounds,
	const std::vector<int> &relativeLimitParams,
	const std::vector<double> &relativeLimitValues)
  :mdm_DCEModelBase(
		AIF, paramNames, initialParams, 
		fixedParams, fixedValues,
    lowerBounds, upperBounds,
		relativeLimitParams,
		relativeLimitValues)
{
  if (pkParamNames_.empty())
    pkParamNames_ = { "F_p", "PS", "v_e", "v_p", "tau_a"};
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.60,  0.2,  0.2,   0.2,  0.0};//{"F_p", "PS", "v_e", "v_p", "tau_a"}
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 1e-5, 1e-5, 1e-5, 1e-5, 0.0 };
  if (upperBounds_.empty())
    upperBounds_ = { 100.0, 10.0, 10.0, 10.0, 0.5 };//, 0.1

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModel2CFM::~mdm_DCEModel2CFM()
{

}

MDM_API std::string mdm_DCEModel2CFM::modelType() const
{
  return "mdm_DCEModel2CFM";
}

MDM_API void mdm_DCEModel2CFM::computeCtModel(size_t nTimes)
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
  const double &PS = pkParams_[1];// efflux flow
  const double &v_e = pkParams_[2];//extravascular, extracellular space
  const double &v_p = pkParams_[3];//plasma volume*/
  const double &tau_a = pkParams_[4];//AIF_ delay
  
  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  //Resample AIF and get AIF times
  AIF_.resample_AIF( tau_a);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> &t = AIF_.AIFTimes();

  auto TP = v_p / F_p;
  auto TE = v_e / PS;
  auto TT = (v_p + v_e) / F_p;
  auto Tpos = TE;
  auto Tneg = TP;
  auto Epos = (TT - Tneg) / (Tpos - Tneg);
  auto Eneg = 1 - Epos;

  auto F_pos = F_p * Epos;
  auto F_neg = F_p * Eneg;
  auto K_pos = 1 / Tpos;
  auto K_neg = 1 / Tneg;

  mdm_Exponentials::biexponential(
    F_pos, F_neg, K_pos, K_neg, Ca_t, t,
    CtModel_);
}

MDM_API void mdm_DCEModel2CFM::checkParams()
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

MDM_API std::vector<double> mdm_DCEModel2CFM::makeLLSMatrix(const std::vector<double>& Ct_sig) const
{
  auto tau_a = pkParams_[4];
  AIF_.resample_AIF(tau_a);
  const auto& Cp_t = AIF_.AIF();
  const auto& t = AIF_.AIFTimes();
  return mdm_Exponentials::make_biexponential_LLS_matrix(
    Ct_sig, Cp_t, t);
}

MDM_API void mdm_DCEModel2CFM::transformLLSolution(const double* B)
{
  double& F_p = pkParams_[0];
  double& PS = pkParams_[1];
  double& v_e = pkParams_[2];
  double& v_p = pkParams_[3];

  F_p = B[3];
  auto T = B[2] / (B[0] * F_p);
  auto ba = std::sqrt(B[1] * B[1] - 4 * B[0]);
  auto T_e = (B[1] + ba) / (2 * B[0]);
  auto T_p = (B[1] - ba) / (2 * B[0]);

  v_p = T_p * F_p;
  v_e = T * F_p - v_p;
  PS = v_e / T_e;

}