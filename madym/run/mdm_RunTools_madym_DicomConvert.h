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

//Helper struct to hold numeric dicom header info for sorter
struct dcmNumericInfo {
  int seriesNumber;
  int  acquisitionNumber;
  int  temporalPositionIdentifier;
  double  sliceLocation;
  int instanceNumber;
};

struct dcmSeriesInfo {
  std::string name;
  std::string manufacturer;
  int index;
  std::vector<std::string> filenames;
  std::vector<dcmNumericInfo> numericInfo;
  int nTimes;
  int nX;
  int nY;
  int nZ;
  double Xmm;
  double Ymm;
  double Zmm;
  double FA = 0;
  double TR = 0;
  double TE = 0;
  double TI = 0;
  double B = 0;
  double gradOri = 0;
  double acquisitionTime;
  bool sortValid;
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
