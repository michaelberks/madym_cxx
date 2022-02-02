/**
 *  @file    mdm_DicomFormat.cxx
 *  @brief   Implementation of class for DICOM image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_DicomFormat.h"

#include <madym/mdm_exception.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <boost/algorithm/string.hpp>

//
MDM_API mdm_Image3D mdm_DicomFormat::readImage3D(const std::string& fileName,
  bool loadXtr)
{
  /*DicomImage *image = new DicomImage(fileName.c_str());
  if (image != NULL)
  {
    if (image->getStatus() == EIS_Normal)
    {
      if (image->isMonochrome())
      {
        image->setMinMaxWindow();
        Uint8 *pixelData = (Uint8 *)(image->getOutputData(8));
        if (pixelData != NULL)
        {
          // do something useful with the pixel data
        }
      }
    }
    else
      mdm_exception(__func__, "Error: cannot load DICOM image (" + std::string(DicomImage::getString(image->getStatus())) + ")");
  }
  delete image;*/
  throw mdm_exception(__func__, "DICOM reading is not yet supported");
  mdm_Image3D img;
  return img;
}


//
MDM_API void mdm_DicomFormat::writeImage3D(const std::string & fileName,
  const mdm_Image3D &img,
  const mdm_ImageDatatypes::DataType dataTypeFlag,
  const mdm_XtrFormat::XTR_type xtrTypeFlag,
  bool compress)
{
  throw mdm_exception(__func__, "DICOM writing is not yet supported");

  /*char uid[100];
  const Uint8 *pixelData;
  const long pixelLength = 0;

  DcmFileFormat fileformat;
  DcmDataset *dataset = fileformat.getDataset();
  dataset->putAndInsertString(DCM_SOPClassUID, UID_SecondaryCaptureImageStorage);
  dataset->putAndInsertString(DCM_SOPInstanceUID, dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
  dataset->putAndInsertString(DCM_PatientName, "Doe^John");

  dataset->putAndInsertUint8Array(DCM_PixelData, pixelData, pixelLength);
  OFCondition status = fileformat.saveFile("test.dcm", EXS_LittleEndianExplicit);
  if (status.bad())
    mdm_exception(__func__, "Error: cannot write DICOM file (" + std::string(status.text()) + ")"); */
}

//
MDM_API bool mdm_DicomFormat::filesExist(const std::string & fileName,
  bool warn)
{
  return false;
}

