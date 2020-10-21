/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "mdm_InputOptions.h"
#include <mdm_version.h>

#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

const std::string mdm_InputOptions::empty_str = "\"\"";

template <>
const std::string& mdm_input_string::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
const std::vector<std::string>& mdm_input_strings::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
const std::vector<int>& mdm_input_ints::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
const std::vector<double>& mdm_input_doubles::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
const int& mdm_input_int::operator() ()  const// specialize only one member
{
	return value_;
}

template <>
const double& mdm_input_double::operator() ()  const// specialize only one member
{
	return value_;
}

template <>
const bool& mdm_input_bool::operator() ()  const// specialize only one member
{
	return value_;
}

inline std::ostream& operator << (std::ostream& os, const mdm_input_str& v)
{
	if (v().empty())
		os << mdm_InputOptions::empty_str;
	else
		os << v();
	return os;
}

inline std::ostream& operator << (std::ostream& os, const mdm_input_string_list& v)
{
	os << "[";
	int i = 0;
	for (const auto ii : v())
	{
		os << "" << ii;
		i++;
		if (i < v().size())
			os << ",";
	}
		

	os << "]";
	return os;
}

inline std::ostream& operator << (std::ostream& os, const mdm_input_int_list& v)
{
	os << "[";
	for (const auto ii : v())
		os << " " << ii;

	os << " ]";
	return os;
}

inline std::ostream& operator << (std::ostream& os, const mdm_input_double_list& v)
{
	os << "[";
	for (const auto ii : v())
		os << " " << ii;

	os << " ]";
	return os;
}

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

	if (s == mdm_InputOptions::empty_str)
		v = mdm_input_str("");
		

	else
		v = mdm_input_str(s);
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

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);

	// Do regex match and convert the interesting part to 
	// int.
	boost::char_separator<char> sep("  ,[]");
	boost::tokenizer<boost::char_separator<char> > tokens(s, sep);
	std::vector<double> vd;
	for (auto t : tokens)
		vd.push_back(boost::lexical_cast<double>(t));

	v = mdm_input_double_list(vd);
}

void validate(boost::any& v,
	const std::vector<std::string>& values,
	mdm_input_int_list* target_type, int)
{

	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);
	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	const std::string& s = po::validators::get_single_string(values);

	// Do regex match and convert the interesting part to 
	// int.
	boost::char_separator<char> sep("  ,[]");
	boost::tokenizer<boost::char_separator<char> > tokens(s, sep);
	std::vector<int> vi;
	for (auto t : tokens)
		vi.push_back(boost::lexical_cast<int>(t));

	v = mdm_input_int_list(vi);
}

/**
*  @brief   Defines options and their default values for all inputs to DCE analysis and T1 mapping tools
*/

#define InputOption(option, option_type) \
         (option.combined_key(), \
					po::value<option_type>(&option.value())->default_value(option()),\
					option.info())
#define InputOptionFlag(flag) \
         (flag.combined_key(), \
					po::bool_switch(&flag.value())->default_value(flag())->multitoken(),\
					flag.info())

MDM_API  mdm_InputOptions::mdm_InputOptions()
{
	help_.add_options()
		("help,h", "Print options and quit")
		("version,v", "Print version and quit")
		;
}

MDM_API bool mdm_InputOptions::to_stream(std::ostream &stream, 
	const mdm_DefaultValues &options) const
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

MDM_API bool mdm_InputOptions::to_file(const std::string &filename, 
	const mdm_DefaultValues &options) const
{
	std::ofstream filestream(filename, std::ios::out);
	if (!filestream.is_open())
		return false;
	to_stream(filestream, options);

	filestream.close();
	return true;
}

