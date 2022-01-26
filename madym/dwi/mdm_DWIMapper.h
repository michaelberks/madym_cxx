/*!
*  @file    mdm_DWIMapper.h
*  @brief   Class for mapping T1 for a full volume from input signal images
*  @details Currently only variable flip angle method supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef mdm_DWIMAPPER_HDR
#define mdm_DWIMAPPER_HDR
#include "mdm_api.h"

#include "mdm_Image3D.h"
#include "mdm_ErrorTracker.h"
#include "dwi/mdm_DWIMethodGenerator.h"

//!Mapping T1 for a full volume from input signal images
/*!
	Also store the resulting T1 and M0 maps, which can also be set from externally precomputed maps.

	Currently only variable flip angle method supported
*/
class mdm_DWIMapper {

public:
	
	//! Constructor
	/*!
	\param errorTracker shared across the volume analysis objects
  \param ROI shared across the volume analysis objects
	\param BvalsThresh 
	*/
	MDM_API mdm_DWIMapper(mdm_ErrorTracker &errorTracker, mdm_Image3D &ROI);
		
	//! Destructor
	/*!
	*/
	MDM_API ~mdm_DWIMapper();

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
	MDM_API void  mapDWI(mdm_DWIMethodGenerator::DWIMethods method);

	//! Map baseline T1 using default class method
	/*!
	\param method selected T1 mapping method
	*/
	MDM_API void  mapDWI();

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
	\param map_name
	\return read-only reference to T1 map
	*/
	MDM_API const mdm_Image3D& model_map(const std::string &map_name) const;

	//! Return T1 value at specified voxel
	/*!
	\param map_name
	\param voxel index, must be >=0 and < T1Map_.numVoxels()
	\return T1 value at voxel
	*/
	MDM_API double model_map(const std::string& map_name, size_t voxel) const;

	//! Return parameter names of diffusion model
	/*!
	\return model parameter names
	*/
	MDM_API std::vector<std::string> paramNames() const;

	//! Return default T1 mapping method
	/*!
	\return T1 method used if no method specified in mapT1
	\see mapT1
	*/
	MDM_API mdm_DWIMethodGenerator::DWIMethods  method() const;

	//! Set default T1 mapping method
	/*!
	\param method used if no method specified in mapT1
	\see mapT1
	*/
	MDM_API void  setMethod(mdm_DWIMethodGenerator::DWIMethods method);

	//!Set BvalsThresh
	/*!
	\param BvalsThresh
	*/
	MDM_API void  setBvalsThresh(const std::vector<double>& BvalsThresh);

protected:

private:

	//Methods:

	//
	std::vector<mdm_Image3D> inputImages_;
	mdm_Image3D SSR_;

	// if ROI not empty, only compute values for ROI
	mdm_Image3D &ROI_;

	// Output image maps
	std::vector<mdm_Image3D> modelMaps_;

	//Reference to an error image. If we don't pass one as a constructor to the class
	// a default empty image will be used
	mdm_ErrorTracker &errorTracker_;

	mdm_DWIMethodGenerator::DWIMethods method_;

	//Name of parameters associated with model maps
	std::vector<std::string> paramNames_;

	// Output image maps
	std::vector<double> BvalsThresh_;

};
#endif /* mdm_T1VolumeAnalysis_HDR */
