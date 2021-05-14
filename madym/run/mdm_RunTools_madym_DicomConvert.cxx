/**
*  @file    mdm_RunTools_madym_DicomConvert.cxx
*  @brief   Implementation of mdm_RunTools_madym_DicomConvert class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_DicomConvert.h"

#include <madym/image_io/dicom/mdm_DicomFormat.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include <madym/image_io/mdm_ImageIO.h>
#include <madym/mdm_Image3D.h>
#include <madym/mdm_SequenceNames.h>

#include <madym/mdm_exception.h>

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <numeric>

#include <dcmtk/dcmdata/dctagkey.h>


namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_DicomConvert::mdm_RunTools_madym_DicomConvert()
  : 
  temporalResolution_(0)
{
}


MDM_API mdm_RunTools_madym_DicomConvert::~mdm_RunTools_madym_DicomConvert()
{
}

//
MDM_API void mdm_RunTools_madym_DicomConvert::run()
{
  //Check required inputs
  checkRequiredInputs();

  //Set curent working dir
  set_up_cwd();

  //Set up output folder
  set_up_output_folder();

  //Set up logging trail
  set_up_logging();

  //Get DCMTK to be quiet!
  OFLog::configure(OFLogger::ERROR_LOG_LEVEL);

  if (options_.dicomSort())
    sortDicomDir();

  else
    readSeriesInfo();

  if (options_.makeT1Inputs())
    makeT1InputVols(seriesInfo_);

  if (options_.makeDyn())
    makeDynamicVols(seriesInfo_);

  if (options_.makeSingle())
    makeSingleVol(seriesInfo_);
}

//
MDM_API int mdm_RunTools_madym_DicomConvert::parseInputs(int argc, const char *argv[])
{
  po::options_description cmdline_options("madym_DicomConvert options");
  po::options_description config_options("madym_DicomConvert config options");

  options_parser_.add_option(cmdline_options, options_.configFile);
  options_parser_.add_option(cmdline_options, options_.dataDir);

		//General output options_
	options_parser_.add_option(config_options, options_.outputDir);
  options_parser_.add_option(config_options, options_.overwrite);

  //Dyn naming
  options_parser_.add_option(config_options, options_.dynDir);
  options_parser_.add_option(config_options, options_.dynName);
  options_parser_.add_option(config_options, options_.nDyns);

  //T1 input naming
  options_parser_.add_option(config_options, options_.T1inputNames);
  options_parser_.add_option(config_options, options_.T1Dir);
  
  //General naming
  options_parser_.add_option(config_options, options_.sequenceFormat);
  options_parser_.add_option(config_options, options_.sequenceStart);
  options_parser_.add_option(config_options, options_.sequenceStep);
  options_parser_.add_option(config_options, options_.meanSuffix);
  options_parser_.add_option(config_options, options_.repeatPrefix);

  //Image format options
  options_parser_.add_option(config_options, options_.imageWriteFormat);
  options_parser_.add_option(config_options, options_.imageDataType);
  options_parser_.add_option(config_options, options_.flipX);
  options_parser_.add_option(config_options, options_.flipY);

  //Dicom options
  options_parser_.add_option(config_options, options_.dicomDir);
  options_parser_.add_option(config_options, options_.dicomSort);
  options_parser_.add_option(config_options, options_.dicomSeriesFile);
  options_parser_.add_option(config_options, options_.makeT1Inputs);
  options_parser_.add_option(config_options, options_.T1inputSeries);
  options_parser_.add_option(config_options, options_.makeDyn);
  options_parser_.add_option(config_options, options_.makeSingle);
  options_parser_.add_option(config_options, options_.dynSeries);
  options_parser_.add_option(config_options, options_.singleSeries);
  options_parser_.add_option(config_options, options_.makeT1Means);
  options_parser_.add_option(config_options, options_.makeDynMean);
  options_parser_.add_option(config_options, options_.dicomFileFilter);
  options_parser_.add_option(config_options, options_.volumeName);

  //Dicom options - scaling
  options_parser_.add_option(config_options, options_.autoScaleTag);
  options_parser_.add_option(config_options, options_.autoOffsetTag);
  options_parser_.add_option(config_options, options_.dicomScale);
  options_parser_.add_option(config_options, options_.dicomOffset);

  //Dicom options - time
  options_parser_.add_option(config_options, options_.dynTimeTag);
  options_parser_.add_option(config_options, options_.temporalResolution);

  //Logging options_
  options_parser_.add_option(config_options, options_.noLog);
  options_parser_.add_option(config_options, options_.noAudit);
  options_parser_.add_option(config_options, options_.quiet);
  options_parser_.add_option(config_options, options_.programLogName);
  options_parser_.add_option(config_options, options_.outputConfigFileName);
  options_parser_.add_option(config_options, options_.auditLogBaseName);
  options_parser_.add_option(config_options, options_.auditLogDir);

  return options_parser_.parseInputs(
    cmdline_options,
    config_options,
    options_.configFile(),
    who(),
    argc, argv);
}

MDM_API std::string mdm_RunTools_madym_DicomConvert::who() const
{
	return "madym_DicomConvert";
}

//*******************************************************************************
// Private:
//*******************************************************************************

void mdm_RunTools_madym_DicomConvert::checkRequiredInputs()
{
  //Check required inputs
  //if (options_.model().empty())
  //{
  //  throw mdm_exception(__func__, "model (option -m) must be provided");
  //}
}

//! Template method for getting numeric info from DICOM header in different numeric formats
template <class T> bool getNumericInfo(DcmFileFormat &fileformat, const DcmTagKey &key, T &info)
{
  try
  {
    info = (T)mdm_DicomFormat::getNumericField(fileformat, key);
    return true;
  }
  catch (mdm_DicomMissingFieldException &e)
  {
    mdm_ProgramLogger::logProgramWarning(__func__, e.what());
    return false;
  }
}

//!Extract info from dicom file header
void mdm_RunTools_madym_DicomConvert::extractInfo(const std::string &filename,
  std::vector< dcmNumericInfo > &info_numeric,
  std::vector< std::string >  &info_filenames)
{
  DcmFileFormat fileformat;
  OFCondition status = fileformat.loadFile(filename.c_str());
  if (status.good())
  {
    dcmNumericInfo info_n;
    bool valid = 
      getNumericInfo(fileformat, DCM_SeriesNumber, info_n.seriesNumber) &&
      getNumericInfo(fileformat, DCM_AcquisitionNumber, info_n.acquisitionNumber) &&
      getNumericInfo(fileformat, DCM_TemporalPositionIdentifier, info_n.temporalPositionIdentifier) &&
      getNumericInfo(fileformat, DCM_SliceLocation, info_n.sliceLocation) &&
      getNumericInfo(fileformat, DCM_InstanceNumber, info_n.instanceNumber);

    //Only continue if acquistion number set
    if (valid && info_n.acquisitionNumber)
    {
      //Add numeric info to the main container and push back filename
      info_filenames.push_back(filename);
      info_numeric.push_back(info_n);
    }
    else
      mdm_ProgramLogger::logProgramWarning(__func__, filename + ": empty acquisition number");

  }
  else
    mdm_ProgramLogger::logProgramWarning(__func__,
      filename + ": cannot read DICOM file (" + status.text() + ")");

}

//! Get list of DCM files in all sub dirs of directory
std::vector<std::string> mdm_RunTools_madym_DicomConvert::getFileList(fs::path directory)
{
  std::vector<std::string> files;
  try
  {
    if (fs::exists(directory))    // does directory actually exist?
    {
      if (fs::is_regular_file(directory))        // is directory a regular file?   
        throw mdm_exception(__func__, directory.string() + " is a file not a directory");

      else if (fs::is_directory(directory))      // is directory a directory?
      {
        std::vector<fs::path> paths;             // store paths,

        std::copy(
          fs::recursive_directory_iterator(directory),
          fs::recursive_directory_iterator(),
          std::back_inserter(paths));
        for (auto p : paths)
        {
          if (fs::is_regular_file(p))
          {
            //If filter is empty, just pushback this filename, otherwise, check it starts with the
            //file filter
            if (options_.dicomFileFilter().empty() || 
              (p.filename().string().rfind(options_.dicomFileFilter(), 0) == 0))
              files.push_back(p.string());
          }            
        }
      }

      else
        throw mdm_exception(__func__, directory.string() + " exists, but is neither a regular file nor a directory");
    }
    else
      throw mdm_exception(__func__, directory.string() + " does not exist");
  }

  catch (const fs::filesystem_error& ex)
  {
    throw mdm_exception(__func__, "Caught boost::filesystem exception" + std::string(ex.what()));
  }
  return files;
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::completeSeriesInfo(dcmSeriesInfo &series)
{
  if (series.filenames.empty())
    return;

  DcmFileFormat fileformat;
  OFCondition status = fileformat.loadFile(series.filenames[0].c_str());

  if (!status.good())
    throw mdm_exception(__func__, "Unable to open " + series.filenames[0]);

  if (series.name.empty())
  {
    series.name = mdm_DicomFormat::getTextField(fileformat, DCM_SeriesDescription);
    if (series.name.empty())
      series.name = mdm_DicomFormat::getTextField(fileformat, DCM_ProtocolName);
  }
  
  series.manufacturer =  mdm_DicomFormat::getTextField(fileformat, DCM_Manufacturer);
  series.nTimes = mdm_DicomFormat::getNumericField(fileformat, DCM_NumberOfTemporalPositions);

  //Call checkSortValid to compute nZ and set sortValid
  checkSortValid(series);
  series.nX = mdm_DicomFormat::getNumericField(fileformat, DCM_Columns);
  series.nY = mdm_DicomFormat::getNumericField(fileformat, DCM_Rows);

  auto pixelSpacing = mdm_DicomFormat::getNumericVector(fileformat, DCM_PixelSpacing, 2);
  series.Xmm = pixelSpacing[0];
  series.Ymm = pixelSpacing[1];
  series.Zmm = mdm_DicomFormat::getNumericField(fileformat, DCM_SliceThickness);

  bool FA_required = true;
  bool TR_required = true;
  bool TI_required = false;
  bool TE_required = true;
  bool B_required = true;
  bool gradOri_required = true;
  bool time_required = true;

  try { series.FA = mdm_DicomFormat::getNumericField(fileformat, DCM_FlipAngle); }
  catch (mdm_DicomMissingFieldException ) {
    if (FA_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_FlipAngle field");
  };

  try { series.TR = mdm_DicomFormat::getNumericField(fileformat, DCM_RepetitionTime); }
  catch (mdm_DicomMissingFieldException ) {
    if (TR_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_RepetitionTime field");
  };

  try { series.TI = mdm_DicomFormat::getNumericField(fileformat, DCM_InversionTime);}
  catch (mdm_DicomMissingFieldException ) {
    if (TI_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_InversionTime field");
  };

  try {series.TE = mdm_DicomFormat::getNumericField(fileformat, DCM_EchoTime);}
  catch (mdm_DicomMissingFieldException ) {
    if (TE_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_EchoTime field");
  };

  try { series.B = mdm_DicomFormat::getNumericField(fileformat, DCM_DiffusionBValue); }
  catch (mdm_DicomMissingFieldException) {
    if (B_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_DiffusionBValue field");
  };

  try { series.gradOri = mdm_DicomFormat::getNumericField(fileformat, DCM_DiffusionGradientOrientation); }
  catch (mdm_DicomMissingFieldException) {
    if (gradOri_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_DiffusionGradientOrientation field");
  }

  try {series.acquisitionTime = mdm_DicomFormat::getNumericField(fileformat, DCM_AcquisitionTime);}
  catch (mdm_DicomMissingFieldException) {
    if (time_required)
      mdm_ProgramLogger::logProgramWarning(__func__, "Series " + series.name + " missing DCM_AcquisitionTime field");
  };
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::printSeriesInfo(const dcmSeriesInfo &series)
{
  /*
  std::stringstream ss;

	ss << 
		"mdm_Image3D:   type " << imgType_ << " image struct at location " << this << "\n" <<
		"voxel matrix is " << nX_ << " x " << nY_ << " x " << nZ_ << 
		", with dimensions " << 
		info_.Xmm.value() << " mm x " << 
		info_.Ymm.value() << " mm x " << 
		info_.Zmm.value() << " mm\n" <<
		"time stamp is " << timeStamp_ << "\n" <<
		"info fields: flip angle is " << info_.flipAngle.value() << ", TR is " << info_.TR.value() << ",\n" <<
    "TE is " << info_.TE.value() << " and B is " << info_.B.value() << " (value < 0.0 => not set)\n" <<
    "and the image data is held at " << &data_ << "\n",
  
  imgString = ss.str();*/
  //mdm_ProgramLogger::logProgramMessage("Series " + std::to_string(series.index) + ": " + series.name);
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::printSeriesInfoSummary(const dcmSeriesInfo &series)
{
  std::string vol = series.nTimes > 1 ? "volumes" : "volume";
  std::stringstream ss;

  ss <<
    "Series " << series.index << ": " << series.name <<
    ", " << series.nTimes << " " << vol << " of size " <<
    "(" << series.nX << " x " << series.nY << " x " << series.nZ << ")" <<
    ", voxel size (" <<
    series.Xmm << " x " <<
    series.Ymm << " x " <<
    series.Zmm << ")\n";

  mdm_ProgramLogger::logProgramMessage(ss.str());
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::writeSeriesInfo(
  const std::vector<dcmSeriesInfo> &seriesInfo)
{

  //Create file for saving the series names
  auto seriesFileRoot = outputPath_ / options_.dicomSeriesFile();
  auto seriesFile = seriesFileRoot.string() + "_names.txt";

  // Open text file for writing filenames and numeric info
  std::ofstream seriesFileStream(seriesFile.c_str(), std::ios::out);
  if (!seriesFileStream)
    throw mdm_exception(__func__, "Can't open series text file for writing " + seriesFile);

  for (const auto &series : seriesInfo)
  {
    auto seriesName = series.name;

    //Write the unmodified series name
    seriesFileStream << seriesName << '\n';

    //Set up paths to files
    auto seriesRoot = (boost::format("%1%_series%2%") % seriesFileRoot.string() % series.index).str();
    auto filenameFile = seriesRoot + "_filenames.txt";
    auto numericFile = seriesRoot + "_info.txt";

    // Open text file for writing filenames and numeric info
    std::ofstream filenameFileStream(filenameFile.c_str(), std::ios::out);
    if (!filenameFileStream)
      throw mdm_exception(__func__, "Can't open series filenames for writing " + filenameFile);

    //Now save the rest of the filenames
    for (auto filename : series.filenames)
      filenameFileStream << filename << '\n';
    filenameFileStream.close();

    const auto &numericInfo = series.numericInfo;
    std::ofstream numericFileStream(numericFile.c_str(), std::ios::out);
    if (!numericFileStream)
      throw mdm_exception(__func__, "Can't open series numeric info for writing " + numericFile);

    //Save the number of rows as the first entry
    numericFileStream << numericInfo.size() << '\n';
    for (auto info : numericInfo)
      numericFileStream << 
      info.seriesNumber << " " << 
      info.acquisitionNumber << " " << 
      info.temporalPositionIdentifier << " " << 
      info.sliceLocation << " " <<
      info.instanceNumber << '\n';
    numericFileStream.close();
  }
  seriesFileStream.close();
}

//--------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::readSeriesInfo()
{
  auto seriesFileRoot = fs::absolute(options_.dicomSeriesFile());
  auto seriesFile = seriesFileRoot.string() + "_names.txt";

  // Open text file for writing filenames and numeric info
  std::ifstream seriesFileStream(seriesFile.c_str(), std::ios::in);
  if (!seriesFileStream)
    throw mdm_exception(__func__, "Can't open series text file for reading " + seriesFile);

  
  //Make sure main series info container empty
  seriesInfo_.clear();

  //Get list of series names, using each to initialise new series info
  std::string name;
  while (!seriesFileStream.eof())
  {
    std::getline(seriesFileStream, name);
    if (!name.empty())
    {
      dcmSeriesInfo series;
      series.name = name;
      series.index = (int)seriesInfo_.size() + 1;
      seriesInfo_.push_back(series);
    }
  }
  seriesFileStream.close();

  //Now fill each series info object

  for (auto &series : seriesInfo_)
  {
    //Set up paths to file
    auto seriesRoot = (boost::format("%1%_series%2%") % seriesFileRoot.string() % series.index).str();
    auto filenameFile = seriesRoot + "_filenames.txt";
    auto numericFile = seriesRoot + "_info.txt";

    // Open text file for reading filenames
    std::ifstream filenameFileStream(filenameFile.c_str(), std::ios::in);
    if (!filenameFileStream)
      throw mdm_exception(__func__, "Can't open series filenames for reading " + filenameFile);

    std::string name;
    while (!filenameFileStream.eof())
    {
      std::getline(filenameFileStream, name);
      if (!name.empty())
        series.filenames.push_back(name);
    }
    filenameFileStream.close();


    // Open text file for reading numeric info
    std::ifstream numericFileStream(numericFile.c_str(), std::ios::in);
    if (!numericFileStream)
      throw mdm_exception(__func__, "Can't open series numeric info for reading " + numericFile);

    int numRows;
    numericFileStream >> numRows;
    
    for (int i_row = 0; i_row < numRows; i_row++)
    {
      dcmNumericInfo info_n;
      numericFileStream >> info_n.seriesNumber;
      numericFileStream >> info_n.acquisitionNumber;
      numericFileStream >> info_n.temporalPositionIdentifier;
      numericFileStream >> info_n.sliceLocation;
      numericFileStream >> info_n.instanceNumber;

      series.numericInfo.push_back(info_n);
    }
    numericFileStream.close();

    //Get the remaining series info
    completeSeriesInfo(series);
  }
}

//--------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::sortDicomDir()
{

  const auto directory = fs::absolute(fs::path(options_.dicomDir()));

  //Get list of all files in the directory tree
  std::vector<std::string>  filenames = getFileList(directory);
  auto n_files = filenames.size();

  if (!n_files)
    throw mdm_exception(__func__, "No files to process found in " + directory.string());

  //Set up containers for DICOM info
  std::vector< dcmNumericInfo > info_numeric;
  std::vector< std::string > info_filenames;

  //Loop through each dicom file, extracting required parameter info
  mdm_ProgramLogger::logProgramMessage(
    "Parsing " + std::to_string(n_files) + " dicom files, may take a while..");

  for (auto i_file = 0; i_file < n_files; i_file++)
  {
    if (!fmod(i_file, 1000))
      mdm_ProgramLogger::logProgramMessage(
        std::to_string(i_file) + " complete");

    extractInfo(filenames[i_file], info_numeric, info_filenames);
  }

  //Check if any of the files returned actual DICOM info
  n_files = info_numeric.size();
  if (!n_files)
    throw mdm_exception(__func__, "None of the files in " + directory.string() + " were valid DICOM images");

  std::vector<size_t> sort_idx(n_files);
  std::iota(sort_idx.begin(), sort_idx.end(), 0); //returns 0, 1, 2,... etc
  std::sort(sort_idx.begin(), sort_idx.end(),
    [&info_numeric](size_t i1, size_t i2) {
    return
      info_numeric[i1].seriesNumber < info_numeric[i2].seriesNumber ? true :
      info_numeric[i1].seriesNumber > info_numeric[i2].seriesNumber ? false :
      info_numeric[i1].acquisitionNumber < info_numeric[i2].acquisitionNumber ? true :
      info_numeric[i1].acquisitionNumber > info_numeric[i2].acquisitionNumber ? false :
      info_numeric[i1].temporalPositionIdentifier < info_numeric[i2].temporalPositionIdentifier ? true :
      info_numeric[i1].temporalPositionIdentifier > info_numeric[i2].temporalPositionIdentifier ? false :
      info_numeric[i1].sliceLocation < info_numeric[i2].sliceLocation ? true : false; });

  //Make sure series info container is empty
  seriesInfo_.clear();

  double currSeriesNum = -1;
  for (auto i_file = 0; i_file < n_files; i_file++)
  {
    auto idx = sort_idx[i_file];
    auto info_n = info_numeric[idx];
    auto filename = info_filenames[idx];
    auto seriesNum = info_n.seriesNumber;

    //If new series number, start new series
    if (seriesNum != currSeriesNum)
    {
      //Add the series name - either from the SeriesDescription tag, or if that's empty
      //the ProtocolName tag
      DcmFileFormat fileformat;
      OFCondition status = fileformat.loadFile(filename.c_str());
      if (!status.good())
        throw mdm_exception(__func__, "Unable to open " + filename[0]);

      auto seriesName = mdm_DicomFormat::getTextField(fileformat, DCM_SeriesDescription);
      if (seriesName.empty())
        seriesName = mdm_DicomFormat::getTextField(fileformat, DCM_ProtocolName);

      //Create a series info struct and push onto the seriesInfo container
      dcmSeriesInfo series;
      series.name = seriesName;
      series.index = (int)seriesInfo_.size() + 1;
      seriesInfo_.push_back(series);

      //Update the current series number
      currSeriesNum = seriesNum;

    }
    //Add the filename and numeric info to the current series info struct
    auto &series = seriesInfo_.back();
    series.filenames.push_back(filename);
    series.numericInfo.push_back(info_n);
  }

  auto nSeries = seriesInfo_.size();
  mdm_ProgramLogger::logProgramMessage(
    "Found " + std::to_string(nSeries) +  " series:");

  for (auto &series : seriesInfo_)
  {
    completeSeriesInfo(series);
    printSeriesInfoSummary(series);
  }

  //Write output lists
  writeSeriesInfo(seriesInfo_);

}

 //---------------------------------------------------------------------
mdm_Image3D mdm_RunTools_madym_DicomConvert::loadDicomImage(
  const dcmSeriesInfo &series, const int startIdx,
  bool isDynamic, int dynNum)
{
  //Get vector filenames for this slice
  std::vector<std::string> sliceNames(series.filenames.begin() + startIdx,
    series.filenames.begin() + startIdx + series.nZ);

  DcmFileFormat fileformat;
  OFCondition status = fileformat.loadFile(sliceNames[0].c_str());
  if (!status.good())
    throw mdm_exception(__func__, "Unable to open " + sliceNames[0]);
  
  //Get various bits of info from DICOM header
  double offset = options_.dicomOffset();
  double scale = options_.dicomScale();
  applyAutoScaling(fileformat, offset, scale);

  std::vector<size_t> dimensions = { (size_t)series.nX, (size_t)series.nY, (size_t)series.nZ };
  std::vector <double> pixelSpacing = { series.Xmm, series.Ymm, series.Zmm };

  //Make the image from the filenames
  auto img = mdm_DicomFormat::loadImageFromDicomSlices(
    dimensions, pixelSpacing, sliceNames, offset, scale,
    options_.flipX(), options_.flipY());

  //Now set meta data from DICOM fields
  if (series.FA)
    img.info().flipAngle.setValue(series.FA);
  if (series.TR)
    img.info().TR.setValue(series.TR);
  if (series.TE)
    img.info().TE.setValue(series.TE);
  if (series.TI)
    img.info().TI.setValue(series.TI);
  if (series.B)
    img.info().B.setValue(series.B);

  double acquisitionTime = isDynamic ?
    getDynamicTime(fileformat, dynNum) :
    series.acquisitionTime;

  img.setTimeStampFromDoubleStr(acquisitionTime);
  return img;
}


//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::checkSortValid(
  dcmSeriesInfo &series)
{
  series.nTimes;
  series.nZ = (int)series.numericInfo.size() / series.nTimes;

  //Check numSlices divided into an integer
  if (series.nZ*series.nTimes != (int)series.numericInfo.size())
  {
    series.sortValid = false;
    mdm_ProgramLogger::logProgramWarning(__func__, 
      series.name + " is not valid, the number of filenames is not an integer mulitple of the number of timepoints.");
    return;
  }
    

  //When we loop through numeric we should see blocks of
  // [1,...,1][2,...,2]...[tn,...,tn] in temporalPosition
  // [z1,z2,...,zn]...[z1,z2,...,zn] in slice location
  for (int i_t = 0; i_t < series.nTimes; i_t++)
  {
    for (int i_z = 0; i_z < series.nZ; i_z++)
    {
      const auto &infoi = series.numericInfo[(i_t*series.nZ) + i_z];

      series.sortValid = infoi.temporalPositionIdentifier == (int)i_t + 1;

      if (series.sortValid && i_t)
      {
        const auto &info0 = series.numericInfo[i_z];
        series.sortValid = infoi.sliceLocation == info0.sliceLocation;
      }
      
      if (!series.sortValid)
        break;
    }
  }
  if (!series.sortValid)
    mdm_ProgramLogger::logProgramWarning(__func__,
      series.name + " is not valid: numeric info did not match the expected format. Check the series log files.");

}

