/*!
*  @file    mdm_T1Mapper.h
*  @brief   Class for mapping T1 for a full volume from input signal images
*  @details Currently only variable flip angle method supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef mdm_T1MAPPER_HDR
#define mdm_T1MAPPER_HDR
#include "mdm_api.h"

#include "mdm_Image3D.h"
#include "mdm_ErrorTracker.h"
#include "t1_methods/mdm_T1MethodGenerator.h"

//!Mapping T1 for a full volume from input signal images
/*!
	Also store the resulting T1 and M0 maps, which can also be set from externally precomputed maps.

	Currently only variable flip angle method supported
*/
class mdm_T1Mapper {

public:
	
	//! Constructor
	/*!
	\param errorTracker shared across the volume analysis objects
  \param ROI shared across the volume analysis objects
	*/
	MDM_API mdm_T1Mapper(mdm_ErrorTracker &errorTracker, mdm_Image3D &ROI);
		
	//! Destructor
	/*!
	*/
	MDM_API ~mdm_T1Mapper();

  //! Reset all the maps to empty
  MDM_API void reset();

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
	\param T1 T1 map, pre-computed externally (eg from an earlier analysis) 
	*/
	MDM_API void setT1(mdm_Image3D T1);

	//! Add a precomputed T1 map
	/*!
	\param M0 M0 map, pre-computed externally (eg from an earlier analysis)
	*/
	MDM_API void setM0(mdm_Image3D M0);

  //! Add a B1 correction map
  /*!
  \param B1 map loaded to correct FAs for inhomogeniety
  */
  MDM_API void setB1(mdm_Image3D B1);

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
	MDM_API const mdm_Image3D& inputImage(size_t idx) const;
		
	
	//! Return read-only reference to T1 map
	/*!
	\return read-only reference to T1 map
	*/
	MDM_API const mdm_Image3D& T1() const;
	
	//! Return read-only reference to M0 map
	/*!
	\return read-only reference to M0 map
	*/
	MDM_API const mdm_Image3D& M0() const;

  //! Return read-only reference to B1 map
  /*!
  \return read-only reference to B1 map
  */
  MDM_API const mdm_Image3D& B1() const;

	//! Return T1 value at specified voxel
	/*!
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	\return T1 value at voxel
	*/
	MDM_API double T1(size_t voxel) const;

	//! Return M0 value at specified voxel
	/*!
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	\return T1 value at voxel
	*/
	MDM_API double M0(size_t voxel) const;

  //! Return B1 value at specified voxel
  /*!
  \param voxel index, must be >=0 and < T1Map_.numVoxels()
  \return T1 value at voxel
  */
  MDM_API double B1(size_t voxel) const;

	//! Set T1 and M0 to zero specified voxel
	/*!
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	*/
	MDM_API void zeroVoxel(size_t voxel);

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

  //! Override TR in input images
  /*!
  If zero passed as input, sets TR from first image
  \param TR recovery time
  */
  MDM_API void  overrideTR(double TR);

protected:

private:

	//Methods:

	//
	std::vector<mdm_Image3D> inputImages_;

	// if ROI not empty, only compute values for ROI
	mdm_Image3D &ROI_;

	// Output image maps
	mdm_Image3D T1_, M0_;

  //B1 correction map
  mdm_Image3D B1_;

	//Reference to an error image. If we don't pass one as a constructor to the class
	// a default empty image will be used
	mdm_ErrorTracker &errorTracker_;

	double noiseThreshold_;

	mdm_T1MethodGenerator::T1Methods method_;

};
#endif /* mdm_T1VolumeAnalysis_HDR */
