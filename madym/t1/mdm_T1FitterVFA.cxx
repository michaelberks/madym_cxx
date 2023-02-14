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

#include "opt/interpolation.h"

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

const double mdm_T1FitterVFA::PI = acos(-1.0);

//
MDM_API mdm_T1FitterVFA::mdm_T1FitterVFA(const std::vector<double> &FAs, const double TR, const bool usingB1)
  :
  mdm_T1FitterBase(),
  B1_(1.0),
  FAs_(FAs),
  TR_(TR),
  usingB1_(usingB1)
{
	if (!FAs_.empty())
		initFAs();

	//Pre-initialise the alglib state
	//Pre-initialise the alglib state
	int nParams = 2;
	std::vector<double> init = { 1000, 1000 };
	std::vector<double> scale = { 1000, 1000 };

	alglib::real_1d_array x;
	alglib::real_1d_array s;

	x.setcontent(nParams, init.data());
	s.setcontent(nParams, scale.data());

	double epsg = 0.00000001;
	double epsf = 0.0000;
	double epsx = 0.0001;
#if _DEBUG
	alglib::ae_int_t maxits = maxIterations_ > 100 ? 100 : maxIterations_;
#else
	alglib::ae_int_t maxits = maxIterations_;
#endif

	alglib::mincgcreate(x, state_);
	alglib::mincgsetcond(state_, epsg, epsf, epsx, maxits);
	alglib::mincgsetscale(state_, s);

#if _DEBUG
	//Provides numerical check of analytic gradient, useful in debugging, but should not be
	//used in release versions
	alglib::mincgoptguardsmoothness(state_);
	alglib::mincgoptguardgradient(state_, 0.001);
#endif
}

//
MDM_API mdm_T1FitterVFA::~mdm_T1FitterVFA()
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

//
MDM_API void mdm_T1FitterVFA::setB1(const double B1)
{
  B1_ = B1;
}

//
MDM_API void mdm_T1FitterVFA::setInputs(const std::vector<double> &inputs)
{
  if (inputs.size() < minimumInputs())
    throw mdm_exception(__func__, "Fewer input signals (" + std::to_string(inputs.size()) +
      ") than minimum required (" + std::to_string(minimumInputs()) + ")");

  if (inputs.size() > maximumInputs())
    throw mdm_exception(__func__, "More input signals (" + std::to_string(inputs.size()) +
      ") than maximum allowed (" + std::to_string(maximumInputs()) + ")");

  if (usingB1_)
  {
    //First n-1 inputs are signals
    auto n = inputs.size();
    signals_.clear();
    for (size_t i = 0; i < n - 1; i++)
      signals_.push_back(inputs[i]);

    //Last input is B1
    B1_ = inputs.back();

    //Reinitialise cos and sin FA given new B1 correction value
    initFAs();
  }
  else
    //Inputs are just signals
    signals_ = inputs;
}

//
MDM_API mdm_ErrorTracker::ErrorCode mdm_T1FitterVFA::fitT1(
	double &T1value, double &M0value, double& EWvalue)
{
	if (signals_.size() != nFAs_)
    throw mdm_exception(__func__, "Number of signals (" + std::to_string(signals_.size()) +
      ") does not match number of FAs (" + std::to_string(nFAs_) + ")");

	//
	// First, we create optimizer object and tune its properties
	//

	//Do linear fit to initialise T1 and M0
	linearFit(T1value, M0value);

	std::vector<double> init_vals = { T1value, M0value };
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
	if (x[0] < 0.0 || x[0] > 10000.0)
	{
		setErrorValuesAndTidyUp(T1value, M0value);
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

  if (usingB1_)
    ifs >> B1_;

	initFAs();
	return true;
}

//
MDM_API int mdm_T1FitterVFA::minimumInputs() const
{
	return 3;
}

//
MDM_API int mdm_T1FitterVFA::maximumInputs() const
{
	return 50;
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
		cosFAs_[i] = std::cos(B1_*FAs_[i]);
		sinFAs_[i] = std::sin(B1_*FAs_[i]);
	}
}

//
void mdm_T1FitterVFA::linearFit(double& T1, double& M0)
{
	alglib::real_1d_array x;
	alglib::real_1d_array y;

	x.setlength(nFAs_);
	y.setlength(nFAs_);
	for (size_t i = 0; i < nFAs_; i++)
	{
		y[i] = signals_[i] / sinFAs_[i];
		x[i] = cosFAs_[i] * y[i];
	}

	alglib::ae_int_t info;
	alglib::barycentricinterpolant pi;
	alglib::real_1d_array p;
	alglib::polynomialfitreport rep;

	//
	// Fitting without individual weights
	//
	// NOTE: result is returned as barycentricinterpolant structure.
	//       if you want to get representation in the power basis,
	//       you can use barycentricbar2pow() function to convert
	//       from barycentric to power representation (see docs for 
	//       POLINT subpackage for more info).
	//
	polynomialfit(x, y, 2, info, pi, rep);
	polynomialbar2pow(pi, p);

	auto E1 = p[1];
	M0 = p[0] / (1 - E1);
	T1 = -TR_ / std::log(p[1]);
}