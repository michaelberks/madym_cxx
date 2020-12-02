//! Defines input types for options from command-line or config file
/*!
\file    mdm_InputTypes.h

\ MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_INPUT_TYPES_HDR
#define MDM_INPUT_TYPES_HDR

#include "mdm_api.h"

#include <string>
#include <vector>

//! Template class, defining an input option comprising a key, value and information text
/*!
We use using boost::program_options to parse options for the T1 mapping and DCE analysis tools from either command-line or a config file. To achieve this we define two levels of classes:

1. mdm_Input acts as container for each option, comprising a key (in long and short form), the option value and information text describing the option.
2. Wrapper classes mdm_input_str, mdm_input_string_list, mdm_input_int_list and mdm_input_double_list to allow us to customise how boost::program_option parse string and vector inputs

For the customised option types, we need to provide the wrapped class to boost::program_options, but use the unwrapped standard library class in the rest of our code. To achieve this, mdm_Input is templated with <T_wrapped,T_unwrapped>. So for example a string option is stored in an object of type mdm_Input<mdm_input_str,std::string>. For input types we do not need to customise (ints, bools and doubles) T_unwrapped = T_wrapped, eg an integer option is stored in an object mdm_Input<int,int>.

To hide the ugly double-templates, we use typdefs #mdm_input_int, #mdm_input_string etc. It seems like a lot of boiler plate, but defining all this here makes is much easier to have a clear and concsise interface for defining and using options in the rest of the library.
*/
template <class T_wrapped, class T_unwrapped>
class mdm_Input {

public:
	
	//! Constructor
	/*!
	\param value input option value of varying type
	\param k long form version of key, set at command line using --
	\param ks single-character version of key, set at command line using -
	\param info information text describing the option

	\see mdm_input_strings
	\see mdm_input_ints
	\see mdm_input_doubles
	\see mdm_input_string
	\see mdm_input_bool
	\see mdm_input_int
	\see mdm_input_double
	*/
  MDM_API mdm_Input(T_wrapped value, std::string k, std::string ks, std::string info) :
		value_(value),
		key_(k),
		key_short_(ks),
		combined_key_(k),
		info_(info)
	{
		if (!key_short_.empty())
		{
			combined_key_.append(",");
			combined_key_.append(key_short_);
		}
	}
	
	//! Return the option information text
	/*!
	\return information text
	*/
  MDM_API const char* info() const { return info_.c_str(); }

	//! Return the long-form version of the option key
	/*!
	\return long-form version of key (as C-string)
	*/
  MDM_API const char* key() const { return key_.c_str(); }

	//! Return the short-form version of the option key
	/*!
	\return short-form version of key (as C-string)
	*/
  MDM_API const char* key_short() const { return key_short_.c_str(); }

	//! Return the combined version of the option key
	/*!
	This is the long-form version, followed by a comma, then the short-form version, with no spaces
	\return combined version of key (as C-string)
	*/
  MDM_API const char* combined_key() const { return combined_key_.c_str(); }

	//! Return the value in its unwrapped form
	/*!
	To custom overload boost::program options, we wrap strings and vectors in container
	class. For example a string is stored in an mdm_input_str object. We overload the () operator 
	to return the value in its unwrapped form, so that an mdm_Input<mdm_input_str>() returns
	a std::string, not the wrapped mdm_input_str.

	\return unwrapped value of option, will be standard library string or vector object
	\see value
	*/
	MDM_API const T_unwrapped& operator() () const;

	//! Return a reference to the wrapped value
	/*!
	To custom overload boost::program options, we wrap strings and vectors in container
	class. For example a string is stored in an mdm_input_str object. Value returns a reference to the 
	wrapped object, so that an mdm_Input<mdm_input_str>::value() returns
	a (reference to a) mdm_input_str not the underlying std::string.

	\return wrapped value of option
	\see operator()
	*/
	MDM_API T_wrapped& value() { return value_; }

  //! Return a const reference to the wrapped value
  /*!
  \return read-only wrapped value of option
  */
  MDM_API const T_wrapped& value() const { return value_; }

	//! Set the option value
	/*!
	\param value this will be wrapped in the appropriate container class
	*/
  MDM_API void set(const T_unwrapped &value) { value_ = T_wrapped(value); }
	

private:
	T_wrapped value_;
	const std::string key_;
	const std::string key_short_;
	std::string combined_key_;
	const std::string info_;
};

