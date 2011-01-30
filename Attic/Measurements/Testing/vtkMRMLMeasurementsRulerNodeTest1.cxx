#include "vtkMRMLMeasurementsRulerNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLMeasurementsRulerNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLMeasurementsRulerNode > node1 = vtkSmartPointer< vtkMRMLMeasurementsRulerNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLMeasurementsRulerNode, node1);

  TEST_SET_GET_STRING(node1, DistanceAnnotationFormat);
  TEST_SET_GET_STRING(node1, CurrentDistanceAnnotation);

  TEST_SET_GET_BOOLEAN(node1, DistanceAnnotationVisibility);

  TEST_SET_GET_STRING(node1, ModelID1);
  TEST_SET_GET_STRING(node1, ModelID2);

  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position1, 100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position1, -100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position2, 100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position2, -100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, PointColour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Point2Colour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, LineColour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, DistanceAnnotationTextColour, 1.0);

//  TEST_SET_GET_DOUBLE(node1, Distance);
  std::cout << "Distance = " << node1->GetDistance() << std::endl;
  TEST_SET_GET_INT(node1, Resolution, 10);
  TEST_SET_GET_INT(node1, Resolution, -10);

  // all have to be the same
  node1->SetDistanceAnnotationScale(1.2, 1.2, 1.2);
  double *ascale = node1->GetDistanceAnnotationScale();
  if (!ascale ||
      ascale[0] != 1.2 ||
      ascale[1] != 1.2 ||
      ascale[2] != 1.2)
    {
    std::cerr << "Error in GetDistanceAnnotationScale() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
