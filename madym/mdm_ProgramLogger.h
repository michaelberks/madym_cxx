 /*!
	*  @file    mdm_ProgramLogger.h
	*  @brief   Class for creating a program and audit log for full DCE and T1 mapping analyses
	*  @details More info...
	*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
	*/

#ifndef MDM_PROGRAMLOGGER_HDR
#define MDM_PROGRAMLOGGER_HDR

#include "mdm_api.h"

#include <string>
#include <fstream>

/*!
	*  @brief   Creates a program and audit log for full model analysis sessions
	*  @details More info...
	*/
class mdm_ProgramLogger {

public:
	//!    Open a program log at given filename, recording the calling program and input options
	/*!
	The program log is saved with analysis output and contains detailed descriptions of key 
	events in the analysis pipeline, any errors, fit failures at specific voxels etc.

	\param    fileName base name of log file
	\param    caller name of calling program
  \return   true on success or false otherwise
	*/
	MDM_API  static bool openProgramLog(const std::string &fileName,
	const std::string &caller);

  //!    Set whether program log messages are also piped to cout
  /*!
  \quiet  if true, messages not displayed in cout
  */
  MDM_API  static void setQuiet(bool quiet);

	//!    Close the program log file stream
	/*!
	\return true on success or false otherwise
	*/
	MDM_API  static bool closeProgramLog();

	//!    Write a message to the program log
	/*!
	\param    message to write to log file
	*/
	MDM_API  static void logProgramMessage(const std::string & message);

  //!    Write an error message to the program log
  /*!
  \param    message to write to log file
  */
  MDM_API  static void logProgramError(const std::string & message);

  //!    Write a warning message to the program log
  /*!
  \param    message to write to log file
  */
  MDM_API  static void logProgramWarning(const std::string & message);

	//!    Open a audit log at given filename, recording the calling program and location of the program log
	/*!
	The audit log is saved at default location for all analysis (configurable with the auditDir input option)
	and records the command called, the resource in which it runs and the location of the program log.

	It does not maintain a detailed list of analysis events, as these are written in the program log.

	\param    fileName base name of log file
	\param    caller name of calling program
	\return   true on success or false otherwise
	*/
	MDM_API  static bool openAuditLog(const std::string &fileName,
		const std::string &caller);

	//!    Close the audit log file stream
	/*!
	\return true on success or false otherwise
	*/
	MDM_API  static bool closeAuditLog();

	//!    Write a message to the audit log
	/*!
	\param    message to write to log file
	\return true on success or false otherwise
	*/
	MDM_API  static bool logAuditMessage(const std::string &message);

private:
	static std::string logTime();
	static std::ofstream program_log_stream_;
	static std::ofstream audit_log_stream_;
  static bool quiet_;
};
#endif /* MDM_PROGRAMLOGGER_HDR */
