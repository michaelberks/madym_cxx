/**
*  @file    mdm_DCEModelGenerator.h
*  @brief Header only class to generate specific instances of DCE models
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_DCEMODELGENERATOR_HDR
#define MDM_DCEMODELGENERATOR_HDR

#include <memory>

#include "mdm_api.h"
#include <mdm_AIF.h>

#include <madym/dce_models/mdm_DCEModelBase.h>
#include <madym/dce_models/mdm_DCEModelETM.h>
#include <madym/dce_models/mdm_DCEModelDIETM.h>
#include <madym/dce_models/mdm_DCEModelAUEM.h>
#include <madym/dce_models/mdm_DCEModelDISCM.h>
#include <madym/dce_models/mdm_DCEModel2CXM.h>
#include <madym/dce_models/mdm_DCEModelDI2CXM.h>
#include <madym/dce_models/mdm_DCEModelDIBEM.h>
#include <madym/dce_models/mdm_DCEModelDIBEM_Fp.h>

//!Header only class to generate specific instances of DCE models
/*! 
	Provides factory-pattern method for instantiating an mdm_DCEModelBase pointer
	into one of the allowed sub-class model instances, based on a given model name

	Any new model implementations should be added to:
	1) a new enum entry in ModelTypes
	2) a new model name in implementedModels()
	3) a new if else case in ParseModelName matching the name to the enum type
	4) a new case in the switch statement of setModel
	
*/
class mdm_DCEModelGenerator {

public:

	//! Defined model types - 
	/* 
	*/
	enum ModelTypes {
		UNDEFINED,
		TOFTS,
		VPSTD,
		ETM,
		DIETM,
		AUEM,
		DISCM,
		CXM,
		DI2CXM,
		DIBEM,
		DIBEM_FP
	};

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

	//! Convert model name from string to type enum
	/*
	\param modelName name of model, must be a member of implementedModels
	\return enum model type, UNSPECIFIED if model name not recognised
	*/
	MDM_API static  ModelTypes ParseModelName(const std::string modelName)
	{
		if (modelName == "TOFTS")
			return TOFTS;

		else if (modelName == "VPSTD")
			return VPSTD;

		else if (modelName == "ETM")
			return ETM;

		else if (modelName == "DIETM")
			return DIETM;

		else if (modelName == "AUEM" || modelName == "GADOXETATE")
			return AUEM;

		else if (modelName == "DISCM" || modelName == "MATERNE")
			return DISCM;

		else if (modelName == "2CXM")
			return CXM;

		else if (modelName == "DI2CXM")
			return DI2CXM;

		else if (modelName == "DIBEM" || modelName == "DIIRF")
			return DIBEM;

		else if (modelName == "DIBEM_FP")
			DIBEM_FP;

		else
			return UNDEFINED;
	}

  /**
	* @brief Instantiates pointer to abstract model class into specific model instance
	* @details Factory-pattern static method to instantiate a DCE model of specific type.
	*	To add a new model, include the model header above, addthe model name to the implemented
	* models list and then add an "if (modelName==MY_MODEL)" clause to this method,
	* instantiating the new model and setting any AIF flags associated with the model.
	* Dual-input models should set both AIF and PIF flags, single inputs AIF only.
	*
	* @param model reference to base model pointer
	* @param AIF AIF associaed with model
	* @param modelName model name, must match one of implemented types
	* @param auto_aif flag if the AIF associated with model is auto-generated
	* @param auto_pif flag if the PIF associated with model is auto-generated
	* @param paramNames if non-empty, overrides default parameter names of model
	* @param initialParams if non-empty, overrides default initial parameter values of model
	* @param paramNames indices of any parameters to be fixed in the model
	* @param paramNames values associated with fixed parameters (if non-empty, overrides initial parameters)
	* @return bool true if model successfully instantiated, false otherwise
	*/
	MDM_API static std::shared_ptr<mdm_DCEModelBase> createModel( 
		mdm_AIF &AIF,
		ModelTypes modelType,
		bool auto_aif, 
		bool auto_pif,
    const std::vector<std::string> &paramNames,
    const std::vector<double> &initialParams,
    const std::vector<int> fixedParams,
    const std::vector<double> fixedValues,
		const std::vector<int> relativeLimitParams,
		const std::vector<double> relativeLimitValues)
  {
    switch (modelType)
		{
		case TOFTS:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);
			return std::make_shared<mdm_DCEModelETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);;
		}

		case VPSTD:
		{
			AIF.setAIFType(mdm_AIF::AIF_STD);
			return std::make_shared < mdm_DCEModelETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case ETM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			return std::make_shared < mdm_DCEModelETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DIETM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			if (auto_pif)
				AIF.setPIFType(mdm_AIF::PIF_FILE);
			else
				AIF.setPIFType(mdm_AIF::PIF_POP);

			return std::make_shared < mdm_DCEModelDIETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case AUEM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			if (auto_pif)
				AIF.setPIFType(mdm_AIF::PIF_FILE);
			else
				AIF.setPIFType(mdm_AIF::PIF_POP);

			return std::make_shared < mdm_DCEModelAUEM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DISCM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			if (auto_pif)
				AIF.setPIFType(mdm_AIF::PIF_FILE);
			else
				AIF.setPIFType(mdm_AIF::PIF_POP);

			return std::make_shared < mdm_DCEModelDISCM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case CXM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			return std::make_shared < mdm_DCEModel2CXM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);

		}

		case DI2CXM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			if (auto_pif)
				AIF.setPIFType(mdm_AIF::PIF_FILE);
			else
				AIF.setPIFType(mdm_AIF::PIF_POP);

			return std::make_shared < mdm_DCEModelDI2CXM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DIBEM:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			if (auto_pif)
				AIF.setPIFType(mdm_AIF::PIF_FILE);
			else
				AIF.setPIFType(mdm_AIF::PIF_POP);

			return std::make_shared < mdm_DCEModelDIBEM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DIBEM_FP:
		{
			if (auto_aif)
				AIF.setAIFType(mdm_AIF::AIF_FILE);
			else
				AIF.setAIFType(mdm_AIF::AIF_POP);

			if (auto_pif)
				AIF.setPIFType(mdm_AIF::PIF_FILE);
			else
				AIF.setPIFType(mdm_AIF::PIF_POP);

			return std::make_shared < mdm_DCEModelDIBEM_Fp>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);

		}
		default:
			abort();
		}
  }

};


#endif //MDM_DCEMODELGENERATOR_HDR
