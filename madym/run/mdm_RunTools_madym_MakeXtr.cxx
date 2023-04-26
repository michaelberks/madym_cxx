/**
*  @file    mdm_RunTools_madym_MakeXtr.cxx
*  @brief   Implementation of mdm_RunTools_madym_MakeXtr class
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2020
*/

#ifndef MDM_API_EXPORTS
#define MDM_API_EXPORTS
#endif // !MDM_API_EXPORTS

#include "mdm_RunTools_madym_MakeXtr.h"

#include <madym/image_io/meta/mdm_XtrFormat.h>
#include <madym/image_io/mdm_ImageIO.h>
#include <madym/utils/mdm_Image3D.h>
#include <madym/utils/mdm_SequenceNames.h>

#include <madym/utils/mdm_exception.h>

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <numeric>


namespace fs = boost::filesystem;

//
MDM_API mdm_RunTools_madym_MakeXtr::mdm_RunTools_madym_MakeXtr()
{
}


MDM_API mdm_RunTools_madym_MakeXtr::~mdm_RunTools_madym_MakeXtr()
{
}

//
MDM_API void mdm_RunTools_madym_MakeXtr::run()
{
  //Set curent working dir
  set_up_cwd();

  if (!options_.T1inputNames().empty())
    makeT1InputXtr();

  if (options_.nDyns() > 0 )
    makeDynamicXtr();

  if (!options_.DWIinputNames().empty())
    makeDWIXtr();

  if (options_.makeDyn())
    mdm_ProgramLogger::logProgramMessage((boost::format(
      "INFO: option %1% is now deprecated. Dynamic XTR files will be generated as long as %2% is > 0.") 
      % options_.makeDyn.key() % options_.nDyns.key()).str());

  if (options_.makeT1Inputs())
    mdm_ProgramLogger::logProgramMessage((boost::format(
      "INFO: option %1% is now deprecated. T1 XTR files will be generated as long as %2% is not empty.")
      % options_.makeT1Inputs.key() % options_.T1inputNames.key()).str());

  if (options_.makeDWIInputs())
    mdm_ProgramLogger::logProgramMessage((boost::format(
      "INFO: option %1% is now deprecated. DWI XTR files will be generated as long as %2% is not empty.")
      % options_.makeDWIInputs.key() % options_.DWIinputNames.key()).str());
  

  mdm_ProgramLogger::logProgramMessage("Finished processing!");
}

//
MDM_API int mdm_RunTools_madym_MakeXtr::parseInputs(int argc, const char *argv[])
{
  po::options_description cmdline_options("madym_MakeXtr options");
  po::options_description config_options("madym_MakeXtr config options");

  options_parser_.add_option(cmdline_options, options_.help);
  options_parser_.add_option(cmdline_options, options_.version);
  options_parser_.add_option(cmdline_options, options_.configFile);
  options_parser_.add_option(cmdline_options, options_.dataDir);

	//General output options_
  options_parser_.add_option(config_options, options_.dynDir);
  options_parser_.add_option(config_options, options_.dynName);
  options_parser_.add_option(config_options, options_.sequenceFormat);
  options_parser_.add_option(config_options, options_.sequenceStart);
  options_parser_.add_option(config_options, options_.sequenceStep);
  options_parser_.add_option(config_options, options_.T1inputNames);
  options_parser_.add_option(config_options, options_.T1Dir);
  options_parser_.add_option(config_options, options_.DWIinputNames);
  options_parser_.add_option(config_options, options_.DWIDir);
  options_parser_.add_option(config_options, options_.nDyns);
  options_parser_.add_option(config_options, options_.T1method);

  // XTR options
  options_parser_.add_option(config_options, options_.makeDyn);
  options_parser_.add_option(config_options, options_.makeT1Inputs);
  options_parser_.add_option(config_options, options_.dynTimesFile);
  options_parser_.add_option(config_options, options_.temporalResolution);
  options_parser_.add_option(config_options, options_.TR);
  options_parser_.add_option(config_options, options_.FA);
  options_parser_.add_option(config_options, options_.VFAs);
  options_parser_.add_option(config_options, options_.TIs);
  options_parser_.add_option(config_options, options_.Bvalues);

  return options_parser_.parseInputs(
    cmdline_options,
    config_options,
    options_.configFile(),
    who(),
    argc, argv);
}

