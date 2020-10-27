/**
*  @file    mdm_RunTools.h
*  @brief   Defines class mdm_RunTools and associated helper class mdm_ToolsOptions
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_INPUT_TYPES_HDR
#define MDM_INPUT_TYPES_HDR

#include "mdm_api.h"

#include <string>
#include <vector>

template <class T, class T_out>
class mdm_input {

public:
	

	mdm_input(T v, std::string k, std::string ks, std::string info) :
		value_(v),
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
	const char* info() const { return info_.c_str(); }
	const char* key() const { return key_.c_str(); }
	const char* key_short() const { return key_short_.c_str(); }
	const char* combined_key() const { return combined_key_.c_str(); }

	MDM_API const T_out& operator() () const;
	MDM_API T& value() { return value_; }
	void set(const T_out &v) { value_ = T(v); }
	

private:
	T value_;
	const std::string key_;
	const std::string key_short_;
	std::string combined_key_;
	const std::string info_;
};

class mdm_input_str {
public:
	static const std::string EMPTY_STR;
	mdm_input_str();
	
	mdm_input_str(std::string s);

	~mdm_input_str();

	const std::string& operator() () const;
	

private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_str& s);
	std::string str;
	
};

class mdm_input_int_list {

public:
	mdm_input_int_list();
	mdm_input_int_list(std::vector<int> l);
	~mdm_input_int_list();

	const std::vector<int>& operator() () const;

private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_int_list& s);
	std::vector<int> list;
};

class mdm_input_double_list {

public:
	mdm_input_double_list();
	mdm_input_double_list(std::vector<double> l);
	~mdm_input_double_list();
	
	const std::vector<double>& operator() () const;
	
private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_double_list& s);
	std::vector<double> list;
};

class mdm_input_string_list {

public:
	mdm_input_string_list();
	mdm_input_string_list(std::vector<std::string> l);
	~mdm_input_string_list();

	const std::vector<std::string>& operator() () const;
	
private:
	friend std::ostream & operator<<(std::ostream &os, const mdm_input_string_list& s);
	std::vector<std::string> list;
};

typedef mdm_input < mdm_input_string_list, std::vector<std::string> > mdm_input_strings;
typedef mdm_input < mdm_input_int_list, std::vector<int> > mdm_input_ints;
typedef mdm_input < mdm_input_double_list, std::vector<double> > mdm_input_doubles;
typedef mdm_input< mdm_input_str, std::string> mdm_input_string;
typedef mdm_input< bool, bool> mdm_input_bool;
typedef mdm_input< int, int> mdm_input_int;
typedef mdm_input< double, double> mdm_input_double;

#endif
