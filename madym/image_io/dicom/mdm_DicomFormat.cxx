/**@(#)Functions to read/write DICOM image files
       These functions are a modified version of the 
       ACR-NEMA ones
*/

#include "mdm_DicomFormat.h"

#include <madym/mdm_exception.h>
#include <dcmtk/dcmimgle/dcmimage.h>

//
MDM_API mdm_Image3D mdm_DicomFormat::readImage3D(const std::string& fileName,
  bool loadXtr)
{
  DicomImage *image = new DicomImage(fileName.c_str());
  if (image != NULL)
  {
    if (image->getStatus() == EIS_Normal)
    {
      if (image->isMonochrome())
      {
        image->setMinMaxWindow();
        Uint8 *pixelData = (Uint8 *)(image->getOutputData(8 /* bits */));
        if (pixelData != NULL)
        {
          /* do something useful with the pixel data */
        }
      }
    }
    else
      mdm_exception(__func__, "Error: cannot load DICOM image (" + std::string(DicomImage::getString(image->getStatus())) + ")");
  }
  delete image;
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
  mdm_exception(__func__, "DICOM writing is not yet supported");

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
  const bool flipY)
{
  //num slices should be the same as the number of dimensions
  assert(dimensions.size() == 3);
  assert(voxelSize.size() == 3);
  assert(dimensions[2] == sliceNames.size());

  mdm_Image3D img;
  img.setDimensions(dimensions[0], dimensions[1], dimensions[2]);
  img.setVoxelDims(voxelSize[0], voxelSize[1], voxelSize[2]);

  //Loop through filenames, loading each dicom file and extracting the pixel data
  size_t currSlice = 0;
  size_t nSliceVoxels = dimensions[0] * dimensions[1];
  for (auto sliceName : sliceNames)
  {
    //Load DICOM slice
    DicomImage slice(sliceName.c_str(), CIF_IgnoreModalityTransformation); //

    if (slice.getStatus() != EIS_Normal)
      throw mdm_exception(__func__,
        sliceName + " did not successsfully load");

    if (!slice.isMonochrome())
      throw mdm_exception(__func__,
        sliceName + " is not a monochrome image");

    //Can we flip the data in Y?
    slice.flipImage(flipX, flipY);

    //Set voxel data - this is not the most efficient, as we effectively double copy
    //but it should be secure and keeps our image class interface clean
    auto bitDepth = slice.getDepth();
    std::vector<Uint16> voxelValues(nSliceVoxels);

    if (!slice.getOutputData(voxelValues.data(), nSliceVoxels * sizeof(Uint16), bitDepth))
      throw mdm_exception(__func__, "Unable to read slice data");

    //Set slice, casting Uint16 vector to double vector
    img.setSlice(currSlice++,
      std::vector<double>(voxelValues.begin(), voxelValues.end()));
  }

  //Apply scaling if set
  if (offset)
    img -= offset;

  if (scale && scale != 1.0)
    img /= scale;

  return img;
}

//
MDM_API double mdm_DicomFormat::getNumericField(DcmFileFormat &fileformat, const DcmTagKey & key)
{
  OFString value;
  if (fileformat.getDataset()->findAndGetOFString(key, value).good())
    return std::stod(value.c_str());
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
