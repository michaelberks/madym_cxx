/**
*  @file    mdm_DWIFitterIVIM.cxx
*  @brief   Implementation of the mdm_DWIFitterIVIM class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DWIFitterIVIM.h"

#include <stdio.h>                
#include <stdlib.h>                
#include <cmath>            /* For cos(), sin() and exp() */
#include <cassert>          /* For assert macro */

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

//
MDM_API mdm_DWIFitterIVIM::mdm_DWIFitterIVIM(const std::vector<double> &Bvals,
  bool fullModel, std::vector<double> BValsThresh)
  :
  mdm_DWIFitterBase(Bvals, { "S0", "d", "f", "dstar" }),
  ADCFitter_(Bvals, false),
  fullModel_(fullModel),
  BValsThresh_(BValsThresh)
{
  //Pre-initialise the alglib state
  int nParams = 4;
  std::vector<double> init = { 1, 1, 0.5, 1 };
  std::vector<double> lowerBounds = { 0, 1e-4, 0, 0 };
  std::vector<double> upperBounds = {1e6, 1e6, 1, 1e6};

  std::vector<double> scale = { 1, 1, 1, 1 };
  

  alglib::real_1d_array x;
  alglib::real_1d_array s;
  alglib::real_1d_array bndl;
  alglib::real_1d_array bndu;

  x.setcontent(nParams, init.data());
  s.setcontent(nParams, scale.data());
  bndl.setcontent(nParams, lowerBounds.data());
  bndu.setcontent(nParams, upperBounds.data());

  double epsg = 0.00000001;
  double epsf = 0.0000;
  double epsx = 0.0001;
#if _DEBUG
  alglib::ae_int_t maxits = maxIterations_ > 100 ? 100 : maxIterations_;
#else
  alglib::ae_int_t maxits = maxIterations_;
#endif

  alglib::minbccreate(x, state_);
  alglib::minbcsetbc(state_, bndl, bndu);
  alglib::minbcsetcond(state_, epsg, epsf, epsx, maxits);
  alglib::minbcsetscale(state_, s);
#if _DEBUG
  //REQUIRES UPDATE TO LATEST ALGLIB VERSION
  //Provides numerical check of analytic gradient, useful in debugging, but should not be
  //used in release versions
  //alglib::minbcoptguardsmoothness(state_);
  //alglib::minbcoptguardgradient(state_, 0.001);
#endif
}

//
MDM_API mdm_DWIFitterIVIM::~mdm_DWIFitterIVIM()
{

}

//
MDM_API mdm_ErrorTracker::ErrorCode mdm_DWIFitterIVIM::fitModel(
	std::vector<double> &params, double& ssr)
{
  auto fit = fitMultipleThresholds();
  params = fit.fitted_params;
  ssr = fit.ssr;

	return mdm_ErrorTracker::ErrorCode::OK;
}

//
MDM_API bool mdm_DWIFitterIVIM::setInputsFromStream(std::istream& ifs,
	const int nSignals)
{
	Bvals_.resize(nSignals);
	signals_.resize(nSignals);
	for (auto &b0 : Bvals_)
	{
		ifs >> b0;
		if (ifs.eof())
			return false;
	}
	for (auto &si : signals_)
		ifs >> si;

	return true;
}

//
MDM_API int mdm_DWIFitterIVIM::minimumInputs() const
{
	return 3;
}

//
MDM_API int mdm_DWIFitterIVIM::maximumInputs() const
{
	return 10;
}

//
/*
Signal from biexponential IVIM model.
  Can be used for generating signals or as a fitting objective function.

  Parameters
  ----------
  pars : bcfit Parameters object
  Contains S0(a.u.), D(mm ^ 2 / s), D* (mm ^ 2 / s), f(dimensionless) values.
  bvals : np array or list
  List of b - values(s / mm ^ 2).
  measured_signals : np array or list, optional
  Measured signals if, fitting.The default is None.
  model_type : string, optional
  Specify "full" or "simple" model.The default is "full".

  Returns
  ------ -
  s : np array
  Array of signals for given S0, D, D*, f, and b - values
  OR
  residuals : np array
  Array of differences between model and measured signals, if fitting.
*/
MDM_API double mdm_DWIFitterIVIM::modelToSignal(
	const std::vector<double> &params, double Bval)
{

  auto s0 = params[0];
  auto d = params[1];
  auto dstar = params[2];
  auto f = params[3];

  return s0 * ((1 - f) * std::exp(-d * Bval) + f * std::exp(-dstar * Bval));
}