//
MDM_API mdm_Image3D mdm_DicomFormat::loadImageFromDicomSlices(
  const std::vector<size_t> &dimensions,
  const std::vector<double> &voxelSize,
  const std::vector<std::string> &sliceNames,
  const double offset,
  const double scale,
  const bool flipX,
  const bool flipY,
  const bool flipZ)
{
  //num slices should be the same as the number of dimensions
  assert(dimensions.size() == 3);
  assert(voxelSize.size() == 3);
  assert(dimensions[2] == sliceNames.size());

  mdm_Image3D img;
  img.setDimensions(dimensions[0], dimensions[1], dimensions[2]);
  img.setVoxelDims(voxelSize[0], voxelSize[1], voxelSize[2]);

  //Loop through filenames, loading each dicom file and extracting the pixel data
  size_t currSlice = flipZ ? dimensions[2] - 1 : 0;
  size_t nSliceVoxels = dimensions[0] * dimensions[1];
  for (auto sliceName : sliceNames)
  {
    //Load DICOM slice
    DicomImage slice(sliceName.c_str(), CIF_IgnoreModalityTransformation); //

    if (slice.getStatus() != EIS_Normal)
      throw mdm_exception(__func__,
        sliceName + " did not successsfully load. Check DICOM dictionary.");

    if (!slice.isMonochrome())
      throw mdm_exception(__func__,
        sliceName + " is not a monochrome image");

    //Apply image flips
    slice.flipImage(flipX, flipY);

    //Get raw pixel data from slice
    auto pixelData = slice.getInterData();

    //Check voxel count and pre-allocate voxel values vector
    if (pixelData->getCount() != nSliceVoxels)
      throw mdm_exception(__func__, "Count from pixelArray does not match expected number of slice voxels");
    std::vector<double> voxelValues(nSliceVoxels);

    //Get pixel representation and use to determine correct cast of pixel array
    //the copy values into voxels vector
    switch (pixelData->getRepresentation())
    {
    case EPR_Uint8: //unsigned 8 bit integer
    {
      auto pixelArray = (Uint8*)pixelData->getData();
      for (size_t vox = 0; vox < nSliceVoxels; vox++)
        voxelValues[vox] = (double)pixelArray[vox];
      break;
    }
    case EPR_Sint8: //signed 8 bit integer
    {
      auto pixelArray = (int8_t*)pixelData->getData();
      for (size_t vox = 0; vox < nSliceVoxels; vox++)
        voxelValues[vox] = (double)pixelArray[vox];
      break;
    }
    case EPR_Uint16: //unsigned 16 bit integer
    {
      auto pixelArray = (Uint16*)pixelData->getData();
      for (size_t vox = 0; vox < nSliceVoxels; vox++)
        voxelValues[vox] = (double)pixelArray[vox];
      break;
    }
    case EPR_Sint16: //signed 16 bit integer
    {
      auto pixelArray = (int16_t*)pixelData->getData();
      for (size_t vox = 0; vox < nSliceVoxels; vox++)
        voxelValues[vox] = (double)pixelArray[vox];
      break;
    }
    case EPR_Uint32: //unsigned 32 bit integer
    {
      auto pixelArray = (Uint32*)pixelData->getData();
      for (size_t vox = 0; vox < nSliceVoxels; vox++)
        voxelValues[vox] = (double)pixelArray[vox];
      break;
    }
    case EPR_Sint32: //signed 32 bit integer
    {
      auto pixelArray = (int32_t*)pixelData->getData();
      for (size_t vox = 0; vox < nSliceVoxels; vox++)
        voxelValues[vox] = (double)pixelArray[vox];
      break;
    }
    }

    //Set slice in the 3D image
    img.setSlice(currSlice, voxelValues);

    if (flipZ)
      currSlice--;
    else
      currSlice++;
  }

  //Apply scaling if set
  if (offset)
    img -= offset;

  if (scale && scale != 1.0)
    img /= scale;

  return img;
}

//! Auxillary struct to help convert hex strings to floats
union ulf
{
  //! int part of union
  long int l;

  //! float part of union
  float f;
};

MDM_API double mdm_DicomFormat::getNumericField(DcmFileFormat &fileformat, const DcmTagKey & key)
{
  OFString value;
  if (fileformat.getDataset()->findAndGetOFStringArray(key, value).good())
  {
    std::string hex(value.c_str());
    std::string hex_endian;
    if (boost::algorithm::contains(hex, "\\"))
    {
      bool little_endian = true;
      if (little_endian)
      {
        std::vector<std::string> hex_parts;
        boost::split(hex_parts, hex, boost::is_any_of("\\"));
        for (size_t i = 0; i < hex_parts.size(); i++)
          hex_endian += hex_parts[hex_parts.size() - 1 - i];
      }
      else
      {
        hex_endian = hex;
        boost::erase_all(hex_endian, "\\");
      }
      ulf u;
      u.l = strtol(hex_endian.c_str(), (char**)NULL, 16);//hex.c_str()

      return u.f;
    }

    return std::stod(value.c_str());

  }

  else
    throw mdm_DicomMissingFieldException(__func__, key);
}

//
MDM_API std::string mdm_DicomFormat::getTextField(DcmFileFormat &fileformat, const DcmTagKey & key)
{
  OFString value;
  if (fileformat.getDataset()->findAndGetOFString(key, value).good())
    return std::string(value.c_str());
  else
    throw mdm_DicomMissingFieldException(__func__, key);
}

//
MDM_API std::vector< double> mdm_DicomFormat::getNumericVector(
  DcmFileFormat &fileformat, const DcmTagKey & key, size_t numValues)
{
  std::vector< double> values(numValues);

  OFString value;
  for (size_t i = 0; i < numValues; i++)
  {
    if (fileformat.getDataset()->findAndGetOFString(key, value, i).good())
      values[i] = std::stod(value.c_str());
    else
      throw mdm_DicomMissingFieldException(__func__, key);
  }

  return values;
}
