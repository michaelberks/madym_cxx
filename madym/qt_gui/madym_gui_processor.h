/*!
*  @file    madym_gui_processor.h
*  @brief   Class for GUI processing tasks that runs in separate thread to main GUI
*  @details Allows processing to proceed without blocking the main GUI. Not currently used.
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#ifndef MADYM_GUI_PROCESSOR
#define MADYM_GUI_PROCESSOR

#include <QObject>

#include <mdm_RunTools.h>

//!Class for GUI processing tasks that runs in separate thread to main GUI
class madym_gui_processor : public QObject
{
  Q_OBJECT

//  INTERFACE

//!Enum defining the the type of tool to process
public:
  enum RunType {
    T1, //!< T1 mapping
    AIF, //!< AIF detection
    DCE //!< DCE tracer-kinetic model fitting
  };

	madym_gui_processor();

  //! Reference to set run tools options
  /*!
  \return options
  */
  mdm_RunTools& madym_exe();

  //! Set a new run tools object of the required sub-type
  /*!
  \param type of run tool required: T1, AIF or DCE
  */
  void set_madym_exe(RunType type);

signals:

	//! QT signal sent when processing finished
	/*!
	*/
	void processing_finished(int);

public slots:

	//! QT slot to do some processing
	/*!
	*/
	void start_processing();


//  IMPLEMENTATION

private: // Methods

private: // Variables
  //Run tools object, dynamically allocated in factory method
  std::unique_ptr<mdm_RunTools> madym_exe_;
	

};

#endif //MADYM_GUI_PROCESSOR