/**
*  @file    mdm_DCEModelDISCM.h
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODELDISCM_HDR
#define MDM_DCEMODELDISCM_HDR

#include "mdm_api.h"

#include "mdm_DCEModelBase.h"
#include <vector>
#include <mdm_AIF.h>

class mdm_DCEModelDISCM : public mdm_DCEModelBase {
public:
  MDM_API mdm_DCEModelDISCM(
    mdm_AIF &AIF,
    const std::vector<std::string> &pkParamNames = std::vector<std::string>(0),
    const std::vector<double> &pkInitParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0));

  MDM_API ~mdm_DCEModelDISCM();

  MDM_API virtual std::string modelType() const;

  MDM_API virtual void computeCtModel(int nTimes);

  MDM_API virtual double checkParams();

  MDM_API virtual void resetRerun();

protected:

private:
  //METHODS:

  //VARIABLES

};

#endif //MDM_DCEMODELDISCM_HDR
