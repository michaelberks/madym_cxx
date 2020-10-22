/**
*  @file    mdm_DCEModel2CXM.h
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/
#ifndef MDM_DCEMODEL2CXM_HDR
#define MDM_DCEMODEL2CXM_HDR

#include "mdm_api.h"

#include "mdm_DCEModelBase.h"
#include <vector>
#include <mdm_AIF.h>

class mdm_DCEModel2CXM : public mdm_DCEModelBase {
public:
  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API mdm_DCEModel2CXM(
    mdm_AIF &AIF,
    const std::vector<std::string> &pkParamNames = std::vector<std::string>(0),
    const std::vector<double> &pkInitParams = std::vector<double>(0),
    const std::vector<int> &fixedParams = std::vector<int>(0),
    const std::vector<double> &fixedValues = std::vector<double>(0),
		const std::vector<int> &relativeLimitParams = std::vector<int>(0),
		const std::vector<double> &relativeLimitValues = std::vector<double>(0));

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API ~mdm_DCEModel2CXM();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual std::string modelType() const;

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void computeCtModel(int nTimes);

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual double checkParams();

  	/**
	* @brief

	* @param
	* @return
	*/
	MDM_API virtual void resetRerun();

protected:

private:
  //METHODS:

  //VARIABLES

};

#endif //MDM_DCEMODEL2CXM_HDR
