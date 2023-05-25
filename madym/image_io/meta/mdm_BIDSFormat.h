/*!
 *  @file    mdm_XtrFormat.h
 *  @brief   Class for Analyze image format reading and writing
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_BIDSFORMAT_H
#define MDM_BIDSFORMAT_H

#include <madym/utils/mdm_api.h>
#include <madym/utils/mdm_Image3D.h>

 //! Analyze image format reading and writing
	/*!
	*/
class mdm_BIDSFormat {


public:

  //! Read BIDS style json file for a single 3D image
  /*!
  \param fileName name of file to read
  \param img Image object to update with meta-parameters from JSON file
  */
  MDM_API static void readImageJSON(const std::string& fileName,
    mdm_Image3D& img);

  //! Read BIDS style json file for a set of 4D images
  /*!
  \param fileName name of file to read
  \param imgs 4D set of image objects to update with meta-parameters from JSON file
  */
  MDM_API static void readImageJSON(const std::string& fileName,
    std::vector<mdm_Image3D>& img);

  //! Write JSON file
  /*!
  \param baseName name of file to write (will be appended with .xtr)
  \param img Image object with meta-parameters to write
  \param typeFlag flag to write in new or old format
  */
  MDM_API static void writeImageJSON(const std::string& baseName,
    const mdm_Image3D& img);

  //! Write JSON file
  /*!
  \param baseName name of file to write (will be appended with .xtr)
  \param imgs 4D set of image object with meta-parameters to write
  \param typeFlag flag to write in new or old format
  */
  MDM_API static void writeImageJSON(const std::string& baseName,
    const std::vector<mdm_Image3D>& imgs);

protected:

private:

};
#endif /* MDM_BIDSFORMAT_H */
