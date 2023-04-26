/**
 *  @file    mdm_NiftiFormat.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_NiftiFormat.h"
#include "nifti_swaps.h"

#include <iostream>
#include <cstdio>
#include <cassert>
#include <iomanip>
#include <string>
#include <sstream>
#include <istream>
#include <ostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>
#include <madym/utils/mdm_platform_defs.h>
#include <madym/image_io/meta/mdm_BIDSFormat.h>

#include <mdm_version.h>

namespace fs = boost::filesystem;

//Variable constants
const std::string mdm_NiftiFormat::extnii = ".nii";   /* modifiable, for possible uppercase */
const std::string mdm_NiftiFormat::exthdr = ".hdr";
const std::string mdm_NiftiFormat::extimg = ".img";
const std::string mdm_NiftiFormat::extnia = ".nia";
const std::string mdm_NiftiFormat::extgz = ".gz";

//
MDM_API mdm_Image3D mdm_NiftiFormat::readImage3D(const std::string &fileName,
	bool loadXtr, bool applyScaling)
{
  if (fileName.empty())
    throw mdm_exception(__func__, "Filename image must not be empty");

  //Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName,
    baseName, ext, gz);

  //Try and read the main NIFTI image file
  mdm_Image3D img;

  nifti_image nii = nifti_image_read(fileName, 1);
  if (!nii.data)
    throw mdm_exception(__func__,  "Error reading " + fileName);

  //Try loading the XTR file first as this may contain info 
  // on axes flipping we need
  //to convert NIFTI transform matrices correctly
  if (loadXtr)
  {
    if (fs::exists(baseName + ".json"))
      mdm_BIDSFormat::readImageJSON(baseName + ".json", img);
    
    else if (fs::exists(baseName + ".xtr"))
      mdm_XtrFormat::readAnalyzeXtr(baseName + ".xtr", img);

    else
      throw mdm_exception(__func__, "No xtr or json file matching " + fileName);
  }

  // Store the voxel matrix dimensions
  int nX = nii.dim[1];
  int nY = nii.dim[2];
  int nZ = nii.dim[3];

  if (nX <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nX = %2%, should be strictly positive")
      % fileName % nX);

  if (nY <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nY = %2%, should be strictly positive")
      % fileName % nY);

  if (nZ <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nX = %2%, should be strictly positive")
      % fileName % nZ);

  if (nii.dim[4] > 1)
    throw mdm_exception(__func__, baseName + " is 4D. We can only use 2D or 3D images");

  img.setDimensions(nX, nY, nZ);

  if (!img)
    throw mdm_exception(__func__, "Can't allocate voxel array for image " + fileName);

  //Store the voxel mm dimensions
  double xmm = nii.pixdim[1];
  double ymm = nii.pixdim[2];
  double zmm = nii.pixdim[3];
  if (xmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, xmm = %2%, should be strictly positive")
      % fileName % xmm);

  if (ymm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, ymm = %2%, should be strictly positive")
      % fileName % ymm);

  if (zmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, zmm = %2%, should be strictly positive")
      % fileName % zmm);

  img.setVoxelDims(xmm, ymm, zmm);

  //Set the voxel grid axes from the NIFTI sform matrix
  nifti_nii_transform_to_img(nii, img);

  //Copy the data
  // ------------------------------------------------------------------------
  switch (nii.datatype)
  {
    case NIFTI_TYPE_UINT8: {
      fromData<uint8_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_UINT16:  {
      fromData<uint16_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_UINT32:  { 
      fromData<uint32_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_UINT64:  { 
      fromData<uint64_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_INT8:  { 
      fromData<int8_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_INT16:  { 
      fromData<int16_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_INT32:  { 
      fromData<int32_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_INT64:  { 
      fromData<int64_t>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_FLOAT32:  { 
      fromData<float>(nii, img, applyScaling);
      break;
    }
    case NIFTI_TYPE_FLOAT64:  { 
      fromData<double>(nii, img, applyScaling);
      break;
    }
    default: {
      throw mdm_exception(__func__, boost::format(
        "Error reading %1%, datatype = %2% not recognised")
        % fileName % nii.datatype);
    }
  }

  //Can now free the nifti image structure
  nifti_image_free(nii);

	return img;
}

MDM_API std::vector<mdm_Image3D> mdm_NiftiFormat::readImage4D(const std::string& fileName,
  bool loadXtr, bool applyScaling)
{
  if (fileName.empty())
    throw mdm_exception(__func__, "Filename image must not be empty");

  //Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName,
    baseName, ext, gz);

  nifti_image nii = nifti_image_read(fileName, 1);
  if (!nii.data)
    throw mdm_exception(__func__, "Error reading " + fileName);

  // Store the voxel matrix dimensions
  int nX = nii.dim[1];
  int nY = nii.dim[2];
  int nZ = nii.dim[3];
  int nImages = nii.dim[4];

  if (nX <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nX = %2%, should be strictly positive")
      % fileName % nX);

  if (nY <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nY = %2%, should be strictly positive")
      % fileName % nY);

  if (nZ <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nZ = %2%, should be strictly positive")
      % fileName % nZ);

  if (nZ <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nImages = %2%, should be strictly positive")
      % fileName % nImages);

  //Try and read the main NIFTI image file
  std::vector<mdm_Image3D> imgs(nImages);

  //Store the voxel mm dimensions
  double xmm = nii.pixdim[1];
  double ymm = nii.pixdim[2];
  double zmm = nii.pixdim[3];
  if (xmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, xmm = %2%, should be strictly positive")
      % fileName % xmm);

  if (ymm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, ymm = %2%, should be strictly positive")
      % fileName % ymm);

  if (zmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, zmm = %2%, should be strictly positive")
      % fileName % zmm);

  //Try loading the XTR file first as this may contain info 
  // on axes flipping we need
  //to convert NIFTI transform matrices correctly
  if (loadXtr)
  {
    if (fs::exists(baseName + ".json"))
      mdm_BIDSFormat::readImageJSON(baseName + ".json", imgs);

    else if (fs::exists(baseName + ".xtr"))
      mdm_XtrFormat::readAnalyzeXtr(baseName + ".xtr", imgs);

    else
      throw mdm_exception(__func__, "No xtr or json file matching " + fileName);
  }

  for (auto& img : imgs)
  {
    //Set dimensions of each image
    img.setDimensions(nX, nY, nZ);
    img.setVoxelDims(xmm, ymm, zmm);

    //Set the voxel grid axes from the NIFTI sform matrix
    nifti_nii_transform_to_img(nii, img);
  }

  //Copy the data
  // ------------------------------------------------------------------------
  switch (nii.datatype)
  {
  case NIFTI_TYPE_UINT8: {
    fromData<uint8_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_UINT16: {
    fromData<uint16_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_UINT32: {
    fromData<uint32_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_UINT64: {
    fromData<uint64_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_INT8: {
    fromData<int8_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_INT16: {
    fromData<int16_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_INT32: {
    fromData<int32_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_INT64: {
    fromData<int64_t>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_FLOAT32: {
    fromData<float>(nii, imgs, applyScaling);
    break;
  }
  case NIFTI_TYPE_FLOAT64: {
    fromData<double>(nii, imgs, applyScaling);
    break;
  }
  default: {
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, datatype = %2% not recognised")
      % fileName % nii.datatype);
  }
  }

  //Can now free the nifti image structure
  nifti_image_free(nii);

  return imgs;
}

//
MDM_API void mdm_NiftiFormat::writeImage3D(const std::string &fileName,
	const mdm_Image3D &img,
	const mdm_ImageDatatypes::DataType dataTypeFlag, 
  const mdm_XtrFormat::XTR_type xtrTypeFlag,
	bool compress, bool applyScaling)
{
  if (!img)
    throw mdm_exception(__func__, "Image for writing must not be empty");

  //Check name is valid
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName, baseName, ext, gz);

  //What to do if gz true but compress false? Let gz override...
  if (gz && !compress)
    compress = true;

  //Create new NIFTI image instance
  nifti_image nii;

  //Set dimesnions fields
  size_t nx, ny, nz;
  img.getDimensions(nx, ny, nz);

  nii.nx = nx;
  nii.ny = ny;
  nii.nz = nz;
  nii.nvox = nx * ny * nz;
  nii.nt = 1;
  nii.dx = img.info().Xmm.value();
  nii.dy = img.info().Ymm.value();
  nii.dz = img.info().Zmm.value();
  nii.scl_slope = 1.0;
  nii.scl_inter = 0.0;

  //Set datatype
  nii.nifti_type = NIFTI_FTYPE::NIFTI1_1;
  nii.datatype = dataTypeFlag;
  std::string descrip("Madym-");
  descrip.append(MDM_VERSION);
  for (size_t s = 0; s < descrip.size(); s++)
    nii.descrip[s] = descrip[s];
  for (size_t s = descrip.size(); s < sizeof(nii.descrip); s++)
    nii.descrip[s] = '\0';
  nii.aux_file[0] = '\0';

  //Set transform matrix
  nifti_img_to_nii_transform(img, nii);

  //Apply scaling if set
  if (applyScaling && img.info().sclSlope.isSet() && img.info().sclInter.isSet())
  {
    nii.scl_slope = img.info().sclSlope.value();
    nii.scl_inter = img.info().sclInter.value();
  }

  //Set data field
  // ------------------------------------------------------------------------
  switch (nii.datatype)
  {
  case NIFTI_TYPE_UINT8: {
    toData<uint8_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_UINT16: {
    toData<uint16_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_UINT32: {
    toData<uint32_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_UINT64: {
    toData<uint64_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_INT8: {
    toData<int8_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_INT16: {
    toData<int16_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_INT32: {
    toData<int32_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_INT64: {
    toData<int64_t>(img, nii);
    break;
  }
  case NIFTI_TYPE_FLOAT32: {
    toData<float>(img, nii);
    break;
  }
  case NIFTI_TYPE_FLOAT64: {
    toData<double>(img, nii);
    break;
  }
  default: {
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, datatype = %2% not recognised")
      % fileName % nii.datatype);
  }
  }

  //Write out as NIFTI
  std::string saveName = baseName + ".nii";
  if (compress)
    saveName += extgz;

  nifti_set_filenames(nii, saveName, 0, 0);
  nifti_image_write(nii);

  //Free NIFTI struct
  nifti_image_free(nii);
  
  // Write *.xtr file
  if (xtrTypeFlag != mdm_XtrFormat::NO_XTR)
  {
    if (xtrTypeFlag == mdm_XtrFormat::BIDS)
      mdm_BIDSFormat::writeImageJSON(baseName, img);
    else
      mdm_XtrFormat::writeAnalyzeXtr(baseName, img, xtrTypeFlag);
  }
    
}

//
MDM_API void mdm_NiftiFormat::writeImage4D(const std::string& fileName,
  const std::vector<mdm_Image3D> imgs,
  const mdm_ImageDatatypes::DataType dataTypeFlag,
  const mdm_XtrFormat::XTR_type xtrTypeFlag,
  bool compress, bool applyScaling)
{
  if (imgs.empty())
    throw mdm_exception(__func__, "Images for writing image must not be empty");

  //Check name is valid
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName, baseName, ext, gz);

  //What to do if gz true but compress false? Let gz override...
  if (gz && !compress)
    compress = true;

  //Create new NIFTI image instance
  nifti_image nii;

  //Take alias to first image to access various bits of header info
  const auto& img = imgs[0];

  //Set dimesnions fields
  size_t nx, ny, nz;
  img.getDimensions(nx, ny, nz);

  nii.nx = nx;
  nii.ny = ny;
  nii.nz = nz;
  nii.nt = imgs.size();
  nii.nvox = nx * ny * nz * nii.nt;
  nii.dx = img.info().Xmm.value();
  nii.dy = img.info().Ymm.value();
  nii.dz = img.info().Zmm.value();
  if (nii.nt > 1)
  {
    auto n = nii.nt - 1;
    nii.dt = (imgs[n].secondsFromTimeStamp() - imgs[0].secondsFromTimeStamp())/n;
  }
  else
  {
    nii.dt = 0;
    nii.time_units = 0;
  }
    
  
  nii.scl_slope = 1.0;
  nii.scl_inter = 0.0;

  //Set datatype
  nii.nifti_type = NIFTI_FTYPE::NIFTI1_1;
  nii.datatype = dataTypeFlag;
  std::string descrip("Madym-");
  descrip.append(MDM_VERSION);
  for (size_t s = 0; s < descrip.size(); s++)
    nii.descrip[s] = descrip[s];
  for (size_t s = descrip.size(); s < sizeof(nii.descrip); s++)
    nii.descrip[s] = '\0';
  nii.aux_file[0] = '\0';

  //Set transform matrix
  nifti_img_to_nii_transform(img, nii);

  //Apply scaling if set
  if (applyScaling && img.info().sclSlope.isSet() && img.info().sclInter.isSet())
  {
    nii.scl_slope = img.info().sclSlope.value();
    nii.scl_inter = img.info().sclInter.value();
  }

  //Set data field
  // ------------------------------------------------------------------------
  switch (nii.datatype)
  {
  case NIFTI_TYPE_UINT8: {
    toData<uint8_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_UINT16: {
    toData<uint16_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_UINT32: {
    toData<uint32_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_UINT64: {
    toData<uint64_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_INT8: {
    toData<int8_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_INT16: {
    toData<int16_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_INT32: {
    toData<int32_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_INT64: {
    toData<int64_t>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_FLOAT32: {
    toData<float>(imgs, nii);
    break;
  }
  case NIFTI_TYPE_FLOAT64: {
    toData<double>(imgs, nii);
    break;
  }
  default: {
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, datatype = %2% not recognised")
      % fileName % nii.datatype);
  }
  }

  //Write out as NIFTI
  std::string saveName = baseName + ".nii";
  if (compress)
    saveName += extgz;

  nifti_set_filenames(nii, saveName, 0, 0);
  nifti_image_write(nii);

  //Free NIFTI struct
  nifti_image_free(nii);

  // Write *.xtr file
  if (xtrTypeFlag != mdm_XtrFormat::NO_XTR)
  {
    if (xtrTypeFlag == mdm_XtrFormat::BIDS)
      mdm_BIDSFormat::writeImageJSON(baseName, imgs);
    else
      mdm_XtrFormat::writeAnalyzeXtr(baseName, img, xtrTypeFlag);
  }
}

//
MDM_API bool mdm_NiftiFormat::filesExist(const std::string & fileName,
  bool warn)
{
  //Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName,
    baseName, ext, gz);

  bool filesExist = false;

  if (ext.empty())
  {
    filesExist =
      (fs::exists(baseName + extimg) && fs::exists(baseName + exthdr)) ||
      (fs::exists(baseName + extimg + extgz) && fs::exists(baseName + exthdr + extgz)) ||
      fs::exists(baseName + extnii) ||
      fs::exists(baseName + extnii + extgz) ||
      fs::exists(baseName + extnia);
  }
  else if (ext == extnii || ext == extnia)
  {
    if (gz)
      filesExist = fs::exists(baseName + ext + extgz);
    else
      filesExist = fs::exists(baseName + ext);
  }
    

  else if (ext == extimg || ext == exthdr)
  {
    if (gz)
      filesExist = fs::exists(baseName + extimg + extgz) && fs::exists(baseName + exthdr + extgz);
    else
      filesExist = fs::exists(baseName + extimg) && fs::exists(baseName + exthdr);
  }

  return filesExist;
}

//**********************************************************************
//Private 
//**********************************************************************
//
//

//
template <class T> void mdm_NiftiFormat::fromData(const nifti_image &nii, mdm_Image3D &img, bool applyScaling)
{
  auto nVoxels = img.numVoxels();
  T* nii_data = static_cast<T*>(nii.data);
  auto slope = applyScaling && !std::isnan(nii.scl_slope) ? nii.scl_slope : 1.0;
  auto inter = applyScaling && !std::isnan(nii.scl_inter) ? nii.scl_inter : 0.0;
  for (size_t i = 0; i < nVoxels; ++i)
    img.setVoxel(i, static_cast<double>(*(nii_data + i))*slope + inter);
    
}

//
template <class T> void mdm_NiftiFormat::fromData(const nifti_image& nii, std::vector<mdm_Image3D>& imgs, bool applyScaling)
{
  auto nImages = imgs.size();
  size_t currImg = 0;
  T* nii_data = static_cast<T*>(nii.data);
  auto nVoxels = imgs[0].numVoxels();
  auto slope = applyScaling && !std::isnan(nii.scl_slope) ? nii.scl_slope : 1.0;
  auto inter = applyScaling && !std::isnan(nii.scl_inter) ? nii.scl_inter : 0.0;

  for (auto& img : imgs)
  {
    auto offset = nVoxels * currImg;
    for (size_t i = 0; i < nVoxels; ++i)
      img.setVoxel(i, static_cast<double>(*(nii_data + offset + i))*slope + inter);

    currImg++;
  }
  
}

//
template <class T> void mdm_NiftiFormat::toData(const mdm_Image3D &img, nifti_image &nii)
{
  nii.nbyper = sizeof(T);
  nii.data = malloc(nii.nvox*nii.nbyper);
  auto slope = std::isnan(nii.scl_slope) ? 1.0 : nii.scl_slope;
  auto inter = std::isnan(nii.scl_inter) ? 1.0 : nii.scl_inter;
  T* nii_data = static_cast<T*>(nii.data);
  for (size_t i = 0; i < nii.nvox; ++i)
    *(nii_data + i) = (T)((img.voxel(i) - inter) / slope);
}

//
template <class T> void mdm_NiftiFormat::toData(const std::vector<mdm_Image3D>& imgs, nifti_image& nii)
{
  nii.nbyper = sizeof(T);
  nii.data = malloc(nii.nvox * nii.nbyper);
  auto slope = std::isnan(nii.scl_slope) ? 1.0 : nii.scl_slope;
  auto inter = std::isnan(nii.scl_inter) ? 1.0 : nii.scl_inter;
  auto nVoxels = imgs[0].numVoxels();
  T* nii_data = static_cast<T*>(nii.data);
  
  size_t currImg = 0;
  for (const auto& img : imgs)
  {
    auto offset = nVoxels * currImg;
    for (size_t i = 0; i < nVoxels; ++i)
      *(nii_data + offset + i) = (T)((img.voxel(i) - inter) / slope);

    currImg++;
  }
}

//
mdm_NiftiFormat::nifti_image mdm_NiftiFormat::nifti_image_read(const std::string &fileName, int read_data)
{

  // determine filename to use for header, will throw exception if no valid header found
  auto hdrName = nifti_findhdrname(fileName);
  
  bool gz = nifti_is_gzfile(hdrName);
  int64_t filesize;
  if (gz) 
    filesize = -1;  /* unknown */
  else
    filesize = nifti_get_filesize(hdrName);

  /**- open file, separate reading of header, extensions and data */
  znzFile fp = znzopen(hdrName.c_str(), "rb", gz);
  if (znz_isnull(fp))
    throw mdm_exception(__func__, "failed to open header file" + hdrName); 

  /**- first try to read dataset as ASCII (and return if so) */
  nifti_image nim;
  int rv = has_ascii_header(fp);
  if (rv < 0) {
    znzclose(fp);
    throw mdm_exception(__func__, "short header read " + hdrName);
  }
  else if (rv == 1) { /* process special file type */
    nim = nifti_read_ascii_image(fp, hdrName, filesize, read_data);
    znzclose(fp);
    return nim;
  }

  int64_t h1size = sizeof(nifti_1_header);
  int64_t h2size = sizeof(nifti_2_header);

  nifti_1_header n1hdr;

  // next read into nifti_1_header and determine nifti type
  int ii = (int)znzread(&n1hdr, 1, h1size, fp);

  if (ii < (int)h1size) {      /* failure? */
    znzclose(fp);
    throw mdm_exception(__func__, "bad binary header read for file" + hdrName);
  }

  // find out what type of header we have
  int ni_ver = nifti_header_version((char *)&n1hdr, h1size);

  int onefile = 0;
  int64_t remain;
  nifti_2_header n2hdr;

  if (ni_ver == 0 || ni_ver == 1) 
  {
    nim = nifti_convert_n1hdr2nim(n1hdr, hdrName);
    onefile = NIFTI_ONEFILE(n1hdr);
  }
  else if (ni_ver == 2) 
  {
    /* fill nifti-2 header and convert */
    memcpy(&n2hdr, &n1hdr, h1size);   /* copy first part */
    remain = h2size - h1size;
    char *posn = (char *)&n2hdr + h1size;
    ii = (int)znzread(posn, 1, remain, fp); /* read remaining part */
    if (ii < (int)remain) 
    {
      znzclose(fp);
      throw mdm_exception(__func__, "short NIFTI-2 header read for file" + hdrName);
    }
    nim = nifti_convert_n2hdr2nim(n2hdr, hdrName);
    onefile = NIFTI_ONEFILE(n2hdr);
  }
  else {
    znzclose(fp);
    throw mdm_exception(__func__, hdrName + ":bad nifti im header version " + std::to_string(ni_ver));
  }

  /*Chris Rorden 2020, fslmaths does not load complex data, so lets maintain incompatibility*/
  if (nim.datatype == NIFTI_TYPE_COMPLEX64) 
  {
    throw mdm_exception(__func__, "Unsupported datatype (COMPLEX64): unable to load " + hdrName);
  }
  /*end CR */

  // check for extensions (any errors here means no extensions)
  if (onefile)     
    remain = nim.iname_offset;
  else               
    remain = filesize;

  if (ni_ver <= 1) 
    remain -= h1size;
  else               
    remain -= h2size;

  (void)nifti_read_extensions(nim, fp, remain);

  znzclose(fp); // close the file

  // read the data if desired, then bug out
  if (read_data) {
    if (nifti_image_load(nim) < 0) {
      nifti_image_free(nim); // take ball, go home.
      throw mdm_exception(__func__, "unable to load image data " + hdrName);
    }
  }
  else nim.data = NULL;

  return nim;
}

/*--------------------------------------------------------------------------*/
/*! Write a nifti_image to disk.

   Since data is properly byte-swapped upon reading, it is assumed
   to be in the byte-order of the current CPU at write time.  Thus,
   nim.byte_order should match that of the current CPU.  Note that
   the nifti_set_filenames() function takes the flag, set_byte_order.

   The following fields of nim affect how the output appears:
    - nifti_type = 0 ==> ANALYZE-7.5 format file pair will be written
    - nifti_type = 1 ==> NIFTI-1 format single file will be written
                         (data offset will be 352+extensions)
    - nifti_type = 2 ==> NIFTI_1 format file pair will be written
    - nifti_type = 3 ==> NIFTI_1 ASCII single file will be written
    - fname is the name of the output file (header or header+data)
    - if a file pair is being written, iname is the name of the data file
    - existing files WILL be overwritten with extreme prejudice
    - if qform_code > 0, the quatern_*, qoffset_*, and qfac fields determine
      the qform output, NOT the qto_xyz matrix; if you want to compute these
      fields from the qto_xyz matrix, you can use the utility function
      nifti_mat44_to_quatern()

   \sa nifti_image_write_bricks, nifti_image_free, nifti_set_filenames,
       nifti_image_write_hdr_img
*//*------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_image_write(nifti_image &nim)
{
  znzFile fp = nifti_image_write_hdr_img(nim, 1, "wb");
  if (fp) 
    free(fp);
}

/* ----------------------------------------------------------------------*/
/*! This writes the header (and optionally the image data) to file
 *
 * If the image data file is left open it returns a valid znzFile handle.
 * It also uses imgfile as the open image file is not null, and modifies
 * it inside.
 *
 * \param nim        nifti_image to write to disk
 * \param write_opts flags whether to write data and/or close file (see below)
 * \param opts       file-open options, probably "wb" from nifti_image_write()
 * \param imgfile    optional open znzFile struct, for writing image data
                     (may be NULL)
 * \param NBL        optional nifti_brick_list, containing the image data
                     (may be NULL)
 *
 * Values for write_opts mode are based on two binary flags
 * ( 0/1 for no-write/write data, and 0/2 for close/leave-open files ) :
 *    -   0 = do not write data and close (do not open data file)
 *    -   1 = write data        and close
 *    -   2 = do not write data and leave data file open
 *    -   3 = write data        and leave data file open
 *
 * \sa nifti_image_write, nifti_image_write_hdr_img, nifti_image_free,
 *     nifti_set_filenames
*//*---------------------------------------------------------------------*/
znzFile mdm_NiftiFormat::nifti_image_write_hdr_img(nifti_image &nim, int write_opts,
  const char * opts)
{
  nifti_1_header n1hdr;
  nifti_2_header n2hdr;
  znzFile        fp = NULL;
  int64_t        ss;
  int            write_data, leave_open;
  int            nver = 1, hsize = (int)sizeof(nifti_1_header);  /* 5 Aug 2015 */

  write_data = write_opts & 1;  /* just separate the bits now */
  leave_open = write_opts & 2;

  if (write_data && !nim.data)
    throw mdm_exception(__func__, "asked to write image data, no image data");

  nifti_set_iname_offset(nim, 1);

  if (nim.nifti_type == NIFTI_FTYPE::ASCII)   /* non-standard case */
    return nifti_write_ascii_image(nim, opts, write_data, leave_open);

  if ((nim.nifti_type == NIFTI_FTYPE::NIFTI2_1) || (nim.nifti_type == NIFTI_FTYPE::NIFTI2_2)) 
  {
    nifti_set_iname_offset(nim, 2);
    if (nifti_convert_nim2n2hdr(nim, n2hdr)) 
      return NULL;
    
    nver = 2;
    hsize = (int)sizeof(nifti_2_header);
  }
  else
    
    if (nifti_convert_nim2n1hdr(nim, n1hdr)) 
    {
      nifti_set_iname_offset(nim, 2);

      if (nifti_convert_nim2n2hdr(nim, n2hdr)) 
        return NULL;

      mdm_ProgramLogger::logProgramWarning(__func__, 
        nim.fname + ": writing as NIFTI-2, instead of NIFTI-1");
      nver = 2; /* we will write NIFTI-2 */
      hsize = (int)sizeof(nifti_2_header);
    }

  /* if writing to 2 files, make sure iname is set and different from fname */
  if (nim.nifti_type != NIFTI_FTYPE::NIFTI1_1) {
    if (!nim.iname.empty() && nim.iname == nim.fname)
      nim.iname = "";
    
    if (nim.iname.empty()) /* then make a new one */
      nim.iname = nifti_makeimgname(nim.fname, nim.nifti_type, 0, 0);

  }

  fp = znzopen(nim.fname.c_str(), opts, nifti_is_gzfile(nim.fname));
  if (znz_isnull(fp))
    throw mdm_exception(__func__, "cannot open output file" + nim.fname);

  /* write the header and extensions */
  if (nver == 2) ss = znzwrite(&n2hdr, 1, hsize, fp); /* write header */
  else            ss = znzwrite(&n1hdr, 1, hsize, fp); /* write header */

  if (ss < hsize) {
    znzclose(fp);
    throw mdm_exception(__func__, "bad header write to output file" + nim.fname);
  }

  /* partial file exists, and errors have been printed, so ignore return */
  if (nim.nifti_type != NIFTI_FTYPE::ANALYZE)
    (void)nifti_write_extensions(fp, nim);

  /* if the header is all we want, we are done */
  if (!write_data && !leave_open)
  {
    znzclose(fp);
    return(fp);
  }
  
  if ((nim.nifti_type != NIFTI_FTYPE::NIFTI1_1) && (nim.nifti_type != NIFTI_FTYPE::NIFTI2_1)) 
  { /* get a new file pointer */
    znzclose(fp);         /* first, close header file */
    
    fp = znzopen(nim.iname.c_str(), opts, nifti_is_gzfile(nim.iname));
    if (znz_isnull(fp)) 
      throw mdm_exception(__func__, "cannot open image file");
    
  }

  znzseek(fp, nim.iname_offset, SEEK_SET);  /* in any case, seek to offset */

  if (write_data) 
    nifti_write_all_data(fp, nim);

  if (!leave_open) 
    znzclose(fp);

  return fp;
}

/*--------------------------------------------------------------------------*/
/*! check whether the given type is on the "approved" list

    The code is valid if it is non-negative, and does not exceed
    NIFTI_MAX_FTYPE.

    \return 1 if nifti_type is valid, 0 otherwise
    \sa NIFTI_FTYPE
*//*------------------------------------------------------------------------*/
int mdm_NiftiFormat::is_valid_nifti_type(int nifti_type)
{
  if (nifti_type >= NIFTI_FTYPE::ANALYZE &&   /* smallest type, 0 */
    nifti_type <= NIFTI_FTYPE::MAX_FTYPE)
    return 1;
  return 0;
}

//
/*----------------------------------------------------------------------*/
/*! check current directory for existing header file

    \return filename of header on success and NULL if no appropriate file
            could be found

    If fname has an uppercase extension, check for uppercase files.

    NB: it allocates memory for hdrname which should be freed
        when no longer required
*//*-------------------------------------------------------------------*/
std::string mdm_NiftiFormat::nifti_findhdrname(const std::string & fileName)
{
  //Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName,
    baseName, ext, gz);

  //Try to fill header name
  std::string hdrName;

  if (ext.empty())
  {
    /* if basename, look for .nii, .nii.gz, .hdr, .hdr.gz, in that order */
    for (const std::string ex : {extnii, extnii+extgz, exthdr, exthdr+extgz })
    {
      if (fs::exists(baseName + ex))
      {
        hdrName = baseName + ex;
        break;
      }
    }
  }
  else if (boost::iequals(ext, extimg))
  {
    /* if .img, look for .hdr, .hdr.gz, .nii, .nii.gz, in that order */
    for (const std::string ex : {exthdr, extnii })
    {
      std::string checkName = gz ? baseName + ex + extgz : baseName + ex;
      if (fs::exists(checkName))
      {
        hdrName = checkName;
        break;
      }
    }
  }
  else
  {
    //File is a valid header, check it exists
    std::string checkName = gz ? baseName + ext + extgz : baseName + ext;
    if (fs::exists(checkName))
      hdrName = checkName;
  }

  //If header name is still empty, we didn't find a valid match, so throw error
  if (hdrName.empty())
    throw mdm_exception(__func__, " could not find NIFTI header file for " + baseName);

  //Otherwise return header name
  return hdrName;

}

/*------------------------------------------------------------------------*/
/*! check current directory for existing image file

    \param fname filename to check for
    \nifti_type  nifti_type for dataset - this determines whether to
                 first check for ".nii" or ".img" (since both may exist)

    \return filename of data/img file on success and NULL if no appropriate
            file could be found

    If fname has a valid, uppercase extension, apply all extensions as
    uppercase.

    NB: it allocates memory for the image filename, which should be freed
        when no longer required
*//*---------------------------------------------------------------------*/
std::string mdm_NiftiFormat::nifti_findimgname(const std::string &fileName, int nifti_type)
{
  // Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName, baseName, ext, gz);

  //Try to fill header name
  std::string imgName;

  if (ext.empty())
  {
    /* only valid extension for ASCII type is .nia, handle first */
    if (nifti_type == NIFTI_FTYPE::ASCII && fs::exists(baseName + extnia))
      imgName = baseName + extnia;

    else {

      /**- test for .nii and .img (don't assume input type from image type) */
      /**- if nifti_type = 1, check for .nii first, else .img first         */
      std::vector<std::string> elist = { extnii, extimg };

      if (nifti_type == NIFTI_FTYPE::NIFTI1_1 || nifti_type == NIFTI_FTYPE::NIFTI2_1)
        elist = { extnii, extimg }; /* should match .nii */
      else
        elist = { extimg, extnii }; /* should match .img */

      for (auto ex : elist)
      {
        std::string checkName = gz ? baseName + ex + extgz : baseName + ex;
        if (fs::exists(checkName))
        {
          imgName = checkName;
          break;
        }
          
      }
    }
  }
  else if (boost::iequals(ext, exthdr)) //is .hdr
  {
    /* if .hdr, look for .img, .img.gz */
    std::string checkName = gz ? baseName + extimg + extgz : baseName + extimg;
    if (fs::exists(checkName))
      imgName = checkName;
    
  }
  else //is .img, .nii or .nia
  {
    //File is a valid header, check it exists
    std::string checkName = gz ? baseName + ext + extgz : baseName + ext;
    if (fs::exists(checkName))
      imgName = checkName;
  }

  
  //If header name is still empty, we didn't find a valid match, so throw error
  if (imgName.empty())
    throw mdm_exception(__func__, " could not find NIFTI image file for " + baseName);

  //Otherwise return header name
  return imgName;
}

//
void mdm_NiftiFormat::parseName(const std::string &fileName, 
  std::string &baseName, std::string &ext, bool &gz)
{
  fs::path p(fileName);
  auto ext_p = p.extension();


  //Check if extension is gz
  gz = false;
  if (boost::iequals(ext_p.string(), extgz))
  {
    gz = true;
    ext_p = p.stem().extension();
    p.replace_extension();
  }
    
  //Get stem and extension
  baseName = p.replace_extension().string();
  ext = ext_p.string();

  //Check extension valid
  bool extValid;
  if (
    boost::iequals(ext, extnii) ||
    boost::iequals(ext, exthdr) ||
    boost::iequals(ext, extimg) ||
    boost::iequals(ext, extnia))
    extValid = true;
  else
    extValid = false;

  if ((!ext.empty() || gz) && !extValid)
    throw mdm_exception(__func__, fileName + "has invalid file extension for NIFTI format");

  if (p.stem().empty())
    throw mdm_exception(__func__, fileName + "is invalid, basename is empty");

#ifndef HAVE_ZLIB
  //If we haven't built with zlib, gz extensions can't be opened
  if (gz)
    throw mdm_exception(__func__, fileName + "has extension gz, but this version of Madym has been built without zlib support");
#endif
}

/*---------------------------------------------------------------------------*/
/*! return the size of a file, in bytes

    \return size of file on success, -1 on error or no file

    changed to return int, -1 means no file or error      20 Dec 2004 [rickr]
*//*-------------------------------------------------------------------------*/
int64_t mdm_NiftiFormat::nifti_get_filesize(const std::string &pathname)
{
  struct stat buf;

  if (stat(pathname.c_str(), &buf) != 0)
    return -1;

  return buf.st_size;
}

/*--------------------------------------------------------------------------*/
/*! Given a datatype code, set number of bytes per voxel and the swapsize.

    \param datatype nifti1 datatype code
    \param nbyper   pointer to return value: number of bytes per voxel
    \param swapsize pointer to return value: size of swap blocks

    \return appropriate values at nbyper and swapsize

    The swapsize is set to 0 if this datatype doesn't ever need swapping.

    \sa NIFTI1_DATATYPES in nifti1.h
*//*------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_datatype_sizes(int datatype, int &nbyper, int &swapsize)
{
  switch (datatype) {
  case mdm_ImageDatatypes::DT_INT8:
  case mdm_ImageDatatypes::DT_UINT8:       nbyper = 1; swapsize = 0; break;

  case mdm_ImageDatatypes::DT_INT16:
  case mdm_ImageDatatypes::DT_UINT16:      nbyper = 2; swapsize = 2; break;

  case mdm_ImageDatatypes::DT_RGB24:       nbyper = 3; swapsize = 0; break;
  case mdm_ImageDatatypes::DT_RGBA32:      nbyper = 4; swapsize = 0; break;

  case mdm_ImageDatatypes::DT_INT32:
  case mdm_ImageDatatypes::DT_UINT32:
  case mdm_ImageDatatypes::DT_FLOAT32:     nbyper = 4; swapsize = 4; break;

  case mdm_ImageDatatypes::DT_COMPLEX64:   nbyper = 8; swapsize = 4; break;

  case mdm_ImageDatatypes::DT_FLOAT64:
  case mdm_ImageDatatypes::DT_INT64:
  case mdm_ImageDatatypes::DT_UINT64:      nbyper = 8; swapsize = 8; break;

  case mdm_ImageDatatypes::DT_FLOAT128:    nbyper = 16; swapsize = 16; break;

  case mdm_ImageDatatypes::DT_COMPLEX128:  nbyper = 16; swapsize = 8; break;

  case mdm_ImageDatatypes::DT_COMPLEX256:  nbyper = 32; swapsize = 16; break;
  }
}

#undef  REVERSE_ORDER
#define REVERSE_ORDER(x) (3-(x))    /* convert MSB_FIRST <--> LSB_FIRST */

/*----------------------------------------------------------------------*/
/*! convert a nifti_1_header into a nift1_image

   \return an allocated nifti_image, or NULL on failure
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_image mdm_NiftiFormat::nifti_convert_n1hdr2nim(nifti_1_header nhdr, const std::string &fname)
{
  int   ii, doswap, ioff;
  int   ni_ver, is_onefile;
  nifti_image nim;

  /* be explicit with pointers */
  nim.data = NULL;

  /**- check if we must swap bytes */
  doswap = need_nhdr_swap(nhdr.dim[0], nhdr.sizeof_hdr); /* swap data flag */

  if (doswap < 0) {
    if (doswap == -1) 
      throw mdm_exception(__func__, "bad dim[0]");

    throw mdm_exception(__func__, "bad sizeof_hdr");
  }

  /**- determine if this is a NIFTI-1 compliant header */

  ni_ver = NIFTI_VERSION(nhdr);
  /*
   * before swapping header, record the Analyze75 orient code
   */
  if (ni_ver == 0)
  {
    /**- in analyze75, the orient code is at the same address as
     *   qform_code, but it's just one byte
     *   the qform_code will be zero, at which point you can check
     *   analyze75_orient if you care to.
     */
    unsigned char c = *((char *)(&nhdr.qform_code));
    nim.analyze75_orient = (analyze_75_orient_code)c;
  }
  if (doswap) 
    swap_nifti_header(&nhdr, ni_ver);

  if (nhdr.datatype == mdm_ImageDatatypes::DT_BINARY || nhdr.datatype == mdm_ImageDatatypes::DT_UNKNOWN)
    throw mdm_exception(__func__, "bad datatype");

  if (nhdr.dim[1] <= 0)
    throw mdm_exception(__func__, "bad dim[1]");

  /* fix bad dim[] values in the defined dimension range */
  for (ii = 2; ii <= nhdr.dim[0]; ii++)
    if (nhdr.dim[ii] <= 0) nhdr.dim[ii] = 1;

  /* fix any remaining bad dim[] values, so garbage does not propagate */
  /* (only values 0 or 1 seem rational, otherwise set to arbirary 1)   */
  for (ii = nhdr.dim[0] + 1; ii <= 7; ii++)
    if (nhdr.dim[ii] != 1 && nhdr.dim[ii] != 0) nhdr.dim[ii] = 1;

#if 0  /* rely on dim[0], do not attempt to modify it   16 Nov 2005 [rickr] */

  /**- get number of dimensions (ignoring dim[0] now) */
  for (ii = 7; ii >= 2; ii--)            /* loop backwards until we  */
    if (nhdr.dim[ii] > 1) break;        /* find a dim bigger than 1 */
  ndim = ii;
#endif

  /**- set bad grid spacings to 1.0 */
  for (ii = 1; ii <= nhdr.dim[0]; ii++) 
  {
    if (nhdr.pixdim[ii] == 0.0) nhdr.pixdim[ii] = 1.0f;
  }

  is_onefile = (ni_ver > 0) && NIFTI_ONEFILE(nhdr);

  if (ni_ver) 
    nim.nifti_type = (is_onefile) ? NIFTI_FTYPE::NIFTI1_1 : NIFTI_FTYPE::NIFTI1_2;
  else         
    nim.nifti_type = NIFTI_FTYPE::ANALYZE;

  ii = nifti_short_order();
  if (doswap)   
    nim.byteorder = REVERSE_ORDER(ii);
  else           
    nim.byteorder = ii;


  /**- set dimensions of data array */
  nim.ndim = nim.dim[0] = nhdr.dim[0];
  nim.nx = nim.dim[1] = nhdr.dim[1];
  nim.ny = nim.dim[2] = nhdr.dim[2];
  nim.nz = nim.dim[3] = nhdr.dim[3];
  nim.nt = nim.dim[4] = nhdr.dim[4];
  nim.nu = nim.dim[5] = nhdr.dim[5];
  nim.nv = nim.dim[6] = nhdr.dim[6];
  nim.nw = nim.dim[7] = nhdr.dim[7];

  for (ii = 1, nim.nvox = 1; ii <= nhdr.dim[0]; ii++)
    nim.nvox *= nhdr.dim[ii];

  /**- set the type of data in voxels and how many bytes per voxel */

  nim.datatype = nhdr.datatype;

  nifti_datatype_sizes(nim.datatype, nim.nbyper, nim.swapsize);
  if (nim.nbyper == 0) 
    throw mdm_exception(__func__, "bad datatype");

  /**- set the grid spacings */

  nim.dx = nim.pixdim[1] = nhdr.pixdim[1];
  nim.dy = nim.pixdim[2] = nhdr.pixdim[2];
  nim.dz = nim.pixdim[3] = nhdr.pixdim[3];
  nim.dt = nim.pixdim[4] = nhdr.pixdim[4];
  nim.du = nim.pixdim[5] = nhdr.pixdim[5];
  nim.dv = nim.pixdim[6] = nhdr.pixdim[6];
  nim.dw = nim.pixdim[7] = nhdr.pixdim[7];

  /**- compute qto_xyz transformation from pixel indexes (i,j,k) to (x,y,z) */

  if (!ni_ver || nhdr.qform_code <= 0) {
    /**- if not nifti or qform_code <= 0, use grid spacing for qto_xyz */

    nim.qto_xyz.m[0][0] = nim.dx;  /* grid spacings */
    nim.qto_xyz.m[1][1] = nim.dy;  /* along diagonal */
    nim.qto_xyz.m[2][2] = nim.dz;

    /* off diagonal is zero */

    nim.qto_xyz.m[0][1] = nim.qto_xyz.m[0][2] = nim.qto_xyz.m[0][3] = 0.0f;
    nim.qto_xyz.m[1][0] = nim.qto_xyz.m[1][2] = nim.qto_xyz.m[1][3] = 0.0f;
    nim.qto_xyz.m[2][0] = nim.qto_xyz.m[2][1] = nim.qto_xyz.m[2][3] = 0.0f;

    /* last row is always [ 0 0 0 1 ] */

    nim.qto_xyz.m[3][0] = nim.qto_xyz.m[3][1] = nim.qto_xyz.m[3][2] = 0.0f;
    nim.qto_xyz.m[3][3] = 1.0f;

    nim.qform_code = NIFTI_XFORM_UNKNOWN;

  }
  else {
    /**- else NIFTI: use the quaternion-specified transformation */

    nim.quatern_b = nhdr.quatern_b;
    nim.quatern_c = nhdr.quatern_c;
    nim.quatern_d = nhdr.quatern_d;

    nim.qoffset_x = nhdr.qoffset_x;
    nim.qoffset_y = nhdr.qoffset_y;
    nim.qoffset_z = nhdr.qoffset_z;

    nim.qfac = (nhdr.pixdim[0] < 0.0) ? -1.0f : 1.0f;  /* left-handedness? */

    nim.qto_xyz = nifti_quatern_to_dmat44(
      nim.quatern_b, nim.quatern_c, nim.quatern_d,
      nim.qoffset_x, nim.qoffset_y, nim.qoffset_z,
      nim.dx, nim.dy, nim.dz,
      nim.qfac);

    nim.qform_code = nhdr.qform_code;
  }

  /**- load inverse transformation (x,y,z) -> (i,j,k) */
  nim.qto_ijk = nifti_dmat44_inverse(nim.qto_xyz);

  /**- load sto_xyz affine transformation, if present */
  if (!ni_ver || nhdr.sform_code <= 0)
    /**- if not nifti or sform_code <= 0, then no sto transformation */
    nim.sform_code = NIFTI_XFORM_UNKNOWN;

  else {
    /**- else set the sto transformation from srow_*[] */
    nim.sto_xyz.m[0][0] = nhdr.srow_x[0];
    nim.sto_xyz.m[0][1] = nhdr.srow_x[1];
    nim.sto_xyz.m[0][2] = nhdr.srow_x[2];
    nim.sto_xyz.m[0][3] = nhdr.srow_x[3];

    nim.sto_xyz.m[1][0] = nhdr.srow_y[0];
    nim.sto_xyz.m[1][1] = nhdr.srow_y[1];
    nim.sto_xyz.m[1][2] = nhdr.srow_y[2];
    nim.sto_xyz.m[1][3] = nhdr.srow_y[3];

    nim.sto_xyz.m[2][0] = nhdr.srow_z[0];
    nim.sto_xyz.m[2][1] = nhdr.srow_z[1];
    nim.sto_xyz.m[2][2] = nhdr.srow_z[2];
    nim.sto_xyz.m[2][3] = nhdr.srow_z[3];

    /* last row is always [ 0 0 0 1 ] */

    nim.sto_xyz.m[3][0] = nim.sto_xyz.m[3][1] = nim.sto_xyz.m[3][2] = 0.0f;
    nim.sto_xyz.m[3][3] = 1.0f;

    nim.sto_ijk = nifti_dmat44_inverse(nim.sto_xyz);

    nim.sform_code = nhdr.sform_code;
  }

  /**- set miscellaneous NIFTI stuff */
  if (ni_ver) {
    nim.scl_slope = nhdr.scl_slope;
    nim.scl_inter = nhdr.scl_inter;

    nim.intent_code = nhdr.intent_code;

    nim.intent_p1 = nhdr.intent_p1;
    nim.intent_p2 = nhdr.intent_p2;
    nim.intent_p3 = nhdr.intent_p3;

    nim.toffset = nhdr.toffset;

    memcpy(nim.intent_name, nhdr.intent_name, 15); nim.intent_name[15] = '\0';

    nim.xyz_units = XYZT_TO_SPACE(nhdr.xyzt_units);
    nim.time_units = XYZT_TO_TIME(nhdr.xyzt_units);

    nim.freq_dim = DIM_INFO_TO_FREQ_DIM(nhdr.dim_info);
    nim.phase_dim = DIM_INFO_TO_PHASE_DIM(nhdr.dim_info);
    nim.slice_dim = DIM_INFO_TO_SLICE_DIM(nhdr.dim_info);

    nim.slice_code = nhdr.slice_code;
  }

  /**- set Miscellaneous ANALYZE stuff */

  nim.cal_min = nhdr.cal_min;
  nim.cal_max = nhdr.cal_max;

  memcpy(nim.descrip, nhdr.descrip, 79); nim.descrip[79] = '\0';
  memcpy(nim.aux_file, nhdr.aux_file, 23); nim.aux_file[23] = '\0';

  /**- set ioff from vox_offset (but at least sizeof(header)) */

  is_onefile = ni_ver && NIFTI_ONEFILE(nhdr);

  if (is_onefile) {
    ioff = (int)nhdr.vox_offset;
    if (ioff < (int) sizeof(nhdr)) ioff = (int) sizeof(nhdr);
  }
  else {
    ioff = (int)nhdr.vox_offset;
  }
  nim.iname_offset = ioff;


  /**- deal with file names if set */
  if (!fname.empty()) {
    nifti_set_filenames(nim, fname, 0, 0);
  }
  else {
    nim.fname = "";
    nim.iname = "";
  }

  /* clear extension fields */
  nim.num_ext = 0;
  nim.ext_list = NULL;

  return nim;
}

/*----------------------------------------------------------------------
 * Read the extensions into the nifti_image struct   08 Dec 2004 [rickr]
 *
 * This function is called just after the header struct is read in, and
 * it is assumed the file pointer has not moved.  The value in remain
 * is assumed to be accurate, reflecting the bytes of space for potential
 * extensions.
 *
 * return the number of extensions read in, or < 0 on error
 *----------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_read_extensions(nifti_image &nim, znzFile fp, int64_t remain)
{
  nifti1_extender    extdr;      /* defines extension existence  */
  nifti1_extension   extn;       /* single extension to process  */
  nifti1_extension * Elist;      /* list of processed extensions */
  int64_t            posn, count;

  /* rcr n2 - add and use nifti2_extension type? */

  if (znz_isnull(fp))
    throw mdm_exception(__func__, "znz file pointer is null");

  posn = znztell(fp);

  if (remain < 16)
    return 0;

  count = znzread(extdr.extension, 1, 4, fp); /* get extender */

  if (count < 4)
    return 0;

  if (extdr.extension[0] != 1)
    return 0;

  remain -= 4;

  /* so we expect extensions, but have no idea of how many there may be */

  count = 0;
  Elist = NULL;
  while (nifti_read_next_extension(&extn, nim, remain, fp) > 0)
  {
    if (nifti_add_exten_to_list(&extn, &Elist, (int)count + 1) < 0) 
    {
      free(Elist);
      throw mdm_exception(__func__, "failed adding ext to list");
    }

    /* we have a new extension */
    remain -= extn.esize;
    count++;
  }

  /* rcr n2 - allow int64_t num ext? */
  nim.num_ext = (int)count;
  nim.ext_list = Elist;

  return count;
}

/*--------------------------------------------------------------------------*/
/*! free 'everything' about a nifti_image struct (including the passed struct)

    free (only fields which are not NULL):
      - fname and iname
      - data
      - any ext_list[i].edata
      - ext_list
      - nim
*//*------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_image_free(nifti_image &nim)
{
  if (nim.data != NULL) 
    free(nim.data);

  (void)nifti_free_extensions(nim);
}

/*--------------------------------------------------------------------------*/
/*! free the nifti extensions

    - If any edata pointer is set in the extension list, free() it.
    - Free ext_list, if it is set.
    - Clear num_ext and ext_list from nim.

    \return 0 on success, -1 on error

    \sa nifti_add_extension, nifti_copy_extensions
*//*------------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_free_extensions(nifti_image &nim)
{
  int c;
  if (nim.num_ext > 0 && nim.ext_list) 
  {
    for (c = 0; c < nim.num_ext; c++)
      if (nim.ext_list[c].edata) free(nim.ext_list[c].edata);
    free(nim.ext_list);
  }

  nim.num_ext = 0;
  nim.ext_list = NULL;

  return 0;
}

/*----------------------------------------------------------------------*/
/*! determine NIFTI version from buffer (check sizeof_hdr and magic)

       \return -1 on error, else NIFTI version
 *//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_header_version(const char *buf, size_t nbytes)
{
  nifti_1_header *n1p = (nifti_1_header *)buf;
  nifti_2_header *n2p = (nifti_2_header *)buf;
  int             sizeof_hdr, sver, nver;

  if (!buf)
    throw mdm_exception(__func__, "NULL buffer pointer");

  if (nbytes < sizeof(nifti_1_header))
    throw mdm_exception(__func__, "nbytes=" + std::to_string(nbytes) + ", too small for test");

  /* try to determine the version based on sizeof_hdr */
  sver = -1;
  sizeof_hdr = n1p->sizeof_hdr;
  if (sizeof_hdr == (int)sizeof(nifti_1_header)) sver = 1;
  else if (sizeof_hdr == (int)sizeof(nifti_2_header)) sver = 2;
  else { /* try swapping */
    nifti_swap_4bytes(1, &sizeof_hdr);
    if (sizeof_hdr == (int)sizeof(nifti_1_header)) sver = 1;
    else if (sizeof_hdr == (int)sizeof(nifti_2_header)) sver = 2;
  }

  /* and check magic field */
  if (sver == 1) nver = NIFTI_VERSION(*n1p);
  else if (sver == 2) nver = NIFTI_VERSION(*n2p);
  else                  nver = -1;

  /* now compare and return */

  if (sver == 1) {
    nver = NIFTI_VERSION(*n1p);
    if (nver == 0) return 0;        /* ANALYZE */
    if (nver == 1) return 1;        /* NIFTI-1 */
    return -1;
  }
  else if (sver == 2) {
    nver = NIFTI_VERSION(*n2p);
    if (nver == 2) return 2;        /* NIFTI-2 */
    throw mdm_exception(__func__, "bad NIFTI-2 magic4= ");
  }

  /* failure */
  throw mdm_exception(__func__, "bad sizeof_hdr = " + std::to_string(n1p->sizeof_hdr));
}

/*----------------------------------------------------------------------*/
/*! convert a nifti_2_header into a nifti_image

   \return an allocated nifti_image, or NULL on failure
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_image mdm_NiftiFormat::nifti_convert_n2hdr2nim(nifti_2_header nhdr, const std::string &fname)
{
  int          ii, doswap, ni_ver, is_onefile;
  nifti_image nim;

  /* be explicit with pointers */
  nim.data = NULL;

  /**- check if we must swap bytes */
  doswap = NIFTI2_NEEDS_SWAP(nhdr); /* swap data flag */

  /**- determine if this is a NIFTI-2 compliant header */
  ni_ver = NIFTI_VERSION(nhdr);
  if (ni_ver != 2)
    throw mdm_exception(__func__, "convert NIFTI-2 hdr2nim: bad version " + std::to_string(ni_ver));
    

  if (doswap) 
    swap_nifti_header(&nhdr, ni_ver);
  
  if (nhdr.datatype == mdm_ImageDatatypes::DT_BINARY || nhdr.datatype == mdm_ImageDatatypes::DT_UNKNOWN)
    throw mdm_exception(__func__, "bad datatype");

  if (nhdr.dim[1] <= 0)
    throw mdm_exception(__func__, "bad dim[1]");

  /* fix bad dim[] values in the defined dimension range */
  for (ii = 2; ii <= nhdr.dim[0]; ii++)
    if (nhdr.dim[ii] <= 0) nhdr.dim[ii] = 1;

  /* fix any remaining bad dim[] values, so garbage does not propagate */
  /* (only values 0 or 1 seem rational, otherwise set to arbirary 1)   */
  for (ii = nhdr.dim[0] + 1; ii <= 7; ii++)
    if (nhdr.dim[ii] != 1 && nhdr.dim[ii] != 0) nhdr.dim[ii] = 1;

  /**- set bad grid spacings to 1.0 */
  for (ii = 1; ii <= nhdr.dim[0]; ii++) {
    if (nhdr.pixdim[ii] == 0.0) nhdr.pixdim[ii] = 1.0;
  }

  is_onefile = (ni_ver > 0) && NIFTI_ONEFILE(nhdr);

  nim.nifti_type = (is_onefile) ? NIFTI_FTYPE::NIFTI1_1 : NIFTI_FTYPE::NIFTI1_2;

  ii = nifti_short_order();
  if (doswap)   
    nim.byteorder = REVERSE_ORDER(ii);
  else           
    nim.byteorder = ii;


  /**- set dimensions of data array */
  nim.ndim = nim.dim[0] = nhdr.dim[0];
  nim.nx = nim.dim[1] = nhdr.dim[1];
  nim.ny = nim.dim[2] = nhdr.dim[2];
  nim.nz = nim.dim[3] = nhdr.dim[3];
  nim.nt = nim.dim[4] = nhdr.dim[4];
  nim.nu = nim.dim[5] = nhdr.dim[5];
  nim.nv = nim.dim[6] = nhdr.dim[6];
  nim.nw = nim.dim[7] = nhdr.dim[7];

  for (ii = 1, nim.nvox = 1; ii <= nhdr.dim[0]; ii++)
    nim.nvox *= nhdr.dim[ii];

  /**- set the type of data in voxels and how many bytes per voxel */
  nim.datatype = nhdr.datatype;

  nifti_datatype_sizes(nim.datatype, nim.nbyper, nim.swapsize);
  if (nim.nbyper == 0) 
    throw mdm_exception(__func__, "bad datatype");

  /**- set the grid spacings */
  nim.dx = nim.pixdim[1] = nhdr.pixdim[1];
  nim.dy = nim.pixdim[2] = nhdr.pixdim[2];
  nim.dz = nim.pixdim[3] = nhdr.pixdim[3];
  nim.dt = nim.pixdim[4] = nhdr.pixdim[4];
  nim.du = nim.pixdim[5] = nhdr.pixdim[5];
  nim.dv = nim.pixdim[6] = nhdr.pixdim[6];
  nim.dw = nim.pixdim[7] = nhdr.pixdim[7];

  /**- compute qto_xyz transformation from pixel indexes (i,j,k) to (x,y,z) */
  if (!ni_ver || nhdr.qform_code <= 0) {
    /**- if not nifti or qform_code <= 0, use grid spacing for qto_xyz */
    nim.qto_xyz.m[0][0] = nim.dx;  /* grid spacings */
    nim.qto_xyz.m[1][1] = nim.dy;  /* along diagonal */
    nim.qto_xyz.m[2][2] = nim.dz;

    /* off diagonal is zero */
    nim.qto_xyz.m[0][1] = nim.qto_xyz.m[0][2] = nim.qto_xyz.m[0][3] = 0.0f;
    nim.qto_xyz.m[1][0] = nim.qto_xyz.m[1][2] = nim.qto_xyz.m[1][3] = 0.0f;
    nim.qto_xyz.m[2][0] = nim.qto_xyz.m[2][1] = nim.qto_xyz.m[2][3] = 0.0f;

    /* last row is always [ 0 0 0 1 ] */
    nim.qto_xyz.m[3][0] = nim.qto_xyz.m[3][1] = nim.qto_xyz.m[3][2] = 0.0f;
    nim.qto_xyz.m[3][3] = 1.0f;

    nim.qform_code = NIFTI_XFORM_UNKNOWN;

  }
  else {
    /**- else NIFTI: use the quaternion-specified transformation */
    nim.quatern_b = nhdr.quatern_b;
    nim.quatern_c = nhdr.quatern_c;
    nim.quatern_d = nhdr.quatern_d;

    nim.qoffset_x = nhdr.qoffset_x;
    nim.qoffset_y = nhdr.qoffset_y;
    nim.qoffset_z = nhdr.qoffset_z;

    nim.qfac = (nhdr.pixdim[0] < 0.0) ? -1.0 : 1.0;  /* left-handedness? */

    nim.qto_xyz = nifti_quatern_to_dmat44(
      nim.quatern_b, nim.quatern_c, nim.quatern_d,
      nim.qoffset_x, nim.qoffset_y, nim.qoffset_z,
      nim.dx, nim.dy, nim.dz,
      nim.qfac);

    nim.qform_code = nhdr.qform_code;
  }

  /**- load inverse transformation (x,y,z) -> (i,j,k) */
  nim.qto_ijk = nifti_dmat44_inverse(nim.qto_xyz);

  /**- load sto_xyz affine transformation, if present */
  if (!ni_ver || nhdr.sform_code <= 0) {
    /**- if not nifti or sform_code <= 0, then no sto transformation */

    nim.sform_code = NIFTI_XFORM_UNKNOWN;
  }
  else {
    /**- else set the sto transformation from srow_*[] */
    nim.sto_xyz.m[0][0] = nhdr.srow_x[0];
    nim.sto_xyz.m[0][1] = nhdr.srow_x[1];
    nim.sto_xyz.m[0][2] = nhdr.srow_x[2];
    nim.sto_xyz.m[0][3] = nhdr.srow_x[3];

    nim.sto_xyz.m[1][0] = nhdr.srow_y[0];
    nim.sto_xyz.m[1][1] = nhdr.srow_y[1];
    nim.sto_xyz.m[1][2] = nhdr.srow_y[2];
    nim.sto_xyz.m[1][3] = nhdr.srow_y[3];

    nim.sto_xyz.m[2][0] = nhdr.srow_z[0];
    nim.sto_xyz.m[2][1] = nhdr.srow_z[1];
    nim.sto_xyz.m[2][2] = nhdr.srow_z[2];
    nim.sto_xyz.m[2][3] = nhdr.srow_z[3];

    /* last row is always [ 0 0 0 1 ] */
    nim.sto_xyz.m[3][0] = nim.sto_xyz.m[3][1] = nim.sto_xyz.m[3][2] = 0.0f;
    nim.sto_xyz.m[3][3] = 1.0f;

    nim.sto_ijk = nifti_dmat44_inverse(nim.sto_xyz);

    nim.sform_code = nhdr.sform_code;
  }

  /**- set miscellaneous NIFTI stuff */
  if (ni_ver) {
    nim.scl_slope = nhdr.scl_slope;
    nim.scl_inter = nhdr.scl_inter;

    nim.intent_code = nhdr.intent_code;

    nim.intent_p1 = nhdr.intent_p1;
    nim.intent_p2 = nhdr.intent_p2;
    nim.intent_p3 = nhdr.intent_p3;

    nim.toffset = nhdr.toffset;

    memcpy(nim.intent_name, nhdr.intent_name, 15); nim.intent_name[15] = '\0';

    nim.xyz_units = XYZT_TO_SPACE(nhdr.xyzt_units);
    nim.time_units = XYZT_TO_TIME(nhdr.xyzt_units);

    nim.freq_dim = DIM_INFO_TO_FREQ_DIM(nhdr.dim_info);
    nim.phase_dim = DIM_INFO_TO_PHASE_DIM(nhdr.dim_info);
    nim.slice_dim = DIM_INFO_TO_SLICE_DIM(nhdr.dim_info);

    nim.slice_code = nhdr.slice_code;
  }

  /**- set Miscellaneous ANALYZE stuff */
  nim.cal_min = nhdr.cal_min;
  nim.cal_max = nhdr.cal_max;

  memcpy(nim.descrip, nhdr.descrip, 79); nim.descrip[79] = '\0';
  memcpy(nim.aux_file, nhdr.aux_file, 23); nim.aux_file[23] = '\0';

  /**- set ioff from vox_offset (but at least sizeof(header)) */
  nim.iname_offset = nhdr.vox_offset;
  if (is_onefile && nhdr.vox_offset < (int64_t)sizeof(nhdr))
    nim.iname_offset = (int64_t)sizeof(nhdr);

  /**- deal with file names if set */
  if (!fname.empty()) 
  {
    nifti_set_filenames(nim, fname, 0, 0);
  }
  else {
    nim.fname = "";
    nim.iname = "";
  }

  /* clear extension fields */
  nim.num_ext = 0;
  nim.ext_list = NULL;

  return nim;
}

/*----------------------------------------------------------------------
 * nifti_image_load
 *----------------------------------------------------------------------*/
 /*! \fn int nifti_image_load( nifti_image &nim )
     \brief Load the image blob into a previously initialized nifti_image.

         - If not yet set, the data buffer is allocated with calloc().
         - The data buffer will be byteswapped if necessary.
         - The data buffer will not be scaled.

     This function is used to read the image from disk.  It should be used
     after a function such as nifti_image_read(), so that the nifti_image
     structure is already initialized.

     \param  nim pointer to a nifti_image (previously initialized)
     \return 0 on success, -1 on failure
     \sa     nifti_image_read, nifti_image_free, nifti_image_unload
 */
int mdm_NiftiFormat::nifti_image_load(nifti_image &nim)
{
  /* set up data space, open data file and seek, then call nifti_read_buffer */
  int64_t ntot, ii;
  znzFile fp;

  /**- open the file and position the FILE pointer */
  fp = nifti_image_load_prep(nim);

  if (fp == NULL)
    throw mdm_exception(__func__, "failed load_prep");

  ntot = nifti_get_volsize(nim);

  /**- if the data pointer is not yet set, get memory space for the image */
  if (nim.data == NULL)
  {
    nim.data = (void *)calloc(1, ntot);  /* create image memory */
    if (nim.data == NULL) 
    {
      znzclose(fp);
      throw mdm_exception(__func__, "failed to alloc " + std::to_string(ntot) + " bytes for image data");
    }
  }

  /**- now that everything is set up, do the reading */
  ii = nifti_read_buffer(fp, nim.data, ntot, nim);

  if (ii < ntot) {
    znzclose(fp);
    free(nim.data);
    nim.data = NULL;
    return -1;  /* errors were printed in nifti_read_buffer() */
  }

  /**- close the file */
  znzclose(fp);

  return 0;
}

/*----------------------------------------------------------------------
 * check whether byte swapping is needed
 *
 * dim[0] should be in [0,7], and sizeof_hdr should be accurate
 *
 * \returns  > 0 : needs swap
 *             0 : does not need swap
 *           < 0 : error condition
 *----------------------------------------------------------------------*/
int mdm_NiftiFormat::need_nhdr_swap(short dim0, int hdrsize)
{
  short d0 = dim0;     /* so we won't have to swap them on the stack */
  int   hsize = hdrsize;

  if (d0 != 0) 
  {     /* then use it for the check */
    if (d0 > 0 && d0 <= 7) 
      return 0;

    nifti_swap_2bytes(1, &d0);        /* swap? */
    if (d0 > 0 && d0 <= 7) 
      return 1;

    throw mdm_exception(__func__, "** NIFTI: bad swapped");
  }

  /* dim[0] == 0 should not happen, but could, so try hdrsize */
  if (hsize == sizeof(nifti_1_header)) 
    return 0;

  nifti_swap_4bytes(1, &hsize);     /* swap? */
  if (hsize == sizeof(nifti_1_header)) 
    return 1;

  throw mdm_exception(__func__, "** NIFTI: bad hsize"); /* bad, naughty hsize */
}

/*-------------------------------------------------------------------------*/
/*! Byte swap NIFTI file header, depending on the version.
*//*---------------------------------------------------------------------- */
void  mdm_NiftiFormat::swap_nifti_header(void * hdr, int ni_ver)
{
  if (ni_ver == 0) nifti_swap_as_analyze((nifti_analyze75 *)hdr);
  else if (ni_ver == 1) nifti_swap_as_nifti1((nifti_1_header *)hdr);
  else if (ni_ver == 2) nifti_swap_as_nifti2((nifti_2_header *)hdr);
  else if (ni_ver >= 0 && ni_ver <= 9)
    mdm_ProgramLogger::logProgramWarning(__func__,
      "not ready for version" + std::to_string(ni_ver));
  
  else 
    mdm_ProgramLogger::logProgramWarning(__func__,
      "illegal version" + std::to_string(ni_ver));
}

/*----------------------------------------------------------------------*/
/*! create and set new filenames, based on prefix and image type

   \param nim            pointer to nifti_image in which to set filenames
   \param prefix         (required) prefix for output filenames
   \param check          check for previous existence of filename
                         (existence is an error condition)
   \param set_byte_order flag to set nim.byteorder here
                         (this is probably a logical place to do so)

   \return 0 on successful update

   \warning this will free() any existing names and create new ones

   \sa nifti_makeimgname, nifti_makehdrname, nifti_type_and_names_match
*//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_set_filenames(nifti_image & nim, const std::string &prefix, int check,
  int set_byte_order)
{
  int comp = nifti_is_gzfile(prefix);

  if (prefix.empty())
    throw mdm_exception(__func__, "filename must not be empty ");  

  /* set and test output filenames */
  nim.fname = nifti_makehdrname(prefix, nim.nifti_type, check, comp);
  nim.iname = nifti_makeimgname(prefix, nim.nifti_type, check, comp);

  if (set_byte_order) nim.byteorder = nifti_short_order();

  return 0;
}

/*----------------------------------------------------------------------*/
/*! creates a filename for storing the header, based on nifti_type

   \param   prefix      - this will be copied before the suffix is added
   \param   nifti_type  - determines the extension, unless one is in prefix
   \param   check       - check for existence (fail condition)
   \param   comp        - add .gz for compressed name

   Note that if prefix provides a file suffix, nifti_type is not used.

   NB: this allocates memory which should be freed

   \sa nifti_set_filenames
*//*-------------------------------------------------------------------*/
std::string mdm_NiftiFormat::nifti_makehdrname(const std::string &fileName, int nifti_type, int check,
  int comp)
{
  // Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName,
    baseName, ext, gz);

  //if no extension, add extension based on type
  std::string hdrName;
  if (ext.empty())
  {
    if (nifti_type == NIFTI_FTYPE::NIFTI1_1 || nifti_type == NIFTI_FTYPE::NIFTI2_1)
      hdrName = baseName + extnii;
    else if (nifti_type == NIFTI_FTYPE::ASCII)
      hdrName = baseName + extnia;
    else
      hdrName = baseName + exthdr;
  }
  else if (boost::iequals(ext, extimg))
    //If .img, convert to .hdr
    hdrName = baseName + exthdr;
  else
    //Use the valid extension
    hdrName = baseName + ext;

#ifdef HAVE_ZLIB  /* if compression is requested, make sure of suffix */
  if (comp || gz)
    hdrName += extgz;
#else
  if (gz || gz)
    throw mdm_exception(__func__, 
      "requested gz compression to write " + fileName + ", but this version of Madym has been built without zlib support.");
#endif

  return hdrName;
}


/*----------------------------------------------------------------------*/
/*! creates a filename for storing the image, based on nifti_type

   \param   prefix      - this will be copied before the suffix is added
   \param   nifti_type  - determines the extension, unless provided by prefix
   \param   check       - check for existence (fail condition)
   \param   comp        - add .gz for compressed name

   Note that if prefix provides a file suffix, nifti_type is not used.

   NB: it allocates memory which should be freed

   \sa nifti_set_filenames
*//*-------------------------------------------------------------------*/
std::string mdm_NiftiFormat::nifti_makeimgname(const std::string &fileName, int nifti_type, int check,
  int comp)
{
  // Parse the filename
  std::string baseName;
  std::string ext;
  bool gz;
  parseName(fileName,
    baseName, ext, gz);

  //if no extension, add extension based on type
  std::string imgName;
  if (ext.empty())
  {
    if (nifti_type == NIFTI_FTYPE::NIFTI1_1 || nifti_type == NIFTI_FTYPE::NIFTI2_1)
      imgName = baseName + extnii;
    else if (nifti_type == NIFTI_FTYPE::ASCII)
      imgName = baseName + extnia;
    else
      imgName = baseName + exthdr;
  }
  else if (boost::iequals(ext, exthdr))
    //If .hdr, convert to .img
    imgName = baseName + extimg;
  else
    //Use the valid extension
    imgName = baseName + ext;

#ifdef HAVE_ZLIB  /* if compression is requested, make sure of suffix */
  if (comp)
    imgName += extgz;
#else
  if (gz)
    throw mdm_exception(__func__,
      "requested gz compression to write " + fileName + ", but this version of Madym has been built without zlib support.");
#endif

  return imgName;
}

//
bool mdm_NiftiFormat::nifti_is_gzfile(const std::string &prefix)
{
  fs::path p(prefix);
  return boost::iequals(p.extension().string(), extgz);
}

/*----------------------------------------------------------------------
 * nifti_read_next_extension  - read a single extension from the file
 *
 * return (>= 0 is okay):
 *
 *     success      : esize
 *     no extension : 0
 *     error        : -1
 *----------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_read_next_extension(nifti1_extension * nex, nifti_image &nim,
  int remain, znzFile fp)
{
  int swap = nim.byteorder != nifti_short_order();
  int count, size, code = -1;

  /* first clear nex */
  nex->esize = nex->ecode = 0;
  nex->edata = NULL;

  if (remain < 16) 
    return 0;

  /* must start with 4-byte size and code */
  count = (int)znzread(&size, 4, 1, fp);
  if (count == 1) count += (int)znzread(&code, 4, 1, fp);

  if (count != 2 || code == -1) {
    znzseek(fp, -4 * count, SEEK_CUR); /* back up past any read */
    return 0;                        /* no extension, no error condition */
  }

  if (swap) 
  {
    nifti_swap_4bytes(1, &size);
    nifti_swap_4bytes(1, &code);
  }

  if (!nifti_check_extension(nim, size, code, remain)) 
  {
    if (znzseek(fp, -8, SEEK_CUR) < 0) /* back up past any read */
      throw mdm_exception(__func__, "failure to back out of extension read!");
    return 0;
  }

  /* now get the actual data */
  nex->esize = size;
  nex->ecode = code;

  size -= 8;  /* subtract space for size and code in extension */
  nex->edata = (char *)malloc(size * sizeof(char));
  if (!nex->edata)
    throw mdm_exception(__func__, 
      "failed to allocate " + std::to_string(size) + " bytes for extension");
      
  count = (int)znzread(nex->edata, 1, size, fp);
  if (count < size) 
  {
    free(nex->edata);
    nex->edata = NULL;
    throw mdm_exception(__func__,
      "read only " + std::to_string(count) + " of " + std::to_string(size) + 
      " bytes for extension");
  }

  /* success! */
  return nex->esize;
}

/*----------------------------------------------------------------------*/
/* nifti_add_exten_to_list     - add a new nifti1_extension to the list

   We will append via "malloc, copy and free", because on an error,
   the list will revert to the previous one.

   return 0 on success, -1 on error (and free the entire list)
*//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_add_exten_to_list(nifti1_extension *  new_ext,
  nifti1_extension ** list, int new_length)
{
  nifti1_extension * tmplist;

  tmplist = *list;
  *list = (nifti1_extension *)malloc(new_length * sizeof(nifti1_extension));

  /* check for failure first */
  if (!*list) 
  {
    if (!tmplist) return -1;  /* no old list to lose */

    *list = tmplist;  /* reset list to old one */
    throw mdm_exception(__func__,
      "failed to alloc " + std::to_string(new_length) +
      " ext structs (" + std::to_string(new_length*(int)sizeof(nifti1_extension)) + " bytes)");
  }

  /* if an old list exists, copy the pointers and free the list */
  if (tmplist) {
    memcpy(*list, tmplist, (new_length - 1) * sizeof(nifti1_extension));
    free(tmplist);
  }

  /* for some reason, I just don't like struct copy... */
  (*list)[new_length - 1].esize = new_ext->esize;
  (*list)[new_length - 1].ecode = new_ext->ecode;
  (*list)[new_length - 1].edata = new_ext->edata;

  return 0;
}

/*----------------------------------------------------------------------
 * nifti_image_load_prep  - prepare to read data
 *
 * Check nifti_image fields, open the file and seek to the appropriate
 * offset for reading.
 *
 * return NULL on failure
 *----------------------------------------------------------------------*/
znzFile mdm_NiftiFormat::nifti_image_load_prep(nifti_image &nim)
{
  /* set up data space, open data file and seek, then call nifti_read_buffer */
  int64_t ntot, ii, ioff;
  znzFile fp;
  char    fname[] = { "nifti_image_load_prep" };

  /**- perform sanity checks */
  if (nim.iname.empty() || nim.nbyper <= 0 || nim.nvox <= 0)
    return NULL;

  ntot = nifti_get_volsize(nim); /* total bytes to read */

  /**- open image data file */
  auto tmpimgname = nifti_findimgname(nim.iname, nim.nifti_type);

  fp = znzopen(tmpimgname.c_str(), "rb", nifti_is_gzfile(tmpimgname));
  if (znz_isnull(fp))
    return NULL;  /* bad open? */

  /**- get image offset: a negative offset means to figure from end of file */
  if (nim.iname_offset < 0) {
    if (nifti_is_gzfile(nim.iname)) 
    {
      znzclose(fp);
      return NULL;
    }
    ii = nifti_get_filesize(nim.iname);
    if (ii <= 0) {
      znzclose(fp);
      return NULL;
    }
    ioff = (ii > ntot) ? ii - ntot : 0;
  }
  else {                              /* non-negative offset   */
    ioff = nim.iname_offset;          /* means use it directly */
  }

  /**- seek to the appropriate read position */
  if (znzseek(fp, (long)ioff, SEEK_SET) < 0) {
    mdm_ProgramLogger::logProgramWarning(__func__, "could not seek to offset in file");
    znzclose(fp);
    return NULL;
  }

  /**- and return the File pointer */
  return fp;
}

/*----------------------------------------------------------------------*/
/*! read ntot bytes of data from an open file and byte swaps if necessary

   note that nifti_image is required for information on datatype, bsize
   (for any needed byte swapping), etc.

   This function does not allocate memory, so dataptr must be valid.
*//*--------------------------------------------------------------------*/
int64_t mdm_NiftiFormat::nifti_read_buffer(znzFile fp, void* dataptr, int64_t ntot,
  nifti_image &nim)
{
  int64_t ii;

  if (dataptr == NULL) 
    return -1;

  ii = znzread(dataptr, 1, ntot, fp);             /* data input */

  /* if read was short, fail */
  if (ii < ntot) 
    return -1;

  /* byte swap array if needed */

  /* ntot/swapsize might not fit as int, use int64_t    6 Jul 2010 [rickr] */
  if (nim.swapsize > 1 && nim.byteorder != nifti_short_order())
    nifti_swap_Nbytes((int)(ntot / nim.swapsize), nim.swapsize, dataptr);

  return ii;
}

/*-------------------------------------------------------------------------*/
/*! Byte swap NIFTI-2 file header.
*//*---------------------------------------------------------------------- */
void mdm_NiftiFormat::nifti_swap_as_nifti2(nifti_2_header * h)
{
  if (!h) {
    mdm_ProgramLogger::logProgramWarning(__func__, "NULL pointer to header");
    return;
  }

  nifti_swap_4bytes(1, &h->sizeof_hdr);

  nifti_swap_2bytes(1, &h->datatype);
  nifti_swap_2bytes(1, &h->bitpix);

  nifti_swap_8bytes(8, h->dim);
  nifti_swap_8bytes(1, &h->intent_p1);
  nifti_swap_8bytes(1, &h->intent_p2);
  nifti_swap_8bytes(1, &h->intent_p3);
  nifti_swap_8bytes(8, h->pixdim);

  nifti_swap_8bytes(1, &h->vox_offset);
  nifti_swap_8bytes(1, &h->scl_slope);
  nifti_swap_8bytes(1, &h->scl_inter);
  nifti_swap_8bytes(1, &h->cal_max);
  nifti_swap_8bytes(1, &h->cal_min);
  nifti_swap_8bytes(1, &h->toffset);

  nifti_swap_4bytes(1, &h->qform_code);
  nifti_swap_4bytes(1, &h->sform_code);

  nifti_swap_8bytes(1, &h->quatern_b);
  nifti_swap_8bytes(1, &h->quatern_c);
  nifti_swap_8bytes(1, &h->quatern_d);
  nifti_swap_8bytes(1, &h->qoffset_x);
  nifti_swap_8bytes(1, &h->qoffset_y);
  nifti_swap_8bytes(1, &h->qoffset_z);

  nifti_swap_8bytes(4, h->srow_x);
  nifti_swap_8bytes(4, h->srow_y);
  nifti_swap_8bytes(4, h->srow_z);

  nifti_swap_4bytes(1, &h->slice_code);
  nifti_swap_4bytes(1, &h->xyzt_units);
  nifti_swap_4bytes(1, &h->intent_code);
}

/*-------------------------------------------------------------------------*/
/*! Byte swap NIFTI-1 file header in various places and ways.
 *  return 0 on success
*//*---------------------------------------------------------------------- */
void mdm_NiftiFormat::nifti_swap_as_nifti1(nifti_1_header * h)
{
  if (!h) {
    mdm_ProgramLogger::logProgramWarning(__func__, "NULL pointer to header");
    return;
  }

  nifti_swap_4bytes(1, &h->sizeof_hdr);
  nifti_swap_4bytes(1, &h->extents);
  nifti_swap_2bytes(1, &h->session_error);

  nifti_swap_2bytes(8, h->dim);
  nifti_swap_4bytes(1, &h->intent_p1);
  nifti_swap_4bytes(1, &h->intent_p2);
  nifti_swap_4bytes(1, &h->intent_p3);

  nifti_swap_2bytes(1, &h->intent_code);
  nifti_swap_2bytes(1, &h->datatype);
  nifti_swap_2bytes(1, &h->bitpix);

  nifti_swap_4bytes(8, h->pixdim);

  nifti_swap_4bytes(1, &h->vox_offset);
  nifti_swap_4bytes(1, &h->scl_slope);
  nifti_swap_4bytes(1, &h->scl_inter);

  nifti_swap_4bytes(1, &h->cal_max);
  nifti_swap_4bytes(1, &h->cal_min);
  nifti_swap_4bytes(1, &h->toffset);
  nifti_swap_4bytes(1, &h->glmax);
  nifti_swap_4bytes(1, &h->glmin);

  nifti_swap_2bytes(1, &h->qform_code);
  nifti_swap_2bytes(1, &h->sform_code);

  nifti_swap_4bytes(1, &h->quatern_b);
  nifti_swap_4bytes(1, &h->quatern_c);
  nifti_swap_4bytes(1, &h->quatern_d);
  nifti_swap_4bytes(1, &h->qoffset_x);
  nifti_swap_4bytes(1, &h->qoffset_y);
  nifti_swap_4bytes(1, &h->qoffset_z);

  nifti_swap_4bytes(4, h->srow_x);
  nifti_swap_4bytes(4, h->srow_y);
  nifti_swap_4bytes(4, h->srow_z);
}

/*-------------------------------------------------------------------------*/
/*! Byte swap as an ANALYZE 7.5 header
 *
 *  return non-zero on failure
*//*---------------------------------------------------------------------- */
void mdm_NiftiFormat::nifti_swap_as_analyze(nifti_analyze75 * h)
{
  if (!h) {
    mdm_ProgramLogger::logProgramWarning(__func__, "NULL pointer to header");
    return;
  }

  nifti_swap_4bytes(1, &h->sizeof_hdr);
  nifti_swap_4bytes(1, &h->extents);
  nifti_swap_2bytes(1, &h->session_error);

  nifti_swap_2bytes(8, h->dim);
  nifti_swap_2bytes(1, &h->unused8);
  nifti_swap_2bytes(1, &h->unused9);
  nifti_swap_2bytes(1, &h->unused10);
  nifti_swap_2bytes(1, &h->unused11);
  nifti_swap_2bytes(1, &h->unused12);
  nifti_swap_2bytes(1, &h->unused13);
  nifti_swap_2bytes(1, &h->unused14);

  nifti_swap_2bytes(1, &h->datatype);
  nifti_swap_2bytes(1, &h->bitpix);
  nifti_swap_2bytes(1, &h->dim_un0);

  nifti_swap_4bytes(8, h->pixdim);

  nifti_swap_4bytes(1, &h->vox_offset);
  nifti_swap_4bytes(1, &h->funused1);
  nifti_swap_4bytes(1, &h->funused2);
  nifti_swap_4bytes(1, &h->funused3);

  nifti_swap_4bytes(1, &h->cal_max);
  nifti_swap_4bytes(1, &h->cal_min);
  nifti_swap_4bytes(1, &h->compressed);
  nifti_swap_4bytes(1, &h->verified);
  nifti_swap_4bytes(1, &h->glmax);
  nifti_swap_4bytes(1, &h->glmin);

  nifti_swap_4bytes(1, &h->views);
  nifti_swap_4bytes(1, &h->vols_added);
  nifti_swap_4bytes(1, &h->start_field);
  nifti_swap_4bytes(1, &h->field_skip);

  nifti_swap_4bytes(1, &h->omax);
  nifti_swap_4bytes(1, &h->omin);
  nifti_swap_4bytes(1, &h->smax);
  nifti_swap_4bytes(1, &h->smin);
}

/*--------------------------------------------------------------------------*/
/*! set the nifti_type field based on fname and iname

    Note that nifti_type is changed only when it does not match
    the filenames.

    \return 0 on success, -1 on error

    \sa is_valid_nifti_type, nifti_type_and_names_match
*//*------------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_set_type_from_names(nifti_image &nim)
{
  //Check fname and iname
  std::string baseName;
  std::string fext, iext;
  bool gz;
  parseName(nim.fname,
    baseName, fext, gz);
  parseName(nim.iname,
    baseName, iext, gz);

  /* type should be NIFTI_FTYPE::ASCII if extension is .nia */
  if (boost::iequals(fext, extnia)) 
  {
    nim.nifti_type = NIFTI_FTYPE::ASCII;
  }
  else {
    /* not too picky here, do what must be done, and then verify */
    if (nim.fname == nim.iname)          /* one file, type 1 */
      nim.nifti_type = NIFTI_FTYPE::NIFTI1_1;
    else if (nim.nifti_type == NIFTI_FTYPE::NIFTI1_1) /* cannot be type 1 */
      nim.nifti_type = NIFTI_FTYPE::NIFTI1_2;
  }

  if (is_valid_nifti_type(nim.nifti_type)) 
    return 0;  /* success! */

  throw mdm_exception(__func__,
    "bad nifti_type " + std::to_string(nim.nifti_type) + " for "
    + nim.fname + " and " + nim.iname);
}

/*----------------------------------------------------------------------
 * check for valid size and code, as well as can be done
 *----------------------------------------------------------------------*/
#define LNI_MAX_NIA_EXT_LEN 100000  /* consider a longer extension invalid */
int mdm_NiftiFormat::nifti_check_extension(nifti_image &nim, int size, int code, int rem)
{
  if (size < 16) 
    return 0;

  if (size > rem) 
    return 0;

  if (size & 0xf) 
    return 0;

  if (nim.nifti_type == NIFTI_FTYPE::ASCII && size > LNI_MAX_NIA_EXT_LEN) 
    return 0;

  return 1;
}

/*----------------------------------------------------------------------*/
/*! convert a nifti_image structure to a nifti_2_header struct

    No allocation is done, this should be used via structure copy.
    As in:
    <pre>
    nifti_2_header my_header;
    my_header = nifti_convert_nim2n2hdr(my_nim_pointer);
    </pre>
*//*--------------------------------------------------------------------*/
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED
int mdm_NiftiFormat::nifti_convert_nim2n2hdr(const nifti_image &nim, nifti_2_header &hdr)
{
  nifti_2_header nhdr;

  memset(&nhdr, 0, sizeof(nhdr));  /* zero out header, to be safe */


  /**- load the ANALYZE-7.5 generic parts of the header struct */

  nhdr.sizeof_hdr = sizeof(nhdr);
  if (nim.nifti_type == NIFTI_FTYPE::NIFTI2_1) 
    strcpy(nhdr.magic, "n+2");
  else                                          
    strcpy(nhdr.magic, "ni2");

  nhdr.datatype = nim.datatype;
  nhdr.bitpix = 8 * nim.nbyper;

  nhdr.dim[0] = nim.ndim;
  nhdr.dim[1] = nim.nx; nhdr.dim[2] = nim.ny; nhdr.dim[3] = nim.nz;
  nhdr.dim[4] = nim.nt; nhdr.dim[5] = nim.nu; nhdr.dim[6] = nim.nv;
  nhdr.dim[7] = nim.nw;

  nhdr.intent_p1 = nim.intent_p1;
  nhdr.intent_p2 = nim.intent_p2;
  nhdr.intent_p3 = nim.intent_p3;

  nhdr.pixdim[0] = 0.0;
  nhdr.pixdim[1] = fabs(nim.dx); nhdr.pixdim[2] = fabs(nim.dy);
  nhdr.pixdim[3] = fabs(nim.dz); nhdr.pixdim[4] = fabs(nim.dt);
  nhdr.pixdim[5] = fabs(nim.du); nhdr.pixdim[6] = fabs(nim.dv);
  nhdr.pixdim[7] = fabs(nim.dw);

  nhdr.vox_offset = nim.iname_offset;

  nhdr.scl_slope = nim.scl_slope;
  nhdr.scl_inter = nim.scl_inter;

  nhdr.cal_max = nim.cal_max;
  nhdr.cal_min = nim.cal_min;
  nhdr.toffset = nim.toffset;

  if (nim.descrip[0] != '\0') {
    memcpy(nhdr.descrip, nim.descrip, 79); nhdr.descrip[79] = '\0';
  }
  if (nim.aux_file[0] != '\0') {
    memcpy(nhdr.aux_file, nim.aux_file, 23); nhdr.aux_file[23] = '\0';
  }

  if (nim.qform_code > 0) {
    nhdr.qform_code = nim.qform_code;
    nhdr.quatern_b = nim.quatern_b;
    nhdr.quatern_c = nim.quatern_c;
    nhdr.quatern_d = nim.quatern_d;
    nhdr.qoffset_x = nim.qoffset_x;
    nhdr.qoffset_y = nim.qoffset_y;
    nhdr.qoffset_z = nim.qoffset_z;
    nhdr.pixdim[0] = (nim.qfac >= 0.0) ? 1.0f : -1.0f;
  }

  if (nim.sform_code > 0) {
    nhdr.sform_code = nim.sform_code;
    nhdr.srow_x[0] = nim.sto_xyz.m[0][0];
    nhdr.srow_x[1] = nim.sto_xyz.m[0][1];
    nhdr.srow_x[2] = nim.sto_xyz.m[0][2];
    nhdr.srow_x[3] = nim.sto_xyz.m[0][3];
    nhdr.srow_y[0] = nim.sto_xyz.m[1][0];
    nhdr.srow_y[1] = nim.sto_xyz.m[1][1];
    nhdr.srow_y[2] = nim.sto_xyz.m[1][2];
    nhdr.srow_y[3] = nim.sto_xyz.m[1][3];
    nhdr.srow_z[0] = nim.sto_xyz.m[2][0];
    nhdr.srow_z[1] = nim.sto_xyz.m[2][1];
    nhdr.srow_z[2] = nim.sto_xyz.m[2][2];
    nhdr.srow_z[3] = nim.sto_xyz.m[2][3];
  }

  nhdr.slice_code = nim.slice_code;
  nhdr.xyzt_units = SPACE_TIME_TO_XYZT(nim.xyz_units, nim.time_units);
  nhdr.intent_code = nim.intent_code;
  if (nim.intent_name[0] != '\0') {
    memcpy(nhdr.intent_name, nim.intent_name, 15);
    nhdr.intent_name[15] = '\0';
  }

  nhdr.dim_info = FPS_INTO_DIM_INFO(nim.freq_dim,
    nim.phase_dim, nim.slice_dim);

  nhdr.unused_str[0] = '\0';  /* not needed, but complete */

  memcpy(&hdr, &nhdr, sizeof(nhdr));

  return 0;
}
DISABLE_WARNING_POP

#undef NIFTI_IS_16_BIT_INT
#define NIFTI_IS_16_BIT_INT(x) ((x) <= 32767 && (x) >= -32768)

#undef N_CHECK_2BYTE_VAL
#define N_CHECK_2BYTE_VAL(fn) do { if( ! NIFTI_IS_16_BIT_INT(nim.fn) ) { \
   mdm_ProgramLogger::logProgramWarning(__func__, "nim. = does not fit into NIFTI-1 header"); return 1; } } while(0)

/*----------------------------------------------------------------------*/
/*! convert a nifti_image structure to a nifti_1_header struct

    No allocation is done, this should be used via structure copy.
    As in:
    <pre>
    nifti_1_header my_header;
    my_header = nifti_convert_nim2n1hdr(my_nim_pointer);
    </pre>
*//*--------------------------------------------------------------------*/
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED
int mdm_NiftiFormat::nifti_convert_nim2n1hdr(const nifti_image & nim, nifti_1_header &hdr)
{
  nifti_1_header nhdr;
  memset(&nhdr, 0, sizeof(nhdr));  /* zero out header, to be safe */


  /**- load the ANALYZE-7.5 generic parts of the header struct */

  nhdr.sizeof_hdr = sizeof(nhdr);
  nhdr.regular = 'r';             /* for some stupid reason */

  N_CHECK_2BYTE_VAL(ndim);
  N_CHECK_2BYTE_VAL(nx);
  N_CHECK_2BYTE_VAL(ny);
  N_CHECK_2BYTE_VAL(nz);
  N_CHECK_2BYTE_VAL(nt);
  N_CHECK_2BYTE_VAL(nu);
  N_CHECK_2BYTE_VAL(nv);
  N_CHECK_2BYTE_VAL(nw);
  N_CHECK_2BYTE_VAL(datatype);
  N_CHECK_2BYTE_VAL(nbyper);

  nhdr.dim[0] = nim.ndim;
  nhdr.dim[1] = nim.nx; nhdr.dim[2] = nim.ny; nhdr.dim[3] = nim.nz;
  nhdr.dim[4] = nim.nt; nhdr.dim[5] = nim.nu; nhdr.dim[6] = nim.nv;
  nhdr.dim[7] = nim.nw;

  nhdr.pixdim[0] = 0.0f;
  nhdr.pixdim[1] = nim.dx; nhdr.pixdim[2] = nim.dy;
  nhdr.pixdim[3] = nim.dz; nhdr.pixdim[4] = nim.dt;
  nhdr.pixdim[5] = nim.du; nhdr.pixdim[6] = nim.dv;
  nhdr.pixdim[7] = nim.dw;

  nhdr.datatype = nim.datatype;
  nhdr.bitpix = 8 * nim.nbyper;

  if (nim.cal_max > nim.cal_min) {
    nhdr.cal_max = nim.cal_max;
    nhdr.cal_min = nim.cal_min;
  }

  if (nim.scl_slope != 0.0) {
    nhdr.scl_slope = nim.scl_slope;
    nhdr.scl_inter = nim.scl_inter;
  }

  if (nim.descrip[0] != '\0') {
    memcpy(nhdr.descrip, nim.descrip, 79); nhdr.descrip[79] = '\0';
  }
  if (nim.aux_file[0] != '\0') {
    memcpy(nhdr.aux_file, nim.aux_file, 23); nhdr.aux_file[23] = '\0';
  }

  /**- Load NIFTI specific stuff into the header */

  if (nim.nifti_type > NIFTI_FTYPE::ANALYZE) { /* then not ANALYZE */

    if (nim.nifti_type == NIFTI_FTYPE::NIFTI1_1) 
      strcpy(nhdr.magic, "n+1");
    else                                          
      strcpy(nhdr.magic, "ni1");

    nhdr.pixdim[1] = (float)fabs(nhdr.pixdim[1]);
    nhdr.pixdim[2] = (float)fabs(nhdr.pixdim[2]);
    nhdr.pixdim[3] = (float)fabs(nhdr.pixdim[3]);
    nhdr.pixdim[4] = (float)fabs(nhdr.pixdim[4]);
    nhdr.pixdim[5] = (float)fabs(nhdr.pixdim[5]);
    nhdr.pixdim[6] = (float)fabs(nhdr.pixdim[6]);
    nhdr.pixdim[7] = (float)fabs(nhdr.pixdim[7]);

    N_CHECK_2BYTE_VAL(intent_code);
    N_CHECK_2BYTE_VAL(qform_code);
    N_CHECK_2BYTE_VAL(sform_code);

    nhdr.intent_code = nim.intent_code;
    nhdr.intent_p1 = nim.intent_p1;
    nhdr.intent_p2 = nim.intent_p2;
    nhdr.intent_p3 = nim.intent_p3;
    if (nim.intent_name[0] != '\0') {
      memcpy(nhdr.intent_name, nim.intent_name, 15);
      nhdr.intent_name[15] = '\0';
    }

    nhdr.vox_offset = (float)nim.iname_offset;
    nhdr.xyzt_units = SPACE_TIME_TO_XYZT(nim.xyz_units, nim.time_units);
    nhdr.toffset = nim.toffset;

    if (nim.qform_code > 0) {
      nhdr.qform_code = nim.qform_code;
      nhdr.quatern_b = nim.quatern_b;
      nhdr.quatern_c = nim.quatern_c;
      nhdr.quatern_d = nim.quatern_d;
      nhdr.qoffset_x = nim.qoffset_x;
      nhdr.qoffset_y = nim.qoffset_y;
      nhdr.qoffset_z = nim.qoffset_z;
      nhdr.pixdim[0] = (nim.qfac >= 0.0) ? 1.0f : -1.0f;
    }
    /*Chris Rorden for fslmaths compatibility only: if qform code is 0, set qfac to 1, unused but makes resulting headers look like fsl*/
    else //this helps for regression testing between this library and fsl, there is no other purpose. Without this you get false alarms
      nhdr.pixdim[0] = 1.0; //default if unknown and not needed
     /*end */

    if (nim.sform_code > 0) {
      nhdr.sform_code = nim.sform_code;
      nhdr.srow_x[0] = nim.sto_xyz.m[0][0];
      nhdr.srow_x[1] = nim.sto_xyz.m[0][1];
      nhdr.srow_x[2] = nim.sto_xyz.m[0][2];
      nhdr.srow_x[3] = nim.sto_xyz.m[0][3];
      nhdr.srow_y[0] = nim.sto_xyz.m[1][0];
      nhdr.srow_y[1] = nim.sto_xyz.m[1][1];
      nhdr.srow_y[2] = nim.sto_xyz.m[1][2];
      nhdr.srow_y[3] = nim.sto_xyz.m[1][3];
      nhdr.srow_z[0] = nim.sto_xyz.m[2][0];
      nhdr.srow_z[1] = nim.sto_xyz.m[2][1];
      nhdr.srow_z[2] = nim.sto_xyz.m[2][2];
      nhdr.srow_z[3] = nim.sto_xyz.m[2][3];
    }

    N_CHECK_2BYTE_VAL(sform_code);

    nhdr.dim_info = FPS_INTO_DIM_INFO(nim.freq_dim,
      nim.phase_dim, nim.slice_dim);
    nhdr.slice_code = nim.slice_code;
  }

  memcpy(&hdr, &nhdr, sizeof(nhdr));

  return 0;
}
DISABLE_WARNING_POP

/* return number of extensions written, or -1 on error */
int mdm_NiftiFormat::nifti_write_extensions(znzFile fp, nifti_image &nim)
{
  nifti1_extension * list;
  char               extdr[4] = { 0, 0, 0, 0 };
  int                c, size, ok = 1;

  if (znz_isnull(fp) || nim.num_ext < 0)
    throw mdm_exception(__func__, "bad params");

  /* if invalid extension list, clear num_ext */
  if (!valid_nifti_extensions(nim)) nim.num_ext = 0;

  /* write out extender block */
  if (nim.num_ext > 0) extdr[0] = 1;
  if (nifti_write_buffer(fp, extdr, 4) != 4)
    throw mdm_exception(__func__, "failed to write extender");

  list = nim.ext_list;
  for (c = 0; c < nim.num_ext; c++) {
    size = (int)nifti_write_buffer(fp, &list->esize, sizeof(int));
    ok = (size == (int)sizeof(int));
    if (ok) {
      size = (int)nifti_write_buffer(fp, &list->ecode, sizeof(int));
      ok = (size == (int)sizeof(int));
    }
    if (ok) {
      size = (int)nifti_write_buffer(fp, list->edata, list->esize - 8);
      ok = (size == list->esize - 8);
    }

    if (!ok) 
      throw mdm_exception(__func__, "failed while writing extension " + std::to_string(c));

    list++;
  }

  return nim.num_ext;
}

/*----------------------------------------------------------------------*/
/*! for each extension, check code, size and data pointer
*//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::valid_nifti_extensions(const nifti_image &nim)
{
  nifti1_extension * ext;
  int                c, errs;

  if (nim.num_ext <= 0 || nim.ext_list == NULL) 
    return 0;

  /* for each extension, check code, size and data pointer */
  ext = nim.ext_list;
  errs = 0;
  for (c = 0; c < nim.num_ext; c++) 
  {
    if (ext->esize <= 0)
      errs++;
    
    else if (ext->esize & 0xf) 
      errs++;

    if (ext->edata == NULL) 
      errs++;

    ext++;
  }

  if (errs > 0) 
    return 0;

  /* if we're here, we're good */
  return 1;
}

/*----------------------------------------------------------------------*/
/*! write the nifti_image data to file (from nim.data or from NBL)

   If NBL is not NULL, write the data from that structure.  Otherwise,
   write it out from nim.data.  No swapping is done here.

   \param  fp  : File pointer
   \param  nim : nifti_image corresponding to the data
   \param  NBL : optional source of write data (if NULL use nim.data)

   \return 0 on success, -1 on failure

   Note: the nifti_image byte_order is set as that of the current CPU.
         This is because such a conversion was made to the data upon
         reading, while byte_order was not set (so the programs would
         know what format the data was on disk).  Effectively, since
         byte_order should match what is on disk, it should bet set to
         that of the current CPU whenever new filenames are assigned.
*//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_write_all_data(znzFile fp, nifti_image & nim)
{
  int64_t ss;

  /* just write one buffer and get out of here */
  if (nim.data == NULL) {
    throw mdm_exception(__func__, "no image data to write");
  }

  ss = nifti_write_buffer(fp, nim.data, nim.nbyper * nim.nvox);
  if (ss < nim.nbyper * nim.nvox) {
    throw mdm_exception(__func__, "wrote only   of  bytes to file");
  }

  /* mark as being in this CPU byte order */
  nim.byteorder = nifti_short_order();

  return 0;
}

/*--------------------------------------------------------------------------
 * nifti_write_buffer just check for a null znzFile and call znzwrite
 *--------------------------------------------------------------------------*/
 /*! \fn int64_t nifti_write_buffer(znzFile fp, void *buffer, int64_t numbytes)
     \brief write numbytes of buffer to file, fp

     \param fp           File pointer (from znzopen) to gzippable nifti datafile
     \param buffer       data buffer to be written
     \param numbytes     number of bytes in buffer to write
     \return number of bytes successfully written
 */
int64_t mdm_NiftiFormat::nifti_write_buffer(znzFile fp, const void *buffer, int64_t numbytes)
{
  /* Write all the image data at once (no swapping here) */
  int64_t ss;
  if (znz_isnull(fp)) {
    throw mdm_exception(__func__, "null file pointer");
  }
  ss = znzwrite((const void*)buffer, 1, numbytes, fp);
  return ss;
}

void  mdm_NiftiFormat::nifti_img_to_nii_transform(const mdm_Image3D& img, nifti_image& nim)
{
  //Convert the image position and orientation for Madym's 3D image meta data
  //to NIFTI's sform fields

  //Get the row (u) and column (v) axes vectors
  const auto& info = img.info();
  auto ux = info.rowDirCosX.value();
  auto uy = info.rowDirCosY.value();
  auto uz = info.rowDirCosZ.value();

  auto vx = info.colDirCosX.value();
  auto vy = info.colDirCosY.value();
  auto vz = info.colDirCosZ.value();

  //Compute the slice xes vector (w) as the cross-product of these
  //corrected by the z-Direction
  auto zdir = info.zDirection.value();
  auto wx = zdir * (uy * vz - uz * vy);
  auto wy = zdir * (uz * vx - ux * vz);
  auto wz = zdir * (ux * vy - uy * vx);

  //The z-direction also encodes the slice distance, so w is now scaled
  //but we need to scale U and V by the pixel dimesnions of the grid
  auto dx = info.Xmm.value();
  auto dy = info.Ymm.value();
  ux *= dx;
  uy *= dx;
  uz *= dx;

  vx *= dy;
  vy *= dy;
  vz *= dy;

  //The origin for NIFTI will be offset from the DICOM origin, depending
  //on whether the image was flipped in X/Y/Z when loaded from the DICOM slices
  //so account for these offsets
  auto offset_ux = info.flipX.value() ? ux * (nim.nx - 1) : 0;
  auto offset_uy = info.flipX.value() ? uy * (nim.nx - 1) : 0;
  auto offset_uz = info.flipX.value() ? uz * (nim.nx - 1) : 0;

  auto offset_vx = info.flipY.value() ? vx * (nim.ny - 1) : 0;
  auto offset_vy = info.flipY.value() ? vy * (nim.ny - 1) : 0;
  auto offset_vz = info.flipY.value() ? vz * (nim.ny - 1) : 0;

  auto offset_wx = info.flipZ.value() ? wx * (nim.nz - 1) : 0;
  auto offset_wy = info.flipZ.value() ? wy * (nim.nz - 1) : 0;
  auto offset_wz = info.flipZ.value() ? wz * (nim.nz - 1) : 0;

  auto tx = info.originX.value() + offset_ux + offset_vx + offset_wx;
  auto ty = info.originY.value() + offset_uy + offset_vy + offset_wy;
  auto tz = info.originZ.value() + offset_uz + offset_vz + offset_wz;

  //Flipping axes also changes their sign in the transform matrix
  auto sign_u = info.flipX.value() ? -1.0 : 1.0;
  auto sign_v = info.flipY.value() ? -1.0 : 1.0;
  auto sign_w = info.flipZ.value() ? -1.0 : 1.0;
  
  //Finally we can fill the transform matrix. DICOM uses LPS co-ordinates
  //while NIFTI uses RAS, so the top two rows are negated UNLESS X/Y
  //flips have been used.
  nim.sto_xyz.m[0][0] = -sign_u * ux;
  nim.sto_xyz.m[0][1] = -sign_v * vx;
  nim.sto_xyz.m[0][2] = -sign_w * wx;
  nim.sto_xyz.m[0][3] = -tx;

  nim.sto_xyz.m[1][0] = -sign_u * uy;
  nim.sto_xyz.m[1][1] = -sign_v * vy;
  nim.sto_xyz.m[1][2] = -sign_w * wy;
  nim.sto_xyz.m[1][3] = -ty;

  nim.sto_xyz.m[2][0] = sign_u * uz;
  nim.sto_xyz.m[2][1] = sign_v * vz;
  nim.sto_xyz.m[2][2] = sign_w * wz;
  nim.sto_xyz.m[2][3] = tz;

  //The last row is always [ 0 0 0 1 ]
  nim.sto_xyz.m[3][0] = nim.sto_xyz.m[3][1] = nim.sto_xyz.m[3][2] = 0.0f;
  nim.sto_xyz.m[3][3] = 1.0f;

  //Use the inbuilt function to create the S to IJK matrix
  nim.sto_ijk = nifti_dmat44_inverse(nim.sto_xyz);
  nim.sform_code = 1;

  //Complete du, dv and dw fields
  nim.du = dx;
  nim.dv = dy;
  nim.dw = std::abs(zdir);
}

void  mdm_NiftiFormat::nifti_nii_transform_to_img(const nifti_image& nim, mdm_Image3D& img)
{
  //Convert the image position and orientation from NIFTI's sform fields
  //to Madym's 3D image meta data

  //Compute the row (u) and column (v) axes vectors from the Sform matrix
  auto& info = img.info();
  auto dx = nim.dx;
  auto dy = nim.dy;
  
  //Flipping axes also changes their sign in the transform matrix
  auto sign_u = info.flipX.value() ? -1.0 : 1.0;
  auto sign_v = info.flipY.value() ? -1.0 : 1.0;
  auto sign_w = info.flipZ.value() ? -1.0 : 1.0;

  info.rowDirCosX.setValue(-sign_u * nim.sto_xyz.m[0][0] / dx);
  info.colDirCosX.setValue(-sign_v * nim.sto_xyz.m[0][1] / dy);

  info.rowDirCosY.setValue(-sign_u * nim.sto_xyz.m[1][0] / dx);
  info.colDirCosY.setValue(-sign_v * nim.sto_xyz.m[1][1] / dy);

  info.rowDirCosZ.setValue(sign_u * nim.sto_xyz.m[2][0] / dx);
  info.colDirCosZ.setValue(sign_v * nim.sto_xyz.m[2][1] / dy);
  
  //Transform the slice direction vector (Wxyz) too, even though
  //this isn't saved directly in the image meta-data
  auto wx = -sign_w * nim.sto_xyz.m[0][2];
  auto wy = -sign_w * nim.sto_xyz.m[1][2];
  auto wz = sign_w * nim.sto_xyz.m[2][2];
  auto dz = std::sqrt(wx*wx + wy*wy + wz*wz);
  wx /= dz;
  wy /= dz;
  wz /= dz;

  //Compute the cross-product of U and V to workout the z-directions
  //corrected by the z-Direction
  auto ux = info.rowDirCosX.value();
  auto uy = info.rowDirCosY.value();
  auto uz = info.rowDirCosZ.value();

  auto vx = info.colDirCosX.value();
  auto vy = info.colDirCosY.value();
  auto vz = info.colDirCosZ.value();

  auto cx = uy * vz - uz * vy;
  auto cy = uz * vx - ux * vz;
  auto cz = ux * vy - uy * vx;
  auto w_dot_c = cx * wx + cy * wy + cz * wz;
  auto zdir = w_dot_c > 0 ? dz : -dz;
  info.zDirection.setValue(zdir);

  //The origin for NIFTI will be offset from the DICOM origin, depending
  //on whether the image was flipped in X/Y/Z when loaded from the DICOM slices
  //so account for these offsets
  auto offset_ux = info.flipX.value() ? -dx * ux * (nim.nx - 1) : 0;
  auto offset_uy = info.flipX.value() ? -dx * uy * (nim.nx - 1) : 0;
  auto offset_uz = info.flipX.value() ? -dx * uz * (nim.nx - 1) : 0;

  auto offset_vx = info.flipY.value() ? -dy * vx * (nim.ny - 1) : 0;
  auto offset_vy = info.flipY.value() ? -dy * vy * (nim.ny - 1) : 0;
  auto offset_vz = info.flipY.value() ? -dy * vz * (nim.ny - 1) : 0;

  auto offset_wx = info.flipZ.value() ? -dz * wx * (nim.nz - 1) : 0;
  auto offset_wy = info.flipZ.value() ? -dz * wy * (nim.nz - 1) : 0;
  auto offset_wz = info.flipZ.value() ? -dz * wz * (nim.nz - 1) : 0;

  info.originX.setValue(-nim.sto_xyz.m[0][3] + offset_ux + offset_vx + offset_wx);
  info.originY.setValue(-nim.sto_xyz.m[1][3] + offset_uy + offset_vy + offset_wy);
  info.originZ.setValue(nim.sto_xyz.m[2][3] + offset_uz + offset_vz + offset_wz);
}

/*----------------------------------------------------------------------*/
/*! return the total volume size, in bytes

    This is computed as nvox * nbyper.
*//*--------------------------------------------------------------------*/
int64_t mdm_NiftiFormat::nifti_get_volsize(const nifti_image &nim)
{
  return (int64_t)nim.nbyper * nim.nvox; /* total bytes */
}

/*----------------------------------------------------------------------*/
/*! get the byte order for this CPU

    - LSB_FIRST means least significant byte, first (little endian)
    - MSB_FIRST means most significant byte, first (big endian)
*//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_short_order(void)   /* determine this CPU's byte order */
{
  union {
    unsigned char bb[2];
    short         ss;
  } fred;

  fred.bb[0] = 1; fred.bb[1] = 0;

  return (fred.ss == 1) ? LSB_FIRST : MSB_FIRST;
}