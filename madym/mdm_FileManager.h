/**
 *  @file    mdm_FileManager.h
 *  @brief   Manager class for reading input and writing ouput of volume-wise model analysis
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_FILELOAD_HDR
#define MDM_FILELOAD_HDR
#include "mdm_api.h"

#include "mdm_AIF.h"
#include "mdm_T1VolumeAnalysis.h"
#include "mdm_DCEVolumeAnalysis.h"
#include "mdm_ErrorTracker.h"

 /**
	*  @brief   Manager class for reading input and writing ouput of volume-wise model analysis
	*/
class mdm_FileManager {

public:
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_FileManager(mdm_AIF &AIF,
		mdm_T1VolumeAnalysis &T1Mapper,
		mdm_DCEVolumeAnalysis &volumeAnalysis,
		mdm_ErrorTracker &errorTracker);
		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_FileManager();

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadErrorImage(const std::string &errorPath, bool warnMissing = false);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadFAImages(const std::vector<std::string> &FApaths);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadT1Image(const std::string &T1path);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadM0Image(const std::string &M0path);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadStDataMaps(const std::string &dynBasePath,
		const std::string &dynPrefix, int nDyns);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadCtDataMaps(const std::string &catBasePath,
		const std::string &catPrefix, int nDyns);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadAIF(const std::string &AIFpath);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadPIF(const std::string &PIFpath);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadROI(const std::string &ROIpath);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool loadParameterMaps(const std::string &paramDir);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool writeOutputMaps(const std::string &outputDir);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool writeModelResiduals(const std::string &outputPath);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool writeErrorMap(const std::string &outputDir);

		/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setWriteCtDataMaps(bool b);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API void setWriteCtModelMaps(bool b);

	/**
* @brief

* @param
* @return
*/
	MDM_API void setSparseWrite(bool b);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool saveAIF(const std::string &AIFpath);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API bool savePIF(const std::string &PIFpath);

protected:

private:

	/*METHODS*/

	bool loadFAImage(const std::string& filePath, int nVFA);

	void makeSequenceFilename(const std::string &path, const std::string &prefix,
		const int fileNumber, std::string &filePath);
	
	bool writeOutputMap(const std::string &mapName, 
		const std::string &outputDir, bool writeXtr = false);

	bool writeOutputMap(const std::string &mapName, const mdm_Image3D &img, 
		const std::string &outputDir, bool writeXtr = false);

	/*VARIABLES*/

	/*Object that store the loaded data and used in processing - 
	set at global level and maintain a local reference in this class*/
	mdm_AIF &AIF_;
	mdm_T1VolumeAnalysis &T1Mapper_;
	mdm_DCEVolumeAnalysis &volumeAnalysis_;
	mdm_ErrorTracker &errorTracker_;

	/*Full file paths to loaded images and AIF*/
	std::vector<std::string>  FAPaths_;
	std::vector<std::string>  dynPaths_;
	std::vector<std::string>  catPaths_;

	std::string  T1Path_;
	std::string  M0Path_;
	std::string  AIFPath_;
  std::string  PIFPath_;
	std::string  ROIPath_;

	bool writeCtDataMaps_;
  bool writeCtModelMaps_;
	bool sparseWrite_;
};

#endif /* MDM_FILELOAD_HDR */

