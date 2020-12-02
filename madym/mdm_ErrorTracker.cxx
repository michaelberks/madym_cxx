/**
*  @file    mdm_ErrorTracker.cxx
*  @brief   Implementation of mdm_ErrorTracker class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_ErrorTracker.h"

#include <cassert>
#include <madym/mdm_exception.h>

//
//
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
MDM_API void mdm_ErrorTracker::setErrorImage(const mdm_Image3D &img)
{
	
  //Check input image is not empty and of correct type
  if (!img)
    throw mdm_exception(__func__, "Trying to set error image from empty image");
    
	else if (img.type() != mdm_Image3D::ImageType::TYPE_ERRORMAP)
    throw mdm_exception(__func__, "Type of input image does not match TYPE_ERRORMAP");
		
	errorImage_ = img;
}

//
MDM_API void mdm_ErrorTracker::initErrorImage(const mdm_Image3D &imgWithDims)
{
	if (errorImage_)
		//Error image has already been set, can just return true and get on silently
		return;

	errorImage_.setType(mdm_Image3D::ImageType::TYPE_ERRORMAP);
	errorImage_.setDimensions(imgWithDims);
}

MDM_API void mdm_ErrorTracker::resetErrorImage()
{
  errorImage_.reset();
}

//
MDM_API void mdm_ErrorTracker::updateVoxel(const size_t voxelIndex, ErrorCode errCode)
{
  
  // Update error at given voxel here - no need for size checks, access will throw
  //appropriate mdm_exception if voxelIndex out of range or image empty
  int errVal = (int)errorImage_.voxel(voxelIndex);
  errVal |= errCode;
	errorImage_.setVoxel(voxelIndex, errVal);
}

MDM_API mdm_Image3D mdm_ErrorTracker::maskSingleErrorCode(const int errCodesInt)
{
	// Following is crude test that fields have been set
	auto nVoxels = errorImage_.numVoxels();
	if (nVoxels <= 0)
    throw mdm_exception(__func__, "Attempting to mask empty error image");

  mdm_Image3D maskOut;
	maskOut.copy(errorImage_);
	maskOut.setType(mdm_Image3D::ImageType::TYPE_ERRORMAP);
	maskOut.setTimeStampFromDoubleStr(errorImage_.timeStamp());

	/* And finally the fun bit */
	for (size_t iVoxel = 0; iVoxel < nVoxels; iVoxel++)
	{
		double mask_val = (int)errorImage_.voxel(iVoxel) & errCodesInt;
		maskOut.setVoxel(iVoxel, mask_val);
	}

	return maskOut;
}

//
