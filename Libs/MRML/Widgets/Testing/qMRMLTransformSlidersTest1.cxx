#include <qMRMLTransformSliders.h>
#include <QApplication>

// Slicer includes
#include "vtkSlicerConfigure.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkMRMLTransformNode.h>
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLTransformSlidersTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget widget;

  qMRMLTransformSliders   mrmlItem( &widget );

  vtkNew<vtkMRMLTransformNode> transformNode;
  mrmlItem.setMRMLTransformNode(transformNode.GetPointer());

  // Check if slider values are updated correctly (not truncated to current slider range)
  // when a matrix with large translation values is set
  double largeTranslation[3] = {100, -1500, 500};
  vtkNew<vtkMatrix4x4> largeTranslationMatrix;
  largeTranslationMatrix->SetElement(0,3, largeTranslation[0]);
  largeTranslationMatrix->SetElement(1,3, largeTranslation[1]);
  largeTranslationMatrix->SetElement(2,3, largeTranslation[2]);
  transformNode->SetMatrixTransformToParent(largeTranslationMatrix.GetPointer());
  // Check if the matrix values are still correct

  transformNode->GetMatrixTransformToParent(largeTranslationMatrix.GetPointer());
  if (largeTranslationMatrix->GetElement(0,3)!=largeTranslation[0]
  || largeTranslationMatrix->GetElement(1,3)!=largeTranslation[1]
  || largeTranslationMatrix->GetElement(2,3)!=largeTranslation[2])
    {
    std::cerr << "Failed to set large translation: expected ["
      << largeTranslation[0] << ", "
      << largeTranslation[1] << ", "
      << largeTranslation[2]
      << "],   found: [ "
      << largeTranslationMatrix->GetElement(0,3) << ", "
      << largeTranslationMatrix->GetElement(1,3) << ", "
      << largeTranslationMatrix->GetElement(2,3) << "]" << std::endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
