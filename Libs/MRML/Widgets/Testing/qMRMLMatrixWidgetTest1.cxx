
// Qt includes
#include <QApplication>

// MRML includes
#include <qMRMLMatrixWidget.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkNew.h>

int qMRMLMatrixWidgetTest1( int argc, char * argv [] )
{
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
