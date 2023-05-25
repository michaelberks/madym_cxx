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

#ifdef USING_DCMTK
  #include <madym/image_io/dicom/mdm_DicomFormat.h>
#endif

MDM_API std::string mdm_ImageIO::toString(ImageFormat fmt)
{
  switch (fmt)
  {
  case ANALYZE: return "ANALYZE";
  case ANALYZE_SPARSE: return "ANALYZE_SP";
  case NIFTI: return "NIFTI";
  case NIFTI_GZ: return "NIFTI_GZ";
  case DICOM: return "DICOM";
  default:
    throw mdm_exception(__func__, "Unknown format option " + fmt);
  }
}

MDM_API const std::vector<std::string> mdm_ImageIO::validFormats()
{
  return {
    toString(ANALYZE),
    toString(ANALYZE_SPARSE),
    toString(NIFTI),
    toString(NIFTI_GZ),
    toString(DICOM)
  };
}

 //
MDM_API mdm_ImageIO::ImageFormat mdm_ImageIO::formatFromString(const std::string& fmt)
{
  if (fmt == toString(ANALYZE))
    return ANALYZE;
  if (fmt == toString(ANALYZE_SPARSE))
    return ANALYZE_SPARSE;
  else if (fmt == toString(NIFTI))
    return NIFTI;
  else if (fmt == toString(NIFTI_GZ))
    return NIFTI_GZ;
  else if (fmt == toString(DICOM))
    return DICOM;
  else
    throw mdm_exception(__func__, "Unknown format option " + fmt);
}

//
MDM_API mdm_Image3D mdm_ImageIO::readImage3D(ImageFormat imgFormat, 
  const std::string &fileName,
	bool loadXtr, bool applyScaling)
{
  switch (imgFormat)
  {
  case ImageFormat::ANALYZE:
    ; //Fall through to use Analyze
  case ImageFormat::ANALYZE_SPARSE:
    return mdm_AnalyzeFormat::readImage3D(fileName, loadXtr);

  case ImageFormat::NIFTI:
    ; //Fall through to use nifti
  case ImageFormat::NIFTI_GZ:
    return mdm_NiftiFormat::readImage3D(fileName, loadXtr, applyScaling);

  case DICOM:
#ifdef USING_DCMTK
    return mdm_DicomFormat::readImage3D(fileName, loadXtr);
#else
    throw mdm_exception(__func__, "Unable to read DICOM image: this version of madym has been built without DICOM support ");
#endif

  case ImageFormat::UNKNOWN:
    ; //Fall through to error

  default:
    throw mdm_exception(__func__, "Unrecognized image format " + std::to_string(imgFormat));
  }
  
}

MDM_API std::vector<mdm_Image3D> mdm_ImageIO::readImage4D(ImageFormat imgFormat,
  const std::string& fileName,
  bool loadXtr, bool applyScaling)
{
  switch (imgFormat)
  {
  case ImageFormat::ANALYZE:
    ; //Fall through to use Analyze
  case ImageFormat::ANALYZE_SPARSE:
    throw mdm_exception(__func__, "Reading 4D Analyze not yet supported, please use NIFTI instead.");

  case ImageFormat::NIFTI:
    ; //Fall through to use nifti
  case ImageFormat::NIFTI_GZ:
    return mdm_NiftiFormat::readImage4D(fileName, loadXtr, applyScaling);

  case DICOM:
#ifdef USING_DCMTK
    throw mdm_exception(__func__, "Reading 4D DICOM not yet supported ");
#else
    throw mdm_exception(__func__, "Unable to read DICOM image: this version of madym has been built without DICOM support ");
#endif

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
	const mdm_ImageDatatypes::DataType dataTypeFlag, 
  const mdm_XtrFormat::XTR_type xtrTypeFlag,
  bool applyScaling)
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
    mdm_NiftiFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, false, applyScaling);
    break;

  case ImageFormat::NIFTI_GZ:
    mdm_NiftiFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, true, applyScaling);
    break;

  case DICOM:
#ifdef USING_DCMTK
    return mdm_DicomFormat::writeImage3D(baseName, img, dataTypeFlag, xtrTypeFlag, true);
#else
    throw mdm_exception(__func__, "Unable to write DICOM image: this version of madym has been built without DICOM support ");
#endif  

  case ImageFormat::UNKNOWN:
    ; //Fall through to error

  default:
    throw mdm_exception(__func__, "Unrecognized image format " + std::to_string(imgFormat));
  }
  
}

//
MDM_API void mdm_ImageIO::writeImage4D(ImageFormat imgFormat,
  const std::string& baseName,
  const std::vector< mdm_Image3D> & imgs,
  const mdm_ImageDatatypes::DataType dataTypeFlag,
  const mdm_XtrFormat::XTR_type xtrTypeFlag,
  bool applyScaling)
{
  if (xtrTypeFlag != mdm_XtrFormat::XTR_type::BIDS)
    throw mdm_exception(__func__, "XTR format must be BIDS for 4D writing. Check input option use_BIDS is set.");

  switch (imgFormat)
  {
  case ImageFormat::NIFTI:
    mdm_NiftiFormat::writeImage4D(baseName, imgs, dataTypeFlag, xtrTypeFlag, false, applyScaling);
    break;

  case ImageFormat::NIFTI_GZ:
    mdm_NiftiFormat::writeImage4D(baseName, imgs, dataTypeFlag, xtrTypeFlag, true, applyScaling);
    break;

  case DICOM:
    ; //Fall through
  case ImageFormat::ANALYZE:
    ; //Fall through

  case ImageFormat::ANALYZE_SPARSE:
    throw mdm_exception(__func__, "4D writing is not supported for Analyze 7.5 or DICOM formats. Use NIFTI or NIFT_GZ");

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

  case DICOM:
#ifdef USING_DCMTK
    return mdm_DicomFormat::filesExist(baseName, warn);
#else
    throw mdm_exception(__func__, "This version of madym has been built without DICOM support ");
#endif  
    

  case ImageFormat::UNKNOWN:
    ; //Fall through to error

  default:
    throw mdm_exception(__func__, "Unrecognized image format " + std::to_string(imgFormat));
  }
}