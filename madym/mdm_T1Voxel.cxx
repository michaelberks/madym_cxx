/**
 *  @file    mdm_T1Voxel.cxx
 *  @brief   Implementation of the mdm_T1Voxel class
 */
#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_T1Voxel.h"

#include <cassert>

#include "mdm_ProgramLogger.h"

const int mdm_T1Voxel::MINIMUM_INPUTS = 3;
const int mdm_T1Voxel::MAXIMUM_INPUTS = 10;

//
MDM_API mdm_T1Voxel::mdm_T1Voxel()
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
MDM_API mdm_T1Voxel::~mdm_T1Voxel()
{

}

//
MDM_API void mdm_T1Voxel::setInputSignals(const std::vector<double> &sigs)
{
	assert(sigs.size() >= MINIMUM_INPUTS);  /* Input arg */
	signals_ = sigs;
}

//****************************************************************
// Protected methods
//****************************************************************

//
void mdm_T1Voxel::setErrorValuesAndTidyUp(const std::string msg, double &T1, double &M0)
{
	mdm_ProgramLogger::logProgramMessage(
		"WARNING: mdm_T1Voxel::TfitT1:   " + msg + "\n");

	//Set default values for M0 and T1
	T1 = 0.0;
	M0 = 0.0;
}