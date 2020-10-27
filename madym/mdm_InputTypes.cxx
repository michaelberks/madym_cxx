/**
 *  @file    mdm_OptionsParser.cxx
 *  @brief   Implementation of mdm_OptionsParser class
 */

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include <mdm_InputTypes.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

const std::string mdm_input_str::EMPTY_STR = "\"\"";

template <>
MDM_API const std::string& mdm_input_string::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
MDM_API const std::vector<std::string>& mdm_input_strings::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
MDM_API const std::vector<int>& mdm_input_ints::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
MDM_API const std::vector<double>& mdm_input_doubles::operator() ()  const// specialize only one member
{
	return value_();
}

template <>
MDM_API const int& mdm_input_int::operator() ()  const// specialize only one member
{
	return value_;
}

template <>
MDM_API const double& mdm_input_double::operator() ()  const// specialize only one member
{
	return value_;
}

template <>
MDM_API const bool& mdm_input_bool::operator() ()  const// specialize only one member
{
	return value_;
}

mdm_input_str::mdm_input_str()
{}

mdm_input_str::mdm_input_str(std::string s) :
	str(s)
{}

mdm_input_str::~mdm_input_str()
{

}

const std::string& mdm_input_str::operator() () const 
{ 
	return str; 
}
//******************************************************************

//*******************************************************************
mdm_input_int_list::mdm_input_int_list() 
{}

mdm_input_int_list::~mdm_input_int_list()
{}

mdm_input_int_list::mdm_input_int_list(std::vector<int> l) 
	: list(l) 
{}

const std::vector<int>& mdm_input_int_list::operator() () const 
{ 
	return list; 
}

//*******************************************************************
mdm_input_double_list::mdm_input_double_list()
{}

mdm_input_double_list::~mdm_input_double_list()
{}

mdm_input_double_list::mdm_input_double_list(std::vector<double> l)
	: list(l)
{}

const std::vector<double>& mdm_input_double_list::operator() () const
{
	return list;
}

//*******************************************************************
mdm_input_string_list::mdm_input_string_list()
{}

mdm_input_string_list::~mdm_input_string_list()
{}

mdm_input_string_list::mdm_input_string_list(std::vector<std::string> l)
	: list(l)
{}

const std::vector<std::string>& mdm_input_string_list::operator() () const
{
	return list;
}

//******************************************************************
//-------------------------------------------------------------------------
std::ostream& operator << (std::ostream& os, const mdm_input_str& v)
{
	if (v().empty())
		os << "\"\"";// v.empty_str();
	else
		os << v();
	return os;
}

//-------------------------------------------------------------------------
std::ostream& operator << (std::ostream& os, const mdm_input_string_list& v)
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

//-------------------------------------------------------------------------

std::ostream& operator << (std::ostream& os, const mdm_input_int_list& v)
{
	os << "[";
	for (const auto ii : v())
		os << " " << ii;

	os << " ]";
	return os;
}

//-------------------------------------------------------------------------

std::ostream& operator << (std::ostream& os, const mdm_input_double_list& v)
{
	os << "[";
	for (const auto ii : v())
		os << " " << ii;

	os << " ]";
	return os;
}
