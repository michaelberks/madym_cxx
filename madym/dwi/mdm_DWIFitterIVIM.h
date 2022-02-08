/*!
*  @file    mdm_DWIFitterVFA.h
*  @brief   Class for fitting IVIM to DWI data
*  @details 
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_DWIFITTERIVIM_HDR
#define MDM_DWIFITTERIVIM_HDR
#include <madym/utils/mdm_api.h>
#include "mdm_DWIFitterBase.h"
#include <madym/utils/mdm_ErrorTracker.h>
#include "mdm_DWIFitterADC.h"

struct bcfitOutput
{
	std::vector<double> fitted_params;
	std::vector<double> residuals;
	double ssr;
	int nvarys;
	int ndata;
	double aic;
	double aicc;
	double bic;
	double rsq;

	mdm_ErrorTracker::ErrorCode success;
};



//! Class for fitting IVIM to DWI data
class mdm_DWIFitterIVIM : public mdm_DWIFitterBase {

public:
    
	
	//! Constructor from set of FAs and repetition time
	/*!
	\param Bvals vector of B0 values sin msecs
	*/
	MDM_API mdm_DWIFitterIVIM(const std::vector<double> &Bvals, bool fullModel, const std::vector<double> &Bvalsthresh);

	//! Default denstructor
	/*!
	*/
	MDM_API ~mdm_DWIFitterIVIM();

	//! Perform IVIM fit
	/*!
	\param params ...
	\param ssr ...
	*/
	MDM_API mdm_ErrorTracker::ErrorCode fitModel(std::vector<double>&params, double& ssr);

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
	MDM_API static double modelToSignal(
		const std::vector<double>& params, const double B0);

	//! Compute signals using...
	/*!
	\params IVIM model parameters
	\param B0s B0 values in msecs
	\return signal
	*/
	MDM_API static std::vector<double> modelToSignals(
		const std::vector<double>& params, const std::vector<double> B0s);

	
private:

	void bcfitIVIM(
		const std::vector<double>& initParams,
		bcfitOutput& fit);

	bcfitOutput fitMultipleThresholds();

	void computeSignalGradient(
		const double& s0, const double& d, const double& f, const double& dstar,
		const double &B,
		double &signal, 
		double& ds0, double& dd, double& df, double& ddstar);

	void computeSSEGradient(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad);

	static void computeSSEGradientAlglib(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad,
		void *context) {
		static_cast<mdm_DWIFitterIVIM*>(context)->computeSSEGradient(
			x, func, grad);
	}

	//Variables

	//!Thresholds on B-value to use in fitting
	std::vector<double> BValsThresh_;

	bool fullModel_;

	//ADC fitter
	mdm_DWIFitterADC ADCFitter_;

	
	
};

#endif /* MDM_DWIFITTERIVIM_HDR */
