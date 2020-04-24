/**
*  @file    mdm_DCEModelAUEM.cxx
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelAUEM.h"
#include <cmath>

MDM_API mdm_DCEModelAUEM::mdm_DCEModelAUEM(
  mdm_AIF &AIF,
  const std::vector<std::string> &pkParamNames,
  const std::vector<double> &pkInitParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
	const std::vector<int> &relativeLimitParams,
	const std::vector<double> &relativeLimitValues)
	:mdm_DCEModelBase(
		AIF, pkParamNames, pkInitParams,
		fixedParams, fixedValues,
		relativeLimitParams,
		relativeLimitValues)
{
  if (pkParamNames_.empty())
    pkParamNames_ = { "Fp", "ve", "ki", "kef", "fa", "aoffset", "voffset" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.6,  0.2,  0.2,   0.1,  0.5,      0.025,     0.00 };//{"Fp", "vecs", "kin", "kout", "fa"}
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

MDM_API void mdm_DCEModelAUEM::computeCtModel(int nTimes)
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
  const double &F_p = pkParams_[0];// flow plasma rate
  const double &v_ecs = pkParams_[1];//extravascular, extracellular space
  const double &k_i = pkParams_[2];//transport constant to liver tissue
  const double &k_ef = pkParams_[3];//efflux to bile duct transfer constant
  const double &f_a = pkParams_[4];//the arterial fraction
  const double &aoffset = pkParams_[5];//the arterial fraction
  const double &voffset = pkParams_[6];//the arterial fraction

  const double TMIN = 1e-9;

  //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
  //Resample AIF and get AIF times
  AIF_.resample_AIF(nTimes, aoffset);
  AIF_.resample_PIF(nTimes, voffset, false, true);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> Cv_t = AIF_.PIF();
  const std::vector<double> &AIFtimes = AIF_.AIFTimes();

  //Compute derived parameters from input parameters
  double T_e = v_ecs / (F_p + k_i); // extracellular mean transit time
  double v_i = 1 - v_ecs; // estimate of intracellular volume
  double T_i = v_i / k_ef; // intracellular mean transit time
  double E_i = k_i / (F_p + k_i); // the hepatic uptake function

  double f_v = 1 - f_a; // estimate of hepatic portal venous fraction

  double  ETie = E_i / (1 - T_e / T_i);//This can also be precomputed

  // Let's rewrite the convolution sum, using the exponential "trick" so 
  //we can compute everything in one forward loop
  double  Fi_t = 0;
  double Fe_t = 0;
  CtModel_[0] = 0;
  double Cp_t0 = (f_a*Ca_t[0] + f_v * Cv_t[0]);

  for (int i_t = 1; i_t < nTimes; i_t++)
  {
    // Get current time, and time change
    double delta_t = AIFtimes[i_t] - AIFtimes[i_t-1];

    //Compute combined arterial and vascular input for this time
    double Cp_t1 = (f_a*Ca_t[i_t] + f_v * Cv_t[i_t]);

    //Update the exponentials for the transfer terms in the two compartments
    double et_i = exp(-delta_t / T_i);
    double et_e = exp(-delta_t / T_e);

    //Use iterative trick to update the convolutions of transfers with the
    //input function
    double A_i = (T_i < TMIN) ? 0 :
      delta_t * 0.5 * (Cp_t1 + Cp_t0 * et_i);
    double A_e = (T_e < TMIN) ? 0 :
      delta_t * 0.5 * (Cp_t1 + Cp_t0 * et_e);

    Fi_t = Fi_t * et_i + A_i;
    Fe_t = Fe_t * et_e + A_e;

    //Combine the two compartments with the rate constant to get the final
    //concentration at this time point
    double C_t = F_p * (ETie*Fi_t + (1 - ETie)*Fe_t);
    if (std::isnan(C_t))
      return;
    CtModel_[i_t] = C_t;
    Cp_t0 = Cp_t1;
  }

  /*
  const double delta_t = AIFtimes[1] - AIFtimes[0];
  for (int i_t = 0; i_t < nTimes; i_t++)
  {
    //Generally I don't like single letter parameter names, but to be consistent with
    //the equations published in Leo's paper, use t as alias for dynamic times
    const double &t = AIFtimes[i_t];
    const double ex_i = std::exp(-t / T_i);
    const double ex_e = std::exp(-t / T_e);

    double ETie = E_i / (1 - T_e / T_i);

    //Make combined input function
    Cp_t[i_t] = (f_a*Ca_t[i_t] + f_v * Cv_t[i_t]);

    //Compute the exchange term for the concentration time series
    Cx_t[i_t] = F_p * (ETie*ex_i + (1 - ETie)*ex_e);

    //Finally convolve the exchange time series with the combined input function
    double Cl_t_sum = 0.0; //conv(Cx_t, Cp_t);
    for (int j_t = 0; j_t <= i_t; j_t++)
    {
      Cl_t_sum += Cx_t[j_t] * Cp_t[i_t - j_t];
    }
    CtModel_[i_t] = Cl_t_sum * delta_t;

  }*/
}

MDM_API double mdm_DCEModelAUEM::checkParams()
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

MDM_API void mdm_DCEModelAUEM::resetRerun()
{
  //Reset selected parameters to their initial value then rerun the optimisation
  std::vector<int> fixedParams = { 5, 6 };
  for (int i = 0; i < fixedParams.size(); i++)
    pkParams_[fixedParams[i]] = pkInitParams_[fixedParams[i]];
}
