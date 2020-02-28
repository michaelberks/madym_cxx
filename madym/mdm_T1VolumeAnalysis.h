/**
*  @file    mdm_T1VolumeAnalysis.h
*  @brief   Class for mapping T1 for a full volume from input signal images
*  @details Currently only variable flip angle method supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef mdm_T1VolumeAnalysis_HDR
#define mdm_T1VolumeAnalysis_HDR
#include "mdm_api.h"

#include "mdm_Image3D.h"
#include "mdm_ErrorTracker.h"

/**
*  @brief   Mapping T1 for a full volume from input signal images
*  @details Currently only variable flip angle method supported
*/
class mdm_T1VolumeAnalysis {

public:

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_T1VolumeAnalysis(mdm_ErrorTracker &errorTracker);/**/
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_T1VolumeAnalysis();

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void addFlipAngleImage(mdm_Image3D FA_img);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void addT1Map(mdm_Image3D T1_img);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void addS0Map(mdm_Image3D S0_img);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void addROI(mdm_Image3D ROI);

	/**
	* Pre-conditions:
	* -  first_image, second_image & third_image loaded and holding the respective FA_* images
	* -  noise_threshold holds a valid value for the upper noise level
	*
	* Post-conditions:
	* -  T1 holds map of T1 values for the current slice, calculated from the three FA_* images
	* -  S0 holds map of S0 values for the current slice, calculated from the three FA_* images
	*
	* Uses madym.h globals:
	* -  first_image, second_image, third_image    (input only)
	* -  T1value, S0value                          (output - values set)
	* -  T1, S0 Imrect maps                        (output - values set)
	*
	* Note:  NOT a stand-alone fn - see pre- and post-conditions, and it uses globals
	*
	* @author   GJM Parker
	* @brief    Calculate T1 and S0 maps from the three pre-contrast flip angle image volumes (FA_*)
	* @version  madym 1.21.alpha
	*/
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void  T1_mapVarFlipAngle();

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D FAImage(int i) const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D T1Map() const;
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_Image3D S0Map() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double T1atVoxel(int voxel) const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double S0atVoxel(int voxel) const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void zeroVoxel(int voxel);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API double  noiseThreshold() const;

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void  setNoiseThreshold(double t);

protected:

private:

	/* Input images - MB major change, store these in a vector container
	allows for variable numbers - not fixed at requring 3*/
	std::vector<mdm_Image3D> FA_images_;

	/*MB if ROI not empty, only compute values for ROI*/
	mdm_Image3D ROI_;

	/* Output images */
	mdm_Image3D T1_, S0_;

	//Reference to an error image. If we don't pass one as a constructor to the class
	// a default empty image will be used
	mdm_ErrorTracker &errorTracker_;

	double noiseThreshold_;

};
#endif /* mdm_T1VolumeAnalysis_HDR */
