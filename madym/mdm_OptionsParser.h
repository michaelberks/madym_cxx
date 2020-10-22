/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_OPTIONS_PARSER_HDR
#define MDM_OPTIONS_PARSER_HDR
#include "mdm_api.h"
#include <mdm_version.h>

#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include <mdm_InputOptions.h>

namespace po = boost::program_options;

/**
*  @brief   Defines options and their default values for all inputs to DCE analysis and T1 mapping tools
*/
class mdm_OptionsParser {
public:
  //:All madym  options
  //--------------------------------------------------------------------
	
	const static std::string EMPTY_STR;

	MDM_API mdm_OptionsParser();
	MDM_API bool to_stream(std::ostream &stream, 
		const mdm_InputOptions &options) const;
	MDM_API bool to_file(const std::string &filename, 
		const mdm_InputOptions &options) const;

	MDM_API int madym_inputs(int argc, const char *argv[],
		mdm_InputOptions &options);
	MDM_API int madym_lite_inputs(int argc, const char *argv[],
		mdm_InputOptions &options);
	MDM_API int calculate_T1_inputs(int argc, const char *argv[],
		mdm_InputOptions &options);
	MDM_API int calculate_T1_lite_inputs(int argc, const char *argv[],
		mdm_InputOptions &options);

	//Overrides for non-commandline input
	MDM_API int madym_inputs(const std::string &argv,
		mdm_InputOptions &options);
	MDM_API int madym_lite_inputs(const std::string &argv,
		mdm_InputOptions &options);
	MDM_API int calculate_T1_inputs(const std::string &argv,
		mdm_InputOptions &options);
	MDM_API int calculate_T1_lite_inputs(const std::string &argv,
		mdm_InputOptions &options);

	MDM_API const std::string& exe_args() const;

	MDM_API const std::string& exe_cmd() const;

private:

	bool parse_command_line(int argc, const char *argv[],
		const po::options_description &combined_options);

	bool help_set(int argc, const po::options_description &combined_options);
	bool version_set();

	bool parse_config_file(const po::options_description &config_options,
		const mdm_InputOptions &options);

	void make_exe_args(int argc, const char *argv[]);
	
	po::variables_map vm_;
	po::options_description help_;
	std::string exe_args_;
	std::string exe_cmd_;

};

#endif /* mdm_RunTools_HDR */
