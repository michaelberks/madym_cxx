//! Abstract base class for implementing DCE tracer-kinetic models
/*!
\file    mdm_DCEModelBase.h

All DCE models should sub-class this abstract class providing an implementation of the following methods:
- #mdm_DCEModelBase::modelType
- #mdm_DCEModelBase::computeCtModel
- #mdm_DCEModelBase::checkParams
						
See git repository wiki for details on configuring models

Original author MA Berks 24 Oct 2018
(c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_DCEMODELBASE_HDR
#define MDM_DCEMODELBASE_HDR

#include "mdm_api.h"

#include <vector>
#include <mdm_AIF.h>
#include <mdm_ErrorTracker.h>

//! Abstract base class for implementing DCE tracer-kinetic models
class mdm_DCEModelBase {
public:
	//! Constructor
	/*!
	\param AIF arterial input function (includes AIF and PIF)
	\param paramNames names of each parameter - used to label output maps (default {})
	\param initialParams initial values for parameters (default {})
	\param fixedParams  indices of parameters not optimised  (default {})
	\param fixedValues  values for fixed parameters (if set, overrides initialParams - default {})
	\param relativeLimitParams  indices of parameters to which relative limits are applied  (default {})
	\param relativeLimitValues  values for relative limits (default {})
	*/
  MDM_API mdm_DCEModelBase(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> relativeLimitParams = std::vector<int>(0),
		const std::vector<double> relativeLimitValues = std::vector<double>(0));

	//! Default destructor
	/*!
	*/
  MDM_API virtual ~mdm_DCEModelBase();

  //! Initialise the model
	/*!
	Sets initial values and fixed status flags for parameters, and computes initial modelled Cm(t)
	\param fixedParams  indices of parameters not optimised  (default {})
	\param fixedValues  values for fixed parameters (if set, overrides initialParams - default {})
	\param relativeLimitParams  indices of parameters to which relative limits are applied  (default {})
	\param relativeLimitValues  values for relative limits (default {})
	*/
	MDM_API virtual void init(
		const std::vector<int> &fixedParams, 
		const std::vector<double> &fixedValues,
		const std::vector<int> &relativeLimitParams, 
		const std::vector<double> &relativeLimitValues);

  //! Resets models
	/*! Clears optimised values and fixed status flags for parameters, clears modelled
	concentration time-series and resets to specified length
	
	\param nTimes number of time-points in reset Cm(t)
	*/
	MDM_API virtual void reset(size_t nTimes);

  //! Return the number of model parameters
	/*!	
	Includes all parameters in the model definition, even those fixed by default
	\return number of model parameters
	*/
	MDM_API virtual int numParams() const;

  //! Return the number of model parameters free to be optimised
	/*!
	This will be the total number of parameters, minus those that are currently fixed	
	\return number of model parameter currently being optimised
	\see num_params
	\see num_fixed
	*/
	MDM_API virtual int numOptimised() const;

  //! Return the number of fixed model parameters
	/*!
	\return number of model parameters currently fixed
	\see num_params
	\see num_optimised
	*/
	MDM_API virtual int numFixed() const;

  //! Return the modelled concentration time-series Cm(t)
	/*!
	\return modelled concentration time-series Cm(t)
	*/
	MDM_API virtual const std::vector<double>& CtModel();

  //! Return the current values of optimised parameters
	/*!
	\return current values of optimised parameters
	*/
	MDM_API virtual std::vector<double>& optimisedParams();

  //! Set the initial value of parameters
	/*!
	\param params initial value for each parameter, length must equal numParams()
	*/
	MDM_API virtual void setInitialParams(const std::vector<double>& params);

	//! Set the value of all optimised parameters
	/*!
	\param params value of each optimised parameter, length must equal numOptimised()
	*/
	MDM_API virtual void setOptimisedParams(const std::vector<double>& params);

  //! Set all model parameters to zero
	/*!
	*/
	MDM_API virtual void zeroParams();

  //! Return the lower bounds for parameters being optimised
	/*!
	\return lower bounds for parameters being optimised, length will be numOptimised()
	*/
	MDM_API virtual const std::vector<double>& optimisedLowerBounds();

	//! Return the upper bounds for parameters being optimised
	/*!
	\return upper bounds for parameters being optimised, length will be numOptimised()
	*/
	MDM_API virtual const std::vector<double>& optimisedUpperBounds();

	//! Return the value of all model parameters
	/*!
	\return value of all model parameters, length will be numParams()
	*/
	MDM_API const std::vector<double>&     params() const;
  	
	//! Return the value of specific model parameter by index
	/*!
	\param paramIdx index (starting from 0), of parameter to return. Must be <= 0 and < numParams().
	\return value of parameter at index paramIdx
	*/
	MDM_API double     params(int paramIdx) const;
  	
	//! Return the value of specific model parameter by name
	/*!
	\param paramName name of parameter to return, must be a member of paramNames()
	\return value of parameter matching name paramName
	*/
	MDM_API double     params(const std::string &paramName) const;

	//! Return the initial value of all model parameters
	/*!
	\return initial value of all model parameters, length will be numParams()
	*/
	MDM_API const std::vector<double>&     initialParams() const;
  
	//! Return the initial value of specific model parameter by index
	/*!
	\param paramIdx index (starting from 0), of parameter to return. Must be <= 0 and < numParams().
	\return initial value of parameter at index paramIdx
	*/
	MDM_API double     initialParams(int paramIdx) const;
  
	//! Return the initial value of specific model parameter by name
	/*!
	\param paramName name of parameter to return, must be a member of paramNames()
	\return initial value of parameter matching name paramName
	*/
	MDM_API double     initialParams(const std::string &paramName) const;

	//! Return the name of model parameter by index
	/*!
	\param paramIdx index (starting from 0), of parameter to return. Must be <= 0 and < numParams().
	\return initial name of parameter at index paramIdx
	*/
	MDM_API const std::string&     paramName(int paramIdx) const;
  
	//! Return all parameter names
	/*!
	\return names of parameters
	*/
	MDM_API const std::vector<std::string>&     paramNames() const;

	//! Return flags specifying which parameters free to be are optimised
	/*!
	\return flags specifying which parameters free to be are optimised (true) vs fixed (false). 
	Will be length numParams().
	*/
	MDM_API const std::vector<bool>&     optimisedParamFlags() const;

	//! Return the relative bounds (if any) for each parameter
	/*!
	\return relative bounds for each parameter.
	*/
	MDM_API const std::vector<double>&     relativeBounds() const;

	//! Return the relative AIF object used by the model
	/*!
	\return AIF object used by the model
	*/
	MDM_API const mdm_AIF&     AIF() const;

  //! Return any errors in fitted parameters.
	/*!
	\return model error code
	\see mdm_ErrorTracker
	*/
	MDM_API virtual mdm_ErrorTracker::ErrorCode getModelErrorCode() const;

	//! Return model sub-class name
	/*!
	Pure virtual function, must be implemented by sub-classes.
	\return model sub-class name
	*/
	MDM_API virtual std::string modelType() const = 0;

  //! Compute modelled concentration time-series Cm(t) with current model parameters.
	/*!
	Pure virtual function, must be implemented by sub-classes.
	\param nTimes number of time-points, starting from 0 at which to compute Cm(t). Must be < AIF.nTimes()
	\return
	*/
	MDM_API virtual void computeCtModel(size_t nTimes) = 0;

  //! Check the validity of fitted model prameters.
	/*!
	Pure virtual function, must be implemented by sub-classes. If any parameter is not valid,
	getModelErrorCode() will return the associated error code.
	\see getModelErrorCode
	*/
	MDM_API virtual void checkParams() = 0;

