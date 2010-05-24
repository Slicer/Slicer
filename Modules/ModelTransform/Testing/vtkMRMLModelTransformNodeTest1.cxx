#include "vtkMRMLModelTransformNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLModelTransformNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLModelTransformNode > node1 = vtkSmartPointer< vtkMRMLModelTransformNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  EXERCISE_BASIC_MRML_METHODS(vtkMRMLModelTransformNode, node1);
  
  TEST_SET_GET_STRING(node1, InputModelID);
  TEST_SET_GET_STRING(node1, TransformNodeID);
  TEST_SET_GET_STRING(node1, OutputModelID);
  TEST_SET_GET_STRING(node1, NewModelName);

  TEST_SET_GET_INT_RANGE(node1, TransformNormals, 0, 1);

 
  return EXIT_SUCCESS;
}
