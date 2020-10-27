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

#include <madym/mdm_ProgramLogger.h>

/* Added so we can deal with stuff */
const int mdm_AnalyzeFormat::MAX_ANALYZE_DIMS  = 8;
const int mdm_AnalyzeFormat::ANALYZE_HDR_SIZE  = 348;
const int mdm_AnalyzeFormat::MAX_IMG_DIMS = 4;

static const int MAX_BYTES = 16;

void swapBytes(char *data, int nBytes)
{
	int   i;
	char  temp[MAX_BYTES];

	assert(data != NULL);
	assert(nBytes != 0);

	for (i = 0; i < nBytes; i++)
		temp[i] = data[i];

	for (i = 0; i < nBytes; i++)
		data[i] = temp[nBytes - i - 1];
}

//
MDM_API bool mdm_AnalyzeFormat::readImage3D(const std::string & fileName, mdm_Image3D &img,
	bool load_xtr)
{
	std::string me = "mdm_AnalyzeFormat::readImage3D";

	int    iDim;
	int    nX = 1, nY = 1, nZ = 1;
	double  xmm = 0.0, ymm = 0.0, zmm = 0.0;

	struct dsr  hdr;

	bool  swapFlag = false;
	bool  xtrExistsFlag = true;
	assert(!fileName.empty());

	std::string baseName = stripAnalyzeExtension(fileName);
	std::string hdrFileName = baseName + ".hdr";
	std::string imgFileName = baseName + ".img";
	std::string xtrFileName = baseName + ".xtr";
	if (!filesExist(baseName, xtrExistsFlag))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: " + me + ":  Missing Analyze file " + baseName + ".hdr/img\n");
		return false;
	}

	// Files seem to exist, so let's start reading them ...
	if (!readAnalyzeHdr(hdrFileName, &hdr))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: " + me + ":  error reading Analyze file " + baseName + ".hdr/img\n");
		return false;
	}

	// Check endian: sizeof_hdr is currently always 348 and so can be used
	// for this purpose.
	if (hdr.hk.sizeof_hdr != ANALYZE_HDR_SIZE)
	{
		swapFlag = true;
		swapBytes((char *)&hdr.hk.sizeof_hdr, sizeof(hdr.hk.sizeof_hdr));
	}

	// Read and store the voxel matrix dimensions
	if (swapFlag)
	{
		for (iDim = 0; iDim <= MAX_IMG_DIMS; iDim++)
		{
			swapBytes((char *)&hdr.dime.dim[iDim], sizeof(hdr.dime.dim[iDim]));
		}
	}
	nX = (int)hdr.dime.dim[1];
	nY = (int)hdr.dime.dim[2];
	if (hdr.dime.dim[3] >= 1)
		nZ = (int)hdr.dime.dim[3];
	if (hdr.dime.dim[4] > 1)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: " + me + ": " + baseName + " is 4D. We can only use 2D or 3D images\n");
		return false;
	}
	img.setDimensions(nX, nY, nZ); //This now resizes the data array

	if (img.numVoxels() <= 0)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: " + me + ": Can't allocate voxel array for image " + imgFileName + "\n");
		return false;
	}

	// Read and store the voxel mm dimensions
	if (swapFlag)
	{
		for (iDim = 0; iDim <= MAX_IMG_DIMS; iDim++)
		{
			swapBytes((char *)&hdr.dime.pixdim[iDim], sizeof(hdr.dime.pixdim[iDim]));
		}
	}
	xmm = (double)hdr.dime.pixdim[1];
	ymm = (double)hdr.dime.pixdim[2];
	if (hdr.dime.dim[3] >= 1)
		zmm = (double)hdr.dime.pixdim[3];
	img.setVoxelDims(xmm, ymm, zmm);

	// We need to do this because datatype is used in readAnalyzeImg()
	if (swapFlag)
		swapBytes((char *)&hdr.dime.datatype, sizeof(hdr.dime.datatype));
	if (!readAnalyzeImg(imgFileName, img, &hdr, swapFlag))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: " + me + ": Can't read Analyze img file " + imgFileName + "\n");
		return false;
	}

	if (load_xtr)
	{
		if (xtrExistsFlag)
			readAnalyzeXtr(xtrFileName, img);
		else
		{
			mdm_ProgramLogger::logProgramMessage(
				"ERROR: " + me + ": No xtr file matching " + hdrFileName + "\n");
		}
	}

	return true;
}

