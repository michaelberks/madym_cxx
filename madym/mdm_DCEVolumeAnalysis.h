/**
 *  @file    mdmDCEVolumeAnalysis.h
 *  @brief   Manager class for DCE analysis, stores input images and output parameter maps
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef mdm_DCEVolumeAnalysis_HDR
#define mdm_DCEVolumeAnalysis_HDR
#include "mdm_api.h"

#include "mdm_Image3D.h"

#include "mdm_ErrorTracker.h"
#include "mdm_DCEVoxel.h"
#include "mdm_DCEModelBase.h"
#include "mdm_T1VolumeAnalysis.h"

/**
* @brief Manager class for DCE analysis, stores input images and output parameter maps
*/
class mdm_DCEVolumeAnalysis {

public:

	//Names of output maps
	const static std::string   MAP_NAME_IAUC; //Appended with IAUC time
	const static std::string   MAP_NAME_RESDIUALS;
	const static std::string   MAP_NAME_ENHANCING;
	const static std::string   MAP_NAME_ROI;
	const static std::string   MAP_NAME_T1;
	const static std::string   MAP_NAME_S0;
	const static std::string   MAP_NAME_CT_SIG; //Signal derived concentration - appended with volume number
	const static std::string   MAP_NAME_CT_MOD; //Model estimated concentration - appended with volume number
	const static std::string   MAP_NAME_ERROR_CODE;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_DCEVolumeAnalysis(mdm_ErrorTracker &errorTracker, mdm_T1VolumeAnalysis &T1_mapper);
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_DCEVolumeAnalysis();

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setROIimage(const mdm_Image3D ROI);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D ROIimage() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void addStDataMap(const mdm_Image3D dynImg);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D StDataMap(int i) const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<mdm_Image3D> &StDataMaps() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void addCtDataMap(const mdm_Image3D ctMap);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D CtDataMap(int i) const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<mdm_Image3D> &CtDataMaps() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D CtModelMap(int i) const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API const std::vector<mdm_Image3D> &CtModelMaps() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D modelMap(const std::string &mapName) const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setModelMap(const std::string &mapName, const mdm_Image3D &map);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API std::string modelType() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API std::vector<double> dynamicTimes() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double dynamicTime(int i) const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API std::vector<std::string> paramNames() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API std::vector<double> IAUCtimes() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setRelaxCoeff(double rc);

	//Flag for which model we're using - MB TODO make this an Enum
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setModel(mdm_DCEModelBase *model);

	//Flag to check if we're testing for enhancment
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setTestEnhancement(bool flag);

	//Flag to check if we're using ratio method for converting to concentration
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setUseRatio(bool flag);

	//Flag to see if we need to compute concentration
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setComputeCt(bool flag);

	//Flag to see if we need to output computed concentration
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setOutputCt(bool flag);

  //Flag to see if we need to output modelled concentration
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setOutputCmod(bool flag);

	//Set the time points at which we calculate IAUC
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setIAUCtimes(const std::vector<double> &times);

  //Set whether we're using temporal varying noise if it's econded in the dynamic series xtr headers
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setUseNoise(bool b);

  //Set time ranges for computing model fit
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setFirstImage(int t);
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setLastImage(int t);

  /**
  */
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool createParameterMaps();

	/**
	*/
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool   fitDCEModel(bool paramMapsInitialised = false, bool optimiseModel = true, const std::vector<int> initMapParams = {});

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int   getNDyns() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int   getNCs_t() const;
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API int   getNCm_t() const;

protected:

private:


	/**
	*/
	void  getSignalsFromVoxel(int voxelIndex, std::vector<double> &data);

	void  getCs_tFromVoxel(int voxelIndex, std::vector<double> &data);

  void  getCm_tFromVoxel(int voxelIndex, std::vector<double> &data);

	/**
	*/
	mdm_DCEVoxel setUpDCEVoxel(int voxelIndex, double tr, double fa);
	void setVoxelErrors(int voxelIndex, const mdm_DCEVoxel &p);

	/**
	*/
	void setVoxelInAllMaps(int voxelIndex, const mdm_DCEVoxel  &v);

  void setVoxelModelError(int voxelIndex, const mdm_DCEVoxel  &v);

	/**
	*/
	void nanVoxelInAllMaps(int voxelIndex);

	/**
	*/
	void zeroVoxelInAllMaps(int voxelIndex);

	/**
	*/
	bool  fitModel(bool paramMapsInitialised, bool optimiseModel, const std::vector<int> initMapParams);

	/* See comments for createParameterMaps() */
	bool createMap(mdm_Image3D& img);

	double timeFromTimeStamp(double timeStamp);

	/*VARIABLES:*/
	mdm_Image3D ROI_image_;
	std::vector<mdm_Image3D> StDataMaps_;
	std::vector<mdm_Image3D> CtDataMaps_;
  std::vector<mdm_Image3D> CtModelMaps_;
	mdm_T1VolumeAnalysis &T1_mapper_;
  std::vector<double> dynamicTimes_;
  std::vector<double> noiseVar_;
  mdm_DCEModelBase *model_;

	//Reference to global error tracker
	mdm_ErrorTracker &errorTracker_;

	/* Images for inputs and output */
	std::vector<mdm_Image3D> pkParamMaps_;
	std::vector<mdm_Image3D> IAUCMaps_;
	mdm_Image3D 	modelResidualsMap_;
	mdm_Image3D enhVoxMap_;	

	//Time points at which calculate IAUC values
	std::vector<double> IAUCTimes_;

	double relaxCoeff_;

	//Flag to check if we're testing for enhancment
	bool testEnhancement_;

	//Flag to check if we're using ratio method for converting to concentration
	bool useRatio_;

	//Flag to see if we need to compute concentration
	bool computeCt_;

	//Flag to see if we need to output computed concentration
	bool outputCt_;

  //Flag to see if we need to output modelled concentration
  bool outputCmod_;

  //Flag if we're using temporal varying noise if it's econded in the dynamic series xtr headers
  bool useNoise_;

  //Start and end points for evaluating model
  int firstImage_;
  int lastImage_;
};

#endif /* mdm_DCEVolumeAnalysis_HDR */
