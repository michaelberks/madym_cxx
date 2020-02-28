/*
 * Edited by GAB 6 feb 2004
 * Reference to xvgr now points to /usr/local/bin
 *
 *  Mods by GAB
 *  7 April 2004 - Moved out
 *  Sep 2004     - Tidying code structure:
 *                 - Removed unnecessary includes
 *                 - Created fitModel.h to improve modularity
 *                 - Moved some functionality into functions to reduce repetition of code sections
 *                 - Made most external variables static to restrict to file scope
 *  Nov 2004     - Added Population AIF
 *  7 July 2005  - Revising fitting code to use better simplex strategy & merging the dev and std madym branches
 *                 - Fix parameter_array indexing so offset is always index 2
 *  24 October 2006 - Fixed sigmoid bug in population AIF
 *                    GJMP & GAB
 *  24 October 2006 - Modifications to allow concentration time series reading. Version 1.06
 *                    GJMP & GAB
 *  9 November 2006 - Modifications to include new definition of enhancing volume. Version 1.08
 *                    GJMP & GAB & CJR
 *                  - Based on 24 July 2006 Mods by CJR
 *                  - added code to determine if a voxel is enhancing or
 *                    not by using a Mann-Whitney-Wilcoxon rank sum test.
 *  23 April 2007   - Modification to remove concentration time series div by 1000. Version 1.09
 *                    GAB
 *  21 July 2008    - Removed 2 bad initialisation bugs in VOI pathway in fitModel.c - Version 1.13.beta GAB
 *  22 July 2011    - Refactoring and TINA removal for BiOxyDyn licensing - Version 1.20.alpha GAB
 *  22 July 2011    - Refactoring for BiOxyDyn licensing - Version 1.21.alpha GAB
 *  4 March 2014    - Moved IAUC stuff to vap.c - Version 1.22 GAB
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS
#include "mdm_DCEVoxel.h"

#include <cmath>      /* For exp()    */

#include "opt/optimization.h"
#include "opt/linalg.h"

#include "mdm_version.h"
#include "mdm_ProgramLogger.h"

const double mdm_DCEVoxel::Ca_BAD1 = -1.0e3;
const double mdm_DCEVoxel::Ca_BAD2 = -2.0e3;
const double mdm_DCEVoxel::T1_TOLERANCE = 1.0e-6;
const double mdm_DCEVoxel::DYN_T1_MAX = 1.0e9;
const double mdm_DCEVoxel::DYN_T1_INVALID = -1.0;

MDM_API mdm_DCEVoxel::mdm_DCEVoxel(
  const std::vector<double> &dynSignals,
	const std::vector<double> &dynConc,
  const std::vector<double> &noiseVar,
	const double T10,
	const double S0,
	const double r1cont,
  const int bolus_time,
	const std::vector<double> &dynamicTimings,
	const double TR,
	const double FA,
	const int n1,
	const int n2,
	const bool testEnhancement,
	const bool useRatio,
	const std::vector<double> &IAUC_times)
	:
  signalData_(dynSignals),
	CtData_(dynConc),
  noiseVar_(noiseVar),
	t10_(T10),
	s0_(S0),
	n1_(n1),
	n2_(n2),
	r1cont_(r1cont),
  bolus_time_(bolus_time),
	IAUC_times_(IAUC_times),
	IAUC_vals_(IAUC_times_.size(), 0),
	modelFitError_(0),
	enhancing_(0),
	dynamicTimings_(dynamicTimings),
	TR_(TR),
	FA_(FA),
	testEnhancement_(testEnhancement),
	useRatio_(useRatio),
  status_(mdm_DCEVoxelStatus::OK)
{
}

MDM_API mdm_DCEVoxel::~mdm_DCEVoxel()
{

}

