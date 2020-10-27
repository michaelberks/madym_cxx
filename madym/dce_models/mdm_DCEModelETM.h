/**
*  @file    mdm_DCEModelETM.h
*  @brief Implements the extended-Tofts model.
*
*	 See git repository wiki for scientific discussion of models.
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODELETM_HDR
#define MDM_DCEMODELETM_HDR

#include "mdm_api.h"
#include "mdm_DCEModelBase.h"

#include <vector>
#include <mdm_AIF.h>
//! Implements the extended-Tofts model.
class mdm_DCEModelETM : public mdm_DCEModelBase {
public:
	
	/*!
	*/
  MDM_API mdm_DCEModelETM(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

  MDM_API ~mdm_DCEModelETM();

  MDM_API virtual std::string modelType() const;

  MDM_API virtual void computeCtModel(int nTimes);

  MDM_API virtual double checkParams();

  MDM_API virtual void resetRerun();

protected:

private:
  //METHODS:

  //VARIABLES
	const static int ETM_ERR_VEPLUSVPGT1;   /* Ve + Vp > 1.0                           - Binary bit 14 set  */
	const static int ETM_ERR_KEPINF;  /* Ktrans / Ve > MDM_KEPMAX                - Binary bit 15 set  */

	const static double ETM_KEPMAX;
};

#endif //MDM_DCEMODELETM_HDR
