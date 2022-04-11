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

#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/utils/mdm_exception.h>

//
MDM_API mdm_T1FitterBase::mdm_T1FitterBase()
	: maxIterations_(500)
{
}

//
MDM_API mdm_T1FitterBase::~mdm_T1FitterBase()
{

}

//****************************************************************
// Protected methods
//****************************************************************

//
void mdm_T1FitterBase::setErrorValuesAndTidyUp(double &T1, double &M0)
{
	//Set default values for M0 and T1
	T1 = 0.0;
	M0 = 0.0;
}