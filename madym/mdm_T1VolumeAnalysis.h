/*!
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
#include "mdm_T1MethodGenerator.h"

//!Mapping T1 for a full volume from input signal images
/*!
	Also store the resulting T1 and M0 maps, which can also be set from externally precomputed maps.

	Currently only variable flip angle method supported
*/
class mdm_T1VolumeAnalysis {

public:
	
	//! Constructor
	/*!
	\param error tracker object shared acorss the volume analysis objects
	*/
	MDM_API mdm_T1VolumeAnalysis(mdm_ErrorTracker &errorTracker);/*!/
		
	//!
	/*!
	*/
	MDM_API ~mdm_T1VolumeAnalysis();

	//! Add input image from which to map T1
	/*!	
	\param img input image (eg aquired at specific flip-angle for the VFA method)
	*/
	MDM_API void addInputImage(mdm_Image3D img);

	//! Map baseline T1 using specified method
	/*!
	\param method selected T1 mapping method
	*/
	MDM_API void  mapT1(mdm_T1MethodGenerator::T1Methods method);

	//! Map baseline T1 using default class method
	/*!
	\param method selected T1 mapping method
	*/
	MDM_API void  mapT1();

	//! Add a precomputed T1 map
	/*!
	\param T1_img T1 map, pre-computed externally (eg from an earlier analysis) 
	*/
	MDM_API void addT1Map(mdm_Image3D T1_img);

	//! Add a precomputed T1 map
	/*!
	\param M0_img M0 map, pre-computed externally (eg from an earlier analysis)
	*/
	MDM_API void addM0Map(mdm_Image3D M0_img);

	//! Add ROI mask, mapping will only be performed for voxels set non-zero in the mask
	/*!
	\param ROI mask image
	*/
	MDM_API void addROI(mdm_Image3D ROI);

	//! Return read-only reference to input images
	/*!
	\return read-only reference to input images
	*/
	MDM_API const std::vector<mdm_Image3D>& inputImages() const;

	//! Return read-only reference to specific input image
	/*!	
	\param idx index of input images to return, must be >= 0, < inputImages_.size()
	\return read-only reference to input image at index idx
	*/
	MDM_API const mdm_Image3D& inputImage(int idx) const;
		
	
	//! Return read-only reference to T1 map
	/*!
	\return read-only reference to T1 map
	*/
	MDM_API const mdm_Image3D& T1Map() const;
	
	//! Return read-only reference to M0 map
	/*!
	\return read-only reference to M0 map
	*/
	MDM_API const mdm_Image3D& M0Map() const;

	//! Return T1 value at specified voxel
	/*!
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	\return T1 value at voxel
	*/
	MDM_API double T1atVoxel(int voxel) const;

	//! Return M0 value at specified voxel
	/*!
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	\return T1 value at voxel
	*/
	MDM_API double M0atVoxel(int voxel) const;

	//! Set T1 and M0 to zero specified voxel
	/*!
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	*/
	MDM_API void zeroVoxel(int voxel);

	//! Return default T1 mapping method
	/*!
	\return T1 method used if no method specified in mapT1
	\see mapT1
	*/
	MDM_API mdm_T1MethodGenerator::T1Methods  method() const;

	//! Set default T1 mapping method
	/*!
	\param method used if no method specified in mapT1
	\see mapT1
	*/
	MDM_API void  setMethod(mdm_T1MethodGenerator::T1Methods method);

	//! Return current noise threshold
	/*!
	Mapping is only applied to voxels with input signal > threshold
	\return noise threshold
	*/
	MDM_API double  noiseThreshold() const;

	//! Set noise threshold
	/*!
	Mapping is only applied to voxels with input signal > threshold
	\param t noise threshold
	*/
	MDM_API void  setNoiseThreshold(double t);

protected:

private:

	//Methods:

	//
	std::vector<mdm_Image3D> inputImages_;

	// if ROI not empty, only compute values for ROI
	mdm_Image3D ROI_;

	// Output image maps
	mdm_Image3D T1_, M0_;

	//Reference to an error image. If we don't pass one as a constructor to the class
	// a default empty image will be used
	mdm_ErrorTracker &errorTracker_;

	double noiseThreshold_;

	mdm_T1MethodGenerator::T1Methods method_;

};
#endif /* mdm_T1VolumeAnalysis_HDR */
