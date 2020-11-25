/**
*  @file    mdm_T1FitterBase.cxx
*  @brief   Implementation of the mdm_T1FitterBase class

*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1FitterBase.h"

#include <cassert>

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

//
MDM_API mdm_T1FitterBase::mdm_T1FitterBase()
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
MDM_API mdm_T1FitterBase::~mdm_T1FitterBase()
{

}

//
MDM_API void mdm_T1FitterBase::setInputSignals(const std::vector<double> &sigs)
{
  if (sigs.size() < minimumInputs())
    throw mdm_exception(__func__, "Fewer input signals (" + std::to_string(sigs.size()) +
      ") than minimum required (" + std::to_string(minimumInputs()) +")");

  if (sigs.size() > maximumInputs())
    throw mdm_exception(__func__, "More input signals (" + std::to_string(sigs.size()) +
      ") than maximum allowed (" + std::to_string(maximumInputs()) + ")");

	signals_ = sigs;
}

//****************************************************************
// Protected methods
//****************************************************************

//
void mdm_T1FitterBase::setErrorValuesAndTidyUp(const std::string msg, double &T1, double &M0)
{
	mdm_ProgramLogger::logProgramWarning(__func__, msg);

	//Set default values for M0 and T1
	T1 = 0.0;
	M0 = 0.0;
}