protected:

  //VARIABLES USED BY ALL BASE CLASSES
  std::vector<double> CtModel_; //!< Fitted concentration time-series using model parameters
  std::vector<double> pkParams_; //!< All model parameters
  std::vector<double> pkParamsOpt_; //!< Parameters we're actually optimising
  std::vector<double> pkInitParams_; //!< Initial values
  std::vector<bool> optParamFlags_; //!< Flags specifying variables that are fixed (flag=0) or to be optimised (flag=1)
  std::vector<std::string>	pkParamNames_; //!< Names of the parameters
                                          
  //Upper and lower bounds to use with optimiser
  std::vector<double> lowerBounds_; //!< Lower bounds for all parameters
  std::vector<double> upperBounds_; //!< Upper bounds for all parameters
	std::vector<double> relativeBounds_; //!< Relative bounds for optimised parameters
  std::vector<double> lowerBoundsOpt_; //!< Lower bounds for optimised parameters 
  std::vector<double> upperBoundsOpt_; //!< Upper bounds for optimised parameters 

  //! Reference to AIF object set at initialization from global volume analysis
  mdm_AIF &AIF_;

	//! Error code of the model after fitting
	mdm_ErrorTracker::ErrorCode errorCode_;

private:
  //METHODS:
  

  
};

#endif //MDM_DCEMODELBASE_HDR
