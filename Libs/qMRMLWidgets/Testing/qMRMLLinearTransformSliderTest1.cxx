#include <qMRMLLinearTransformSlider.h>
#include <QApplication>

// std includes
#include <stdlib.h>
#include <iostream>

int qMRMLLinearTransformSliderTest1( int , char * [] )
{
  QWidget * widget = new QWidget;

  qMRMLLinearTransformSlider   mrmlItem( widget );
  
  delete widget;

  return EXIT_SUCCESS;
}
