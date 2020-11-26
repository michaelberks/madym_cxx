/*!
 *  @file    mdm_DCEModelFitter.h
 *  @brief   Class that holds DCE time-series data and an asssociated tracer kinetic model
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_DCEMODELFITTER_HDR
#define MDM_DCEMODELFITTER_HDR
#include <madym/mdm_api.h>

#include <madym/dce_models/mdm_DCEModelBase.h>
#include <madym/mdm_DCEVoxel.h>

#include <madym/opt/linalg.h>

#include <vector>
#include <memory>

//! Holds DCE time-series data and an asssociated tracer kinetic model
/*!
*/
class mdm_DCEModelFitter {
public:

	
	//! Constructor
	/*!
	\param model tracer-kinetic model applied to voxel
	\param timepoint0 first timepoint used in model fit 
	\param timepointN last timepoint used in model fit 
	\param noiseVar temporal-varying noise (if empty, constant noise=1 used)
	\param maxIterations number of iterations to use in fit (if 0, fit to convergence)
	*/
	MDM_API mdm_DCEModelFitter(
		mdm_DCEModelBase &model,
    const size_t timepoint0,
		const size_t timepointN,
    const std::vector<double> &noiseVar,
		const int maxIterations = 0);

	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_DCEModelFitter();

  //! Compute modelled C(t) at initial model parameters
  /*!
  \param CtData contrast-agent concentration time-series
  */
	MDM_API void initialiseModelFit(const std::vector<double> &CtData);

	//! Optimise tracer-kinetic model fit to concentration time-series
	/*!
  \param status validity staus of voxel to fit
	\param enhancing enhancing status of voxel to fit
  */
	MDM_API void  fitModel(const mdm_DCEVoxel::mdm_DCEVoxelStatus status, bool enhancing);
		
	//! Return first timepoint used in computing model fit
	/*!
	\return first timepoint used in computing model fit
	*/
	MDM_API size_t timepoint0() const;
	
	//! Return last timepoint used in computing model fit
	/*!
	\return last timepoint used in computing model fit
	*/
	MDM_API size_t timepointN() const;
  
	//! Return signal-derived contrast-agent concentration time-series
	/*!
	\return model-estimated contrast-agent concentration time-series
	*/
	MDM_API const std::vector<double>&	CtModel() const;
	
	//! Return model fit error (sum of squared residuals)
	/*!
	\return fit error
	*/
	MDM_API double     modelFitError() const;


protected:

private:

	/*METHODS*/

	//
	double CtSSD(const std::vector<double> &parameter_array);

  double computeSSD(const std::vector<double> &CtModel) const;

	static void CtSSDalglib(const alglib::real_1d_array &x, alglib::real_1d_array &func, void *context) {
		std::vector<double> params(x.getcontent(), x.getcontent() + x.length());
		func[0] = static_cast<mdm_DCEModelFitter*>(context)->CtSSD(params);
	}

	//
	void optimiseModel();

	/*VARIABLES*/
  mdm_DCEModelBase &model_;

  
  size_t timepoint0_;
  size_t timepointN_;
  std::vector<double> noiseVar_;			//DCE time series vector of estimated noise variance for each temporal volume

	double modelFitError_; //SSD error between catData (actual concentrations) and catModel (fitted concentrations)

  const std::vector<double>* CtData_;

	//Upper an lower bounds to use with optimiser
	alglib::real_1d_array lowerBoundsOpt_;
	alglib::real_1d_array upperBoundsOpt_;

	//Maximum number of iterations applied
	int maxIterations_;
};



#endif /* MDM_DCEVOXEL_HDR */
