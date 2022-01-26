/*!
*  @file    mdm_DWIFitterVFA.h
*  @brief   Class for fitting ADC to DWI data
*  @details
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_DWIFITTERADC_HDR
#define MDM_DWIFITTERADC_HDR
#include "mdm_api.h"
#include "mdm_DWIFitterBase.h"
#include "mdm_ErrorTracker.h"

//! Class for fitting ADC to DWI data
class mdm_DWIFitterADC : public mdm_DWIFitterBase {

public:


	//! Constructor from set of FAs and repetition time
	/*!
	\param B0s vector of B0 values sin msecs
	*/
	MDM_API mdm_DWIFitterADC(const std::vector<double>& B0s, bool linearFit);

	//! Default denstructor
	/*!
	*/
	MDM_API ~mdm_DWIFitterADC();

	//! Perform ADC fit
	/*!
	\param params ...
	\param ssr ...
	*/
	MDM_API mdm_ErrorTracker::ErrorCode fitModel(std::vector<double>& params, double& ssr);

	//! Set inputs for fitting ADC to a single line of an input data stream buffer
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
	\params ADC model parameters
	\param B0 B0 values in msecs
	\return signal
	*/
	MDM_API static double modelToSignal(
		const std::vector<double>& params, const double B0);

	//! Compute signals using...
	/*!
	\params ADC model parameters
	\param B0s B0 values in msecs
	\return signal
	*/
	MDM_API static const std::vector<double> modelToSignals(
		const std::vector<double>& params, const std::vector<double> B0s);


private:

	void computeSignalGradient(
		const double& S0, const double& ADC, const double &Bval,
		double& signal, double& signal_dS0, double& signal_dADC);

	void computeSSEGradient(
		const alglib::real_1d_array& x, double& func, alglib::real_1d_array& grad);

	static void computeSSEGradientAlglib(
		const alglib::real_1d_array& x, double& func, alglib::real_1d_array& grad,
		void* context) {
		static_cast<mdm_DWIFitterADC*>(context)->computeSSEGradient(
			x, func, grad);
	}

	void linearFit(double &S0, double &ADC, double& ssr);

	bool linearFit_;

};

#endif /* MDM_DWIFITTERADC_HDR */
