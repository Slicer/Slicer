#include <qMRMLLinearTransformSlider.h>
#include <QApplication>

// Slicer includes
#include "vtkSlicerConfigure.h"

// VTK includes
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
#include <QSurfaceFormat>
#include <QVTKOpenGLWidget.h>
#endif

// STD includes

int qMRMLLinearTransformSliderTest1( int argc , char * argv [] )
{
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
  // Set default surface format for QVTKOpenGLWidget
  QSurfaceFormat format = QVTKOpenGLWidget::defaultFormat();
  format.setSamples(0);
  QSurfaceFormat::setDefaultFormat(format);
#endif

  QApplication app( argc, argv );

  QWidget widget;

  qMRMLLinearTransformSlider   mrmlItem( &widget );

  return EXIT_SUCCESS;
}
