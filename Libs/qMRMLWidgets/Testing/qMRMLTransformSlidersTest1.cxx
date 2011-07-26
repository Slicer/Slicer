#include <qMRMLTransformSliders.h>
#include <QApplication>

// STD includes

int qMRMLTransformSlidersTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  QWidget widget;

  qMRMLTransformSliders   mrmlItem( &widget );

  return EXIT_SUCCESS;
}
