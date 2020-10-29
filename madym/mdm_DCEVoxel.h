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

#include <madym/opt/linalg.h>

#include <vector>
#include <memory>

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
    M0_BAD = 4 ///> Baseline M0 is invalid
  };

	
	//! Constructor
	/*!
	
	\param dynSignals time-series of dynamic signals (if empty, requires dynConc)
	\param dynConc time-series of signal-derived concentration (if empty, computed from dynSignals)
	\param noiseVar temporal-varying noise (if empty, constant noise=1 used)
	\param T10 baseline T1
	\param M0 baseline M0 (if not set, requires useM0Ratio true)
	\param r1Const relaxivity coefficient of contrast-agent
	\param bolus_time timepoint bolus injected
	\param dynamicTimings time in minutes of each series timepoint
	\param TR repetition time
	\param FA flip angle
	\param timepoint0 first timepoint used in model fit 
	\param timepointN last timepoint used in model fit 
	\param testEnhancement flag to test enhancement
	\param useM0Ratio flag to use M0 ratio method
	\param IAUC_times times at which compute IAUC
	*/
	MDM_API mdm_DCEVoxel(
		mdm_DCEModelBase &model,
    const std::vector<double> &dynSignals,
		const std::vector<double> &dynConc,
    const std::vector<double> &noiseVar,
		const double T10,
		const double M0,
		const double r1Const,
    const int bolus_time,
		const std::vector<double> &dynamicTimings,
		const double TR,
		const double FA,
		const int timepoint0,
		const int timepointN,
		const bool testEnhancement,
		const bool useM0Ratio,
		const std::vector<double> &IAUC_times);

	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_DCEVoxel();

  //! Convert signal time-series to contrast agent concentration
	/*!
	*/
	MDM_API void computeCtFromSignal();

  //! Compute modelled C(t) at initial model parameters
  /*!
	*/
	MDM_API void initialiseModelFit();

	//! Optimise tracer-kinetic model fit to concentration time-series
	/*!
	*/
	MDM_API void  fitModel();

	//! Compute IAUC values at selected times
	/*!
	*/
	MDM_API void computeIAUC();

  //! Return the current error status
	/*!
	\see mdm_DCEVoxelStatus
	*/
	MDM_API mdm_DCEVoxelStatus status() const;
		
	//! Return first timepoint used in computing model fit
	/*!
	\return first timepoint used in computing model fit
	*/
	MDM_API int timepoint0() const;
	
	//! Return last timepoint used in computing model fit
	/*!
	\return last timepoint used in computing model fit
	*/
	MDM_API int timepointN() const;
	
	//! Return signal time-series
	/*!
	\return signal time-series
	*/
	MDM_API const std::vector<double>& signalData() const;

	//! Return signal-derived contrast-agent concentration time-series
	/*!
	\return signal-derived contrast-agent concentration time-series
	*/
	MDM_API const std::vector<double>&	CtData() const;
  
	//! Return signal-derived contrast-agent concentration time-series
	/*!
	\return model-estimated contrast-agent concentration time-series
	*/
	MDM_API const std::vector<double>&	CtModel() const;
	
	//! Return baseline T1
	/*!
	\return baseline T1
	*/
	MDM_API double     T1() const;
	
	//! Return baseline M0
	/*!
	\return baseline M0
	*/
	MDM_API double     M0() const;
	
	//! Return contrast-agent relaxivity coefficient
	/*!
	\return contrast-agent relaxivity coefficient
	*/
	MDM_API double     r1Const() const;
	
	//! Return IAUC value at specified index
	/*!
	\param idx index into IAUC values
	\return IAUC value at specified index
	*/
	MDM_API double			IAUC_val(int idx) const;

	//! Return IAUC time at specified index
	/*
	\param idx index into IAUC times
	\return IAUC time at specified index
	*/
	MDM_API double			IAUC_time(int idx) const;
	
	//! Return model fit error (sum of squared residuals)
	/*!
	\return fit error
	*/
	MDM_API double     modelFitError() const;
	
	//! Return enhancing status
	/*!
	\return enhancing status, true if voxel is enhancing OR testEnhancement is set false
	\see testEnhancement()
	*/
	MDM_API bool      enhancing() const;
	
	//! Return test enhancement flag
	/*!
	\return test enhancement flag, if true, only enhancing voxels are fitted
	*/
	MDM_API bool			testEnhancement() const;
	
	//! Return flag for using M0 ratio
	/*!
	\return test enhancement flag, if true, only enhancing voxels are fitted
	*/
	MDM_API bool			useM0Ratio() const;

