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
#include <madym/image_io/meta/mdm_BIDSFormat.h>
#include <madym/image_io/mdm_ImageIO.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/utils/mdm_SequenceNames.h>

#include <madym/utils/mdm_exception.h>

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

  //Set xtrType for writing
  xtrType_ = (options_.nifti4D() || options_.useBIDS()) ? 
    mdm_XtrFormat::XTR_type::BIDS : mdm_XtrFormat::XTR_type::NEW_XTR;

  if (options_.makeT1Inputs())
    makeT1InputVols(seriesInfo_);

  if (options_.makeDyn())
    makeDynamicVols(seriesInfo_);

  if (options_.makeDWIInputs())
    makeDWIInputs(seriesInfo_);

  if (options_.makeSingle())
    makeSingleVol(seriesInfo_);
}

//
MDM_API int mdm_RunTools_madym_DicomConvert::parseInputs(int argc, const char *argv[])
{
  po::options_description cmdline_options("madym_DicomConvert options");
  po::options_description config_options("madym_DicomConvert config options");

  options_parser_.add_option(cmdline_options, options_.help);
  options_parser_.add_option(cmdline_options, options_.version);

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

  //DWI naming
  options_parser_.add_option(config_options, options_.DWIinputNames);
  options_parser_.add_option(config_options, options_.DWIDir);

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
  options_parser_.add_option(config_options, options_.flipZ);
  options_parser_.add_option(config_options, options_.niftiScaling);
  options_parser_.add_option(config_options, options_.nifti4D);
  options_parser_.add_option(config_options, options_.useBIDS);

  //Dicom options
  options_parser_.add_option(config_options, options_.dicomDir);
  options_parser_.add_option(config_options, options_.dicomSort);
  options_parser_.add_option(config_options, options_.dicomSeriesFile);
  options_parser_.add_option(config_options, options_.makeT1Inputs);
  options_parser_.add_option(config_options, options_.makeDWIInputs);
  options_parser_.add_option(config_options, options_.T1inputSeries);
  options_parser_.add_option(config_options, options_.DWIinputSeries);
  options_parser_.add_option(config_options, options_.makeDyn);
  options_parser_.add_option(config_options, options_.makeSingle);
  options_parser_.add_option(config_options, options_.dynSeries);
  options_parser_.add_option(config_options, options_.singleSeries);
  options_parser_.add_option(config_options, options_.makeT1Means);
  options_parser_.add_option(config_options, options_.makeDynMean);
  options_parser_.add_option(config_options, options_.makeBvalueMeans);
  options_parser_.add_option(config_options, options_.dicomFileFilter);
  options_parser_.add_option(config_options, options_.sliceFilterTag);
  options_parser_.add_option(config_options, options_.sliceFilterMatchValue);
  options_parser_.add_option(config_options, options_.volumeName);
  options_parser_.add_option(config_options, options_.singleVolNames);

  //Dicom options - scaling
  options_parser_.add_option(config_options, options_.autoScaleTag);
  options_parser_.add_option(config_options, options_.autoOffsetTag);
  options_parser_.add_option(config_options, options_.dicomScale);
  options_parser_.add_option(config_options, options_.dicomOffset);

  //Dicom scanner settings
  options_parser_.add_option(config_options, options_.FATag);
  options_parser_.add_option(config_options, options_.FARequired);

  options_parser_.add_option(config_options, options_.TRTag);
  options_parser_.add_option(config_options, options_.TRRequired);

  options_parser_.add_option(config_options, options_.TITag);
  options_parser_.add_option(config_options, options_.TIRequired);

  options_parser_.add_option(config_options, options_.TETag);
  options_parser_.add_option(config_options, options_.TERequired);

  options_parser_.add_option(config_options, options_.BTag);
  options_parser_.add_option(config_options, options_.BRequired);

  options_parser_.add_option(config_options, options_.gradOriTag);
  options_parser_.add_option(config_options, options_.gradOriRequired);

  //Dicom options - time
  options_parser_.add_option(config_options, options_.dynTimeTag);
  options_parser_.add_option(config_options, options_.dynTimeRequired);
  options_parser_.add_option(config_options, options_.temporalResolution);
  options_parser_.add_option(config_options, options_.dynTimeFormat);
  
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

//! Template method for getting numeric info from DICOM header in different numeric formats
template <class T> bool getTextInfo(DcmFileFormat& fileformat, const DcmTagKey& key, T& info)
{
  try
  {
    info = (T)mdm_DicomFormat::getTextField(fileformat, key);
    return true;
  }
  catch (mdm_DicomMissingFieldException& e)
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
    //Check if a slice filter is in place, if so skip any frames that don't match the
    //required filter value
    if (!options_.sliceFilterTag().first.empty())
    {
      DcmTagKey filterTag;
      setDicomTag(options_.sliceFilterTag, filterTag);
      bool matched = false;
      try
      {
        for (const auto& value : options_.sliceFilterMatchValue())
        {
          if (mdm_DicomFormat::getTextField(fileformat, filterTag) == value)
          {
            matched = true;
            break;
          }
        }
      }
      catch (mdm_DicomMissingFieldException)
      {
        mdm_ProgramLogger::logProgramWarning(__func__, "Attribute for slice_filter_tag (" +
          options_.sliceFilterTag().first + "," + options_.sliceFilterTag().second + 
          ") is not set for " + filename);
      }
      //If not matched, return without trying to check any further info
      if (!matched)
        return;
    }

    dcmNumericInfo info_n;
    bool valid = 
      getNumericInfo(fileformat, DCM_SeriesNumber, info_n.seriesNumber) &&
      getNumericInfo(fileformat, DCM_AcquisitionNumber, info_n.acquisitionNumber) &&
      getNumericInfo(fileformat, DCM_SliceLocation, info_n.sliceLocation) &&
      getNumericInfo(fileformat, DCM_InstanceNumber, info_n.instanceNumber);

    if (!getNumericInfo(fileformat, DCM_TemporalPositionIdentifier, info_n.temporalPositionIdentifier))
      info_n.temporalPositionIdentifier = getDynamicTime(fileformat, 0);

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

void mdm_RunTools_madym_DicomConvert::getScannerSetting(
  DcmFileFormat& fileFormat,
  const std::string& seriesName,
  const std::string&settingName, 
  const mdm_input_dicom_tag& customTag,
  const DcmTagKey& defaultTag, 
  const bool required, 
  double& setting)
{
  DcmTagKey tagKey;
  if (customTag().first.empty())
    tagKey = defaultTag;
  else
    setDicomTag(customTag, tagKey);

  getScannerSetting(fileFormat, seriesName, settingName, tagKey,
    required, setting);
}

void mdm_RunTools_madym_DicomConvert::getScannerSetting(
  DcmFileFormat& fileFormat,
  const std::string& seriesName,
  const std::string& settingName,
  const DcmTagKey& tagKey,
  const bool required,
  double& setting)
{
  try { setting = mdm_DicomFormat::getNumericField(fileFormat, tagKey); }
  catch (mdm_DicomMissingFieldException) {
    if (required)
      mdm_ProgramLogger::logProgramWarning(__func__,
        (boost::format(
          "Series %1% missing %2% expected in field %3%."
        ) % seriesName % settingName % tagKey.toString()).str());
  };
}

void mdm_RunTools_madym_DicomConvert::getScannerSetting(
  DcmFileFormat& fileFormat,
  const std::string& seriesName,
  const std::string& settingName,
  const mdm_input_dicom_tag& customTag,
  const DcmTagKey& defaultTag,
  const bool required,
  std::vector<double>& setting, 
  size_t numValues)
{
  DcmTagKey tagKey;
  if (customTag().first.empty())
    tagKey = defaultTag;
  else
    setDicomTag(customTag, tagKey);

  getScannerSetting(fileFormat, seriesName, settingName, tagKey,
    required, setting, numValues);
}

void mdm_RunTools_madym_DicomConvert::getScannerSetting(
  DcmFileFormat& fileFormat,
  const std::string& seriesName,
  const std::string& settingName,
  const DcmTagKey& tagKey,
  const bool required,
  std::vector<double>& setting,
  size_t numValues)
{
  try { setting = mdm_DicomFormat::getNumericVector(fileFormat, tagKey, numValues); }
  catch (mdm_DicomMissingFieldException) {
    if (required)
      mdm_ProgramLogger::logProgramWarning(__func__,
        (boost::format(
          "Series %1% missing %2% expected in field %3%."
        ) % seriesName % settingName % tagKey.toString()).str());
  };
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::getSeriesName(dcmSeriesInfo& series, DcmFileFormat& fileformat)
{
  //1st try series description
  getTextInfo(fileformat, DCM_SeriesDescription, series.name);

  //Then try Protocol name
  if (series.name.empty())
    getTextInfo(fileformat, DCM_ProtocolName, series.name);

  //Finally, create using series number if still empty
  if (series.name.empty())
    series.name = "series " + std::to_string(series.index);
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::getVolumeAxesDirections(
    const std::string& firstSlice, const std::string& lastSlice,
    const std::string& seriesName, const int nSlices, mdm_Image3D::MetaData& info)
{
    //Load in the final slice and get it's image position
    DcmFileFormat firstSliceFile;
    OFCondition status = firstSliceFile.loadFile(firstSlice.c_str());

    if (!status.good())
        throw mdm_exception(__func__, "Unable to open first slice DICOM file " + firstSlice);

    //Load in the final slice and get it's image position
    DcmFileFormat lastSliceFile;
    status = lastSliceFile.loadFile(lastSlice.c_str());

    if (!status.good())
        throw mdm_exception(__func__, "Unable to open last slice DICOM file " + lastSlice);

    std::vector<double> pos1, pos2, ori1;

    getScannerSetting(
        firstSliceFile, seriesName, "ImagePositionPatient",
        DCM_ImagePositionPatient, true, pos1, 3);
    getScannerSetting(
        lastSliceFile, seriesName, "ImagePositionPatient",
        DCM_ImagePositionPatient, true, pos2, 3);
    getScannerSetting(
        firstSliceFile, seriesName, "ImageOrientationPatient",
        DCM_ImageOrientationPatient, true, ori1, 6);

    //Check if image position and orientation are set
    if (pos1.size() != 3 || pos2.size() != 3 || ori1.size() != 6)
    {
        mdm_ProgramLogger::logProgramWarning(__func__,
            seriesName + ": unable to obtain image position and orientation from DICOM headers");
        return;
    }
    
    //Set image position and orientation in the info data
    info.originX.setValue(pos1[0]);
    info.originY.setValue(pos1[1]);
    info.originZ.setValue(pos1[2]);
    
    info.rowDirCosX.setValue(ori1[0]);
    info.rowDirCosY.setValue(ori1[1]);
    info.rowDirCosZ.setValue(ori1[2]);
    info.colDirCosX.setValue(ori1[3]);
    info.colDirCosY.setValue(ori1[4]);
    info.colDirCosZ.setValue(ori1[5]);

    //Get vector from first to last frame
    auto dx = pos2[0] - pos1[0];
    auto dy = pos2[1] - pos1[1];
    auto dz = pos2[2] - pos1[2];
    auto mag = std::sqrt(dx * dx + dy * dy + dz * dz);
    dx /= mag;
    dy /= mag;
    dz /= mag;

    //Compute the distance between slices
    auto zd = mag / (nSlices - 1);

    //Compute cross product of row and column axes
    auto ux = ori1[0];
    auto uy = ori1[1];
    auto uz = ori1[2];

    auto vx = ori1[3];
    auto vy = ori1[4];
    auto vz = ori1[5];

    auto wx = uy * vz - uz * vy;
    auto wy = uz * vx - ux * vz;
    auto wz = ux * vy - uy * vx;

    //w(x,y,z) should be approximately parallel to d(x,y,z), so
    //their dot product should be either 1 or -1
    auto dot = dx * wx + dy * wy + dz * wz;

    if (std::abs(std::abs(dot) - 1.0) > 1e-3)
        mdm_ProgramLogger::logProgramWarning(__func__,
            seriesName + ": cross product of row and column axes orientation does not match "
            "the orientation of the vector from first to last slice image positions");

    //The z-direction is the sign of dot multiple by the distance between slices
    info.zDirection.setValue( dot > 0 ? zd : -zd );

}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::completeSeriesInfo(dcmSeriesInfo &series, int nDyns)
{
  if (series.filenames.empty())
    return;

  DcmFileFormat fileformat;
  OFCondition status = fileformat.loadFile(series.filenames[0].c_str());

  if (!status.good())
    throw mdm_exception(__func__, "Unable to open " + series.filenames[0]);

  //Complete series name if it's empty
  if (series.name.empty())
    getSeriesName(series, fileformat);
  
  getTextInfo(fileformat, DCM_Manufacturer, series.manufacturer);
  
  getNumericInfo(fileformat, DCM_NumberOfTemporalPositions, series.nTimes);

  if (nDyns > series.nTimes)
    series.nTimes = nDyns;

  //Call checkSortValid to compute nZ and set sortValid
  checkSortValid(series);
  getNumericInfo(fileformat, DCM_Columns, series.nX);
  getNumericInfo(fileformat, DCM_Rows, series.nY);

  std::vector<double> pixelSpacing;
  getScannerSetting(
    fileformat, series.name, "PixelSpacing",
    DCM_PixelSpacing, true, pixelSpacing, 2);
  series.Xmm = pixelSpacing[0];
  series.Ymm = pixelSpacing[1];
  getNumericInfo(fileformat, DCM_SliceThickness, series.Zmm);

  //Get scanner settings
  getScannerSetting(
    fileformat, series.name, "FA", options_.FATag, 
    DCM_FlipAngle, options_.FARequired(), series.FA);

  getScannerSetting(
    fileformat, series.name, "TR", options_.TRTag, 
    DCM_RepetitionTime, options_.TRRequired(), series.TR);

  getScannerSetting(
    fileformat, series.name, "TI", options_.TITag, 
    DCM_InversionTime, options_.TIRequired(), series.TI);

  getScannerSetting(
    fileformat, series.name, "TE", options_.TETag, 
    DCM_EchoTime, options_.TERequired(), series.TE);

  getScannerSetting(
    fileformat, series.name, "B", options_.BTag, 
    DCM_DiffusionBValue, options_.BRequired(), series.B);

  getScannerSetting(
    fileformat, series.name, "gradientOrientation", options_.gradOriTag, 
    DCM_DiffusionGradientOrientation, options_.gradOriRequired(), series.gradOri);

  getScannerSetting(
    fileformat, series.name, "acquisitionTime", options_.dynTimeTag,
    DCM_AcquisitionTime, options_.dynTimeRequired(), series.acquisitionTime);
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::printSeriesInfoSummary(const dcmSeriesInfo &series, std::ofstream& file)
{
  std::string vol = series.nTimes > 1 ? "volumes" : "volume";
  std::stringstream ss;

  ss <<
    "Series " << series.index << ": " 
    << series.numericInfo[0].seriesNumber << " "
    << series.name <<
    ", " << series.nTimes << " " << vol << " of size " <<
    "(" << series.nX << " x " << series.nY << " x " << series.nZ << ")" <<
    ", voxel size (" <<
    series.Xmm << " x " <<
    series.Ymm << " x " <<
    series.Zmm << ")\n";

  mdm_ProgramLogger::logProgramMessage(ss.str());

  file << ss.str();
}

//-----------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::writeSeriesInfo(
  std::vector<dcmSeriesInfo> &seriesInfo)
{

  //Create file for saving the series names
  auto seriesFileRoot = outputPath_ / options_.dicomSeriesFile();
  auto seriesNamesFile = seriesFileRoot.string() + "_names.txt";
  auto seriesSummaryFile = seriesFileRoot.string() + "_summary.txt";

  // Open text files for writing filenames and summary info
  std::ofstream seriesNamesFileStream(seriesNamesFile.c_str(), std::ios::out);
  if (!seriesNamesFileStream)
    throw mdm_exception(__func__, "Can't open series text file for writing " + seriesNamesFile);

  std::ofstream seriesSummaryFileStream(seriesSummaryFile.c_str(), std::ios::out);
  if (!seriesSummaryFileStream)
    throw mdm_exception(__func__, "Can't open series text file for writing " + seriesSummaryFile);

  //Print number of series found
  auto nSeries = seriesInfo_.size();
  mdm_ProgramLogger::logProgramMessage(
    "Found " + std::to_string(nSeries) + " series:");

  seriesSummaryFileStream << "Found " << nSeries << " series:\n\n";

  for (auto &series : seriesInfo)
  {
    //Complete the series information and print summary
    completeSeriesInfo(series);
    printSeriesInfoSummary(series, seriesSummaryFileStream);

    //Complete the series info file entries
    auto seriesName = series.name;

    //Write the unmodified series name
    seriesNamesFileStream << seriesName << '\n';

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
  seriesNamesFileStream.close();
  seriesSummaryFileStream.close();
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
    auto nDyns = (series.index == options_.dynSeries()) ? options_.nDyns() : 0;
    completeSeriesInfo(series, nDyns);
  }
}

//--------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::sortDicomDir()
{
  checkDynamicTime();
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

      std::string seriesName;
      getTextInfo(fileformat, DCM_SeriesDescription, seriesName);
      if (seriesName.empty())
        getTextInfo(fileformat, DCM_ProtocolName, seriesName);

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

  //Write output lists
  writeSeriesInfo(seriesInfo_);

}

 //---------------------------------------------------------------------
mdm_Image3D mdm_RunTools_madym_DicomConvert::loadDicomImage(
  const dcmSeriesInfo& series, const int startIdx,
  bool isDynamic, int dynNum)
{
  //Get vector filenames for this slice
  std::vector<std::string> sliceNames(series.filenames.begin() + startIdx,
    series.filenames.begin() + startIdx + series.nZ);

  return loadDicomImage(series, sliceNames, isDynamic, dynNum);
}

//---------------------------------------------------------------------
mdm_Image3D mdm_RunTools_madym_DicomConvert::loadDicomImage(const dcmSeriesInfo& series,
  const std::vector<std::string>& sliceNames,
  bool isDynamic, int dynNum,
  double Bvalue, const std::vector<double> gradOri)
{
  DcmFileFormat fileformat;
  OFCondition status = fileformat.loadFile(sliceNames[0].c_str());
  if (!status.good())
    throw mdm_exception(__func__, "Unable to open " + sliceNames[0]);
  
  //Get various bits of info from DICOM header
  double offset = options_.dicomOffset();
  double scale = options_.dicomScale();
  applyAutoScaling(fileformat, offset, scale);

  auto nZ = sliceNames.size();
  std::vector<size_t> dimensions = { (size_t)series.nX, (size_t)series.nY, nZ };
  std::vector <double> pixelSpacing = { series.Xmm, series.Ymm, series.Zmm };

  //Make the image from the filenames
  auto img = mdm_DicomFormat::loadImageFromDicomSlices(
    dimensions, pixelSpacing, sliceNames, offset, scale,
    options_.flipX(), options_.flipY(), options_.flipZ());

  //Now set meta data from DICOM fields
  auto& info = img.info();

  //Set scl slope and intercept from scale and offset - note sclSlope is a multiplier not a divisor, so take reciprocal
  info.sclSlope.setValue(1 / scale);
  info.sclInter.setValue(offset);

  //Get image position information
  getVolumeAxesDirections(
      sliceNames.front(), sliceNames.back(), series.name, nZ, info);

  info.flipX.setValue(options_.flipX());
  info.flipY.setValue(options_.flipY());
  info.flipZ.setValue(options_.flipZ());

  if (series.FA)
    info.flipAngle.setValue(series.FA);
  if (series.TR)
    info.TR.setValue(series.TR);
  if (series.TE)
    info.TE.setValue(series.TE);
  if (series.TI)
    info.TI.setValue(series.TI);

  if (Bvalue >= 0)
  {
    info.B.setValue(Bvalue);
    info.gradOriX.setValue(gradOri[0]);
    info.gradOriY.setValue(gradOri[1]);
    info.gradOriZ.setValue(gradOri[2]);
  }
    
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
  series.nZ = (int)series.numericInfo.size() / series.nTimes;
  series.sortValid = true;

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
  for (int i_z = 0; i_z < series.nZ; i_z++)
  {
    for (int i_t = 1; i_t < series.nTimes; i_t++)
    {
      const auto& info_t0 = series.numericInfo[(i_t-1)*series.nZ + i_z];
      const auto &info_t1 = series.numericInfo[i_t*series.nZ + i_z];
      series.sortValid = info_t1.sliceLocation == info_t0.sliceLocation &&
          info_t1.temporalPositionIdentifier > info_t0.temporalPositionIdentifier;
      
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
void mdm_RunTools_madym_DicomConvert::setDicomTag(const mdm_input_dicom_tag&tagInput, DcmTagKey &tag)
{
  try
  {
    auto group = std::stoul(tagInput().first, 0, 16);
    auto element = std::stoul(tagInput().second, 0, 16);
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
  if (!options_.autoOffsetTag().first.empty())
    setDicomTag(options_.autoOffsetTag, autoOffsetTag_);

  if (!options_.autoScaleTag().first.empty())
    setDicomTag(options_.autoScaleTag, autoScaleTag_);
}

//
void mdm_RunTools_madym_DicomConvert::checkDynamicTime()
{
  //If input options dynTimeTag is not empty, attempt to set the tag from that
  if (!options_.dynTimeTag().first.empty())
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
  if (temporalResolution_ <= 0 && dynamicTimeTag_.hasValidGroup())
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

    //Check how to process timestamp
    if (options_.dynTimeFormat() == "timestamp")
    {
      ; //Do nothing
    }
    else if (options_.dynTimeFormat() == "msecs")
    {
      dynTime = mdm_Image3D::secsToTimestamp(dynTime / 1000);
    }
    else if (options_.dynTimeFormat() == "seconds")
    {
      dynTime = mdm_Image3D::secsToTimestamp(dynTime);
    }
    else if (options_.dynTimeFormat() == "minutes")
    {
      dynTime = mdm_Image3D::secsToTimestamp(60*dynTime);
    }
    else
    {
      throw mdm_exception(__func__, "Value for " + std::string(options_.dynTimeFormat.key()) +
        "option (" + options_.dynTimeFormat() +
        ") not recognised. Must be one of [timestamp, msecs, seconds, minutes]");
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
  //Deal with deprecated volumeName input
  if (!options_.volumeName().empty() && options_.singleVolNames().empty())
    options_.singleVolNames.set({ options_.volumeName() });

  //Check length of series indices match length of volume names
  auto nSeries = options_.singleVolNames().size();
  if ( options_.singleSeries().size() != nSeries)
    throw mdm_exception(__func__, boost::format(
      "Length of %1% (%2%) does not match length of %3% (%4%)."
    ) % options_.singleSeries.key() % options_.singleSeries().size() 
    % options_.singleVolNames.key() % nSeries);

  checkAutoScaling();

  for (size_t i = 0; i < nSeries; i++)
  {
    const auto& index = options_.singleSeries()[i] - 1;
    if (index < 0 || index >= seriesInfo.size())
      throw mdm_exception(__func__,
        boost::format("Dicom series index (%1%) must be >= 0 and < %2%")
        % index % seriesInfo.size());

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

    auto volumeName = fs::absolute(options_.singleVolNames()[i]);
    fs::create_directories(volumeName.parent_path());

    mdm_ImageIO::writeImage3D(imageWriteFormat,
      volumeName.string(), img, imageDatatype, xtrType_, options_.niftiScaling());
    mdm_ProgramLogger::logProgramMessage("Created 3D image " + volumeName.string() + " from series " + 
      std::to_string(series.index) + ": " + series.name);
  }

  
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::makeT1InputVols(
  const std::vector<dcmSeriesInfo> &seriesInfo)
{
  checkAutoScaling();

  auto nInputs = options_.T1inputSeries().size();
  if (nInputs != options_.T1inputNames().size())
    throw mdm_exception(__func__,
      boost::format("Number of elements in %1% (%2%) does not match %3% (%4%)")
      % options_.T1inputNames.key() % options_.T1inputNames().size()
      % options_.T1inputSeries.key() % nInputs);

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

    mdm_ProgramLogger::logProgramMessage("Creating T1 input files " + T1Name + " from series " +
      std::to_string(series.index) + ": " + series.name + " ...");

    //Check this sequence was validly sorted
    if (!series.sortValid)
      throw mdm_exception(__func__, boost::format(
        "Series %1% was not sorted properly. Check the series log files"
      ) % series.name);

    //Set up mean image
    mdm_Image3D meanImg;

    //Set up output path/directory - outpath is already absolute
    fs::path T1Dir = fs::path(fs::absolute(options_.T1Dir())) / fs::path(T1Name);

    //For 3D writing, create a directory for the indiviudal repeats
    if (!options_.nifti4D())
      fs::create_directories(T1Dir);

    //Create images container in case writing 4D. If we're not, it will stay unused
    std::vector< mdm_Image3D > imgs;
    for (int i_rpt = 0; i_rpt < series.nTimes; i_rpt++)
    {
      auto startIdx = i_rpt * series.nZ;
      auto img = loadDicomImage(series, startIdx);
      img.setType(mdm_Image3D::ImageType::TYPE_T1WTSPGR);

      if (options_.nifti4D())
        imgs.push_back(img);

      else
      {
        //Make output name
        auto outputName = mdm_SequenceNames::makeSequenceFilename(
          T1Dir.string(), options_.repeatPrefix(), i_rpt + 1, options_.sequenceFormat(),
          options_.sequenceStart(), options_.sequenceStep());

        //Write the output image and xtr file
        mdm_ImageIO::writeImage3D(imageWriteFormat,
          outputName, img, imageDatatype, xtrType_, options_.niftiScaling());
        mdm_ProgramLogger::logProgramMessage("Created T1 input file " + outputName);
      }

      if (options_.makeT1Means())
      {
        if (!i_rpt)
          meanImg = img;

        else
          meanImg += img;
      }
    }
    if (options_.nifti4D())
    {
      //For 4D writing, the name is just the T1 dir we'd have used for the 3D writing
      auto outputName = T1Dir;
      fs::create_directories(outputName.parent_path());


      //Write the output image and xtr file
      mdm_ImageIO::writeImage4D(imageWriteFormat,
        outputName.string(), imgs, imageDatatype, xtrType_, options_.niftiScaling());
      mdm_ProgramLogger::logProgramMessage("Created 4D T1 input file " + outputName.string());
    }
    if (options_.makeT1Means())
    {
      //Finalise the mean image
      meanImg /= series.nTimes;
      auto meanName = T1Dir.string() + options_.meanSuffix();
      mdm_ImageIO::writeImage3D(imageWriteFormat,
        meanName, meanImg, imageDatatype, xtrType_, options_.niftiScaling());
      mdm_ProgramLogger::logProgramMessage("Created mean T1 input file " + meanName);
    }
  }
}

//---------------------------------------------------------------------
std::vector<mdm_RunTools_madym_DicomConvert::DWIBvalueVolumes>
  mdm_RunTools_madym_DicomConvert::sortDWI(const dcmSeriesInfo &seriesInfo)
{
  //Set-up container for info on each B-value/gradient combo
  std::vector<DWIBvalueVolumes> DWIBvalueList;

  //Loop through each file in series, assigning to unique combos
  //of B-value and gradient orientation. We can assume filenames are already
  //sorted by slice-location
  auto nImages = seriesInfo.numericInfo.size();
  for (size_t i_im = 0; i_im < nImages; i_im++)
  {
    const auto& file = seriesInfo.filenames[i_im];

    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(file.c_str());

    if (!status.good())
      mdm_ProgramLogger::logProgramWarning(__func__,
        file + ": cannot read DICOM file (" + status.text() + ")");

    //Get B-value and gradient orientation
    double Bvalue;
    getScannerSetting(
      fileformat, seriesInfo.name, "B-value", options_.BTag,
      DCM_DiffusionBValue, true, Bvalue);

    std::vector<double> gradOri;
    getScannerSetting(
      fileformat, seriesInfo.name, "gradientOrientation", options_.gradOriTag,
      DCM_DiffusionGradientOrientation, true, gradOri, 3);

    //Check if this B-value/gradient already found, and if so, add this filename
    // to the matched DWI volume info
    auto createNew = true;
    for (auto& BvalueInfo : DWIBvalueList)
    {
      if (Bvalue == BvalueInfo.Bvalue)
      {
        for (auto& volumeInfo : BvalueInfo.volumes)
        {
          if (gradOri == volumeInfo.gradOri)
          {
            volumeInfo.fileNames.push_back(file);
            createNew = false;
            break;
          }
        }
        
        if (createNew)
        {
          //B-value matched, but new orientation
          DWIvolumeInfo volumeInfo;
          volumeInfo.fileNames.push_back(file);
          volumeInfo.Bvalue = Bvalue;
          volumeInfo.gradOri = gradOri;
          BvalueInfo.volumes.push_back(volumeInfo);
          createNew = false;
        }
      }
    }

    //New B-value
    if (createNew)
    {
      //Create a new B-value info struct
      DWIBvalueVolumes BvalueInfo;
      BvalueInfo.Bvalue = Bvalue;

      //Create a new grad volumes struct and add this to B-value struct
      DWIvolumeInfo volumeInfo;
      volumeInfo.fileNames.push_back(file);
      volumeInfo.Bvalue = Bvalue;
      volumeInfo.gradOri = gradOri;
      BvalueInfo.volumes.push_back(volumeInfo);

      //Add the new Bvalue info to the main list
      DWIBvalueList.push_back(BvalueInfo);
    }  
  }

  //Check we've sorted into a valid sequence
  if (!checkDWIsortValid(DWIBvalueList))
    throw mdm_exception(__func__, boost::format(
      "DWI series %1% was not sorted properly. Check the series log files"
    ) % seriesInfo.name);

  //Otherwise return the volume info
  return DWIBvalueList;
  
}

//---------------------------------------------------------------------
bool mdm_RunTools_madym_DicomConvert::checkDWIsortValid(const std::vector<DWIBvalueVolumes>& DWIBvalueList)
{
  //All volumes should have the same number of slices
  size_t nSlices = 0 ;
  for (const auto & BValueInfo : DWIBvalueList)
  {
    for (const auto volumeInfo : BValueInfo.volumes)
    {
      if (!nSlices)
        nSlices = volumeInfo.fileNames.size();

      else if (volumeInfo.fileNames.size() != nSlices)
        return false;
    }
    

  }
  return true;
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::makeDWIInputs(
  const std::vector<dcmSeriesInfo>& seriesInfo)
{
  auto nInputs = options_.DWIinputSeries().size();
  if (nInputs != options_.DWIinputNames().size())
    throw mdm_exception(__func__,
      boost::format("Number of elements in %1% (%2%) does not match %3% (%4%)")
      % options_.DWIinputNames.key() % options_.DWIinputNames().size() 
      % options_.DWIinputSeries.key() % nInputs);

  for (size_t i_dwi = 0; i_dwi < nInputs; i_dwi++)
  {
    const auto& DWIName = options_.DWIinputNames()[i_dwi];
    const auto& index = options_.DWIinputSeries()[i_dwi] - 1;

    if (index < 0 || index >= seriesInfo.size())
      throw mdm_exception(__func__,
        boost::format("DWI input series index (%1%) must be >= 0 and < %2%")
        % index % seriesInfo.size());

    mdm_ProgramLogger::logProgramMessage("Creating DWI input files " 
      + DWIName + " from series " +
      std::to_string(seriesInfo[index].index) 
      + ": " + seriesInfo[index].name + " ...");

    makeDWIInputVols(seriesInfo[index], DWIName);
  }
    
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_DicomConvert::makeDWIInputVols(
  const dcmSeriesInfo& seriesInfo, const std::string& basename)
{
  checkAutoScaling();

  //Get list of filenames sorted in unique combos of B-value and orientation
 auto DWIBvalueList = sortDWI(seriesInfo);

  //Get read and write format
  auto imageWriteFormat = mdm_ImageIO::formatFromString(options_.imageWriteFormat());
  auto imageDatatype = static_cast<mdm_ImageDatatypes::DataType>(options_.imageDataType());

  //For 4D writing, make a single vectors of all B-value inputs, and a second vector of mean imgs
  //for 3D writing these will stay unused
  auto write4D = options_.nifti4D();
  std::vector< mdm_Image3D > imgs;
  std::vector< mdm_Image3D > mean_imgs;

  //Loop over each B-value/Gradient orientation combo
  for (const auto BvalueInfo : DWIBvalueList)
  {

    //Set up mean image
    mdm_Image3D meanImg;

    //Make B-value basename
    auto BvalueName = write4D ? "" :(boost::format("%1%_%2%") % basename % int(BvalueInfo.Bvalue)).str();
    fs::path DWIDir = write4D ? "" : fs::path(options_.DWIDir()) / fs::path(BvalueName);

    if (!write4D)
    {
      //Set up output directory - outpath is already absolute
      fs::create_directories(DWIDir);
    }
    

    auto nVolumes = BvalueInfo.volumes.size();
    for (size_t i_v = 0; i_v < nVolumes; i_v++)
    {
      const auto volumeInfo = BvalueInfo.volumes[i_v];
      auto img = loadDicomImage(seriesInfo, volumeInfo.fileNames, false, 0, volumeInfo.Bvalue, volumeInfo.gradOri);
      img.setType(mdm_Image3D::ImageType::TYPE_DWI);

      if (write4D)
        imgs.push_back(img);
      else
      {
        //Make output name
        auto seq_start = volumeInfo.Bvalue ? options_.sequenceStart() : 0;
        auto outputName = mdm_SequenceNames::makeSequenceFilename(
          DWIDir.string(), BvalueName + "_orient_", i_v + 1, options_.sequenceFormat(),
          seq_start, options_.sequenceStep());

        //Write the output image and xtr file
        mdm_ImageIO::writeImage3D(imageWriteFormat,
          outputName, img, imageDatatype, xtrType_, options_.niftiScaling());
        mdm_ProgramLogger::logProgramMessage("Created DWI input file " + outputName);
      }

      if (options_.makeBvalueMeans())
      {
        if (!i_v)
          meanImg = img;

        else
          meanImg += img;
      }

    }

    if (options_.makeBvalueMeans())
    {
      if (write4D)
        mean_imgs.push_back(meanImg);

      else
      {
        //Finalise the mean image
        meanImg /= nVolumes;
        auto meanName = (boost::format("%1%%2%") % DWIDir.string() % options_.meanSuffix()).str();

        mdm_ImageIO::writeImage3D(imageWriteFormat,
          meanName, meanImg, imageDatatype, xtrType_, options_.niftiScaling());
        mdm_ProgramLogger::logProgramMessage("Created mean DWI input file " + meanName);
      }
    }
  }

  //Write the 4D image volumes for the indiviudal B-values and their means
  if (write4D)
  {
    auto DWIName = fs::path(options_.DWIDir()) / basename;
    fs::create_directories(DWIName.parent_path());

    mdm_ImageIO::writeImage4D(imageWriteFormat,
      DWIName.string(), imgs, imageDatatype, xtrType_, options_.niftiScaling());
    mdm_ProgramLogger::logProgramMessage("Created 4D DWI image " + DWIName.string());

    if (options_.makeBvalueMeans())
    {
      auto DWIMeanName = DWIName.string() + options_.meanSuffix();
      mdm_ImageIO::writeImage4D(imageWriteFormat,
        DWIMeanName, mean_imgs, imageDatatype, xtrType_, options_.niftiScaling());
      mdm_ProgramLogger::logProgramMessage("Created 4D image of DWI means over B-value " + DWIMeanName);
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
    mdm_ProgramLogger::logProgramWarning(__func__, (boost::format(
      "Series %1% was not sorted properly. Making dynamics will proceed but you are advised to check the output carefully."
    ) % series.name).str());

  //Set up mean image
  mdm_Image3D meanImg;

  //Set up output directory
  fs::path dynDir = fs::path(fs::absolute(options_.dynDir()));
  fs::create_directories(dynDir);

  //Cap the total number of dynamics saved if set in options
  auto nTimes = (options_.nDyns() && options_.nDyns() < series.nTimes) ?
    options_.nDyns() : series.nTimes;

  //Get write format from options
  auto imageWriteFormat = mdm_ImageIO::formatFromString(options_.imageWriteFormat());
  auto imageDatatype = static_cast<mdm_ImageDatatypes::DataType>(options_.imageDataType());
  
  mdm_ProgramLogger::logProgramMessage("Creating dynamic sequence with "
    + std::to_string(nTimes) + " timepoints from series " +
    std::to_string(series.index) + ": " + series.name + " ...");

  //Create images container in case writing 4D. If we're not, it will stay unused
  std::vector< mdm_Image3D > imgs;
  for (int i_dyn = 0; i_dyn < nTimes; i_dyn++)
  {
    auto startIdx = i_dyn * series.nZ;

    auto img = loadDicomImage(series, startIdx, true, i_dyn);
    img.setType(mdm_Image3D::ImageType::TYPE_T1DYNAMIC);

    if (options_.nifti4D())
      imgs.push_back(img);

    else
    {
      //Make output name
      auto outputName = mdm_SequenceNames::makeSequenceFilename(
        dynDir.string(), options_.dynName(), i_dyn + 1, options_.sequenceFormat(),
        options_.sequenceStart(), options_.sequenceStep());

      //Write the output image and xtr file
      mdm_ImageIO::writeImage3D(imageWriteFormat,
        outputName, img, imageDatatype, xtrType_, options_.niftiScaling());
      mdm_ProgramLogger::logProgramMessage("Created dynamic image " + outputName);
    }
    

    if (options_.makeDynMean())
    {
      if (!i_dyn)
        meanImg = img;

      else
        meanImg += img;
    }
  }

  //If 4D, write the images now
  if (options_.nifti4D())
  {
    auto dynName = dynDir / options_.dynName();

    mdm_ImageIO::writeImage4D(imageWriteFormat,
      dynName.string(), imgs, imageDatatype, xtrType_, options_.niftiScaling());
    mdm_ProgramLogger::logProgramMessage("Created 4D dynamic image " + dynName.string());
  }

  if (options_.makeDynMean())
  {
    //Finalise the mean image
    meanImg /= nTimes;
    meanImg.setType(mdm_Image3D::ImageType::TYPE_DYNMEAN);

    auto meanName = dynDir / (options_.dynName() + options_.meanSuffix());
    mdm_ImageIO::writeImage3D(imageWriteFormat,
      meanName.string(), meanImg, imageDatatype, xtrType_, options_.niftiScaling());
    mdm_ProgramLogger::logProgramMessage("Created temporal mean of dynamic images " + meanName.string());
  }
}