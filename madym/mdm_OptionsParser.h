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

	MDM_API mdm_OptionsParser();
	MDM_API bool to_stream(std::ostream &stream, 
		const mdm_InputOptions &options) const;
	MDM_API bool to_file(const std::string &filename, 
		const mdm_InputOptions &options) const;
	
	MDM_API int parse_inputs(
		po::options_description &cmdline_options,
		po::options_description &config_options,
		const std::string &configFile,
		int argc, const char *argv[]);

	MDM_API int parse_inputs(
		po::options_description &config_options,
		int argc, const char *argv[]);


	MDM_API const std::string& exe_args() const;

	MDM_API const std::string& exe_cmd() const;

	template<class T, class T_out>
	MDM_API void add_option(po::options_description &config_options, mdm_input<T, T_out> &option);

private:

	bool parse_command_line(int argc, const char *argv[],
		const po::options_description &combined_options);

	bool help_set(int argc, const po::options_description &combined_options);
	bool version_set();

	bool parse_config_file(const po::options_description &config_options,
		const std::string &configFile);

	void make_exe_args(int argc, const char *argv[]);
	
	po::variables_map vm_;
	po::options_description help_;
	std::string exe_args_;
	std::string exe_cmd_;

};

#endif /* mdm_RunTools_HDR */
