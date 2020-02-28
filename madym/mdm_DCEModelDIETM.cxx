/**
*  @file    mdm_DCEModelDIETM.cxx
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelDIETM.h"
#include <cmath>

const double mdm_DCEModelDIETM::ETM_KEPMAX = 42.0;

MDM_API mdm_DCEModelDIETM::mdm_DCEModelDIETM(
  mdm_AIF &AIF,
  const std::vector<std::string> &pkParamNames,
  const std::vector<double> &pkInitParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues)
  :mdm_DCEModelBase(AIF, pkParamNames, pkInitParams, fixedParams, fixedValues)
{
  //Default values specific to tofts model
  if (pkParamNames_.empty())
    pkParamNames_ = { "ktrans", "ve", "vp", "fa", "aoffset", "voffset" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.2, 0.2, 0.20 , 0.5, 0.0, 0.0 }; //{kin_, ve_, offset_, vp_, kout_}
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 1e-20, 1e-20, 0.0, 0.0, 0.0, -0.5};
  if (upperBounds_.empty())
    upperBounds_ = { 10.0, 10.0, 10.0, 1.0, 0.5, 0.5};

  mdm_DCEModelBase::init(fixedParams, fixedValues);
}

MDM_API mdm_DCEModelDIETM::~mdm_DCEModelDIETM()
{

}

MDM_API std::string mdm_DCEModelDIETM::modelType() const
{
  return "mdm_DCEModelDIETM";
}

MDM_API void mdm_DCEModelDIETM::computeCtModel(int nTimes)
{
  //Reset all the model concentrations to 0
  for (int i_t = 0; i_t < nTimes; i_t++)
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
  const double &aoffset = pkParams_[4];
  const double &voffset = pkParams_[5];

  //Resample AIF and get AIF times
  AIF_.resample_AIF(nTimes, aoffset);
  AIF_.resample_PIF(nTimes, voffset, false, true);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> Cv_t = AIF_.PIF();
  const std::vector<double> &t = AIF_.AIFTimes();
  double f_v = 1 - f_a; // estimate of hepatic portal venous fraction

  if (ve == 0.0 || kTrans == 0.0)
  {
    for (int i_t = 0; i_t < nTimes; i_t++)
      CtModel_[i_t] = vp * (f_a*Ca_t[i_t] + f_v*Cv_t[i_t]);
    return;
  }

  double integral = 0.0;
  double kep = kTrans / ve;

	double Cp_t0 = (f_a*Ca_t[0] + f_v * Cv_t[0]);
	CtModel_[0] = vp * Cp_t0;
  for (int i_t = 1; i_t < nTimes; i_t++)
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

MDM_API double mdm_DCEModelDIETM::checkParams()
{ 
	//First check all finite and not NaN
	for (double p : pkParams_)
	{
		if (std::isnan(p) || !std::isfinite(p))
		{
			errorCode_ = mdm_ErrorTracker::DCE_FIT_FAIL;
			return BAD_FIT_SSD;
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
	return 0;
}

MDM_API void mdm_DCEModelDIETM::resetRerun()
{
  //Reset selected parameters to their initial value then rerun the optimisation
  std::vector<int> fixedParams = { 4, 5 };
  for (int i = 0; i < fixedParams.size(); i++)
    pkParams_[fixedParams[i]] = pkInitParams_[fixedParams[i]];
}

