/**
*  @file    mdm_SequenceNames.h
*  @brief Header only class to generate sequence names
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_SEQUENCENAMES_HDR
#define MDM_SEQUENCENAMES_HDR

#include <string>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

//! Header only class to provide method for generating sequence names from user options
class mdm_SequenceNames {

public:

  //Generates sequence filenames from user options
  /*!
  \param path directory path to file name (leave empty if path included in prefix)
  \param prefix prefix of file name
  \param fileNumber current file number in sequence
  \param fileNumberFormat string format for converting number to string
  \param startIndex first index of volume names in index
  \param stepSize step between indexes of volume names in sequence
  \return filepath to current sequence volume
  */
  static std::string makeSequenceFilename(
    const std::string &path, const std::string &prefix,
    const int fileNumber, const std::string &fileNumberFormat,
    const int startIndex, const int stepSize)
  {
    auto index = (fileNumber-1) * stepSize + startIndex;
    auto formattedFilenumber = boost::format(fileNumberFormat.c_str()) % index;
    auto imageName = boost::format("%1%%2%") % prefix % formattedFilenumber;
    return path.empty() ? imageName.str() :
      (fs::path(path) / imageName.str()).string();
  }
};

#endif //MDM_SEQUENCENAMES_HDR