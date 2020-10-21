/**
 *  @file    mdm_ProgramLogger.c
 *  @brief   Functions for logging madym messages
 *
 *  Original author GA Buonaccorsi 4 April 2013
 *  (c) Copyright Bioxydyn Ltd, 2012
 *  Not for distribution, internal use only
 *
 *  Documentation comments are in *.h, except for static methods
 *  Version info and list of modifications in comment at end of file
 *
 *  NOTE:  Unix-specific because of functions in unistd.h
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif

#include "mdm_ProgramLogger.h"

#include <cassert>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <string>
#include <iostream>
#include <cstdlib>     //getenv()
#include <mdm_version.h>
#include <mdm_platform_defs.h>

#include <boost/filesystem.hpp>
#include <boost/asio/ip/host_name.hpp>


std::ofstream mdm_ProgramLogger::program_log_stream_;
std::ofstream mdm_ProgramLogger::audit_log_stream_;

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

	return logProgramMessage(msg);

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

/**
 * @author   GA Buonaccorsi
 * @version  4 April 2013
 */
MDM_API bool mdm_ProgramLogger::logProgramMessage(const std::string &message)
{
	if (!program_log_stream_)
	{
		std::cout << message << std::endl;
		return false;
	}

	/* First write to ASCII log */
	program_log_stream_ << message;
	return true;
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

/**/
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
/**
 */
std::string mdm_ProgramLogger::logTime()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	return ss.str();
}
/*
 */
