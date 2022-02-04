/**
*  @file    mdm_DWIFitterADC.cxx
*  @brief   Implementation of the mdm_DWIFitterADC class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DWIFitterADC.h"

#include <stdio.h>                
#include <stdlib.h>                
#include <cmath>            /* For cos(), sin() and exp() */
#include <cassert>          /* For assert macro */

#include <madym/opt/interpolation.h>
#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

//
MDM_API mdm_DWIFitterADC::mdm_DWIFitterADC(const std::vector<double>& Bvals, bool linearFit)
	:
	mdm_DWIFitterBase(Bvals, { "S0", "ADC" }),
	linearFit_(linearFit)
{
	//Pre-initialise the alglib state
	if (!linearFit_)
	{
		int nParams = 2;
		std::vector<double> init = { 100, 1e-3 };
		std::vector<double> lowerBounds = { 0, 1e-4 };
		std::vector<double> upperBounds = { 1e6, 1e6 };

		std::vector<double> scale = { 100, 1e-3 };


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
		//Provides numerical check of analytic gradient, useful in debugging, but should not be
		//used in release versions
		alglib::minbcoptguardsmoothness(state_);
		alglib::minbcoptguardgradient(state_, 0.001);
#endif
	}
	
}

//
MDM_API mdm_DWIFitterADC::~mdm_DWIFitterADC()
{

}

//
MDM_API mdm_ErrorTracker::ErrorCode mdm_DWIFitterADC::fitModel(
	std::vector<double>& params, double &ssr)
{
	//Resize output appropriately
	params.resize(2);

	// Only peform fit if all signals are non - zero
	for (auto s : signals_to_fit_)
	{
		if (s <= 0)
		{
			params[0] = NAN;
			params[1] = NAN;
			ssr = NAN;
			return mdm_ErrorTracker::ErrorCode::DWI_INPUT_ZERO;
		}
	}

	//Do linear ADC fit
	// Get starting values from linear fit
	linearFit(params[0], params[1], ssr);

	//What to do about returning linear fit here?
	if (linearFit_)
		return mdm_ErrorTracker::ErrorCode::OK;
	

	//Otherwise, use lin fit as init values for full non-linear fit
	alglib::real_1d_array x;
	x.attach_to_ptr(2, params.data());

	// Optimize and evaluate results
	try
	{
		minbcrestartfrom(state_, x);
		alglib::minbcoptimize(state_, &computeSSEGradientAlglib, NULL, this);
		minbcresults(state_, x, rep_);

#if _DEBUG
		//
		// Check that OptGuard did not report errors
		//
		alglib::optguardreport ogrep;
		alglib::minbcoptguardresults(state_, ogrep);
		std::cout << "Optimisation guard results:\n";
		std::cout << "Bad gradient suspected:" << (ogrep.badgradsuspected ? "true" : "false") << "\n"; // EXPECTED: false
		std::cout << "Non c0 suspected:" << (ogrep.nonc0suspected ? "true" : "false") << "\n"; // EXPECTED: false
		std::cout << "Non c1 suspected:" << (ogrep.nonc1suspected ? "true" : "false") << "\n"; // EXPECTED: false
#endif
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
		setErrorValuesAndTidyUp(params);
		return mdm_ErrorTracker::DWI_MAX_ITER;
	}

	params[0] = x[0]; //s0 - do we need to do this if we attached pointer above?
	params[1] = x[1]; //ADC
	alglib::real_1d_array g;
	g.setlength(2);
	computeSSEGradient(x, ssr, g);
	return mdm_ErrorTracker::ErrorCode::OK;

	
}

//
MDM_API bool mdm_DWIFitterADC::setInputsFromStream(std::istream& ifs,
	const int nSignals)
{
	Bvals_.resize(nSignals);
	signals_.resize(nSignals);
	for (auto& Bval : Bvals_)
	{
		ifs >> Bval;
		if (ifs.eof())
			return false;
	}
	for (auto& si : signals_)
		ifs >> si;

	signals_to_fit_ = signals_;
	Bvals_to_fit_ = Bvals_;

	return true;
}

//
MDM_API int mdm_DWIFitterADC::minimumInputs() const
{
	return 3;
}

//
MDM_API int mdm_DWIFitterADC::maximumInputs() const
{
	return 10;
}

//
MDM_API double mdm_DWIFitterADC::modelToSignal(
	const std::vector<double>& params, double Bval)
{
	auto s0 = params[0];// 's0'
	auto adc = params[1];// 'adc'
	return s0 * std::exp(-adc * Bval);
}

MDM_API std::vector<double> mdm_DWIFitterADC::modelToSignals(
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

void mdm_DWIFitterADC::computeSignalGradient(
	const double& S0, const double& ADC, const double& B,
	double& signal, double& signal_dS0, double& signal_dADC)
{
	// Preliminary Calculations
	auto Ed = std::exp(-ADC * B);

	//signal and partial derivatives of signal by model param
	signal_dS0 = Ed;
	signal = S0 * signal_dS0;
	signal_dADC = -B*signal;
}

void mdm_DWIFitterADC::computeSSEGradient(
	const alglib::real_1d_array& x, double& func, alglib::real_1d_array& grad)
{
	const double& S0 = x[0];
	const double& ADC = x[1];
	auto n = signals_to_fit_.size();

	func = 0;
	grad[0] = 0;
	grad[1] = 0;

	double s, dS0, dADC;
	for (int i = 0; i < n; i++)
	{
		computeSignalGradient(
			S0, ADC, Bvals_to_fit_[i],
			s, dS0, dADC);
		double diff = s - signals_to_fit_[i];
		func += diff * diff;
		grad[0] += 2 * dS0 * diff;
		grad[1] += 2 * dADC * diff;
	}
}

//
void mdm_DWIFitterADC::linearFit(double& S0, double& ADC, double& ssr)
{
	alglib::real_1d_array x;
	alglib::real_1d_array y;
	auto n = signals_to_fit_.size();
	x.setcontent(n, Bvals_to_fit_.data());
	y.setlength(n);
	for (size_t i = 0; i < n; i++)
		y[i] = std::log(signals_to_fit_[i]);

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

	S0 = std::exp(p[0]);
	ADC = -p[1];

	alglib::real_1d_array g;
	g.setlength(2);
	computeSSEGradient(x, ssr, g);
}