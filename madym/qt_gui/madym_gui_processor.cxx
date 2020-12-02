/*!
*  @file    madym_gui_processor.cxx
*  @brief   Implementation of madym_gui_processor class
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_processor.h"

#include <madym/mdm_ProgramLogger.h>
#include <madym/run/mdm_RunTools_madym_T1.h>
#include <madym/run/mdm_RunTools_madym_DCE.h>
#include <madym/run/mdm_RunTools_madym_AIF.h>

static const std::string GUI = "_GUI";

//  Class that pops an image from the processing queue, performs any
//  necessary processing on the frame, and places it on the save queue for
//  the saver to deal with.
 
//
// Public methods
//

madym_gui_processor::madym_gui_processor()
{
}

//
mdm_RunTools& madym_gui_processor::madym_exe()
{
  return *madym_exe_;
}

//
void madym_gui_processor::set_madym_exe(RunType type)
{
  switch (type)
  {
  case RunType::T1:
  {
    madym_exe_.reset(new mdm_RunTools_madym_T1());
    break;
  }
  case RunType::AIF:
  {
    madym_exe_.reset(new mdm_RunTools_madym_AIF());
    break;
  }
  case RunType::DCE:
  {
    madym_exe_.reset(new mdm_RunTools_madym_DCE());
    break;
  }
  }
}
 
//
// Public slots
//
//: Begin transferring images from the main queue to the save queue.
void madym_gui_processor::start_processing()
{
  //Make sure options config file is empty, because when we call parse args
  //we don't want to read a config file
  mdm_ProgramLogger::logProgramMessage(
    "******************************************************\n"
    "Starting " + madym_exe_->who() + "...\n"
    "******************************************************\n"
  );
  madym_exe_->options().configFile.set("");
  madym_exe_->parseInputs(madym_exe_->who() + GUI);

  //Run the tool
  int result = madym_exe_->run_catch();
  emit processing_finished(result);
}

//
// Private methods
//