//---------------------------------------------------------------------
//
void mdm_RunTools_madym_DicomConvert::setDicomTag(const mdm_input_strings &tagInput, DcmTagKey &tag)
{
  //Make sure input has exactly 2 elements
  if (tagInput().size() != 2)
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key()) +
      ": dicom tag definitions must have exactly 2 elements");

  try 
  {
    auto group = std::stoul(tagInput()[0], 0, 16);
    auto element = std::stoul(tagInput()[1], 0, 16);
    tag.set(group, element);
  }
  catch (const std::invalid_argument &e)
  {
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key()) 
      + ": " + e.what());
  }
  catch (const std::out_of_range &e)
  {
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key())
      + ": " + e.what());
  }
  if (!tag.hasValidGroup())
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key())
      + std::string(tag.toString().c_str()) + " is not a valid tag key");
}

//---------------------------------------------------------------------
//
void mdm_RunTools_madym_DicomConvert::setDicomTag(const mdm_input_string &tagInput, DcmTagKey &tag)
{
  //Make sure input has exactly 2 elements
  if (tagInput().size() != 13)
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key()) +
      " = " + tagInput() +
      ": dicom tag definitions must by of form 0xAAAA_0xAAAA");

  if (tagInput().substr(0,2) != "0x")
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key()) +
      " = " + tagInput() +
      ": dicom tag definitions must by of form 0xAAAA_0xAAAA");

  if (tagInput().substr(7, 2) != "0x")
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key()) +
      " = " + tagInput() +
      ": dicom tag definitions must by of form 0xAAAA_0xAAAA");

  try
  {
    auto group = std::stoul(tagInput().substr(2, 4), 0, 16);
    auto element = std::stoul(tagInput().substr(9, 4), 0, 16);
    tag.set(group, element);
  }
  catch (const std::invalid_argument &e)
  {
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key())
      + ": " + e.what());
  }
  catch (const std::out_of_range &e)
  {
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key())
      + ": " + e.what());
  }
  if (!tag.hasValidGroup())
    throw mdm_exception(__func__, "Error parsing " + std::string(tagInput.key())
      + std::string(tag.toString().c_str()) + " is not a valid tag key");
}

