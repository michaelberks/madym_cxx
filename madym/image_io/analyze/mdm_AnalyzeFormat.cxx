/**
 *  @file    mdm_AnalyzeFormat.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_AnalyzeFormat.h"

#include <cstdio>
#include <cassert>
#include <iomanip>
#include <string>
#include <sstream>
#include <istream>
#include <ostream>

#include <boost/filesystem.hpp>

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

/* Added so we can deal with stuff */
const int mdm_AnalyzeFormat::MAX_ANALYZE_DIMS  = 8;
const int mdm_AnalyzeFormat::ANALYZE_HDR_SIZE  = 348;
const int mdm_AnalyzeFormat::MAX_IMG_DIMS = 4;

//
MDM_API mdm_Image3D mdm_AnalyzeFormat::readImage3D(const std::string &fileName,
	bool load_xtr)
{
	mdm_Image3D img;
  AnalyzeHdr  hdr;

  if (fileName.empty())
    throw mdm_exception(__func__, "Filename image must not be empty");

  std::string baseName = stripAnalyzeExtension(fileName);
  std::string hdrFileName = baseName + ".hdr";
  std::string imgFileName = baseName + ".img";
  std::string xtrFileName = baseName + ".xtr";

  
  if (!filesExist(baseName, false))
    throw mdm_exception(__func__, "Missing Analyze file " + baseName + ".hdr / img");

	if (load_xtr)
	{
		auto xtrExistsFlag = boost::filesystem::exists(xtrFileName);
		if (xtrExistsFlag)
			mdm_XtrFormat::readAnalyzeXtr(xtrFileName, img);
		else
			throw mdm_exception(__func__, "No xtr file matching " + hdrFileName);
	}

  // Files seem to exist, so let's start reading them ...
  readAnalyzeHdr(hdrFileName, hdr);

  // Check endian: sizeof_hdr is currently always 348 and so can be used
  // for this purpose.
  bool  swapFlag = false;
  if (hdr.header_key_.sizeof_hdr != ANALYZE_HDR_SIZE)
  {
    swapFlag = true;
    mdm_Image3D::swapBytes(hdr.header_key_.sizeof_hdr);
  }

  // Read and store the voxel matrix dimensions
  if (swapFlag)
    for (int dim = 0; dim <= MAX_IMG_DIMS; dim++)
      mdm_Image3D::swapBytes(hdr.dimensions_.dim[dim]);
    

  int nX = hdr.dimensions_.dim[1];
  int nY = hdr.dimensions_.dim[2];
  int nZ = hdr.dimensions_.dim[3];

  if (nX <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nX = %2%, should be strictly positive")
      % hdrFileName % nX);

  if (nY <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nY = %2%, should be strictly positive")
      % hdrFileName % nY);

  if (nZ <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, nX = %2%, should be strictly positive")
      % hdrFileName % nZ);

  if (hdr.dimensions_.dim[4] > 1)
    throw mdm_exception(__func__, baseName + " is 4D. We can only use 2D or 3D images");

  img.setDimensions(nX, nY, nZ);

  if (!img)
    throw mdm_exception(__func__, "Can't allocate voxel array for image " + imgFileName);


  // Read and store the voxel mm dimensions
  if (swapFlag)
    for (int dim = 0; dim <= MAX_IMG_DIMS; dim++)
      mdm_Image3D::swapBytes(hdr.dimensions_.pixdim[dim]);
    
  
  double xmm = hdr.dimensions_.pixdim[1];
  double ymm = hdr.dimensions_.pixdim[2];
  double zmm = hdr.dimensions_.pixdim[3];
  if (xmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, xmm = %2%, should be strictly positive")
      % hdrFileName % xmm);

  if (ymm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, ymm = %2%, should be strictly positive")
      % hdrFileName % ymm);

  if (zmm <= 0)
    throw mdm_exception(__func__, boost::format(
      "Error reading %1%, zmm = %2%, should be strictly positive")
      % hdrFileName % zmm);

  img.setVoxelDims(xmm, ymm, zmm);

  // We need to do this because datatype is used in readAnalyzeImg()
  if (swapFlag)
    mdm_Image3D::swapBytes(hdr.dimensions_.datatype);

  readAnalyzeImg(imgFileName, img, hdr, swapFlag);

	return img;
}

