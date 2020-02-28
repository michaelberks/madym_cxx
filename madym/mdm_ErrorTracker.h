/**
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

/**
*  @brief   Records error codes for each voxel through the DCE modelling process
*/

class mdm_ErrorTracker {

public:

		/* 
		*Error Codes
		*
		* Notes : Using an int(32 bits) to represent errors = > a max of 32 error conditions including "no error"
		*        We use a 32 - bit int because Analyze 7.5 can't handle 64-bit ints - consider modifying in future
		*
		*        Bits 7 - 9 are not used because we removed the rank sum enhancement criterion
		*        They may be restored in future if that criterion is restored, or re - used if it is not
		*/
	static const int OK;								// No error condition                      - Binary no bits set
	static const int VFA_THRESH_FAIL;		// SigInt(FA = 2deg) < UserSetThreshold    - Binary bit 1 set  
	static const int T1_INIT_FAIL;			// Initialisation of T1 fitting failed     - Binary bit 2 set  
	static const int T1_FIT_FAIL;				// Error in main T1 calculation routine    - Binary bit 3 set  
	static const int T1_MAX_ITER;				// Hit max iterations in T1 calculation    - Binary bit 4 set  
	static const int T1_MAD_VALUE;			// (T1 < 0.0) || (T1 > 6000.0)             - Binary bit 5 set  
	static const int S0_NEGATIVE;				// Earlier error condition caused S0 = 0.0 - Binary bit 6 set 
	static const int NON_ENH_IAUC;			// Voxel non-enhancing by IAUC60 < 0.0     - Binary bit 7 set  
	static const int CA_IS_NAN;					// [CA](t) == NaN                          - Binary bit 8 set 
	static const int DYNT1_NEGATIVE;		// T1(t) < 0.0                             - Binary bit 9 set 
	static const int DCE_INVALID_INPUT; // Input value NaN or -ve                  - Binary bit 10 set 
	static const int DCE_FIT_FAIL;			// Error in model fitting optimisation     - Binary bit 11 set
	static const int DCE_INVALID_PARAM;	// Error in model fitting optimisation     - Binary bit 11 set

	//
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_ErrorTracker();

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_ErrorTracker();

/**
 * @brief    Return the error image
 * @return   Const reference to error image member variable (a mdm_Image3D object)
 */
	MDM_API const mdm_Image3D& errorImage() const;

/**
 * @brief    Set error image
 * @param    img   mdm_Image3D object
 * @param    errString     String array to hold error message if required
 * @return   bool true on success or false otherwise
 */
	MDM_API bool setErrorImage(const mdm_Image3D &imgWithDims);

	/**
 * @brief    Initialise error image, copying dimensions from existing image
 * @param    imgWithDims   mdm_Image3D object with voxel and matrix dimensions to copy
 * @param    errString     String array to hold error message if required
 * @return   bool true on success or false otherwise
 */
	MDM_API bool initErrorImage(const mdm_Image3D &img);

/**
 * @brief    Update a voxel in the error image with the specified error code
 * @param    voxelIndex  Integer image voxel index (from x, y, z co-ordinates)
 * @param    errCode     Integer error code
 * @return   Integer 0 on success or 1 if error code map is  not initialised
 * DbC stuff ...
 * @pre    voxelIndex is a valid error image co-ordinate
 * @pre    errCode is a valid error code
 * @pre    the error map must have been initialised using qbiSetErrorImg()
 * @post   the error image voxel will be updated with the new error code
 */
	MDM_API bool updateVoxel(const int voxelIndex, const int errCode);

/**
 * @brief    Write the error image, free its storage and null the pointers
 * @param    fileName    String name of Analyze error image
 * @param    errString   String array to hold error message if required
 * @return   Integer 0 on success or 1 otherwise
 * DbC stuff ...
 * @pre    errString is a valid pointer to a string to hold an error message
 * @post   the error image will be written to an Analyze hdr/img file pair
 * @post   all storage associated with the error image will be freed
 * @post   on error, errString holds the error message, overwriting any previous contents
 */
	MDM_API mdm_Image3D maskSingleErrorCode(const int errCodesInt);

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
