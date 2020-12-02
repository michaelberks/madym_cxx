/**
*  @file    mdm_DCEModelAUEM.h
*  @brief Implements the active-uptake and efflux model.
*
*	 See git repository wiki for scientific discussion of models.
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODELAUEM_HDR
#define MDM_DCEMODELAUEM_HDR

#include "mdm_api.h"

#include "mdm_DCEModelBase.h"
#include <vector>
#include <mdm_AIF.h>
//! Implements the active-uptake and efflux model.
class mdm_DCEModelAUEM : public mdm_DCEModelBase {
public:
	
  //! AUEM model constructor
  MDM_API mdm_DCEModelAUEM(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

  MDM_API ~mdm_DCEModelAUEM();

  MDM_API virtual std::string modelType() const;

  MDM_API virtual void computeCtModel(size_t nTimes);

  MDM_API virtual void checkParams();

protected:

private:
  //METHODS:

  //VARIABLES

};

#endif //MDM_DCEMODELAUEM_HDR