//
MDM_API void mdm_AnalyzeFormat::writeImage3D(const std::string &fileName,
	const mdm_Image3D &img,
	const mdm_ImageDatatypes::DataType dataTypeFlag, const mdm_XtrFormat::XTR_type xtrTypeFlag,
	bool sparse)
{
  auto baseName = stripAnalyzeExtension(fileName);

  if (baseName.empty())
    throw mdm_exception(__func__, "Basename for writing image must not be empty");

  if (!img)
    throw mdm_exception(__func__, "Image for writing image must not be empty");

	// Ensure all hdr fields have been initialised, set the req'd fields from
	// img and the req'd data type fields and write hdr to file.
  AnalyzeHdr  hdr;
	hdrBlankInit(hdr);
	setHdrFieldsFromImage3D(hdr, img, dataTypeFlag, sparse);
  writeAnalyzeHdr(baseName, hdr);

	//TODO: we don't bother writing scaling any more. I can't see the point
	//and just causes hassle
	hdr.dimensions_.roi_scale = 1.0;

	//Write analyze now takes care of different output types
	writeAnalyzeImg(baseName, img, dataTypeFlag, sparse);

	// Write *.xtr file
	if (xtrTypeFlag != mdm_XtrFormat::NO_XTR)
    mdm_XtrFormat::writeAnalyzeXtr(baseName, img, xtrTypeFlag);

}

//
bool mdm_AnalyzeFormat::filesExist(const std::string & baseName,
  bool warn)
{

  assert(!baseName.empty());

  std::string hdrName = baseName + ".hdr";
  if (!boost::filesystem::exists(hdrName))
  {
    if (warn)
      mdm_ProgramLogger::logProgramWarning(
        __func__, hdrName + " does not exist");
    return false;
  }


  std::string imgName = baseName + ".img";
  if (!boost::filesystem::exists(imgName))
  {
    if (warn)
      mdm_ProgramLogger::logProgramWarning(
        __func__, imgName + " does not exist");
    return false;
  }

  return true;
}

//**********************************************************************
//Private 
//**********************************************************************
//

//
std::string mdm_AnalyzeFormat::stripAnalyzeExtension(const std::string & fileName)
{

  assert(!fileName.empty());
  auto p = boost::filesystem::path(fileName).replace_extension();
  return p.string();
}

//
void mdm_AnalyzeFormat::writeAnalyzeHdr(const std::string &baseName,
                       const AnalyzeHdr &hdr)
{
  std::string  hdrFileName = baseName + ".hdr";

  // Open Analyze header file stream for writing
	std::ofstream hdrFileStream(hdrFileName, std::ios::out | std::ios::binary);
  if (!hdrFileStream)
    throw mdm_exception(__func__, "Can't open Analyze header file " + hdrFileName);
    

	//Write the binary stream ***CHECK THIS LINE****
	hdrFileStream.write((char*)&hdr, sizeof(hdr));

  if (!hdrFileStream.good())
    throw mdm_exception(__func__, "Can't write Analyze header values to file " + hdrFileName);
    
	hdrFileStream.close();

  if (hdrFileStream.is_open())
    throw mdm_exception(__func__, "Failed to close Analyze header file " + hdrFileName);
   
}

//
void mdm_AnalyzeFormat::writeAnalyzeImg(const std::string &baseName,
	const mdm_Image3D& img,
	const mdm_ImageDatatypes::DataType typeFlag,
	bool sparse)
{
  
  assert(!baseName.empty());
  std::string imgFileName = baseName + ".img";

  //Open Analyze image file stream for writing
	std::ofstream imgFileStream(imgFileName, std::ios::out | std::ios::binary);
  if (!imgFileStream)
    throw mdm_exception(__func__, "Can't open Analyze image file " + imgFileName);

  //Based on the desired output type, call templated version
	//of images toBinaryStream function
  switch (typeFlag)
  {
  case mdm_ImageDatatypes::DT_UNSIGNED_CHAR:
		img.toBinaryStream<char>(imgFileStream, sparse);
		break;
	case mdm_ImageDatatypes::DT_SIGNED_SHORT:
		img.toBinaryStream<short>(imgFileStream, sparse);
    break;
  case mdm_ImageDatatypes::DT_SIGNED_INT:
		img.toBinaryStream<int>(imgFileStream, sparse);
    break;
  case mdm_ImageDatatypes::DT_FLOAT:
		img.toBinaryStream<float>(imgFileStream, sparse);
    break;
	case mdm_ImageDatatypes::DT_DOUBLE:
		img.toBinaryStream<double>(imgFileStream, sparse);
		break;
  default:
    throw mdm_exception(__func__, "Analyze data type unsupported - " + imgFileName);
    break;
  }
  // TEST ALSO FOR fclose() ERROR
	imgFileStream.close();
  if (imgFileStream.is_open())
    throw mdm_exception(__func__, "Failed to close Analyze image file " + imgFileName);
    
}

