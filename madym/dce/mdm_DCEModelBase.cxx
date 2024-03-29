/**
*  @file    mdm_DCEModelBase.cxx
*  @brief   Implementation of abtract base class for DCE models 
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DCEModelBase.h"
#include <madym/utils/mdm_platform_defs.h>

MDM_API mdm_DCEModelBase::mdm_DCEModelBase(
  mdm_AIF &AIF,
  const std::vector<std::string> &paramNames,
  const std::vector<double> &initialParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
  const std::vector<double>& lowerBounds,
  const std::vector<double>& upperBounds,
	const std::vector<int> &relativeLimitParams,
	const std::vector<double> &relativeLimitValues,
  int repeatParam,
  const std::vector<double>& repeatValues)
  :CtModel_(0),
  AIF_(AIF),
  pkParams_(0),
  pkParamsOpt_(0),
  pkParamNames_(paramNames),
  pkInitParams_(initialParams),
  lowerBounds_(lowerBounds),
  upperBounds_(upperBounds),
  repeatParam_(repeatParam-1),
  repeatValues_(repeatValues),
	errorCode_(mdm_ErrorTracker::OK),
  currRpt_(0)
{
  
}

MDM_API mdm_DCEModelBase::~mdm_DCEModelBase()
{

}

MDM_API void mdm_DCEModelBase::init(
	const std::vector<int> &fixedParams,
	const std::vector<double> &fixedValues,
	const std::vector<int> &relativeLimitParams,
	const std::vector<double> &relativeLimitValues)
{
  //Check if we have any fixed parameters
  lowerBoundsOpt_.clear();
  upperBoundsOpt_.clear();
  
  //Set optimise flag to false for any fixed parameters
  for (int i = 0; i < fixedParams.size(); i++)
  {
    int fp = fixedParams[i] - 1;
    if (fp < optParamFlags_.size())
    {
      optParamFlags_[fp] = false;
      
      //If fixed values are supplied, use these to overwrite default
      //initial parameters
      if (i < fixedValues.size())
        pkInitParams_[fp] = fixedValues[i];
    }    
  }

  //If we have repeat parameter, this also is fixed
  if (repeatParam_ >= 0 && repeatParam_ < optParamFlags_.size())
    optParamFlags_[repeatParam_] = 0;

	//Set relative limits for any specified parameters
	relativeBounds_.resize(numParams(), 0);
	for (int i = 0; i < relativeLimitParams.size(); i++)
	{
		int rp = relativeLimitParams[i] - 1;
		if (rp < numParams())
		{
			//If fixed values are supplied, use these to overwrite default
			//initial parameters
			if (i < relativeLimitValues.size())
				relativeBounds_[rp] = relativeLimitValues[i];
		}
	}
    
  //Set upper and lower bounds for parameters to be optimised
  for (int i = 0; i < numParams(); i++)
  {
    if (optParamFlags_[i])
    {
      lowerBoundsOpt_.push_back(lowerBounds_[i]);
      upperBoundsOpt_.push_back(upperBounds_[i]);
    }
  }

	//Reset to size of AIF
	reset(AIF_.AIFTimes().size());
}

MDM_API void mdm_DCEModelBase::reset(size_t nTimes)
{
  if (nTimes)
    CtModel_.resize(nTimes);

  if (!numParams())
    return;

  pkParams_ = pkInitParams_;
  pkParamsOpt_.clear();
  for (int i = 0, j = 0; i < numParams(); i++)
  {
    if (optParamFlags_[i])
      pkParamsOpt_.push_back(pkParams_[i]);
  }
  
}

MDM_API int mdm_DCEModelBase::numParams() const
{
  return (int)pkInitParams_.size();
}

MDM_API int mdm_DCEModelBase::numOptimised() const
{
  return (int)pkParamsOpt_.size();  
}

MDM_API int mdm_DCEModelBase::numFixed() const
{
  return numParams() - numOptimised();
}

MDM_API const std::vector<double>& mdm_DCEModelBase::CtModel()
{
  return CtModel_;
}

MDM_API std::vector<double>& mdm_DCEModelBase::optimisedParams()
{
  return pkParamsOpt_;
}

MDM_API void mdm_DCEModelBase::setOptimisedParams(const std::vector<double>& optimisedParams)
{
  for (int i = 0, j = 0, n = numParams(); i < n; i++)
  {
    if (optParamFlags_[i])
    {
      pkParams_[i] = optimisedParams[j];
      pkParamsOpt_[j] = optimisedParams[j];
      j++;
    }
  }
}

MDM_API void mdm_DCEModelBase::setParams(const std::vector<double>& params)
{
  pkParams_ = params;
}

MDM_API void mdm_DCEModelBase::setInitialParams(const std::vector<double>& params)
{
  pkInitParams_ = params;
	
	//See if we have any relative bounds to update
	for (int i = 0, n = numParams(), j = 0; i < n; i++)
	{
		if (optParamFlags_[i])
		{
			if (relativeBounds_[i])
			{
				//Make sure relative bounds don't push past lower/upper limits
				lowerBoundsOpt_[j] = std::fmax(lowerBounds_[i], pkInitParams_[i] - relativeBounds_[i]);
				upperBoundsOpt_[j] = std::fmin(upperBounds_[i], pkInitParams_[i] + relativeBounds_[i]);
			}
			j++;
		}
	}
		
}

MDM_API void mdm_DCEModelBase::zeroParams()
{
  for (int i = 0, n = numParams(); i < n; i++)
    pkParams_[i] = 0;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::optimisedLowerBounds()
{
  return lowerBoundsOpt_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::optimisedUpperBounds()
{
  return upperBoundsOpt_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::params() const
{
	return pkParams_;
}

MDM_API double     mdm_DCEModelBase::params(int paramIdx) const
{
  return pkParams_[paramIdx];
}
MDM_API double     mdm_DCEModelBase::params(const std::string &paramName) const
{
  for (int i = 0; i < pkParamNames_.size(); i++)
  {
    if (paramName == pkParamNames_[i])
      return pkParams_[i];
  }
  return FLT_MIN;
}

MDM_API const std::vector<double>&     mdm_DCEModelBase::initialParams() const
{
	return pkInitParams_;
}

MDM_API double     mdm_DCEModelBase::initialParams(int paramIdx) const
{
  return pkInitParams_[paramIdx];
}
MDM_API double     mdm_DCEModelBase::initialParams(const std::string &paramName) const
{
  for (int i = 0; i < pkParamNames_.size(); i++)
  {
    if (paramName == pkParamNames_[i])
      return pkInitParams_[i];
  }
  return FLT_MIN;
}

MDM_API const std::string& mdm_DCEModelBase::paramName(int paramIdx) const
{
  return pkParamNames_[paramIdx];
}
MDM_API const std::vector<std::string>& mdm_DCEModelBase::paramNames() const
{
  return pkParamNames_;
}

MDM_API const std::vector<bool>& mdm_DCEModelBase::optimisedParamFlags() const
{
  return optParamFlags_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::lowerBounds() const
{
	return lowerBounds_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::upperBounds() const
{
  return upperBounds_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::relativeBounds() const
{
  return relativeBounds_;
}

MDM_API const int mdm_DCEModelBase::repeatParam() const
{
  return repeatParam_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::repeatValues() const
{
  return repeatValues_;
}

MDM_API const mdm_AIF& mdm_DCEModelBase::AIF() const
{
  return AIF_;
}

MDM_API mdm_ErrorTracker::ErrorCode mdm_DCEModelBase::getModelErrorCode() const
{
	return errorCode_;
}

MDM_API std::vector<double> mdm_DCEModelBase::makeLLSMatrix(const std::vector<double>& Ct_sig) const
{
  throw mdm_exception(__func__, boost::format(
    "Model (%1%) does support LLS solving")
    % modelType());
}

MDM_API void mdm_DCEModelBase::transformLLSolution(const double* B)
{
  throw mdm_exception(__func__, boost::format(
    "Model (%1%) does support LLS solving")
    % modelType());
}

MDM_API bool mdm_DCEModelBase::singleFit()
{
  return repeatValues_.empty();
}

MDM_API bool mdm_DCEModelBase::nextRepeatParam()
{
  if (currRpt_ >= repeatValues_.size())
  {
    currRpt_ = 0;
    return false;
  }

  pkInitParams_[repeatParam_] = repeatValues_[currRpt_++];
  reset();
  return true;

}