void mdm_RunTools_madym_DicomConvert::checkAutoScaling()
{
  if (!options_.autoOffsetTag().empty())
    setDicomTag(options_.autoOffsetTag, autoOffsetTag_);

  if (!options_.autoScaleTag().empty())
    setDicomTag(options_.autoScaleTag, autoScaleTag_);
}

//
void mdm_RunTools_madym_DicomConvert::checkDynamicTime()
{
  //If input options dynTimeTag is not empty, attempt to set the tag from that
  if (!options_.dynTimeTag().empty())
    setDicomTag(options_.dynTimeTag, dynamicTimeTag_);

  //Else use default of DCM_AcquisitionTime
  else 
    dynamicTimeTag_.set(DCM_AcquisitionTime);

  //If temporal resolution set, dynamic time is computed from acquisition time of first image
  // + i_dyn*temporalResolution
  if (options_.temporalResolution())
    temporalResolution_ = options_.temporalResolution();
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::applyAutoScaling(DcmFileFormat &fileformat,
  double &offset, double &scale)
{
  

  if (autoOffsetTag_.hasValidGroup())
  {
    try
    {
      offset += mdm_DicomFormat::getNumericField(fileformat, autoOffsetTag_);
    }
    catch (mdm_DicomMissingFieldException &e)
    {
      mdm_ProgramLogger::logProgramWarning(__func__, e.what());
      mdm_ProgramLogger::logProgramWarning(__func__,
        "Auto offset tag is set, but could not access " + 
        std::string(autoOffsetTag_.toString().c_str()) + " to get intercept value");
    }
  }
  if (autoScaleTag_.hasValidGroup())
  {
    try
    {
      scale *= mdm_DicomFormat::getNumericField(fileformat, autoScaleTag_);
    }
    catch (mdm_DicomMissingFieldException &e)
    {
      mdm_ProgramLogger::logProgramWarning(__func__, e.what());
      mdm_ProgramLogger::logProgramWarning(__func__,
        "Auto scale tag is set, but could not access " +
        std::string(autoOffsetTag_.toString().c_str()) + " to get slope value");
    }
  }
}

//---------------------------------------------------------------------
double mdm_RunTools_madym_DicomConvert::getDynamicTime(DcmFileFormat &fileformat, int dynNum)
{
  //Attempt to get initial time from dicom field
  double dynTime = 0;
  if (dynamicTimeTag_.hasValidGroup())
  {
    try
    {
      dynTime = mdm_DicomFormat::getNumericField(fileformat, dynamicTimeTag_);
    }
    catch (mdm_DicomMissingFieldException &e)
    {
      mdm_ProgramLogger::logProgramError(__func__, e.what());
      throw mdm_exception(__func__, "Dynamic time tag is set, but could not access " +
        std::string(dynamicTimeTag_.toString().c_str()));
    }
  }
  
  if (dynNum && temporalResolution_)
  {
    //Need to compute final dynamic time from initial acquistion time, plus i_t*temp_res
    //taking into account time format is hhmmss:msecs
    auto timeInSecs = mdm_Image3D::timestampToSecs(dynTime);
    timeInSecs += (dynNum*temporalResolution_);
    dynTime = mdm_Image3D::secsToTimestamp(timeInSecs);
  }

  return dynTime;
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::makeSingleVol(
  const std::vector<dcmSeriesInfo> &seriesInfo)
{
  const auto &index = options_.singleSeries() - 1;
  if (index < 0 || index >= seriesInfo.size())
    throw mdm_exception(__func__,
      boost::format("Dicom series index (%1%) must be >= 0 and < %2%")
      % index % seriesInfo.size());

  checkAutoScaling();

  const auto& series = seriesInfo[index];

  //Check this sequence was validly sorted
  if (!series.sortValid)
    throw mdm_exception(__func__, boost::format(
      "Series %1% was not sorted properly. Check the series log files"
    ) % series.name);

  auto img = loadDicomImage(series, 0, false);

  //Get write format from options
  auto imageWriteFormat = mdm_ImageIO::formatFromString(options_.imageWriteFormat());
  auto imageDatatype = static_cast<mdm_ImageDatatypes::DataType>(options_.imageDataType());

  auto volumeName = outputPath_ / options_.volumeName();
  
  mdm_ImageIO::writeImage3D(imageWriteFormat,
    volumeName.string(), img, imageDatatype, mdm_XtrFormat::NEW_XTR);
  mdm_ProgramLogger::logProgramMessage("Created 3D image " + volumeName.string());
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::makeT1InputVols(
  const std::vector<dcmSeriesInfo> &seriesInfo)
{
  checkAutoScaling();

  auto nInputs = options_.T1inputSeries().size();
  assert(nInputs = options_.T1inputNames().size());

  auto imageWriteFormat = mdm_ImageIO::formatFromString(options_.imageWriteFormat());
  auto imageDatatype = static_cast<mdm_ImageDatatypes::DataType>(options_.imageDataType());

  for (size_t i_t1 = 0; i_t1 < nInputs; i_t1++)
  {
    
    const auto &T1Name = options_.T1inputNames()[i_t1];
    const auto &index = options_.T1inputSeries()[i_t1]-1;

    if (index < 0 || index >= seriesInfo.size())
      throw mdm_exception(__func__,
        boost::format("T1 input series index (%1%) must be >= 0 and < %2%")
        % index % seriesInfo.size());

    const auto &series = seriesInfo[index];

    //Check this sequence was validly sorted
    if (!series.sortValid)
      throw mdm_exception(__func__, boost::format(
        "Series %1% was not sorted properly. Check the series log files"
      ) % series.name);

    //Set up mean image
    mdm_Image3D meanImg;

    //Set up output directory - outpath is already absolute
    fs::path T1Dir = fs::path(options_.T1Dir()) / fs::path(T1Name);
    fs::create_directories(T1Dir);

    for (int i_rpt = 0; i_rpt < series.nTimes; i_rpt++)
    {
      auto startIdx = i_rpt * series.nZ;
      auto img = loadDicomImage(series, startIdx);

      //Make output name
      auto outputName = mdm_SequenceNames::makeSequenceFilename(
        T1Dir.string(), options_.repeatPrefix(), i_rpt + 1, options_.sequenceFormat(),
        options_.sequenceStart(), options_.sequenceStep());

      //Write the output image and xtr file
      mdm_ImageIO::writeImage3D(imageWriteFormat,
        outputName, img, imageDatatype, mdm_XtrFormat::NEW_XTR);
      mdm_ProgramLogger::logProgramMessage("Created T1 input file " + outputName);

      if (options_.makeT1Means())
      {
        if (!i_rpt)
          meanImg = img;

        else
          meanImg += img;
      }
    }

    if (options_.makeT1Means())
    {
      //Finalise the mean image
      meanImg /= series.nTimes;
      auto meanName = T1Dir.string() + options_.meanSuffix();
      mdm_ImageIO::writeImage3D(imageWriteFormat,
        meanName, meanImg, imageDatatype, mdm_XtrFormat::NEW_XTR);
      mdm_ProgramLogger::logProgramMessage("Created mean T1 input file " + meanName);
    }
  }
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::makeDynamicVols(
  const std::vector<dcmSeriesInfo> &seriesInfo)
{
  const auto &index = options_.dynSeries()-1;

  if (index < 0 || index >= seriesInfo.size())
    throw mdm_exception(__func__,
      boost::format("Dynamic series index (%1%) must be >= 0 and < %2%")
      % index % seriesInfo.size());

  checkAutoScaling();
  checkDynamicTime();

  const auto& series = seriesInfo[index];

  //Check this sequence was validly sorted
  if (!series.sortValid)
    throw mdm_exception(__func__, boost::format(
      "Series %1% was not sorted properly. Check the series log files"
    ) % series.name);

  //Set up mean image
  mdm_Image3D meanImg;

  //Set up output directory - outpath is already absolute
  fs::path dynDir = fs::path(options_.dynDir());
  fs::create_directories(dynDir);

  //Cap the total number of dynamics saved if set in options
  auto nTimes = (options_.nDyns() && options_.nDyns() < series.nTimes) ?
    options_.nDyns() : series.nTimes;

  //Get write format from options
  auto imageWriteFormat = mdm_ImageIO::formatFromString(options_.imageWriteFormat());
  auto imageDatatype = static_cast<mdm_ImageDatatypes::DataType>(options_.imageDataType());
  
  mdm_ProgramLogger::logProgramMessage("Converting DICOM into DCE series with "
    + std::to_string(nTimes) + " timepoints...");

  for (int i_dyn = 0; i_dyn < nTimes; i_dyn++)
  {
    auto startIdx = i_dyn * series.nZ;

    auto img = loadDicomImage(series, startIdx, true, i_dyn);

    //Make output name
    auto outputName = mdm_SequenceNames::makeSequenceFilename(
      dynDir.string(), options_.dynName(), i_dyn+1, options_.sequenceFormat(),
      options_.sequenceStart(), options_.sequenceStep());

    //Write the output image and xtr file
    mdm_ImageIO::writeImage3D(imageWriteFormat,
      outputName, img, imageDatatype, mdm_XtrFormat::NEW_XTR);
    mdm_ProgramLogger::logProgramMessage("Created dynamic image " + outputName);

    if (options_.makeDynMean())
    {
      if (!i_dyn)
        meanImg = img;

      else
        meanImg += img;
    }
  }

  if (options_.makeDynMean())
  {
    //Finalise the mean image
    meanImg /= nTimes;
    auto meanName = dynDir / (options_.dynName() + options_.meanSuffix());
    mdm_ImageIO::writeImage3D(imageWriteFormat,
      meanName.string(), meanImg, imageDatatype, mdm_XtrFormat::NEW_XTR);
    mdm_ProgramLogger::logProgramMessage("Created temporal mean of dynamic images " + meanName.string());
  }
}