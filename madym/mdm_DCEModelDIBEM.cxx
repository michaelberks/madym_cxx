/**
*  @file    mdm_DCEModelDIBEM.cxx
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelDIBEM.h"
#include <cmath>

MDM_API mdm_DCEModelDIBEM::mdm_DCEModelDIBEM(
  mdm_AIF &AIF,
  const std::vector<std::string> &pkParamNames,
  const std::vector<double> &pkInitParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues)
  :mdm_DCEModelBase(AIF, pkParamNames, pkInitParams, fixedParams, fixedValues)
{
  if (pkParamNames_.empty())
    pkParamNames_ = { "Fpos", "Fneg", "Kpos", "Kneg", "fa", "aoffset", "voffset" };
  if (pkInitParams_.empty())
    pkInitParams_ = {    0.2,    0.2,    0.5,    4.0,  0.5,       0.025,    0.0};
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5 };
  if (upperBounds_.empty())
    upperBounds_ = { 100.0, 100.0, 100, 100, 1.0, 0.5, 0.5 };//, 0.1

  mdm_DCEModelBase::init(fixedParams, fixedValues);
}

MDM_API mdm_DCEModelDIBEM::~mdm_DCEModelDIBEM()
{

}

MDM_API std::string mdm_DCEModelDIBEM::modelType() const
{
  return "mdm_DCEModelDIBEM";
}

MDM_API void mdm_DCEModelDIBEM::computeCtModel(int nTimes)
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
  const double &F_pos = pkParams_[0];// flow plasma rate
  const double &F_neg = pkParams_[1];// efflux flow
  const double &K_pos = pkParams_[2];//extravascular, extracellular space
  const double &K_neg = pkParams_[3];//plasma volume*/
  const double &f_a = pkParams_[4];//the arterial fraction
  const double &aoffset = pkParams_[5];//AIF delay
  const double &voffset = pkParams_[6];//the arterial fraction
  double f_v = 1 - f_a; // estimate of hepatic portal venous fraction
  const double KMAX = 1e6;

  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  //Resample AIF and get AIF times
  std::vector<double> Ca_t;
  if (f_a)
  {
    AIF_.resample_AIF(nTimes, aoffset);
    Ca_t = AIF_.AIF();
  }
  else
    Ca_t.resize(nTimes, 0);

  std::vector<double> Cv_t;
  if (f_v)
  {
    AIF_.resample_PIF(nTimes, voffset, false, true);
    Cv_t = AIF_.PIF();
  }
  else
    Cv_t.resize(nTimes, 0);
  
  const std::vector<double> &AIFtimes = AIF_.AIFTimes();

  // Let's rewrite the convolution sum, using the exponential "trick" so 
  //we can compute everything in one forward loop
  double  Ft_pos = 0;
  double Ft_neg = 0;

  double Cp_t0 = (f_a*Ca_t[0] + f_v * Cv_t[0]);

  for (int i_t = 1; i_t < nTimes; i_t++)
  {

    // Get current time, and time change
    double delta_t = AIFtimes[i_t] - AIFtimes[i_t - 1];

    //Compute combined arterial and vascular input for this time
    double Cp_t1 = (f_a*Ca_t[i_t] + f_v * Cv_t[i_t]);

    //Update the exponentials for the transfer terms in the two compartments
    double e_delta_pos = exp(-delta_t * K_pos);
    double e_delta_neg = exp(-delta_t * K_neg);

    //Use iterative trick to update the convolutions of transfers with the
    //input function
    double A_pos = (K_pos > KMAX) ? 0 :
      delta_t * 0.5 * (Cp_t1 + Cp_t0 * e_delta_pos);
    double A_neg = (K_neg > KMAX) ? 0 :
      delta_t * 0.5 * (Cp_t1 + Cp_t0 * e_delta_neg);

    Ft_pos = Ft_pos * e_delta_pos + A_pos;
    Ft_neg = Ft_neg * e_delta_neg + A_neg;

    //Combine the two compartments with the rate constant to get the final
    //concentration at this time point
    double C_t = F_neg * Ft_neg + F_pos * Ft_pos;

    //If for any reason this computes NaN, set to zero and bug out now
    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;
    Cp_t0 = Cp_t1;

  }
}

MDM_API double mdm_DCEModelDIBEM::checkParams()
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

MDM_API void mdm_DCEModelDIBEM::resetRerun()
{
  //Reset selected parameters to their initial value then rerun the optimisation
  std::vector<int> fixedParams = { 5, 6 };
  for (int i = 0; i < fixedParams.size(); i++)
    pkParams_[fixedParams[i]] = pkInitParams_[fixedParams[i]];
}
