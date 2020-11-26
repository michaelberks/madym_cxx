/**
*  @file    mdm_RunToolsDCEFite.cxx
*  @brief   Implementation of mdm_RunToolsDCEFit class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunToolsDCEFit.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/dce_models/mdm_DCEModelGenerator.h>
#include <madym/mdm_exception.h>

namespace fs = boost::filesystem;

//
MDM_API mdm_RunToolsDCEFit::mdm_RunToolsDCEFit(mdm_InputOptions &options, mdm_OptionsParser &options_parser)
	: mdm_RunTools(options, options_parser)
{
}


MDM_API mdm_RunToolsDCEFit::~mdm_RunToolsDCEFit()
{

}

//
void mdm_RunToolsDCEFit::setModel(const std::string &modelName,
	const std::vector<std::string> &paramNames,
	const std::vector<double> &initialParams,
	const std::vector<int> fixedParams,
	const std::vector<double> fixedValues,
	const std::vector<int> relativeLimitParams,
	const std::vector<double> relativeLimitValues)
{
	auto modelType = mdm_DCEModelGenerator::ParseModelName(modelName);
	if (modelType == mdm_DCEModelGenerator::UNDEFINED)
    throw mdm_exception(__func__, "Invalid or unsupported model (from command-line)");

  if (AIF_.AIFType() == mdm_AIF::AIF_TYPE::AIF_UNDEFINED)
    throw mdm_exception(__func__, "Tried to create model before AIF set");

	model_ = mdm_DCEModelGenerator::createModel(AIF_,
		modelType, paramNames,
		initialParams, fixedParams, fixedValues,
		relativeLimitParams, relativeLimitValues);
}

void mdm_RunToolsDCEFit::setAIF()
{
  //Parse input AIF options
  // - aifName overrides all
  // - aifMap overrides aifType
  // - warn if aifType not default but doesn't match aifName/Map if specified
  // - error if aifType set to aifFile/Map but aifName/Map not specified

  mdm_AIF::AIF_TYPE aifType = mdm_AIF::AIF_TYPE(options_.aifType());

  if (!options_.aifName().empty())
  {
    if (aifType != mdm_AIF::AIF_TYPE::AIF_FILE && aifType != mdm_AIF::AIF_TYPE::AIF_POP)
      mdm_ProgramLogger::logProgramWarning(__func__,
        "AIF name supplied but AIF type set to non-default mis-matched type. Using AIF from file\n"
      );
    aifType = mdm_AIF::AIF_TYPE::AIF_FILE;
  }
  else if (!options_.aifMap().empty())
  {
    if (aifType != mdm_AIF::AIF_TYPE::AIF_MAP && aifType != mdm_AIF::AIF_TYPE::AIF_POP)
      mdm_ProgramLogger::logProgramWarning(__func__,
        "AIF map supplied but AIF type set to non-default mis-matched type. Using AIF from map\n"
      );
    aifType = mdm_AIF::AIF_TYPE::AIF_MAP;
  }
  else if (aifType == mdm_AIF::AIF_TYPE::AIF_FILE && options_.aifName().empty())
    throw mdm_exception(__func__, "AIF type set to read from file but AIF name empty");

  else if (aifType == mdm_AIF::AIF_TYPE::AIF_MAP && options_.aifName().empty())
    throw mdm_exception(__func__, "AIF type set to read from map but AIF map empty");

  //Set AIF type, if user has given incorrect input this should trigger an exception
  AIF_.setAIFType(aifType);

  //PIF for now is simpler, if pifName is given, set to from file, otherwise population
  //We may look to change this behaviour to bring inline with AIF setting
  if (options_.pifName().empty())
    AIF_.setPIFType(mdm_AIF::PIF_POP);
  else
    AIF_.setPIFType(mdm_AIF::PIF_FILE);
}