MDM_API const std::vector<double> mdm_DWIFitterIVIM::modelToSignals(
  const std::vector<double>& params, const std::vector<double> B0s)
{
  std::vector<double> sigs;
  for (auto B0 : B0s)
    sigs.push_back(modelToSignal(params, B0));
  return sigs;
}

//**********************************************************************
//Private methods
//**********************************************************************

void mdm_DWIFitterIVIM::computeSignalGradient(
  const double& s0, const double& d, const double& f, const double& dstar,
  const double& B,
  double& signal,
  double& ds0, double& dd, double& df, double& ddstar)
{
  // Preliminary Calculations
  auto Ed = std::exp(-d * B);
  auto Edstar = fullModel_ ? std::exp(-dstar * B) : 0;

  //Partial derivative sof signal by model param
  ds0 = (1 - f) * Ed + f * Edstar;
  dd = s0 * (f - 1) * B * Ed;
  df = s0 * (Edstar - Ed);
  ddstar = -s0 * f * B * Edstar;

  //Signal
  signal = s0 * ds0;

  //What about bad values?
}

//
void mdm_DWIFitterIVIM::computeSSEGradient(
	const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad)
{
  const double& s0 = x[0];
  const double& d = x[1];
  const double& f = x[2];
  const double& dstar = fullModel_ ? x[3] : 0;
  auto n = signals_to_fit_.size();

  func = 0;
  grad[0] = 0;
  grad[1] = 0;
  grad[2] = 0;

  if (fullModel_)
    grad[3] = 0;

  double s, ds0, dd, df, ddstar;
  for (int i = 0; i < n; i++)
  {
    computeSignalGradient(
      s0, d, f, dstar, Bvals_to_fit_[i],
      s, ds0, dd, df, ddstar);
    double diff = s - signals_to_fit_[i];
    func += diff * diff;
    grad[0] += 2 * ds0 * diff;
    grad[1] += 2 * dd * diff;
    grad[2] += 2 * df * diff;
    if (fullModel_)
      grad[3] += 2 * ddstar * diff;
  }
}

mdm_ErrorTracker::ErrorCode mdm_DWIFitterIVIM::bcfitIVIM(
  const std::vector<double>& initParams,
  bcfitOutput &fit)
{
  //
  // First, we create optimizer object and tune its properties
  //
  alglib::real_1d_array x;
  int nParams = fullModel_ ? 4 : 3;
  x.setcontent(nParams, initParams.data());

  //
  fit.fitted_params.resize(4);

  // Optimize and evaluate results
  try
  {
    minbcrestartfrom(state_, x);
    minbcoptimize(state_, &computeSSEGradientAlglib, NULL, this);
    minbcresults(state_, x, rep_);
  }
  catch (alglib::ap_error e)
  {
    //setErrorValuesAndTidyUp(T1value, M0value);
    return mdm_ErrorTracker::DWI_FIT_FAIL;
  }
  int iterations = int(rep_.iterationscount);

  // Check for non-convergence
  if (iterations >= maxIterations_)
  {
    setErrorValuesAndTidyUp(fit.fitted_params);
    return mdm_ErrorTracker::DWI_MAX_ITER;
  }

  fit.fitted_params[0] = x[0]; //s0
  fit.fitted_params[1] = x[1]; //d
  fit.fitted_params[2] = x[2]; //f

  if (fullModel_)
    fit.fitted_params[3] = x[3]; //dstar

  alglib::real_1d_array g;
  g.setlength(nParams);
  computeSSEGradient(x, fit.ssr, g);
  return mdm_ErrorTracker::OK;
}

bcfitOutput setNan()
{
  bcfitOutput fit;
  for (auto& p : fit.fitted_params)
    p = NAN;
  fit.ssr = NAN;
  return fit;
}

void correctAic(bcfitOutput& fit)
{
  auto k = fit.nvarys;
  auto n = fit.ndata;
  if (n - k == 1)
    // Correction factor will involve division by 0...can't calculate AICc
    fit.aicc = NAN;
  else
  {
    auto corr_factor = 2 * k * (k + 1) / (n - k - 1);
    fit.aicc = fit.aic + corr_factor;
  }
}

double calculateRsq(const std::vector<double> sigs, double ssr)
{
  double sig_mean = 0;
  for (auto s : sigs)
    sig_mean += (s / sigs.size());

  double ss_diff_from_mean = 0;
  for (auto s : sigs)
  {
    auto diff_from_mean = s - sig_mean;
    ss_diff_from_mean += (s * s);
  }

  return 1 - (ssr / ss_diff_from_mean);
}


