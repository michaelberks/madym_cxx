/*!
*  @file    mdm_T1FitterBase.h
//!   Abstract base class for estimating T1 (and M0) in a single voxel
*  @details Currently variable flip angle and inversion recovery methods supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_DWIFITTERBASE_HDR
#define MDM_DWIFITTERBASE_HDR

#include "mdm_api.h"
#include "mdm_ErrorTracker.h"
#include "opt/optimization.h"

#include <vector>
#include <iostream>

//!   Abstract base class for fitting models to DWI data
/*!
*  Currently only variable flip angle method supported
*/
class mdm_DWIFitterBase {

public:

	//! Default constructor
	/*!
	Pre-conditions alglib optimiser
	\param Bvals vector of B0 values sin msecs
	\param paramNames name sof parameters in instantiated sub-class
	*/
	MDM_API mdm_DWIFitterBase(
		const std::vector<double>& Bvals, const std::vector<std::string> &paramNames);

	//! Default destructor
	MDM_API virtual ~mdm_DWIFitterBase();

	//! Set B values
	/*!
	\param B-values in msecs
	*/
	MDM_API virtual void setBvals(const std::vector<double>& Bvals);


	//! Set signals matching B-values
	/*!
	* \param 
	*/
	MDM_API virtual void setSignals(const std::vector<double>& sigs);

	//! Set B values
	/*!
	\param B-values in msecs that we're going to fit (as subset of Bvals_)
	*/
	MDM_API virtual void setBvalsToFit(const std::vector<double>& Bvals);


	//! Set signals matching B-values to fit
	/*!
	* \param signals set of signals matching BvalsToFit_
	*/
	MDM_API virtual void setSignalsToFit(const std::vector<double>& sigs);

	//! Fit DWI at a single voxel.
	/*!
	All sub-classes must implement this method.

	\param params reference to hold computed DWI model parameters
	\param ssr sum-of-squared residuals for model fit
	*/
	MDM_API virtual mdm_ErrorTracker::ErrorCode fitModel(std::vector<double> &params, double &ssr) = 0;

	//! Set inputs for fitting DWI model from a single line of an input data stream buffer
	/*!
	All sub-classes must implement this method.

	\param ifs input data stream
	\param nSignals number of signals in sample
	\return false if streams EOF flag is reached, true otherwise
	*/
	MDM_API virtual bool setInputsFromStream(std::istream& ifs, 
		const int nSignals) = 0;

	//! Return minimum inputs required, must be implemented by derived subclass
	/*
	\return minimum number of input signals required
	*/
	MDM_API virtual int minimumInputs() const = 0;

	//! Return maximum inputs allowed, must be implemented by derived subclass
	/*
	\return maximum number of input signals allowed
	*/
	MDM_API virtual int maximumInputs() const = 0;

	//! Return parameter names
	/*
	\return name sof model parameters
	*/
	MDM_API virtual const std::vector<std::string> paramNames() const;

	//! Return number of parameters in model
	/*
	\return number of parameters in model
	*/
	MDM_API virtual size_t nParams() const;

protected:
	//! Heper method to clear up after any fit failures
	/*
	\param msg message to log in program log
	\param params reference to hold default error-fit values when model fails
	*/
	static void setErrorValuesAndTidyUp(std::vector<double>& params);
	
  //! Signals
	std::vector<double> signals_;

	//!B-values
	std::vector<double> Bvals_;

	//! Signals to fit - we sometimes want this to be a subset of all signals
	std::vector<double> signals_to_fit_;

	//!B-values - we sometimes want this to be a subset of all B-values
	std::vector<double> Bvals_to_fit_;

	//! Parameter names
	const std::vector<std::string> paramNames_;
	
  //! Maximum number of iterations in optimisation, if 0 runs to convergence
	int maxIterations_;
	
	alglib::minbcstate   state_; //!< Cached ALGLIB internal
	alglib::minbcreport rep_; //!< Cached ALGLIB internal

private:
	
};

#endif /* MDM_DWIFITTERBASE_HDR */
