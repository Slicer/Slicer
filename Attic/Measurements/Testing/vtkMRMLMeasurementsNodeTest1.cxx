#include "vtkMRMLMeasurementsNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLMeasurementsNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLMeasurementsNode > node1 = vtkSmartPointer< vtkMRMLMeasurementsNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLMeasurementsNode, node1);
  
  TEST_SET_GET_BOOLEAN( node1, Visibility);
  
  return EXIT_SUCCESS;
}
