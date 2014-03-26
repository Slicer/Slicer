#include <qMRMLLinearTransformSlider.h>
#include <QApplication>

// STD includes

int qMRMLLinearTransformSliderTest1( int argc , char * argv [] )
{
  QApplication app( argc, argv );

  QWidget widget;

  qMRMLLinearTransformSlider   mrmlItem( &widget );

  return EXIT_SUCCESS;
}
