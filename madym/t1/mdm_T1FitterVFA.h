/*!
*  @file    mdm_T1FitterVFA.h
*  @brief   Class for estimating T1 (and M0) in a single voxel using variable fli-angle method
*  @details 
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_T1FITTERVFA_HDR
#define MDM_T1FITTERVFA_HDR
#include "mdm_api.h"
#include "mdm_T1FitterBase.h"
#include "mdm_ErrorTracker.h"

//! Class for estimating T1 (and M0) in a single voxel using VFA method
class mdm_T1FitterVFA : public mdm_T1FitterBase {

public:
    
	
	//! Constructor from set of FAs and repetition time
	/*!
	\param FAs vector of variable flip-angles in radians
	\param TR repetition time in ms
  \param usingB1 flag if using B1 correction
	*/
	MDM_API mdm_T1FitterVFA(const std::vector<double> &FAs, const double TR, const bool usingB1);

	//! Default denstructor
	/*!
	*/
	MDM_API ~mdm_T1FitterVFA();

	//! Set variable flip angles
	/*!
	\param FAs vector of flip-angles in radians
	*/
	MDM_API void setFAs(const std::vector<double> &FAs);

	//! Set repetition time
	/*!
	\param TR repetition time
	*/
	MDM_API void setTR(const double TR);

  //! Set B1
  /*!
  \param B1 B1 value
  */
  MDM_API void setB1(const double B1);

  //! Set inputs that vary on per voxel basis from which T1 will be estimated
  /*!
  If using B1 correction, inputs should be an nFA + 1 element vector, with signals in the first
  nFA elements and the B1 correction at the end. Otherwise an nFA element vector signals.
  \param inputs vector of signals (and B1 correction) from which T1 will be estimated
  */
  MDM_API void setInputs(const std::vector<double> &inputs);

	//! Perform T1 fit using variable flip-angle method
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

	//! Compute signal using SPGR equation, given T1, M0, FA and TR
	/*!
	\param T1  in ms
	\param M0 magnetisation constant
	\param FA flip-angle in radians
	\param TR repetition time in ms
	\return signal
	*/
	MDM_API static double T1toSignal(
		const double T1, const double M0, const double FA, const double TR);

	
private:

	void computeSignalGradient(const double &T1, const double &M0,
		const double &cosFA, const double &sinFA,
		double &signal, double &signal_dT1, double &signal_dM0);

	void computeSSEGradient(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad);

	static void computeSSEGradientAlglib(
		const alglib::real_1d_array &x, double &func, alglib::real_1d_array &grad,
		void *context) {
		static_cast<mdm_T1FitterVFA*>(context)->computeSSEGradient(
			x, func, grad);
	}

	void initFAs();

	std::vector<double> FAs_;
  double TR_;
	double B1_;
  bool usingB1_;

	//Convenient to cache these when FAs set
	int nFAs_;
	std::vector<double> cosFAs_;
	std::vector<double> sinFAs_;

	static const double PI;
	
};

#endif /* MDM_T1CALC_HDR */
