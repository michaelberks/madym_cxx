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
#include <madym/mdm_ProgramLogger.h>
#include <madym/t1_methods/mdm_T1FitterBase.h>
#include <madym/t1_methods/mdm_T1FitterVFA.h>
#include <madym/t1_methods/mdm_T1FitterIR.h>

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
    VFA_B1, ///> Variable flip-angle method B1 corrected
		IR ///> Inversion recovery method
	};

  //! Returns list of implemented model names
  /*!
	\return List of implemented model names
	*/
	MDM_API static const std::vector<std::string> implementedMethods()
	{
		return {
	    toString(VFA),
      toString(VFA_B1),
      toString(IR)
		};
	}

  //!Return string form of method
  /*
  \param method enum code of T1 method
  \return string form of method
  */
  MDM_API static std::string toString(T1Methods methodType)
  {
    switch (methodType)
    {
    case VFA: return "VFA";
    case VFA_B1: return "VFA_B1";
    case IR: return "IR";
    default:
      throw mdm_exception(__func__, "T1 method " + std::to_string(methodType) + " not valid");
    }
  }

	//! Convert T1 method string to enum
	/*
	\param method string name of T1 mapping method, must be a member of implementedMethods
	\return method enum T1 mapping method, UNSPECIFIED if method name not recognised
	*/
	MDM_API static T1Methods parseMethodName(const std::string &method, bool B1Correction)
  {
    if (method == toString(VFA))
    {
      if (B1Correction)
      {
        mdm_ProgramLogger::logProgramWarning(__func__,
          "T1 mapping method VFA selected, with B1 correction set to true. Using method VFA_B1 instead.");
        return VFA_B1;
      }
      return VFA;
    }
			
    if (method == toString(VFA_B1))
      return VFA_B1;
		
    else if (method == toString(IR))
			return IR;

		else
			throw mdm_exception(__func__, "T1 method " + method + " not recognised");
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
    const auto PI = acos(-1.0);
			
    switch (methodType)
		{
		case VFA:
		{
			std::vector<double> FAs;
			for (auto img : inputImages)
				FAs.push_back(img.info().flipAngle.value()  * PI / 180);

			//Get tr value from first FA image - assume same for all images?
			double TR = inputImages[0].info().TR.value();

      return std::make_unique<mdm_T1FitterVFA>(FAs, TR, false);
		}
    case VFA_B1:
    {
      std::vector<double> FAs;
      for (auto img : inputImages)
        FAs.push_back(img.info().flipAngle.value()  * PI / 180);

      //Get tr value from first FA image - assume same for all images?
      double TR = inputImages[0].info().TR.value();

      return std::make_unique<mdm_T1FitterVFA>(FAs, TR, true);
    }
    case IR:
    {
      std::vector<double> TIs;
      for (auto img : inputImages)
        TIs.push_back(img.info().TI.value());

      double TR = inputImages[0].info().TR.value();

      return std::make_unique<mdm_T1FitterIR>(TIs, TR);
    }
		default:
      throw mdm_exception(__func__, "T1 method " + std::to_string(methodType) + " not valid");
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
      std::vector<double> empty;
      auto T1Fitter = std::make_unique<mdm_T1FitterVFA>(empty, options.TR(), false);
			return T1Fitter;
		}
    case VFA_B1:
    {
      std::vector<double> empty;
      auto T1Fitter = std::make_unique<mdm_T1FitterVFA>(empty, options.TR(), true);
      return T1Fitter;
    }
    case IR:
    {
      std::vector<double> empty;
      auto T1Fitter = std::make_unique<mdm_T1FitterIR>(empty, options.TR());
      return T1Fitter;
    }
		default:
			abort();
		}

	}

};


#endif //MDM_T1METHODGENERATOR_HDR
