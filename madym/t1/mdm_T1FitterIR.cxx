/**
*  @file    mdm_T1FitterIR.cxx
*  @brief   Implementation of the mdm_T1FitterIR class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1FitterIR.h"

#include <stdio.h>                
#include <stdlib.h>                
#include <cmath>            /* For cos(), sin() and exp() */
#include <cassert>          /* For assert macro */

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>
//
MDM_API mdm_T1FitterIR::mdm_T1FitterIR(const std::vector<double> &TIs, const double TR)
  :
  mdm_T1FitterBase(),
  TIs_(TIs),
  TR_(TR)
{
#if _DEBUG
	//Provides numerical check of analytic gradient, useful in debugging, but should not be
	//used in release versions
	alglib::mincgoptguardsmoothness(state_);
	alglib::mincgoptguardgradient(state_, 0.001);
#endif
}

//
MDM_API mdm_T1FitterIR::~mdm_T1FitterIR()
{

}

//
MDM_API void mdm_T1FitterIR::setTIs(const std::vector<double> &TIs)
{
	TIs_ = TIs;
}

//
MDM_API void mdm_T1FitterIR::setTR(const double TR)
{
	TR_ = TR;
}

//
MDM_API void mdm_T1FitterIR::setInputs(const std::vector<double> &inputs)
{
  if (inputs.size() < minimumInputs())
    throw mdm_exception(__func__, "Fewer input signals (" + std::to_string(inputs.size()) +
      ") than minimum required (" + std::to_string(minimumInputs()) + ")");

  if (inputs.size() > maximumInputs())
    throw mdm_exception(__func__, "More input signals (" + std::to_string(inputs.size()) +
      ") than maximum allowed (" + std::to_string(maximumInputs()) + ")");
  
  //Inputs are just signals
  signals_ = inputs;
}

//
MDM_API mdm_ErrorTracker::ErrorCode mdm_T1FitterIR::fitT1(
	double &T1value, double &M0value)
{
	if (signals_.size() != TIs_.size())
    throw mdm_exception(__func__, "Number of signals (" + std::to_string(signals_.size()) +
      ") does not match number of TIs (" + std::to_string(TIs_.size()) + ")");

	//
	// First, we create optimizer object and tune its properties
	//
	std::vector<double> init_vals = { 1000.0, signals_.back() * 5.0 };
	alglib::real_1d_array x;
	x.attach_to_ptr(2, init_vals.data());

	// Optimize and evaluate results
	try
	{
		mincgrestartfrom(state_, x);
		alglib::mincgoptimize(state_, &computeSSEGradientAlglib, NULL, this);
		mincgresults(state_, x, rep_);

#if _DEBUG
		//
		// Check that OptGuard did not report errors
		//
		alglib::optguardreport ogrep;
		alglib::mincgoptguardresults(state_, ogrep);
		std::cout << "Optimisation guard results:\n";
		std::cout << "Bad gradient suspected:" << (ogrep.badgradsuspected ? "true" : "false") << "\n"; // EXPECTED: false
		std::cout << "Non c0 suspected:" << (ogrep.nonc0suspected ? "true" : "false") << "\n"; // EXPECTED: false
		std::cout << "Non c1 suspected:" << (ogrep.nonc1suspected ? "true" : "false") << "\n"; // EXPECTED: false
#endif
	}
	catch (alglib::ap_error e)
	{
		setErrorValuesAndTidyUp(T1value, M0value);
		return mdm_ErrorTracker::T1_FIT_FAIL;
	}
	int iterations = int(rep_.iterationscount);


	// Check for non-convergence
	if (iterations >= maxIterations_)
	{
		setErrorValuesAndTidyUp(T1value, M0value);
		return mdm_ErrorTracker::T1_MAX_ITER;
	}

	// Check for crap fit or bonkers result
	if (x[0] < 0.0 || x[0] > 6000.0)
	{
		setErrorValuesAndTidyUp(T1value, M0value);
		return mdm_ErrorTracker::T1_MAD_VALUE;
	}

	T1value = x[0];
	M0value = x[1];
	return mdm_ErrorTracker::OK;
}

//
MDM_API bool mdm_T1FitterIR::setInputsFromStream(std::istream& ifs,
	const int nSignals)
{
	TIs_.resize(nSignals);
	signals_.resize(nSignals);
	for (auto &ti : TIs_)
	{
		ifs >> ti;
		if (ifs.eof())
			return false;

	}
	for (auto &si : signals_)
		ifs >> si;

  return true;
}

//
MDM_API int mdm_T1FitterIR::minimumInputs() const
{
	return 3;
}

//
MDM_API int mdm_T1FitterIR::maximumInputs() const
{
	return 50;
}

//
MDM_API double mdm_T1FitterIR::T1toSignal(
	const double T1, const double M0, const double TI, const double TR)
{
	double E_TI = std::exp(-TI / T1);
  double E_TR = std::exp(-TR / T1);

  return std::abs(M0 * (1 - 2*E_TI + E_TR));
}

//**********************************************************************
//Private methods
//**********************************************************************

void mdm_T1FitterIR::computeSignalGradient(const double &T1, const double &M0,
	const double &TI,
	double &signal, double &signal_dT1, double &signal_dM0)
{
  //Signal model is:
  //S = | M0*(1 - 2*exp(-TI/T1) + e(-TR/T1) ) |

  //If M0 or T1 are 0, the signal is 0 and the signal model is non-differentiable
  if (!M0 || !T1)
  {
    signal = 0;
    signal_dT1 = 1000000000.0; // something very big
    signal_dM0 = 1000000000.0; // something very big
    return;
  }

	// Preliminary Calculations - T1, M0 both > 0
	double E_TI = exp(-TI / T1);
  double E_TR = exp(-TR_ / T1);

	// signal intensity relationship - most efficient to compute
	//ds/dM0, then multiply to get s
	signal_dM0 = 1 - 2*E_TI + E_TR;
	signal = M0 * signal_dM0;

	//Compute deriavtives with respect to T1
	// First derivative - ds/dT1
	signal_dT1 = M0 * (-2*E_TI*TI + E_TR*TR_) / (T1*T1);

  //We haven't taken in to account the absolute operator on the signal model yet
  //If the signal is negative, invert the sign on the signal and the two
  //partial derivatives
  if (signal < 0)
  {
    signal *= -1;
    signal_dM0 *= -1;
    signal_dT1 *= -1;
  }

	
}

void mdm_T1FitterIR::computeSSEGradient(
	const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad)
{
	const double &T1 = x[0];
	const double &M0 = x[1];
	func = 0;
	grad[0] = 0;
	grad[1] = 0;
	double s, s_dT1, s_dM0;
	for (int i = 0; i < TIs_.size(); i++)
	{
		computeSignalGradient(T1, M0, TIs_[i],
			s, s_dT1, s_dM0);
		double diff = s - signals_[i];
		func += diff * diff;
		grad[0] += 2 * s_dT1*diff;
		grad[1] += 2 * s_dM0*diff;
	}
}