//! Manager class for reading input and writing ouput of volume-wise model analysis
/*!
*  @file    mdm_FileManager.h
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_FILELOAD_HDR
#define MDM_FILELOAD_HDR
#include "mdm_api.h"

#include "mdm_AIF.h"
#include "mdm_T1VolumeAnalysis.h"
#include "mdm_DCEVolumeAnalysis.h"
#include "mdm_ParamSummaryStats.h"
#include "mdm_ErrorTracker.h"

//!   Manager class for reading input and writing ouput of volume-wise model analysis
/*!
*/
class mdm_FileManager {

public:
	//! Maximum number of dynamic time-series images permitted
	static const int MAX_DYN_IMAGES;

	//! Constructor
	/*!
	\param volumeAnalysis reference to volume analysis object
	*/
	MDM_API mdm_FileManager(mdm_DCEVolumeAnalysis &volumeAnalysis);
		
	//! Destructor
	/*!
	*/
	MDM_API ~mdm_FileManager();

	//! Load in error codes map
	/*!
	\param errorPath filepath to error codes map
	\param warnMissing flag, if true, returns false if image file doesn't exist. 
	If false, silently creates new empty image if existing image doesn't exist.
	\return true if image loads successfully. False if file doesn't exist (and warnMissing true) or load error.
	*/
	MDM_API bool loadErrorMap(const std::string &errorPath, bool warnMissing = false);

	//! Load signal image volumes for mapping baseline T1
	/*!
	\param T1InputPaths list of filepaths to input signal images
	\return true if all images successfully loaded. False if any files don't exist or load errors.
	*/
	MDM_API bool loadT1MappingInputImages(const std::vector<std::string> &T1InputPaths);

	//! Load baseline T1 image
	/*!
	\param T1path filepath to baseline T1 image
	\return true if image loads successfully. False if file doesn't exist or load error.
	*/
	MDM_API bool loadT1Map(const std::string &T1path);

	//! Load M0 image
	/*!
	\param M0path filepath to M0 image
	\return true if image loads successfully. False if file doesn't exist or load error.
	*/
	MDM_API bool loadM0Map(const std::string &M0path);

	//! Load DCE time-series signal volumes
	/*!
	DCE time-series volumes are loaded by pattern matching a base file name with the series index 1,...,nDyns
	appended. The default pattern assumes the index is directly appended with no additional formatting, 
	thus if the base path is dynamic/dyn_, then it will load images dynamic/dyn_1.hdr, dynamic/dyn_2.hdr, etc.
	However a custom pattern can be specified, eg use "%03u" to load dynamic/dyn_001.hdr, dynamic/dyn_002.hdr, etc.
	\param basePath base of filepath to dynamic time-series images, to which StPrefix is appended
	\param StPrefix appended to basePath, forming base pattern to match files using series index
	\param nDyns number of images to load. If 0, loads all images matching filename pattern
	\param indexPattern string format specification, to convert integers 1,...,nDyns into a string
	\return true if all images successfully loaded. False if any files don't exist (before nDyns reached) or load errors.
	*/
	MDM_API bool loadStDataMaps(const std::string &basePath,
		const std::string &StPrefix, int nDyns, const std::string &indexPattern="%01u");

	//! Load DCE time-series contrast-agent concentration volumes
	/*!
	\param basePath base of filepath to dynamic time-series images, to which dynPrefix is appended
	\param CtPrefix appended to basePath, forming base pattern to match files using series index
	\param nDyns number of images to load. If 0, loads all images matching filename pattern
	\param indexPattern string format specification, to convert integers 1,...,nDyns into a string
	\return true if all images successfully loaded. False if any files don't exist (before nDyns reached) or load errors.
	\see loadStDataMaps
	*/
	MDM_API bool loadCtDataMaps(const std::string &basePath,
		const std::string &CtPrefix, int nDyns, const std::string &indexPattern = "%01u");

