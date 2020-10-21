/**
 *  @file    mdm_T1Voxel.cxx
 *  @brief   T1 calculation stuff for MaDyM
 *
 *  Original Author GJM Parker 2001-2002
 *  Moved to this file and structurally modified by GA Buonaccorsi
 *  (c) Copyright ISBE, University of Manchester 2002
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1Voxel.h"

#include <stdio.h>                
#include <stdlib.h>                
#include <cmath>            /* For cos(), sin() and exp() */
#include <cassert>          /* For assert macro */

#include "mdm_ErrorTracker.h"
#include "mdm_ProgramLogger.h"

const int mdm_T1Voxel::MINIMUM_FAS = 3;
const int mdm_T1Voxel::MAXIMUM_FAS = 10;

//
MDM_API mdm_T1Voxel::mdm_T1Voxel(const std::vector<double> &FAs, const double TR)
	: FAs_(FAs),
	TR_(TR),
	delta_(1.0),
	maxIterations_(500)
{
	if (!FAs.empty())
		initFAs();

	//Pre-initialise the alglib state
	alglib::real_1d_array x = "[1000,1000]";
	alglib::real_1d_array s = "[1,1]";
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
}

//
MDM_API mdm_T1Voxel::mdm_T1Voxel()
	: mdm_T1Voxel({}, 0)
{

}

//
MDM_API void mdm_T1Voxel::setFAs(const std::vector<double> &FAs)
{
	FAs_ = FAs;
	initFAs();
}

//
MDM_API void mdm_T1Voxel::setSignals(const std::vector<double> &signals)
{
	assert(signals.size() >= MINIMUM_FAS);  /* Input arg */
	signals_ = signals;
}

//
MDM_API void mdm_T1Voxel::setTR(const double TR)
{
	TR_ = TR;
}

