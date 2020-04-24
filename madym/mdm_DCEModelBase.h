/**
*  @file    mdm_DCEModelBase.h
*  @brief   Abstract base class for implementing DCE tracer-kinetic models
*  @details All DCE models should sub-class this abstract class providing an
*						an implementation of the following methods:
*						- modelType
*           - computeCtModel
*						- checkParams
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_DCEMODELBASE_HDR
#define MDM_DCEMODELBASE_HDR

#include "mdm_api.h"

#include <vector>
#include <mdm_AIF.h>
#include <mdm_version.h>
#include <mdm_ErrorTracker.h>

class mdm_DCEModelBase {
public:
	/**
	* @brief
 Constructor
	*	@param pkParamNames names of each parameter - used to label output maps (default {})
	* @param pkInitParams initial values for parameters (default {})
	* @param fixedParams  indices of parameters not optimised  (default {})
	* @param fixedValues  values for fixed parameters (if set, overrides initParams - default {})
	*/
  MDM_API mdm_DCEModelBase(
    mdm_AIF &AIF,
    const std::vector<std::string> &pkParamNames = std::vector<std::string>(0),
    const std::vector<double> &pkInitParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> relativeLimitParams = std::vector<int>(0),
		const std::vector<double> relativeLimitValues = std::vector<double>(0));

	/**
	* @brief

	* @param
	* @return
	*/
  MDM_API ~mdm_DCEModelBase();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void init(
		const std::vector<int> &fixedParams, 
		const std::vector<double> &fixedValues,
		const std::vector<int> &relativeLimitParams, 
		const std::vector<double> &relativeLimitValues);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void reset(int nTimes);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual int num_dims() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual int num_opt() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual int num_fixed() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual const std::vector<double>& CtModel();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual std::vector<double>& optParams();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void setPkParams(const std::vector<double>& params);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void setPkInitParams(const std::vector<double>& params);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void zeroParams();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual const std::vector<double>& lowerBoundsOpt();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual const std::vector<double>& upperBoundsOpt();

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<double>&     pkParams() const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     pkParams(int paramIdx) const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     pkParams(const std::string &paramName) const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<double>&     pkInitParams() const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     pkInitParams(int paramIdx) const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     pkInitParams(const std::string &paramName) const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::string&     pkParamName(int paramIdx) const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<std::string>&     pkParamNames() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<bool>&     optParamFlags() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const mdm_AIF&     AIF() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual int getModelErrorCode();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual std::string modelType() const = 0;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void computeCtModel(int nTimes) = 0;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual double checkParams() = 0;

protected:
  //VARIABLES USED BY ALL BASE CLASSES
  //We may set these to be private with appropriate getters/setters later
  std::vector<double>						CtModel_;			//DCE time series vector of fitted concentrations using model parameters
  std::vector<double>			pkParams_;			//Array of all model parameters
  std::vector<double>			pkParamsOpt_;		//Array parameters we're actually optimising
  std::vector<double>			pkInitParams_;	//Array of initial values
  std::vector<bool>				optParamFlags_; //Array of flags specifying variables that are fixed (flag=0) or to be optimised (flag=1)
  std::vector<std::string>	pkParamNames_;	//Names of the parameters - must be in same order as pkParams
                                          
  //Upper an lower bounds to use with optimiser
  std::vector<double> lowerBounds_;
  std::vector<double> upperBounds_;
	std::vector<double> relativeBounds_;
  std::vector<double> lowerBoundsOpt_;
  std::vector<double> upperBoundsOpt_;

  //Reference to AIF object set at initialization from global volume analysis
  mdm_AIF &AIF_;

	//Error code of the model
	int errorCode_;

	const double BAD_FIT_SSD;

private:
  //METHODS:
  

  
};

#endif //MDM_DCEMODELBASE_HDR
