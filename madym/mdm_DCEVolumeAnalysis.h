/*!
 *  @file    mdmDCEVolumeAnalysis.h
 *  @brief   Manager class for DCE analysis, stores input images and output parameter maps
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef mdm_DCEVolumeAnalysis_HDR
#define mdm_DCEVolumeAnalysis_HDR
#include <madym/mdm_api.h>

#include <madym/mdm_Image3D.h>

#include <madym/mdm_ErrorTracker.h>
#include <madym/mdm_DCEVoxel.h>
#include <madym/dce_models/mdm_DCEModelBase.h>
#include <madym/mdm_T1VolumeAnalysis.h>
#include <madym/mdm_DCEModelFitter.h>

#include <memory>

//! Manager class for DCE analysis, stores input images and output parameter maps
/*!
*/
class mdm_DCEVolumeAnalysis {

public:

	// Names of output maps

	//! Base name of IAUC maps, appended with IAUC time
	const static std::string   MAP_NAME_IAUC;

	//! Name of model residuals maps
	const static std::string   MAP_NAME_RESDIUALS;

	//! Name of enhancing map
	const static std::string   MAP_NAME_ENHANCING;

	//! Name of ROI mask
	const static std::string   MAP_NAME_ROI;

  //! Name of AIF map
  const static std::string   MAP_NAME_AIF;

	//! Name of T1 map
	const static std::string   MAP_NAME_T1;

	//! Name of M0 map
	const static std::string   MAP_NAME_M0;

	//! Name of signal derived concentration - appended with volume number
	const static std::string   MAP_NAME_CT_SIG;

	//! Name of modelled concentration - appended with volume number
	const static std::string   MAP_NAME_CT_MOD;

	//! Name of error map
	const static std::string   MAP_NAME_ERROR_CODE;

	//! Default constructor
	/*!
	*/
	MDM_API mdm_DCEVolumeAnalysis();
	
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_DCEVolumeAnalysis();

	//! Return reference to error tracker
	/*!
	\return reference to error tracker
	*/
	MDM_API mdm_ErrorTracker &errorTracker();

	//! Return reference to T1 mapper
	/*!
	\return reference to T1 mapper
	*/
	MDM_API mdm_T1VolumeAnalysis &T1Mapper();

  //! Return read-only reference to T1 mapper
  /*!
  \return const reference to T1 mapper
  */
  MDM_API const mdm_T1VolumeAnalysis &T1Mapper() const;

	//! Set ROI mask
	/*!
	\param ROI mask, dimensions must match those of dynamic series
	\return
	*/
	MDM_API void setROI(const mdm_Image3D ROI);

	//! Return ROI mask
	/*!
	\return ROI mask, if not set, empty image returned
	*/
	MDM_API mdm_Image3D ROI() const;

  //! Set AIF map
  /*!
  \param map, dimensions must match those of dynamic series
  */
  MDM_API void setAIFmap(const mdm_Image3D map);

  //! Compute AIF from dynamic images using current AIF map
  /*!
  \return AIF average C(t) for all voxels i, where AIFmap.voxel(i) == mdm_AIF::SELECTED
  */
  MDM_API std::vector<double> AIFfromMap();


  //! Return AIF map
  /*!
  \return AIF map, if not set, empty image returned
  */
  MDM_API mdm_Image3D AIFmap() const;

	//! Add a signal map to the end of the dynamic time-series S(t)
	/*!
	\param dynImg signal image
	\return
	*/
	MDM_API void addStDataMap(const mdm_Image3D dynImg);

	//! Get signal map at specific time-point in dynamic series
	/*!
	
	\param timepoint
	\return signal map at specified time-point as mdm_Image3D object
	*/
	MDM_API mdm_Image3D StDataMap(int timepoint) const;

  //! Get all signal maps in dynamic series
	/*!
	\return all signal maps in dynamic series
	*/
	MDM_API const std::vector<mdm_Image3D> &StDataMaps() const;

	//! Add a signal-derived contrast-agent concentration map to the end of the dynamic series C(t)
	/*!
	\param ctMap signal-derived contrast-agent concentration map
	*/
	MDM_API void addCtDataMap(const mdm_Image3D ctMap);

	//! Get signal-derived contrast-agent concentration map at specific time-point in dynamic series
	/*!

	\param timepoint
	\return signal-derived contrast-agent concentration map at specified time-point as mdm_Image3D object
	*/
	MDM_API mdm_Image3D CtDataMap(int timepoint) const;

