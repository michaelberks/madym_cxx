/**
*  @file    mdm_DCEModelDIBEM.h
*  @brief Implements the generic dual-input bi-exponential model.
*
*	 See git repository wiki for scientific discussion of models.
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODELDIBEM_HDR
#define MDM_DCEMODELDIBEM_HDR

#include "mdm_api.h"

#include "mdm_DCEModelBase.h"

#include <vector>
#include <mdm_AIF.h>
//! Implements the generic dual-input bi-exponential model.
class mdm_DCEModelDIBEM : public mdm_DCEModelBase {
public:
	
  //! Dual-input bi-exponential model constructor
  MDM_API mdm_DCEModelDIBEM(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

  MDM_API ~mdm_DCEModelDIBEM();

  MDM_API virtual std::string modelType() const;

  MDM_API virtual void computeCtModel(size_t nTimes);

  MDM_API virtual void checkParams();

protected:

private:
  //METHODS:

  //VARIABLES

};

#endif //MDM_DCEMODELDIBEM_HDR
