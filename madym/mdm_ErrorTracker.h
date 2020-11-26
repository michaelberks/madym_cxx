/*!
*  @file    mdm_ErrorTracker.h
*  @brief   Class that records error codes for each voxel through the DCE modelling process
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_ERRORTRACKER_HDR
#define MDM_ERRORTRACKER_HDR

#include "mdm_api.h"

#include <madym/mdm_Image3D.h>
#include <string>

/*!
*  @brief   Records error codes for each voxel through the DCE modelling process
*/

class mdm_ErrorTracker {

public:

	//! Enum of defined error codes in T1 mapping and tracer-kinetic model fitting
	/*!
	Each code uses a bit in a 32-bit integer, so that codes maybe added (bit-wise) and indiviudal
	codes recovered from the final aggergate code
	*/
	enum ErrorCode {
		OK = 0,										///> No error condition                      - Binary no bits set
		VFA_THRESH_FAIL = 1,			///> SigInt(FA = 2deg) < UserSetThreshold    - Binary bit 1 set  
		T1_INIT_FAIL = 2,					///> Initialisation of T1 fitting failed     - Binary bit 2 set  
		T1_FIT_FAIL = 4,					///> Error in main T1 calculation routine    - Binary bit 3 set  
		T1_MAX_ITER = 8,					///> Hit max iterations in T1 calculation    - Binary bit 4 set  
		T1_MAD_VALUE = 16,				///> (T1 < 0.0) || (T1 > 6000.0)             - Binary bit 5 set  
		M0_NEGATIVE = 32,					///> Earlier error condition caused M0 = 0.0 - Binary bit 6 set 
		NON_ENH_IAUC = 64,				///> Voxel non-enhancing by IAUC60 < 0.0     - Binary bit 7 set  
		CA_IS_NAN = 128,					///> [CA](t) == NaN                          - Binary bit 8 set 
		DYNT1_NEGATIVE = 256,			///> T1(t) < 0.0                             - Binary bit 9 set 
		DCE_INVALID_INPUT = 512,	///> Input value NaN or -ve                  - Binary bit 10 set 
		DCE_FIT_FAIL = 1024,			///> Error in model fitting optimisation     - Binary bit 11 set
		DCE_INVALID_PARAM = 2048,	///> Error in model fitting optimisation     - Binary bit 12 set
	};
	
	//! Default constructor
	/*!
	*/
	MDM_API mdm_ErrorTracker();

	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_ErrorTracker();

	//!    Return the error image
	/*!
	\return   Const reference to error image member variable (a mdm_Image3D object)
	*/
	MDM_API const mdm_Image3D& errorImage() const;

	//!    Set error image
	/*!
	Input image must be non-empty and of type mdm_Image3D#imageType#TYPE_ERRORMAP, otherwise
	the error image will not be set, and the function will return false.
	\param    imgWithDims   mdm_Image3D object, must have type mdm_Image3D#imageType#TYPE_ERRORMAP
	\see mdm_Image3D#imageType
	*/
	MDM_API void setErrorImage(const mdm_Image3D &imgWithDims);

	//!    Initialise error image, copying dimensions from existing image
	/*!
	\param    img   mdm_Image3D object with voxel and matrix dimensions to copy
	*/
	MDM_API void initErrorImage(const mdm_Image3D &img);

	//!    Update a voxel in the error image with the specified error code
	/*!
	\param    voxelIndex  Integer image voxel index (from x, y, z co-ordinates), must be >=0 and 
	< errorImage_.numVoxels()
	\param    errCode     Integer error code
	\see ErrorCode
	*/
	MDM_API void updateVoxel(const size_t voxelIndex, ErrorCode errCode);

	//! Return mask image where all voxels matching the given error code are set to 1
	/*!
	\param errCode error code to match
	\return masked image as mdm_Image3D object
	*/
	MDM_API mdm_Image3D maskSingleErrorCode(const int errCode);

private:
	mdm_Image3D errorImage_;

};

#endif /* MDM_ERRORTRACKER_HDR */

/*
 *  Modifications:
 *  20-24 Nov 2009 (GAB)
 *  - Created
 *  ============================    Version 0.2    ============================
 *  14 May 2010 (GAB)
 *  - Added QBIERR_CATOOBIG
 *  ============================    Version 1.0    ============================
 *  2 March 2012 (GAB)
 *  - Refactor for BDL, adding MDM* error codes rather than QBI*
 */
