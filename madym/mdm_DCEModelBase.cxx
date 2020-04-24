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
#include "mdm_platform_defs.h"
#include <math.h>

MDM_API mdm_DCEModelBase::mdm_DCEModelBase(
  mdm_AIF &AIF,
  const std::vector<std::string> &pkParamNames,
  const std::vector<double> &pkInitParams,
  const std::vector<int> &fixedParams,
  const std::vector<double> &fixedValues,
	const std::vector<int> relativeLimitParams,
	const std::vector<double> relativeLimitValues)
  :CtModel_(0),
  AIF_(AIF),
  pkParams_(0),
  pkParamsOpt_(0),
  pkParamNames_(pkParamNames),
  pkInitParams_(pkInitParams),
	errorCode_(mdm_ErrorTracker::OK),
	BAD_FIT_SSD(DBL_MAX)
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

	//Set relative limits for any specified parameters
	relativeBounds_.resize(num_dims(), 0);
	for (int i = 0; i < relativeLimitParams.size(); i++)
	{
		int rp = relativeLimitParams[i] - 1;
		if (rp < num_dims())
		{
			//If fixed values are supplied, use these to overwrite default
			//initial parameters
			if (i < relativeLimitValues.size())
				relativeBounds_[rp] = relativeLimitValues[i];
		}
	}
    
  //Set upper and lower bounds for parameters to be optimised
  for (int i = 0; i < num_dims(); i++)
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

MDM_API void mdm_DCEModelBase::reset(int nTimes)
{
  pkParams_ = pkInitParams_;
  pkParamsOpt_.clear();
  for (int i = 0, j = 0; i < num_dims(); i++)
  {
    if (optParamFlags_[i])
    {
      pkParamsOpt_.push_back(pkParams_[i]);
      //simplexLambdaOpt_.push_back(simplexLambda_[i]);
    }
  }
  CtModel_.resize(nTimes);
}

MDM_API int mdm_DCEModelBase::num_dims() const
{
  return pkInitParams_.size();
}

MDM_API int mdm_DCEModelBase::num_opt() const
{
  return pkParamsOpt_.size();  
}

MDM_API int mdm_DCEModelBase::num_fixed() const
{
  return num_dims() - num_opt();
}

MDM_API const std::vector<double>& mdm_DCEModelBase::CtModel()
{
  return CtModel_;
}

MDM_API std::vector<double>& mdm_DCEModelBase::optParams()
{
  return pkParamsOpt_;
}

MDM_API void mdm_DCEModelBase::setPkParams(const std::vector<double>& optParams)
{
  for (int i = 0, j = 0, n = num_dims(); i < n; i++)
  {
    if (optParamFlags_[i])
    {
      pkParams_[i] = optParams[j];
      pkParamsOpt_[j] = optParams[j];
      j++;
    }
  }
}

MDM_API void mdm_DCEModelBase::setPkInitParams(const std::vector<double>& params)
{
  pkInitParams_ = params;
	
	//See if we have any relative bounds to update
	for (int i = 0, n = num_dims(), j = 0; i < n; i++)
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
  for (int i = 0, n = num_dims(); i < n; i++)
    pkParams_[i] = 0;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::lowerBoundsOpt()
{
  return lowerBoundsOpt_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::upperBoundsOpt()
{
  return upperBoundsOpt_;
}

MDM_API const std::vector<double>& mdm_DCEModelBase::pkParams() const
{
	return pkParams_;
}

MDM_API double     mdm_DCEModelBase::pkParams(int paramIdx) const
{
  return pkParams_[paramIdx];
}
MDM_API double     mdm_DCEModelBase::pkParams(const std::string &paramName) const
{
  for (int i = 0; i < pkParamNames_.size(); i++)
  {
    if (paramName == pkParamNames_[i])
      return pkParams_[i];
  }
  return FLT_MIN;
}

MDM_API const std::vector<double>&     mdm_DCEModelBase::pkInitParams() const
{
	return pkInitParams_;
}

MDM_API double     mdm_DCEModelBase::pkInitParams(int paramIdx) const
{
  return pkInitParams_[paramIdx];
}
MDM_API double     mdm_DCEModelBase::pkInitParams(const std::string &paramName) const
{
  for (int i = 0; i < pkParamNames_.size(); i++)
  {
    if (paramName == pkParamNames_[i])
      return pkInitParams_[i];
  }
  return FLT_MIN;
}

MDM_API const std::string&     mdm_DCEModelBase::pkParamName(int paramIdx) const
{
  return pkParamNames_[paramIdx];
}
MDM_API const std::vector<std::string>&     mdm_DCEModelBase::pkParamNames() const
{
  return pkParamNames_;
}

MDM_API const std::vector<bool>&     mdm_DCEModelBase::optParamFlags() const
{
  return optParamFlags_;
}

MDM_API const mdm_AIF&     mdm_DCEModelBase::AIF() const
{
  return AIF_;
}

MDM_API int mdm_DCEModelBase::getModelErrorCode()
{
	return errorCode_;
}