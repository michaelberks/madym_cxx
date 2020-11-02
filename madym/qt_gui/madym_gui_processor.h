/*!
*  @file    madym_gui_processor.h
*  @brief   Class for GUI processing tasks that runs in separate thread to main GUI
*  @details Allows processing to proceed without blocking the main GUI. Not currently used.
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MADYM_GUI_PROCESSOR
#define MADYM_GUI_PROCESSOR

#include <QObject>

//!Class for GUI processing tasks that runs in separate thread to main GUI
class madym_gui_processor : public QObject
{
  Q_OBJECT

//  INTERFACE

public:
	madym_gui_processor();

  //! Start processing task. Not currently used.
  void start_processing();
  
  //! Check if currently processing
	/*!
	\return true if currently processing, false otherwise
	*/
  bool is_processing() const;

signals:

	//! QT signal sent when processing finished
	/*!
	*/
	void processing_finished();

public slots:

	//! QT slot to do some processing
	/*!
	*/
	void process_series();


//  IMPLEMENTATION

private: // Methods

  //! Emit a signal to stop processing.
  void stop_processing();

private: // Variables

	

};

#endif //MADYM_GUI_PROCESSOR