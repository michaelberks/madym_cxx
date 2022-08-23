
/**
*  @file    mdm_DCEModel2CXM.cxx
*  @brief   Implementation of mdm_DCEModel2CXM class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModel2CXM.h"
#include <madym/dce/mdm_Exponentials.h>
#include <cmath>

MDM_API mdm_DCEModel2CXM::mdm_DCEModel2CXM(
  mdm_AIF &AIF,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initialParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
  const std::vector<double>& lowerBounds,
  const std::vector<double>& upperBounds,
  const std::vector<int>& relativeLimitParams,
  const std::vector<double>& relativeLimitValues,
  int repeatParam,
  const std::vector<double>& repeatValues)
  :mdm_DCEModelBase(
    AIF, paramNames, initialParams,
    fixedParams, fixedValues,
    lowerBounds, upperBounds,
    relativeLimitParams,
    relativeLimitValues,
    repeatParam,
    repeatValues)
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

MDM_API mdm_DCEModel2CXM::~mdm_DCEModel2CXM()
{

}

MDM_API std::string mdm_DCEModel2CXM::modelType() const
{
  return "mdm_DCEModel2CXM";
}

MDM_API void mdm_DCEModel2CXM::computeCtModel(size_t nTimes)
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

  double K_pos;
  double K_neg;
  double E_pos;

  //Method 1: Sourbron 2011
  if (F_p > 0 && PS > 0)
  {
    // First derive the secondary parameters from the input Pk parameters
    double E = PS / (PS + F_p); //Extraction fraction
    double e = v_e / (v_p + v_e); //Extractcellular fraction

    double tau = (E - E * e + e) / (2 * E);
    double tau_root = std::sqrt(1 - 4 * (E*e*(1 - E)*(1 - e)) / std::pow(E - E * e + e, 2));
    double tau_pos = tau * (1 + tau_root);
    double tau_neg = tau * (1 - tau_root);

    K_pos = F_p / ((v_p + v_e)*tau_neg);
    K_neg = F_p / ((v_p + v_e)*tau_pos);

    E_pos = (tau_pos - 1) / (tau_pos - tau_neg);
  }
  else
  {
    // Method 2
    double Kp = (F_p + PS) / v_p;
    double Ke = PS / v_e;
    double Kb = F_p / v_p;

    double K_sum = 0.5*(Kp + Ke);
    double K_root = 0.5 * std::sqrt(std::pow(Kp + Ke, 2) - 4 * Ke*Kb);
    K_pos = K_sum - K_root;
    K_neg = K_sum + K_root;

    E_pos = (K_neg - Kb) / (K_neg - K_pos);
  }

  if (std::isnan(K_neg) || std::isnan(K_pos) || std::isnan(E_pos))
  {
    return;
  }
  
  double F_pos = F_p*E_pos;
  double F_neg = F_p*(1 - E_pos);

  mdm_Exponentials::biexponential(
    F_pos, F_neg, K_pos, K_neg, Ca_t, t,
    CtModel_);
}

MDM_API void mdm_DCEModel2CXM::checkParams()
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

MDM_API std::vector<double> mdm_DCEModel2CXM::makeLLSMatrix(const std::vector<double>& Ct_sig) const
{
  auto tau_a = pkParams_[4];
  AIF_.resample_AIF(tau_a);
  const auto& Cp_t = AIF_.AIF();
  const auto& t = AIF_.AIFTimes();
  return mdm_Exponentials::make_biexponential_LLS_matrix(
    Ct_sig, Cp_t, t);
}

MDM_API void mdm_DCEModel2CXM::transformLLSolution(const double* B)
{
  double& F_p = pkParams_[0];
  double& PS = pkParams_[1];
  double& v_e = pkParams_[2];
  double& v_p = pkParams_[3];

  F_p = B[3];
  auto T = B[2] / (B[0] * F_p);
  auto T_e = B[1] / B[0] - T;
  auto T_p = 1 / (B[0] * T_e);

  v_p = T_p * F_p;
  v_e = T * F_p - v_p;
  PS = v_e / T_e;

}