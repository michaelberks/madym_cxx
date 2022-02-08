/**
*  @file    mdm_DCEModelDI2CXM.h
*  @brief Implements a dual-input two-compartment exchange model.
*
*	 See git repository wiki for scientific discussion of models.
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODELDI2CXM_HDR
#define MDM_DCEMODELDI2CXM_HDR

#include <madym/utils/mdm_api.h>

#include "mdm_DCEModelBase.h"
#include <vector>
#include <madym/dce/mdm_AIF.h>
//! Implements a dual-input two-compartment exchange model.
class mdm_DCEModelDI2CXM : public mdm_DCEModelBase {
public:
	
  //! Dual-input 2CXM model constructor
  MDM_API mdm_DCEModelDI2CXM(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
    const std::vector<double>& lowerBounds = std::vector<double>(0),
    const std::vector<double>& upperBounds = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

  MDM_API ~mdm_DCEModelDI2CXM();

  MDM_API virtual std::string modelType() const;

  MDM_API virtual void computeCtModel(size_t nTimes);

  MDM_API virtual void checkParams();

protected:

private:
  //METHODS:

  //VARIABLES

};

#endif //MDM_DCEMODELDI2CXM_HDR
