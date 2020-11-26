/**
*  @file    mdm_T1FitterVFA.cxx
*  @brief   Implementation of the mdm_T1FitterVFA class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1FitterVFA.h"

#include <stdio.h>                
#include <stdlib.h>                
#include <cmath>            /* For cos(), sin() and exp() */
#include <cassert>          /* For assert macro */

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

const double mdm_T1FitterVFA::PI = acos(-1.0);

//
MDM_API mdm_T1FitterVFA::mdm_T1FitterVFA(const std::vector<double> &FAs, const double TR)
	:
	mdm_T1FitterBase(),
	delta_(1.0),
	FAs_(FAs),
	TR_(TR)
{
	if (!FAs.empty())
		initFAs();
}

//
MDM_API mdm_T1FitterVFA::~mdm_T1FitterVFA()
{

}

//
MDM_API mdm_T1FitterVFA::mdm_T1FitterVFA()
	: mdm_T1FitterVFA({}, 0)
{

}

//
MDM_API void mdm_T1FitterVFA::setFAs(const std::vector<double> &FAs)
{
	FAs_ = FAs;
	initFAs();
}

//
MDM_API void mdm_T1FitterVFA::setTR(const double TR)
{
	TR_ = TR;
}

MDM_API mdm_ErrorTracker::ErrorCode mdm_T1FitterVFA::fitT1(
	double &T1value, double &M0value)
{
	if (signals_.size() != nFAs_)
    throw mdm_exception(__func__, "NUmber of signals (" + std::to_string(signals_.size()) +
      ") does not match number of FAs (" + std::to_string(nFAs_) + ")");

	//
	// First, we create optimizer object and tune its properties
	//
	std::vector<double> init_vals = { 1000.0, signals_[0] * 30.0 };
	alglib::real_1d_array x;
	x.attach_to_ptr(2, init_vals.data());

	// Optimize and evaluate results
	try
	{
		mincgrestartfrom(state_, x);
		alglib::mincgoptimize(state_, &computeSSEGradientAlglib, NULL, this);
		mincgresults(state_, x, rep_);
	}
	catch (alglib::ap_error e)
	{
		setErrorValuesAndTidyUp("Error 2 - alglib:CG() failed", T1value, M0value);
		return mdm_ErrorTracker::T1_FIT_FAIL;
	}
	int iterations = int(rep_.iterationscount);


	// Check for non-convergence
	if (iterations >= maxIterations_)
	{
		setErrorValuesAndTidyUp("Error 3 - alglib:CG() hit max iterations", T1value, M0value);
		return mdm_ErrorTracker::T1_MAX_ITER;
	}

	// Check for crap fit or bonkers result
	if (x[0] < 0.0 || x[0] > 6000.0)
	{
		setErrorValuesAndTidyUp("Error 4 - Mad values", T1value, M0value);
		return mdm_ErrorTracker::T1_MAD_VALUE;
	}

	T1value = x[0];
	M0value = x[1];
	return mdm_ErrorTracker::OK;
}

//
MDM_API bool mdm_T1FitterVFA::setInputsFromStream(std::istream& ifs,
	const int nSignals)
{
	FAs_.resize(nSignals);
	signals_.resize(nSignals);
	for (auto &fa : FAs_)
	{
		ifs >> fa;
		if (ifs.eof())
			return false;

		fa *= (PI / 180);
	}
	for (auto &si : signals_)
		ifs >> si;

	initFAs();
	return true;
}

//
MDM_API void mdm_T1FitterVFA::setFixedScannerSettings(const std::vector<double> &settings)
{
  if (settings.size() != 1)
    throw mdm_exception(__func__, "Input settings has incorrect size (" + std::to_string(settings.size()) +
      "), exactly 1 fixed setting (TR) should be supplied for VFA mapping");
	setTR(settings[0]);
}

//
MDM_API void mdm_T1FitterVFA::setVariableScannerSettings(const std::vector<double> &settings)
{
	//Would be nice to assert size match to signals, but don't know if signals have been set yet?
	setFAs(settings);
}

//
MDM_API int mdm_T1FitterVFA::minimumInputs() const
{
	return 3;
}

//
MDM_API int mdm_T1FitterVFA::maximumInputs() const
{
	return 10;
}

//
MDM_API double mdm_T1FitterVFA::T1toSignal(
	const double T1, const double M0, const double FA, const double TR)
{
	double E1 = std::exp(-TR / T1);
	return M0 * std::sin(FA) * (1 - E1) / (1 - std::cos(FA) * E1);
}

//**********************************************************************
//Private methods
//**********************************************************************

void mdm_T1FitterVFA::computeSignalGradient(const double &T1, const double &M0,
	const double &cosFA, const double &sinFA,
	double &signal, double &signal_dT1, double &signal_dM0)
{
	// Preliminary Calculations
	double E = T1 ? exp(-TR_ / T1) : 0.0; //Only set if non-zero T1
	double A = 1.0 - E * cosFA;

	// signal intensity relationship - most efficient to compute
	//ds/dM0, then multiply to get s
	signal_dM0 = sinFA * (1 - E) / A;
	signal = M0 * signal_dM0;

	//Compute deriavtives with respect to T1 - only valid if T1 > 0
	if (T1)
		// First derivative - ds/dT1
		signal_dT1 = M0 * sinFA * TR_ * E * (cosFA-1)
			/ (A * A * T1 * T1);
	else
		signal_dT1 = 1000000000.0; // something very big
}

void mdm_T1FitterVFA::computeSSEGradient(
	const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad)
{
	const double &T1 = x[0];
	const double &M0 = x[1];
	func = 0;
	grad[0] = 0;
	grad[1] = 0;
	double s, s_dT1, s_dM0;
	for (int i = 0; i < nFAs_; i++)
	{
		computeSignalGradient(T1, M0, cosFAs_[i], sinFAs_[i],
			s, s_dT1, s_dM0);
		double diff = s - signals_[i];
		func += diff * diff;
		grad[0] += 2 * s_dT1*diff;
		grad[1] += 2 * s_dM0*diff;
	}
}

//
void mdm_T1FitterVFA::initFAs()
{
	nFAs_ = int(FAs_.size());
  if (nFAs_ < minimumInputs())
    throw mdm_exception(__func__, "Fewer FAs (" + std::to_string(nFAs_) +
      ") than minimum required (" + std::to_string(minimumInputs()) + ")");

  if (nFAs_ > maximumInputs())
    throw mdm_exception(__func__, "More FAs (" + std::to_string(nFAs_) +
      ") than maximum allowed (" + std::to_string(maximumInputs()) + ")");


	cosFAs_.resize(nFAs_);
	sinFAs_.resize(nFAs_);
	for (int i = 0; i < nFAs_; i++)
	{
		cosFAs_[i] = std::cos(delta_*FAs_[i]);
		sinFAs_[i] = std::sin(delta_*FAs_[i]);
	}
}