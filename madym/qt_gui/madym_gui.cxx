/*!
*  @file    madym_gui.cxx
*  @brief   Main executable for madym GUI
*  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
*/

#include "madym_gui_ui.h"

#include <QApplication>

//!Launch the madym GUI
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	madym_gui_ui main_window;
  main_window.show();

	return a.exec();
}
