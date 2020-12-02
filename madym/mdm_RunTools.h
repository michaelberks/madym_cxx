/*!
*  @file    mdm_RunTools.h
*  @brief   Defines abstract class mdm_RunTools providing methods common to all analysis pipelines
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_HDR
#define MDM_RUNTOOLS_HDR

#include "mdm_api.h"
#include <mdm_OptionsParser.h>
#include <madym/mdm_FileManager.h>
#include <madym/mdm_VolumeAnalysis.h>
#include <madym/mdm_AIF.h>
#include <madym/mdm_T1Mapper.h>
#include <madym/mdm_ErrorTracker.h>

#include <madym/dce_models/mdm_DCEModelBase.h>

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
//!   Defines abstract class mdm_RunTools providing methods common to all analysis pipelines
/*!
	All tools that run final analysis pipelines should implemnent a run tools class
	that derives from this base class. This is a pure abstract class, and any subclass must implement:

	1. parseInputs
	
	2. run
	
	When tools are executed, their main function should instante the appropriate run tools object,
	call parseInputs to set any input options from the user, and then call run to do the actual
	analysis.

	The run tools object can make use of some of the generic methods provided here (eg. creating an
	output directory). They can optionally derive from the additional run tools sub-classes, that
	provide more specific methods for eg DCE fitting or T1 mapping

	\see mdm_RunToolsT1Fit
	\see mdm_RunToolsDCEFit
	\see mdm_RunToolsVolumeAnalysis
*/
class mdm_RunTools {

public:
		
	//! Constructor
	/*!
	\param options structure holding global set of input options
	\param options_parser parse command line/config file/GUI inputs and saves into the input options
	*/
	MDM_API mdm_RunTools();
		
	//! Virtual destructor
	MDM_API virtual ~mdm_RunTools();
  	
	//! Run the analysis pipeline, with catch block to tidy up log file. 
	/*!
  Calls pure virtual function run, which must be
	implemented by the derived classes that will be instantiated into run tools objects.
	*/
	MDM_API int run_catch();

	/*! parseInputs overload for when there isn't have a command line to parse
	//!
	parseInputs should always be called before run, to ensure the mdm_InputOptions object
	is mapped into the mdm_OptionsParser variable map, allowing eg a config file for the 
	session to be written. However when run is called from the GUI (or test script), there is
	no main exe set of args to parse. This overload instead takes a std::string giving the
	calling program (eg madym_GUI_run_DCE), and creates dummy int argc and char *argv[]
	variables to use the main parseInputs(int argc, const char *argv[]) method.
	\param argv string giving the calling program
	*/
	MDM_API int parseInputs(const std::string &argv);

  /*! save current options in configuration file
  //!
  parseInputs should always be called this, to ensure the mdm_InputOptions object
  is mapped into the mdm_OptionsParser variable map.
  \param filepath of saved config file
  */
  MDM_API void saveConfigFile(const std::string &filepath) const;

  //! Reference to options so it can be configured by GUI
  /*!
  \return reference to options
  */
  MDM_API mdm_InputOptions &options();

	//! parse command and/or config file arguments.
	/*!
	parseInputs should always be called before run, to ensure the mdm_InputOptions object
	is mapped into the mdm_OptionsParser variable map, allowing eg a config file for the
	session to be written.

	All derived classes that will be instantiated into run tools objects, must implement this method.
	Typically this will consist of adding the set of mdm_InputOptions options required by the tool
	to its mdm_OptionsParser, and then calling the parser's own parseInputs method.
	
	\param argc count of command line arguments from main exe
	\param argv list of arguments from main exe
	\return 0 on success, non-zero if error or help/version options specified
	\see mdm_OptionsParser#parseInputs
	*/
	MDM_API virtual int parseInputs(int argc, const char *argv[]) = 0;

	//! Return name of the tool
	/*!
  Must be implemented by the derived classes that will be instantiated into run tools objects.
	\return name of the tool 
  */
  MDM_API virtual std::string who()  const = 0;
	
protected:
  //! Run the analysis pipeline 
  /*!
  Must be implemented by the derived classes that will be instantiated into run tools objects.
  */
  MDM_API virtual void run() = 0;

	//! Return the current time, in standardised string format
	/*!
	\return current time, in standardised string format
	*/
	std::string timeNow();

  //! Changes current working directory if dataDir specified in inputs 
  /*!
  */
  void set_up_cwd();
	
	//! Sets up output folder for program analysis
	/*!
	1. Converts user specified output folder to absolute path if a relative path has been given.
	2. Checks to see if output folder exists.
		- If it doesn't, creates new folder.
		- If it does, checks if it is empty.
		- If existing folder is non-empty and mdm_InputOptions#overwrite is false, returns error.
	*/
	void set_up_output_folder();

	//! Sets up program logs
	/*!
	Opens program log in analysis output folder and audit log in folder specified by mdm_InputOptions#auditDir,
	appending the calling program name and the current time to create unique filenames for each.
	*/
	void set_up_logging();

	//Variables:
	//! Structure containing analysis options. 
	/*! Initialised with default values on instantiation, then configured by user input from the command-line,
	config file or GUI (or can be set directly programatically).
	*/
	mdm_InputOptions options_;

	//! Parse command-line/config file options, saving the updates into options_
	mdm_OptionsParser options_parser_;

	//! Stores analysis output path
	fs::path outputPath_;

private:
  //Methods:

  //! Helper function to call on successful analysis completion.
  /*!
  Closes any open program/audit logs, and returns 0 for successful program completion
  to the original calling function.
  */
  void mdm_progExit();

  //! Helper function to call when a fatal error occurs
  /*!
  Logs the specified error message, closes the program/audit logs, and returns 1 for
  program completion with errors to the original calling function.
  */
  void mdm_progAbort(const std::string &err_str);
	
};

#endif /* mdm_RunTools_HDR */
