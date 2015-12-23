#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationLineDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationLineDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_DOUBLE_RANGE(node1, LineThickness, -1.0, 10.0);

  // LabelPosition is clamped, so test manually
  TEST_SET_GET_DOUBLE(node1, LabelPosition, 0.0);
  TEST_SET_GET_DOUBLE(node1, LabelPosition, 0.01);
  TEST_SET_GET_DOUBLE(node1, LabelPosition, 0.5);
  TEST_SET_GET_DOUBLE(node1, LabelPosition, 0.99);
  TEST_SET_GET_DOUBLE(node1, LabelPosition, 1.0);

  TEST_SET_GET_BOOLEAN(node1, LabelVisibility);
  TEST_SET_GET_DOUBLE_RANGE(node1, TickSpacing, 0.0, 100.0);

  return EXIT_SUCCESS;
}