	//! Get all signal-derived contrast-agent concentration maps
	/*!

	\return signal-derived contrast-agent concentration maps
	*/
	MDM_API const std::vector<mdm_Image3D> &CtDataMaps() const;

	//! Get model-estimated contrast-agent concentration map at specific time-point in dynamic series
	/*!

	\param timepoint
	\return model-estimated contrast-agent concentration map at specified time-point as mdm_Image3D object
	*/
	MDM_API mdm_Image3D CtModelMap(int timepoint) const;

	//! Get all model estimated contrast-agent concentration maps
	/*!

	\return model-estimated contrast-agent concentration maps
	*/
	MDM_API const std::vector<mdm_Image3D> &CtModelMaps() const;

	//! Return specified DCE-map given name
	/*!
	
	\param mapName name of parameter map to return
	\return DCE-map
	*/
	MDM_API mdm_Image3D DCEMap(const std::string &mapName) const;

  //! Set DCE-map by name
	/*!	
	\param mapName to set
	\param map to set
	*/
	MDM_API void setDCEMap(const std::string &mapName, const mdm_Image3D &map);

	//! Return type of tracer-kinetic model fitted
	/*!	
	\return model type
	\see mdm_DCEModelBase
	*/
	MDM_API std::string modelType() const;

	//! Return all dynamic series times
	/*!
	\return time in minutes of each time-point in dynamic series
	*/
	MDM_API std::vector<double> dynamicTimes() const;

	//! Return specific dynamic series time
	/*!
	\param timepoint
	\return time in minutes of each time-point
	*/
	MDM_API double dynamicTime(int timepoint) const;

	//! Return parameter names of tracer-kinetic model
	/*!
	\return model parameter names
	\see mdm_DCEModelBase#paramNames
	*/
	MDM_API std::vector<std::string> paramNames() const;

	//! Return times at which IAUC maps are computed
	/*!
	\return times at which IAUC maps are computed
	*/
	MDM_API std::vector<double> IAUCtimes() const;

	//! Set relaxivity coefficient of contrast-agent
	/*!
	\param rc relaxivity coefficient of contrast-agent
	*/
	MDM_API void setR1Const(double rc);

  //! Set prebolus image
  /*!
  \param prebolus timepoint of bolus injection
  */
  MDM_API void setPrebolusImage(int prebolus);

	//! Set the tracer-kineti cmodel
	/*!
	\param model pointer to an instantiation of a specific DCE model
	*/
	MDM_API void setModel(std::shared_ptr<mdm_DCEModelBase> model);

	//! Set test enhancement flag
	/*!
	\param flag true if testing for enchancement. If false all voxels will be fitted.
	*/
	MDM_API void setTestEnhancement(bool flag);

	//! Set use M0 ratio flag
	/*!
	\param flag true if using ratio method to estimate M0, false if using supplied M0 map
	*/
	MDM_API void setM0Ratio(bool flag);

	//! Set flag to compute C(t) from signal
	/*!
	\param flag true if input is signal to convert to contrast-agent concentration. False if C(t) directly supplied.
	\return
	*/
	MDM_API void setComputeCt(bool flag);

	//! Set flag to output signal-derived concentration maps
	/*!
	\param flag
	*/
	MDM_API void setOutputCt(bool flag);

	//! Set flag to output model estimated concentration maps
	/*!
	\param flag
	*/
	MDM_API void setOutputCmod(bool flag);

	//! Set the time points at which we calculate IAUC
	/*!
	\param times IAUC times
	\param convertToMins flag to convert input from seconds to minutes. Default true. Set to false if
	input already in minutes.
	\return
	*/
	MDM_API void setIAUCtimes(const std::vector<double> &times, bool convertToMins = true);

  //!Set temporal varying noise flag
	/*!
	\param flag true if using temporal varying noise (requires noise estimates to be included xtr files of dynamic time series maps)
	*/
	MDM_API void setUseNoise(bool flag);

  
  //! Set first timepoint used in computing model fit
	/*!
	\param timepoint must be >=0 and < numDynamics()
	*/
	MDM_API void setFirstImage(int timepoint);

	//! Set last timepoint used in computing model fit
	/*!
	\param timepoint must be >=0 and < numDynamics()
	*/
	MDM_API void setLastImage(int timepoint);

	//! Set maximum number of iterations used in computing model fit
	/*!
	\param maxItr if 0, no limit set, optimisation runs to convergence
	*/
	MDM_API void setMaxIterations(int maxItr);