//
void mdm_AnalyzeFormat::readAnalyzeImg(const std::string &imgFileName,
	mdm_Image3D &img,
	const AnalyzeHdr &hdr,
	const bool swapFlag)
{
	// Open Analyze image file stream for reading
	std::ifstream imgFileStream(imgFileName, std::ios::in | std::ios::binary);
	if (!imgFileStream)
    throw mdm_exception(__func__, "Can't open Analyze image file " + imgFileName);

	//Get datatype, if it's odd or equal to 6, it's our sparse
	//format, with 5 added to the datatype
	bool sparse = false;
	int datatype = hdr.dimensions_.datatype;
	if (datatype==6 || datatype % 2)
	{
		datatype -= 5;
		sparse = true;
	}

	//Now call images fromBinaryStream method, templated on the
	//datatype
  try
  {
    switch (datatype)
    {
    case mdm_ImageDatatypes::DT_UNSIGNED_CHAR:
      img.fromBinaryStream<char>(imgFileStream,
        sparse, swapFlag);
      break;

    case mdm_ImageDatatypes::DT_SIGNED_SHORT:
      img.fromBinaryStream<short>(imgFileStream,
        sparse, swapFlag);
      break;
    case mdm_ImageDatatypes::DT_SIGNED_INT:
      img.fromBinaryStream<int>(imgFileStream,
        sparse, swapFlag);
      break;
    case mdm_ImageDatatypes::DT_FLOAT:
      img.fromBinaryStream<float>(imgFileStream,
        sparse, swapFlag);
      break;
    case mdm_ImageDatatypes::DT_DOUBLE:
      img.fromBinaryStream<double>(imgFileStream,
        sparse, swapFlag);
      break;

    default:
      throw mdm_exception(__func__, "Analyze data type unsupported");
    }
  }
	catch (mdm_exception &e)
  {
    e.append("Failed to read Analyze image file data - " + imgFileName);
    throw;
	}
}

//
void mdm_AnalyzeFormat::readAnalyzeHdr(const std::string &hdrFileName,
	AnalyzeHdr &hdr)
{

	assert(!hdrFileName.empty());

	// Open Analyze header file stream for reading
	std::ifstream hdrFileStream(hdrFileName, std::ios::in | std::ios::binary);
  if (!hdrFileStream)
    throw mdm_exception(__func__, "Can't open Analyze header file " + hdrFileName);
	
	// Read values from header file
	size_t expectedSize = sizeof(hdr);
	hdrFileStream.read((char*)&hdr, expectedSize);
	size_t nRead = hdrFileStream.gcount();
	if (hdrFileStream.fail() || nRead != expectedSize)
    throw mdm_exception(__func__, "Can't read Analyze header values " + hdrFileName);
		
	hdrFileStream.close();

	if (hdrFileStream.is_open())
    throw mdm_exception(__func__, "Failed to close Analyze header file " + hdrFileName);
		
}

//
//----------------------------------------------------------------------------
void  mdm_AnalyzeFormat::setHdrFieldsFromImage3D(AnalyzeHdr &hdr,
	const mdm_Image3D &img,
	const int typeFlag,
	bool sparse)
{
	size_t   nX, nY, nZ;

	if(hdr.header_key_.sizeof_hdr != 348)
    throw mdm_exception(__func__, "Header key must have size 348 bytes. Cannot process Analyze hdr");

	img.getDimensions(nX, nY, nZ);
	hdr.header_key_.extents = (int)(nX * nY);

	hdr.dimensions_.dim[0] = (short)4;
	hdr.dimensions_.dim[1] = (short)nX;
	hdr.dimensions_.dim[2] = (short)nY;
	hdr.dimensions_.dim[3] = (short)nZ;
	hdr.dimensions_.dim[4] = (short)1;

	hdr.dimensions_.pixdim[0] = (float) 4.0;
	hdr.dimensions_.pixdim[1] = (float)img.info().Xmm.value();
	hdr.dimensions_.pixdim[2] = (float)img.info().Ymm.value();
	hdr.dimensions_.pixdim[3] = (float)img.info().Zmm.value();
	switch (typeFlag)
	{
	case mdm_ImageDatatypes::DT_UNSIGNED_CHAR:
		hdr.dimensions_.datatype = (short)mdm_ImageDatatypes::DT_UNSIGNED_CHAR;
		hdr.dimensions_.bitpix = (short) sizeof(char) * 8;
		break;
	case mdm_ImageDatatypes::DT_SIGNED_SHORT:
		hdr.dimensions_.datatype = (short)mdm_ImageDatatypes::DT_SIGNED_SHORT;
		hdr.dimensions_.bitpix = (short) sizeof(short) * 8;
		break;
	case mdm_ImageDatatypes::DT_SIGNED_INT:
		hdr.dimensions_.datatype = (short)mdm_ImageDatatypes::DT_SIGNED_INT;
		hdr.dimensions_.bitpix = (short) sizeof(int) * 8;
		break;
	case mdm_ImageDatatypes::DT_FLOAT:
		hdr.dimensions_.datatype = (short)mdm_ImageDatatypes::DT_FLOAT;
		hdr.dimensions_.bitpix = (short) sizeof(float) * 8;
		break;
	case mdm_ImageDatatypes::DT_DOUBLE:
		hdr.dimensions_.datatype = (short)mdm_ImageDatatypes::DT_DOUBLE;
		hdr.dimensions_.bitpix = (short) sizeof(double) * 8;
		break;
	default:
		// TODO SEND MESSAGE "TYPE NOT SUPPORTED FOR O/P" TO ERROR LOG
		break;
	}

	//For sparse writing, add 5 to the data type
	if (sparse)
		hdr.dimensions_.datatype += 5;
}

