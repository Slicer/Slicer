
// Qt includes
#include <QApplication>

// Slicer includes
#include "vtkSlicerConfigure.h"

// MRML includes
#include <qMRMLMatrixWidget.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkNew.h>
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
#include <QSurfaceFormat>
#include <QVTKOpenGLWidget.h>
#endif

int qMRMLMatrixWidgetTest1( int argc, char * argv [] )
{
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
  // Set default surface format for QVTKOpenGLWidget
  QSurfaceFormat format = QVTKOpenGLWidget::defaultFormat();
  format.setSamples(0);
  QSurfaceFormat::setDefaultFormat(format);
#endif

  QApplication app(argc, argv);

  QWidget widget;

  qMRMLMatrixWidget   mrmlItem( &widget );

  vtkMRMLTransformNode* nullTransformNode = 0;

  {
    vtkMRMLTransformNode* expectedTransformNode = nullTransformNode;
    vtkMRMLTransformNode* currentTransformNode = mrmlItem.mrmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with mrmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  mrmlItem.setMRMLTransformNode(nullTransformNode);

  {
    vtkMRMLTransformNode* expectedTransformNode = nullTransformNode;
    vtkMRMLTransformNode* currentTransformNode = mrmlItem.mrmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with mrmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  vtkNew<vtkMRMLTransformNode> transformNode;
  mrmlItem.setMRMLTransformNode(transformNode.GetPointer());

  {
    vtkMRMLTransformNode* expectedTransformNode = transformNode.GetPointer();
    vtkMRMLTransformNode* currentTransformNode = mrmlItem.mrmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with mrmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  mrmlItem.setMRMLTransformNode(nullTransformNode);

  {
    vtkMRMLTransformNode* expectedTransformNode = nullTransformNode;
    vtkMRMLTransformNode* currentTransformNode = mrmlItem.mrmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with mrmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
      }
  }

  return EXIT_SUCCESS;
}
