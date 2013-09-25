/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\main.cpp
 *
 * \brief	Implements the main function.
 */

#include <QApplication>
#include "mainwindow.h"

//! [0]
int main(int argc, char **argv)
{
	// initialize the QApplication object	
	QApplication a(argc, argv);

	// create the main window
	MainWindow mw;
	QDesktopWidget * desktop = QApplication::desktop();
	QSize size = mw.sizeHint();
	int w = size.width();
	int h = size.height();
	int screen_width = desktop->width();
	int screen_height = desktop->height();
	mw.setGeometry(screen_width/2 - w/2, screen_height/2 - h/2, w, h);
	// show the main window
	mw.show();

	// start the main program execution loop
	int result = a.exec();

	// cleanup
	return result;
}
//! [0]