/**
* Pre-conditions:
*
* Post-conditions
* -  Global T1value holds current dynamic T1
*
* Uses madym.h globals:
* -  concentrationMapFlag                (input only)
* -  T1value                             (output - value set)
*
* Note:  NOT a stand-alone fn - see pre-conditions & use of globals
*
* @brief    Convert signal intensity time-series to [CA] time series
* @param    p        Pointer to Permeability struct holding input data
* @param    tr       Float TR (repetition time) value
* @param    fa       Float FA (flip angle) value
* @return   double*   Pointer to [CA] time series array
*/
MDM_API void mdm_DCEVoxel::computeCtFromSignal()
{
  double  R1gd = r1cont() * 0.001;   /* Use millisec instead of sec (as in user interface) */

  /* MB - this function is now only called when we're converting
  signal intensities to concentration. If we already have concentrations
  this function isn't (and logically shouldn't) be called, so no need to check
  here*/
  const size_t &n_times = signalData().size();
  CtData_.resize(n_times);

  // Only calculate if T1(0) > 0.0
  if (t10() <= 0.0)
  {
		mdm_ProgramLogger::logProgramMessage(
			"Warning: mdm_DCEVoxel::computeCtFromSignal: Baseline T1 <= 0.0\n");
    status_ = mdm_DCEVoxelStatus::T10_BAD;
    return;
  }
  
  const auto PI = acos(-1.0);
  double sinfa = sin(FA_ * PI / 180);
  double cosfa = cos(FA_ * PI / 180);

  // This is a bad check.  It assumes invalid baseline T1 is flagged by invalid S0
  /*MB - so don't do it??*/
  //if (p.s0() > 0.0)

  // Calculate dyn_pbm
  // Need to check that we've got the pb time points
  if (bolus_time_ >= n1())
  {
    double   sum_pbm = 0.0;
    int     num_pbm = 0;
    for (int k = n1(); k < bolus_time_; k++)
    {
      sum_pbm += signalData()[k];
      num_pbm++;
    }
    double s_pbm = sum_pbm / num_pbm;

    for (size_t k = 0; k < n_times; k++)
    {
      double R1value;
      int errorCode;
      if (useRatio())
        R1value = computeT1DynPBM(signalData()[k], s_pbm, cosfa, errorCode);
      else
        R1value = computeT1DynS0(signalData()[k], sinfa, cosfa, errorCode);    
      
      CtData_[k] = (R1value - 1.0 / t10()) / R1gd;

      if (errorCode)
        status_ = mdm_DCEVoxelStatus::DYN_T1_BAD;

      else if (isnan(CtData_[k]))
        status_ = mdm_DCEVoxelStatus::CA_NAN;
    }
  }
  else
  {
    for (size_t k = 0; k < n_times; k++)
    {
      CtData_[k] = Ca_BAD1;
      status_ = mdm_DCEVoxelStatus::S0_BAD;
    }      
  }
}

/**
* @brief    Calculate dynamic T1 from S0 and dynamic signal intensity
* @param    t1_0  - Baseline T1 value
* @param    st    - dynamic signal intensity
* @param    s_pbm - prebolus mean dynamic signal intensity
* @param    cosfa - cos(flip angle)
* @param    tr    - TR (repetition time)
* @return   Float dynamic T1 value (0.0 on divide by zero or other error)
*
* @author   Gio Buonaccorsi
* @version  1.21.alpha (12 August 2013)
*/
double mdm_DCEVoxel::computeT1DynPBM(const double &st, const double &s_pbm, const double &cosfa, int &errorCode)
{
  /*
  *  Yes, it looks horrible and over-complicated.  I don't care.
  *  Too many div by zeros to account for, and a log zero to boot
  */
  errorCode = 0;
  if (s_pbm < T1_TOLERANCE)
    errorCode = -1;

  double expTR_T10 = exp(-TR_ / t10());
  double S1_S0 = st / s_pbm;

  double denominator = 1.0 - cosfa * expTR_T10;
  if (std::abs(denominator) < T1_TOLERANCE)
    errorCode = -2;

  double fraction1 = (1.0 - expTR_T10) / denominator;

  denominator = 1.0 - S1_S0 * cosfa * fraction1;
  if (std::abs(denominator) < T1_TOLERANCE)
    errorCode = -3;

  double fraction2 = (1.0 - S1_S0 * fraction1) / denominator;
  if (std::abs(fraction2) < T1_TOLERANCE)
    errorCode = -3;

  double logF2 = log(fraction2);
  double  R1_t = logF2 / -TR_;
  if (R1_t < 0.0)
    errorCode = -4;
  else if (1.0 / R1_t > DYN_T1_MAX)
    errorCode = -5;

  return R1_t;
}