/*
Fit IVIM - biexponential decay with b-value.

  Parameters
  ----------
  bvals : np array or list
  List of b - values(s / mm ^ 2).
    signals: np array or list
  List of signal intensities to fit.
  bval_thresh : float / list
  Threshold for separating 'high' and 'low' b - values when determining
  starting values for fit(s / mm ^ 2).
  If a list is passed, each value is used to generate starting values,
  with the final result taken from the fit with the lowest sum of squares
  residuals
  model_type : string, optional
  Specify "full" or "simple" model.The default is "full".
  min_method : string, optional
  String specifying bcfit minimisation method.The default is 'nelder'.
  TODO : try different ways of initialising fits / contraining variables.
  The default is None.
  plot : string, optional
  Choose to plot the fit
  ylims : list, optional
  Specify ylims for plot.

  Returns
  ------ -
  fit_results : dict
  Dictionary of fitted parameters and goodness of fit metrics from
  bcfit MinimizerResult.
*/
bcfitOutput mdm_DWIFitterIVIM::fitMultipleThresholds()
{
  auto nBvals = Bvals_.size();

  // Only peform fit if all signals are non - zero
  for (auto s : signals_)
  {
    if (s <= 0)
    {
      return setNan();
    }   
  }

  // Loop over starting values generated for different thresholds
  double min_ssr = 1e10;
  std::vector<double> signals_fitted;
  std::vector<double> bvals_fitted;

  // Calculate starting values, depending on model_type
  
  // Get starting values from fit to subset of bvals
  // Use high bvals for S0 interceptand D starting value
  bcfitOutput best_fit;
  for (auto bthresh : BValsThresh_)
  {
    // Collect all starting values
    std::vector<double> starting_vals(4);

    std::vector<double> Bvals_hi;
    std::vector<double> signals_hi;

    std::vector<double> Bvals_lo;
    std::vector<double> signals_lo;

    double s0_meas = 0.0;

    for (size_t i_b = 0; i_b < nBvals; i_b++)
    {
      if (Bvals_[i_b] >= bthresh)
      {
        Bvals_hi.push_back(Bvals_[i_b]);
        signals_hi.push_back(signals_[i_b]);
      }
      else //Bvals_[i_b] < bthresh
      {
        Bvals_lo.push_back(Bvals_[i_b]);
        signals_lo.push_back(signals_[i_b]);
      }

      if (!Bvals_[i_b])
        s0_meas = signals_[i_b];
    }

    //Do ADC fit on high B-values
    double res;
    std::vector<double> initial_fit_high;
    ADCFitter_.setSignalsToFit(signals_hi);
    ADCFitter_.setBvalsToFit(Bvals_hi);
    ADCFitter_.fitModel(initial_fit_high, res);
      
    auto s0_inter = initial_fit_high[0]; //s0
    auto d_strt = initial_fit_high[1]; //adc
    double f_strt;

    if (fullModel_)
    {
      // Use low bvals for S0and D* starting values
      std::vector<double> initial_fit_low;
      
      //Do ADC fit on low B-values
      ADCFitter_.setSignalsToFit(signals_lo);
      ADCFitter_.setBvalsToFit(Bvals_lo);
      ADCFitter_.fitModel(initial_fit_low, res);

      auto s0_strt = initial_fit_low[0]; // "s0"
      auto dstar_strt = initial_fit_low[1];// "adc"

      // Starting value for f from ratio of two "S0" values
      // This is only valid if s0_inter < s0_strt, otherwise set f to 0
      f_strt = (s0_strt > s0_inter) ? 1 - s0_inter / s0_strt : 0;

      starting_vals[0] = s0_strt;
      starting_vals[3] = dstar_strt;

      signals_to_fit_ = signals_;
      Bvals_to_fit_ = Bvals_;
    }
    else
    {
      // Starting value for f from ratio of estimatedand measured s0
      f_strt = 1 - s0_inter / s0_meas;

      starting_vals[0] = s0_meas;

      signals_to_fit_ = signals_hi;
      Bvals_to_fit_ = Bvals_hi;
    }

    //Same for both and full and simple
    starting_vals[1] = d_strt;
    starting_vals[2] = f_strt;

    //Do LM fit based on these starting values:
    bcfitOutput fit;
    bcfitIVIM(starting_vals, fit);

    if (fit.ssr < min_ssr)
    {
      min_ssr = fit.ssr;
      best_fit = fit;
      signals_fitted = signals_to_fit_;
      bvals_fitted = Bvals_to_fit_;
    }
  }

  // Output
  if (!best_fit.success)
  {
    return setNan();
  } 
  
  // Calculate corrected AIC(AICc)
  //correctAic(best_fit);

  // Rsq calculation
  //best_fit.rsq = calculateRsq(signals_fitted, best_fit.ssr);
  return best_fit;
}
