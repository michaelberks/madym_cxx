/**
*  @file    mdm_DCEVoxel.cxx
*  @brief   Implementation of mdm_DCEVoxel class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS
#include "mdm_DCEVoxel.h"

#include <cmath>

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
	mdm_DCEModelBase &model,
  const std::vector<double> &dynSignals,
	const std::vector<double> &dynConc,
  const std::vector<double> &noiseVar,
	const double T10,
	const double M0,
	const double r1Const,
  const int injectionImg,
	const std::vector<double> &dynamicTimings,
	const double TR,
	const double FA,
	const int timepoint0,
	const int timepointN,
	const bool testEnhancement,
	const bool useM0Ratio,
	const std::vector<double> &IAUC_times)
	:
	model_(model),
  signalData_(dynSignals),
	CtData_(dynConc),
  noiseVar_(noiseVar),
	t10_(T10),
	m0_(M0),
	timepoint0_(timepoint0),
	timepointN_(timepointN),
	r1Const_(r1Const),
  injectionImg_(injectionImg),
	IAUC_times_(IAUC_times),
	IAUC_vals_(0),
	modelFitError_(0),
	enhancing_(0),
	dynamicTimings_(dynamicTimings),
	TR_(TR),
	FA_(FA),
	testEnhancement_(testEnhancement),
	useM0Ratio_(useM0Ratio),
  status_(mdm_DCEVoxelStatus::OK)
{
}

MDM_API mdm_DCEVoxel::~mdm_DCEVoxel()
{

}

//
MDM_API void mdm_DCEVoxel::computeCtFromSignal()
{
  double  R1gd = r1Const() * 0.001;  // Use millisec instead of sec (as in user interface)

  //
  const size_t &n_times = signalData().size();
  CtData_.resize(n_times);

  // Only calculate if T1(0) > 0.0
  if (T1() <= 0.0)
  {
		mdm_ProgramLogger::logProgramMessage(
			"Warning: mdm_DCEVoxel::computeCtFromSignal: Baseline T1 <= 0.0\n");
    status_ = mdm_DCEVoxelStatus::T10_BAD;
    return;
  }
  
  const auto PI = acos(-1.0);
  double sinfa = sin(FA_ * PI / 180);
  double cosfa = cos(FA_ * PI / 180);

  // Calculate dyn_pbm
  // Need to check that we've got the pb time points
  if (injectionImg_ >= timepoint0())
  {
    double   sum_pbm = 0.0;
    int     num_pbm = 0;
    for (int k = timepoint0(); k < injectionImg_; k++)
    {
      sum_pbm += signalData()[k];
      num_pbm++;
    }
    double s_pbm = sum_pbm / num_pbm;

    for (size_t k = 0; k < n_times; k++)
    {
      double R1value;
      int errorCode;
      if (useM0Ratio())
        R1value = computeT1DynPBM(signalData()[k], s_pbm, cosfa, errorCode);
      else
        R1value = computeT1DynM0(signalData()[k], sinfa, cosfa, errorCode);    
      
      CtData_[k] = (R1value - 1.0 / T1()) / R1gd;

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
      status_ = mdm_DCEVoxelStatus::M0_BAD;
    }      
  }
}

//
double mdm_DCEVoxel::computeT1DynPBM(const double &st, const double &s_pbm, const double &cosfa, int &errorCode)
{
  //  Yes, it looks horrible and over-complicated.  I don't care.
  //  Too many div by zeros to account for, and a log zero to boot
  errorCode = 0;
  if (s_pbm < T1_TOLERANCE)
    errorCode = -1;

  double expTR_T10 = exp(-TR_ / T1());
  double S1_M0 = st / s_pbm;

  double denominator = 1.0 - cosfa * expTR_T10;
  if (std::abs(denominator) < T1_TOLERANCE)
    errorCode = -2;

  double fraction1 = (1.0 - expTR_T10) / denominator;

  denominator = 1.0 - S1_M0 * cosfa * fraction1;
  if (std::abs(denominator) < T1_TOLERANCE)
    errorCode = -3;

  double fraction2 = (1.0 - S1_M0 * fraction1) / denominator;
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

//
double mdm_DCEVoxel::computeT1DynM0(const double &st, const double & sinfa, const double & cosfa, int &errorCode)
{
  errorCode = 0;
  double num = M0() * sinfa - st;
  double denom = M0() * sinfa - st * cosfa;
  double   R1_t = R1_t = -log(num / denom) / TR_;

  if (std::abs(num) < T1_TOLERANCE)
    errorCode = -1;
  else if (std::abs(denom) < T1_TOLERANCE)
    errorCode = -2;

  return R1_t;
}

//
std::vector<double> mdm_DCEVoxel::computeIAUC(const std::vector<double> &times)
{
	auto nIAUC = times.size();
	std::vector<double> vals(nIAUC, 0);

	if (!nIAUC)
		return vals;

	size_t nTimes = dynamicTimings_.size();

	double cumulativeCt = 0;
	const double bolusTime = dynamicTimings_[injectionImg_];

	//This relies on IAUC times being sorted, which we enforce externally to save
	//time, but for robustness could do so here?
	int currIAUCt = 0;
	for (auto i_t = injectionImg_; i_t < nTimes; i_t++)
	{
		double elapsedTime = dynamicTimings_[i_t] - bolusTime;
		double delta_t = dynamicTimings_[i_t] - dynamicTimings_[i_t - 1];
		double delta_Ct = CtData_[i_t] + CtData_[i_t - 1];
		double addedCt = delta_t * delta_Ct / 2.0;

		//If we exceed time for any IAUC time, set the val
		if (elapsedTime > times[currIAUCt])
		{
			//Compute the extra littl ebit of trapezium...
			double t_frac = 1.0 - (elapsedTime - times[currIAUCt]) / delta_t;

			vals[currIAUCt] = cumulativeCt + t_frac * addedCt;

			//If this was the last time point, we can break
			if (currIAUCt == nIAUC - 1)
				break;
			else
				currIAUCt++;;
		}

		//Add to the cumulative Ct
		cumulativeCt += addedCt;
	}
	return vals;
}

bool mdm_DCEVoxel::checkEnhancing()
{
	bool enhancing = true;
	if (IAUC_vals_.empty())
	{
		auto iauc60 = computeIAUC({ 1.0 });
		enhancing = iauc60[0] > 0;
	}
	else
	{
		// TODO better test enhancement checks?
		for (const auto iauc : IAUC_vals_)
		{
			if (iauc <= 0.0)
			{
				enhancing = false;
				break;
			}
		}
	}
	return enhancing;
}

//
double mdm_DCEVoxel::CtSSD(const std::vector<double> &parameter_array)
{
  //Set the full parameter array in the model from the optimised subset
  model_.setOptimisedParams(parameter_array);

	//Get model to check params are ok - returns non-zero for bad value
  double model_check = model_.checkParams();
  if (model_check)
    return model_check;

	//If we got this far the parameter values look OK, so ...
	// Calculate model [CA](t) for current parameter set
	model_.computeCtModel(timepointN());

	//Compute SSD
  return computeSSD(model_.CtModel());
}

double mdm_DCEVoxel::computeSSD(const std::vector<double> &CtModel) const
{
  double  ssd = 0.0;
  for (int i = timepoint0(); i < timepointN(); i++)
  {
    double diff = (CtData_[i] - CtModel[i]);
    ssd += (diff * diff) / noiseVar_[i];
  }

  return ssd;
}

//
void mdm_DCEVoxel::optimiseModel()
{

  std::vector<double> optimisedParams = model_.optimisedParams();
	alglib::real_1d_array x;
	x.attach_to_ptr(optimisedParams.size(), optimisedParams.data());
	//alglib::minbleicstate state;
	//alglib::minbleicreport rep;
	alglib::minnsstate state;
	alglib::minnsreport rep;

	//
	// These variables define stopping conditions for the optimizer.
	//
	// We use very simple condition - |g|<=epsg
	//
	//double epsg = 0.000001;
	//double epsf = 0;
	double epsx = 0;

	double radius = 0.1;
	double rho = 0.0;

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
    //alglib::minbleiccreatef(x, diffstep, state);
    //alglib::minbleicsetbc(state, lowerBoundsOpt_, upperBoundsOpt_);
    //alglib::minbleicsetcond(state, epsg, epsf, epsx, maxits);
    //alglib::minbleicoptimize(state, &CtSSDalglib, NULL, this);
    //alglib::minbleicresults(state, x, rep);

		alglib::minnscreatef(x, diffstep, state);
		alglib::minnssetalgoags(state, radius, rho);
		alglib::minnssetcond(state, epsx, maxits);
		//alglib::minnssetscale(state, s);

		alglib::minnssetbc(state, lowerBoundsOpt_, upperBoundsOpt_);
		alglib::minnsoptimize(state, &CtSSDalglib, NULL, this);
		alglib::minnsresults(state, x, rep);
  }
  catch (alglib::ap_error e)
  {
    printf("ALGLIB error msg: %s\n", e.msg.c_str());
  }
	

  //Copy the parameters we've optimised into optimisedParams array
  for (size_t i = 0, n = optimisedParams.size(); i < n; i++)
    optimisedParams[i] = x[i];

	//Get the final model fit error - this also sets the parameters back to the model structure
  modelFitError_ = CtSSD(optimisedParams);
}

//Run an initial model fit using the current model parameters (does not optimise new parameters)
MDM_API void mdm_DCEVoxel::initialiseModelFit()
{

  // Calculate [CA](t) from the signal intensity time-series array
  if (CtData_.empty())
    computeCtFromSignal(); //If any problems, this will change permStatus setting

  if (noiseVar_.empty())
    noiseVar_.resize(timepointN_, 1.0);

  //Reset the model
  model_.reset(timepointN_);

  //Copy the lower bounds into the container required by the optimiser
  int nopt = model_.num_optimised();
  lowerBoundsOpt_.setlength(nopt);
  upperBoundsOpt_.setlength(nopt);
  for (int i = 0; i < nopt; i++)
  {
    lowerBoundsOpt_[i] = model_.optimisedLowerBounds()[i];
    upperBoundsOpt_[i] = model_.optimisedUpperBounds()[i];
  }

  //Get an initial SSD
  modelFitError_ = CtSSD(model_.optimisedParams());
}

//
MDM_API void mdm_DCEVoxel::fitModel()
{
  enhancing_ = true;

  //Check if any issues with voxel
  if (status_ != mdm_DCEVoxelStatus::OK && status_ != mdm_DCEVoxelStatus::DYN_T1_BAD)
  {
    model_.zeroParams();
		for (auto &iauc : IAUC_vals_)
			iauc = 0.0;
    modelFitError_ = 0.0;
    enhancing_ = false;
  }

	// Use IAUC to check for enhancement (since 1.22 unless user says no)
	else if (testEnhancement_)
	{
		enhancing_ = checkEnhancing();
	}
	if (enhancing_)
	{	
		//Fit pharmacokinetic model
		optimiseModel();
  }
	else
	{
		model_.zeroParams();
		for (auto &iauc : IAUC_vals_)
			iauc = 0.0;
		modelFitError_ = 0.0;
	}
}

//
MDM_API void mdm_DCEVoxel::computeIAUC()
{
	IAUC_vals_ = computeIAUC(IAUC_times_);
}

MDM_API mdm_DCEVoxel::mdm_DCEVoxelStatus mdm_DCEVoxel::status() const
{
  return status_;
}

MDM_API int mdm_DCEVoxel::timepoint0() const
{
	return timepoint0_;
}
MDM_API int mdm_DCEVoxel::timepointN() const
{
	return timepointN_;
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
  return model_.CtModel();
}

MDM_API double     mdm_DCEVoxel::T1() const
{
	return t10_;
}
MDM_API double     mdm_DCEVoxel::M0() const
{
	return m0_;
}
MDM_API double     mdm_DCEVoxel::r1Const() const
{
	return r1Const_;
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
MDM_API bool			mdm_DCEVoxel::useM0Ratio() const
{
	return useM0Ratio_;
}