/**
* @brief    Calculate dynamic T1 from S0 and dynamic signal intensity
* @param    s0    - M0 in signal intensity domain
* @param    st    - dynamic signal intensity
* @param    sinfa - sin(flip angle)
* @param    cosfa - cos(flip angle)
* @param    tr    - TR (repetition time)
* @return   Float dynamic T1 value (0.0 on divide by zero or other error)
*
* @author   Gio Buonaccorsi
* @version  1.21.alpha (12 August 2013)
*/
double mdm_DCEVoxel::computeT1DynS0(const double &st, const double & sinfa, const double & cosfa, int &errorCode)
{
  errorCode = 0;
  double num = s0() * sinfa - st;
  double denom = s0() * sinfa - st * cosfa;
  double   R1_t = R1_t = -log(num / denom) / TR_;

  if (std::abs(num) < T1_TOLERANCE)
    errorCode = -1;
  else if (std::abs(denom) < T1_TOLERANCE)
    errorCode = -2;

  return R1_t;
}

/**
 * Pre-conditions:
 * -  parameter_array has ndim valid values
 *
 * Post-conditions
 * -  G* file scope static all set (see below)
 *
 * Uses madym.h globals:
 * -  modelType_
 * Uses file-scope statics:
 * -  GnData, Gx, Gy, Gdose, Gk, Gve, Gvp, Goffset, GHct
 * -  catFromModel_[]
 *
 * This is the cost function passed in the parameter list to simplexmin(), so its heading
 * is fixed.  The parameter_array holds the values passed to and returned from simplex, 
 * as a simple set of values, so we have to know the ordering and their meanings.
 *
 * The parameter values are checked to be sure they take sensible values and a high cost
 * (FLT_MAX) is returned if they do not.  The SSE is calculated, comparing the [CA] values 
 * calculated from the measured signal intensity time series (Gy[]) to the model (file scope 
 * static array catFromModel_[]), and this SSE returned.
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker with mods by GA Buonaccorsi
 * @brief    Calculate errors on CA(t) estimated from data cf. fitted model
 * @version  madym 1.21.alpha
 * @param    ndim              Integer number of elements in parameter_array
 * @param    parameter_array   Double array holding model parameters (from simplex)
 * @return   double    Sum squared differences (data to model) or FLT_MAX
 */
/*double mdm_DCEVoxel::catSSD(int ndim, std::vector<double> &parameter_array)
{
	return catSSD(ndim, parameter_array.data());
}*/

double mdm_DCEVoxel::CtSSD(const std::vector<double> &parameter_array)
{
  //Set the full parameter array in the model from the optimised subset
  model_->setPkParams(parameter_array);

	//Get model to check params are ok - returns non-zero for bad value
  double model_check = model_->checkParams();
  if (model_check)
    return model_check;

	//If we got this far the parameter values look OK, so ...
	// Calculate model [CA](t) for current parameter set
	model_->computeCtModel(n2());

	//Compute SSD
  return computeSSD(model_->CtModel());
}

double mdm_DCEVoxel::computeSSD(const std::vector<double> &CtModel) const
{
  double  ssd = 0.0;
  for (int i = n1(); i < n2(); i++)
  {
    double diff = (CtData_[i] - CtModel[i]);
    ssd += (diff * diff) / noiseVar_[i];
  }

  return ssd;
}

/**
 * Pre-conditions:
 * -  All inputs adequately initialised
 *
 * Post-conditions
 * -  All outputs have valid values
 * -  G* file scope static all set (see below)
 *
 * Simplex set-up and results propagation.
 *
 * Depending on the model, the parameter and lambda arrays are set up (see NR)
 * then the simplex is run and the results are copied to the global mirror
 * variables.  Model-to-data SSD is also calculated and returned.
 *
 * Uses madym.h globals:
 * -  modelType_, max_iterations
 * Uses file-scope statics:
 * -  GnData, Gx, Gy, Gdose, Gk, Gve, Gvp, Goffset, GHct
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker (mods by GA Buonaccorsi)
 * @brief    Set up and do simplex fit to required model
 * @version  madym 1.21.alpha
 * @param    nTimes    Integer number of data points in the time series - INPUT
 * @param    x        Float array of timing data                       - INPUT
 * @param    y        Float array of CA(t) data                        - INPUT
 * @param    kTrans   Pointer to double holding Ktrans                  - OUTPUT
 * @param    ve       Pointer to double holding relative volume of EES  - OUTPUT
 * @param    vp       Pointer to double holding relative plasma volume  - OUTPUT
 * @param    offset   Pointer to double holding  offset time for AIF    - OUTPUT
 * @param    ssd      Pointer to double holding SSD for model v data    - OUTPUT
 */