/*
*/
MDM_API mdm_Image3D mdm_AnalyzeFormat::readImage3D(const std::string &fileName,
	bool load_xtr)
{
	assert(!fileName.empty());

	mdm_Image3D img;
	if (!readImage3D(fileName, img, load_xtr))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readImg : returning empty image\n");
	}

	return img;
}

/*
*/
MDM_API bool mdm_AnalyzeFormat::writeImage3D(const std::string &baseName,
	const mdm_Image3D &img,
	const Data_type dataTypeFlag, const XTR_type xtrTypeFlag,
	bool sparse)
{
	//int   nVoxels = img.numVoxels();

	struct dsr  hdr;
	assert(!baseName.empty());

	// Ensure all hdr fields have been initialised, set the req'd fields from
	// img and the req'd data type fields and write hdr to file.
	hdrBlankInit(&hdr);
	setHdrFieldsFromImage3D(&hdr, img, dataTypeFlag, sparse);
	if (!writeAnalyzeHdr(baseName, &hdr))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeImage3D: "
			"Failed to write Analyze header to " + baseName + "\n");
		return false;
	}

	//TODO: we don't bother writing scaling any more. I can't see the point
	//and just causes hassle
	hdr.dime.roi_scale = 1.0;

	//Write analyze now takes care of different output types
	if (!writeAnalyzeImg(baseName, img, dataTypeFlag, sparse))
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeImage3D: "
			"Failed to write output to " + baseName + "\n");
		return false;
	}

	// Write *.xtr files only if info has been set (default values are all NaN)
	// But ignore TE for now ... GAB 5 June 2007
	if (xtrTypeFlag != NO_XTR)
	{
		writeAnalyzeXtr(baseName, img, xtrTypeFlag);
	}

	return true;
}

/*
*/
MDM_API std::string mdm_AnalyzeFormat::stripAnalyzeExtension(const std::string & fileName)
{

	assert(!fileName.empty());

	return (boost::filesystem::path(fileName).parent_path() /
		boost::filesystem::path(fileName).stem()).string();
}

/*
*/
MDM_API bool mdm_AnalyzeFormat::filesExist(const std::string & baseName, bool &xtrExistsFlag,
	bool warn)
{

	assert(!baseName.empty());
	xtrExistsFlag = false;

	std::string fileName = baseName + ".hdr";
	if (!boost::filesystem::exists(fileName))
	{
		if (warn)
			mdm_ProgramLogger::logProgramMessage(
				"WARNING: mdm_AnalyzeFormat::filesExist: "
				+ fileName + " does not exist");
		return false;
	}


	fileName = baseName + ".img";
	if (!boost::filesystem::exists(fileName))
	{
		if (warn)
			mdm_ProgramLogger::logProgramMessage(
				"WARNING: mdm_AnalyzeFormat::filesExist: "
				+ fileName + " does not exist");
		return false;
	}

	fileName = baseName + ".xtr";
	if (!boost::filesystem::exists(fileName))
	{
		xtrExistsFlag = false;
	}
	else
		xtrExistsFlag = true;

	return true;
}


//**********************************************************************
//Private 
//**********************************************************************
//
bool mdm_AnalyzeFormat::writeNewXtr(std::ofstream *xtrFileStream,
	const mdm_Image3D &img)
{
  assert(xtrFileStream);
  assert(img.numVoxels());

	std::vector<std::string> keys;
	std::vector<double> values;
	img.getSetKeyValuePairs(keys, values);

	for (int i = 0; i < keys.size(); i++)
	{
		if (keys[i] == img.info_.TimeStamp.key())
			*xtrFileStream << keys[i] << "\t"
			<< std::fixed << std::setw(11) << std::setprecision(6) << img.timeStamp() << std::endl;
		else
			*xtrFileStream << keys[i] << "\t" << values[i] << std::endl;
	}
  return true;
}

