//! Manager class for reading input and writing ouput of volume-wise model analysis
/*!
*  @file    mdm_FileManager.h
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_FILELOAD_HDR
#define MDM_FILELOAD_HDR
#include <madym/utils/mdm_api.h>

#include "mdm_VolumeAnalysis.h"
#include "mdm_ParamSummaryStats.h"
#include <madym/image_io/mdm_ImageDatatypes.h>
#include <madym/image_io/mdm_ImageIO.h>
#include <madym/image_io/meta/mdm_XtrFormat.h>
#include <madym/utils/mdm_Image3D.h>

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
	MDM_API mdm_FileManager(mdm_VolumeAnalysis &volumeAnalysis);
		
	//! Destructor
	/*!
	*/
	MDM_API ~mdm_FileManager();

	//! Load in error tracker map
	/*!
	\param errorPath filepath to error tracker map
	*/
	MDM_API void loadErrorTracker(const std::string &errorPath);

	//! Load signal image volumes for mapping baseline T1
	/*!
	\param T1InputPaths list of filepaths to input signal images
	\param useNifti4D if true, load 4D NIFTI images and return average of repeats for each input
	*/
	MDM_API void loadT1MappingInputImages(const std::vector<std::string> &T1InputPaths, bool useNifti4D);

	//! Load baseline T1 image
	/*!
	\param path filepath to baseline T1 image
	*/
	MDM_API void loadT1Map(const std::string &path);

	//! Load M0 image
	/*!
	\param path filepath to M0 image
	*/
	MDM_API void loadM0Map(const std::string &path);

  //! Load B1 correction map
  /*!
  \param path filepath to B1 correction image
  \param B1Scaling scaling applied to B1 correction
  */
  MDM_API void loadB1Map(const std::string &path, const double B1Scaling);

	//! Load signal image volumes for DWI modelling
	/*!
	\param DWIInputPaths list of filepaths to input signal images
	\param useNifti4D if true, load 4D NIFTI images and return average of each B-value as 3D image to add
	*/
	MDM_API void loadDWIMappingInputImages(const std::vector<std::string>& DWIInputPaths, bool useNifti4D);

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
  \param startIndex start index of sequence names
  \param stepSize step size between indices in sequence names
	\param Ct if true, loads as concentration maps, if false signal images
	*/
	MDM_API void loadDynamicTimeseries(const std::string &basePath,
		const std::string &StPrefix, int nDyns, const std::string &indexPattern,
    const int startIndex, const int stepSize, bool Ct);

	//! Load DCE time-series signal volumes from a 4D nifti
	/*!
	DCE time-series loaded from a single 4D NIFTI image
	\param basePath base of filepath to dynamic time-series images, to which StPrefix is appended
	\param StName appended to basePath, forming full path to image
	\param Ct if true, loads as concentration maps, if false signal images
	*/
	MDM_API void loadDynamicTimeseries(const std::string& basePath,
		const std::string& StName, bool Ct);

	//! Load ROI mask image
	/*!
	\param path of ROI mask image
	*/
	MDM_API void loadROI(const std::string &path);

  //! Load AIF map
  /*!
  \param path filepath to AIF map
  */
  MDM_API void loadAIFmap(const std::string &path);

	//! Load tracer-kinetic model parameter maps
	/*!
	\param paramDir directory containing parameter maps. This must contain image volumes with names matching the parameter names specified in the volume analysis model
  \param initMapParams indices of parameters to load from maps (indexing starts at 1 as from user input)
	\see mdm_DCEModelBase#paramNames
	*/
	MDM_API void loadParameterMaps(const std::string &paramDir,
    const std::vector<int> &initMapParams);

  //! Load model residuals
  /*!
  \param path filepath to residuals
  */
  MDM_API void loadModelResiduals(const std::string &path);

  //! Save ROI mask
  /*!
  \param outputDir directory path
  \param name of output image file
  */
  MDM_API void saveROI(const std::string &outputDir, const std::string &name);

  //! Save AIF map
  /*!
  \param outputDir directory path
  \param name of output image file
  */
  MDM_API void saveAIFmap(const std::string &outputDir, const std::string &name);

	//! Save General output maps to disk
	/*!
	\param outputDir directory in which to write output maps.
	*/
	MDM_API void saveGeneralOutputMaps(const std::string& outputDir);

	//! Save T1 output maps to disk
	/*!
	\param outputDir directory in which to write output maps.
	*/
	MDM_API void saveT1OutputMaps(const std::string& outputDir);

	//! Save dynamic C(t) maps to disk as series of 3D vols
	/*!
	\param outputDir directory in which to write output maps.
	\param Ct_sigPrefix prefix to use for signal-derived C(t)
	\param Ct_modPrefix prefix to use for modelled C(t)
	\param indexPattern string format specification, to convert integers 1,...,nDyns into a string
	\param startIndex start index of sequence names
	\param stepSize step size between indices in sequence names
	*/
	MDM_API void saveDynamicOutputMaps(const std::string& outputDir,
		const  std::string& Ct_sigPrefix, const  std::string& Ct_modPrefix,
		const std::string& indexPattern,
		const int startIndex, const int stepSize);

	//! Save dynamic C(t) maps to disk as single 4D image
	/*!
	\param outputDir directory in which to write output maps.
	\param Ct_sigPrefix prefix to use for signal-derived C(t)
	\param Ct_modPrefix prefix to use for modelled C(t)
	*/
	MDM_API void saveDynamicOutputMaps(const std::string& outputDir,
		const  std::string& Ct_sigPrefix, const  std::string& Ct_modPrefix);

	//! Save DCE-specific output maps to disk
	/*!
	\param outputDir directory in which to write output maps.
	*/
	MDM_API void saveDCEOutputMaps(const std::string& outputDir);

	//! Save DWI specific output maps to disk
	/*!
	\param outputDir directory in which to write output maps.
	*/
	MDM_API void saveDWIOutputMaps(const std::string& outputDir);

	//! Save model residuals map to disk
	/*!
	\param outputDir directory in which to write output map.
	\return true if map saved successfully. False if save error.
	*/
	MDM_API void saveModelResiduals(const std::string &outputDir);

	//! Save parameter stats file
	/*!
	\param outputDir directory in which to write output maps.
	*/
	MDM_API void saveSummaryStats(const std::string &outputDir);

	//! Save error codes map to disk
	/*!
	\param outputDir directory in which to write output map.
  \param name of output image file
	*/
	MDM_API void saveErrorTracker(const std::string &outputDir, const std::string &name);

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

  //! Set image format for reading
  /*!
  \param fmt image format, must match one of the valid format options, otherwise throws exception
  \see mdm_ImageIO
  */
  MDM_API void setImageReadFormat(const std::string &fmt);

  //! Set image format for writing output
  /*!
  \param fmt image format, must match one of the valid format options, otherwise throws exception
  \see mdm_ImageIO
  */
  MDM_API void setImageWriteFormat(const std::string &fmt);

	//! Set NIFTI rescaling flag
	/*!
	\param flag if true, applies scl slope intercept rescaling when reading/writing NIFTI images
	\see mdm_ImageIO
	*/
	MDM_API void setApplyNiftiScaling(bool flag);

	//! Set image meta information format for writing output
	/*!
	\param use_bids if true uses BIDS JSON format, otherwise uses original Madym XTR format
	*/
	MDM_API void setXtrType(bool use_bids);

