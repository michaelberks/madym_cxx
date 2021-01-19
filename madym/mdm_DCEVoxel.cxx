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


#include <algorithm>

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

const double mdm_DCEVoxel::Ca_BAD1 = -1.0e3;
const double mdm_DCEVoxel::Ca_BAD2 = -2.0e3;
const double mdm_DCEVoxel::T1_TOLERANCE = 1.0e-6;
const double mdm_DCEVoxel::DYN_T1_MAX = 1.0e9;
const double mdm_DCEVoxel::DYN_T1_INVALID = -1.0;

MDM_API mdm_DCEVoxel::mdm_DCEVoxel(
	const std::vector<double> &dynSignals,
	const std::vector<double> &dynConc,
  const size_t injectionImg,
	const std::vector<double> &dynamicTimings,
	const std::vector<double> &IAUC_times)
	:
	StData_(dynSignals),
	CtData_(dynConc),
  injectionImg_(injectionImg),
	IAUC_times_(IAUC_times),
	IAUC_vals_(0),
	enhancing_(true),
	dynamicTimings_(dynamicTimings),
	status_(mdm_DCEVoxelStatus::OK)
{
}

MDM_API mdm_DCEVoxel::~mdm_DCEVoxel()
{

}

//
MDM_API void mdm_DCEVoxel::computeCtFromSignal(
  const double T1, const double FA, const double TR, const double r1Const,
  const double M0, const double B1, size_t timepoint0)
{
  //Only apply if we have signal data to convert
  const auto &nTimes = StData().size();
  if (!nTimes)
    return;

  double r1Const_ms = r1Const*0.001;  // Use millisec instead of sec (as in user interface)
  CtData_.resize(nTimes);

  // Only calculate if T1(0) > 0.0
  if (T1 <= 0.0)
  {
		mdm_ProgramLogger::logProgramWarning(__func__, " Baseline T1 <= 0.0");
    status_ = mdm_DCEVoxelStatus::T10_BAD;
    return;
  }

  // Calculate dyn_pbm
  double meanPrebolusSignal;
  if (!M0)
  {
    // Need to check that we've got the pb time points
    if (injectionImg_ > timepoint0)
    {
      double prebolusSum = 0.0;
      size_t nPrebolus = 0;
      for (size_t k = timepoint0; k < injectionImg_; k++)
      {
        prebolusSum += StData()[k];
        nPrebolus++;
      }
      meanPrebolusSignal = prebolusSum / nPrebolus;
    }
    else
    {
      for (auto & c : CtData_)
        c = Ca_BAD1;
        
      status_ = mdm_DCEVoxelStatus::M0_BAD;
      return;
    }  
  }
 
  //Precompute cos and sin FA, corrected by B1 value
  const auto PI = acos(-1.0);
  double sinFA = sin(B1 *FA * PI / 180);
  double cosFA = cos(B1 *FA * PI / 180);

  //Compute R1 and hence signal at each time point
  for (size_t k = 0; k < nTimes; k++)
  {
    double R1value;
    int errorCode;
    if (M0)
      R1value = computeT1DynM0(StData_[k], M0, cosFA, sinFA, TR, errorCode);
    else
      R1value = computeT1DynPBM(StData_[k], meanPrebolusSignal, T1, cosFA, sinFA, TR, errorCode);
           
    CtData_[k] = (R1value - 1.0 / T1) / r1Const_ms;

    if (errorCode)
      status_ = mdm_DCEVoxelStatus::DYN_T1_BAD;

    else if (std::isnan(CtData_[k]))
      status_ = mdm_DCEVoxelStatus::CA_NAN;
  }

}

//
double mdm_DCEVoxel::computeT1DynPBM(const double st, const double meanPrebolusSignal, 
  const double T1, const double cosFA, const double sinFA, const double TR, int &errorCode)
{
  //  Yes, it looks horrible and over-complicated.  I don't care.
  //  Too many div by zeros to account for, and a log zero to boot
  errorCode = 0;
  if (meanPrebolusSignal < T1_TOLERANCE)
    errorCode = -1;

  auto expTR_T10 = exp(-TR / T1);
  auto S1_M0 = st / meanPrebolusSignal;

  auto denominator = 1.0 - cosFA * expTR_T10;
  if (std::abs(denominator) < T1_TOLERANCE)
    errorCode = -2;

  auto fraction1 = (1.0 - expTR_T10) / denominator;

  denominator = 1.0 - S1_M0 * cosFA * fraction1;
  if (std::abs(denominator) < T1_TOLERANCE)
    errorCode = -3;

  auto fraction2 = (1.0 - S1_M0 * fraction1) / denominator;
  if (std::abs(fraction2) < T1_TOLERANCE)
    errorCode = -3;

  auto  R1_t = log(fraction2) / -TR;
  if (R1_t < 0.0)
    errorCode = -4;
  else if (1.0 / R1_t > DYN_T1_MAX)
    errorCode = -5;

  return R1_t;
}

//
double mdm_DCEVoxel::computeT1DynM0(const double st, const double M0,
  const double cosFA, const double sinFA, const double TR, int &errorCode)
{
  errorCode = 0;
  auto num = M0 * sinFA - st;
  auto denom = M0 * sinFA - st * cosFA;
  auto R1_t = -log(num / denom) / TR;

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
	size_t currIAUCt = 0;
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

//
MDM_API void mdm_DCEVoxel::computeIAUC()
{
	IAUC_vals_ = computeIAUC(IAUC_times_);
}

MDM_API mdm_DCEVoxel::mdm_DCEVoxelStatus mdm_DCEVoxel::status() const
{
  return status_;
}

//
MDM_API const std::vector<double>& mdm_DCEVoxel::StData() const
{
	return StData_;
}

//
MDM_API const std::vector<double>&	mdm_DCEVoxel::CtData() const
{
	return CtData_;
}

//
MDM_API double mdm_DCEVoxel::IAUC_val(size_t i) const
{
  if (i >= IAUC_vals_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access IAUC value %1% when there are only %2% IAUC times")
      % i % IAUC_vals_.size());
    
  return IAUC_vals_[i];
}

//
MDM_API double mdm_DCEVoxel::IAUC_time(size_t i) const
{
  if (i >= IAUC_times_.size())
    throw mdm_exception(__func__, boost::format(
      "Attempting to access IAUC time %1% when there are only %2% IAUC times")
      % i % IAUC_times_.size());

  return IAUC_times_[i]; 
}

//
MDM_API bool mdm_DCEVoxel::enhancing() const
{
	return enhancing_;
}

//
MDM_API void mdm_DCEVoxel::testEnhancing()
{
  bool enhancing_ = true;
  if (IAUC_vals_.empty())
  {
    auto iauc60 = computeIAUC({ 1.0 });
    enhancing_ = iauc60[0] > 0;
  }
  else
  {
    // TODO better test enhancement checks?
    for (const auto iauc : IAUC_vals_)
    {
      if (iauc <= 0.0)
      {
        enhancing_ = false;
        break;
      }
    }
  }
  if (!enhancing_)
    status_ = NON_ENHANCING;
}

