/**
*  @file    mdm_RunToolsT1Fit.cxx
*  @brief   Implementation of mdm_RunToolsT1Fit class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunToolsT1Fit.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsT1Fit::mdm_RunToolsT1Fit()
{
}


MDM_API mdm_RunToolsT1Fit::~mdm_RunToolsT1Fit()
{

}

//
void mdm_RunToolsT1Fit::checkNumInputs(mdm_T1MethodGenerator::T1Methods methodType, 
  const int& numInputs)
{
	//This is a bit rubbish - instantiating a whole new object just to get
	//some limits returned. But we want limits defined by the derived T1 method class,
	//and want to check these to parse user input before the actual fitting objects
	//get created.
	auto T1fitter = mdm_T1MethodGenerator::createFitter(methodType, options_);

	if (numInputs < T1fitter->minimumInputs())
    throw mdm_exception(__func__, "not enough signal inputs for T1 method " + options_.T1method());
	
	else if (numInputs > T1fitter->maximumInputs())
    throw mdm_exception(__func__, "too many signal inputs for T1 method " + options_.T1method());
}

