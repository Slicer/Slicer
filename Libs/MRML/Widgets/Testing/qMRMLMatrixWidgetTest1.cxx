
// Qt includes
#include <QApplication>

// Slicer includes
#include "vtkSlicerConfigure.h"

// MRML includes
#include <qMRMLMatrixWidget.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

int qMRMLMatrixWidgetTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget widget;

  qMRMLMatrixWidget   mrmlItem( &widget );

  vtkMRMLTransformNode* nullTransformNode = nullptr;

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
