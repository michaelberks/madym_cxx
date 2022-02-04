/**
*  @file    mdm_DWIFitterBase.cxx
*  @brief   Implementation of the mdm_DWIFitterBase class

*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DWIFitterBase.h"

#include <cassert>

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

//
MDM_API mdm_DWIFitterBase::mdm_DWIFitterBase(
	const std::vector<double>& Bvals, const std::vector<std::string>& paramNames)
	:
	Bvals_(Bvals),
	Bvals_to_fit_(Bvals),
	paramNames_(paramNames),
	maxIterations_(500)
{ 
}

//
MDM_API mdm_DWIFitterBase::~mdm_DWIFitterBase()
{

}

//
MDM_API void mdm_DWIFitterBase::setBvals(const std::vector<double>& Bvals)
{
	//When we set main B-values, also update B-values to fit
	Bvals_ = Bvals;
	Bvals_to_fit_ = Bvals_;
}

//
MDM_API void mdm_DWIFitterBase::setSignals(const std::vector<double>& sigs)
{
	//When we set main signals, also update signals to fit
	signals_ = sigs;
	signals_to_fit_ = sigs;
}

//
MDM_API void mdm_DWIFitterBase::setBvalsToFit(const std::vector<double>& Bvals)
{
	Bvals_to_fit_ = Bvals;
}


//
MDM_API void mdm_DWIFitterBase::setSignalsToFit(const std::vector<double>& sigs)
{
	signals_to_fit_ = sigs;
}

MDM_API const std::vector<std::string> mdm_DWIFitterBase::paramNames() const
{
	return paramNames_;
}

//! Return number of parameters in model
/*
\return number of parameters in model
*/
MDM_API size_t mdm_DWIFitterBase::nParams() const
{
	return paramNames_.size();
}

//****************************************************************
// Protected methods
//****************************************************************

//
void mdm_DWIFitterBase::setErrorValuesAndTidyUp(std::vector<double> &params)
{
	for (auto& p : params)
		p = 0;
}