	//! Load ROI mask image
	/*!
	\param path of ROI mask image
	\return true if image loads successfully. False if file doesn't exist or load error.
	*/
	MDM_API bool loadROI(const std::string &path);

  //! Save ROI mask
  /*!
  \param outputDir directory path
  \param name of ROI mask image
  \return true if image saves successfully. False if save error.
  */
  MDM_API bool saveROI(const std::string &outputDir, const std::string &name);

  //! Load AIF map
  /*!
  \param path filepath to ROI mask image
  \return true if image loads successfully. False if file doesn't exist or load error.
  */
  MDM_API bool loadAIFmap(const std::string &path);

  //! Save AIF map
  /*!
  \param outputDir directory path
  \param name of AIF map
  \return true if image saves successfully. False if save error.
  */
  MDM_API bool saveAIFmap(const std::string &outputDir, const std::string &name);

	//! Load tracer-kinetic model parameter maps
	/*!
	\param paramDir directory containing parameter maps. This must contain image volumes with names matching the parameter names specified in the volume analysis model
	\return true if maps loads successfully. False if files doesn't exist or load error.
	\see mdm_DCEModelBase#paramNames
	*/
	MDM_API bool loadParameterMaps(const std::string &paramDir);

	//! Save all output maps to disk
	/*!
	\param outputDir directory in which to write output maps.
	\return true if maps saved successfully. False if save error.
	*/
	MDM_API bool saveOutputMaps(const std::string &outputDir);

	//! Save model residuals map to disk
	/*!
	\param outputDir directory in which to write output map.
	\return true if map saved successfully. False if save error.
	*/
	MDM_API bool saveModelResiduals(const std::string &outputDir);

	//! Save parameter stats file
	/*!
	\param outputDir directory in which to write output maps.
	\return true if stats saved successfully. False if save error.
	*/
	MDM_API bool saveSummaryStats(const std::string &outputDir);

	//! Save error codes map to disk
	/*!
	\param outputDir directory in which to write output map.
	\return true if map saved successfully. False if save error.
	*/
	MDM_API bool saveErrorMap(const std::string &outputDir);

	//! Set flag to write out signal-derived concentration time-series maps
	/*!
	\param flag if true, writes out time-series C(t)
	*/
	MDM_API void setSaveCtDataMaps(bool flag);

	//! Set flag to write out model estimated concentration time-series maps
	/*!
	\param flag if true, writes out time-series Cm(t)
	*/
	MDM_API void setSaveCtModelMaps(bool flag);

	//! Set flag to write out in sparse format
	/*!
	\param flag if true, writes out images using sparse format
	\see mdm_AnalyzeFormat
	*/
	MDM_API void setSparseWrite(bool flag);

protected:

private:

	/*METHODS*/

	bool loadT1InputImage(const std::string& filePath, int nVFA);

	void makeSequenceFilename(const std::string &path, const std::string &prefix,
		const int fileNumber, std::string &filePath, const std::string &fileNumberFormat);
	
	bool saveOutputMap(const std::string &mapName, 
		const std::string &outputDir, bool writeXtr = false);

	bool saveOutputMap(const std::string &mapName, const mdm_Image3D &img, 
		const std::string &outputDir, bool writeXtr = false);

	bool saveMapsSummaryStats(const std::string &roiName, mdm_ParamSummaryStats &stats);

	bool saveMapSummaryStats(const std::string &mapName, const mdm_Image3D &img, 
		mdm_ParamSummaryStats &stats);

	/*VARIABLES*/

	/*Object that store the loaded data and used in processing - 
	set at global level and maintain a local reference in this class*/
	mdm_T1VolumeAnalysis &T1Mapper_;
	mdm_DCEVolumeAnalysis &volumeAnalysis_;
	mdm_ErrorTracker &errorTracker_;

	bool writeCtDataMaps_;
  bool writeCtModelMaps_;
	bool sparseWrite_;
};

#endif /* MDM_FILELOAD_HDR */

