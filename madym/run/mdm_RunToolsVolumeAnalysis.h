/*!
*  @file    mdm_RunToolsVolumeAnalysis.h
*  @brief    Defines abstract class mdm_RunToolsVolumeAnalysis providing methods for analysis pipelines that operate on whole image volumes (as oppposed to input data files used in the lite tools)

*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_VOLUMEANALYSIS_HDR
#define MDM_RUNTOOLS_VOLUMEANALYSIS_HDR
#include "mdm_api.h"
#include <madym/run/mdm_RunToolsT1Fit.h>

namespace fs = boost::filesystem;

//! Abstract base class providing methods common to tools that operate on whole image volumes
/*!
 \see mdm_RunTools
*/
class mdm_RunToolsVolumeAnalysis : public virtual mdm_RunToolsT1Fit {

public:

		
	//! Constructor
	/*!
	*/
	MDM_API mdm_RunToolsVolumeAnalysis(mdm_InputOptions &options, mdm_OptionsParser &options_parser);
		
	//! Virtual destructor
	/*!
	*/
	MDM_API virtual ~mdm_RunToolsVolumeAnalysis();

protected:
	//Methods:

	//! Load error map
	/*!
	1. Create path to error map from user options
	2. Check if existing error map exists if so, load it
	3. If error map doesn't exist, create a new empty one
	4. Save the error map path for future use
	*/
	MDM_API void loadErrorMap();

	//! Check if user has specified an ROI mask, and if so, load it
	MDM_API void loadROI();

  //! Load set of dynamic signal images
  MDM_API void loadSt();

  //! Load set of dynamic contrast agent concentration maps
  MDM_API void loadCt();

  //! Load precomputed T1 map
  MDM_API void loadT1();

	//! Check if there are T1 signal inputs to load, and if so, load them
	MDM_API void loadT1Inputs();

  //
  MDM_API void mapT1();

	//! Write all output maps
	MDM_API void writeOutput();

	//Variables:

	//! Object for managing all image volume IO
	mdm_FileManager fileManager_;

	//! Analysis object that performs specified analyses on all volume voxels
	mdm_DCEVolumeAnalysis volumeAnalysis_;

private:
	fs::path errorMapPath_;
};

#endif
