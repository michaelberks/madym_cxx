/*!
*  @file    mdm_OptionsParser.h
*  @brief   Class for passing input options from the command line or config files
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_OPTIONS_PARSER_HDR
#define MDM_OPTIONS_PARSER_HDR
#include "mdm_api.h"

#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include <mdm_InputOptions.h>

namespace po = boost::program_options;

//! Class for passing input options from the command line or config files
/*!
  # Option parsing in Madym
	DCE analysis requires a lot of configurable options to run. Different tools require various overlapping 
	subsets of these options. We have designed an API for configuring these options with the following 
	objectives:

	1. Tools can specify which subset of options they require, eg the lite version of T1 mapper only requires 
	9 options, while the full DCE tracer-kinetic analysis requires more than 30.

	2. Options should be selected from a single global pool. This ensures where an option is used in multiple 
	tools (eg the T1 noise threshold) it has the same default value, option key and information text in all 
	tools. This ensures consistency in default behaviour, eg in T1 mapping in the standalone tool or as part
	of full DCE analysis, or between the full and lite versions of the tools.

	3. The Madym GUI permits the same set of options and runs the same analysis pipeline as the commandline tools

	To achieve this we use 3 classes:  mdm_InputOptions, mdm_OptionsParser and mdm_RunTools (and its sub-classes).

	1. mdm_InputOptions maintains the global pool of options, defining their default values, keys and information
	text

	2. Each tool then sub-classes mdm_RunTools, implementing the parseInputs method to register the specific 
	subset of options it requires to run

	3. mdm_OptionsParser parses the command-line and/or config file for the specific set of options passed to it 
	by the mdm_RunTools object. The parsed options automatically update the values in the global mdm_InputOptions 
	object, which are then available for the mdm_RunTools object to use.

	The Madym GUI works by linking its input widget callbacks directly to a global mdm_InputOptions object. 
	When the user selects to run an analysis, the appropriate mdm_RunTools object is created, ensuring the 
	default option values and analysis pipeline used are the same as the command-line tools.

	\see mdm_InputOptions
	\see mdm_RunTools
*/
class mdm_OptionsParser {
public:

  //! Enum type for return code after parsing inputs
	enum ParseType {
		OK = 0, //!< Inputs parsed OK, continue with run
		VERSION = 1, //!< Version set, exit calling program 0
		HELP = 2, //!< Help set, exit calling program 0
		CMD_ERROR = 3, //!< Error parsing command line
		CONFIG_ERROR = 3, //!< Error parsing config file
	};

	//! Default constructor
	MDM_API mdm_OptionsParser();

	//! Write option keys and values to an output stream
	/*!
	The input options object will contain *all* madym options, however only
	those in the options parser variables map will be output to the stream. This
	allows writing output config files for specific tools (eg a T1 mapping config
	file doesn't need all the additional options for a DCE analysis).

	\param stream output stream to write to
	\param options set of madym options
	\param caller name of the calling tool
	\return true if write successful, false if any errors
	*/
	MDM_API bool to_stream(std::ostream &stream, 
		const mdm_InputOptions &options, const std::string &caller) const;

	//! Write option keys and values to file
	/*!
	\param filename
	\param options set of madym options
	\param caller name of the calling tool
	\return true if write successful, false if any errors (eg unable to open file)
	\see to_stream
	*/
	MDM_API bool to_file(const std::string &filename, 
		const mdm_InputOptions &options, const std::string &caller) const;
	
	//! Parse inputs from command *and* config file
	/*!
	\param cmdline_options collection of options that can *only* be read from the command line 
	(eg the config file flag)
	\param config_options collection of options that can be read from *both* the command line and a config file
	\param configFile name of config file to read (empty if no config to check). This 
	is usually set with a reference to the configFile mdm_Input option, which may be 
	empty when the method is called (the default option), but may then be set by the 
	command-line parser if the user includesthe config option in their command line call.
	\param configType checks correct config file is parsed, input file must have \c \#configType as first line
	\param argc count of command line arguments, from main executable
	\param argv array of command line arguments, from main executable
  \return succes code enum
		- OK success, continue execution
		- CMD_ERROR problem reading commandline, exit
		- HELP help flag set, print options description and exit
		- VERSION version flag set, print version and exit
		- CONFIG_ERROR problem reading config file, exit
	*/
	MDM_API ParseType parseInputs(
		po::options_description &cmdline_options,
		po::options_description &config_options,
		const std::string &configFile,
		const std::string &configType,
		int argc, const char *argv[]);

	//! Parse inputs from command line *only*
	/*!
	\param cmdline_options collection of options that can be read from the command line
	\param argc count of command line arguments, from main executable
	\param argv array of command line arguments, from main executable
	\return int succes code
		- OK success, continue execution
		- CMD_ERROR problem reading commandline, exit
		- HELP help flag set, print options description and exit
		- VERSION version flag set, print version and exit
	*/
	MDM_API ParseType parseInputs(
		po::options_description &cmdline_options,
		int argc, const char *argv[]);

	//! Return string descriptor of command line arguments
	/*!
	\return the contents of argv from a main executable, concatenated into a single string
	*/
	MDM_API const std::string& exe_args() const;

	//! Return the executable command
	/*
	\return the executable name, eg argv[0] converted to a std::string
	*/
	MDM_API const std::string& exe_cmd() const;

	//! Add a madym input option to a program options descriptor
	/*!
	This is templated for the different input types defined in mdm_Input. Defining this method
	here allows us to include the boost::program_options::validate method overrides for 
	parsing custom option types in a single place.
	\param config_options options descriptor
	\param option madym input option
	\see mdm_Input
	*/
	template<class T, class T_out>
	MDM_API void add_option(po::options_description &config_options, mdm_Input<T, T_out> &option);

private:

	bool parse_command_line(int argc, const char *argv[],
		const po::options_description &combined_options);

	bool help_set(int argc, const po::options_description &combined_options);
	bool version_set();

	bool parse_config_file(const po::options_description &config_options,
		const std::string &configFile, const std::string &configType);

	bool check_config_type(std::ifstream &ifs, const std::string &configType);

	void make_exe_args(int argc, const char *argv[]);
	
	po::variables_map vm_;
	po::options_description help_;
	std::string exe_args_;
	std::string exe_cmd_;

};

#endif /* mdm_RunTools_HDR */