//
bool mdm_AnalyzeFormat::writeOldXtr(std::ofstream *xtrFileStream,
	const mdm_Image3D &img)
{
  int   hrs, mins;
  double secs;

  assert(xtrFileStream);
  assert(img.numVoxels());

  /* Convert and write values from extra info to file */
	double timeStamp = img.timeStamp();
  hrs  = (int) (timeStamp / 10000);
  mins = (int) ((timeStamp - ((double) hrs * 10000.0)) / 100);
  secs = timeStamp - ((double) hrs * 10000.0) - ((double) mins * 100.0);

  /* TEST FOR WRITE ERRORS */
	*xtrFileStream << "voxel dimensions" << ":\t" 
		<< img.info_.Xmm.value() << " " 
		<< img.info_.Ymm.value() << " "
		<< img.info_.Zmm.value() << std::endl;
	*xtrFileStream << "flip angle" << ":\t" << img.info_.flipAngle.value() << std::endl;
	*xtrFileStream << "TR" << ":\t" << img.info_.TR.value() << std::endl;
	*xtrFileStream << "timestamp" << ":\t" << hrs << " " << mins << " " << secs << " " << timeStamp << std::endl;

  return true;
}

//
bool mdm_AnalyzeFormat::writeAnalyzeXtr(const std::string &baseName,
                      const mdm_Image3D &img,
                      const XTR_type typeFlag)
{
  assert(!baseName.empty());
  assert(img.numVoxels());

  std::string xtrFileName = baseName + ".xtr";

  // Open Analyze header file stream for reading
	std::ofstream xtrFileStream(xtrFileName.c_str(), std::ios::out);
  if (!xtrFileStream)
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeXtr: "
			"Can't open Analyze extra info file" +  xtrFileName + "\n");
    return false;
  }

  // Write values to extra info file
  if (typeFlag == OLD_XTR)
		mdm_AnalyzeFormat::writeOldXtr(&xtrFileStream, img);
  else
		mdm_AnalyzeFormat::writeNewXtr(&xtrFileStream, img);

	xtrFileStream.close();
  if (xtrFileStream.is_open())
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeXtr: "
			"Failed to close Analyze extra info file " + xtrFileName + "\n");
    return false;
  }

  return true;
}

//
bool mdm_AnalyzeFormat::writeAnalyzeHdr(const std::string &baseName,
                       const struct dsr *const hdr)
{
  assert(!baseName.empty());
  assert(baseName[0] != '\0');
  assert(hdr != NULL);

	std::string  hdrFileName = baseName + ".hdr";

  // Open Analyze header file stream for writing
	std::ofstream hdrFileStream(hdrFileName, std::ios::out | std::ios::binary);
  if (!hdrFileStream)
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeHdr: "
			"Can't open Analyze header file " + hdrFileName + "\n");
    return false;
  }

	//Write the binary stream ***CHECK THIS LINE****
	hdrFileStream.write((char*)hdr, sizeof(*hdr));

  if (!hdrFileStream.good())
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeHdr: "
			"Can't write Analyze header values to file " + hdrFileName + "\n");
    return false;
  }
	hdrFileStream.close();

  if (hdrFileStream.is_open())
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeHdr: "
			"Failed to close Analyze header file " + hdrFileName + "\n");
    return false;
  }

  return true;
}

//
bool mdm_AnalyzeFormat::writeAnalyzeImg(const std::string &baseName,
	const mdm_Image3D& img,
	const Data_type typeFlag,
	bool sparse)
{
  
  assert(!baseName.empty());
  std::string imgFileName = baseName + ".img";

  //Open Analyze image file stream for writing
	std::ofstream imgFileStream(imgFileName, std::ios::out | std::ios::binary);
  if (!imgFileStream)
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeImg: "
			"Can't open Analyze image file " + imgFileName + "\n");
    return false;
  }

  //Based on the desired output type, call templated version
	//of images toBinaryStream function
  switch (typeFlag)
  {
	case DT_UNSIGNED_CHAR:
		img.toBinaryStream<char>(imgFileStream, sparse);
		break;
	case DT_SIGNED_SHORT:
		img.toBinaryStream<short>(imgFileStream, sparse);
    break;
  case DT_SIGNED_INT:
		img.toBinaryStream<int>(imgFileStream, sparse);
    break;
  case DT_FLOAT:
		img.toBinaryStream<float>(imgFileStream, sparse);
    break;
	case DT_DOUBLE:
		img.toBinaryStream<double>(imgFileStream, sparse);
		break;
  default:
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeImg: "
			"Analyze data type unsupported - " + imgFileName + "\n");
    break;
  }
  // TEST ALSO FOR fclose() ERROR
	imgFileStream.close();
  if (imgFileStream.is_open())
  {
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::writeAnalyzeImg: "
			"Failed to close Analyze image file " + imgFileName + "\n");
    return false;
  }

  return true;
}

