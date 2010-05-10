/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __TestingMacros_h
#define __TestingMacros_h

#include "vtkDebugLeaks.h"
#include "vtkSmartPointer.h"

#define DEBUG_LEAKS_ENABLE_EXIT_ERROR() \
  vtkDebugLeaks::SetExitError(true);

#define EXERCISE_BASIC_OBJECT_METHODS( object ) \
    { \
    if ( object == NULL ) \
      { \
      std::cerr << "EXERCISE_BASIC_OBJECT_METHODS( with NULL object )" << std::endl;  \
      return EXIT_FAILURE;  \
      } \
    object->Print( std::cout );  \
    std::cout << "Name of Class = " << object->GetClassName() << std::endl; \
    std::cout << "Name of Superclass = " << object->Superclass::GetClassName() << std::endl; \
    }

#define TRY_EXPECT_ITK_EXCEPTION( command ) \
  try \
    {  \
    std::cout << "Trying " << #command << std::endl; \
    command;  \
    std::cerr << "Failed to catch expected exception" << std::endl;  \
    return EXIT_FAILURE;  \
    }  \
  catch( itk::ExceptionObject & excp )  \
    {  \
    std::cout << "Caught expected exception" << std::endl;  \
    std::cout << excp << std::endl; \
    }  


#define TRY_EXPECT_NO_ITK_EXCEPTION( command ) \
  try \
    {  \
    std::cout << "Trying " << #command << std::endl; \
    command;  \
    }  \
  catch( itk::ExceptionObject & excp )  \
    {  \
    std::cerr << excp << std::endl; \
    return EXIT_FAILURE;  \
    }  


#define TEST_ITK_SET_GET( variable, command ) \
  if( variable.GetPointer() != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable.GetPointer() << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }


