/**
*  @file    mdm_DCEModelETM.cxx
*  @brief   Implementation of mdm_DCEModelETM class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelETM.h"
#include <madym/mdm_Exponentials.h>
#include <cmath>

const double mdm_DCEModelETM::ETM_KEPMAX = 42.0;

MDM_API mdm_DCEModelETM::mdm_DCEModelETM(
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
  //Default values specific to tofts model
  if (pkParamNames_.empty())
    pkParamNames_ = { "Ktrans", "v_e", "v_p", "tau_a" };
  if (pkInitParams_.empty())
    pkInitParams_ = { 0.2, 0.2, 0.2, 0.0}; //
  if (optParamFlags_.empty())
    optParamFlags_ = { true, true, true, true };
  if (lowerBounds_.empty())
    lowerBounds_ = { 0.0, 0.0, 0.0, 0.0 };
  if (upperBounds_.empty())
    upperBounds_ = { 10.0, 1.0, 1.0, 0.5 };

  mdm_DCEModelBase::init(fixedParams, fixedValues, relativeLimitParams, relativeLimitValues);
}

MDM_API mdm_DCEModelETM::~mdm_DCEModelETM()
{

}

MDM_API std::string mdm_DCEModelETM::modelType() const
{
  return "mdm_DCEModelETM";
}

MDM_API void mdm_DCEModelETM::computeCtModel(size_t nTimes)
{
  //Reset all the model concentrations to 0
  for (size_t i_t = 0; i_t < nTimes; i_t++)
    CtModel_[i_t] = 0;

  for (const double& param : pkParams_)
  {
    if (std::isnan(param))
      return;
  }

  const double &Ktrans =   pkParams_[0];
  const double &v_e =       pkParams_[1];
  const double &v_p =       pkParams_[2];
  const double &tau_a =  pkParams_[3];

  //Resample AIF and get AIF times (I don't usually like single letter variables
  //but to be consistent with paper formulae, use AIF times = t)
  AIF_.resample_AIF( tau_a);
  const std::vector<double> Ca_t = AIF_.AIF();
  const std::vector<double> &t = AIF_.AIFTimes();

  if (v_e == 0.0 || Ktrans == 0.0)
  {
    for (size_t i = 0; i < nTimes; i++)
      CtModel_[i] = v_p * Ca_t[i];
    return;
  }

  double  integral = 0.0;
  double kep = Ktrans / v_e;

  CtModel_[0] = v_p * Ca_t[0];
  for (size_t i_t = 1; i_t < nTimes; i_t++)
  {
    double delta_t = t[i_t] - t[i_t - 1];
    double e_delta = std::exp(-kep * delta_t);
    double A = delta_t * 0.5 * (Ca_t[i_t] + Ca_t[i_t - 1] * e_delta);

    integral = integral*e_delta + A;
    double C_t = v_p * Ca_t[i_t] + Ktrans * integral;

    if (std::isnan(C_t))
      return;

    CtModel_[i_t] = C_t;
  }
}

MDM_API void mdm_DCEModelETM::checkParams()
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
	const double &v_e = pkParams_[1];
	const double &v_p = pkParams_[3];
	//const double &kout = pkParams_[4];
	double vePlusVp = v_e + v_p;
	//double kep = kout / v_e;
	if ((vePlusVp > 1.0) )//|| (kep <= ETM_KEPMAX))
		errorCode_ = mdm_ErrorTracker::DCE_INVALID_PARAM;
	
	errorCode_ = mdm_ErrorTracker::OK;
}    

MDM_API std::vector<double> mdm_DCEModelETM::makeLLSMatrix(const std::vector<double>& Ct_sig) const
{
  //throw mdm_exception(__func__, boost::format(
  //  "Model (%1%) supports LLS solving but it has not yet been implemented")
  //  % modelType());

  auto tau_a = pkParams_[3];
  AIF_.resample_AIF(tau_a);
  const auto& Cp_t = AIF_.AIF();
  const auto& t = AIF_.AIFTimes();

  auto  n_t = t.size();
  std::vector<double> A(n_t * 3);

  auto Cp_t_int = mdm_Exponentials::trapz_integral(Cp_t, t);
  auto Ctis_t_int = mdm_Exponentials::trapz_integral(Ct_sig, t);

  size_t curr_pt = 0;
  for (size_t i_row = 0; i_row < n_t; i_row++)
  {
    A[curr_pt++] = Cp_t_int[i_row];
    A[curr_pt++] = -Ctis_t_int[i_row];
    A[curr_pt++] = Cp_t[i_row];
  }

  return A;
}

MDM_API void mdm_DCEModelETM::transformLLSolution(const double* B)
{
  double& Ktrans = pkParams_[0];
  double& v_e = pkParams_[1];
  double& v_p = pkParams_[2];

  auto k_2 = B[1];
  v_p = B[2];
  Ktrans = B[0] - k_2 * v_p;
  v_e = Ktrans / k_2;
}