//
bool mdm_AnalyzeFormat::readAnalyzeImg(const std::string &imgFileName,
	mdm_Image3D &img,
	const struct dsr *const hdr,
	const bool swapFlag)
{
	// Open Analyze image file stream for reading
	std::ifstream imgFileStream(imgFileName, std::ios::in | std::ios::binary);
	if (!imgFileStream)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeImg: "
			"Can't open Analyze image file " + imgFileName + "\n");
		return false;
	}

	//Get datatype, if it's odd or equal to 6, it's our sparse
	//format, with 5 added to the datatype
	bool sparse = false;
	int datatype = hdr->dime.datatype;
	if (datatype==6 || datatype % 2)
	{
		datatype -= 5;
		sparse = true;
	}

	//Now call images fromBinaryStream method, templated on the
	//datatype
	bool success;
	switch (datatype)
	{
	case DT_UNSIGNED_CHAR:
		success = img.fromBinaryStream<char>(imgFileStream,
			sparse, swapFlag);
		break;

	case DT_SIGNED_SHORT:
		success = img.fromBinaryStream<short>(imgFileStream,
			sparse, swapFlag);
		break;
	case DT_SIGNED_INT:
		success = img.fromBinaryStream<int>(imgFileStream,
			sparse, swapFlag);
		break;
	case DT_FLOAT:
		success = img.fromBinaryStream<float>(imgFileStream,
			sparse, swapFlag);
		break;
	case DT_DOUBLE:
		success = img.fromBinaryStream<double>(imgFileStream,
			sparse, swapFlag);
		break;

	default:
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeImg: "
			"Analyze data type unsupported - " + imgFileName + "\n");
		return false;
	}
	if (!success)
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeImg: "
			"Failed to read Analyze image file data - " + imgFileName + "\n");

	return success;
}

//
bool mdm_AnalyzeFormat::readAnalyzeHdr(const std::string &hdrFileName,
	struct dsr *const hdr)
{

	assert(!hdrFileName.empty());
	assert(hdr != NULL);

	// Open Analyze header file stream for reading
	std::ifstream hdrFileStream(hdrFileName, std::ios::in | std::ios::binary);
	if (!hdrFileStream)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeHdr: "
			"Can't open Analyze header file " + hdrFileName + "\n");
		return false;
	}
	// Read values from header file
	size_t expectedSize = sizeof(*hdr);
	hdrFileStream.read((char*)hdr, expectedSize);
	size_t nRead = hdrFileStream.gcount();
	if (hdrFileStream.fail() || nRead != expectedSize)
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeHdr: "
			"Can't read Analyze header values " + hdrFileName + "\n");
		return false;
	}
	hdrFileStream.close();
	if (hdrFileStream.is_open())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeHdr: "
			"Failed to close Analyze header file " + hdrFileName + "\n");
		return false;
	}

	return true;
}

//
bool mdm_AnalyzeFormat::readOldXtr(std::ifstream *xtrFileStream,
	mdm_Image3D &img)
{
	assert(xtrFileStream->is_open());

	/* Read values from extra info file */
	/* TEST FOR READ ERRORS */
	std::string str;
	double f;
	*xtrFileStream >> str >> str >> f >> f >> f;
	*xtrFileStream >> str >> str >> f;
	img.info_.flipAngle.setValue(f);
	*xtrFileStream >> str >> f;
	img.info_.TR.setValue(f);
	*xtrFileStream >> str >> f >> f >> f >> f;
	img.setTimeStamp(f);

	return true;
}

