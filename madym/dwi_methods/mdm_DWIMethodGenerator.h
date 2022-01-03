/**
*  @file    t1_methods/mdm_T1MethodGenerator.h
*  @brief Header only class to generate specific instances of DCE models
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_DWIMETHODGENERATOR_HDR
#define MDM_DWIMETHODGENERATOR_HDR

#include <memory>

#include "mdm_api.h"

#include <madym/mdm_Image3D.h>
#include <madym/mdm_InputOptions.h>
#include <madym/mdm_ProgramLogger.h>
#include <madym/dwi_methods/mdm_DWIFitterBase.h>
#include <madym/dwi_methods/mdm_DWIFitterADC.h>
#include <madym/dwi_methods/mdm_DWIFitterIVIM.h>

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
class mdm_DWIMethodGenerator {

public:

	//! Defined model types - 
	/* 
	*/
	enum DWIMethods {
		UNDEFINED, ///> Method not recognised
		ADC, ///> ADC
    IVIM ///> IVIM
	};

  //! Returns list of implemented model names
  /*!
	\return List of implemented model names
	*/
	MDM_API static const std::vector<std::string> implementedMethods()
	{
		return {
	    toString(ADC),
      toString(IVIM)
		};
	}

  //!Return string form of method
  /*
  \param method enum code of DWI method
  \return string form of method
  */
  MDM_API static std::string toString(DWIMethods methodType)
  {
    switch (methodType)
    {
    case ADC: return "ADC";
    case IVIM: return "IVIM";
    default:
      throw mdm_exception(__func__, "DWI method " + std::to_string(methodType) + " not valid");
    }
  }

	//! Convert DWI method string to enum
	/*
	\param method string name of DWI mapping method, must be a member of implementedMethods
	\return method enum DWI mapping method, UNSPECIFIED if method name not recognised
	*/
	MDM_API static DWIMethods parseMethodName(const std::string &method, bool B1Correction)
  {
    if (method == toString(ADC))
    {
      return ADC;
    }
			
    else if (method == toString(IVIM))
			return IVIM;

		else
			throw mdm_exception(__func__, "DWI method " + method + " not recognised");
	}

  //! Factory method for creating specific DWI mapping object given user specified method
	/*! 
	This overload is for use with volume analysis, and in addition to returning an object of
	the specified DWI method, configures the return object with meta-data (eg VFA values) required
	to run the method from the input signal images
	\param methodType enum code of specified DWI method
	\param inputImages signal input images
	\return shared pointer to DWI fitter using specified method
	*/
	MDM_API static std::unique_ptr<mdm_DWIFitterBase> createFitter( 
		DWIMethods methodType, const std::vector<mdm_Image3D> &inputImages)
  {
		const auto &nSignals = inputImages.size();
			
    switch (methodType)
		{
		case ADC:
		{
			std::vector<double> B0s;
			for (auto img : inputImages)
				B0s.push_back(img.info().B.value());

      return std::make_unique<mdm_DWIFitterADC>(B0s);
		}
    case IVIM:
    {
			std::vector<double> B0s;
			for (auto img : inputImages)
				B0s.push_back(img.info().B.value());

			return std::make_unique<mdm_DWIFitterIVIM>(B0s);
    }
		default:
      throw mdm_exception(__func__, "DWI method " + std::to_string(methodType) + " not valid");
		}
  }

	//! Factory method for creating specific DWI mapping object given user specified method
	/*!
	This overload is for use with lite analysis tools, and in addition to returning an object of
	the specified DWI method, configures the return object with meta-data (eg VFA values) required
	to run the method from the input options structure
	\param method enum code of specified DWI method
	\param options set by user to configure mapping tool
	\return shared pointer to DWI fitter using specified method
	*/
	MDM_API static std::unique_ptr<mdm_DWIFitterBase> createFitter(DWIMethods method,
		const mdm_InputOptions &options)
	{
		switch (method)
		{
		case ADC:
		{
      std::vector<double> empty;
      auto DWIFitter = std::make_unique<mdm_DWIFitterADC>(empty);
			return DWIFitter;
		}
    case IVIM:
    {
      std::vector<double> empty;
      auto DWIFitter = std::make_unique<mdm_DWIFitterIVIM>(empty);
      return DWIFitter;
    }
		default:
			abort();
		}

	}

};


#endif //MDM_DWIMETHODGENERATOR_HDR
