#ifndef MDM_DICOMFORMAT_H
#define MDM_DICOMFORMAT_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include <madym/image_io/mdm_ImageDatatypes.h>

#include <dcmtk/dcmdata/dctk.h> 

//! NIFTI image format reading and writing
 /*!
 */
class mdm_DicomFormat {

public:

  //!    Read Analyze format file(s) and return mdm_Image3D object
  /*!
  \param    fileName		name of file from which to read the data
  \param		loadXtr			flag, if true tries to load .xtr file too
  \return   mdm_Image3D object containing image read from disk
  */
  MDM_API static mdm_Image3D readImage3D(const std::string& fileName,
    bool loadXtr);


  //!    Write mdm_Image3D to QBI extended Analyze hdr/img/xtr file set
  /*!
  \param    fileName      base name for file (gets .hdr/.img/.xtr appended)
  \param    img           mdm_Image3D holding the data to be written to file
  \param    dataTypeFlag  integer data type flag; see Data_type enum
  \param    xtrTypeFlag   integer xtr type flag; 0 for old, 1 for new
  \param		compress			flag, if true, write out compressed image (nii.gz)
  */
  MDM_API static void writeImage3D(const std::string & fileName,
    const mdm_Image3D &img,
    const mdm_ImageDatatypes::DataType dataTypeFlag,
    const mdm_XtrFormat::XTR_type xtrTypeFlag,
    bool compress = false);

  //!    Test for existence of the file with the specified basename
  /*!
  \param    fileName base name of files to test
  \param    warn  flag, if true triggers warning for program logger if files don't exist
  \return   bool true if files exist, false otherwise
  */
  MDM_API static bool filesExist(const std::string & fileName,
    bool warn = false);

  //!    Load DICOM slices into an mdm_Image3D object 
  /*!
  \param  dimensions of the 3D image volume
  \param  voxelSize  of the 3D image volume
  \param  sliceNames list of slice names
  \param  offset optional value to offset the voxel values v_out = (v_in - offset)/scale
  \param  scale optional value to scale the voxel values v_out = (v_in - offset)/scale
  \param flipX flip each slice horizontally before transferring data (default false)
  \param flipY flip each slice vertically before transferring data (default true)
  \return   mdm_Image3D object
  */
  MDM_API static mdm_Image3D loadImageFromDicomSlices(
    const std::vector<size_t> &dimensions,
    const std::vector<double> &voxelSize,
    const std::vector<std::string> &sliceNames,
    const double offset = 0,
    const double scale = 1.0,
    const bool flipX = false,
    const bool flipY = true);

  //! Get value of a numeric field from DICOM header
  /*!
  \param fileformat refernce to DICOM header object
  \param key tag key to the field
  \return numeric value of field
  \throw mdm_DicomMissingFieldException if field not found in header object
  */
  MDM_API static double getNumericField(DcmFileFormat &fileformat, const DcmTagKey &key);

  //! Get value of a text field from DICOM header
  /*!
  \param fileformat refernce to DICOM header object
  \param key tag key to the field
  \return text value of field
  \throw mdm_DicomMissingFieldException if field not found in header object
  */
  MDM_API static std::string getTextField(DcmFileFormat &fileformat, const DcmTagKey & key);

  //! Get values of a numeric vector field from DICOM header
  /*!
  \param fileformat refernce to DICOM header object
  \param key tag key to the field
  \param numValues number of values in the vector
  \return vector of numeric values for field
  \throw mdm_DicomMissingFieldException if field not found in header object
  */
  MDM_API static std::vector< double> getNumericVector(
    DcmFileFormat &fileformat, const DcmTagKey & key, size_t numValues);

protected:

private:
};

//! Speciliased exception thrown when user requests a field not present in a DICOM header
 /*!
 */
class mdm_DicomMissingFieldException : virtual public mdm_exception {

public:
  //! Constructor from standard string message
  /*!
  \param func name of throwing function
  \param key tag key of missing field, used to auto-generate exception method
  */
  mdm_DicomMissingFieldException(const char* func, DcmTagKey key) noexcept
    : mdm_exception(func, "Missing key ")
  {
    err.append(key.toString().c_str());
  }
};

#endif /* MDM_DICOMFORMAT_H */