//
bool mdm_AnalyzeFormat::readNewXtr(std::ifstream *xtrFileStream,
	mdm_Image3D &img)
{
	std::vector<std::string> keys;
	std::vector<double> values;

	assert(xtrFileStream->is_open());

	/* Read values from extra info file */
	/* TEST FOR READ ERRORS */
	std::string str;
	double f;
	while (!xtrFileStream->eof())
	{
		*xtrFileStream >> str >> f;
		keys.push_back(str);
		values.push_back(f);
	}
	img.setMetaData(keys, values);

	return true;
}

//
bool mdm_AnalyzeFormat::readAnalyzeXtr(const std::string &xtrFileName,
	mdm_Image3D &img)
{


	std::string firstString;
	
	assert(!xtrFileName.empty());

	std::ifstream xtrFileStream(xtrFileName, std::ios::in);
	/* Open Analyze header file stream for reading */
	if (!xtrFileStream.is_open())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeXtr: "
			"Can't open Analyze extra info file " + xtrFileName + "\n");
		return false;
	}

	/* Read values from extra info file */
	xtrFileStream >> firstString;
	xtrFileStream.seekg(0);
	if ((firstString == "voxel") || (firstString == "Voxel"))
	{
		readOldXtr(&xtrFileStream, img);
	}
	else
	{
		readNewXtr(&xtrFileStream, img);
	}

	xtrFileStream.close();
	if (xtrFileStream.is_open())
	{
		mdm_ProgramLogger::logProgramMessage(
			"ERROR: mdm_AnalyzeFormat::readAnalyzeHdr: "
			"Failed to close Analyze extra info file " + xtrFileName + "\n");
		return false;
	}

	return true;
}

//
//----------------------------------------------------------------------------
void  mdm_AnalyzeFormat::setHdrFieldsFromImage3D(struct dsr *const hdr,
	const mdm_Image3D img,
	const int typeFlag,
	bool sparse)
{
	int   nX, nY, nZ;

	assert(hdr != NULL);
	assert(hdr->hk.sizeof_hdr == 348);

	img.getDimensions(nX, nY, nZ);
	hdr->hk.extents = (int)(nX * nY);

	hdr->dime.dim[0] = (short)4;
	hdr->dime.dim[1] = (short)nX;
	hdr->dime.dim[2] = (short)nY;
	hdr->dime.dim[3] = (short)nZ;
	hdr->dime.dim[4] = (short)1;

	hdr->dime.pixdim[0] = (double) 4.0;
	hdr->dime.pixdim[1] = (double)img.info_.Xmm.value();
	hdr->dime.pixdim[2] = (double)img.info_.Ymm.value();
	hdr->dime.pixdim[3] = (double)img.info_.Zmm.value();
	switch (typeFlag)
	{
	case DT_UNSIGNED_CHAR:
		hdr->dime.datatype = (short)DT_UNSIGNED_CHAR;
		hdr->dime.bitpix = (short) sizeof(char) * 8;
		break;
	case DT_SIGNED_SHORT:
		hdr->dime.datatype = (short)DT_SIGNED_SHORT;
		hdr->dime.bitpix = (short) sizeof(short) * 8;
		break;
	case DT_SIGNED_INT:
		hdr->dime.datatype = (short)DT_SIGNED_INT;
		hdr->dime.bitpix = (short) sizeof(int) * 8;
		break;
	case DT_FLOAT:
		hdr->dime.datatype = (short)DT_FLOAT;
		hdr->dime.bitpix = (short) sizeof(float) * 8;
		break;
	case DT_DOUBLE:
		hdr->dime.datatype = (short)DT_DOUBLE;
		hdr->dime.bitpix = (short) sizeof(double) * 8;
		break;
	default:
		// TODO SEND MESSAGE "TYPE NOT SUPPORTED FOR O/P" TO ERROR LOG
		break;
	}

	//For sparse writing, add 5 to the data type
	if (sparse)
		hdr->dime.datatype += 5;
}

