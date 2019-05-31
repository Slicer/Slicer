#include <qMRMLLinearTransformSlider.h>
#include <QApplication>

// Slicer includes
#include "vtkSlicerConfigure.h"

// VTK includes
#include "qMRMLWidget.h"

// STD includes

int qMRMLLinearTransformSliderTest1( int argc , char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget widget;

  qMRMLLinearTransformSlider   mrmlItem( &widget );

  return EXIT_SUCCESS;
}
