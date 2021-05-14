/*!
 *  @file    mdm_ImageIO.h
 *  @brief   Class for Analyze image format reading and writing
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_IMAGEIO_H
#define MDM_IMAGEIO_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include <madym/image_io/analyze/mdm_AnalyzeFormat.h>

 //! Analyze image format reading and writing
	/*!
	*/
class mdm_ImageIO {

public:

  //!    Enum of recognized image IO formats
  /*!
  .xtr files are used by Madym to encode meta-information not stored in
  Analyze headers. This enum checks the xtr version. The .xtr version will
  be detected automatically during read. The new format will be used for writing.
  */
  enum ImageFormat {
    UNKNOWN = -1, ///< Image does not have a matching .xtr file
    ANALYZE = 0, ///< Analyze 7.5 format, images are stored as .hdr/.img pairs
    ANALYZE_SPARSE = 1, ///< Custom sparse version of Analyze 7.5 format, images are stored as .hdr/.img pairs, with only non-zero voxel values+indices stored in .img
    NIFTI = 2, ///< NIFTI 2 format, images are stored as .nii
    NIFTI_GZ = 3, ///< NIFTI 2 format zlib compressed, images are stored as .nii.gz
    DICOM = 4, ///< DICOM format, as implemented using DCMTK library
  };

  //! Convert ImageFormat enum value to string
  /*!
  Throws exception if enum not in range
  \param    fmt		must match one of the valid formats
  \return   format as string
  \see validFormats
  \see ImageFormat
  */
  MDM_API static std::string toString(ImageFormat fmt);

  //! Returns list of valid formats
  /*!
  \return List of implemented model names
  */
  MDM_API static const std::vector<std::string> validFormats();

  //! Convert string to ImageFormat enum value
  /*!
  Throws exception if string not valid
  \param    fmt		string input, must match one of the valid formats
  \return   format as ImageFormat enum value
  \see validFormats
  \see ImageFormat
  */
  MDM_API static ImageFormat formatFromString(const std::string& fmt);

	//!    Read Analyze/NIFTI format file(s) and return mdm_Image3D object
	/*!
  \param    imgFormat format of image (Analyze, NIFTI etc) to check
	\param    fileName		name of file from which to read the data
	\param		loadXtr			flag, if true tries to load .xtr file too
	\return   mdm_Image3D object containing image read from disk
	*/
	MDM_API static mdm_Image3D readImage3D(ImageFormat imgFormat, 
    const std::string& fileName,
    bool loadXtr);

	
	//!    Write mdm_Image3D to QBI extended Analyze (hdr/img/xtr) or NIFTI (nii/xtr) file set
	/*!
  \param    imgFormat format of image (Analyze, NIFTI etc) to check
	\param    baseName      base name for file (gets .hdr/.img/.xtr appended)
	\param    img           mdm_Image3D holding the data to be written to file
	\param    dataTypeFlag  integer data type flag; see Data_type enum
	\param    xtrTypeFlag   integer xtr type flag; 0 for old, 1 for new
	*/
	MDM_API static void writeImage3D(ImageFormat imgFormat,
		const std::string & baseName,
    const mdm_Image3D &img,
		const mdm_ImageDatatypes::DataType dataTypeFlag, const mdm_XtrFormat::XTR_type xtrTypeFlag);

  //!    Test for existence of the file with the specified basename and format appropriate extension
  /*!
  \param    imgFormat format of image (Analyze, NIFTI etc) to check
  \param    baseName base name of files to test
  \param    warn  flag, if true triggers warning for program logger if files don't exist
  \return   bool true if files exist, false otherwise
  */
  MDM_API static bool filesExist(ImageFormat imgFormat,
    const std::string & baseName,
    bool warn = false);

protected:

private:
	
};


#endif /* MDM_IMAGEIO_H */
