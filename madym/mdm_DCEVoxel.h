/**
 *  @file    mdm_DCEVoxel.h
 *  @brief   Class that holds DCE time-series data and an asssociated tracer kinetic model
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_DCEVOXEL_HDR
#define MDM_DCEVOXEL_HDR
#include "mdm_api.h"

#include "mdm_DCEModelBase.h"

#include "opt/linalg.h"

#include <vector>

/**
* @brief Holds DCE time-series data and an asssociated tracer kinetic model
*/
class mdm_DCEVoxel {
public:

  enum mdm_DCEVoxelStatus
  {
    OK = 0,
    DYN_T1_BAD = 1,
    CA_NAN = 2,
    T10_BAD = 3,
    S0_BAD = 4
  };

	//Force initialization with data required to make model fit? If so, make default constructor
	//private
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_DCEVoxel(
    const std::vector<double> &dynSignals,
		const std::vector<double> &dynConc,
    const std::vector<double> &noiseVar,
		const double T10,
		const double S01,
		const double r1Const,
    const int bolus_time,
		const std::vector<double> &dynamicTimings,
		const double TR,
		const double FA,
		const int n1,
		const int n2,
		const bool testEnhancement,
		const bool useRatio,
		const std::vector<double> &IAUC_times);
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_DCEVoxel();

  /**
  * Pre-conditions:
  *
  * Post-conditions
  * -  Global T1value holds current dynamic T1
  *
  * Uses madym.h globals:
  * -  concentrationMapFlag                (input only)
  * -  T1value                             (output - value set)
  *
  * Note:  NOT a stand-alone fn - see pre-conditions & use of globals
  *
  * @brief    Convert signal intensity time-series to [CA] time series
  * @param    p        Pointer to Permeability struct holding input data
  * @param    tr       Float TR (repetition time) value
  * @param    fa       Float FA (flip angle) value
  * @return   double*   Pointer to [CA] time series array
  */
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void computeCtFromSignal();

  //Run an initial model fit using the current model parameters (does not optimise new parameters)
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void initialiseModelFit(mdm_DCEModelBase &model);

	/* The business end */
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void  fitModel();

	/**
	* Moved here for version 2.0
	*
	* Depending on the model, the parameter and lambda arrays are set up (see NR)
	* then the simplex is run and the results are copied to the global mirror
	* variables.  Model-to-data SSD is also calculated and returned.
	*
	* Uses madym.h globals:
	* -  mdmCfg.prebolus
	*
	* Note:  NOT a stand-alone fn - see pre-conditions & side-effects
	*
	* @author   GJM Parker (mods by GA Buonaccorsi)
	* @brief    Calculate Integrated area under the CA(t) curve (simple summation)
	* @version  madym 1.21.alpha
	* @param    nTimes           Integer number of data points in the time series - INPUT
	* @param    seconds         Float array of timing data                       - INPUT
	* @param    concentration   Float array of CA(t) data                        - INPUT
	* @param    IAUC60          Pointer to double holding AUC to 60 s             - OUTPUT
	* @param    IAUC90          Pointer to double holding AUC to 90 s             - OUTPUT
	* @param    IAUC120         Pointer to double holding AUC to 120 s            - OUTPUT
	*/
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void calculateIAUC();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_DCEVoxelStatus status() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int n1() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int n2() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<double>& signalData() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<double>&	CtData() const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<double>&	CtModel() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     t10() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     s0() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     r1Const() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double			IAUC_val(int i) const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double			IAUC_time(int i) const;
  
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     pkParams(int paramIdx) const;
	
	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     pkParams(const std::string &paramName) const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double     modelFitError() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool      enhancing() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool			testEnhancement() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool			useRatio() const;

protected:

private:

	/*METHODS*/

  /**
  * @brief    Calculate dynamic T1 from S0 and dynamic signal intensity
  * @param    t1_0  - Baseline T1 value
  * @param    st    - dynamic signal intensity
  * @param    s_pbm - prebolus mean dynamic signal intensity
  * @param    cosfa - cos(flip angle)
  * @param    tr    - TR (repetition time)
  * @return   Float dynamic T1 value (0.0 on divide by zero or other error)
  *
  * @author   Gio Buonaccorsi
  * @version  1.21.alpha (12 August 2013)
  */
  double computeT1DynPBM(const double &st, const double &s_pbm, const double &cosfa, int &errorCode);

  /**
  * @brief    Calculate dynamic T1 from S0 and dynamic signal intensity
  * @param    s0    - M0 in signal intensity domain
  * @param    st    - dynamic signal intensity
  * @param    sinfa - sin(flip angle)
  * @param    cosfa - cos(flip angle)
  * @param    tr    - TR (repetition time)
  * @return   Float dynamic T1 value (0.0 on divide by zero or other error)
  *
  * @author   Gio Buonaccorsi
  * @version  1.21.alpha (12 August 2013)
  */
  double computeT1DynS0(const double &st, const double & sinfa, const double & cosfa, int &errorCode);