MDM_API std::string mdm_RunTools_madym_MakeXtr::who() const
{
	return "madym_MakeXtr";
}

//*******************************************************************************
// Private:
//*******************************************************************************

//---------------------------------------------------------------------
void mdm_RunTools_madym_MakeXtr::readDynamicTimes()
{
  //Try and open the file and read in the times
  std::ifstream dynTimesStream(
    fs::absolute(options_.dynTimesFile()).string(), std::ios::in);

  if (!dynTimesStream.is_open())
  {
    throw mdm_exception(__func__, "error opening dynamic times file, Check it exists");
  }
  
  dynamicTimes_.clear();
  for (int i = 0; i < options_.nDyns(); i++)
  {
    double t;
    dynTimesStream >> t;
    dynamicTimes_.push_back(t);
  }
}

//---------------------------------------------------------------------
double mdm_RunTools_madym_MakeXtr::getDynamicTime(int dynNum)
{
  double secs;
  if (dynamicTimes_.size() == options_.nDyns())
    secs = 60 * dynamicTimes_[dynNum];
  else
    secs = dynNum * options_.temporalResolution();

  return mdm_Image3D::secsToTimestamp(secs);
  
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_MakeXtr::makeT1InputXtr()
{
  //Parse T1 method from string, will abort if method type not recognised
  auto methodType = mdm_T1MethodGenerator::parseMethodName(options_.T1method(), false);

  switch (methodType)
  {
  case mdm_T1MethodGenerator::VFA:; //fall through
  case mdm_T1MethodGenerator::VFA_B1:
    makeVFAXtr(); break;
  case mdm_T1MethodGenerator::IR: makeIRXtr(); break;
  default:
    throw mdm_exception(__func__, "T1 method not recognised");
  }
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_MakeXtr::makeVFAXtr()
{
  //Check required inputs are set
  auto nInputs = options_.T1inputNames().size();

  if (!nInputs)
    throw mdm_exception(__func__, "T1 input names must be set");

  if (nInputs != options_.VFAs().size())
    throw mdm_exception(__func__,
      boost::format("Number of elements in VFAs (%1%) does not match number input names (%2%)")
      % options_.VFAs().size() % nInputs);

  auto TR = options_.TR();

  if (!TR)
    throw mdm_exception(__func__, "TR must not be zero for VFA T1 mapping");

  for (size_t i_t1 = 0; i_t1 < nInputs; i_t1++)
  {
    auto T1path = fs::absolute(
      fs::path(options_.T1Dir()) / options_.T1inputNames()[i_t1]);
    T1path.replace_extension();

    auto FA = options_.VFAs()[i_t1];

    //Set up mean image
    mdm_Image3D img;
    img.info().flipAngle.setValue(FA);
    img.info().TR.setValue(TR);

    mdm_XtrFormat::writeAnalyzeXtr(T1path.string(), img, mdm_XtrFormat::NEW_XTR);
    mdm_ProgramLogger::logProgramMessage("Created T1 input XTR file " + T1path.string() + ".xtr");
  }
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_MakeXtr::makeIRXtr()
{
  //Check required inputs are set
  auto nInputs = options_.T1inputNames().size();

  if (!nInputs)
    throw mdm_exception(__func__, "T1 input names must be set");

  if (nInputs != options_.TIs().size())
    throw mdm_exception(__func__,
      boost::format("Number of elements in TIs (%1%) does not match number input names (%2%)")
      % options_.TIs().size() % nInputs);

  auto TR = options_.TR();
  auto FA = options_.FA();

  for (size_t i_t1 = 0; i_t1 < nInputs; i_t1++)
  {
    auto T1path = fs::absolute(
      fs::path(options_.T1Dir()) / options_.T1inputNames()[i_t1]);
    T1path.replace_extension();

    auto TI = options_.TIs()[i_t1];

    //Set up mean image
    mdm_Image3D img;
    img.info().flipAngle.setValue(FA);
    img.info().TR.setValue(TR);
    img.info().TI.setValue(TI);

    mdm_XtrFormat::writeAnalyzeXtr(T1path.string(), img, mdm_XtrFormat::NEW_XTR);
    mdm_ProgramLogger::logProgramMessage("Created T1 input XTR file " + T1path.string() + ".xtr");
  }
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_MakeXtr::makeDynamicXtr()
{
  //Check required inputs are set
  if (!options_.nDyns())
    throw mdm_exception(__func__, "nDyns must not be zero for dynamic series");

  if (!options_.dynTimesFile().empty())
    readDynamicTimes();

  else if (!options_.temporalResolution())
    throw mdm_exception(__func__, "Either temporalResolution or a dynamic times file must be set");

  //Get basename of dynamic volumes from input options
  fs::path dynPath = fs::absolute(fs::path(options_.dynDir()) / options_.dynName());
  std::string dynPrefix = dynPath.filename().string();
  std::string dynBasePath = dynPath.remove_filename().string();

  auto TR = options_.TR();
  auto FA = options_.FA();

  if (!TR)
    throw mdm_exception(__func__, "TR must not be zero for dynamic series");

  if (!FA)
    throw mdm_exception(__func__, "FA must not be zero for dynamic series");

  //Get write format from options
  for (int i_dyn = 0; i_dyn < options_.nDyns(); i_dyn++)
  {
    mdm_Image3D img;
    img.info().flipAngle.setValue(FA);
    img.info().TR.setValue(TR);

    double acquisitionTime = getDynamicTime(i_dyn);
    img.setTimeStampFromDoubleStr(acquisitionTime);


    auto outputName = mdm_SequenceNames::makeSequenceFilename(
      dynBasePath, dynPrefix, i_dyn + 1, options_.sequenceFormat(),
      options_.sequenceStart(), options_.sequenceStep());

    mdm_XtrFormat::writeAnalyzeXtr(outputName, img, mdm_XtrFormat::NEW_XTR);
    mdm_ProgramLogger::logProgramMessage("Created dynamic XTR file " + outputName + ".xtr");
  }
}

//---------------------------------------------------------------------
void mdm_RunTools_madym_MakeXtr::makeDWIXtr()
{
  //Check required inputs are set
  auto nInputs = options_.DWIinputNames().size();

  if (!nInputs)
    throw mdm_exception(__func__, "DWI input names must be set");

  if (nInputs != options_.Bvalues().size())
    throw mdm_exception(__func__,
      boost::format("Number of elements in Bvalues (%1%) does not match number input names (%2%)")
      % options_.Bvalues().size() % nInputs);

  //auto TR = options_.TR();
  //auto FA = options_.FA();

  for (size_t i_b = 0; i_b < nInputs; i_b++)
  {
    auto DWIpath = fs::absolute(
      fs::path(options_.DWIDir()) / options_.DWIinputNames()[i_b]);
    DWIpath.replace_extension();

    auto B = options_.Bvalues()[i_b];

    //Set up mean image
    mdm_Image3D img;
    img.info().B.setValue(B);

    //if (FA)
    //  img.info().flipAngle.setValue(FA);
    //if (TR)
    //  img.info().TR.setValue(TR);
    

    mdm_XtrFormat::writeAnalyzeXtr(DWIpath.string(), img, mdm_XtrFormat::NEW_XTR);
    mdm_ProgramLogger::logProgramMessage("Created DWI input XTR file " + DWIpath.string() + ".xtr");
  }
}