//
void  mdm_AnalyzeFormat::hdrBlankInit(struct dsr *const hdr)
{
	int i;

	assert(hdr != NULL);

	hdr->hk.sizeof_hdr = (int) sizeof(struct dsr);       /* Should be 348 */

	for (i = 0; i < 10; i++)
		hdr->hk.data_type[i] = '\0';
	for (i = 0; i < 18; i++)
		hdr->hk.db_name[i] = '\0';
	hdr->hk.extents = (int)0;
	hdr->hk.session_error = (short)0;
	hdr->hk.regular = 'r';
	hdr->hk.hkey_un0 = ' ';

	for (i = 0; i < 8; i++)
		hdr->dime.dim[i] = (short)0;
	for (i = 0; i < 4; i++)
		hdr->dime.vox_units[i] = '\0';
	std::strcpy(hdr->dime.vox_units, "mm");
	for (i = 0; i < 8; i++)
		hdr->dime.cal_units[i] = '\0';
	hdr->dime.unused1 = (short)0;
	hdr->dime.datatype = (short)DT_UNKNOWN;
	hdr->dime.bitpix = (short)0;
	hdr->dime.dim_un0 = (short)0;
	for (i = 0; i < 8; i++)
		hdr->dime.pixdim[i] = (float)0;
	hdr->dime.vox_offset = (float) 0.0;
	// This is where mricro expects to find a scale factor
	hdr->dime.roi_scale = (float) 1.0;
	hdr->dime.funused1 = (float) 0.0;
	hdr->dime.funused2 = (float) 0.0;
	hdr->dime.cal_max = (float) 0.0;
	hdr->dime.cal_min = (float) 0.0;
	hdr->dime.compressed = (int) 0.0;
	hdr->dime.verified = (int) 0.0;
	hdr->dime.glmax = (int)0;
	hdr->dime.glmin = (int)0;

	for (i = 0; i < 80; i++)
		hdr->hist.descrip[i] = '\0';
	for (i = 0; i < 24; i++)
		hdr->hist.aux_file[i] = '\0';
	hdr->hist.orient = (char)0;
	for (i = 0; i < 10; i++)
		hdr->hist.originator[i] = '\0';
	for (i = 0; i < 10; i++)
		hdr->hist.generated[i] = '\0';
	for (i = 0; i < 10; i++)
		hdr->hist.scannum[i] = '\0';
	for (i = 0; i < 10; i++)
		hdr->hist.patient_id[i] = '\0';
	for (i = 0; i < 10; i++)
		hdr->hist.exp_date[i] = '\0';
	for (i = 0; i < 10; i++)
		hdr->hist.exp_time[i] = '\0';
	for (i = 0; i < 3; i++)
		hdr->hist.hist_un0[i] = '\0';
	hdr->hist.views = (int)0;
	hdr->hist.vols_added = (int)0;
	hdr->hist.start_field = (int)0;
	hdr->hist.field_skip = (int)0;
	hdr->hist.omax = (int)0;
	hdr->hist.omin = (int)0;
	hdr->hist.smax = (int)0;
	hdr->hist.smin = (int)0;
}

//
void  mdm_AnalyzeFormat::hdrToString(std::string &hdrString, const struct dsr *const hdr)
{

	assert(hdr != NULL);

	// Use C++ string stream class for this
	std::stringstream ss;

	ss <<
		"qbiAnalyzeHdr:   header struct of size " << hdr->hk.sizeof_hdr << " at location " << hdr << "\n" <<
		"the voxel matrix is " << hdr->dime.dim[1] << " x " << hdr->dime.dim[2] << " x " << hdr->dime.dim[3] <<
		", with dimensions " << hdr->dime.pixdim[1] << " x " << hdr->dime.pixdim[2] << " x " << hdr->dime.pixdim[3] << " " << hdr->dime.vox_units << "\n" <<
		"the offset is " << hdr->dime.vox_offset << ", the image extents " << hdr->hk.extents << ", and the scale factor " << hdr->dime.roi_scale << "\n" <<
		"the data type is " << hdr->dime.datatype << ", i.e. " << hdr->dime.bitpix << " bits per pixel\n";
	hdrString = ss.str();
}

//
