/**
 *  @file    mdm_OptionsParser.cxx
 *  @brief   Implementation of mdm_OptionsParser class
 */

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include <mdm_InputTypes.h>

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

