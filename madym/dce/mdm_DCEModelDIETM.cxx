/**
*  @file    mdm_DCEModelDIETM.cxx
*  @brief   Implementation of mdm_DCEModelDIETM class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelDIETM.h"
#include <cmath>

const double mdm_DCEModelDIETM::ETM_KEPMAX = 42.0;

MDM_API mdm_DCEModelDIETM::mdm_DCEModelDIETM(
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
  //Default values specific to tofts model
  if (pkParamNames_.empty())
    pkParamNames_ = { "Ktrans", "v_e", "v_p", "f_a", "tau_a", "tau_v" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.2, 0.2, 0.20 , 0.5, 0.0, 0.0 }; //{kin_, ve_, offset_, vp_, kout_}
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 1e-20, 1e-20, 0.0, 0.0, 0.0, -0.5};
  if (upperBounds_.empty())
    upperBounds_ = { 10.0, 10.0, 10.0, 1.0, 0.5, 0.5};

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelDIETM::~mdm_DCEModelDIETM()
{

}

MDM_API std::string mdm_DCEModelDIETM::modelType() const
{
  return "mdm_DCEModelDIETM";
}

MDM_API void mdm_DCEModelDIETM::computeCtModel(size_t nTimes)
{
  //Reset all the model concentrations to 0
  for (size_t i_t = 0; i_t < nTimes; i_t++)
    CtModel_[i_t] = 0;

  for (const double& param : pkParams_)
  {
    if (std::isnan(param))
      return;
  }

  const double &kTrans = pkParams_[0];
  const double &ve = pkParams_[1];
  const double &vp = pkParams_[2];
  const double &f_a = pkParams_[3];
  const double &tau_a = pkParams_[4];
  const double &tau_v = pkParams_[5];

  //Resample AIF and get AIF times
  AIF_.resample_AIF( tau_a);
  AIF_.resample_PIF( tau_v, false, true);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> Cv_t = AIF_.PIF();
  const std::vector<double> &t = AIF_.AIFTimes();
  double f_v = 1 - f_a; // estimate of hepatic portal venous fraction

  if (ve == 0.0 || kTrans == 0.0)
  {
    for (size_t i_t = 0; i_t < nTimes; i_t++)
      CtModel_[i_t] = vp * (f_a*Ca_t[i_t] + f_v*Cv_t[i_t]);
    return;
  }

  double integral = 0.0;
  double kep = kTrans / ve;

	double Cp_t0 = (f_a*Ca_t[0] + f_v * Cv_t[0]);
	CtModel_[0] = vp * Cp_t0;
  for (size_t i_t = 1; i_t < nTimes; i_t++)
  {
		//Get average of AIF and PIF, weighted by their respective fractions
    double Cp_t1 = (f_a*Ca_t[i_t] + f_v * Cv_t[i_t]);

    double delta_t = t[i_t] - t[i_t - 1];
    double e_delta = std::exp(-kep * delta_t);
    double A = delta_t * 0.5 * (Cp_t1 + Cp_t0 * e_delta);

    integral = integral*e_delta + A;
    double C_t = vp * Cp_t1 + kTrans * integral;
    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;
    Cp_t0 = Cp_t1;
  }
}

MDM_API void mdm_DCEModelDIETM::checkParams()
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

	//Now do ETM specific checks
	const double &ve = pkParams_[1];
	const double &vp = pkParams_[3];
	//const double &kout = pkParams_[4];
	double vePlusVp = ve + vp;
	//double kep = kout / ve;
	if ((vePlusVp > 1.0) )//|| (kep <= ETM_KEPMAX))
		errorCode_ = mdm_ErrorTracker::DCE_INVALID_PARAM;

	errorCode_ = mdm_ErrorTracker::OK;
	return;
}