void mdm_DCEVoxel::optimiseModel()
{
  /* do simplex minimisation on the parameters that aren't flagged as fixed*/
	//mdm_simplexMin::set_max_iterations(2000);
	//ssd = mdm_simplexMin::simplexmin(pkParamsOpt_, simplexLambdaOpt_, this, catSDDforwarder, FTOL);

  std::vector<double> optParams = model_->optParams();
	alglib::real_1d_array x;
	x.attach_to_ptr(optParams.size(), optParams.data());
	alglib::minbleicstate state;
	alglib::minbleicreport rep;

	//
	// These variables define stopping conditions for the optimizer.
	//
	// We use very simple condition - |g|<=epsg
	//
	double epsg = 0.000001;
	double epsf = 0;
	double epsx = 0;
#if _DEBUG
	alglib::ae_int_t maxits = 100;
#else
  alglib::ae_int_t maxits = 0;
#endif

	//
	// This variable contains differentiation step
	//
	double diffstep = 1.0e-4;

	//
	// Now we are ready to actually optimize something:
	// * first we create optimizer
	// * we add boundary constraints
	// * we tune stopping conditions
	// * and, finally, optimize and obtain results...
	//
  try
  {
    alglib::minbleiccreatef(x, diffstep, state);
    alglib::minbleicsetbc(state, lowerBoundsOpt_, upperBoundsOpt_);
    alglib::minbleicsetcond(state, epsg, epsf, epsx, maxits);
    alglib::minbleicoptimize(state, &CtSSDalglib, NULL, this);
    alglib::minbleicresults(state, x, rep);
  }
  catch (alglib::ap_error e)
  {
    printf("ALGLIB error msg: %s\n", e.msg.c_str());
  }
	

  //Copy the parameters we've optimised into optParams array
  for (size_t i = 0, n = optParams.size(); i < n; i++)
    optParams[i] = x[i];

	//Get the final model fit error - this also sets the parameters back to the model structure
  modelFitError_ = CtSSD(optParams);
}

//Run an initial model fit using the current model parameters (does not optimise new parameters)
MDM_API void mdm_DCEVoxel::initialiseModelFit(mdm_DCEModelBase &model)
{
  //Set the model
  model_ = &model;

  /*
  * Calculate [CA](t) from the signal intensity time-series array
  */
  if (CtData_.empty())
    computeCtFromSignal(); //If any problems, this will change permStatus setting

  if (noiseVar_.empty())
    noiseVar_.resize(n2_, 1.0);

  //Reset the model
  model_->reset(n2_);

  //Copy the lower bounds into the container required by the optimiser
  int nopt = model_->num_opt();
  lowerBoundsOpt_.setlength(nopt);
  upperBoundsOpt_.setlength(nopt);
  for (int i = 0; i < nopt; i++)
  {
    lowerBoundsOpt_[i] = model_->lowerBoundsOpt()[i];
    upperBoundsOpt_[i] = model_->upperBoundsOpt()[i];
  }

  //Get an initial SSD
  modelFitError_ = CtSSD(model_->optParams());
}

/**
 * Pre-conditions:
 * -  Permeability struct allocated and initialised
 *
 * Side-effects (Post-conditions)
 * -  Permeability struct holds valid parameter values for current voxel
 *
 * Uses madym.h globals:
 * -  full_filename[]
 *
 * Uses mdm_DCEVolumeAnalysis.h globals:
 * -  timings[]
 *
 * Note:  NOT a stand-alone fn - see pre-conditions & side-effects
 *
 * @author   GJM Parker (mods by GA Buonaccorsi)
 * @brief    Fit model for data in given Permeability struct
 * @version  madym 1.22
 * @param    p    Pointer to Permeability struct to hold model data
 */