//! Wrapper class for standard strings to pass to boost::program_options
class mdm_input_str {
public:
	//! Defines a placeholder to use for empty strings in config files
	static const std::string EMPTY_STR;

	//! Default constructor
  MDM_API mdm_input_str();

	//! Constructor from a standard string
	/*!
	\param str set string value
	*/
  MDM_API mdm_input_str(std::string str);

	//! Default destructor
  MDM_API ~mdm_input_str();

  //! Operator to return option value as a string
  MDM_API const std::string& operator() () const;

private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_str& s);
	std::string str_;
	
};

//! Wrapper class for integer vectors to pass to boost::program_options
class mdm_input_int_list {

public:
	//! Default constructor
  MDM_API mdm_input_int_list();

	//! Constructor from a standard integer vector
	/*!
	\param list set integer vector value
	*/
  MDM_API mdm_input_int_list(std::vector<int> list);

  //! Constructor from string
  /*!
  \param str string form used in config_file/cmd line option
  */
  MDM_API mdm_input_int_list(const std::string &str);

	//! Default destructor
  MDM_API ~mdm_input_int_list();

  //! Operator to return option value as vector of ints
  MDM_API const std::vector<int>& operator() () const;

  //!Set value from string
  /*!
  \param str string form used in config_file/cmd line option
  */
  MDM_API void fromString(const std::string &str);

  //!Return string format of list
  /*!
  \return string form used in config_file/cmd line option
  */
  MDM_API std::string toString() const;

private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_int_list& list)
  {
    os << list.toString();
    return os;
  }
	std::vector<int> list_;
};

//! Wrapper class for double vectors to pass to boost::program_options
class mdm_input_double_list {

public:
	//! Default constructor
  MDM_API mdm_input_double_list();

	//! Constructor from a standard double vector
	/*!
	\param list set double vector value
	*/
  MDM_API mdm_input_double_list(std::vector<double> list);

  //! Constructor from string
  /*!
  \param str string form used in config_file/cmd line option
  */
  MDM_API mdm_input_double_list(const std::string &str);

	//! Default destructor
  MDM_API ~mdm_input_double_list();
	
  //! Operator to return option value as vector of doubles
  MDM_API const std::vector<double>& operator() () const;
	
  //!Set value from string
  /*!
  \param str string form used in config_file/cmd line option
  */
  MDM_API void fromString(const std::string &str);

  //!Return string format of list
  /*!
  \return string form used in config_file/cmd line option
  */
  MDM_API std::string toString() const;

private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_double_list& list)
  {
    os << list.toString();
    return os;
  }
	std::vector<double> list_;
};

//! Wrapper class for string vectors to pass to boost::program_options
class mdm_input_string_list {

public:
	//! Default constructor
  MDM_API mdm_input_string_list();

	//! Constructor from a standard string vector
	/*!
	\param list set string vector value
	*/
  MDM_API mdm_input_string_list(std::vector<std::string> list);

  //! Constructor from string
  /*!
  \param str string form used in config_file/cmd line option
  */
  MDM_API mdm_input_string_list(const std::string &str);

	//! Default destructor
  MDM_API ~mdm_input_string_list();

  //! Operator to return option value as vector of strings
  MDM_API const std::vector<std::string>& operator() () const;
	
  //!Set value from string
  /*!
  \param str string form used in config_file/cmd line option
  */
  MDM_API void fromString(const std::string &str);

  //!Return string format of list
  /*!
  \return string form used in config_file/cmd line option
  */
  MDM_API std::string toString() const;

private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_string_list& list)
  {
    os << list.toString();
    return os;
  }
  std::vector<std::string> list_;
};

//! Input option for strings
typedef mdm_Input < mdm_input_string_list, std::vector<std::string> > mdm_input_strings;

//! Input option for integer lists
typedef mdm_Input < mdm_input_int_list, std::vector<int> > mdm_input_ints;

//! Input option for double lists
typedef mdm_Input < mdm_input_double_list, std::vector<double> > mdm_input_doubles;

//! Input option for string lists
typedef mdm_Input< mdm_input_str, std::string> mdm_input_string;

//! Input option for bools
typedef mdm_Input< bool, bool> mdm_input_bool;

//! Input option for ints
typedef mdm_Input< int, int> mdm_input_int;

//! Input option for doubles
typedef mdm_Input< double, double> mdm_input_double;

#endif
