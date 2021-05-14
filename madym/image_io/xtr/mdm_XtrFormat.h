/*!
 *  @file    mdm_XtrFormat.h
 *  @brief   Class for Analyze image format reading and writing
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_XTRFORMAT_H
#define MDM_XTRFORMAT_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>

 //! Analyze image format reading and writing
	/*!
	*/
class mdm_XtrFormat {

public:

	//!    Enum of recognized .xtr formats
	/*!
	.xtr files are used by Madym to encode meta-information not stored in 
	Analyze headers. This enum checks the xtr version. The .xtr version will
	be detected automatically during read. The new format will be used for writing.
	*/
	enum XTR_type {
		NO_XTR = -1, ///< Image does not have a matching .xtr file
		OLD_XTR = 0, ///< Old format
		NEW_XTR = 1, ///< Current format
	};

  //! Read XTR file
  /*!
  \param xtrFileName name of file to read
  \param img Image object to update with meta-parameters from XTR file
  */
  MDM_API static void readAnalyzeXtr(const std::string &xtrFileName,
    mdm_Image3D &img);

  //! Write XTR file
  /*!
  \param baseName name of file to write (will be appended with .xtr)
  \param img Image object with meta-parameters to write
  \param typeFlag flag to write in new or old format
  */
  MDM_API static void writeAnalyzeXtr(const std::string &baseName,
    const mdm_Image3D &img,
    const XTR_type typeFlag);

protected:

private:

	//
	static void writeNewXtr(std::ofstream &xtrFileStream,
		const mdm_Image3D &img);

	//
	static void writeOldXtr(std::ofstream &xtrFileStream,
		const mdm_Image3D &img);

	//
	static void readOldXtr(std::ifstream &xtrFileStream,
		mdm_Image3D &img);

	//
	static void readNewXtr(std::ifstream &xtrFileStream,
		mdm_Image3D &img);
};


#endif /* MDM_ANALYZEFORMAT_H */
