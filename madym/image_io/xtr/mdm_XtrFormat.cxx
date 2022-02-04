/**
 *  @file    mdm_XtrFormat.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_XtrFormat.h"

#include <string>
#include <sstream>
#include <istream>
#include <ostream>

#include <boost/filesystem.hpp>

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>


 //
MDM_API void mdm_XtrFormat::readAnalyzeXtr(const std::string &xtrFileName,
  mdm_Image3D &img)
{


  std::string firstString;

  std::ifstream xtrFileStream(xtrFileName, std::ios::in);
  /* Open Analyze header file stream for reading */
  if (!xtrFileStream.is_open())
    throw mdm_exception(__func__, "Can't open Analyze extra info file " + xtrFileName);

  /* Read values from extra info file */
  xtrFileStream >> firstString;
  xtrFileStream.seekg(0);
  if ((firstString == "voxel") || (firstString == "Voxel"))
  {
    readOldXtr(xtrFileStream, img);
  }
  else
  {
    readNewXtr(xtrFileStream, img);
  }

  xtrFileStream.close();
  if (xtrFileStream.is_open())
    throw mdm_exception(__func__, "Failed to close Analyze extra info file " + xtrFileName);

  img.setMetaDataSource(xtrFileName);
}

//
void mdm_XtrFormat::writeAnalyzeXtr(const std::string &baseName,
  const mdm_Image3D &img,
  const XTR_type typeFlag)
{
  std::string xtrFileName = baseName + ".xtr";

  // Open Analyze header file stream for reading
  std::ofstream xtrFileStream(xtrFileName.c_str(), std::ios::out);
  if (!xtrFileStream)
    throw mdm_exception(__func__, "Can't open Analyze extra info file" + xtrFileName);

  // Write values to extra info file
  if (typeFlag == OLD_XTR)
    mdm_XtrFormat::writeOldXtr(xtrFileStream, img);
  else
    mdm_XtrFormat::writeNewXtr(xtrFileStream, img);

  xtrFileStream.close();
  if (xtrFileStream.is_open())
    throw mdm_exception(__func__, "Failed to close Analyze extra info file " + xtrFileName);

}

//**********************************************************************
//Private 
//**********************************************************************
//

//
void mdm_XtrFormat::writeNewXtr(std::ofstream &xtrFileStream,
	const mdm_Image3D &img)
{
  img.metaDataToStream(xtrFileStream);
}

//
void mdm_XtrFormat::writeOldXtr(std::ofstream &xtrFileStream,
	const mdm_Image3D &img)
{
  /* Convert and write values from extra info to file */
	double timeStamp = img.timeStamp();
  int hrs  = (int) (timeStamp / 10000);
  int mins = (int) ((timeStamp - ((double) hrs * 10000.0)) / 100);
  double secs = timeStamp - ((double) hrs * 10000.0) - ((double) mins * 100.0);

  /* TEST FOR WRITE ERRORS */
	xtrFileStream << "voxel dimensions" << ":\t" 
		<< img.info().Xmm.value() << " " 
		<< img.info().Ymm.value() << " "
		<< img.info().Zmm.value() << std::endl;
	xtrFileStream << "flip angle" << ":\t" << img.info().flipAngle.value() << std::endl;
	xtrFileStream << "TR" << ":\t" << img.info().TR.value() << std::endl;
	xtrFileStream << "timestamp" << ":\t" << hrs << " " << mins << " " << secs << " " << timeStamp << std::endl;
}

//
void mdm_XtrFormat::readOldXtr(std::ifstream &xtrFileStream,
	mdm_Image3D &img)
{
	/* Read values from extra info file */
	img.setMetaDataFromStreamOld(xtrFileStream);

}

//
void mdm_XtrFormat::readNewXtr(std::ifstream &xtrFileStream,
	mdm_Image3D &img)
{
	std::vector<std::string> keys;
	std::vector<double> values;

	/* Read values from extra info file */
	/* TEST FOR READ ERRORS */
	img.setMetaDataFromStream(xtrFileStream);
}

//
