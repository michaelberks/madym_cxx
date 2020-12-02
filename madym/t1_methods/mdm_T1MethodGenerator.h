/**
*  @file    t1_methods/mdm_T1MethodGenerator.h
*  @brief Header only class to generate specific instances of DCE models
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_T1METHODGENERATOR_HDR
#define MDM_T1METHODGENERATOR_HDR

#include <memory>

#include "mdm_api.h"

#include <madym/mdm_Image3D.h>
#include <madym/mdm_InputOptions.h>

#include <madym/t1_methods/mdm_T1FitterBase.h>
#include <madym/t1_methods/mdm_T1FitterVFA.h>

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
class mdm_T1MethodGenerator {

public:

	//! Defined model types - 
	/* 
	*/
	enum T1Methods {
		UNDEFINED, ///> Method not recognised
		VFA, ///> Variable flip-angle method
		//IR ///> Inversion recovery method
	};

	/**
	* @brief Returns list of implemented model names
	* @return List of implemented model names
	*/
	MDM_API static const std::vector<std::string> implementedMethods()
	{
		return {
	"VFA"
		};
	}

	//! Convert T1 method string to enum
	/*
	\param method string name of T1 mapping method, must be a member of implementedMethods
	\return method enum T1 mapping method, UNSPECIFIED if method name not recognised
	*/
	MDM_API static T1Methods ParseMethodName(const std::string &method) {
		if (method == "VFA")
			return VFA;
		//else if (method == "IR")
		//	return IR;
		else
			abort();
	}

  //! Factory method for creating specific T1 mapping object given user specified method
	/*! 
	This overload is for use with volume analysis, and in addition to returning an object of
	the specified T1 method, configures the return object with meta-data (eg VFA values) required
	to run the method from the input signal images
	\param methodType enum code of specified T1 method
	\param inputImages signal input images
	\return shared pointer to T1 fitter using specified method
	*/
	MDM_API static std::unique_ptr<mdm_T1FitterBase> createFitter( 
		T1Methods methodType, const std::vector<mdm_Image3D> &inputImages)
  {
		const auto &nSignals = inputImages.size();

    switch (methodType)
		{
		case VFA:
		{
			auto T1Fitter = std::make_unique<mdm_T1FitterVFA>();
			std::vector<double> FAs;
			const auto PI = acos(-1.0);
			for (auto img : inputImages)
				FAs.push_back(img.info().flipAngle.value()  * PI / 180);

			//Get tr value from first FA image - assume same for all images?
			double TR = inputImages[0].info().TR.value();
			T1Fitter->setFixedScannerSettings({ TR });
			T1Fitter->setVariableScannerSettings(FAs);
			return T1Fitter;
		}

		default:
			abort();
		}
  }

	//! Factory method for creating specific T1 mapping object given user specified method
	/*!
	This overload is for use with lite analysis tools, and in addition to returning an object of
	the specified T1 method, configures the return object with meta-data (eg VFA values) required
	to run the method from the input options structure
	\param method enum code of specified T1 method
	\param options set by user to configure mapping tool
	\return shared pointer to T1 fitter using specified method
	*/
	MDM_API static std::unique_ptr<mdm_T1FitterBase> createFitter(T1Methods method,
		const mdm_InputOptions &options)
	{
		switch (method)
		{
		case VFA:
		{
			auto T1Fitter = std::make_unique<mdm_T1FitterVFA>();
			T1Fitter->setFixedScannerSettings({ options.TR() });
			return T1Fitter;
		}
		default:
			abort();
		}

	}

};


#endif //MDM_T1METHODGENERATOR_HDR
