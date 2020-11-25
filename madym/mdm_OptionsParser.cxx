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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

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
void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_string_list* target_type, int)
{

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);

	std::string s2(s);
	boost::erase_all(s2, "[");
	boost::erase_all(s2, "]");

	// Do regex match and convert the interesting part to 
	// int.
	boost::escaped_list_separator<char> sep{ "", ",", "\"\'" };
	boost::tokenizer<boost::escaped_list_separator<char> > tokens(s2, sep);
	std::vector<std::string> vs;
	for (auto t : tokens)
	{
		std::string s3(t);
		boost::trim(s3);
		if (!s3.empty())
			vs.push_back(s3);
	}	

	v = mdm_input_string_list(vs);
}

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

	// Do regex match and convert the interesting part to 
	// int.
  std::vector<std::string> tokens;
  boost::split(tokens, s, boost::is_any_of("  ,\[\]"));

  std::vector<double> vd;
  for (auto t : tokens)
    if (!t.empty())
      vd.push_back(boost::lexical_cast<double>(t));

	v = mdm_input_double_list(vd);
}

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

	// Do regex match and convert the interesting part to 
	// int.
  std::vector<std::string> tokens;
  boost::split(tokens, s, boost::is_any_of("  ,\[\]"));

  std::vector<int> vi;
  for (auto t : tokens)
  {
    if (!t.empty())
    {
      std::vector<std::string> range;
      boost::split(range, t, boost::is_any_of("-"));

      if (range.size() == 1)
        vi.push_back(boost::lexical_cast<int>(t));

      else if (range.size() == 2)
      {
        int start = boost::lexical_cast<int>(range[0]);
        int end = boost::lexical_cast<int>(range[1]);
        for (int i = start; i <= end; i++)
          vi.push_back(i);
      }
      else
        throw po::error("Range operation for integer lists should be of form i-j");
    }
      
  } 

	v = mdm_input_int_list(vi);
}

/**
*  @brief   Defines options and their default values for all inputs to DCE analysis and T1 mapping tools
*/

MDM_API  mdm_OptionsParser::mdm_OptionsParser()
{
	help_.add_options()
		("help,h", "Print options and quit")
		("version,v", "Print version and quit")
		;
}

/*
*/
MDM_API bool mdm_OptionsParser::to_stream(std::ostream &stream, 
	const mdm_InputOptions &options) const
{
	//Print out the config and cwd options first, commented so they don't get
	//read in by the Boosts config reader
	stream << "#" << options.configFile.key() << " = " << options.configFile() << "\n";
	stream << "#" << options.dataDir.key() << " = " << options.dataDir() << "\n";

	for (const auto& it : vm_) 
	{
		auto &key = it.first;
		if (key == options.configFile.key() || key == options.dataDir.key())
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
		stream << "\n";
	}
	return true;
}

/*
*/
MDM_API bool mdm_OptionsParser::to_file(const std::string &filename, 
	const mdm_InputOptions &options) const
{
	std::ofstream filestream(filename, std::ios::out);
	if (!filestream.is_open())
		return false;
	to_stream(filestream, options);

	filestream.close();
	return true;
}

/*
*/
MDM_API int mdm_OptionsParser::parseInputs(
	po::options_description &cmdline_options,
	po::options_description &config_options,
	const std::string &configFile,
	int argc, const char *argv[])
{
	po::options_description combined_options("");
	combined_options.add(cmdline_options).add(config_options).add(help_);

	//Parse the command-line
	if (!parse_command_line(argc, argv, combined_options))
		return 1;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, combined_options))
		return 2;

	//Check if version set
	if (version_set())
		return 3;

	//Check if config file set, if so try and open it
	if (!parse_config_file(config_options, configFile))
		return 4;

	return 0;
}

MDM_API int mdm_OptionsParser::parseInputs(
	po::options_description &cmdline_options,
	int argc, const char *argv[])
{
	cmdline_options.add(help_);

	//Parse the command line
	if (!parse_command_line(argc, argv, cmdline_options))
		return 1;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, cmdline_options))
		return 2;

	//Check if version set
	if (version_set())
		return 3;

	return 0;
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
	mdm_input_string &o);
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_int &o);
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_double &o);
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_strings &o);
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_ints &o);
template void MDM_API mdm_OptionsParser::add_option(po::options_description &c,
	mdm_input_doubles &o);

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
	if (argc == 1 || vm_.count("help")) {
		std::cout << combined_options << "\n";
		return true;
	}
	return false;
}

bool mdm_OptionsParser::version_set()
{
	//Check if version set
	if (vm_.count("version")) {
		std::cout << MDM_VERSION << "\n";
		return true;
	}
	return false;
}

bool mdm_OptionsParser::parse_config_file(const po::options_description &config_options,
	const std::string &configFile)
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
				//Parse the config file into the variables map
				po::store(po::parse_config_file(ifs, config_options), vm_);
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

