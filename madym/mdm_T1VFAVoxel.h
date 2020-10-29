/*!
*  @file    mdm_T1Voxel.h
*  @brief   Class for estimating T1 (and M0) in a single voxel
*  @details Currently only variable flip angle method supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_T1VFAVOXEL_HDR
#define MDM_T1VFAVOXEL_HDR
#include "mdm_api.h"
#include "mdm_T1Voxel.h"
#include "mdm_ErrorTracker.h"

//! Class for estimating T1 (and M0) in a single voxel using VFA method
class mdm_T1VFAVoxel : public mdm_T1Voxel {

public:
    
	
	//! Constructor from set of FAs and repetition time
	/*!
	\param FAs vector of variable flip-angles in radians
	\param TR repetition time in ms
	*/
	MDM_API mdm_T1VFAVoxel(const std::vector<double> &FAs, const double TR);

	//! Default constructor
	/*!
	*/
	MDM_API mdm_T1VFAVoxel();

	//! Default denstructor
	/*!
	*/
	MDM_API ~mdm_T1VFAVoxel();

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

	//! Perform T1 fit using variable flip-angle method
	/*!
	\param T1value reference to hold computed T1
	\param M0value reference to hold computed M0
	*/
	MDM_API mdm_ErrorTracker::ErrorCode fitT1(double &T1value, double &M0value);

	//! Fit T1 for a single line of an input data stream buffer
	/*!
	\param ifs input data stream
	\param nSignals number of signals in sample
	\param T1value reference to hold computed T1
	\param M0value reference to hold computed M0
	*/
	MDM_API mdm_ErrorTracker::ErrorCode fitT1(std::istream& ifs, 
		const int nSignals, double &T1value, double &M0value, bool &eof);

	//! Set any fixed scanner settings required to estimate T1
	/*!
	VFA requires knowing TR. This should be passed in as the only element of settings.
	\param settings vector of length 1, with TR (in ms) as the single element.
	*/
	MDM_API void setFixedScannerSettings(const std::vector<double> &settings);

	//! Set any variable scanner settings required to estimate T1
	/*!
	This is used to pass in the variable flip-angles, which should be a single
	vector of length nSignals.
	*/
	MDM_API void setVariableScannerSettings(const std::vector<double> &settings);

	//! Compute signal using SPGR equation, given T1, M0, FA and TR
	/*!
	\param T1  in ms
	\param M0 magnetisation constant
	\param FA flip-angle in radians
	\param repetition time in ms
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
		static_cast<mdm_T1VFAVoxel*>(context)->computeSSEGradient(
			x, func, grad);
	}

	void initFAs();

	std::vector<double> FAs_;
	double TR_;
	double delta_;

	//Convenient to cache these when FAs set
	int nFAs_;
	std::vector<double> cosFAs_;
	std::vector<double> sinFAs_;

	static const double PI;
	
};

#endif /* MDM_T1CALC_HDR */
