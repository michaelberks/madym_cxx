/**
 *  @file    mdm_exception.h
 *  @brief   Define custom exception(s) for use in mdm library

 *
 */

#ifndef MDM_EXCEPTION_H
#define MDM_EXCEPTION_H

#include <string>
#include <boost/format.hpp>

//! Custom exception class
/*
Provides simple subclass of std exception, that forces thrower to
include function name, and with option to append to what() message
*/
class mdm_exception : virtual public std::exception {

public:
  //! Constructor from standard string message
  /*!
  \param func name of throwing function
  \param msg exception message displayed by what()
  */
  mdm_exception(const char* func, const std::string &msg) noexcept 
    : err(func) 
  {
    err.append(": ");
    err.append(msg);
  }

  //! Overload to allow construction directly from boost::format object
  /*!
  \param func name of throwing function
  \param msg exception message displayed by what()
  */
  mdm_exception(const char* func, const boost::format &msg) noexcept 
    : mdm_exception(func, msg.str())
  {}

  //! Return the exception's message
  /*!
  \return exception message (will include throwing function name)
  */
  virtual const char* what() const noexcept
  {
    return err.c_str();
  }
  
  //! Append to message on newline
  /*!
  \param msg message to append
  */
  void append(const std::string &msg)
  {
    err.append("\n");
    err.append(msg.c_str());
  }

  //! Append to message on newline directly from boost::format object
  /*!
  \param msg message to append
  */
  void append(const boost::format &msg)
  {
   append(msg.str());
  }

protected:
  //! Exception message displayed by what()
  std::string err;

};

#endif /* MDM_EXCEPTION_H */
