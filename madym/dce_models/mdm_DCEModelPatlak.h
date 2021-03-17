/**
*  @file    mdm_DCEModelPatlak.h
*  @brief Implements the extended-Tofts model.
*
*	 See git repository wiki for scientific discussion of models.
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODELPATLAK_HDR
#define MDM_DCEMODELPATLAK_HDR

#include "mdm_api.h"
#include "mdm_DCEModelBase.h"

#include <vector>
#include <mdm_AIF.h>
//! Implements the extended-Tofts model.
class mdm_DCEModelPatlak : public mdm_DCEModelBase {
public:
	
  //! ETM model constructor
  MDM_API mdm_DCEModelPatlak(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

  MDM_API ~mdm_DCEModelPatlak();

  MDM_API virtual std::string modelType() const;

  MDM_API virtual void computeCtModel(size_t nTimes);

  MDM_API virtual void checkParams();

protected:

private:
  //METHODS:

  //VARIABLES
	
};

#endif //MDM_DCEMODELETM_HDR
