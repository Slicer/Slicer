
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

int qMRMLMatrixWidgetTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  const QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget widget;

  qMRMLMatrixWidget mrmlItem(&widget);

  vtkMRMLTransformNode* const nullTransformNode = nullptr;

  {
    vtkMRMLTransformNode* const expectedTransformNode = nullTransformNode;
    vtkMRMLTransformNode* const currentTransformNode = mrmlItem.mrmlTransformNode();
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
    vtkMRMLTransformNode* const expectedTransformNode = nullTransformNode;
    vtkMRMLTransformNode* const currentTransformNode = mrmlItem.mrmlTransformNode();
    if (expectedTransformNode != currentTransformNode)
    {
      std::cerr << "Line " << __LINE__ << " - Problem with mrmlTransformNode()\n"
                << "  expectedTransformNode:" << expectedTransformNode << "\n"
                << "  currentTransformNode:" << currentTransformNode << std::endl;
      return EXIT_FAILURE;
    }
  }

  const vtkNew<vtkMRMLTransformNode> transformNode;
  mrmlItem.setMRMLTransformNode(transformNode.GetPointer());

  {
    vtkMRMLTransformNode* const expectedTransformNode = transformNode.GetPointer();
    vtkMRMLTransformNode* const currentTransformNode = mrmlItem.mrmlTransformNode();
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
    vtkMRMLTransformNode* const expectedTransformNode = nullTransformNode;
    vtkMRMLTransformNode* const currentTransformNode = mrmlItem.mrmlTransformNode();
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
