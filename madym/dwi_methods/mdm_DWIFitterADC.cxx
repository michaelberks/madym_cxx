/**
*  @file    mdm_DWIFitterADC.cxx
*  @brief   Implementation of the mdm_DWIFitterADC class
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_DWIFitterADC.h"

#include <stdio.h>                
#include <stdlib.h>                
#include <cmath>            /* For cos(), sin() and exp() */
#include <cassert>          /* For assert macro */

#include <madym/mdm_ProgramLogger.h>
#include <madym/mdm_exception.h>

const double mdm_DWIFitterADC::PI = acos(-1.0);

//
MDM_API mdm_DWIFitterADC::mdm_DWIFitterADC(const std::vector<double>& B0s)
	:
	mdm_DWIFitterBase()
{
	if (!B0s.empty())
		initB0s();
}

//
MDM_API mdm_DWIFitterADC::~mdm_DWIFitterADC()
{

}

//
MDM_API void mdm_DWIFitterADC::setB0s(const std::vector<double>& B0s)
{
	B0s_ = B0s;
	initB0s();
}

//
MDM_API void mdm_DWIFitterADC::setInputs(const std::vector<double>& inputs)
{
	if (inputs.size() < minimumInputs())
		throw mdm_exception(__func__, "Fewer input signals (" + std::to_string(inputs.size()) +
			") than minimum required (" + std::to_string(minimumInputs()) + ")");

	if (inputs.size() > maximumInputs())
		throw mdm_exception(__func__, "More input signals (" + std::to_string(inputs.size()) +
			") than maximum allowed (" + std::to_string(maximumInputs()) + ")");

	//Inputs are just signals
	signals_ = inputs;
}

//
MDM_API mdm_ErrorTracker::ErrorCode mdm_DWIFitterADC::fitModel(
	std::vector<double>& params)
{
	return mdm_ErrorTracker::OK;
}

//
MDM_API bool mdm_DWIFitterADC::setInputsFromStream(std::istream& ifs,
	const int nSignals)
{
	B0s_.resize(nSignals);
	signals_.resize(nSignals);
	for (auto& b0 : B0s_)
	{
		ifs >> b0;
		if (ifs.eof())
			return false;

		b0 *= (PI / 180);
	}
	for (auto& si : signals_)
		ifs >> si;

	initB0s();
	return true;
}

//
MDM_API int mdm_DWIFitterADC::minimumInputs() const
{
	return 3;
}

//
MDM_API int mdm_DWIFitterADC::maximumInputs() const
{
	return 10;
}

//
MDM_API double mdm_DWIFitterADC::modeltoSignal(
	const std::vector<double>& params, double B0)
{
	return 0;
}

//**********************************************************************
//Private methods
//**********************************************************************

void mdm_DWIFitterADC::computeSignalGradient(const std::vector<double>& params,
	double& signal, double& signal_dT1, double& signal_dM0)
{
}

void mdm_DWIFitterADC::computeSSEGradient(
	const alglib::real_1d_array& x, double& func, alglib::real_1d_array& grad)
{
}

//
void mdm_DWIFitterADC::initB0s()
{
	nB0s_ = int(B0s_.size());
	if (nB0s_ < minimumInputs())
		throw mdm_exception(__func__, "Fewer B0s (" + std::to_string(nB0s_) +
			") than minimum required (" + std::to_string(minimumInputs()) + ")");

	if (nB0s_ > maximumInputs())
		throw mdm_exception(__func__, "More B0s (" + std::to_string(nB0s_) +
			") than maximum allowed (" + std::to_string(maximumInputs()) + ")");

}