MDM_API int mdm_T1Voxel::fitT1_VFA(
	double &T1value, double &M0value)
{
	assert(signals_.size() == nFAs_);
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
	int iterations = rep_.iterationscount;

#if _DEBUG
	std::cout << "Alglib complete after " << iterations << std::endl;
#endif
	/* Check for non-convergence */
	if (iterations >= maxIterations_)
	{
		setErrorValuesAndTidyUp("Error 3 - alglib:CG() hit max iterations", T1value, M0value);
		return mdm_ErrorTracker::T1_MAX_ITER;
	}

	/* Check for crap fit or bonkers result*/
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
MDM_API double mdm_T1Voxel::T1toSignal(
	const double T1, const double M0, const double FA, const double TR)
{
	double E1 = std::exp(-TR / T1);
	return M0 * std::sin(FA) * (1 - E1) / (1 - std::cos(FA) * E1);
}

//
void mdm_T1Voxel::setErrorValuesAndTidyUp(const std::string msg, double &T1, double &M0)
{
	mdm_ProgramLogger::logProgramMessage(
		"WARNING: mdm_T1Voxel::T1_calcVarFlipAngle:   " + msg + "\n");

	//Set default values for M0 and T1
	T1 = 0.0;
	M0 = 0.0;
}

void mdm_T1Voxel::computeSignalGradient(const double &T1, const double &M0,
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

void mdm_T1Voxel::computeSSEGradient(
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
void mdm_T1Voxel::initFAs()
{
	nFAs_ = FAs_.size();
	assert(nFAs_ >= MINIMUM_FAS);

	cosFAs_.resize(nFAs_);
	sinFAs_.resize(nFAs_);
	for (int i = 0; i < nFAs_; i++)
	{
		cosFAs_[i] = std::cos(delta_*FAs_[i]);
		sinFAs_[i] = std::sin(delta_*FAs_[i]);
	}
}

/* 
This is the old NR in-house optimiser stuff. I've replaced with the Alglib
stuff a faster and more flexible (while just as accurate). Kept for posterity
but may be removed
//
MDM_API int mdm_T1Voxel::fitT1_VFA(
	const std::vector<double> &signal, const std::vector<double> &fa,
	const std::vector<double> &sigma,
	const double tr,
	double &T1value, double &M0value)
{
	assert(signal.size() >= MINIMUM_FAS);
assert(fa.size() == signal.size());

//Initialise sensible values for T1 and M0
T1value = 1000.0;
M0value = signal[0] * 30.0;

// Initialise A and list a for mrmqmin
double  deltavalue = 1.0; //Flip angle factor
std::vector<double> a = { T1value, M0value, deltavalue, tr };
std::vector <int> opt_idx = { 0, 1 };

//Set up remaining inputs for mrqmin   
std::vector< std::vector<double> >covar(2, std::vector<double>(2, 0));
std::vector<double> da(2, 0);

//Initialise LM algorithm, NR p. 685,  initialise requires alambda < 0
double alambda = -1.0;
double chisq = (double)LONG_MAX;   //

//
if (mdm_NRUtils::mrqmin(fa, signal, sigma, a, opt_idx, covar, covar, da, da, chisq, T1toSignal, alambda, 0) != 0)
{
	setErrorValuesAndTidyUp("Error 1 - mrqmin() failed initialisation", T1value, M0value);
	return mdm_ErrorTracker::T1_INIT_FAIL;
}
// We got here so we're OK - save the value of chi-squared
double chisq_previous = chisq;

// Now loop over fitting routine.  Find min Chi-squared in global minima
int itr = 0;
int    maxIterations = 500;
for (; itr < maxIterations; itr++)
{
	if (mdm_NRUtils::mrqmin(fa, signal, sigma, a, opt_idx, covar, covar, da, da, chisq, T1toSignal, alambda, 0) != 0)
	{
		setErrorValuesAndTidyUp("Error 2 - mrqmin() failed during main loop", T1value, M0value);
		return mdm_ErrorTracker::T1_FIT_FAIL;
	}

	// Check chisq for convergence
	if ((chisq / chisq_previous <= 1 && chisq / chisq_previous > 0.999 && alambda > 1000000000))
		break;
	else
		chisq_previous = chisq;
}

// Check for non-convergence
if (itr >= maxIterations)
{
	setErrorValuesAndTidyUp("Error 3 - mrqmin() hit max iterations", T1value, M0value);
	return mdm_ErrorTracker::T1_MAX_ITER;
}

// Check for crap fit or bonkers result
if (a[0] < 0.0 || a[0] > 6000.0)
{
	setErrorValuesAndTidyUp("Error 4 - Mad values", T1value, M0value);
	return mdm_ErrorTracker::T1_MAD_VALUE;
}

///We got here so we're OK
T1value = a[0];
M0value = a[1];

return mdm_ErrorTracker::OK;
}

//
MDM_API void mdm_T1Voxel::T1toSignal(const double x_FA, std::vector<double> a,
	double &y_SI, std::vector<double> &dyda)
{
	const double &T1 = a[0];
	const double &M0 = a[1];
	const double &delta = a[2];
	const double &TR = a[3];

	// Preliminary Calculations
	double cosine = cos(delta * x_FA);
	double sine = sin(delta * x_FA);
	double Exp = T1 ? exp(-TR / T1) : 0.0; //Only set if non-zero T1
	double A = 1.0 - Exp * cosine;

	// signal intensity relationship
	y_SI = M0 * sine * (1 - Exp) / A;

	//Compute deriavtives with respect to each member of a
	dyda.resize(4);
	if (T1)
	{
		// First derivative - ds/dT1
		dyda[0] = M0 * sine * TR * Exp * (-1 + cosine)
			/ ((-1 + cosine * Exp) * (-1 + cosine * Exp)) / (T1 * T1);

		// First derivative - ds/dTR
		dyda[3] = -M0 * sine * Exp * (-1 + cosine)
			/ ((-1 + cosine * Exp) * (-1 + cosine * Exp)) / T1;
	}

	else
	{
		dyda[0] = 1000000000.0; // something very big
		dyda[3] = 1000000000.0; // something very big
	}

	// First derivative - ds/dM0
	dyda[1] = y_SI / M0;

	// First derivative - ds/ddelta
	dyda[2] = M0 * x_FA * (-1 + Exp) *
		(-cosine + cosine * cosine * Exp + sine * sine * Exp)
		/ ((-1 + cosine * Exp) * (-1 + cosine * Exp));

}

Below is all the old C stuff from Gio and Geoff. I've kept it here commented
out for now, but will probably delete. Note it doesn appear to be slightly quicker than
the C++ replacement, but I'm happy to take a small performance hit (eg a couple of seconds
over a whole 3D image volume of >100k voxels) in return for nicely readable and debuggable code
//
void T1_funcVarFlipAngle(double x_FA, double *a, double *y_SI, double *dyda, int na)
{
  double cosine, sine, Exp, A;

  // a[1]:T1 a[2]:M0 a[3]:delta a[4]:TR x_FA:flip angle
  // Preliminary Calculations
  cosine = cos(a[3] * x_FA);
  sine   = sin(a[3] * x_FA);
  if (a[1] != 0)                // Dangerous comparison for a double
    Exp = exp(-a[4] / a[1]);
  else
    Exp = 0.0;

  A = 1.0 - Exp * cosine;

  //signal intensity relationship
  *y_SI = a[2] * sine * (1 - Exp) / A;

  // First derivative - ds/dT1
  if (a[1] != 0)
    dyda[1] = a[2] * sine * a[4] * Exp * (-1 + cosine)
              / ((-1 + cosine * Exp) * (-1 + cosine * Exp)) / (a[1] * a[1]);
  else
    dyda[1] = 1000000000.0; // something very big

  // First derivative - ds/dM0
  dyda[2] = *y_SI / a[2];

  // First derivative - ds/ddelta
  dyda[3] = a[2] * x_FA * (-1 + Exp) * (-cosine + cosine * cosine * Exp + sine * sine * Exp)
            / ((-1 + cosine * Exp) * (-1 + cosine * Exp));

  // First derivative - ds/dTR
  if (a[1] != 0)
    dyda[4] = -a[2] * sine * Exp * (-1 + cosine)
              / ((-1 + cosine * Exp) * (-1 + cosine * Exp)) / a[1];
  else
    dyda[4] = 1000000000.0; // something very big
}

/

//
void setErrorValuesAndTidyUp(double **covar, double **alpha, double *a, double *x, double *y, double *z, int *opt_idx, const std::string msg, double &T1, double &M0)
{
  std::string logmsg, errString;

  assert(covar != NULL);
  assert(alpha != NULL);
  assert(a     != NULL);
  assert(x != NULL);
  assert(y != NULL);
  assert(z != NULL);
  assert(opt_idx != NULL);

	mdm_ProgramLogger::logProgramMessage(
		"WARNING: mdm_T1Voxel::T1_calcVarFlipAngle:   " + msg + "\n");

  // Thanks for the memory ... 
  free(a);
  free(x);
  free(y);
  free(z);
  free(opt_idx);
	mdm_NRUtils::free_matrix(alpha, 1, 4, 1, 4);
	mdm_NRUtils::free_matrix(covar, 1, 4, 1, 4);

	//Set default values for M0 and T1
	T1 = 0.0;
	M0 = 0.0;
}

MDM_API int mdm_T1Voxel::T1_calcVarFlipAngle(const std::vector<double> &signal, const std::vector<double> &fa,
  const double tr,
  double &T1value, double &M0value)
{
  const int    nFAs = signal.size();     // This amounts to an assumption that MDM_NFAS flip angles have been used
  //double x_FA[nFAs+1], y_SI[nFAs + 1], sigma[nFAs + 1];        // Input arrays for mrqmin - need 4 elements cos NR stuff doesn't use element 0

  double *x_FA, *y_SI, *sigma;
  x_FA = (double *)malloc((unsigned)(nFAs + 1) * sizeof(double));
  y_SI = (double *)malloc((unsigned)(nFAs + 1) * sizeof(double));
  sigma = (double *)malloc((unsigned)(nFAs + 1) * sizeof(double));

  assert(nFAs >= MINIMUM_FAS);  // Input arg
  assert(fa.size() == nFAs);  // Input arg

  double *a;      // Input matrix for mrqmin
  int   *opt_idx;  // Ditto

  double   chisq, alambda;
  double **covar, **alpha;
  double   chisq_previous;

  double  deltavalue = 1.0;
  int    maxIterations = 500;
  int    nfit = 2;
  
  

  //
  // Initialise values for fitting - MB NOT ANY MORE!
  // Note:  T1value, and M0value are globals from madym.h
  //
  T1value = 1000.0;
  for (int i = nFAs; i > 0; i--)
  {
    x_FA[i] = fa[i - 1];// *0.017453292;   // Flip angle of image FA_(i - 1) in radians
    y_SI[i] = signal[i - 1];             // Sig int of image FA_(i - 1)
    sigma[i] = 0.1;                       // Initial estimate of stdev
  }
  // Initialise M0 to something reasonable
  M0value = y_SI[1] * 30.0;

  // matrix() is from mdm_NRUtils.c - gets memory for alpha[][] and covar[][]
  alpha = mdm_NRUtils::matrix(1, 4, 1, 4);
  covar = mdm_NRUtils::matrix(1, 4, 1, 4);

  //Initialise mrqmin() input matrices - need 5 elements cos NR stuff doesn't use 0th elements
  a = (double *)malloc(sizeof(double) * 5);
  opt_idx = (int *)malloc(sizeof(double) * 5);
  a[1] = T1value;     // T1 from the madym.h global ... 
  a[2] = M0value;     // M0 from the madym.h global ... 
  a[3] = deltavalue;  // Flip angle factor 
  a[4] = tr;
  opt_idx[1] = 1;       // 1 => variable parameter in minimisation 
  opt_idx[2] = 2;       // 2 => variable parameter in minimisation 
  opt_idx[3] = 0;       // 0 => fixed parameter in minimisation 
  opt_idx[4] = 0;       // 0 => fixed parameter in minimisation 
  
   // Initialise LM algorithm
   // NR p. 685,  initialise requires alambda < 0
   // Local modification says it has to be -1.0
   //
  alambda = -1.0;
  chisq = (double)LONG_MAX;   // From limits.h - not sure why it's not FLOAT_MAX
  //
   //  mrqmin()
   //  x_FA, y_SI, sigma, nFAs - nFAs points (x, y) with their stdev estimates
   //  a, opt_idx, 4            - 4 coefficients a, with flags opt_idx indicating if they should be varied or fixed (0 => fixed)
   //  nfit and final 0       - local mods to NR routine
   //  rest are fairly obvious so best to look at NR p. 685
   //

  if (mdm_NRUtils::mrqmin(x_FA, y_SI, sigma, nFAs, a, 4, opt_idx, nfit, covar, alpha, &chisq, T1_funcVarFlipAngle, &alambda, 0) != 0)
  {
    setErrorValuesAndTidyUp(covar, alpha, a, x_FA, y_SI, sigma,
      opt_idx, "Error 1 - mrqmin() failed initialisation", T1value, M0value);
    return MDMERR_T1INITFAILED;
  }
  // We got here so we're OK - save the value of chi-squared
  chisq_previous = chisq;

  // Now loop over fitting routine.  Find min Chi-squared in global minima
  int itr = 0;
  for (; itr < maxIterations; itr++)
  {
    if (mdm_NRUtils::mrqmin(x_FA, y_SI, sigma, nFAs, a, 4, opt_idx, nfit, covar, alpha, &chisq, T1_funcVarFlipAngle, &alambda, 0) != 0)
    {
      setErrorValuesAndTidyUp(covar, alpha, a, x_FA, y_SI, sigma,
        opt_idx, "Error 2 - mrqmin() failed during main loop", T1value, M0value);
      return MDMERR_T1MAINFAILED;
    }

    /// Check chisq for convergence
    if ((chisq / chisq_previous <= 1 && chisq / chisq_previous > 0.999 && alambda > 1000000000))
      break;
    else
      chisq_previous = chisq;
  }

  //
   // Free memory held as static in mrqmin
   //
   // Note - Not done for the various "return" blocks ...
   //      - May not need to be done - read up on mrqmin() in NR book ...
   //
  alambda = -2.0;
  mdm_NRUtils::mrqmin(x_FA, y_SI, sigma, nFAs, a, 4, opt_idx, nfit, covar, alpha, &chisq, T1_funcVarFlipAngle, &alambda, 0);

  // Check for non-convergence
  if (itr >= maxIterations)
  {
    setErrorValuesAndTidyUp(covar, alpha, a, x_FA, y_SI, sigma,
      opt_idx, "Error 3 - mrqmin() hit max iterations", T1value, M0value);
    return MDMERR_T1MAXITER;
  }

  // Check for crap fit or bonkers result
  if (a[1] < 0.0 || a[1] > 6000.0)
  {
    setErrorValuesAndTidyUp(covar, alpha, a, x_FA, y_SI, sigma,
      opt_idx, "Error 4 - Mad values", T1value, M0value);
    return MDMERR_T1MADVALUE;
  }

  //We got here so we're OK
  // Set madym.h global values, tidy up heap memory and return
  T1value = a[1];
  M0value = a[2];

  free(a);
  free(opt_idx);
  free(x_FA);
  free(y_SI);
  free(sigma);

  mdm_NRUtils::free_matrix(alpha, 1, 4, 1, 4);
  mdm_NRUtils::free_matrix(covar, 1, 4, 1, 4);

  return MDMERR_OK;
}*/