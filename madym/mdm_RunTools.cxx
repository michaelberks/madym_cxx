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

#include <madym/mdm_ProgramLogger.h>

namespace fs = boost::filesystem;


//
MDM_API mdm_RunTools::mdm_RunTools(mdm_InputOptions &options, mdm_OptionsParser &options_parser)
  :
  options_(options),
	options_parser_(options_parser)
{
	//Make the audit log absolute before we change directory
	options_.auditLogDir.set( fs::absolute(options_.auditLogDir()).string() );

	//If dataDir is set in the options, change the current path to this
	if (!options_.dataDir().empty())
		fs::current_path(fs::absolute(options_.dataDir()));
}


MDM_API mdm_RunTools::~mdm_RunTools()
{

}


//

//-----------------------------------------------------------
//-----------------------------------------------------------
// Private methods:
//-----------------------------------------------------------
int mdm_RunTools::mdm_progExit()
{
	std::string success_msg = options_parser_.exe_cmd() + " completed successfully.\n";
	mdm_ProgramLogger::logProgramMessage(success_msg);
	mdm_ProgramLogger::logAuditMessage(success_msg);
	mdm_ProgramLogger::closeAuditLog();
	mdm_ProgramLogger::closeProgramLog();
	return 0;
}

void mdm_RunTools::mdm_progAbort(const std::string &err_str)
{

  std::string error_msg = options_parser_.exe_cmd() + " ABORTING: " + err_str + "\n";

	mdm_ProgramLogger::logProgramMessage(error_msg);
	mdm_ProgramLogger::logAuditMessage(error_msg);
	mdm_ProgramLogger::closeAuditLog();
	mdm_ProgramLogger::closeProgramLog();
  exit(1);
}

std::string mdm_RunTools::timeNow()
{
	
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "_%Y%m%d_%H%M%S_");
	return ss.str();
}

/**/

void mdm_RunTools::set_up_logging(fs::path outputPath)
{
	//Set up paths to error image and audit logs, using default names if not user supplied
	// and using boost::filesystem to make absolute paths
	const std::string exe_cmd = fs::path(options_parser_.exe_cmd()).stem().string();
	std::string auditName = exe_cmd + timeNow() + options_.auditLogBaseName();
	std::string programName = exe_cmd + timeNow() + options_.programLogName();
	std::string configName = exe_cmd + timeNow() + options_.outputConfigFileName();

	fs::path programLogPath = outputPath / programName;
	fs::path configFilePath = outputPath / configName;

	//Note the default audit path doesn't use the output directory (unless user specifically set so)
	fs::path auditDir = fs::absolute(options_.auditLogDir());
	if (!is_directory(auditDir))
		create_directories(auditDir);
	fs::path auditPath = auditDir / auditName;

	std::string caller = options_parser_.exe_cmd() + " " + MDM_VERSION;
	mdm_ProgramLogger::openAuditLog(auditPath.string(), caller);
	mdm_ProgramLogger::logAuditMessage("Command args: " + options_parser_.exe_args());
	mdm_ProgramLogger::openProgramLog(programLogPath.string(), caller);
	mdm_ProgramLogger::logProgramMessage("Command args: " + options_parser_.exe_args());
	std::cout << "Opened audit log at " << auditPath.string() << std::endl;

	//Save the config file of input options
	options_parser_.to_file(configFilePath.string(), options_);

	//Log location of program log and config file in audit log
	mdm_ProgramLogger::logAuditMessage(
		"Program log saved to " + programLogPath.string() + "\n");
	mdm_ProgramLogger::logAuditMessage(
		"Config file saved to " + configFilePath.string() + "\n");
}