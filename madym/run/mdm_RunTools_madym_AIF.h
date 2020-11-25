/*!
*  @file    mdm_RunTools_madym_AIF.h
*  @brief   Defines class mdm_RunTools_madym_AIF to run the lite version of the DCE analysis tool
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_MADYM_AIF_HDR
#define MDM_RUNTOOLS_MADYM_AIF_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsVolumeAnalysis.h>
#include <madym/run/mdm_RunToolsT1Fit.h>

#include <madym/mdm_AIF.h>

//! Class to run the lite version of the DCE analysis tool
/*!
*/
class mdm_RunTools_madym_AIF : public mdm_RunToolsVolumeAnalysis {

public:

		
	//! Constructor
	/*!
	\param options set of analysis options
	\param options_parser_ object for parsing input options
	\return
	*/
	MDM_API mdm_RunTools_madym_AIF(mdm_InputOptions &options, mdm_OptionsParser &options_parser_);
		
	//! Default destructor
	/*!
	*/
	MDM_API ~mdm_RunTools_madym_AIF();

	//! parse user inputs specific to DCE analysis
	/*!
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	using mdm_RunTools::parseInputs;
	MDM_API int parseInputs(int argc, const char *argv[]);

protected:
  //! Runs the AIF auto fit tool
  /*!
  1. Parses and validates input options
  2. Sets specified tracer-kinetic model
  3. Opens input data file
  4. Processes each line in input data file, fitting tracer-kineti model to input signals/concentrations,
  writing fited parameters and IAUC measurements to output file
  5. Closes input/output file and reports the number of samples processed.
  \return 0 on success, non-zero otherwise
  */
  MDM_API int run();

private:
  //Methods:
  void checkRequiredInputs();

  void setFileManagerParams();

  void setVolumeAnalysisParams();

  //! Compute AIF automatically from sequence of dynamic images
  void computeAutoAIF();

  //Helper functions for computing auto AIF

  //
  void computeAutoAIFSlice(
    const int slice,
    mdm_Image3D &AIFmap,
    const std::vector<int> &xRange,
    const std::vector<int> &yRange,
    std::vector<int> &candidateVoxels,
    std::vector<double> &candidateMaxSignals);

  // 
  void processSlices(
    const int slice,
    mdm_Image3D &AIFmap,
    std::vector<int> &candidateVoxels,
    std::vector<double> &candidateMaxSignals);

  // 
  void getSliceCandidateVoxels(
    const int slice,
    const std::vector<int> &xRange,
    const std::vector<int> &yRange,
    mdm_Image3D &AIFmap,
    mdm_Image3D &AIFmapSlice,
    std::vector<int> &candidateVoxels,
    std::vector<double> &candidateMaxSignals);

  //
  void selectVoxelsFromCandidates(
    mdm_Image3D &AIFmap,
    const std::vector<int> &candidateVoxels,
    const std::vector<double> &candidateMaxSignals);

  //
  void saveAIF(const std::string &sliceName);

  //
  bool validCandidate(
    const std::vector<mdm_Image3D> &dynImages,
    mdm_Image3D &AIFmap,
    mdm_Image3D &AIFmapSlice,
    int voxelIndex, double &maxSig);

  //
  void getMinMaxSignal(const std::vector<double> &signalData,
    double &minSignal, double&maxSignal, int &maxImg);

  //
  double prebolusNoiseThresh(const std::vector<double> &signalData,
    const int arrivalImg);

	//Variables:
  mdm_AIF AIF_;
};

#endif
