#include <qMRMLTransformSliders.h>
#include <QApplication>

// std includes
#include <stdlib.h>
#include <iostream>

int qMRMLTransformSlidersTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  QWidget * widget = new QWidget;

  qMRMLTransformSliders   mrmlItem( widget );

  delete widget;
  
  return EXIT_SUCCESS;
}
