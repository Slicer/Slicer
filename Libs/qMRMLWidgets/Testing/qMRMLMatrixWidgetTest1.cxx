#include <qMRMLMatrixWidget.h>
#include <QApplication>

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLMatrixWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  QWidget widget;

  qMRMLMatrixWidget   mrmlItem( &widget );
  
  return EXIT_SUCCESS;
}
