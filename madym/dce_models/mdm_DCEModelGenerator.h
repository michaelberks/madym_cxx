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
#include <madym/dce_models/mdm_DCEModelNONE.h>
#include <madym/dce_models/mdm_DCEModelETM.h>
#include <madym/dce_models/mdm_DCEModelDIETM.h>
#include <madym/dce_models/mdm_DCEModelAUEM.h>
#include <madym/dce_models/mdm_DCEModelDISCM.h>
#include <madym/dce_models/mdm_DCEModel2CXM.h>
#include <madym/dce_models/mdm_DCEModelDI2CXM.h>
#include <madym/dce_models/mdm_DCEModelDIBEM.h>
#include <madym/dce_models/mdm_DCEModelDIBEM_Fp.h>
#include <madym/dce_models/mdm_DCEModelPatlak.h>

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
    NONE,
		TOFTS,
		ETM,
		DIETM,
		AUEM,
		DISCM,
		CXM,
		DI2CXM,
		DIBEM,
		DIBEM_FP,
		PATLAK
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
	"PATLAK",
	"TOFTS",
  "NONE"
		};
	}

	//! Convert model name from string to type enum
	/*
	\param modelName name of model, must be a member of implementedModels
	\return enum model type, UNSPECIFIED if model name not recognised
	*/
	MDM_API static  ModelTypes ParseModelName(const std::string modelName)
	{
    if (modelName == "NONE")
      return NONE;

		if (modelName == "TOFTS" || modelName == "TM")
			return TOFTS;

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
			return DIBEM_FP;

		else if (modelName == "PATLAK")
			return PATLAK;

		else
			return UNDEFINED;
	}

	//!Instantiates pointer to abstract model class into specific model instance
  /*!
	Factory-pattern static method to instantiate a DCE model of specific type.
	To add a new model, include the model header above, addthe model name to the implemented
	models list and then add an "if (modelName==MY_MODEL)" clause to this method,
	instantiating the new model and setting any AIF flags associated with the model.
	Dual-input models should set both AIF and PIF flags, single inputs AIF only.
	
	\param AIF AIF associated with model
	\param modelType specifies model type
	\param paramNames if non-empty, overrides default parameter names of model
	\param initialParams if non-empty, overrides default initial parameter values of model
	\param fixedParams indices of any parameters to be fixed in the model
	\param fixedValues values associated with fixed parameters (if non-empty, overrides initial parameters)
	\param relativeLimitParams  indices of parameters to which relative limits are applied  (default {})
	\param relativeLimitValues  values for relative limits (default {})
	\return shared pointer to new model object of specified type
	*/
	MDM_API static std::shared_ptr<mdm_DCEModelBase> createModel( 
		mdm_AIF &AIF,
		ModelTypes modelType,
		const std::vector<std::string> &paramNames,
    const std::vector<double> &initialParams,
    const std::vector<int> fixedParams,
    const std::vector<double> fixedValues,
		const std::vector<int> relativeLimitParams,
		const std::vector<double> relativeLimitValues)
  {
    switch (modelType)
		{
    case NONE:
    {
      return std::make_shared<mdm_DCEModelNONE>(AIF, 
        std::vector<std::string>(),
        std::vector<double>(),
        std::vector<int>(),
        std::vector<double>(),
        std::vector<int>(),
        std::vector<double>());
    }

		case TOFTS:
		{
			return std::make_shared<mdm_DCEModelETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);;
		}

		case ETM:
		{
			return std::make_shared < mdm_DCEModelETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DIETM:
		{
			return std::make_shared < mdm_DCEModelDIETM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case AUEM:
		{
			return std::make_shared < mdm_DCEModelAUEM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DISCM:
		{
			return std::make_shared < mdm_DCEModelDISCM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case CXM:
		{
			return std::make_shared < mdm_DCEModel2CXM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);

		}

		case DI2CXM:
		{
			return std::make_shared < mdm_DCEModelDI2CXM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DIBEM:
		{
			return std::make_shared < mdm_DCEModelDIBEM>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);
		}

		case DIBEM_FP:
		{
			return std::make_shared < mdm_DCEModelDIBEM_Fp>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);

		}

		case PATLAK:
		{
			return std::make_shared < mdm_DCEModelPatlak>(AIF, paramNames, initialParams,
				fixedParams, fixedValues,
				relativeLimitParams, relativeLimitValues);

		}
		default:
			abort();
		}
  }

};


#endif //MDM_DCEMODELGENERATOR_HDR
