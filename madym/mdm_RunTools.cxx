/**
*  @file    mdm_RunTools.cxx
*  @brief   Implementation of mdm_RunTools class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools.h"

#include <iostream>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time

#include <mdm_version.h>
#include <madym/mdm_exception.h>
#include <madym/mdm_platform_defs.h>

#include <madym/mdm_ProgramLogger.h>

namespace fs = boost::filesystem;


//
MDM_API mdm_RunTools::mdm_RunTools()
{
}

//
MDM_API mdm_RunTools::~mdm_RunTools()
{

}

//
MDM_API int mdm_RunTools::parseInputs(const std::string &argv)
{
	const char*argvc[] = { argv.c_str() };
	return parseInputs(0, argvc);
}

//
MDM_API void mdm_RunTools::saveConfigFile(const std::string &filepath) const
{
  options_parser_.to_file(filepath, options_, who());
}

//
MDM_API mdm_InputOptions & mdm_RunTools::options()
{
  return options_;
}

//
MDM_API int mdm_RunTools::run_catch()
{
  try {
    run();
  }
  catch (mdm_exception &e)
  {
    mdm_ProgramLogger::logProgramError(__func__, e.what());
    mdm_progAbort("mdm_Exception caught");
    return 1;
  }
  catch (std::exception &e)
  {
    mdm_ProgramLogger::logProgramError(__func__, e.what());
    mdm_progAbort("std::exception caught");
    return 1;
  }
  catch (...)
  {
    mdm_progAbort("Unhandled exception caught, aborting");
    return 1;
  }

  //Tidy up the logging objects
  mdm_progExit();
  return 0;
}

//

//-----------------------------------------------------------
//-----------------------------------------------------------
// Private methods:
//-----------------------------------------------------------
void mdm_RunTools::mdm_progExit()
{
	std::string success_msg = options_parser_.exe_cmd() + " completed successfully.\n";
	mdm_ProgramLogger::logProgramMessage(success_msg);
	mdm_ProgramLogger::logAuditMessage(success_msg);
	mdm_ProgramLogger::closeAuditLog();
	mdm_ProgramLogger::closeProgramLog();
}

void mdm_RunTools::mdm_progAbort(const std::string &err_str)
{

  std::string error_msg = options_parser_.exe_cmd() + " ABORTING: " + err_str + "\n";
  
  mdm_ProgramLogger::logProgramMessage(error_msg);
  mdm_ProgramLogger::logAuditMessage(error_msg);
  mdm_ProgramLogger::closeAuditLog();
  mdm_ProgramLogger::closeProgramLog();

	std::cerr << error_msg << std::endl;
}

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED
std::string mdm_RunTools::timeNow()
{
	
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "_%Y%m%d_%H%M%S_");
	return ss.str();
}
DISABLE_WARNING_POP

//
void mdm_RunTools::set_up_cwd()
{
  //Make the audit log absolute before we change directory
  options_.auditLogDir.set(fs::absolute(options_.auditLogDir()).string());

  //If dataDir is set in the options, change the current path to this
  if (!options_.dataDir().empty())
    fs::current_path(fs::absolute(options_.dataDir()));
}

//
void mdm_RunTools::set_up_output_folder()
{
	//Using boost filesyetm, can call one line to make absolute path from input
	//regardless of whether relative or absolute path has been given
	outputPath_ = fs::absolute(options_.outputRoot()+options_.outputDir());

	//We probably don't need to check if directory exists, just call create... regardless
	//but we do so anyway
	if (!fs::is_directory(outputPath_))
		fs::create_directories(outputPath_);

	/* If we've got this file already warn user of previous analysis and quit */
  if (!options_.overwrite() && !fs::is_empty(outputPath_))
    throw mdm_exception(__func__, 
      "Output directory is not empty (use option -O to overwrite existing data)");
}

/**/

void mdm_RunTools::set_up_logging()
{
	//Set up paths to error image and audit logs, using default names if not user supplied
	// and using boost::filesystem to make absolute paths

  const std::string exe_cmd = fs::path(options_parser_.exe_cmd()).stem().string();
  std::string caller = options_parser_.exe_cmd() + " " + MDM_VERSION;

  if (!options_.noAudit())
  {
    //Note the default audit path doesn't use the output directory (unless user specifically set so)
    std::string auditName = exe_cmd + timeNow() + options_.auditLogBaseName();
    fs::path auditDir = fs::absolute(options_.auditLogDir());
    if (!is_directory(auditDir))
      create_directories(auditDir);
    fs::path auditPath = auditDir / auditName;

    mdm_ProgramLogger::openAuditLog(auditPath.string(), caller);
    mdm_ProgramLogger::logAuditMessage("Command args: " + options_parser_.exe_args());
  }

  mdm_ProgramLogger::setQuiet(options_.quiet());
  if (!options_.noLog())
  {
    std::string programName = exe_cmd + timeNow() + options_.programLogName();
    fs::path programLogPath = outputPath_ / programName;
    mdm_ProgramLogger::openProgramLog(programLogPath.string(), caller);

    //Log location of program log and config file in audit log
    if (!options_.noAudit())
      mdm_ProgramLogger::logAuditMessage(
        "Program log saved to " + programLogPath.string());
  }

  //Log the command arguments
	mdm_ProgramLogger::logProgramMessage("Command args: " + options_parser_.exe_args());
	
	//Save the config file of input options
  std::string configName = exe_cmd + timeNow() + options_.outputConfigFileName();
  fs::path configFilePath = outputPath_ / configName;
  saveConfigFile(configFilePath.string());
	mdm_ProgramLogger::logProgramMessage(
		"Config file saved to " + configFilePath.string());
}