protected:

private:

	/*METHODS*/
	
	void saveOutputMap(const std::string &mapName, 
		const std::string &outputDir, bool writeXtr = false);

	void saveOutputMap(const std::string &mapName, const mdm_Image3D &img, 
		const std::string &outputDir, bool writeXtr = false,
    const mdm_ImageDatatypes::DataType format = mdm_ImageDatatypes::DT_FLOAT);

  void saveMapsSummaryStats(const std::string &roiName, mdm_ParamSummaryStats &stats);

  void saveMapSummaryStats(const std::string &mapName, const mdm_Image3D &img,
		mdm_ParamSummaryStats &stats);

  template <class T> void loadAndSetImage(
    const std::string &path, const std::string &msgName, T setFunc,
    const mdm_Image3D::ImageType type, bool loadXtr, double scaling = 1.0);

	/*VARIABLES*/

	/*Object that store the loaded data and used in processing - 
	set at global level and maintain a local reference in this class*/
	mdm_VolumeAnalysis &volumeAnalysis_;

	bool writeCtDataMaps_;
  bool writeCtModelMaps_;
  mdm_ImageIO::ImageFormat imageWriteFormat_;
  mdm_ImageIO::ImageFormat imageReadFormat_;
	bool applyNiftiScaling_;
	mdm_XtrFormat::XTR_type xtrType_;
};

#endif /* MDM_FILELOAD_HDR */

