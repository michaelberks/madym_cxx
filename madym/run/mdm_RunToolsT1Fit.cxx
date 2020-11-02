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

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsT1Fit::mdm_RunToolsT1Fit()
{
}


MDM_API mdm_RunToolsT1Fit::~mdm_RunToolsT1Fit()
{

}

//
mdm_T1MethodGenerator::T1Methods mdm_RunToolsT1Fit::parseMethod(const std::string &method)
{
	auto methodType = mdm_T1MethodGenerator::ParseMethodName(
		options_.T1method());

	if (methodType == mdm_T1MethodGenerator::UNDEFINED)
		mdm_progAbort("T1 method not recognised");

	return methodType;
}

//
bool mdm_RunToolsT1Fit::checkNumInputs(mdm_T1MethodGenerator::T1Methods methodType, const int& numInputs)
{
	//This is a bit rubbish - instantiating a whole new object just to get
	//some limits returned. But we want limits defined by the derived T1 method class,
	//but want to check these to parse user input before the actual fitting objects
	//get created.
	auto T1fitter = mdm_T1MethodGenerator::createFitter(methodType, options_);

	if (numInputs < T1fitter->minimumInputs())
	{
		mdm_progAbort("not enough variable flip angle file names");
	}
	else if (numInputs > T1fitter->maximumInputs())
	{
		mdm_progAbort("too many variable flip angle file names");
	}
	return true;
}

