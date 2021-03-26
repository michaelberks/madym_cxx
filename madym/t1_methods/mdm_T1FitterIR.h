/*!
*  @file    mdm_T1FitterIR.h
*  @brief   Class for estimating T1 (and M0) in a single voxel using inversion recovery method
*  @details 
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_T1FITERRIR_HDR
#define MDM_T1FITERRIR_HDR
#include "mdm_api.h"
#include "mdm_T1FitterBase.h"
#include "mdm_ErrorTracker.h"

//! Class for estimating T1 (and M0) in a single voxel using inversion recovery method
class mdm_T1FitterIR : public mdm_T1FitterBase {

public:
    
	
	//! Constructor from set of inversion times and repetition time
	/*!
	\param TIs vector of inversion recovery times in ms
	\param TR repetition time in ms
	*/
	MDM_API mdm_T1FitterIR(const std::vector<double> &TIs, const double TR);

	//! Default denstructor
	/*!
	*/
	MDM_API ~mdm_T1FitterIR();

	//! Set inversion recovery times
	/*!
	\param TIs vector of inversion recovery times in msecs
	*/
	MDM_API void setTIs(const std::vector<double> &TIs);

	//! Set repetition time
	/*!
	\param TR repetition time
	*/
	MDM_API void setTR(const double TR);

  //! Set inputs that vary on per voxel basis from which T1 will be estimated
  /*!
  If using B1 correction, inputs should be an nIR + 1 element vector, with signals in the first
  nIR elements and the B1 correction at the end. Otherwise an nIR element vector signals.
  \param inputs vector of signals (and B1 correction) from which T1 will be estimated
  */
  MDM_API void setInputs(const std::vector<double> &inputs);

	//! Perform T1 fit using inversion recovery method
	/*!
	\param T1value reference to hold computed T1
	\param M0value reference to hold computed M0
	*/
	MDM_API mdm_ErrorTracker::ErrorCode fitT1(double &T1value, double &M0value);

	//! Set inputs for computing T1 from a single line of an input data stream buffer
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

	//! Compute signal using SPGR equation, given T1, M0, IR and TR
	/*!
	\param T1  in ms
	\param M0 magnetisation constant
	\param IR inversion recovery time in ms
	\param TR repetition time in ms
	\return signal
	*/
	MDM_API static double T1toSignal(
		const double T1, const double M0, const double IR, const double TR);

	
private:

	void computeSignalGradient(const double &T1, const double &M0,
		const double &TI,
		double &signal, double &signal_dT1, double &signal_dM0);

	void computeSSEGradient(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad);

	static void computeSSEGradientAlglib(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad,
		void *context) {
		static_cast<mdm_T1FitterIR*>(context)->computeSSEGradient(
			x, func, grad);
	}

	std::vector<double> TIs_;
  double TR_;
	
};

#endif /* MDM_T1CALC_HDR */
