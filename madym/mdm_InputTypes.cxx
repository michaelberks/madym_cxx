/**
*  @file    mdm_InputTypes.cxx
*  @brief   Implementation of mdm_InputTypes class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
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
#include <madym/mdm_exception.h>
#include <madym/mdm_platform_defs.h>

const std::string mdm_input_str::EMPTY_STR = "\"\"";

//Suppress doxygen warnings caused by typedefs/templates below

//! @cond Doxygen_Suppress

// Template specialization to return option value as standard string
template <>
MDM_API const std::string& mdm_input_string::operator() ()  const
{
	return value_();
}

// Template specialization to return option value as vector of standard strings
template <>
MDM_API const std::vector<std::string>& mdm_input_strings::operator() ()  const
{
	return value_();
}

// Template specialization to return option value as vector of ints
template <>
MDM_API const std::vector<int>& mdm_input_ints::operator() ()  const
{
	return value_();
}

// Template specialization to return option value as vector of doubles
template <>
MDM_API const std::vector<double>& mdm_input_doubles::operator() ()  const
{
	return value_();
}

// Template specialization to return option value as int
template <>
MDM_API const int& mdm_input_int::operator() ()  const
{
	return value_;
}

// Template specialization to return option value as double
template <>
MDM_API const double& mdm_input_double::operator() ()  const
{
	return value_;
}

// Template specialization to return option value as bool
template <>
MDM_API const bool& mdm_input_bool::operator() ()  const// specialize only one member
{
	return value_;
}

//Switch doxygen parsing back on
//! @endcond 


MDM_API mdm_input_str::mdm_input_str()
{}

MDM_API mdm_input_str::mdm_input_str(std::string str) :
	str_(str)
{}

MDM_API mdm_input_str::~mdm_input_str()
{

}

MDM_API const std::string& mdm_input_str::operator() () const
{ 
	return str_; 
}
//******************************************************************

//*******************************************************************
MDM_API mdm_input_int_list::mdm_input_int_list()
{}

MDM_API mdm_input_int_list::~mdm_input_int_list()
{}

MDM_API mdm_input_int_list::mdm_input_int_list(std::vector<int> list)
	: list_(list)
{}

MDM_API mdm_input_int_list::mdm_input_int_list(const std::string &str)
{
  fromString(str);
}

MDM_API const std::vector<int>& mdm_input_int_list::operator() () const
{ 
	return list_; 
}

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNKNOWN_ESCAPE_SEQUENCE

MDM_API void mdm_input_int_list::fromString(const std::string &str)
{
  //Format:
  // optional enclosing [ ]
  // comma separated
  // - indicates range between start and end items

  //eg: [1-3, 5] should generate {1, 2, 3, 5}

  // Do regex match and convert the interesting part to 
  // int.
  std::vector<std::string> tokens;
  boost::split(tokens, str, boost::is_any_of("  ,\[\]"));

  list_.clear();
  for (auto t : tokens)
  {
    if (!t.empty())
    {
      std::vector<std::string> range;
      boost::split(range, t, boost::is_any_of("-"));

      if (range.size() == 1)
        list_.push_back(boost::lexical_cast<int>(t));

      else if (range.size() == 2)
      {
        int start = boost::lexical_cast<int>(range[0]);
        int end = boost::lexical_cast<int>(range[1]);
        for (int i = start; i <= end; i++)
          list_.push_back(i);
      }
      else
        throw mdm_exception(__func__, 
          "Error parsing " + str + " to integer list. "
          "Range operation for integer lists should be of form i-j");
    }

  }
}
DISABLE_WARNING_POP

MDM_API std::string mdm_input_int_list::toString() const
{
  if (list_.empty())
    return "[]";

  std::stringstream ss;
  ss << "[" << list_[0];

  bool range_open = false;
  for (int idx = 1; idx < list_.size(); idx++)
  {
    bool is_increment = list_[idx] == list_[idx - 1] + 1;
    if (!range_open)
    {
      if (is_increment)
      {
        ss << "-";
        range_open = true;
      }
      else
        ss << "," << list_[idx];
    }
    else if (!is_increment)
    {
      ss << list_[idx - 1] << "," << list_[idx];
      range_open = false;
    }
  }
  if (range_open)
    ss << list_.back();

  ss << " ]";
  return ss.str();
}

//*******************************************************************
MDM_API mdm_input_double_list::mdm_input_double_list()
{}

MDM_API mdm_input_double_list::~mdm_input_double_list()
{}

MDM_API mdm_input_double_list::mdm_input_double_list(std::vector<double> list)
	: list_(list)
{}

MDM_API mdm_input_double_list::mdm_input_double_list(const std::string &str)
{
  fromString(str);
}

MDM_API const std::vector<double>& mdm_input_double_list::operator() () const
{
	return list_;
}

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNKNOWN_ESCAPE_SEQUENCE
MDM_API void mdm_input_double_list::fromString(const std::string &str)
{
  // Do regex match and convert the interesting part to 
  // int.
  std::vector<std::string> tokens;
  boost::split(tokens, str, boost::is_any_of("  ,\[\]"));

  list_.clear();
  for (auto t : tokens)
    if (!t.empty())
      list_.push_back(boost::lexical_cast<double>(t));
}
DISABLE_WARNING_POP

//
MDM_API std::string mdm_input_double_list::toString() const
{
  std::stringstream ss;
  ss << "[";
  int i = 0;
  for (const auto d : list_)
  {
    ss << "" << d;
    i++;
    if (i < list_.size())
      ss << ",";
  }
  ss << "]";
  return ss.str();
}

//*******************************************************************
MDM_API mdm_input_string_list::mdm_input_string_list()
{}

MDM_API mdm_input_string_list::~mdm_input_string_list()
{}

MDM_API mdm_input_string_list::mdm_input_string_list(std::vector<std::string> list)
	: list_(list)
{}

MDM_API mdm_input_string_list::mdm_input_string_list(const std::string &str)
{
  fromString(str);
}

MDM_API const std::vector<std::string>& mdm_input_string_list::operator() () const
{
	return list_;
}

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNKNOWN_ESCAPE_SEQUENCE

MDM_API void mdm_input_string_list::fromString(const std::string &str)
{
  std::string s2(str);
  boost::erase_all(s2, "[");
  boost::erase_all(s2, "]");

  // Do regex match and convert the interesting part to 
  // int.
  boost::escaped_list_separator<char> sep{ "", ",", "\"\'" };
  boost::tokenizer<boost::escaped_list_separator<char> > tokens(s2, sep);

  list_.clear();
  for (auto t : tokens)
  {
    std::string s3(t);
    boost::trim(s3);
    if (!s3.empty())
      list_.push_back(s3);
  }
}
DISABLE_WARNING_POP

MDM_API std::string mdm_input_string_list::toString() const
{
  std::stringstream ss;
  ss << "[";
  int i = 0;
  for (const auto str : list_)
  {
    ss << "" << str;
    i++;
    if (i < list_.size())
      ss << ",";
  }
  ss << "]";
  return ss.str();
}

//******************************************************************
//-------------------------------------------------------------------------

//! Operator overload to print custom string type to streams
std::ostream& operator << (std::ostream& os, const mdm_input_str& v)
{
	if (v().empty())
		os << mdm_input_str::EMPTY_STR;
	else
		os << v();
	return os;
}
