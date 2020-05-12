/**
*  @file    mdm_DCEModelGenerator.h
*  @brief
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

/*switch (modelType_)
{
case MDM_TOFTS:   // Basic Tofts
break;
case MDM_VPTOFTS: // Tofts plus Vp
break;
case MDM_GADOXETATE: // Gadoxetate
break;
default:
std::cout << "Model type " << modelType_ << " not recognised" << std::endl;
}*/

#ifndef MDM_DCEMODELGENERATOR_HDR
#define MDM_DCEMODELGENERATOR_HDR

#include <assert.h>

#include "mdm_api.h"
#include <mdm_AIF.h>

#include <madym/mdm_DCEModelBase.h>
#include <madym/mdm_DCEModelETM.h>
#include <madym/mdm_DCEModelDIETM.h>
#include <madym/mdm_DCEModelAUEM.h>
#include <madym/mdm_DCEModelDISCM.h>
#include <madym/mdm_DCEModel2CXM.h>
#include <madym/mdm_DCEModelDI2CXM.h>
#include <madym/mdm_DCEModelDIBEM.h>
#include <madym/mdm_DCEModelDIBEM_Fp.h>

/**
	* @brief Header only class to generate specific instances of DCE models
	* @details Provides factory-pattern method for instantiating an mdm_DCEModelBase pointer
	*	 into one of the allowed sub-class model instances, based on a given model name
	*/
class mdm_DCEModelGenerator {

public:
  /**
  * @brief Returns list of implemented model names
	* @return List of implemented model names
	*/
	MDM_API static const std::vector<std::string> implementedModels()
  {
    return {
  "ETM",
  "DIETM",
  "DISCM",
  "2CXM",
  "DI2CXM",
	"AUEM",
  "DIBEM",
  "TOFTS",
	"VPSTD"
    };
  }

  /**
	* @brief Instantiates pointer to abstract model class into specific model instance
	* @details Factory-pattern static method to instantiate a DCE model of specific type.
	*	To add a new model, include the model header above, addthe model name to the implemented
	* models list and then add an "if (model_name==MY_MODEL)" clause to this method,
	* instantiating the new model and setting any AIF flags associated with the model.
	* Dual-input models should set both AIF and PIF flags, single inputs AIF only.
	*
	* @param model reference to base model pointer
	* @param AIF AIF associaed with model
	* @param model_name model name, must match one of implemented types
	* @param auto_aif flag if the AIF associated with model is auto-generated
	* @param auto_pif flag if the PIF associated with model is auto-generated
	* @param paramNames if non-empty, overrides default parameter names of model
	* @param initParams if non-empty, overrides default initial parameter values of model
	* @param paramNames indices of any parameters to be fixed in the model
	* @param paramNames values associated with fixed parameters (if non-empty, overrides initial parameters)
	* @return bool true if model successfully instantiated, false otherwise
	*/
	MDM_API static bool setModel(
    mdm_DCEModelBase * &model,
    mdm_AIF &AIF,
    const std::string &model_name, 
		bool auto_aif, 
		bool auto_pif,
    const std::vector<std::string> &paramNames,
    const std::vector<double> &initParams,
    const std::vector<int> fixedParams,
    const std::vector<double> fixedValues,
		const std::vector<int> relativeLimitParams,
		const std::vector<double> relativeLimitValues)
  {
    assert(!model_name.empty());

    if (model)
      delete model;
    model = NULL;

    if (model_name == "TOFTS")
    {
      model = new mdm_DCEModelETM(AIF, paramNames, initParams, 
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);

      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

    }

    else if (model_name == "VPSTD")
    {
      model = new mdm_DCEModelETM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      AIF.setAIFflag(mdm_AIF::AIF_STD);
    }

    else if (model_name == "ETM")
    {
      model = new mdm_DCEModelETM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

    }

    else if (model_name == "DIETM")
    {
      model = new mdm_DCEModelDIETM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

      if (auto_pif)
        AIF.setPIFflag(mdm_AIF::PIF_FILE);
      else
        AIF.setPIFflag(mdm_AIF::PIF_POP);

    }

    else if (model_name == "AUEM" || model_name == "GADOXETATE")
    {
      model = new mdm_DCEModelAUEM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

      if (auto_pif)
        AIF.setPIFflag(mdm_AIF::PIF_FILE);
      else
        AIF.setPIFflag(mdm_AIF::PIF_POP);

    }

    else if (model_name == "DISCM" || model_name == "MATERNE")
    {
      model = new mdm_DCEModelDISCM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

      if (auto_pif)
        AIF.setPIFflag(mdm_AIF::PIF_FILE);
      else
        AIF.setPIFflag(mdm_AIF::PIF_POP);

    }

    else if (model_name == "2CXM")
    {
      model = new mdm_DCEModel2CXM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

    }

    else if (model_name == "DI2CXM")
    {
      model = new mdm_DCEModelDI2CXM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

      if (auto_pif)
        AIF.setPIFflag(mdm_AIF::PIF_FILE);
      else
        AIF.setPIFflag(mdm_AIF::PIF_POP);

    }

    else if (model_name == "DIBEM" || model_name == "DIIRF")
    {
      model = new mdm_DCEModelDIBEM(AIF, paramNames, initParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
      if (auto_aif)
        AIF.setAIFflag(mdm_AIF::AIF_FILE);
      else
        AIF.setAIFflag(mdm_AIF::AIF_POP);

      if (auto_pif)
        AIF.setPIFflag(mdm_AIF::PIF_FILE);
      else
        AIF.setPIFflag(mdm_AIF::PIF_POP);

    }

		else if (model_name == "DIBEM_FP")
		{
		model = new mdm_DCEModelDIBEM_Fp(AIF, paramNames, initParams,
			fixedParams, fixedValues,
			relativeLimitParams, relativeLimitValues);
		if (auto_aif)
			AIF.setAIFflag(mdm_AIF::AIF_FILE);
		else
			AIF.setAIFflag(mdm_AIF::AIF_POP);

		if (auto_pif)
			AIF.setPIFflag(mdm_AIF::PIF_FILE);
		else
			AIF.setPIFflag(mdm_AIF::PIF_POP);

		}

    else
      return false;

    return true;


  }

};


#endif //MDM_DCEMODELGENERATOR_HDR
