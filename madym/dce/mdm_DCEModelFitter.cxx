/**
*  @file    mdm_DCEModelFitter.cxx
*  @brief   Implementation of mdm_DCEModelFitter class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS
#include "mdm_DCEModelFitter.h"

//#include <cmath>
#include <algorithm>

#include "opt/optimization.h"
#include "opt/interpolation.h"
#include "opt/linalg.h"

#include <madym/utils/mdm_exception.h>
#include <cfloat>

MDM_API mdm_DCEModelFitter::mdm_DCEModelFitter(
	mdm_DCEModelBase &model,
	const size_t timepoint0,
	const size_t timepointN,
  const std::vector<double> &noiseVar,
  const std::string &type,
	const int maxIterations)
	:
	model_(model),
  timepoint0_(timepoint0),
	timepointN_(timepointN),
  noiseVar_(noiseVar),
  modelFitError_(0),
  type_(typeFromString(type)),
	maxIterations_(maxIterations),
  BAD_FIT_SSD(DBL_MAX)
{
}

MDM_API mdm_DCEModelFitter::~mdm_DCEModelFitter()
{

}

MDM_API std::string mdm_DCEModelFitter::toString(FitterTypes type)
{
  switch (type)
  {
  case LLS: return "LLS";
  case BLEIC: return "BLEIC";
  case NS: return "NS";
  default:
    throw mdm_exception(__func__, "Unknown optimisation type option " + type);
  }
}

MDM_API const std::vector<std::string> mdm_DCEModelFitter::validTypes()
{
  return {
    toString(LLS),
    toString(BLEIC),
    toString(NS)
  };
}

//
MDM_API mdm_DCEModelFitter::FitterTypes mdm_DCEModelFitter::typeFromString(const std::string& type)
{
  if (type == toString(LLS))
    return LLS;
  if (type == toString(BLEIC))
    return BLEIC;
  else if (type == toString(NS))
    return NS;
  else
    throw mdm_exception(__func__, boost::format(
      "Optimisation type (%1%) is not recognised. Must be one of LLS, BLEIC or NS")
      % type);
}

//Run an initial model fit using the current model parameters (does not optimise new parameters)
MDM_API void mdm_DCEModelFitter::initialiseModelFit(const std::vector<double> &CtData)
{
  //Set CtData (this will destory old object)
  CtData_ = &CtData;

  //Check timepointN_ valid
  if (timepointN_ <= 0 || timepointN_ > CtData.size())
    timepointN_ = CtData.size();

  //Check timepoint0_ valid
  if (timepointN_ <= 0 || timepoint0_ >= timepointN_)
    timepoint0_ = 0;

  //Reset the model
  model_.reset(timepointN_);

  //If NULL model, just return
  if (!model_.numParams())
    return;

  //Set default uniform noise is temporally varying noise not set
  if (noiseVar_.empty())
    noiseVar_.resize(timepointN_, 1.0);

  //Copy the lower bounds into the container required by the optimiser
  int nopt = model_.numOptimised();
  lowerBoundsOpt_.setlength(nopt);
  upperBoundsOpt_.setlength(nopt);
  for (int i = 0; i < nopt; i++)
  {
    lowerBoundsOpt_[i] = model_.optimisedLowerBounds()[i];
    upperBoundsOpt_[i] = model_.optimisedUpperBounds()[i];
  }

  //Get an initial SSD
  modelFitError_ = CtSSD();
}

//
MDM_API void mdm_DCEModelFitter::fitModel(
  const mdm_DCEVoxel::mdm_DCEVoxelStatus status)
{
  //If NULL model, just return
  if (!model_.numParams())
    return;

  //Check CtData has been set
  if (!CtData_)
    throw mdm_exception(__func__, "CtData not set");

  //Check if any issues with voxel.
  if (
    status != mdm_DCEVoxel::mdm_DCEVoxelStatus::OK &&
    status != mdm_DCEVoxel::mdm_DCEVoxelStatus::DYN_T1_BAD)
  {
    model_.zeroParams();
    modelFitError_ = 0.0;
    return;
  }

  //Fit pharmacokinetic model
  optimiseModel();

}

MDM_API size_t mdm_DCEModelFitter::timepoint0() const
{
  return timepoint0_;
}
MDM_API size_t mdm_DCEModelFitter::timepointN() const
{
  return timepointN_;
}

MDM_API const std::vector<double>&	mdm_DCEModelFitter::CtModel() const
{
  return model_.CtModel();
}

MDM_API double mdm_DCEModelFitter::modelFitError() const
{
  return modelFitError_;
}

//-----------------------------------------------------------------------
// Private
//-------------------------------------------------------------------------

//
double mdm_DCEModelFitter::CtSSD()
{

	//Get model to check params are ok - returns non-zero for bad value
  model_.checkParams();
  if (model_.getModelErrorCode() != mdm_ErrorTracker::ErrorCode::OK)
    return BAD_FIT_SSD;

	//If we got this far the parameter values look OK, so ...
	// Calculate model [CA](t) for current parameter set
	model_.computeCtModel(timepointN_);

	//Compute SSD
  return computeSSD(model_.CtModel());
}

//
double mdm_DCEModelFitter::CtSSD(
  const std::vector<double>& parameter_array)
{
  //Set the full parameter array in the model from the optimised subset
  model_.setOptimisedParams(parameter_array);

  return CtSSD();
}

//
double mdm_DCEModelFitter::computeSSD(
  const std::vector<double> &CtModel) const
{
  double  ssd = 0.0;
  for (size_t i = timepoint0_; i < timepointN_; i++)
  {
    double diff = ((*CtData_)[i] - CtModel[i]);
    ssd += (diff * diff) / noiseVar_[i];
  }

  return ssd;
}

//
void mdm_DCEModelFitter::optimiseModel()
{
  //Check if we're repeating fits at a given parameter
  if (model_.singleFit())
    optimiseModelOnce();

  else
  {
    //For repeat fits, we loop over the repeats, saving the parameters that provide best fit
    lowestModelFitError_ = DBL_MAX;
    while (model_.nextRepeatParam())
    {
      optimiseModelOnce();
      if (modelFitError_ < lowestModelFitError_)
      {
        bestParams_ = model_.params();
        lowestModelFitError_ = modelFitError_;
      }
    }
    //Once complete, set the model parameters to the saved best ones
    //Recompute modelled C(t), and set the model fit error
    model_.setParams(bestParams_);
    model_.computeCtModel(timepointN_);
    modelFitError_ = lowestModelFitError_;
  }
    

  //Reset CtData_ to NULL, this forces the user to call initialiseModelFit before fitModel
  //and avoids potential dangling pointer. No danger of memory leak because CtData_ is never 
  //created from new.
  CtData_ = NULL;
}

//
void mdm_DCEModelFitter::optimiseModelOnce()
{
  bool use_nonlinear = type_ != FitterTypes::LLS;
  auto optimisedParams = model_.optimisedParams();

  if (use_nonlinear)
  {
    
    alglib::real_1d_array x;
    x.attach_to_ptr(optimisedParams.size(), optimisedParams.data());

#if _DEBUG
    alglib::ae_int_t maxits = std::max(maxIterations_, 100);
#else
    alglib::ae_int_t maxits = maxIterations_;
#endif

    switch (type_)
    {
    case BLEIC:
      optimiseModel_bleic(x, maxits); break;
    case NS:
      optimiseModel_ns(x, maxits); break;

    default:
      throw mdm_exception(__func__, "Optimisation type not recognised");
    }
    //optimiseModel_ns(x, maxits);
    

    //Copy the parameters we've optimised into optimisedParams array
    for (size_t i = 0, n = optimisedParams.size(); i < n; i++)
      optimisedParams[i] = x[i];

    //Set the full parameter array in the model from the optimised subset
    model_.setOptimisedParams(optimisedParams);
  }
  else
    optimiseModel_lls();

  //Get the final model fit error
  modelFitError_ = CtSSD();
}

void mdm_DCEModelFitter::optimiseModel_ns(alglib::real_1d_array &x, alglib::ae_int_t maxits)
{
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
    alglib::minnscreatef(x, diffstep, state);
    alglib::minnssetalgoags(state, radius, rho);
    alglib::minnssetcond(state, epsx, maxits);

    alglib::minnssetbc(state, lowerBoundsOpt_, upperBoundsOpt_);
    alglib::minnsoptimize(state, &CtSSDalglib, NULL, this);
    alglib::minnsresults(state, x, rep);
  }
  catch (alglib::ap_error e)
  {
    printf("ALGLIB error msg: %s\n", e.msg.c_str());
  }
}

void mdm_DCEModelFitter::optimiseModel_bleic(alglib::real_1d_array& x, alglib::ae_int_t maxits)
{
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
}

void mdm_DCEModelFitter::optimiseModel_lls()
{
  // Solve w . C = w . A . B for B where
  // C is the observed signal-derived CA concentration (CtData_)
  // A is the matrix for LLS solving returned by the model class
  // W is a set of weights
  // B contains the model parameters

  //Set up A, calling the models LLS matrix generator. If LLS solving
  //isn't set up for the specific model sub-class, an exception will be thrown
  alglib::real_2d_array A; //N x M
  const auto& LLSmat = model_.makeLLSMatrix(*CtData_);
  auto N = CtData_->size();
  auto M = LLSmat.size() / N;
  A.setcontent(N, M, LLSmat.data());
  
  //Copy the signal derived CA time-serie sinto C
  alglib::real_1d_array C; //N x 1
  C.setcontent(CtData_->size(), CtData_->data());

  //Copy the temporally varying noise variance into W
  alglib::real_1d_array w;
  w.setlength(N);
  for (size_t i = 0; i < N; i++)
    w[i] = 1 / noiseVar_[i]; //Need to invert

  alglib::ae_int_t info;
  alglib::real_1d_array B; //M x 1
  alglib::lsfitreport rep;

  //
  // Linear fitting with weights
  //
  alglib::lsfitlinearw(C, w, A, info, B, rep);

  model_.transformLLSolution(B.getcontent());
}