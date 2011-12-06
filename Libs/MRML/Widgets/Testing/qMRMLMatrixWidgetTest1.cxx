#include <qMRMLMatrixWidget.h>
#include <QApplication>

// STD includes

int qMRMLMatrixWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  QWidget widget;

  qMRMLMatrixWidget   mrmlItem( &widget );
  
  return EXIT_SUCCESS;
}