MDM_API void mdm_DCEVoxel::fitModel()
{
  if (!model_)
    return;

  /* Use IAUC to check for enhancement (since 1.22 unless user says no */
  if (status_ != mdm_DCEVoxelStatus::OK && status_ != mdm_DCEVoxelStatus::DYN_T1_BAD)
  {
    model_->zeroParams();
		for (auto &iauc : IAUC_vals_)
			iauc = 0.0;
    modelFitError_ = 0.0;
    enhancing_ = 0;
  }
	else if (testEnhancement_ && ((IAUC_vals_[0] <= 0.0) || (IAUC_vals_[1] <= 0.0)))
  {
  /*
   * This is where I cut CJR's enhancement stuff
   * GAB 22 Feb 2012 (madym 1.21.alpha at BDL)
   */
    model_->zeroParams();
		for (auto &iauc : IAUC_vals_)
			iauc = 0.0;
    modelFitError_ = 0.0;
    enhancing_ = 0;
  }
  else
  {
    /* The voxel is enhancing, so set the map value */
    enhancing_ = 1;
    
    /*
     * Fit pharmacokinetic model
     */
		optimiseModel();
  }
}

/**
* Moved here for version 2.0
* MB - This is a fucntion at the voxel level
*
* Depending on the model, the parameter and lambda arrays are set up (see NR)
* then the simplex is run and the results are copied to the global mirror
* variables.  Model-to-data SSD is also calculated and returned.
*
* Uses madym.h globals:
* -  mdmCfg.prebolus
*
* Note:  NOT a stand-alone fn - see pre-conditions & side-effects
*
* @author   GJM Parker (mods by GA Buonaccorsi)
* @brief    Calculate Integrated area under the CA(t) curve (simple summation)
* @version  madym 1.21.alpha
* @param    nTimes           Integer number of data points in the time series - INPUT
* @param    dynamicTimings_         Float array of timing data                       - INPUT
* @param    concentration   Float array of CA(t) data                        - INPUT
* @param    IAUC60          Pointer to double holding AUC to 60 s             - OUTPUT
* @param    IAUC90          Pointer to double holding AUC to 90 s             - OUTPUT
* @param    IAUC120         Pointer to double holding AUC to 120 s            - OUTPUT
*/
MDM_API void mdm_DCEVoxel::IAUC_calc()
{
	/*MB must check - are dynamic timings in seconds or minutes? Adjust to seconds here if so*/
	size_t nTimes = dynamicTimings_.size();

	for (auto & iauc : IAUC_vals_)
		iauc = 0.0;

  for (size_t i_t = bolus_time_; i_t < nTimes; i_t++)
	{

		for (size_t i_iauc = 0, n_iauc = IAUC_times_.size(); i_iauc < n_iauc; i_iauc++)
		{
			if (dynamicTimings_[i_t] - dynamicTimings_[bolus_time_] <= IAUC_times_[i_iauc])
				IAUC_vals_[i_iauc] += (CtData_[i_t] + CtData_[i_t - 1]) *
				(dynamicTimings_[i_t] - dynamicTimings_[i_t - 1]) / 2.0;

		}
	}
}

MDM_API mdm_DCEVoxel::mdm_DCEVoxelStatus mdm_DCEVoxel::status() const
{
  return status_;
}

MDM_API int mdm_DCEVoxel::n1() const
{
	return n1_;
}
MDM_API int mdm_DCEVoxel::n2() const
{
	return n2_;
}
MDM_API const std::vector<double>& mdm_DCEVoxel::signalData() const
{
	return signalData_;
}
MDM_API const std::vector<double>&	mdm_DCEVoxel::CtData() const
{
	return CtData_;
}
MDM_API const std::vector<double>&	mdm_DCEVoxel::CtModel() const
{
  return model_->CtModel();
}

MDM_API double     mdm_DCEVoxel::t10() const
{
	return t10_;
}
MDM_API double     mdm_DCEVoxel::s0() const
{
	return s0_;
}
MDM_API double     mdm_DCEVoxel::r1cont() const
{
	return r1cont_;
}
MDM_API double     mdm_DCEVoxel::IAUC_val(int i) const
{
	return IAUC_vals_[i];
}
MDM_API double			mdm_DCEVoxel::IAUC_time(int i) const
{
	return IAUC_times_[i];
}

MDM_API double     mdm_DCEVoxel::modelFitError() const
{
	return modelFitError_;
}
MDM_API bool       mdm_DCEVoxel::enhancing() const
{
	return enhancing_;
}

MDM_API bool			mdm_DCEVoxel::testEnhancement() const
{
	return testEnhancement_;
}
MDM_API bool			mdm_DCEVoxel::useRatio() const
{
	return useRatio_;
}
