/*!
*  @file    mdm_DWIMapper.h
*  @brief   Class for fitting diffusion models to 3D image volumes
*  @details Currently ADC and IVIM models supported
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef mdm_DWIMAPPER_HDR
#define mdm_DWIMAPPER_HDR
#include <madym/utils/mdm_api.h>

#include <madym/utils/mdm_Image3D.h>
#include <madym/utils/mdm_ErrorTracker.h>
#include "dwi/mdm_DWIModelGenerator.h"

//!Fits diffusion models to 3D image volumes and stores the resulting model parameter maps
/*!
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

	//! Add input signal image
	/*!	
	\param img input image (eg aquired at specific B-value)
	*/
	MDM_API void addInputImage(mdm_Image3D img);

	//! Fit diffusion model to each voxel in volume
	/*!
	\param model selected diffusion model (eg ADC or IVIM)
	*/
	MDM_API void  mapDWI(mdm_DWIModelGenerator::DWImodels model);

	//! Fit diffusion model to each voxel in volume using default class model
	/*!
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
		
	
	//! Return read-only reference to model parameter map
	/*!
	\param map_name
	\return read-only reference to model parameter map
	*/
	MDM_API const mdm_Image3D& model_map(const std::string &map_name) const;

	//! Return model parameter at specified voxel
	/*!
	\param map_name
	\param voxel index, must be >=0 and < numVoxels()
	\return model parameter value at voxel
	*/
	MDM_API double model_map(const std::string& map_name, size_t voxel) const;

	//! Return parameter names of diffusion model
	/*!
	\return model parameter names
	*/
	MDM_API std::vector<std::string> paramNames() const;

	//! Return default model
	/*!
	\return model used if no model specified in mapDWI
	\see mapDWI
	*/
	MDM_API mdm_DWIModelGenerator::DWImodels  model() const;

	//! Set diffusion model
	/*!
	\param model used if no model specified in mapDWI
	\see mapT1
	*/
	MDM_API void  setModel(mdm_DWIModelGenerator::DWImodels model);

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

	mdm_DWIModelGenerator::DWImodels model_;

	//Name of parameters associated with model maps
	std::vector<std::string> paramNames_;

	// Output image maps
	std::vector<double> BvalsThresh_;

};
#endif /* mdm_T1VolumeAnalysis_HDR */