protected:

private:

	/*METHODS*/

  /*!
  //!    Calculate dynamic T1 from M0 and dynamic signal intensity
  \param    t1_0  - Baseline T1 value
  \param    st    - dynamic signal intensity
  \param    s_pbm - prebolus mean dynamic signal intensity
  \param    cosfa - cos(flip angle)
  \param    tr    - TR (repetition time)
  \return   Float dynamic T1 value (0.0 on divide by zero or other error)
  *
  * @author   Gio Buonaccorsi
  * @version  1.21.alpha (12 August 2013)
  */
  double computeT1DynPBM(const double &st, const double &s_pbm, const double &cosfa, int &errorCode);

  /*!
  //!    Calculate dynamic T1 from M0 and dynamic signal intensity
  \param    s0    - M0 in signal intensity domain
  \param    st    - dynamic signal intensity
  \param    sinfa - sin(flip angle)
  \param    cosfa - cos(flip angle)
  \param    tr    - TR (repetition time)
  \return   Float dynamic T1 value (0.0 on divide by zero or other error)
  *
  * @author   Gio Buonaccorsi
  * @version  1.21.alpha (12 August 2013)
  */
  double computeT1DynM0(const double &st, const double & sinfa, const double & cosfa, int &errorCode);

	/*!
	*/
	double CtSSD(const std::vector<double> &parameter_array);

  double computeSSD(const std::vector<double> &catModel) const;

	static void CtSSDalglib(const alglib::real_1d_array &x, alglib::real_1d_array &func, void *context) {
		std::vector<double> params(x.getcontent(), x.getcontent() + x.length());
		func[0] = static_cast<mdm_DCEVoxel*>(context)->CtSSD(params);
	}

	static double CtSDDforwarder(void* context, const std::vector<double> &parameter_array) {
		return static_cast<mdm_DCEVoxel*>(context)->CtSSD(parameter_array);
	}

	/*!
	*/
	void optimiseModel();

	/*VARIABLES*/
  mdm_DCEModelBase &model_;

	mdm_DCEVoxelStatus status_;

  
	int										timepoint0_, timepointN_;				//n1(=0?), n2_ total number of datapoints
	std::vector<double>					signalData_;		//DCE time series vector of signals
	std::vector<double>					CtData_;				//DCE time series vector of signal-derived concentrations
  std::vector<double>					noiseVar_;			//DCE time series vector of estimated noise variance for each temporal volume
	double								t10_;						//T1 value for this voxel
	double								m0_;						//M0 value for this voxel - not used if using ratio method
	double								r1Const_;				//relaxivity constant of tissue 
  int                   bolus_time_;    //Time point of injection
	std::vector<double>		IAUC_times_;		//Vector of times (in secs) at which to caclucate IAUC values (and the resulting values)
	std::vector<double>		IAUC_vals_; 
	double								modelFitError_; //SSD error between catData (actual concentrations) and catModel (fitted concentrations)
	bool									enhancing_;				//Flag if the voxel enhanced

	//Reference to dynamic times, set at initialization from global volume analysis
	const std::vector<double> &dynamicTimings_;

	//TR and FA values, constant and set at init
	const double TR_;
	const double FA_;

	//Flag to check if we're testing for enhancment
	bool testEnhancement_;

	//Flag to check if we use the ratio method for scaling signal or M0 computed alongside T1
	bool useM0Ratio_;

	//Upper an lower bounds to use with optimiser
	alglib::real_1d_array lowerBoundsOpt_;
	alglib::real_1d_array upperBoundsOpt_;

	/*Some limits and error values when computing dynamic T1 and hence concentrations*/
	const static double Ca_BAD1;
	const static double Ca_BAD2;
	const static double T1_TOLERANCE;
	const static double DYN_T1_MAX;
	const static double DYN_T1_INVALID;
};



#endif /* MDM_DCEVOXEL_HDR */
