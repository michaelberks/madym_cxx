/*!
*  @file    mdm_RunToolsT1Fit.h
*  @brief   Defines abstract class mdm_RunToolsT1Fit providing methods for analysis pipelines common to different T1 mapping tools
*  @details More info...
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MDM_RUNTOOLS_T1_FIT_HDR
#define MDM_RUNTOOLS_T1_FIT_HDR
#include "mdm_api.h"
#include <madym/mdm_RunTools.h>
#include <madym/t1_methods/mdm_T1MethodGenerator.h>

//! Abstract base class providing methods common to T1 mapping tools
/*!
 \see mdm_RunTools
*/
class mdm_RunToolsT1Fit : public virtual mdm_RunTools {

public:

		
	//! Default constructor
	MDM_API mdm_RunToolsT1Fit();
		
	//! Virtual destructor
	/*!
	*/
	MDM_API virtual ~mdm_RunToolsT1Fit();

protected:
	
	//! Check there are a valid number of signal inputs for a given T1 method
	/*!
  Throws mdm_exception if number of inputs not valid
	\param methodType T1 method
	\param numInputs number of input signals specified by user
	*/
	void checkNumInputs(mdm_T1MethodGenerator::T1Methods methodType, const int& numInputs);

	//Variables:

private:

};

#endif