  //! Initialise all DCE and tracer-kinetic model maps
	/*!
	Creates maps of appropiate size for each output map, with zero-values in all voxels. 
	Must be called prior to model fitting to ensure there are output containers in which
	to store fitted values, IAUC measures etc.
	*/
	MDM_API bool initialiseParameterMaps();

	//! Fit DCE tracer-kinetic model to all voxels
	/*!
	\param paramMapsInitialised flag if model parameters are loaded from which initial values are set
	\param optimiseModel flag to optimise parameter fits. If false, modelled concentration will be computed
	at the initial values for each voxel.
	\param  initMapParams indices of parameters to initialise from maps. Ignored if paramMapsInitialised is false.
	\return true if model successfully fitted to voxels. Note this returns true even if there were fit
	failures in individual voxels. It only checks volume-wise issues (eg missing dynamic input maps)
	*/
	MDM_API bool   fitDCEModel(bool paramMapsInitialised = false, 
		bool optimiseModel = true, 
		const std::vector<int> initMapParams = {});

	//! Return length of dynamic time-series
	/*!
	\return length of dynamic time-series
	*/
	MDM_API int   numDynamics() const;

  //! Return average concentration time-series for voxels in a map
  /*!
  \param map defining voxels to include in average
  \param map_val value of voxels in map to average
  \param meanCt (output) average of C(t) for all voxels i, where map.voxel(i) == map_val
  \param badVoxels (output) list of voxel indices not included eg due to error in conversion to Ct
  */
  MDM_API void computeMeanCt(
    const mdm_Image3D &map, double map_val, 
    std::vector<double> &meanCt, std::vector<int> &badVoxels) const;

protected:

private:

  mdm_DCEVoxel setUpVoxel(int voxelIndex) const;


	/*!
	*/
	void  voxelStData(int voxelIndex, std::vector<double> &data) const;

	void  voxelCtData(int voxelIndex, std::vector<double> &data) const;

  void  voxelCtModel(int voxelIndex, std::vector<double> &data) const;

	/*!
	*/
	void setVoxelErrors(int voxelIndex, const mdm_DCEVoxel &p);

	/*!
	*/
	void setVoxelInAllMaps(int voxelIndex, 
    const mdm_DCEVoxel  &vox, const mdm_DCEModelFitter &fitter);

  /*!
  */
  void setVoxelInAllMaps(int voxelIndex, double value);

  void setVoxelModelError(int voxelIndex, const mdm_DCEModelFitter &fitter);

	/*!
	*/
	bool  fitModel(bool paramMapsInitialised, bool optimiseModel, const std::vector<int> initMapParams);

	/* See comments for initialiseParameterMaps() */
	bool createMap(mdm_Image3D& img);

  int   numSt() const;

	int   numCtSignal() const;
	
	int   numCtModel() const;

	/*VARIABLES:*/
	mdm_Image3D ROI_;
  mdm_Image3D AIFmap_;
	std::vector<mdm_Image3D> StDataMaps_;
	std::vector<mdm_Image3D> CtDataMaps_;
  std::vector<mdm_Image3D> CtModelMaps_;
  std::vector<double> dynamicTimes_;
  std::vector<double> noiseVar_;
	std::shared_ptr < mdm_DCEModelBase > model_;
  mdm_Image3D referenceDynamicImg_;
  int prebolusImage_;

	mdm_T1VolumeAnalysis T1_mapper_;
	mdm_ErrorTracker errorTracker_;

	/* Images for inputs and output */
	std::vector<mdm_Image3D> pkParamMaps_;
	std::vector<mdm_Image3D> IAUCMaps_;
	mdm_Image3D 	modelResidualsMap_;
	mdm_Image3D enhVoxMap_;	

	//Time points at which calculate IAUC values
	std::vector<double> IAUCTimes_;
	std::vector<double> IAUCTMinutes_;

	double r1Const_;

	//Flag to check if we're testing for enhancment
	bool testEnhancement_;

	//Flag to check if we're using ratio method for converting to concentration
	bool useM0Ratio_;

	//Flag to see if we need to compute concentration
	bool computeCt_;

	//Flag to see if we need to output computed concentration
	bool outputCt_sig_;

  //Flag to see if we need to output modelled concentration
  bool outputCt_mod_;

  //Flag if we're using temporal varying noise if it's econded in the dynamic series xtr headers
  bool useNoise_;

  //Start and end points for evaluating model
  int firstImage_;
  int lastImage_;

	//Maximum number of iterations
	//Maximum number of iterations applied
	int maxIterations_;
};

#endif /* mdm_DCEVolumeAnalysis_HDR */
