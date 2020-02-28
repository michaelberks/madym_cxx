 /**
	*  @file    mdm_ProgramLogger.h
	*  @brief   Class for creating a program and audit log for full model analysis sessions
	*  @details More info...
	*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
	*/

#ifndef MDM_PROGRAMLOGGER_HDR
#define MDM_PROGRAMLOGGER_HDR

#include "mdm_api.h"

#include <string>
#include <fstream>

/**
	*  @brief   Creates a program and audit log for full model analysis sessions
	*  @details More info...
	*/
class mdm_ProgramLogger {

public:
/**
 * @brief    Set file stream pointers, open streams for appending then log caller & time
 * @param    fileName    String base name of log file
 * @param    caller      String name of calling program (may include version)
 * @param    errString   String array to hold error message if required
 * @return   Integer 0 on success or 1 otherwise
 * DbC stuff ...
 * @pre    fileName is a valid pointer to a non-empty file name string
 * @pre    caller is a valid pointer to a non-empty file name string
 * @pre    errString is a valid pointer to a string to hold an error message
 * @post   text and binary logs opened with contents indicating caller and time
 * @post   on error, errString holds the error message, overwriting any previous contents
 */
	MDM_API  static bool openProgramLog(const std::string &fileName,
	const std::string &caller);

/**
 * @brief    Close the program log file stream pointers
 * @param    errString   String array to hold error message if required
 * @return   Integer 0 on success or 1 otherwise
 * DbC stuff ...
 * @pre    errString is a valid pointer to a string to hold an error message
 * @post   log files closed, after writing closing time
 * @post   on error, errString holds the error message, overwriting any previous contents
 */
	MDM_API  static bool closeProgramLog();

/**
 * @brief    Write a message to the program log files (with error report)
 * @param    message     String message to write to log file
 * @param    errString   String array to hold error message if required
 * @return   Integer 0 on success or 1 otherwise
 * DbC stuff ...
 * @pre    message is a valid pointer to a non-empty message string
 * @pre    errString is a valid pointer to a string to hold an error message
 * @post   on error, errString holds the error message, overwriting any previous contents
 * @post   message is written to the program logs
 *
 * Note: Logs must have been opened using bdlOpenProgramLog()
 *       message should have a '\n' at the end, for formatting purposes
 */
	MDM_API  static bool logProgramMessage(const std::string & message);

	/**
 * @brief    Set file stream pointers, open streams for appending then log caller & time
 * @param    fileName    String base name of log file
 * @param    caller      String name of calling program (may include version)
 * @param    errString   String array to hold error message if required
 * @return   Integer 0 on success or 1 otherwise
 * DbC stuff ...
 * @pre    fileName is a valid pointer to a non-empty file name string
 * @pre    caller is a valid pointer to a non-empty file name string
 * @pre    errString is a valid pointer to a string to hold an error message
 * @post   text and binary logs opened with contents indicating caller and time
 * @post   on error, errString holds the error message, overwriting any previous contents
 */
	MDM_API  static bool openAuditLog(const std::string &fileName,
		const std::string &caller);

	/**
	 * @brief    Close the audit log file stream pointers
	 * @param    errString   String array to hold error message if required
	 * @return   Integer 0 on success or 1 otherwise
	 * DbC stuff ...
	 * @pre    errString is a valid pointer to a string to hold an error message
	 * @post   log files closed, after writing closing time
	 * @post   on error, errString holds the error message, overwriting any previous contents
	 */
	MDM_API  static bool closeAuditLog();

	/**
	 * @brief    Write a message to the audit log files (with error report)
	 * @param    message     String message to write to log file
	 * @param    errString   String array to hold error message if required
	 * @return   Integer 0 on success or 1 otherwise
	 * DbC stuff ...
	 * @pre    message is a valid pointer to a non-empty message string
	 * @pre    errString is a valid pointer to a string to hold an error message
	 * @post   on error, errString holds the error message, overwriting any previous contents
	 * @post   message is written to the audit logs
	 *
	 * Note: Logs must have been opened using bdlOpenAuditLog()
	 *       message should have a '\n' at the end, for formatting purposes
	 */
	MDM_API  static bool logAuditMessage(const std::string &message);

private:
	static std::string logTime();
	static std::ofstream program_log_stream_;
	static std::ofstream audit_log_stream_;
};
#endif /* MDM_PROGRAMLOGGER_HDR */

/*
 *  Modifications:
 *  11 April 2013 (GAB)
 *  - Created
 */
