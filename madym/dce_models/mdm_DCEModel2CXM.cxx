
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
#include <cmath>

MDM_API mdm_DCEModel2CXM::mdm_DCEModel2CXM(
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
  
  const double KMAX = 1e9;
  
  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  //Resample AIF and get AIF times
  AIF_.resample_AIF( tau_a);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> &AIFtimes = AIF_.AIFTimes();

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

  // Let's rewrite the convolution sum, using the exponential "trick" so 
  //we can compute everything in one forward loop
  double  Ft_pos = 0;
  double Ft_neg = 0;

  for (size_t i_t = 1; i_t < nTimes; i_t++)
  {

    // Get current time, and time change
    double delta_t = AIFtimes[i_t] - AIFtimes[i_t-1];

    //Update the exponentials for the transfer terms in the two compartments
    double e_delta_pos = exp(-delta_t * K_pos);
    double e_delta_neg = exp(-delta_t * K_neg);

    //Use iterative trick to update the convolutions of transfers with the
    //input function
    double A_pos = (K_pos > KMAX) ? 0 : 
      delta_t * 0.5 * (Ca_t[i_t] + Ca_t[i_t - 1] * e_delta_pos);
    double A_neg = (K_pos > KMAX) ? 0 : 
      delta_t * 0.5 * (Ca_t[i_t] + Ca_t[i_t - 1] * e_delta_neg);

    Ft_pos = Ft_pos * e_delta_pos + A_pos;
    Ft_neg = Ft_neg * e_delta_neg + A_neg;

    //Combine the two compartments with the rate constant to get the final
    //concentration at this time point
    double C_t = F_neg*Ft_neg + F_pos*Ft_pos;

    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;

  }
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