MDM_API int mdm_InputOptions::madym_inputs(int argc, const char *argv[],
	mdm_DefaultValues &options)
{
	po::options_description cmdline_options("madym options");
	po::options_description config_options("madym config options");

	cmdline_options.add_options()
		//Generic input options applied to all command-line tools
		InputOption(options.configFile, mdm_input_str)
		InputOption(options.dataDir, mdm_input_str)
		;

	config_options.add_options()
		//DCE input options
		InputOptionFlag(options.inputCt)
		InputOption(options.dynName, mdm_input_str)
		InputOption(options.dynDir, mdm_input_str)
		InputOption(options.dynFormat, mdm_input_str)
		InputOption(options.nDyns, int)
		InputOption(options.injectionImage, int)

		//ROI options
		InputOption(options.roiName, mdm_input_str)

		//T1 calculation options
		InputOption(options.T1method, mdm_input_str)
		InputOption(options.T1inputNames, mdm_input_string_list)
		InputOption(options.T1noiseThresh, double)
		InputOption(options.nT1Inputs, int)

		//Signal to concentration options
		InputOptionFlag(options.M0Ratio)
		InputOption(options.T1Name, mdm_input_str)
		InputOption(options.M0Name, mdm_input_str)
		InputOption(options.r1Const, double)

		//AIF options
		InputOption(options.aifName, mdm_input_str)
		InputOption(options.pifName, mdm_input_str)
		InputOption(options.aifSlice, int)
		InputOption(options.dose, double)
		InputOption(options.hct, double)

		//Model options
		InputOption(options.model, mdm_input_str)
		InputOption(options.initParams, mdm_input_double_list)
		InputOption(options.initMapsDir, mdm_input_str)
		InputOption(options.initMapParams, mdm_input_int_list)
		InputOption(options.paramNames, mdm_input_string_list)
		InputOption(options.fixedParams, mdm_input_int_list)
		InputOption(options.fixedValues, mdm_input_double_list)
		InputOption(options.relativeLimitParams, mdm_input_int_list)
		InputOption(options.relativeLimitValues, mdm_input_double_list)
		InputOption(options.firstImage, int)
		InputOption(options.lastImage, int)

		InputOptionFlag(options.noOptimise)
		InputOptionFlag(options.dynNoise)
		InputOptionFlag(options.testEnhancement)
		InputOption(options.maxIterations, int)

		//DCE only output options
		InputOptionFlag(options.outputCt_sig)
		InputOptionFlag(options.outputCt_mod)
		InputOption(options.IAUCTimes, mdm_input_double_list)

		//General output options
		InputOption(options.outputDir, mdm_input_str)
		InputOptionFlag(options.overwrite)
		InputOptionFlag(options.sparseWrite)

		//Logging options
		InputOption(options.errorCodesName, mdm_input_str)
		InputOption(options.programLogName, mdm_input_str)
		InputOption(options.outputConfigFileName, mdm_input_str)
		InputOption(options.auditLogBaseName, mdm_input_str)
		InputOption(options.auditLogDir, mdm_input_str)
		;

	po::options_description combined_options("");
	combined_options.add(cmdline_options).add(config_options).add(help_);

	//Parse the command-line
	if (!parse_command_line(argc, argv, combined_options))
		return 2;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, combined_options))
		return 3;

	//Check if version set
	if (version_set())
		return 4;

	//Check if config file set, if so try and open it
	if (!parse_config_file(config_options, options))
		return 5;

	return 0;
	
}

MDM_API int mdm_InputOptions::madym_lite_inputs(int argc, const char *argv[],
	mdm_DefaultValues &options)
{
	po::options_description config_options("madym-lite config options");

	config_options.add_options()
		InputOption(options.dataDir, mdm_input_str)

		//DCE input options
		InputOption(options.inputDataFile, mdm_input_str)
		InputOptionFlag(options.inputCt)
		InputOption(options.dynTimesFile, mdm_input_str)
		InputOption(options.nDyns, int)
		InputOption(options.injectionImage, int)

		//Signal to concentration options
		InputOptionFlag(options.M0Ratio)
		InputOption(options.r1Const, double)
		InputOption(options.FA, double)
		InputOption(options.TR, double)

		//AIF options
		InputOption(options.aifName, mdm_input_str)
		InputOption(options.pifName, mdm_input_str)
		InputOption(options.dose, double)
		InputOption(options.hct, double)

		//Model options
		InputOption(options.model, mdm_input_str)
		InputOption(options.initParams, mdm_input_double_list)
		InputOption(options.initParamsFile, mdm_input_str)
		InputOption(options.paramNames, mdm_input_string_list)
		InputOption(options.fixedParams, mdm_input_int_list)
		InputOption(options.fixedValues, mdm_input_double_list)
		InputOption(options.relativeLimitParams, mdm_input_int_list)
		InputOption(options.relativeLimitValues, mdm_input_double_list)
		InputOption(options.firstImage, int)
		InputOption(options.lastImage, int)

		InputOptionFlag(options.noOptimise)
		InputOption(options.dynNoiseFile, mdm_input_str)
		InputOptionFlag(options.testEnhancement)
		InputOption(options.maxIterations, int)

		//DCE only output options
		InputOptionFlag(options.outputCt_sig)
		InputOptionFlag(options.outputCt_mod)
		InputOption(options.IAUCTimes, mdm_input_double_list)

		//General output options
		InputOption(options.outputName, mdm_input_str)
		InputOption(options.outputDir, mdm_input_str)
		;

	config_options.add(help_);

	//Parse the command line
	if (!parse_command_line(argc, argv, config_options))
		return 1;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, config_options))
		return 2;

	//Check if version set
	if (version_set())
		return 3;

	return 0;
}

