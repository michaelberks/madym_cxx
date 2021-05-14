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

class mdm_SequenceNames {

public:
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