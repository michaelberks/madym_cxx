/**
*  @file    mdm_DCEModelMLDRW.cxx
*  @brief   Implementation of mdm_DCEModelMLDRW class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelMLDRW.h"
#include <madym/dce/mdm_Exponentials.h>
#include <cmath>

const double mdm_DCEModelMLDRW::ETM_KEPMAX = 42.0;
const double mdm_DCEModelMLDRW::PI = acos(-1.0);

MDM_API mdm_DCEModelMLDRW::mdm_DCEModelMLDRW(
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
    pkParamNames_ = { "alpha", "kappa", "MTT", "Ktrans", "kep"};
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.2, 0.2, 0.2, 0.2, 0.2}; //
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0, 0.0, 0.0 };
  if (upperBounds_.empty())
    upperBounds_ = { 100.0, 100.0, 100.0, 100.0, 100.0 };

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelMLDRW::~mdm_DCEModelMLDRW()
{

}

MDM_API std::string mdm_DCEModelMLDRW::modelType() const
{
  return "mdm_DCEModelMLDRW";
}

MDM_API void mdm_DCEModelMLDRW::computeCtModel(size_t nTimes)
{
  //Reset all the model concentrations to 0
  for (size_t i_t = 0; i_t < nTimes; i_t++)
    CtModel_[i_t] = 0;

  for (const double& param : pkParams_)
  {
    if (std::isnan(param))
      return;
  }

  const double &alpha = pkParams_[0];
  const double& kappa = pkParams_[1];
  const double& MTT = pkParams_[2];
  const double &Ktrans =   pkParams_[3];
  const double &kep =       pkParams_[4];

  //Resample AIF and get AIF times (I don't usually like single letter variables
  //but to be consistent with paper formulae, use AIF times = t)
  std::vector<double> Ca_t(nTimes, 0.0);
  const std::vector<double> &t = AIF_.AIFTimes();

  double  integral = 0.0;
  //double kep = Ktrans / v_e;

  Ca_t[0] = 0;
  for (size_t i_t = 1; i_t < nTimes; i_t++)
  {
    Ca_t[i_t] = IF(alpha, kappa, MTT, t[i_t]);

    double delta_t = t[i_t] - t[i_t - 1];
    double e_delta = std::exp(-kep * delta_t);
    double A = delta_t * 0.5 * (Ca_t[i_t] + Ca_t[i_t - 1] * e_delta);

    integral = integral*e_delta + A;
    double C_t = Ca_t[i_t] + Ktrans * integral;

    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;
  }
}

MDM_API void mdm_DCEModelMLDRW::checkParams()
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

	errorCode_ = mdm_ErrorTracker::OK;
}

double mdm_DCEModelMLDRW::IF(double alpha, double kappa, double MTT, double t)
{
  return alpha* std::sqrt(kappa / (2 * PI * t))*
    std::exp(-kappa * std::pow(t - MTT, 2.0) / (2 * t));
}