/*!
*  @file    madym_gui_processor.cxx
*  @brief   Implementation of madym_gui_processor class
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_processor.h"

//  Class that pops an image from the processing queue, performs any
//  necessary processing on the frame, and places it on the save queue for
//  the saver to deal with.
 
//
// Public methods
//

madym_gui_processor::madym_gui_processor()
{
}
 
//: Begin transferring images from the main queue to the save queue.
void madym_gui_processor::start_processing( )
{
  ;
}

//: Return true if there are still frames to process.
bool madym_gui_processor::is_processing( ) const
{
  return false;
}
 
//
// Public slots
//
 
//: Pop a frame from the main queue, process it, and push it onto the save 
//  queue.
void madym_gui_processor::process_series()
{
  // Check if we're waiting to process more frames.
	if (is_processing())
	{
		stop_processing();
	}	
}

//
// Private methods
//
 
//: Emit a signal to indicate that all frames have been processed.
void madym_gui_processor::stop_processing()
{
		emit processing_finished();
}