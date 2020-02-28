/**
*  @file    mdm_vul_arg.h
*  @brief   Additional template methods for vul_arg, to allow passing numeric vectors as input from command line
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/
#ifndef MDM_VUL_ARG_HDR
#define MDM_VUL_ARG_HDR

#include <vul/vul_arg.h>
#include <iostream>

//Write our own vul_arg template to pass a list of strings (needed to pass parameter names at cmd line)
//We've just copied the code from the std::vector<double> template, only we don't need to do the string to
//double conversion in the parse function
//: std::vector<double>
template <> void settype(vul_arg<std::vector<std::string> > &argmt) { argmt.type_ = "double list"; }

template <> void print_value(std::ostream &s, vul_arg<std::vector<std::string> > const &argmt)
{
  for (unsigned int i = 0; i < argmt().size(); ++i)
    s << ' ' << argmt()[i];
}

template <> int parse(vul_arg<std::vector<std::string> >* argmt, char ** argv)
{
  if (!argv || !argv[0]) {
    // no input
    std::cerr << "vul_arg_parse: Expected a vector of strings, none is provided.\n";
    return -1;
  }

  int sucked = 0;
  // Defaults should be cleared when the user supplies a value
  argmt->value_.clear();
  char *current = argv[0];

  std::cout << current << std::endl;

  char *pch = strtok(current, ",");
  while (pch != NULL)
  {
    argmt->value_.push_back(pch);
    ++sucked;
    pch = strtok(NULL, ",");
  }
  return sucked > 0;
}

#endif //MDM_VUL_ARG_HDR
