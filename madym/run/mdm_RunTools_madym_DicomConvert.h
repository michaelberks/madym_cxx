/*!
*  @file    mdm_RunTools_madym_DicomConvert.h
*  @brief   Defines class mdm_RunTools_madym_DicomConvert to run the DICOM image converter tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_DCE_LITE_HDR
#define MDM_RUNTOOLS_MADYM_DCE_LITE_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>

#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dctk.h> 

//! Helper struct to hold numeric dicom header info for sorter
struct dcmNumericInfo {
  int seriesNumber; ///< series number
  int  acquisitionNumber; ///< acquisitionNumber
  int  temporalPositionIdentifier;  ///< temporalPositionIdentifier
  double  sliceLocation;  ///< sliceLocation
  int instanceNumber;  ///< instanceNumber
};

//! Defines the set of information for an invidual dicome series
struct dcmSeriesInfo {
  std::string name; ///< Name of the series
  std::string manufacturer; ///< Manufacturer of the scanner (eg Philips)
  int index; ///< Index in the list of sequences processed
  std::vector<std::string> filenames; ///< List of paths to DICOM images in the series
  std::vector<dcmNumericInfo> numericInfo; ///< Info struct for each DICOM image in series
  int nTimes; ///< Number of temporal positions in series
  int nX; ///< number of voxels in X-axis
  int nY; ///< number of voxels in Y-axis
  int nZ; ///< number of voxels in Z-axis
  double Xmm; ///< size of voxels in X-axis in mm
  double Ymm; ///< size of voxels in X-axis in mm
  double Zmm; ///< size of voxels in X-axis in mm
  double FA = 0; ///< Flip-angle 
  double TR = 0; ///< Repetition time in ms
  double TE = 0; ///< echo time in ms
  double TI = 0; ///< inversion time in ms
  double B = 0; ///< B-value
  double gradOri = 0; ///< Gradient orientation
  double acquisitionTime; ///< Acquisition time
  bool sortValid; ///< Cache flag to check the series has been validly sorted
};

//! Class to run the lite version of the DCE analysis tool
/*!
*/
class mdm_RunTools_madym_DicomConvert : public mdm_RunTools {

public:

		
	//! Constructor
	MDM_API mdm_RunTools_madym_DicomConvert();
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_DicomConvert();

	//! parse user inputs specific to DCE analysis
	/*!
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	using mdm_RunTools::parseInputs;
	MDM_API int parseInputs(int argc, const char *argv[]);

	//! Return name of the tool
	/*!
  Must be implemented by the derived classes that will be instantiated into run tools objects.
	\return name of the tool 
  */
  MDM_API std::string who() const;
	
protected:
  //! Runs the lite version of DCE analysis
  /*!
  1. Parses and validates input options
  2. Sets specified tracer-kinetic model
  3. Opens input data file
  4. Processes each line in input data file, fitting tracer-kineti model to input signals/concentrations,
  writing fited parameters and IAUC measurements to output file
  5. Closes input/output file and reports the number of samples processed.
  Throws mdm_exception if errors encountered
  */
  MDM_API void run();

private:
  //Methods:
  void checkRequiredInputs();

  //
  void sortDicomDir();

  //
  void makeSingleVol(
    const std::vector<dcmSeriesInfo> &seriesInfo);

  //
  void makeT1InputVols(
    const std::vector<dcmSeriesInfo> &seriesInfo);

  //
  void makeDynamicVols(
    const std::vector<dcmSeriesInfo> &seriesInfo);

  //!Extract info from dicom file header
  void extractInfo(const std::string &filename,
    std::vector< dcmNumericInfo > &info_numeric,
    std::vector< std::string >  &info_filenames);

  //
  std::vector<std::string> getFileList(boost::filesystem::path directory);

  //
  void completeSeriesInfo(dcmSeriesInfo &series);

  //
  void printSeriesInfo(const dcmSeriesInfo &series);

  //
  void printSeriesInfoSummary(const dcmSeriesInfo &series);

  //
  void writeSeriesInfo(
    const std::vector<dcmSeriesInfo> &seriesInfo);

  //
  void readSeriesInfo();

  //
  mdm_Image3D loadDicomImage(const dcmSeriesInfo &series, const int startIdx,
    bool isDynamic = false, int dynNum = 0);


  //
  void setDicomTag(const mdm_input_strings &tagInput, DcmTagKey &tag);

  //
  void setDicomTag(const mdm_input_string &tagInput, DcmTagKey &tag);

  //
  void checkSortValid(dcmSeriesInfo &series);

  //
  void checkAutoScaling();

  //
  void checkDynamicTime();
  
  //
  void applyAutoScaling(DcmFileFormat &fileformat,
    double &offset, double &scale);

  //
  double getDynamicTime(DcmFileFormat &fileformat, int dynNum);

  std::string makeSequenceFilename(
    const fs::path &filepath, const std::string &prefix,
    const int fileNumber, const std::string &fileNumberFormat);

	//Variables:
  std::vector<dcmSeriesInfo> seriesInfo_;
  
  DcmTagKey dynamicTimeTag_;
  DcmTagKey autoScaleTag_;
  DcmTagKey autoOffsetTag_;

  double temporalResolution_;
};

#endif
