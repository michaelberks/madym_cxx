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

#include "mdm_ProgramLogger.h"

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
	assert(sigs.size() >= minimumInputs());
	assert(sigs.size() <= maximumInputs());  /* Input arg */
	signals_ = sigs;
}

//****************************************************************
// Protected methods
//****************************************************************

//
void mdm_T1FitterBase::setErrorValuesAndTidyUp(const std::string msg, double &T1, double &M0)
{
	mdm_ProgramLogger::logProgramMessage(
		"WARNING: mdm_T1FitterBase::TfitT1:   " + msg + "\n");

	//Set default values for M0 and T1
	T1 = 0.0;
	M0 = 0.0;
}