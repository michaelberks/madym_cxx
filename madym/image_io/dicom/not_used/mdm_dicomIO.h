#ifndef MDM_DICOMFORMAT_H
#define MDM_DICOMFORMAT_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include <madym/image_io/mdm_ImageDatatypes.h>

#include "dicom_dic.h"

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
  \return   bool 0 for success or 1 for failure
  */
  MDM_API static void writeImage3D(const std::string & fileName,
    const mdm_Image3D &img,
    const mdm_ImageDatatypes::DataType dataTypeFlag,
    const mdm_XtrFormat::XTR_type xtrTypeFlag,
    bool compress = false);

  //!    Test for existence of the file with the specified basename and all NIFTI extensions (.img, .hdr, .nii etc)
  /*!
  \param    fileName base name of files to test
  \param    warn  flag, if true triggers warning for program logger if files don't exist
  \return   bool true if files exist, false otherwise
  */
  MDM_API static bool filesExist(const std::string & fileName,
    bool warn = false);

protected:

private:

  //Reading 
  bool    dicom_hdr_TR_extract(FILE *fp, mdm_Image3D &im);
  bool    dicom_hdr_flip_angle_extract(FILE *fp, mdm_Image3D &im);
  bool    dicom_hdr_image_time_extract(FILE *fp, mdm_Image3D &im);
  bool    dicom_hdr_voxelscale_extract(FILE *fp, mdm_Image3D &im);
  bool    dicom_hdr_voxelscale_extract_CT(FILE *fp, mdm_Image3D &im);
  bool    dicom_hdr_image_time_extract_GSKGE(FILE * fp, mdm_Image3D &im);
  bool    dicom_hdr_image_time_extract_GSKS(FILE * fp, mdm_Image3D &im);
  mdm_Image3D dicom_read_image(const std::string &pathname, int file_type);
  mdm_Image3D dicom_read_multiformat_image(const std::string &pathname, FILE *fp_img, int file_type);
  
  //writing
  static mdm_Image3D & mdm_DicomFormat::imrect_to_ushort(mdm_Image3D &imrect1);

  static bool mdm_DicomFormat::dicom_write_preamble(FILE * stream, char *err_msg);
  static bool mdm_DicomFormat::dicom_write_att(DCM_TAG tag, char *vr, unsigned int vl, void *vf, FILE *stream);

  static bool mdm_DicomFormat::dicom_write_header(FILE * stream, mdm_Image3D &imrect, char *err_msg);

  static bool dicom_write_pixeldata(FILE * stream, mdm_Image3D &imrect, const std::string & pathname);

  static bool dicom_write_image(mdm_Image3D &imrect, const std::string &pathname);

  //Aux functions
  static void set_dicom_format(int format);

  static int dblock_vr_conv(unsigned int *nbytes, FILE * fp);

  static bool dicom_part10_endian_swap(FILE * fp);

  static int dicom_preread_tests(FILE *fp);

  static mdm_Image3D & im_dicom_conv(mdm_Image3D &im);

  static int dicom_format;
};

#endif /* MDM_DICOMFORMAT_H */