	/**
	* Pre-conditions:
	* -  parameter_array has ndim valid values
	*
	* Post-conditions
	* -  G* file scope static all set (see below)
	*
	* Uses madym.h globals:
	* -  mdmCfg.model_flag
	* Uses file-scope statics:
	* -  GnData, Gx, Gy, Gdose, Gk, Gve, Gvp, Goffset, GHct
	* -  catFromModel[]
	*
	* This is the cost function passed in the parameter list to simplexmin(), so its heading
	* is fixed.  The parameter_array holds the values passed to and returned from simplex,
	* as a simple set of values, so we have to know the ordering and their meanings.
	*
	* The parameter values are checked to be sure they take sensible values and a high cost
	* (FLT_MAX) is returned if they do not.  The SSE is calculated, comparing the [CA] values
	* calculated from the measured signal intensity time series (Gy[]) to the model (file scope
	* static array catFromModel[]), and this SSE returned.
	*
	* Note:  NOT a stand-alone fn - see pre-conditions & side-effects
	*
	* @author   GJM Parker with mods by GA Buonaccorsi
	* @brief    Calculate errors on CA(t) estimated from data cf. fitted model
	* @version  madym 1.21.alpha
	* @param    ndim              Integer number of elements in parameter_array
	* @param    parameter_array   Double array holding model parameters (from simplex)
	* @return   double    Sum squared differences (data to model) or FLT_MAX
	*/
	//double catSSD(int ndim, std::vector<double> &parameter_array);
	double CtSSD(const std::vector<double> &parameter_array);

  double computeSSD(const std::vector<double> &catModel) const;

	/*static void CtSSDalglib(const alglib::real_1d_array &x, double &func, void *context) {
		std::vector<double> params(x.getcontent(), x.getcontent() + x.length());
		func = static_cast<mdm_DCEVoxel*>(context)->CtSSD(params);
	}*/

	static void CtSSDalglib(const alglib::real_1d_array &x, alglib::real_1d_array &func, void *context) {
		std::vector<double> params(x.getcontent(), x.getcontent() + x.length());
		func[0] = static_cast<mdm_DCEVoxel*>(context)->CtSSD(params);
	}


	static double CtSDDforwarder(void* context, const std::vector<double> &parameter_array) {
		return static_cast<mdm_DCEVoxel*>(context)->CtSSD(parameter_array);
	}

	/**
	* Pre-conditions:
	* -  All inputs adequately initialised
	*
	* Post-conditions
	* -  All outputs have valid values
	* -  G* file scope static all set (see below)
	*
	* Simplex set-up and results propagation.
	*
	* Depending on the model, the parameter and lambda arrays are set up (see NR)
	* then the simplex is run and the results are copied to the global mirror
	* variables.  Model-to-data SSD is also calculated and returned.
	*
	* Uses madym.h globals:
	* -  mdmCfg.model_flag, max_iterations
	* Uses file-scope statics:
	* -  GnData, Gx, Gy, Gdose, Gk, Gve, Gvp, Goffset, GHct
	*
	* Note:  NOT a stand-alone fn - see pre-conditions & side-effects
	*
	* @author   GJM Parker (mods by GA Buonaccorsi)
	* @brief    Set up and do simplex fit to required model
	* @version  madym 1.21.alpha
	* @param    nTimes    Integer number of data points in the time series - INPUT
	* @param    kTrans   Pointer to double holding Ktrans                  - OUTPUT
	* @param    ve       Pointer to double holding relative volume of EES  - OUTPUT
	* @param    vp       Pointer to double holding relative plasma volume  - OUTPUT
	* @param    offset   Pointer to double holding  offset time for AIF    - OUTPUT
	* @param    ssd      Pointer to double holding SSD for model v data    - OUTPUT
	*/
	void optimiseModel();

	/*VARIABLES*/
  mdm_DCEVoxelStatus status_;

  mdm_DCEModelBase *model_;

	int										n1_, n2_;				//n1(=0?), n2_ total number of datapoints
	std::vector<double>					signalData_;		//DCE time series vector of signals
	std::vector<double>					CtData_;				//DCE time series vector of signal-derived concentrations
  std::vector<double>					noiseVar_;			//DCE time series vector of estimated noise variance for each temporal volume
	double								t10_;						//T1 value for this voxel
	double								s0_;						//M0 (aka S0) value for this voxel - not used if using ratio method
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

	//Flag to check if we use the ratio method for scaling signal or S0 computed alongside T1
	bool useRatio_;

	//Upper an lower bounds to use with optimiser
	std::vector<double> lowerBounds_;
	std::vector<double> upperBounds_;
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
