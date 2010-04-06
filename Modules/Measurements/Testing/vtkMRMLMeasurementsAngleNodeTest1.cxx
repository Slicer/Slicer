#include "vtkMRMLMeasurementsAngleNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLMeasurementsAngleNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLMeasurementsAngleNode > node1 = vtkSmartPointer< vtkMRMLMeasurementsAngleNode >::New();
 EXERCISE_BASIC_OBJECT_METHODS( node1 );

  node1->UpdateReferences();

  vtkSmartPointer< vtkMRMLMeasurementsAngleNode > node2 = vtkSmartPointer< vtkMRMLMeasurementsAngleNode >::New();

  node2->Copy( node1 );

  node2->Reset();

  node2->StartModify();

  std::string nodeTagName = node1->GetNodeTagName();

  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  std::string attributeName;
  std::string attributeValue;

  node1->SetAttribute( attributeName.c_str(), attributeValue.c_str() );
  std::string attributeValue2 = node1->GetAttribute( attributeName.c_str() );

  if( attributeValue != attributeValue2 )
    {
    std::cerr << "Error in Set/GetAttribute() " << std::endl;
    return EXIT_FAILURE;
    }
  
  TEST_SET_GET_BOOLEAN( node1, HideFromEditors );
  TEST_SET_GET_BOOLEAN( node1, Selectable );

  TEST_SET_GET_STRING( node1, Description );
  TEST_SET_GET_STRING( node1, SceneRootDir );
  TEST_SET_GET_STRING( node1, Name );
  TEST_SET_GET_STRING( node1, SingletonTag );

  TEST_SET_GET_BOOLEAN( node1, Visibility);
  
  TEST_SET_GET_BOOLEAN( node1, ModifiedSinceRead );
  TEST_SET_GET_BOOLEAN( node1, SaveWithScene );
  TEST_SET_GET_BOOLEAN( node1, AddToScene );
  TEST_SET_GET_BOOLEAN( node1, Selected );

  TEST_SET_GET_STRING(node1, LabelFormat);
  TEST_SET_GET_STRING(node1, CurrentAngleAnnotation);

  TEST_SET_GET_BOOLEAN(node1, LabelVisibility);
  TEST_SET_GET_BOOLEAN(node1, Ray1Visibility);
  TEST_SET_GET_BOOLEAN(node1, Ray2Visibility);
  TEST_SET_GET_BOOLEAN(node1, ArcVisibility);
  
  TEST_SET_GET_STRING(node1, ModelID1);
  TEST_SET_GET_STRING(node1, ModelID2);
  TEST_SET_GET_STRING(node1, ModelIDCenter);
  
  // test the doubles and ints
  node1->SetPosition1(10, 11, 12);
  double *p = node1->GetPosition1();
  if (p == NULL ||
      p[0] != 10 ||
      p[1] != 11 ||
      p[2] != 12)
    {
    std::cerr << "Error in GetPosition1() " << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetPosition2(13, 14, 15);
  p = node1->GetPosition2();
  if (p == NULL ||
      p[0] != 13 ||
      p[1] != 14 ||
      p[2] != 15)
    {
    std::cerr << "Error in GetPosition2() " << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetPositionCenter(16, 17, 18);
  p = node1->GetPositionCenter();
  if (p == NULL ||
      p[0] != 16 ||
      p[1] != 17 ||
      p[2] != 18)
    {
    std::cerr << "Error in GetPositionCenter() " << std::endl;
    return EXIT_FAILURE;
    }
  
  double angle = node1->GetAngle();
  std::cout << "Angle = " << angle << std::endl;
  
  node1->SetResolution(10);
  if (node1->GetResolution() != 10)
    {
    std::cerr << "Error in GetResolution() " << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetPointColour(1.0, 0.5555, 0.222);
  double *c = node1->GetPointColour();
  if (c == NULL ||
      c[0] != 1.0 ||
      c[1] != 0.5555 ||
      c[2] != 0.222)
    {
    std::cerr << "Error in GetPointColour() " << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetPoint2Colour(0.9, 0.666, 0.888);
  c = node1->GetPoint2Colour();
  if (c == NULL ||
      c[0] != 0.9 ||
      c[1] != 0.666 ||
      c[2] != 0.888)
    {
    std::cerr << "Error in GetPoint2Colour() " << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetPointCentreColour(0.9, 0.666, 0.888);
  c = node1->GetPointCentreColour();
  if (c == NULL ||
      c[0] != 0.9 ||
      c[1] != 0.666 ||
      c[2] != 0.888)
    {
    std::cerr << "Error in GetPointCentreColour() " << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetLineColour(0.333, 0.444, 0.555);
  c = node1->GetLineColour();
  if (c == NULL ||
      c[0] != 0.333 ||
      c[1] != 0.444 ||
      c[2] != 0.555)
    {
    std::cerr << "Error in GetLineColour() " << std::endl;
    return EXIT_FAILURE;
    }
  
  node1->SetArcColour(0.333, 0.444, 0.555);
  c = node1->GetArcColour();
  if (c == NULL ||
      c[0] != 0.333 ||
      c[1] != 0.444 ||
      c[2] != 0.555)
    {
    std::cerr << "Error in GetArcColour() " << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetLabelTextColour(0.333, 0.444, 0.555);
  c = node1->GetLabelTextColour();
  if (c == NULL ||
      c[0] != 0.333 ||
      c[1] != 0.444 ||
      c[2] != 0.555)
    {
    std::cerr << "Error in  GetLabelTextColour() " << std::endl;
    return EXIT_FAILURE;
    }

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

  node1->Modified();
  vtkMRMLScene * scene = node1->GetScene();

  if( scene != NULL )
    {
    std::cerr << "Error in GetScene() " << std::endl;
    return EXIT_FAILURE;
    }

  std::string stringToEncode = "Thou Shall Test !";
  std::string stringURLEncoded = node1->URLEncodeString( stringToEncode.c_str() );

  std::string stringDecoded = node1->URLDecodeString( stringURLEncoded.c_str() );

  if( stringDecoded != stringToEncode )
    {
    std::cerr << "Error in URLEncodeString/URLDecodeString() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
