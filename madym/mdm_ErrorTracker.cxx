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
#include <madym/mdm_ProgramLogger.h>

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
MDM_API bool mdm_ErrorTracker::setErrorImage(const mdm_Image3D &img)
{
	
  //Check input image is not empty and of correct type
  if (img.numVoxels() <= 0)
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::setErrorImage : Input image is empty\n");
    return false;
  }
	else if (img.type() != mdm_Image3D::ImageType::TYPE_ERRORMAP)
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
	if (errorImage_.numVoxels() > 0)
		//Error image has already been set, can just return true and get on silently
		return true;

	errorImage_.setType(mdm_Image3D::ImageType::TYPE_ERRORMAP);
	errorImage_.setDimensions(imgWithDims);

	/* ... and make sure it worked */
	if (errorImage_.numVoxels() <= 0)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::setErrorImage Failed to set up error image\n");
		return false;
	}

	return true;
}

/*
 */
MDM_API bool mdm_ErrorTracker::updateVoxel(const int voxelIndex, ErrorCode errCode)
{
  assert(voxelIndex >= 0 && voxelIndex < errorImage_.numVoxels());

  if (!errorImage_.numVoxels())
  {
    return false;
  }

  /* Update error at given voxel here */
  int errVal = (int)errorImage_.voxel(voxelIndex);
  errVal |= errCode;
	errorImage_.setVoxel(voxelIndex, errVal);

  return true;
}

MDM_API mdm_Image3D mdm_ErrorTracker::maskSingleErrorCode(const int errCodesInt)
{
	

	mdm_Image3D maskOut;

	// Following is crude test that fields have been set
	int nVoxels = errorImage_.numVoxels();
	if (nVoxels <= 0)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_ErrorTracker::maskSingleErrorCode: Input image has a zero voxel dimension\n");
		return maskOut;
	}

	maskOut.copy(errorImage_);
	maskOut.setType(mdm_Image3D::ImageType::TYPE_ERRORMAP);
	maskOut.setTimeStampFromDoubleStr(errorImage_.timeStamp());

	/* And finally the fun bit */
	for (int iVoxel = 0; iVoxel < nVoxels; iVoxel++)
	{
		double mask_val = (int)errorImage_.voxel(iVoxel) & errCodesInt;
		maskOut.setVoxel(iVoxel, mask_val);
	}

	return maskOut;
}

//
