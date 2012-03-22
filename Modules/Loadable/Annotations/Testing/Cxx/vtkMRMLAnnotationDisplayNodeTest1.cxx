#include "vtkMRMLAnnotationDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationDisplayNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLAnnotationDisplayNode > node1 = vtkSmartPointer< vtkMRMLAnnotationDisplayNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  node1->UpdateReferences();

  vtkSmartPointer< vtkMRMLAnnotationDisplayNode > node2 = vtkSmartPointer< vtkMRMLAnnotationDisplayNode >::New();

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

  TEST_SET_GET_BOOLEAN( node1, ModifiedSinceRead );
  TEST_SET_GET_BOOLEAN( node1, SaveWithScene );
  TEST_SET_GET_BOOLEAN( node1, AddToScene );
  TEST_SET_GET_BOOLEAN( node1, Selected );

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