//
void  mdm_AnalyzeFormat::hdrBlankInit(AnalyzeHdr &hdr)
{
	
	hdr.header_key_.sizeof_hdr = (int) sizeof(struct AnalyzeHdr);       /* Should be 348 */

	for (auto &el : hdr.header_key_.data_type)//i = 0; i < 10; i++
		el = '\0';

	for (auto &el : hdr.header_key_.db_name) //18
		el = '\0';

	hdr.header_key_.extents = (int)0;
	hdr.header_key_.session_error = (short)0;
	hdr.header_key_.regular = 'r';
	hdr.header_key_.hkey_un0 = ' ';

	for (auto &el : hdr.dimensions_.dim)
		el = (short)0;
	
  for (auto &el : hdr.dimensions_.vox_units)
    el = '\0';
	hdr.dimensions_.vox_units[0] = 'm';
  hdr.dimensions_.vox_units[1] = 'm';

	for (auto &el : hdr.dimensions_.cal_units)
		el = '\0';

	hdr.dimensions_.unused1 = (short)0;
	hdr.dimensions_.datatype = (short)mdm_ImageDatatypes::DT_UNKNOWN;
	hdr.dimensions_.bitpix = (short)0;
	hdr.dimensions_.dim_un0 = (short)0;
	for (auto &el : hdr.dimensions_.pixdim)
		el = (float)0;
	hdr.dimensions_.vox_offset = (float) 0.0;
	// This is where mricro expects to find a scale factor
	hdr.dimensions_.roi_scale = (float) 1.0;
	hdr.dimensions_.funused1 = (float) 0.0;
	hdr.dimensions_.funused2 = (float) 0.0;
	hdr.dimensions_.cal_max = (float) 0.0;
	hdr.dimensions_.cal_min = (float) 0.0;
	hdr.dimensions_.compressed = (int) 0.0;
	hdr.dimensions_.verified = (int) 0.0;
	hdr.dimensions_.glmax = (int)0;
	hdr.dimensions_.glmin = (int)0;

	for (auto &el : hdr.history_.descrip)
		el = '\0';
	for (auto &el : hdr.history_.aux_file)
		el = '\0';
	hdr.history_.orient = (char)0;
	for (auto &el : hdr.history_.originator)
		el = '\0';
	for (auto &el : hdr.history_.generated)
		el = '\0';
	for (auto &el : hdr.history_.scannum)
		el = '\0';
	for (auto &el : hdr.history_.patient_id)
		el = '\0';
	for (auto &el : hdr.history_.exp_date)
		el = '\0';
	for (auto &el : hdr.history_.exp_time)
		el = '\0';
	for (auto &el : hdr.history_.hist_un0)
		el = '\0';
	hdr.history_.views = (int)0;
	hdr.history_.vols_added = (int)0;
	hdr.history_.start_field = (int)0;
	hdr.history_.field_skip = (int)0;
	hdr.history_.omax = (int)0;
	hdr.history_.omin = (int)0;
	hdr.history_.smax = (int)0;
	hdr.history_.smin = (int)0;
}

//
void  mdm_AnalyzeFormat::hdrToString(std::string &hdrString, const AnalyzeHdr &hdr)
{

	// Use C++ string stream class for this
	std::stringstream ss;

	ss <<
		"qbiAnalyzeHdr:   header struct of size " << hdr.header_key_.sizeof_hdr << " at location " << &hdr << "\n" <<
		"the voxel matrix is " << hdr.dimensions_.dim[1] << " x " << hdr.dimensions_.dim[2] << " x " << hdr.dimensions_.dim[3] <<
		", with dimensions " << hdr.dimensions_.pixdim[1] << " x " << hdr.dimensions_.pixdim[2] << " x " << hdr.dimensions_.pixdim[3] << " " << hdr.dimensions_.vox_units << "\n" <<
		"the offset is " << hdr.dimensions_.vox_offset << ", the image extents " << hdr.header_key_.extents << ", and the scale factor " << hdr.dimensions_.roi_scale << "\n" <<
		"the data type is " << hdr.dimensions_.datatype << ", i.e. " << hdr.dimensions_.bitpix << " bits per pixel\n";
	hdrString = ss.str();
}

//
