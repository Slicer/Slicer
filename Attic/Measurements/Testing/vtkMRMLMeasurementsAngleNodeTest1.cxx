#include "vtkMRMLMeasurementsAngleNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLMeasurementsAngleNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLMeasurementsAngleNode > node1 = vtkSmartPointer< vtkMRMLMeasurementsAngleNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLMeasurementsAngleNode, node1);
  
  

  TEST_SET_GET_BOOLEAN( node1, Visibility);
    

  TEST_SET_GET_STRING(node1, LabelFormat);
  TEST_SET_GET_STRING(node1, CurrentAngleAnnotation);

  TEST_SET_GET_BOOLEAN(node1, LabelVisibility);
  TEST_SET_GET_BOOLEAN(node1, Ray1Visibility);
  TEST_SET_GET_BOOLEAN(node1, Ray2Visibility);
  TEST_SET_GET_BOOLEAN(node1, ArcVisibility);
  
  TEST_SET_GET_STRING(node1, ModelID1);
  TEST_SET_GET_STRING(node1, ModelID2);
  TEST_SET_GET_STRING(node1, ModelIDCenter);

  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position1, 100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position1, -100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position2, 100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Position2, -100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, PositionCenter, 100.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, PositionCenter, -100.0);
 
  //TEST_SET_GET_DOUBLE(node1, Angle);
  std::cout << "Angle = " << node1->GetAngle() << std::endl;
  node1->UpdateCurrentAngleAnnotation();
  std::cout << "Angle annotation = " << node1->GetCurrentAngleAnnotation() << std::endl;
  
  TEST_SET_GET_INT(node1, Resolution, 10);
  TEST_SET_GET_INT_RANDOM(node1, Resolution, 100);

  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, PointColour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, Point2Colour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, PointCentreColour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, LineColour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, ArcColour, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, LabelTextColour, 1.0);
  

  // all have to be the same
  node1->SetLabelScale(1.2, 1.2, 1.2);
  double *lscale = node1->GetLabelScale();
  if (!lscale ||
      lscale[0] != 1.2 ||
      lscale[1] != 1.2 ||
      lscale[2] != 1.2)
    {
    std::cerr << "Error in GetLabelScale() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
