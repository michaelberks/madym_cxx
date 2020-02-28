/**
 *  @file    mdm_ErrorTracker.cxx
 *  @brief   Functions for mapping voxel errors
 *
 *  Original author GA Buonaccorsi 20 Nov 2009
 *  (c) Copyright ISBE, University of Manchester 2009
 *
 *  Documentation comments are in *.h, except for static methods
 *  Version info and list of modifications in comment at end of file
 */

#ifndef MDM_ERR_API_EXPORTS
#define MDM_ERR_API_EXPORTS
#endif
#include "mdm_api.h"

#include "mdm_ErrorTracker.h"

#include <cassert>
#include <madym/mdm_ProgramLogger.h>

/*
	* Error Codes
	*
	* Notes : Using an int(32 bits) to represent errors = > a max of 32 error conditions including "no error"
	*        We use a 32 - bit int because Analyze 7.5 can't handle 64-bit ints - consider modifying in future
	*
	*        Bits 7 - 9 are not used because we removed the rank sum enhancement criterion
	*        They may be restored in future if that criterion is restored, or re - used if it is not
*/
const int mdm_ErrorTracker::OK = 0;									// No error condition                      - Binary no bits set
const int mdm_ErrorTracker::VFA_THRESH_FAIL = 1;		// SigInt(FA = 2deg) < UserSetThreshold    - Binary bit 1 set  
const int mdm_ErrorTracker::T1_INIT_FAIL = 2;				// Initialisation of T1 fitting failed     - Binary bit 2 set  
const int mdm_ErrorTracker::T1_FIT_FAIL = 4;				// Error in main T1 calculation routine    - Binary bit 3 set  
const int mdm_ErrorTracker::T1_MAX_ITER = 8;				// Hit max iterations in T1 calculation    - Binary bit 4 set  
const int mdm_ErrorTracker::T1_MAD_VALUE = 16;			// (T1 < 0.0) || (T1 > 6000.0)             - Binary bit 5 set  
const int mdm_ErrorTracker::S0_NEGATIVE = 32;				// Earlier error condition caused S0 = 0.0 - Binary bit 6 set 
const int mdm_ErrorTracker::NON_ENH_IAUC = 64;			// Voxel non-enhancing by IAUC60 < 0.0     - Binary bit 7 set  
const int mdm_ErrorTracker::CA_IS_NAN = 128;				// [CA](t) == NaN                          - Binary bit 8 set 
const int mdm_ErrorTracker::DYNT1_NEGATIVE = 256;		// T1(t) < 0.0                             - Binary bit 9 set 
const int mdm_ErrorTracker::DCE_INVALID_INPUT = 512;// Input value NaN or -ve                  - Binary bit 10 set 
const int mdm_ErrorTracker::DCE_FIT_FAIL = 1024;		// Error in model fitting optimisation     - Binary bit 11 set
const int mdm_ErrorTracker::DCE_INVALID_PARAM = 2048;// Model fit produced invalid params			 - Binary bit 12 set

MDM_API mdm_ErrorTracker::mdm_ErrorTracker()
{

}

MDM_API mdm_ErrorTracker::~mdm_ErrorTracker()
{

}

//
//
MDM_API const mdm_Image3D& mdm_ErrorTracker::errorImage() const
{
	return errorImage_;
}

//
//
MDM_API bool mdm_ErrorTracker::setErrorImage(const mdm_Image3D &img)
{
	
  //Check input image is not empty and of correct type
  if (img.getNvoxels() <= 0)
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::setErrorImage : Input image is empty\n");
    return false;
  }
	else if (img.getType() != mdm_Image3D::imageType::TYPE_ERRORMAP)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::setErrorImage: "
			"Type of input image does not match TYPE_ERRORMAP\n");
		return false;
	}
	errorImage_ = img;
  return true;
}

//
MDM_API bool mdm_ErrorTracker::initErrorImage(const mdm_Image3D &imgWithDims)
{
	if (errorImage_.getNvoxels() > 0)
		//Error image has already been set, can just return true and get on silently
		return true;

	errorImage_.setType(mdm_Image3D::imageType::TYPE_ERRORMAP);
	errorImage_.copyMatrix(imgWithDims);

	/* ... and make sure it worked */
	if (errorImage_.getNvoxels() <= 0)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::setErrorImage Failed to set up error image\n");
		return false;
	}

	return true;
}

/**
 * @author   GA Buonaccorsi
 * @version  1.0 (2 Mar 2012)
 */
MDM_API bool mdm_ErrorTracker::updateVoxel(const int voxelIndex, const int errCode)
{
  assert(voxelIndex >= 0);

  if (!errorImage_.getNvoxels())
  {
    return false;
  }

  /* Update error at given voxel here */
  int errVal = (int)errorImage_.getVoxel(voxelIndex);
  errVal |= errCode;
	errorImage_.setVoxel(voxelIndex, errVal);

  return true;
}

MDM_API mdm_Image3D mdm_ErrorTracker::maskSingleErrorCode(const int errCodesInt)
{
	

	mdm_Image3D maskOut;

	// Following is crude test that fields have been set
	int nVoxels = errorImage_.getNvoxels();
	if (nVoxels <= 0)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::maskSingleErrorCode: Input image has a zero voxel dimension\n");
		return maskOut;
	}

	maskOut.copyFields(errorImage_);
	maskOut.setTimeStamp(errorImage_.getTimeStamp());

	/* And finally the fun bit */
	for (int iVoxel = 0; iVoxel < nVoxels; iVoxel++)
	{
		double mask_val = (int)errorImage_.getVoxel(iVoxel) & errCodesInt;
		maskOut.setVoxel(iVoxel, mask_val);
	}

	return maskOut;
}

//
