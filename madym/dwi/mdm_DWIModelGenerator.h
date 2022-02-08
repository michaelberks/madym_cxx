/**
*  @file    dwi/mdm_DWIModelGenerator.h
*  @brief Header only class to generate specific instances of DCE models
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_DWIMODELGENERATOR_HDR
#define MDM_DWIMODELGENERATOR_HDR

#include <memory>

#include <madym/utils/mdm_api.h>

#include <madym/utils/mdm_Image3D.h>
#include <madym/run/mdm_InputOptions.h>
#include <madym/utils/mdm_ProgramLogger.h>
#include <madym/dwi/mdm_DWIFitterBase.h>
#include <madym/dwi/mdm_DWIFitterADC.h>
#include <madym/dwi/mdm_DWIFitterIVIM.h>

//!Header only class to generate specific instances of DCE models
/*!
	Provides factory-pattern method for instantiating an mdm_DWIFitterBase pointer
	into one of the allowed sub-class model instances, based on a given model name

	Any new model implementations should be added to:
	1) a new enum entry in ModelTypes
	2) a new model name in models()
	3) a new if else case in ParseModelName matching the name to the enum type
	4) a new case in the switch statement of setModel

*/
class mdm_DWIModelGenerator {

public:

	//! Defined model types - 
	/*
	*/
	enum DWImodels {
		UNDEFINED, ///> Model not recognised
		ADC, ///> ADC
		ADC_linear, ///> ADC, using only a linear fit
		IVIM, ///> IVIM
		IVIM_simple, ///> IVIM simplified version with no D* term
	};

	//! Returns list of implemented model names
	/*!
	\return List of implemented model names
	*/
	MDM_API static const std::vector<std::string> models()
	{
		return {
			toString(ADC),
			toString(ADC_linear),
			toString(IVIM),
			toString(IVIM_simple),
		};
	}

	//!Return string form of model
	/*
	\param model enum code of DWI model
	\return string form of model
	*/
	MDM_API static std::string toString(DWImodels modelType)
	{
		switch (modelType)
		{
		case ADC: return "ADC";
		case ADC_linear: return "ADC-linear";
		case IVIM: return "IVIM";
		case IVIM_simple: return "IVIM-simple";
		default:
			throw mdm_exception(__func__, "DWI model " + std::to_string(modelType) + " not valid");
		}
	}

	//! Convert DWI model string to enum
	/*
	\param model string name of DWI mapping model, must be a member of models
	\return model enum DWI mapping model, UNSPECIFIED if model name not recognised
	*/
	MDM_API static DWImodels parseModelName(const std::string& model)
	{
		if (model == toString(ADC))
			return ADC;

		else if (model == toString(ADC_linear))
			return ADC_linear;

		else if (model == toString(IVIM))
			return IVIM;

		else if (model == toString(IVIM_simple))
			return IVIM_simple;

		else
			throw mdm_exception(__func__, "DWI model " + model + " not recognised");
	}

	//! Factory method for creating specific DWI mapping object given user specified model
	/*!
	This overload is for use with volume analysis, and in addition to returning an object of
	the specified DWI model, configures the return object with meta-data (eg B values) required
	to run the model from the input signal images
	\param modelType enum code of specified DWI model
	\param inputImages signal input images
	\return shared pointer to DWI fitter using specified model
	*/
	MDM_API static std::unique_ptr<mdm_DWIFitterBase> createFitter(
		DWImodels modelType, const std::vector<mdm_Image3D>& inputImages,
		const std::vector<double>& BvalsThresh)
	{
		const auto& nSignals = inputImages.size();

		switch (modelType)
		{
		case ADC:
		{
			std::vector<double> B0s;
			for (auto img : inputImages)
				B0s.push_back(img.info().B.value());

			return std::make_unique<mdm_DWIFitterADC>(B0s, false);
		}
		case ADC_linear:
		{
			std::vector<double> B0s;
			for (auto img : inputImages)
				B0s.push_back(img.info().B.value());

			return std::make_unique<mdm_DWIFitterADC>(B0s, true);
		}
		case IVIM:
		{
			std::vector<double> B0s;
			for (auto img : inputImages)
				B0s.push_back(img.info().B.value());

			return std::make_unique<mdm_DWIFitterIVIM>(B0s, true, BvalsThresh);
		}
		case IVIM_simple:
		{
			std::vector<double> B0s;
			for (auto img : inputImages)
				B0s.push_back(img.info().B.value());

			return std::make_unique<mdm_DWIFitterIVIM>(B0s, false, BvalsThresh);
		}
		default:
			throw mdm_exception(__func__, "DWI model " + std::to_string(modelType) + " not valid");
		}
	}

	//! Factory method for creating specific DWI mapping object given user specified model
	/*!
	This overload is for use with lite analysis tools, and in addition to returning an object of
	the specified DWI model, configures the return object with meta-data (eg B values) required
	to run the model from the input options structure
	\param model enum code of specified DWI model
	\param options set by user to configure mapping tool
	\return shared pointer to DWI fitter using specified model
	*/
	MDM_API static std::unique_ptr<mdm_DWIFitterBase> createFitter(DWImodels model, const std::vector<double> BValsThresh = {})
	{
		std::vector<double> empty;
		switch (model)
		{
		case ADC:
		{
			auto DWIFitter = std::make_unique<mdm_DWIFitterADC>(empty, false);
			return DWIFitter;
		}
		case ADC_linear:
		{
			auto DWIFitter = std::make_unique<mdm_DWIFitterADC>(empty, true);
			return DWIFitter;
		}
		case IVIM:
		{
			auto DWIFitter = std::make_unique<mdm_DWIFitterIVIM>(empty, true, BValsThresh);
			return DWIFitter;
		}
		case IVIM_simple:
		{
			auto DWIFitter = std::make_unique<mdm_DWIFitterIVIM>(empty, false, BValsThresh);
			return DWIFitter;
		}
		default:
			abort();
		}

	}

};


#endif //MDM_DWImodelGENERATOR_HDR
