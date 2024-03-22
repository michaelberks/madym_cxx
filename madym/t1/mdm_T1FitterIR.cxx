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

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>
//
MDM_API mdm_T1FitterIR::mdm_T1FitterIR(const std::vector<double> &TIs, const double TR, 
	const bool fitEfficiencyWeighting, const std::vector<double>& init_params)
  :
  mdm_T1FitterBase(),
  TIs_(TIs),
  TR_(TR),
	fitEfficiencyWeighting_(fitEfficiencyWeighting),
	init_params_(init_params)
{
	//Pre-initialise the alglib state
	int nParams = fitEfficiencyWeighting_ ? 3 : 2;
	std::vector<double> init = { 1000, 1000, 1.0};

	std::vector<double> lowerBounds = { 0, 0, 0};
	std::vector<double> upperBounds = { 1e5, 1e6, 1};

	alglib::real_1d_array x;
	alglib::real_1d_array bndl;
	alglib::real_1d_array bndu;

	x.setcontent(nParams, init.data());
	bndl.setcontent(nParams, lowerBounds.data());
	bndu.setcontent(nParams, upperBounds.data());

	double epsg = 0.00000000001;
	double epsf = 0.0000;
	double epsx = 0.0000000001;

#if _DEBUG
	alglib::ae_int_t maxits = maxIterations_ > 100 ? 100 : maxIterations_;
#else
	alglib::ae_int_t maxits = maxIterations_;
#endif

	alglib::minbccreate(x, state_);
	alglib::minbcsetbc(state_, bndl, bndu);
	alglib::minbcsetcond(state_, epsg, epsf, epsx, maxits);
	alglib::minbcsetprecscale(state_);

#if _DEBUG
	//Provides numerical check of analytic gradient, useful in debugging, but should not be
	//used in release versions
	alglib::minbcoptguardsmoothness(state_);
	alglib::minbcoptguardgradient(state_, 0.001);
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

double get_scale(double init)
{
	return  std::pow(10, std::max((double)std::round(std::log10(init)), 0.0));
}

//
MDM_API mdm_ErrorTracker::ErrorCode mdm_T1FitterIR::fitT1(
	double &T1value, double &M0value, double& EWvalue)
{
	if (signals_.size() != TIs_.size())
    throw mdm_exception(__func__, "Number of signals (" + std::to_string(signals_.size()) +
      ") does not match number of TIs (" + std::to_string(TIs_.size()) + ")");

	//
	// First, we create optimizer object and tune its properties
	//
	int n_params;
	double init_T1, init_M0;
	double init_EW = 1.0;

	if (fitEfficiencyWeighting_)
	{
		//Run an initial fit with efficiency fixed to 1.0 to get
		//initial values for T1 and M0
		
		mdm_T1FitterIR* fitter = new mdm_T1FitterIR(TIs_, TR_, false, init_params_);
		fitter->setInputs(signals_);
		fitter->fitT1(init_T1, init_M0, init_EW);
		delete fitter;

		n_params = 3;
		
	}
	else
	{
		auto nInit = init_params_.size();

		if (nInit)
		{
			//If user has only given one value, use this for initialising T1
			init_T1 = init_params_[0];

			if (nInit > 1)
				//If user gives second value, use this for M0
				init_M0 = init_params_[1];
			else
				//Set M0 from signals
				init_M0 = signals_.back();
		}
		else //No user initialisation, set M0 from signals, T1 defaults to 1000
		{
			init_T1 = 1000.0;
			init_M0 = signals_.back();
		}
		n_params = 2;
	}

	std::vector<double> init_vals = { init_T1, init_M0, init_EW };
	std::vector<double> init_scale = { get_scale(init_T1), get_scale(init_M0), 1 };
	alglib::real_1d_array x;
	alglib::real_1d_array s;
	
	// Optimize and evaluate results
	try
	{
		x.setcontent(n_params, init_vals.data());
		s.setcontent(n_params, init_scale.data());
		alglib::minbcsetscale(state_, s);
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

		std::cout << "T1 initalised to " << init_vals[0] << ", scale = " << init_scale[0] << '\n';
		std::cout << "M0 initalised to " << init_vals[1] << ", scale = " << init_scale[1] << '\n';
		std::cout << "Fitting to [" << signals_.front() << ", " << signals_.back() << "], with TR = " << TR_ << "\n";
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
	EWvalue = fitEfficiencyWeighting_ ? x[2] : 1.0;
#if _DEBUG
	std::cout << "Fitted T1 = " << T1value << ", M0 = " << M0value << "\n";
#endif
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
	const double T1, const double M0, const double TI, const double TR, const double EW /*=1.0*/)
{
	double E_TI = std::exp(-TI / T1);
  double E_TR = std::exp(-TR / T1);

  return std::abs(M0 * (1 - 2*EW*E_TI + E_TR));
}

//**********************************************************************
//Private methods
//**********************************************************************

void mdm_T1FitterIR::computeSignalGradient(
	const double &T1, const double &M0, const double& EW,
	const double &TI,
	double &signal, double &signal_dT1, double &signal_dM0, double &signal_dEW
	)
{
  //Signal model is:
  //S = | M0*(1 - 2*EW*exp(-*TI/T1) + e(-TR/T1) ) |
	//where we only fit EW if fitEfficiencyWeighting_ is true

  //If M0 or T1 are 0, the signal is 0 and the signal model is non-differentiable
  if (!M0 || !T1)
  {
    signal = 0;
    signal_dT1 = 1000000000.0; // something very big
    signal_dM0 = 1000000000.0; // something very big
		signal_dEW = 1000000000.0; // something very big
    return;
  }

	// Preliminary Calculations - T1, M0 both > 0
	double E_TI = exp(-TI / T1);
  double E_TR = exp(-TR_ / T1);

	// signal intensity relationship - most efficient to compute
	//ds/dM0, then multiply to get s
	signal_dM0 = 1 - 2*EW*E_TI + E_TR;
	signal = M0 * signal_dM0;

	//Compute deriavtives with respect to T1
	// First derivative - ds/dT1
	signal_dT1 = M0 * (-2*EW*E_TI*TI + E_TR*TR_) / (T1*T1);

	//Compute derivatives with respect to lambda (L)
	signal_dEW = fitEfficiencyWeighting_ ? -2 * M0 * E_TI : 0;

  //We haven't taken in to account the absolute operator on the signal model yet
  //If the signal is negative, invert the sign on the signal and the two
  //partial derivatives
  if (signal < 0)
  {
    signal *= -1;
    signal_dM0 *= -1;
    signal_dT1 *= -1;
		signal_dEW *= -1;
  }

	
}

void mdm_T1FitterIR::computeSSEGradient(
	const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad)
{
	const double &T1 = x[0];
	const double &M0 = x[1];
	const double &EW = fitEfficiencyWeighting_ ? x[2] : 1.0;

	func = 0;
	grad[0] = 0;
	grad[1] = 0;
	if (fitEfficiencyWeighting_)
		grad[2] = 0;

	double s, s_dT1, s_dM0, s_dEW;
	for (int i = 0; i < TIs_.size(); i++)
	{
		computeSignalGradient(T1, M0, EW, TIs_[i],
			s, s_dT1, s_dM0, s_dEW);
		double diff = s - signals_[i];
		func += diff * diff;
		grad[0] += 2 * s_dT1*diff;
		grad[1] += 2 * s_dM0*diff;
		if (fitEfficiencyWeighting_)
			grad[2] += 2 * s_dEW * diff;
	}
}