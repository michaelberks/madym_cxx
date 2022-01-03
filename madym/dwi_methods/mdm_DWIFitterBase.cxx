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

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

//
MDM_API mdm_DWIFitterBase::mdm_DWIFitterBase()
	: maxIterations_(500)
{
	//Pre-initialise the alglib state
	alglib::real_1d_array x = "[1000,1000]";
	alglib::real_1d_array s = "[1,1]";
	double epsg = 0.00000001;
	double epsf = 0.0000;
	double epsx = 0.0001;
#if _DEBUG
	alglib::ae_int_t maxits = maxIterations_ > 100 ? 100 : maxIterations_;
#else
	alglib::ae_int_t maxits = maxIterations_;
#endif

	alglib::mincgcreate(x, state_);
	alglib::mincgsetcond(state_, epsg, epsf, epsx, maxits);
	alglib::mincgsetscale(state_, s);
}

//
MDM_API mdm_DWIFitterBase::~mdm_DWIFitterBase()
{

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