MDM_API int mdm_InputOptions::calculate_T1_inputs(int argc, const char *argv[],
	mdm_DefaultValues &options)
{
	po::options_description cmdline_options("calculate_T1 options");
	po::options_description config_options("calculate_T1 config options");

	cmdline_options.add_options()
		//Generic input options applied to all command-line tools
		InputOption(options.configFile, mdm_input_str)
		InputOption(options.dataDir, mdm_input_str)
		;

	config_options.add_options()

		//ROI options
		InputOption(options.roiName, mdm_input_str)

		//T1 calculation options
		InputOption(options.T1method, mdm_input_str)
		InputOption(options.T1inputNames, mdm_input_string_list)
		InputOption(options.T1noiseThresh, double)
		InputOption(options.nT1Inputs, int)

		//General output options
		InputOption(options.outputDir, mdm_input_str)
		InputOptionFlag(options.sparseWrite)
		InputOptionFlag(options.overwrite)

		//Logging options
		InputOption(options.errorCodesName, mdm_input_str)
		InputOption(options.programLogName, mdm_input_str)
		InputOption(options.outputConfigFileName, mdm_input_str)
		InputOption(options.auditLogBaseName, mdm_input_str)
		InputOption(options.auditLogDir, mdm_input_str)
		;

	po::options_description combined_options("");
	combined_options.add(cmdline_options).add(config_options).add(help_);

	//Parse the command-line
	if (!parse_command_line(argc, argv, combined_options))
		return 2;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, combined_options))
		return 3;

	//Check if version set
	if (version_set())
		return 4;

	//Check if config file set, if so try and open it
	if (!parse_config_file(config_options, options))
		return 5;

	return 0;
}

MDM_API int mdm_InputOptions::calculate_T1_lite_inputs(int argc, const char *argv[],
	mdm_DefaultValues &options)
{
	po::options_description config_options("calculate_T1_lite config options");

	config_options.add_options()
		InputOption(options.dataDir, mdm_input_str)
		//T1 calculation options
		InputOption(options.inputDataFile, mdm_input_str)
		InputOption(options.T1method, mdm_input_str)
		InputOption(options.FA, double)
		InputOption(options.TR, double)
		InputOption(options.T1noiseThresh, double)
		InputOption(options.nT1Inputs, int)

		//General output options
		InputOption(options.outputDir, mdm_input_str)
		InputOption(options.outputName, mdm_input_str)
		;
	config_options.add(help_);

	//Parse the command line
	if (!parse_command_line(argc, argv, config_options))
		return 1;

	//Check if help set, if so, just display options and quit
	if (help_set(argc, config_options))
		return 2;

	//Check if version set
	if (version_set())
		return 3;

	return 0;
}

//Overrides for non-commandline input
MDM_API int mdm_InputOptions::madym_inputs(const std::string &argv,
	mdm_DefaultValues &options)
{
	const char*argvc[] = { argv.c_str() };
	return madym_inputs(0, argvc, options);
}

MDM_API int mdm_InputOptions::madym_lite_inputs(const std::string &argv,
	mdm_DefaultValues &options)
{
	const char*argvc[] = { argv.c_str() };
	return madym_lite_inputs(0, argvc, options);
}

MDM_API int mdm_InputOptions::calculate_T1_inputs(const std::string &argv,
	mdm_DefaultValues &options)
{
	const char*argvc[] = { argv.c_str() };
	return calculate_T1_inputs(0, argvc, options);
}

MDM_API int mdm_InputOptions::calculate_T1_lite_inputs(const std::string &argv,
	mdm_DefaultValues &options)
{
	const char*argvc[] = { argv.c_str() };
	return calculate_T1_lite_inputs(0, argvc, options);
}

MDM_API const std::string& mdm_InputOptions::exe_args() const
{
	return exe_args_;
}

MDM_API const std::string& mdm_InputOptions::exe_cmd() const
{
	return exe_cmd_;
}

//****************************************************************************
// Private functions
//****************************************************************************

bool mdm_InputOptions::parse_command_line(int argc, const char *argv[],
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

bool mdm_InputOptions::help_set(int argc, const po::options_description &combined_options)
{
	//Check if help set, if so, just display options and quit
	if (argc == 1 || vm_.count("help")) {
		std::cout << combined_options << "\n";
		return true;
	}
	return false;
}

bool mdm_InputOptions::version_set()
{
	//Check if version set
	if (vm_.count("version")) {
		std::cout << MDM_VERSION << "\n";
		return true;
	}
	return false;
}

bool mdm_InputOptions::parse_config_file(const po::options_description &config_options,
	const mdm_DefaultValues &options)
{
	try
	{
		//Check if config file set, if so try and open it
		if (!options.configFile().empty())
		{
			std::ifstream ifs(options.configFile());
			if (!ifs)
			{
				//If it won't open, return an error
				std::cout << "can not open config file: " << options.configFile() << "\n";
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
		std::cout << "Boost::program_options Error parsing command line" << std::endl;
		std::cout << e.what();
		return false;
	}
	catch (const boost::bad_lexical_cast &e)
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

void mdm_InputOptions::make_exe_args(int argc, const char *argv[])
{
	exe_cmd_ = std::string(argv[0]);
	exe_args_ = exe_cmd_;
	for (int i = 1; i < argc; i++)
	{
		exe_args_ += " " + std::string(argv[i]);
	}
	exe_args_ += "\n";
}

