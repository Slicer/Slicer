#include <qMRMLMatrixWidget.h>
#include <QApplication>

// std includes
#include <stdlib.h>
#include <iostream>

int qMRMLMatrixWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  QWidget * widget = new QWidget;

  qMRMLMatrixWidget   mrmlItem( widget );
  
  delete widget;

  return EXIT_SUCCESS;
}
