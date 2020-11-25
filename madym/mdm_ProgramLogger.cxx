/**
*  @file    mdm_ProgramLogger.cxx
*  @brief   implements the mdm_ProgramLogger class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_ProgramLogger.h"

#include <cassert>
#include <sstream> // stringstream
#include <string>
#include <iostream>
#include <cstdlib>     //getenv()
#include <mdm_version.h>
#include <mdm_platform_defs.h>

#include <boost/filesystem.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time.hpp>


std::ofstream mdm_ProgramLogger::program_log_stream_;
std::ofstream mdm_ProgramLogger::audit_log_stream_;
bool mdm_ProgramLogger::quiet_ = false;

//
MDM_API bool mdm_ProgramLogger::openProgramLog(const std::string &fileName,
	const std::string &caller)
{
	assert(!fileName.empty());

	program_log_stream_.open(fileName);
	if (!program_log_stream_)
	{
		std::cerr << "Failed to open program log " << fileName << std::endl;
		return false;
	}

	std::string msg = "Log opened at " + logTime() + "\n";
	
	std::string user = "";
	char *userc = getenv(PLATFORM_USER);
	if (userc != NULL)
		user = std::string(userc);

	boost::filesystem::path cwd = boost::filesystem::current_path();
	std::string hostName = boost::asio::ip::host_name();

	msg += "User: " + user + ";   Host: " + hostName + "\n";
	msg += "Ran in: " + cwd.string() + "\n";
  logProgramMessage(msg);
	
  return true;

}

//!    Set whether program log messages are also piped to cout
  /*!
  \quiet  if true, messages not displayed in cout
  */
MDM_API  void mdm_ProgramLogger::setQuiet(bool quiet)
{
  quiet_ = quiet;
}

//
MDM_API bool mdm_ProgramLogger::closeProgramLog()
{
	if (!program_log_stream_)
	{
		return false;
	}

	std::string msg = "Log closed at " + logTime() + "\n";
	logProgramMessage(msg);

	//Try and close the stream
	program_log_stream_.close();
	if (program_log_stream_.is_open())
	{
		std::cerr << "Program log not closed" << std::endl;
		return false;
	}
	
	return true;
}

//
MDM_API void mdm_ProgramLogger::logProgramMessage(const std::string &message)
{
	if (!quiet_)
		std::cout << message << std::endl;
	
  if (program_log_stream_)
	  program_log_stream_ << message;

}

//
MDM_API  void mdm_ProgramLogger::logProgramError(const std::string & message)
{
  std::cerr << "ERROR: " << message << std::endl;

  if (program_log_stream_)
    program_log_stream_ << "ERROR: " << message;
}

//
MDM_API  void mdm_ProgramLogger::logProgramWarning(const std::string & message)
{
  std::cerr << "WARNING: " << message << std::endl;

  if (program_log_stream_)
    program_log_stream_ << "WARNING: " << message;
}

MDM_API bool  mdm_ProgramLogger::openAuditLog(const std::string &fileName,
	const std::string &caller)
{
	assert(!fileName.empty());

	audit_log_stream_.open(fileName);
	if (!audit_log_stream_)
	{
		std::cerr << "Failed to open audit log " << fileName << std::endl;
		return false;
	}
  std::cout << "Opened audit log at " << fileName << std::endl;

	std::string msg = "Log opened at " + logTime() + "\n";

	std::string user = "";
	char *userc = getenv(PLATFORM_USER);
	if (userc != NULL)
		user = std::string(userc);

	boost::filesystem::path cwd = boost::filesystem::current_path();
	std::string hostName = boost::asio::ip::host_name();

	msg += "User: " + user + ";   Host: " + hostName + "\n";
	msg += "Ran in: " + cwd.string() + "\n";


	return logAuditMessage(msg);
}

//
MDM_API bool mdm_ProgramLogger::closeAuditLog()
{
	if (!audit_log_stream_)
	{
		return false;
	}

	std::string msg = "Log closed at " + logTime() + "\n";
	logAuditMessage(msg);

	//Try and close the stream
	audit_log_stream_.close();
	if (audit_log_stream_.is_open())
	{
		std::cerr << "Audit log not closed" << std::endl;
		return false;
	}

	return true;
}

//
MDM_API bool mdm_ProgramLogger::logAuditMessage(const std::string &message)
{
	assert(!message.empty());

	if (!audit_log_stream_)
	{
		std::cerr << "logProgramMessage: audit log not open." << std::endl;
		return false;
	}

	/* First write to ASCII log */
	audit_log_stream_ << message;
	return true;
}

//****************************************************************************
// Private
//****************************************************************************
//
std::string mdm_ProgramLogger::logTime()
{
	boost::posix_time::ptime timeLocal =
		boost::posix_time::second_clock::local_time();

	return boost::posix_time::to_simple_string(timeLocal);
}