#define TEST_ITK_SET_GET_VALUE( variable, command ) \
  if( variable != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_BOOLEAN( object, variable ) \
  object->Set##variable( false ); \
  object->Set##variable( true ); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->Set##variable( false ); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##On(); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in On/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##Off(); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Off/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_INT( object, variable ) \
  object->Set##variable( 2 ); \
  object->Set##variable( -2 ); \
  if( object->Get##variable() != -2 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } 

#define TEST_SET_GET_DOUBLE( object, variable ) \
  object->Set##variable( 2.0l ); \
  object->Set##variable( -2.0l ); \
  if( object->Get##variable() != -2.0l ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    }

#define TEST_SET_GET_VECTOR3_DOUBLE( object, variable ) \
  {                                                     \
  object->Set##variable( 1.0, 2.0, 3.0 );               \
  double *val = object->Get##variable();                \
  if( val == NULL || val[0] != 1.0 || val[1] != 2.0 || val[2] != 3.0 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  }

#define TEST_SET_GET_STRING( object, variable ) \
  { \
  const char * originalStringPointer = object->Get##variable(); \
  std::string originalString; \
  if( originalStringPointer != NULL ) \
    { \
    originalString = originalStringPointer; \
    } \
  std::string string1 = "testingIsGood"; \
  object->Set##variable( string1.c_str() ); \
  if( object->Get##variable() != string1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  std::string string2 = "moreTestingIsBetter"; \
  object->Set##variable( string2.c_str() ); \
  if( object->Get##variable() != string2 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << std::endl; \
    return EXIT_FAILURE; \
    } \
  if( originalStringPointer != NULL ) \
    { \
    object->Set##variable( originalString.c_str() );  \
    } \
  else \
    { \
    object->Set##variable( NULL ); \
    } \
  }

#define EXERCISE_BASIC_MRML_METHODS( className, node ) \
  {\
    vtkSmartPointer < className > node1 = vtkSmartPointer < className >::New(); \
    node1->Copy(node);                                                  \
    node->Reset();                                                      \
    int mod = node->StartModify();                                      \
    std::string nodeTagName = node->GetNodeTagName();                   \
    std::cout << "Node Tag Name = " << nodeTagName << std::endl;        \
    std::string attributeName = std::string("attName");                 \
    std::string attributeValue = std::string("attValue");               \
    node->SetAttribute( attributeName.c_str(), attributeValue.c_str() ); \
    std::string attributeValue2 = node->GetAttribute( attributeName.c_str() ); \
    if( attributeValue != attributeValue2 )                             \
      {                                                                 \
      std::cerr << "Error in Set/GetAttribute() " << std::endl;         \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->EndModify(mod);                                               \
    TEST_SET_GET_BOOLEAN( node, HideFromEditors );                      \
    TEST_SET_GET_BOOLEAN( node, Selectable );                           \
    TEST_SET_GET_STRING( node, Description );                           \
    TEST_SET_GET_STRING( node, SceneRootDir );                          \
    TEST_SET_GET_STRING( node, Name );                                  \
    node->UpdateID("newID");                                            \
    if (strcmp(node->GetID(), "newID") != 0)                            \
      {                                                                 \
      std::cerr << "Error in UpdateID()" << std::endl;                  \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->CopyID(node1);                                                \
    if (node->GetID() != node1->GetID())                                \
      {                                                                 \
      std::cerr << "Error in CopyID()" << std::endl;                    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    TEST_SET_GET_STRING( node, SingletonTag );                          \
    TEST_SET_GET_BOOLEAN( node, ModifiedSinceRead );                    \
    TEST_SET_GET_BOOLEAN( node, SaveWithScene );                        \
    TEST_SET_GET_BOOLEAN( node, AddToScene );                           \
    TEST_SET_GET_BOOLEAN( node, DisableModifiedEvent);                  \
    TEST_SET_GET_BOOLEAN( node, Selected );                             \
    node->Modified();                                                   \
    node->InvokePendingModifiedEvent();                                 \
    node1->SetName("copywithsinglemodified");                           \
    node->CopyWithSingleModifiedEvent(node1);                           \
    node1->SetName("copywithoutmodified");                              \
    node->CopyWithoutModifiedEvent(node1);                              \
    node1->SetName("copywithscenewithsinglemodified");                  \
    node->CopyWithSceneWithSingleModifiedEvent(node1);                  \
    node1->SetName("copywithscenewithoutmodified");                     \
    node->CopyWithSceneWithoutModifiedEvent(node1);                     \
    vtkMRMLScene * scene = node->GetScene();                            \
                                                                        \
    if( scene != NULL )                                                 \
      {                                                                 \
      std::cerr << "Error in GetScene() " << std::endl;                 \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    node->UpdateReferences();                                           \
    node->UpdateReferenceID("oldID", "newID");                          \
                                                                        \
    std::string stringToEncode = "Thou Shall Test !";                   \
    std::string stringURLEncoded = node1->URLEncodeString( stringToEncode.c_str() ); \
    std::string stringDecoded = node1->URLDecodeString( stringURLEncoded.c_str() ); \
    if( stringDecoded != stringToEncode )                               \
      {                                                                 \
      std::cerr << "Error in URLEncodeString/URLDecodeString() " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    const char *atts[] = {"id", "vtkMRMLMeasurementsNodeTest1", "name", "MyName", "description", "Testing a mrml node", "hideFromEditors", "false", "selectable", "true", "selected", "true", NULL}; \
    node->ReadXMLAttributes(atts);                                      \
    if (strcmp(node->GetID(), "vtkMRMLMeasurementsNodeTest1") != 0)     \
      {                                                                 \
      std::cerr << "Error in ReadXMLAttributes! id should be vtkMRMLMeasurementsNodeTest1, but is " << node->GetID() << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->WriteXML(std::cout, 0);                                       \
    std::cout << std::endl;                                             \
  }

#define EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS( node ) \
  {\
    vtkMRMLTransformNode *tnode2 = node->GetParentTransformNode();\
    if (tnode2 != NULL)\
      {\
      std::cerr << "ERROR: parent transform node is not null" << std::endl;\
      return EXIT_FAILURE; \
      }\
    node1->SetAndObserveTransformNodeID(NULL);\
    const char *node_tid = node1->GetTransformNodeID();\
    if (node_tid != NULL)                              \
      {\
      std::cerr << "ERROR: with observing transform node id" << std::endl;\
      return EXIT_FAILURE; \
      }\
    bool canApplyNonLinear = node->CanApplyNonLinearTransforms();\
    std::cout << "Node can apply non linear transforms? " << (canApplyNonLinear == true ? "yes" : "no") << std::endl;\
  }

#define EXERCISE_BASIC_STORABLE_MRML_METHODS( node ) \
  {                                                  \
    if (node->GetNumberOfStorageNodes() != 0)        \
      {                                              \
      std::cerr << "Error in getting number of storage nodes." << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveStorageNodeID("noid");                           \
    node->AddAndObserveStorageNodeID("badid");                          \
    node->SetAndObserveNthStorageNodeID(2, "nothing");                  \
    node->SetSlicerDataType("testing");                                 \
    if (strcmp(node->GetSlicerDataType(), "testing") != 0)              \
      {                                                                 \
      std::cerr << "ERROR set/get slicer data type" << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    const char *snodeid = node->GetNthStorageNodeID(0);                 \
    if (strcmp(snodeid, "noid") != 0)                                        \
      {                                                                 \
      std::cerr << "ERROR getting 0th storage node id, instead of noid got " << (snodeid  == NULL ? "null" : snodeid) << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    vtkMRMLStorageNode *snode = node->GetNthStorageNode(0);             \
    if (snode != NULL)                                                   \
      {                                                                 \
      std::cerr << "ERROR getting 0th storage node" << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    snode = node->CreateDefaultStorageNode();                           \
    if (snode == NULL)                                                  \
      {                                                                 \
      std::cerr << "ERROR creating and getting default storage node" << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    snode->Delete();                                                    \
    vtkTagTable *tagtable = node->GetUserTagTable();                    \
    if (tagtable == NULL)                                               \
      {                                                                 \
      std::cerr << "ERROR getting tag table" << std::endl;              \
      return EXIT_FAILURE;                                              \
      }                                                                 \
  }

#define EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( node ) \
  {                                                  \
    if (node->GetNumberOfDisplayNodes() != 0)        \
      {                                              \
      std::cerr << "Error in getting number of display nodes." << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveDisplayNodeID("noid");                           \
    node->AddAndObserveDisplayNodeID("badid");                          \
    node->SetAndObserveNthDisplayNodeID(2, "nothing");                  \
    const char *dnodeid = node->GetNthDisplayNodeID(0);                 \
    if (strcmp(dnodeid, "noid") != 0)                                   \
      {                                                                 \
      std::cerr << "ERROR getting 0th display node id, instead of noid got " << (dnodeid  == NULL ? "null" : dnodeid) << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    vtkMRMLDisplayNode *dnode = node->GetNthDisplayNode(0);             \
    if (dnode != NULL)                                                   \
      {                                                                 \
      std::cerr << "ERROR getting 0th display node" << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    vtkPolyData *pdata = node1->GetPolyData();                          \
    if (pdata != NULL)                                                  \
      {                                                                 \
      std::cerr << "ERROR getting null polydata" << std::endl;          \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    pdata = vtkPolyData::New();                                         \
    node1->SetAndObservePolyData(pdata);                                \
    if (node1->GetPolyData() != pdata)                                  \
      {                                                                 \
      std::cerr << "ERROR getting polydata" << std::endl;               \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    pdata->Delete();                                                    \
  }

#define EXERCISE_BASIC_DISPLAY_MRML_METHODS( node ) \
  {                                                     \
    if (node->GetPolyData() != NULL)                    \
      {                                                 \
      std::cerr << "Error getting null polydata" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    if (node->GetImageData() != NULL)                    \
      {                                                 \
      std::cerr << "Error getting null image" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    vtkMRMLDisplayableNode *dnode = node->GetDisplayableNode(); \
    if (dnode != NULL)                                          \
      {                                                 \
      std::cerr << "Error getting null displayable node" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    node->UpdatePolyDataPipeline();                             \
    node->UpdateImageDataPipeline();                            \
    node->SetAndObserveTextureImageData(NULL);                  \
    if (node->GetTextureImageData() != NULL)                    \
      {                                                                 \
      std::cerr << "Error getting null texture image data " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveColorNodeID(NULL);                               \
    if (node->GetColorNodeID() != NULL)                                 \
      {                                                                 \
      std::cerr << "Error getting null color node id " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    if (node->GetColorNode() != NULL)                                   \
      {                                                                 \
      std::cerr << "Error getting null color node " << std::endl;    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetActiveScalarName("testingScalar");                         \
    if (strcmp(node->GetActiveScalarName(), "testingScalar") != 0)      \
      {                                                                 \
      std::cerr << "Error getting active scalar name" << std::endl;      \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    TEST_SET_GET_VECTOR3_DOUBLE(node, Color);                            \
    TEST_SET_GET_VECTOR3_DOUBLE(node, SelectedColor);                   \
    TEST_SET_GET_DOUBLE(node, SelectedAmbient);                         \
    TEST_SET_GET_DOUBLE(node, SelectedSpecular);                        \
    TEST_SET_GET_DOUBLE(node, Opacity);                                 \
    TEST_SET_GET_DOUBLE(node, Ambient);                                 \
    TEST_SET_GET_DOUBLE(node, Diffuse);                                 \
    TEST_SET_GET_DOUBLE(node, Specular);                                \
    TEST_SET_GET_DOUBLE(node, Power);                                   \
    TEST_SET_GET_BOOLEAN(node, Visibility);                             \
    TEST_SET_GET_BOOLEAN(node, Clipping);                               \
    TEST_SET_GET_BOOLEAN(node, SliceIntersectionVisibility);            \
    TEST_SET_GET_BOOLEAN(node, BackfaceCulling);                        \
    TEST_SET_GET_BOOLEAN(node, ScalarVisibility);                       \
    TEST_SET_GET_BOOLEAN(node, VectorVisibility);                       \
    TEST_SET_GET_BOOLEAN(node, TensorVisibility);                       \
    TEST_SET_GET_BOOLEAN(node, AutoScalarRange);                        \
    double range[2] = {-10, 10};                                        \
    node->SetScalarRange(range);                                        \
    double *getrange = node->GetScalarRange();                          \
    if (getrange == NULL || getrange[0] != range[0] || getrange[1] != range[1]) \
      {                                                                 \
      std::cerr << "ERROR getting range" << std::endl;                  \
      return EXIT_FAILURE;                                              \
      }                                                                 \
  }
#endif
