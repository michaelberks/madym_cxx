/**
*  @file    mdm_OptionsParser.cxx
*  @brief   Implementation of mdm_OptionsParser class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_OptionsParser.h"

#include <iostream>
#include <fstream>

#include <mdm_version.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <madym/utils/mdm_exception.h>
#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_platform_defs.h>

//! Custom validator for input type mdm_input_str
void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_str* target_type, int)
{

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);

	// Do regex match and convert the interesting part to 
	// int.
	mdm_input_str str(s);
	if (s == mdm_input_str::EMPTY_STR)
		v = mdm_input_str("");
		
	else
		v = str;
}

//! Custom validator for input type mdm_input_string_list
void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_string_list* target_type, int)
{

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);
	v = mdm_input_string_list(s);
}

//! Custom validator for input type mdm_input_double_list
void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_double_list* target_type, int)
{
  //Format:
  // optional enclosing [ ]
  // comma separated

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);
  v = mdm_input_double_list(s);
}

//! Custom validator for input type mdm_input_int_list
void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_int_list* target_type, int)
{
  //Format:
  // optional enclosing [ ]
  // comma separated
  // - indicates range between start and end items

  //eg: [1-3, 5] should generate {1, 2, 3, 5}

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);
	v = mdm_input_int_list(s);
}

//! Custom validator for input type mdm_input_dicomTag
void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_dicomTag* target_type, int)
{

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);
	v = mdm_input_dicomTag(s);
}

//
MDM_API  mdm_OptionsParser::mdm_OptionsParser()
{
}

//
MDM_API bool mdm_OptionsParser::to_file(const std::string &filepath, 
	const std::string& cmdFilepath,
	const mdm_InputOptions &options, const std::string &caller) const
{
	std::ofstream filestream(filepath, std::ios::out);
	if (!filestream.is_open())
		return false;
	all_to_stream(filestream, options, caller);

	filestream.close();

	if (!cmdFilepath.empty())
	{
		std::ofstream cmdFilestream(cmdFilepath, std::ios::out);
		if (!cmdFilestream.is_open())
			return false;

		if (options.guiSetOptions.empty())
			cmd_to_stream(cmdFilestream, options);
		else
			gui_to_stream(cmdFilestream, options);

		cmdFilestream.close();
	}
	return true;
}

//
MDM_API mdm_OptionsParser::ParseType mdm_OptionsParser::parseInputs(
	po::options_description &cmdline_options,
	po::options_description &config_options,
	const std::string &configFile,
	const std::string &configType,
	int argc, const char *argv[])
{
	po::options_description combined_options("");
	combined_options.add(cmdline_options).add(config_options);

	//Parse the command-line
	if (!parse_command_line(argc, argv, combined_options))
		return CMD_ERROR;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, combined_options))
		return HELP;

	//Check if version set
	if (version_set())
		return VERSION;

	//Check if config file set, if so try and open it
	if (!parse_config_file(config_options, configFile, configType))
		return CONFIG_ERROR;

	return OK;
}

MDM_API mdm_OptionsParser::ParseType mdm_OptionsParser::parseInputs(
	po::options_description &cmdline_options,
	int argc, const char *argv[])
{

	//Parse the command line
	if (!parse_command_line(argc, argv, cmdline_options))
		return CMD_ERROR;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, cmdline_options))
		return HELP;

	//Check if version set
	if (version_set())
		return VERSION;

	return OK;
}

MDM_API const std::string& mdm_OptionsParser::exe_args() const
{
	return exe_args_;
}

MDM_API const std::string& mdm_OptionsParser::exe_cmd() const
{
	return exe_cmd_;
}

template<class T, class T_out>
MDM_API void mdm_OptionsParser::add_option(po::options_description &config_options, 
	mdm_Input<T, T_out> &option)
{
	config_options.add_options()
		(option.combined_key(),
		po::value<T>(&option.value())->default_value(option()),
		option.info());
}

template void MDM_API mdm_OptionsParser::add_option(po::options_description &c, 
	mdm_input_string &o); //!< Template declaration for string input
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_int &o); //!< Template declaration for int input
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_double &o); //!< Template declaration for double input
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_strings &o);  //!< Template declaration for list of strings input
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_ints &o);  //!< Template declaration for list of ints input
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_doubles &o);  //!< Template declaration for list of doubles input
template void MDM_API mdm_OptionsParser::add_option(po::options_description& c,
	mdm_input_dicom_tag& o);  //!< Template declaration for list of doubles input

//! Template specialization for bool input
template<>
MDM_API void mdm_OptionsParser::add_option(po::options_description &config_options, mdm_input_bool &b)
{
	config_options.add_options()
		(b.combined_key(), \
			po::bool_switch(&b.value())->default_value(b())->multitoken(), \
			b.info());
}

//****************************************************************************
// Private functions
//****************************************************************************

bool mdm_OptionsParser::all_to_stream(std::ostream& stream,
	const mdm_InputOptions& options, const std::string& caller) const
{
	//Print out the config and cwd options first, commented so they don't get
	//read in by the Boosts config reader
	stream << "#" << caller << "\n";
	stream << "#" << options.version.key() << " = " << MDM_VERSION << "\n";
	stream << "#" << options.configFile.key() << " = " << options.configFile() << "\n";
	stream << "#" << options.dataDir.key() << " = " << options.dataDir() << "\n";

	return to_stream(stream, vm_, options, false);
}

bool mdm_OptionsParser::cmd_to_stream(std::ostream& stream,
	const mdm_InputOptions& options) const
{
	//Print out the config and cwd options first, commented so they don't get
	//read in by the Boosts config reader
	return to_stream(stream, cmd_vm_, options, true);
}

bool mdm_OptionsParser::gui_to_stream(std::ostream& stream,
	const mdm_InputOptions& options) const
{
	for (auto const& it : options.guiSetOptions)
	{
		stream << it.first  // string (key)
			<< " = "
			<< it.second // string's value 
			<< std::endl;
	}
	return true;
}

bool mdm_OptionsParser::to_stream(std::ostream& stream, const po::variables_map& vm,
	const mdm_InputOptions& options, bool nondefault_only) const
{
	for (const auto& it : vm)
	{
		auto& key = it.first;
		if (key == options.configFile.key() || key == options.dataDir.key() ||
			key == options.help.key() || key == options.version.key())
			continue;

		if (nondefault_only && vm[key].defaulted())
			continue;

		stream << key << " = ";
		auto& value = it.second.value();
		if (auto v = boost::any_cast<bool>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<int>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<double>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<mdm_input_str>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<mdm_input_string_list>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<mdm_input_int_list>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<mdm_input_double_list>(&value))
			stream << *v;
		else if (auto v = boost::any_cast<mdm_input_dicomTag>(&value))
			stream << *v;
		stream << "\n";
	}
	return true;
}

bool mdm_OptionsParser::parse_command_line(int argc, const char *argv[],
	const po::options_description &combined_options)
{
	//Combine argv into a standard string
	make_exe_args(argc, argv);

	//Clear the vm
	vm_.clear();

	//Parse the command line
	try
	{
		po::store(po::command_line_parser(argc+int(!argc), argv).
			options(combined_options).run(), vm_); //

		po::store(po::command_line_parser(argc + int(!argc), argv).
			options(combined_options).run(), cmd_vm_); //

		po::notify(vm_);	
	}
	catch (const po::error &e)
	{
		std::cout << "Boost::program_options Error parsing command line" << std::endl;
		std::cout << e.what();
		return false;
	}
	catch (const boost::bad_lexical_cast& e)
	{
		std::cout << "Boost::error Error parsing command line" << std::endl;
		std::cout << e.what();
		return false;
	}
  catch (const mdm_exception& e)
  {
    std::cout << "Madym::error Error parsing command line" << std::endl;
    std::cout << e.what();
    return false;
  }
	catch (...)
	{
		std::cout << "Unhandled error parsing command line" << std::endl;
		return false;
	}
	return true;
}

bool mdm_OptionsParser::help_set(int argc, const po::options_description &combined_options)
{
	//Check if help set, if so, just display options and quit
	if (argc == 1 || vm_["help"].as<bool>()) {
		std::stringstream ss;
		ss << combined_options << "\n";
		mdm_ProgramLogger::logProgramMessage(ss.str());
		return true;
	}
	return false;
}

bool mdm_OptionsParser::version_set()
{
	//Check if version set
	if (vm_["version"].as<bool>()) {
		mdm_ProgramLogger::logProgramMessage(MDM_VERSION);
		return true;
	}
	return false;
}

bool mdm_OptionsParser::parse_config_file(const po::options_description &config_options,
	const std::string &configFile, const std::string &configType)
{
	try
	{
		//Check if config file set, if so try and open it
		if (!configFile.empty())
		{
			std::ifstream ifs(configFile);
			if (!ifs)
			{
				//If it won't open, return an error
				std::cout << "can not open config file: " << configFile << "\n";
				return false;
			}
			else
			{
				//Check config file is of correct type
				std::stringstream iss;
				if (!check_config_type(ifs, configType, iss))
					return false;

				//Parse the config file into the variables map
				po::store(po::parse_config_file(iss, config_options), vm_);
				po::notify(vm_);
			}
		}
	}
	catch (const po::error &e)
	{
		std::cout << "Boost::program_options Error parsing config file" << std::endl;
		std::cout << e.what();
		return false;
	}
	catch (const boost::bad_lexical_cast &e)
	{
		std::cout << "Boost::error Error parsing config file" << std::endl;
		std::cout << e.what();
		return false;
	}
	catch (...)
	{
		std::cout << "Unhandled error parsing command line" << std::endl;
		return false;
	}
	return true;
}

bool mdm_OptionsParser::check_config_type(std::ifstream &ifs, const std::string &configType, std::stringstream& ss)
{
	//Get the first line of the config file
	std::string inputConfigType;
	std::getline (ifs, inputConfigType);

	//Remove the # prefix
	inputConfigType.erase(0,1);

	//Deal with different line ends
	bool badLineEnd = false;
	auto badLineEndPos = inputConfigType.find(NEWLINE_FIND);
	if (badLineEndPos != std::string::npos)
	{
		inputConfigType.erase(badLineEndPos);
		badLineEnd = true;
	}
	
	//Check config type
	if (inputConfigType != configType)
	{
		std::cout << "Input config type " << inputConfigType << " does not match required type "
			<< configType << std::endl;
		return false;
	}

	//If there was mismatched line-ending, return to the start of the file
	//as we may have consumed the whole file
	if (badLineEnd)
		ifs.seekg(0, ifs.beg);

	//Copy file content into stringstream
	ss << ifs.rdbuf();

	//If necessary update the stringstream to replace bad line ends
	if (badLineEnd)
	{
		std::cout << "Replaced line-endings in config file\n";
		auto ss_str = ss.str();
		boost::replace_all(ss_str, NEWLINE_FIND, NEWLINE_REPLACE);
		ss.str(ss_str);
	}
	return true;
}

void mdm_OptionsParser::make_exe_args(int argc, const char *argv[])
{
	exe_cmd_ = std::string(argv[0]);
	exe_args_ = exe_cmd_;
	for (int i = 1; i < argc; i++)
	{
		exe_args_ += " " + std::string(argv[i]);
	}
	exe_args_ += "\n";
}

