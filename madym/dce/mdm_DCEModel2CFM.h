/**
*  @file    mdm_DCEModel2CFM.h
*  @brief Implements the two-compartment exchange model.
*
*	 See git repository wiki for scientific discussion of models.
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODEL2CFM_HDR
#define MDM_DCEMODEL2CFM_HDR

#include <madym/utils/mdm_api.h>

#include "mdm_DCEModelBase.h"
#include <vector>
#include <madym/dce/mdm_AIF.h>
//! Implements the two-compartment exchange model.
class mdm_DCEModel2CFM : public mdm_DCEModelBase {
public:
	
  //! 2CFM model constructor
	MDM_API mdm_DCEModel2CFM(
    mdm_AIF &AIF,
    const std::vector<std::string> &paramNames = std::vector<std::string>(0),
    const std::vector<double> &initialParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
    const std::vector<double>& lowerBounds = std::vector<double>(0),
    const std::vector<double>& upperBounds = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

	MDM_API ~mdm_DCEModel2CFM();

	MDM_API virtual std::string modelType() const;

	MDM_API virtual void computeCtModel(size_t nTimes);

	MDM_API virtual void checkParams();

  MDM_API std::vector<double> makeLLSMatrix(const std::vector<double>& Ct_sig) const;

  MDM_API void transformLLSolution(const double* B);

protected:

private:
  //METHODS:

  //VARIABLES

};

#endif //MDM_DCEMODEL2CFM_HDR
