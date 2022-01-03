/*!
*  @file    mdm_DWIFitterVFA.h
*  @brief   Class for fitting IVIM to DWI data
*  @details 
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_DWIFITTERIVIM_HDR
#define MDM_DWIFITTERIVIM_HDR
#include "mdm_api.h"
#include "mdm_DWIFitterBase.h"
#include "mdm_ErrorTracker.h"

//! Class for fitting IVIM to DWI data
class mdm_DWIFitterIVIM : public mdm_DWIFitterBase {

public:
    
	
	//! Constructor from set of FAs and repetition time
	/*!
	\param B0s vector of B0 values sin msecs
	*/
	MDM_API mdm_DWIFitterIVIM(const std::vector<double> &B0s);

	//! Default denstructor
	/*!
	*/
	MDM_API ~mdm_DWIFitterIVIM();

	//! Set variable flip angles
	/*!
	\param FAs vector of flip-angles in radians
	*/
	MDM_API void setB0s(const std::vector<double> &B0s);


	//! Set inputs that vary on per voxel basis
	/*!
	*/
	MDM_API void setInputs(const std::vector<double> &inputs);

	//! Perform T1 fit using variable flip-angle method
	/*!
	\param T1value reference to hold computed T1
	\param M0value reference to hold computed M0
	*/
	MDM_API mdm_ErrorTracker::ErrorCode fitModel(std::vector<double>&params);

	//! Set inputs for fitting IVIM to a single line of an input data stream buffer
	/*!
	All sub-classes must implement this method.

	\param ifs input data stream
	\param nSignals number of signals in sample
	\return false if streams EOF flag is reached, true otherwise
	*/
	MDM_API virtual bool setInputsFromStream(std::istream& ifs,
		const int nSignals);

	//! Return minimum inputs required, must be implemented by derived subclass
	/*
	\return minimum number of input signals required for T1 fitting method
	*/
	MDM_API int minimumInputs() const;

	//! Return maximum inputs allowed, must be implemented by derived subclass
	/*
	\return maximum number of input signals allowed in T1 fitting method
	*/
	MDM_API int maximumInputs() const;

	//! Compute signal using...
	/*!
	\params IVIM model parameters
	\param B0 B0 values in msecs
	\return signal
	*/
	MDM_API static double modeltoSignal(
		const std::vector<double>& params, const double B0);

	
private:

	void computeSignalGradient(const std::vector<double>& params,
		double &signal, double &signal_dT1, double &signal_dM0);

	void computeSSEGradient(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad);

	static void computeSSEGradientAlglib(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad,
		void *context) {
		static_cast<mdm_DWIFitterIVIM*>(context)->computeSSEGradient(
			x, func, grad);
	}

	void initB0s();

	std::vector<double> B0s_;
	size_t nB0s_;

	static const double PI;
	
};

#endif /* MDM_DWIFITTERIVIM_HDR */
