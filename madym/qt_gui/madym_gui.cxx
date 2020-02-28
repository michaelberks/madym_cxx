#include "madym_gui_ui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	madym_gui_ui main_window;
  main_window.show();

	return a.exec();
}
