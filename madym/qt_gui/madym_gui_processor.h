#ifndef MADYM_GUI_PROCESSOR
#define MADYM_GUI_PROCESSOR

#include <QObject>
/*#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QVector>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iosfwd>*/

class madym_gui_processor : public QObject
{
  Q_OBJECT

//  INTERFACE

public:
	madym_gui_processor();

  //: Begin transferring images from the main queue to the save queue.
  void start_processing();
  
  //: Return true if there are still frames to process.
  bool is_processing() const;

signals:

	void processing_finished();

public slots:

  //: Pop a frame from the main queue, process it, and push it onto the save 
  //  queue.
	void process_series();


//  IMPLEMENTATION

private: // Methods

  //: Emit a signal to indicate that all frames have been processed.
  void stop_processing();

private: // Variables

	

};

#endif //MADYM_GUI_PROCESSOR