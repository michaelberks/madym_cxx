/*!
*  @file    mdm_T1FitterBase.h
//!   Abstract base class for estimating T1 (and M0) in a single voxel
*  @details Currently only variable flip angle method supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_T1VOXEL_HDR
#define MDM_T1VOXEL_HDR

#include "mdm_api.h"
#include "mdm_ErrorTracker.h"
#include "opt/optimization.h"

#include <vector>
#include <iostream>

//!   Abstract base class for estimating T1 (and M0) in a single voxel
/*!
*  Currently only variable flip angle method supported
*/
class mdm_T1FitterBase {

public:

	//! Default constructor
	/*!
	Pre-conditions alglib optimiser
	*/
	MDM_API mdm_T1FitterBase();

	//! Default destructor
	MDM_API virtual ~mdm_T1FitterBase();

	//! Set signals from which T1 will be estimated
	/*!
	\param sigs vector of signals from which T1 will be estimated
	*/
	MDM_API void setInputSignals(const std::vector<double> &sigs);

  //! Fit T1 at a single voxel.
	/*!
	All sub-classes must implement this method.

	\param T1value reference to hold computed T1
	\param M0value reference to hold computed M0 
	*/
	MDM_API virtual mdm_ErrorTracker::ErrorCode fitT1(double &T1value, double &M0value) = 0;

	//! Fit T1 for a single line of an input data stream buffer
	/*!
	All sub-classes must implement this method.

	\param ifs input data stream
	\param nSignals number of signals in sample
	\param T1value reference to hold computed T1
	\param M0value reference to hold computed M0
	\param eof reference to flag, set true if streams EOF flag is reached
	*/
	MDM_API virtual mdm_ErrorTracker::ErrorCode fitT1(std::istream& ifs, 
		const int nSignals, double &T1value, double &M0value, bool &eof) = 0;

	//! Set any fixed scanner settings required to estimate T1
	/*!
	All sub-classes must implement this method.

	Different T1 estimation methods may require knowledge of different scanner settings.
	This abstract method takes a vector<double> as input. It is up to the derived sub-class
	implementations to define how they extract data from this container.

	\param settings vector of fixed scanner settings (eg TR for VFA)
	*/
	MDM_API virtual void setFixedScannerSettings(const std::vector<double> &settings) = 0;

	//! Set any variable scanner settings required to estimate T1
	/*!
	All sub-classes must implement this method.

	Different T1 estimation methods may require knowledge of different scanner settings.
	This abstract method takes a vector<double> as input. It is up to the derived sub-class
	implementations to define how they extract data from this container.

	\param settings vector of variable scanner settings (eg FAs for VFA)
	*/
	MDM_API virtual void setVariableScannerSettings(const std::vector<double> &settings) = 0;

	//! Return minimum inputs required, must be implemented by derived subclass
	/*
	\return minimum number of input signals required for T1 fitting method
	*/
	MDM_API virtual int minimumInputs() const = 0;

	//! Return maximum inputs allowed, must be implemented by derived subclass
	/*
	\return maximum number of input signals allowed in T1 fitting method
	*/
	MDM_API virtual int maximumInputs() const = 0;

protected:
	//! Heper method to clear up after any fit failures
	/*
	\param msg message to log in program log
	\param T1value reference to hold default error-fit value for T1
	\param M0value reference to hold default error-fit value for M0
	*/
	static void setErrorValuesAndTidyUp(const std::string msg, double &T1, double &M0);
	
	std::vector<double> signals_;
	
	int maxIterations_;
	alglib::mincgstate state_;
	alglib::mincgreport rep_;

private:
	
};

#endif /* MDM_T1CALC_HDR */
