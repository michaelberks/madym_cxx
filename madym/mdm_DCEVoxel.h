/*!
 *  @file    mdm_DCEVoxel.h
 *  @brief   Class that holds DCE time-series data and an asssociated tracer kinetic model
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_DCEVOXEL_HDR
#define MDM_DCEVOXEL_HDR
#include <madym/mdm_api.h>

#include <madym/dce_models/mdm_DCEModelBase.h>

//! Holds DCE time-series data and an asssociated tracer kinetic model
/*!
*/
class mdm_DCEVoxel {
public:

	//! Enum of current voxel error status
	/*!
	*/
  enum mdm_DCEVoxelStatus
  {
    OK = 0, ///> No errors
    DYN_T1_BAD = 1, ///> Dynamic T1 invalid at one or more timepoints
    CA_NAN = 2, ///> NaNs found in signal-derived concentration
    T10_BAD = 3, ///> Baseline T1 is invalid
    M0_BAD = 4, ///> Baseline M0 is invalid
    NON_ENHANCING = 5 ///> No CA uptake
  };

	
	//! Constructor
	/*!
	\param dynSignals time-series of dynamic signals (if empty, requires dynConc)
	\param dynConc time-series of signal-derived concentration (if empty, computed from dynSignals)
	\param injectionImg timepoint bolus injected
	\param dynamicTimings time in minutes of each series timepoint
	\param IAUC_times times at which compute IAUC
	*/
	MDM_API mdm_DCEVoxel(
		const std::vector<double> &dynSignals,
		const std::vector<double> &dynConc,
    const size_t injectionImg,
		const std::vector<double> &dynamicTimings,
		const std::vector<double> &IAUC_times);

	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_DCEVoxel();

  //! Convert signal time-series to contrast agent concentration
	/*!
  \param T1 baseline T1 
  \param FA flip-angle in degrees
  \param TR repetition in ms
  \param r1Const relaxivity constant of contrast-agent
  \param M0 baseline magnetisation constant
  \param timepoint0 first time-point to use in pre-bolus noise estimation (default 0)
	*/
	MDM_API void computeCtFromSignal(
    const double T1, const double FA, const double TR, const double r1Const,
    const double M0, size_t timepoint0 = 0);

	//! Compute IAUC values at selected times
	/*!
	*/
	MDM_API void computeIAUC();

  //! Return the current error status
	/*!
	\see mdm_DCEVoxelStatus
	*/
	MDM_API mdm_DCEVoxelStatus status() const;
	
	//! Return signal time-series
	/*!
	\return signal time-series
	*/
	MDM_API const std::vector<double>& StData() const;

	//! Return signal-derived contrast-agent concentration time-series
	/*!
	\return signal-derived contrast-agent concentration time-series
	*/
	MDM_API const std::vector<double>&	CtData() const;
	
	//! Return IAUC value at specified index
	/*!
	\param idx index into IAUC values
	\return IAUC value at specified index
	*/
	MDM_API double			IAUC_val(size_t idx) const;

	//! Return IAUC time at specified index
	/*
	\param idx index into IAUC times
	\return IAUC time at specified index
	*/
	MDM_API double			IAUC_time(size_t idx) const;
	
	//! Return enhancing status
	/*!
	\return enhancing status, true if voxel is enhancing OR testEnhancement is set false
	\see testEnhancing()
	*/
	MDM_API bool      enhancing() const;

  //! Test to see if voxel is enhancing, sets internal enhancing flag
  /*!
  */
  MDM_API void testEnhancing();


protected:

private:

	/*METHODS*/

  //
  double computeT1DynPBM(const double st, const double s_pbm, 
    const double T1, const double FA, const double TR, int &errorCode);

  //
  double computeT1DynM0(const double st, const double M0, 
    const double FA, const double TR, int &errorCode);

  //
	std::vector<double> computeIAUC(const std::vector<double> &times);


	

	/*VARIABLES*/

	mdm_DCEVoxelStatus status_;

	std::vector<double> StData_;		//DCE time series vector of signals
	std::vector<double> CtData_;				//DCE time series vector of signal-derived concentrations
  
	
  //double T1_;						//T1 value for this voxel
	//double M0_;						//M0 value for this voxel - not used if using ratio method
	//double r1Const_;				//relaxivity constant of tissue 
  size_t injectionImg_;    //Time point of injection
	std::vector<double> IAUC_times_; //Vector of times (in secs) at which to caclucate IAUC values
	std::vector<double> IAUC_vals_; 
	
	bool enhancing_; //Flag if the voxel enhanced

	//Reference to dynamic times, set at initialization from global volume analysis
	const std::vector<double> &dynamicTimings_;

	//TR and FA values, constant and set at init
	//const double TR_;
	//const double FA_;

	/*Some limits and error values when computing dynamic T1 and hence concentrations*/
	const static double Ca_BAD1;
	const static double Ca_BAD2;
	const static double T1_TOLERANCE;
	const static double DYN_T1_MAX;
	const static double DYN_T1_INVALID;
};



#endif /* MDM_DCEVOXEL_HDR */
