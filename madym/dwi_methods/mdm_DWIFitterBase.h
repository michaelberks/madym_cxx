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
	*/
	MDM_API mdm_DWIFitterBase();

	//! Default destructor
	MDM_API virtual ~mdm_DWIFitterBase();

	//! Set inputs that vary on per voxel basis
	/*!
	\param sigs vector of signals
	*/
	MDM_API virtual void setInputs(const std::vector<double> &sigs) = 0;

	//! Fit DWI at a single voxel.
	/*!
	All sub-classes must implement this method.

	\param params reference to hold computed DWI model parameters
	*/
	MDM_API virtual mdm_ErrorTracker::ErrorCode fitModel(std::vector<double> &params) = 0;

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

protected:
	//! Heper method to clear up after any fit failures
	/*
	\param msg message to log in program log
	\param params reference to hold default error-fit values when model fails
	*/
	static void setErrorValuesAndTidyUp(std::vector<double>& params);
	
  //! Signals to fit
	std::vector<double> signals_;
	
  //! Maximum number of iterations in optimisation, if 0 runs to convergence
	int maxIterations_;
	alglib::mincgstate state_; //!< Cached ALGLIB internal
	alglib::mincgreport rep_; //!< Cached ALGLIB internal

private:
	
};

#endif /* MDM_DWIFITTERBASE_HDR */
