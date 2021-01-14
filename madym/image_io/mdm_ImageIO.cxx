/**
 *  @file    mdm_ImageIO.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_ImageIO.h"
#include <madym/image_io/nifti/mdm_NiftiFormat.h>

 //
MDM_API mdm_ImageIO::ImageFormat mdm_ImageIO::formatFromString(const std::string& fmt)
{
  if (fmt == "ANALYZE")
    return ANALYZE;
  if (fmt == "ANALYZE_SP")
    return ANALYZE_SPARSE;
  else if (fmt == "NIFTI")
    return NIFTI;
  else if (fmt == "NIFTI_GZ")
    return NIFTI_GZ;
  else
    throw mdm_exception(__func__, "Unknown format option " + fmt);
}

//
MDM_API mdm_Image3D mdm_ImageIO::readImage3D(ImageFormat imgFormat, 
  const std::string &fileName,
	bool load_xtr)
{
  switch (imgFormat)
  {
  case ImageFormat::ANALYZE:
    ; //Fall through to use Analyze
  case ImageFormat::ANALYZE_SPARSE:
    return mdm_AnalyzeFormat::readImage3D(fileName, load_xtr);

  case ImageFormat::NIFTI:
    ; //Fall through to use nifti
  case ImageFormat::NIFTI_GZ:
    return mdm_NiftiFormat::readImage3D(fileName, load_xtr);

  case ImageFormat::UNKNOWN:
    ; //Fall through to error

  default:
    throw mdm_exception(__func__, "Unrecognized image format " + std::to_string(imgFormat));
  }
  
}

//
MDM_API void mdm_ImageIO::writeImage3D(ImageFormat imgFormat, 
  const std::string &baseName,
	const mdm_Image3D &img,
	const mdm_ImageDatatypes::DataType dataTypeFlag, const mdm_XtrFormat::XTR_type xtrTypeFlag)
{
  switch (imgFormat)
  {
  case ImageFormat::ANALYZE:
    mdm_AnalyzeFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, false);
    break;

  case ImageFormat::ANALYZE_SPARSE:
    mdm_AnalyzeFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, true);
    break;

  case ImageFormat::NIFTI:
    mdm_NiftiFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, false);
    break;

  case ImageFormat::NIFTI_GZ:
    mdm_NiftiFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, true);
    break;

  case ImageFormat::UNKNOWN:
    ; //Fall through to error

  default:
    throw mdm_exception(__func__, "Unrecognized image format " + std::to_string(imgFormat));
  }
  
}

//
MDM_API bool mdm_ImageIO::filesExist(ImageFormat imgFormat,
  const std::string & baseName,
  bool warn)
{
  switch (imgFormat)
  {
  case ImageFormat::ANALYZE:
    return mdm_AnalyzeFormat::filesExist(baseName, warn);

  case ImageFormat::NIFTI:
    ; //Fall through to use nifti
  case ImageFormat::NIFTI_GZ:
    return mdm_NiftiFormat::filesExist(baseName, warn);

  case ImageFormat::UNKNOWN:
    ; //Fall through to error

  default:
    throw mdm_exception(__func__, "Unrecognized image format " + std::to_string(imgFormat));
  }
}