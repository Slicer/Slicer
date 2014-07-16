/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __vtkMRMLCoreTestingMacros_h
#define __vtkMRMLCoreTestingMacros_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkNew.h>

// STD includes
#include <vector>

// ----------------------------------------------------------------------------
/// tests basic vtkObject methods
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

// ----------------------------------------------------------------------------
/// try the itk command passed in, succeeding if catch an exception
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

// ----------------------------------------------------------------------------
/// try the command passed in, succeeding if no exception
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

// ----------------------------------------------------------------------------
/// test itk set/get
#define TEST_ITK_SET_GET( variable, command ) \
  if( variable.GetPointer() != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable.GetPointer() << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }

// ----------------------------------------------------------------------------
/// test itk set/get
#define TEST_ITK_SET_GET_VALUE( variable, command ) \
  if( variable != command )   \
    {   \
    std::cerr << "Error in " << #command << std::endl; \
    std::cerr << "Expected " << variable << std::endl; \
    std::cerr << "but got  " << command << std::endl; \
    return EXIT_FAILURE; \
    }

// ----------------------------------------------------------------------------
/// test object by calling Set on the variable with false, true, 0, 1, On, Off
#define TEST_SET_GET_BOOLEAN( object, variable ) \
  object->Set##variable( false ); \
  object->Set##variable( true ); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << ", Get"#variable << " is " << object->Get##variable() << " instead of 1" << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->Set##variable( false ); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << ", Get"#variable << " is " << object->Get##variable() << " instead of 0" << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##On(); \
  if( object->Get##variable() != 1 ) \
    {   \
    std::cerr << "Error in On/Get"#variable << ", Get"#variable << " is " << object->Get##variable() << " instead of 1" << std::endl; \
    return EXIT_FAILURE; \
    } \
  object->variable##Off(); \
  if( object->Get##variable() != 0 ) \
    {   \
    std::cerr << "Error in Off/Get"#variable << ", Get"#variable << " is " << object->Get##variable() << " instead of 0" << std::endl; \
    return EXIT_FAILURE; \
    }

// ----------------------------------------------------------------------------
/// test an integer variable on the object by setting it to input value using Set, and
/// testing it via the Get
#define TEST_SET_GET_INT( object, variable, value )        \
  {                                                        \
    object->Set##variable( value );                        \
    if( object->Get##variable() != value )                 \
      {                                                    \
      std::cerr << "Error in Set/Get"#variable << " using value " << value << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
  }

// ----------------------------------------------------------------------------
/// Test an integer variable on object over the range, calls test set get in
/// with min - epsilon, min, min + epsilon, (min+max)/2, max - epsilon, max,
/// max + epsilon, where first and last test should report errors
/// epsilon defined as 1
#define TEST_SET_GET_INT_RANGE( object, variable, min, max ) \
  {                                                         \
    int epsilon = 1;                                        \
    int val = min - epsilon;                                \
    TEST_SET_GET_INT( object, variable, val);               \
    val = min;                                              \
    TEST_SET_GET_INT( object, variable, val);               \
    val = min + epsilon;                                    \
    TEST_SET_GET_INT( object, variable, val);               \
    val = (min + max) / 2;                                  \
    TEST_SET_GET_INT( object, variable, val);               \
    val = max - epsilon;                                    \
    TEST_SET_GET_INT( object, variable, val);               \
    val = max;                                              \
    TEST_SET_GET_INT( object, variable, val);               \
    val = max + epsilon;                                    \
    TEST_SET_GET_INT( object, variable, val);               \
  }

// ----------------------------------------------------------------------------
/// test an integer variable on the object by setting it to a random value up
/// to max using Set, and  testing it via the Get
#define TEST_SET_GET_INT_RANDOM( object, variable, max )    \
  {                                                         \
    int val = (int)(vtkMath::Random() * max);               \
    object->Set##variable( val );                           \
    if( object->Get##variable() != val )                    \
      {                                                     \
      std::cerr << "Error in Set/Get"#variable << " using random value " << val << std::endl; \
      return EXIT_FAILURE;                                  \
      }                                                     \
  }

// ----------------------------------------------------------------------------
/// Test an unsigned integer variable on object over the range, calls test set get in
/// with min - epsilon, min, min + epsilon, (min+max)/2, max - epsilon, max,
/// max + epsilon, where first and last test should report errors
/// epsilon defined as 1
#define TEST_SET_GET_UNSIGNED_INT_RANGE( object, variable, min, max ) \
  {                                                         \
    unsigned int epsilon = 1;                               \
    unsigned int val = min - epsilon;                      \
    TEST_SET_GET_INT( object, variable, val);               \
    val = min;                                              \
    TEST_SET_GET_INT( object, variable, val);               \
    val = min + epsilon;                                    \
    TEST_SET_GET_INT( object, variable, val);               \
    val = (min + max) / 2;                                  \
    TEST_SET_GET_INT( object, variable, val);               \
    val = max - epsilon;                                    \
    TEST_SET_GET_INT( object, variable, val);               \
    val = max;                                              \
    TEST_SET_GET_INT( object, variable, val);               \
    val = max + epsilon;                                    \
    TEST_SET_GET_INT( object, variable, val);               \
  }

// ----------------------------------------------------------------------------
/// test a double variable on the object by setting it to input value using Set, and
/// testing it via the Get
#define TEST_SET_GET_DOUBLE( object, variable, value )    \
  {                                             \
    object->Set##variable( value );               \
    if( object->Get##variable() != value )        \
      {                                         \
      std::cerr << "Error in Set/Get"#variable << " using value " << value << std::endl; \
      return EXIT_FAILURE;                                      \
      }                                                         \
  }

// ----------------------------------------------------------------------------
/// Test a double variable on object over the range, calls test set get in
/// with min - epsilon, min, min + epsilon, (min+max)/2, max - epsilon, max,
/// max + epsilon, where first and last test should report errors
/// epsilon set to 1.0
#define TEST_SET_GET_DOUBLE_RANGE( object, variable, min, max )         \
  {                                                                     \
    double epsilon = 1.0;                                               \
    double val = min - epsilon;                                         \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
    val = min;                                                          \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
    val = min + epsilon;                                                \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
    val = (min + max) / 2.0;                                            \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
    val = max - epsilon;                                                \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
    val = max;                                                          \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
    val = max + epsilon;                                                \
    TEST_SET_GET_DOUBLE( object, variable, val);                        \
  }

// ----------------------------------------------------------------------------
/// test an integer variable on the object by setting it to a random value up
/// to max using Set, and  testing it via the Get
#define TEST_SET_GET_DOUBLE_RANDOM( object, variable, max ) \
  {                                             \
    double val = vtkMath::Random() * max;     \
    object->Set##variable( val );               \
    if( object->Get##variable() != val )        \
      {                                         \
      std::cerr << "Error in  Set/Get"#variable << ", using random value " << val << std::endl; \
      return EXIT_FAILURE;                                      \
      }                                                         \
  }

// ----------------------------------------------------------------------------
/// test a vector variable on the object by setting it to a the values x, y, z
/// passed in using Set, and  testing it via the Get
#define TEST_SET_GET_VECTOR3_DOUBLE( object, variable, x, y, z )    \
  {                                                                 \
    object->Set##variable( x, y, z );                               \
    double *val = object->Get##variable();                          \
    if( val == NULL || val[0] != x || val[1] != y || val[2] != z )  \
      {                                                             \
      std::cerr << "Error in Set/Get"#variable << std::endl;        \
      return EXIT_FAILURE;                                          \
      }                                                             \
  }

// ----------------------------------------------------------------------------
/// Test a double vector variable on object over the range, calls test set get in
/// with min - epsilon, min, min + epsilon, (min+max)/2, max - epsilon, max,
/// max + epsilon, where first and last test should report errors. For now all
/// three elements are set to the same thing each time.
/// epsilon set to 1.0
#define TEST_SET_GET_VECTOR3_DOUBLE_RANGE( object, variable, min, max )  \
  {                                                                     \
    double epsilon = 1.0;                                               \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, min - epsilon, min - epsilon, min - epsilon); \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, min, min, min);       \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, min + epsilon, min + epsilon, min + epsilon); \
    double half = (min+max/2.0);                                        \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, half, half, half);    \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, max - epsilon, max - epsilon, max - epsilon); \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, max, max, max);       \
    TEST_SET_GET_VECTOR3_DOUBLE(object, variable, max + epsilon, max + epsilon, max + epsilon); \
  }

// ----------------------------------------------------------------------------
/// test a vector variable on the object by setting all it's elements to the same random value up
/// to max using Set, and  testing it via the Get
#define TEST_SET_GET_VECTOR3_DOUBLE_RANDOM( object, variable, max )  \
  {                                                                  \
    double x = vtkMath::Random() * max;                              \
    double y = vtkMath::Random() * max;                              \
    double z = vtkMath::Random() * max;                              \
    object->Set##variable( x, y, z );                                \
    double val[3] = {0.0, 0.0, 0.0};                                 \
    object->Get##variable(val);                                      \
    if( val[0] != x || val[1] != y || val[2] != z )                  \
      {                                                             \
      std::cerr << "Error in Set/Get"#variable << " with " << x << ", " << y << ", " << z << std::endl; \
      return EXIT_FAILURE;                                          \
      }                                                             \
  }

// ----------------------------------------------------------------------------
/// test a string variable on the object by calling Set/Get
#define TEST_SET_GET_STRING( object, variable ) \
  { \
  const char * originalStringPointer = object->Get##variable(); \
  std::string originalString; \
  if( originalStringPointer != NULL ) \
    { \
    originalString = originalStringPointer; \
    } \
  object->Set##variable( "testing with a const char");                  \
  if( strcmp(object->Get##variable(), "testing with a const char") != 0) \
    {                                                                   \
    std::cerr << "Error in Set/Get"#variable << " with a string literal" << std::endl; \
    return EXIT_FAILURE;                                                \
    }                                                                   \
  std::string string1 = "testingIsGood"; \
  object->Set##variable( string1.c_str() ); \
  if( object->Get##variable() != string1 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << ", tried to set to " << string1.c_str() << " but got " << (object->Get##variable() ? object->Get##variable() : "null") << std::endl; \
    return EXIT_FAILURE; \
    } \
  std::string string2 = "moreTestingIsBetter"; \
  object->Set##variable( string2.c_str() ); \
  if( object->Get##variable() != string2 ) \
    {   \
    std::cerr << "Error in Set/Get"#variable << ", tried to set to " << string2.c_str() << " but got " << (object->Get##variable() ? object->Get##variable() : "null") << std::endl; \
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
  }                                                                  \

// ----------------------------------------------------------------------------
/// Slicer Libs/MRML/vtkMRMLNode exercises
#define EXERCISE_BASIC_MRML_METHODS( className, node )                  \
  {                                                                     \
    /* Test CreateNodeInstance() */                                     \
    vtkMRMLNode * newNode = node1->CreateNodeInstance();                \
    if( newNode == NULL )                                               \
      {                                                                 \
      std::cerr << "Error in CreateNodeInstance()" << std::endl;        \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    newNode->Delete();                                                  \
                                                                        \
    /* Test UpdateScene() */                                            \
    node->UpdateScene(NULL);                                            \
                                                                        \
    /* Test New() */                                                    \
    vtkSmartPointer < className > node1 =                               \
      vtkSmartPointer < className >::New();                             \
                                                                        \
    /* Test GetID() */                                                  \
    {                                                                   \
    const char * outputPointer = node1->GetID();                        \
    if (outputPointer != NULL)                                          \
      {                                                                 \
      std::cerr << "Error in GetID !\n"                                 \
                << "  expected: (null)\n"                               \
                << "  current: " << outputPointer << std::endl;         \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    }                                                                   \
                                                                        \
    /* Test GetNodeTagName() */                                         \
    {                                                                   \
    const char * outputPointer = node1->GetNodeTagName();               \
    if (outputPointer == NULL || strlen(outputPointer) == 0)            \
      {                                                                 \
      std::cerr << "Error in GetNodeTagName !\n"                        \
                << "  expected: (NOT null or empty)\n"                  \
                << "  current: (null or empty)" << std::endl;           \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    }                                                                   \
                                                                        \
    /* Test Copy() */                                                   \
    node1->Copy(node);                                                  \
    node->Reset();                                                      \
                                                                        \
                                                                        \
    /* Test SetAttribute() / GetAttribute() */                          \
    int mod = node->StartModify();                                      \
    std::string attributeName = std::string("attName");                 \
    std::string attributeValue = std::string("attValue");               \
    node->SetAttribute(                                                 \
        attributeName.c_str(), attributeValue.c_str() );                \
    std::string attributeValue2 =                                       \
        node->GetAttribute( attributeName.c_str() );                    \
    if( attributeValue != attributeValue2 )                             \
      {                                                                 \
      std::cerr << "Error in Set/GetAttribute() " << std::endl;         \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->EndModify(mod);                                               \
                                                                        \
    /* Test getters */                                                  \
    TEST_SET_GET_BOOLEAN( node, HideFromEditors );                      \
    TEST_SET_GET_BOOLEAN( node, Selectable );                           \
    TEST_SET_GET_STRING( node, Description );                           \
    TEST_SET_GET_STRING( node, SceneRootDir );                          \
    TEST_SET_GET_STRING( node, Name );                                  \
    TEST_SET_GET_STRING( node, SingletonTag );                          \
    TEST_SET_GET_BOOLEAN( node, SaveWithScene );                        \
    TEST_SET_GET_BOOLEAN( node, AddToScene );                           \
    TEST_SET_GET_BOOLEAN( node, DisableModifiedEvent);                  \
    TEST_SET_GET_BOOLEAN( node, Selected );                             \
                                                                        \
    node->Modified();                                                   \
    node->InvokePendingModifiedEvent();                                 \
    node1->SetName("copywithsinglemodified");                           \
    node->CopyWithSingleModifiedEvent(node1);                           \
    node1->SetName("copywithoutmodified");                              \
    node->CopyWithoutModifiedEvent(node1);                              \
    node1->SetName("copywithscenewithsinglemodified");                  \
    node->CopyWithSceneWithSingleModifiedEvent(node1);                  \
                                                                        \
    /* Test GetScene() */                                               \
    vtkMRMLScene * scene = node->GetScene();                            \
    if( scene != NULL )                                                 \
      {                                                                 \
      std::cerr << "Error in GetScene() " << std::endl;                 \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    /* Test UpdateReferences() */                                       \
    node->UpdateReferences();                                           \
    node->UpdateReferenceID("oldID", "newID");                          \
                                                                        \
    /* Test URLEncodeString() */                                        \
    {                                                                   \
    const char* inputPointer = "Thou Shall Test !";                     \
    const char* exepectedPointer = "Thou%20Shall%20Test%20!";           \
    const char* outputPointer = node1->URLEncodeString(inputPointer);   \
    if (outputPointer == NULL)                                          \
      {                                                                 \
      std::cerr << "Problem with URLEncodeString()!\n"                  \
                << "  expected: " << exepectedPointer << "\n"           \
                << "  current: (null)" << std::endl;                    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    if (strcmp(outputPointer, exepectedPointer) != 0)                   \
      {                                                                 \
      std::cerr << "Problem with URLEncodeString()!\n"                  \
                << "  expected: " << exepectedPointer << "\n"           \
                << "  current: " << outputPointer << std::endl;         \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    }                                                                   \
                                                                        \
    /* Test URLDecodeString() */                                        \
    {                                                                   \
    const char* inputPointer = "Thou%20Shall%20Test%20!";               \
    const char* exepectedPointer = "Thou Shall Test !";                 \
    const char* outputPointer = node1->URLDecodeString(inputPointer);   \
    if (outputPointer == NULL)                                          \
      {                                                                 \
      std::cerr << "Problem with URLDecodeString()!\n"                  \
                << "  expected: " << exepectedPointer << "\n"           \
                << "  current: (null)" << std::endl;                    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    if (strcmp(outputPointer, exepectedPointer) != 0)                   \
      {                                                                 \
      std::cerr << "Problem with URLDecodeString()!\n"                  \
                << "  expected: " << exepectedPointer << "\n"           \
                << "  current: " << outputPointer << std::endl;         \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    }                                                                   \
                                                                        \
    /* Test ReadXMLAttributes() */                                      \
    const char *atts[] = {                                              \
              "id", "vtkMRMLNodeTest1",                                 \
              "name", "MyName",                                         \
              "description", "Testing a mrml node",                     \
              "hideFromEditors", "false",                               \
              "selectable", "true",                                     \
              "selected", "true",                                       \
              NULL};                                                    \
    node->ReadXMLAttributes(atts);                                      \
                                                                        \
    if (strcmp(node->GetID(), "vtkMRMLNodeTest1") != 0)                 \
      {                                                                 \
      std::cerr << "Error in ReadXMLAttributes! id should be vtkMRMLNodeTest1, but is " << node->GetID() << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    /* Test WriteXML */                                                 \
    std::cout << "WriteXML output:" << std::endl;                       \
    node->WriteXML(std::cout, 0);                                       \
    std::cout << std::endl;                                             \
  }

// ----------------------------------------------------------------------------
/// For testing nodes in Libs/MRML that are transformable. Calls the basic
/// mrml methods macro first.
#define EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS( className, node )    \
  {                                                                     \
    EXERCISE_BASIC_MRML_METHODS(className, node);                       \
    vtkMRMLTransformNode *tnode2 = node->GetParentTransformNode();      \
    if (tnode2 != NULL)                                                 \
      {                                                                 \
      std::cerr << "ERROR: parent transform node is not null" << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node1->SetAndObserveTransformNodeID(NULL);                          \
    const char *node_tid = node1->GetTransformNodeID();                 \
    if (node_tid != NULL)                                               \
      {                                                                 \
      std::cerr << "ERROR: with observing transform node id" << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    bool canApplyNonLinear = node->CanApplyNonLinearTransforms();       \
    std::cout << "Node can apply non linear transforms? " << (canApplyNonLinear == true ? "yes" : "no") << std::endl; \
  }

// ----------------------------------------------------------------------------
/// For testing nodes in Libs/MRML that are storable. Calls the basic
/// transformable mrml methods macro first.
#include "vtkMRMLStorageNode.h"
#define EXERCISE_BASIC_STORABLE_MRML_METHODS( className, node )  \
  {                                                  \
    EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(className, node );    \
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

// ----------------------------------------------------------------------------
/// For testing nodes in Libs/MRML that are displayable. Calls the basic
/// transformable mrml methods macro first.
#define EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( className, node )      \
  {                                                                     \
    EXERCISE_BASIC_STORABLE_MRML_METHODS( className, node );            \
    if (node->GetNumberOfDisplayNodes() != 0)                           \
      {                                                                 \
      std::cerr << "Error in getting number of display nodes." << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveDisplayNodeID("noid");                           \
    node->AddAndObserveDisplayNodeID("badid");                          \
    node->SetAndObserveNthDisplayNodeID(2, "nothing");                  \
    const char *dnodeid = node->GetNthDisplayNodeID(0);                 \
    if (dnodeid == NULL ||  strcmp(dnodeid, "noid") != 0)               \
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
  }

// ----------------------------------------------------------------------------
/// For testing nodes in Libs/MRML that are subclasses of the display node. Calls the basic
/// mrml methods macro first.
#if (VTK_MAJOR_VERSION <= 5)
#define EXERCISE_BASIC_DISPLAY_MRML_METHODS( className, node )   \
  {                                                     \
    EXERCISE_BASIC_MRML_METHODS( className, node);   \
    vtkMRMLDisplayableNode *dnode = node->GetDisplayableNode(); \
    if (dnode != NULL)                                          \
      {                                                 \
      std::cerr << "Error getting null displayable node" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    node->SetAndObserveTextureImageData(NULL);                  \
    if (node->GetTextureImageData() != NULL)                    \
      {                                                                 \
      std::cerr << "Error getting null texture image data " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveColorNodeID(NULL);                               \
    if (node->GetColorNodeID() != NULL)                                 \
      {                                                                 \
      std::cerr << "Error getting null color node id " << std::endl;    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    if (node->GetColorNode() != NULL)                                   \
      {                                                                 \
      std::cerr << "Error getting null color node " << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetActiveScalarName("testingScalar");                         \
    if (strcmp(node->GetActiveScalarName(), "testingScalar") != 0)      \
      {                                                                 \
      std::cerr << "Error getting active scalar name" << std::endl;     \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, Color, 0.0, 1.0);           \
    TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, SelectedColor, 0.0, 1.0);   \
    TEST_SET_GET_DOUBLE_RANGE(node, SelectedAmbient, 0.0, 1.0);             \
    TEST_SET_GET_DOUBLE_RANGE(node, SelectedSpecular, 0.0, 1.0);            \
    TEST_SET_GET_DOUBLE_RANGE(node, Opacity, 0.0, 1.0);                     \
    TEST_SET_GET_DOUBLE_RANGE(node, Ambient, 0.0, 1.0);                     \
    TEST_SET_GET_DOUBLE_RANGE(node, Diffuse, 0.0, 1.0);                     \
    TEST_SET_GET_DOUBLE_RANGE(node, Specular, 0.0, 1.0);                    \
    TEST_SET_GET_DOUBLE_RANGE(node, Power, 0.0, 1.0);                       \
    TEST_SET_GET_BOOLEAN(node, Visibility);                             \
    TEST_SET_GET_BOOLEAN(node, Clipping);                               \
    TEST_SET_GET_BOOLEAN(node, SliceIntersectionVisibility);            \
    TEST_SET_GET_INT_RANGE(node, SliceIntersectionThickness, 0, 10);   \
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
    TEST_SET_GET_INT_RANGE(node1.GetPointer(), ScalarRangeFlag, 0, 3);  \
  }
#else
#define EXERCISE_BASIC_DISPLAY_MRML_METHODS( className, node )   \
  {                                                     \
    EXERCISE_BASIC_MRML_METHODS( className, node);   \
    vtkMRMLDisplayableNode *dnode = node->GetDisplayableNode(); \
    if (dnode != NULL)                                          \
      {                                                 \
      std::cerr << "Error getting null displayable node" << std::endl;  \
      return EXIT_FAILURE;                                      \
      }                                                         \
    node->SetTextureImageDataConnection(NULL);                  \
    if (node->GetTextureImageDataConnection() != NULL)                    \
      {                                                                 \
      std::cerr << "Error getting null texture image data " << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetAndObserveColorNodeID(NULL);                               \
    if (node->GetColorNodeID() != NULL)                                 \
      {                                                                 \
      std::cerr << "Error getting null color node id " << std::endl;    \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    if (node->GetColorNode() != NULL)                                   \
      {                                                                 \
      std::cerr << "Error getting null color node " << std::endl;       \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->SetActiveScalarName("testingScalar");                         \
    if (strcmp(node->GetActiveScalarName(), "testingScalar") != 0)      \
      {                                                                 \
      std::cerr << "Error getting active scalar name" << std::endl;     \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, Color, 0.0, 1.0);           \
    TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node, SelectedColor, 0.0, 1.0);   \
    TEST_SET_GET_DOUBLE_RANGE(node, SelectedAmbient, 0.0, 1.0);             \
    TEST_SET_GET_DOUBLE_RANGE(node, SelectedSpecular, 0.0, 1.0);            \
    TEST_SET_GET_DOUBLE_RANGE(node, Opacity, 0.0, 1.0);                     \
    TEST_SET_GET_DOUBLE_RANGE(node, Ambient, 0.0, 1.0);                     \
    TEST_SET_GET_DOUBLE_RANGE(node, Diffuse, 0.0, 1.0);                     \
    TEST_SET_GET_DOUBLE_RANGE(node, Specular, 0.0, 1.0);                    \
    TEST_SET_GET_DOUBLE_RANGE(node, Power, 0.0, 1.0);                       \
    TEST_SET_GET_BOOLEAN(node, Visibility);                             \
    TEST_SET_GET_BOOLEAN(node, Clipping);                               \
    TEST_SET_GET_BOOLEAN(node, SliceIntersectionVisibility);            \
    TEST_SET_GET_INT_RANGE(node, SliceIntersectionThickness, 0, 10);   \
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

// ----------------------------------------------------------------------------
#include <vtkStringArray.h>
#include <vtkURIHandler.h>
/// For testing nodes in Libs/MRML that are subclasses of the storage node. Calls the basic
/// mrml methods macro first.
#define EXERCISE_BASIC_STORAGE_MRML_METHODS( className, node )   \
  {                                                 \
    EXERCISE_BASIC_MRML_METHODS(className, node);    \
    node->ReadData(NULL);                           \
    node->WriteData(NULL);                          \
    TEST_SET_GET_STRING(node, FileName);            \
    const char *f0 = node->GetNthFileName(0);       \
    std::cout << "Filename 0 = " << (f0 == NULL ? "NULL" : f0) << std::endl; \
    TEST_SET_GET_BOOLEAN(node, UseCompression);         \
    TEST_SET_GET_STRING(node, URI);                     \
    vtkURIHandler *handler = vtkURIHandler::New();      \
    node->SetURIHandler(NULL);                          \
    if (node->GetURIHandler() != NULL)                  \
      {                                                 \
      std::cerr << "ERROR getting null uri handler" << std::endl;   \
      return EXIT_FAILURE;                                          \
      }                                                             \
    node->SetURIHandler(handler);                       \
    if (node->GetURIHandler() == NULL)                  \
      {                                                 \
      std::cerr << "ERROR getting not null uri handler" << std::endl;   \
      return EXIT_FAILURE;                                          \
      }                                                             \
    node->SetURIHandler(NULL);                                      \
    handler->Delete();                                              \
    TEST_SET_GET_INT_RANGE(node, ReadState, 0, 5);                  \
    const char *rstate = node->GetReadStateAsString();              \
    std::cout << "Read state, after int test = " << rstate << std::endl; \
    node->SetReadStatePending();                                    \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Pending = " << rstate << std::endl;   \
    node->SetReadStateIdle();                                       \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Idle = " << rstate << std::endl;      \
    node->SetReadStateScheduled();                                  \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Scheduled = " << rstate << std::endl; \
    node->SetReadStateTransferring();                               \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Transferring = " << rstate << std::endl;   \
    node->SetReadStateTransferDone();                               \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, TransfrerDone = " << rstate << std::endl;   \
    node->SetReadStateCancelled();                                  \
    rstate = node->GetReadStateAsString();                          \
    std::cout << "Read state, Cancelled = " << rstate << std::endl; \
                                                                    \
    TEST_SET_GET_INT_RANGE(node, WriteState, 0, 5);                  \
    const char *wstate = node->GetWriteStateAsString();              \
    std::cout << "Write state, after int test = " << wstate << std::endl; \
    node->SetWriteStatePending();                                    \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Pending = " << wstate << std::endl;   \
    node->SetWriteStateIdle();                                       \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Idle = " << wstate << std::endl;      \
    node->SetWriteStateScheduled();                                  \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Scheduled = " << wstate << std::endl; \
    node->SetWriteStateTransferring();                               \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Transferring = " << wstate << std::endl;   \
    node->SetWriteStateTransferDone();                               \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, TransfrerDone = " << wstate << std::endl;   \
    node->SetWriteStateCancelled();                                  \
    wstate = node->GetWriteStateAsString();                          \
    std::cout << "Write state, Cancelled = " << wstate << std::endl; \
                                                                     \
    std::string fullName = node->GetFullNameFromFileName();          \
    std::cout << "fullName = " << fullName.c_str() << std::endl; \
    std::string fullName0 = node->GetFullNameFromNthFileName(0);     \
    std::cout << "fullName0 = " << fullName0.c_str() << std::endl; \
                                                                        \
    vtkStringArray *types = node->GetSupportedWriteFileTypes();         \
    std::cout << "Supported write types:" << std::endl;                 \
    for (vtkIdType i = 0; i < types->GetNumberOfValues(); i++)                 \
      {                                                                 \
      std::cout << "\t" << types->GetValue(i).c_str() << std::endl;      \
      }                                                                 \
    int sup = node->SupportedFileType(NULL);                            \
    std::cout << "Filename or uri supported? " << sup << std::endl;     \
    sup = node->SupportedFileType("testing.vtk");                       \
    std::cout << ".vtk supported?  " << sup << std::endl;     \
    sup = node->SupportedFileType("testing.nrrd");            \
    std::cout << ".nrrd supported?  " << sup << std::endl;     \
    sup = node->SupportedFileType("testing.mgz");            \
    std::cout << ".mgz supported?  " << sup << std::endl;     \
                                                               \
    TEST_SET_GET_STRING(node, WriteFileFormat);                \
    node->AddFileName("testing.txt");                          \
    std::cout << "Number of file names = " << node->GetNumberOfFileNames() << std::endl; \
    int check = node->FileNameIsInList("testing.txt");\
    if (check != 1)                                                     \
      {                                                                 \
      std::cerr << "ERROR: file name not in list!" << std::endl;        \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->ResetNthFileName(0, "moretesting.txt");                       \
    node->ResetNthFileName(100, "notinlist.txt");                       \
    node->ResetNthFileName(0, NULL);                                    \
    check = node->FileNameIsInList("notinlist");                        \
    if (check != 0)                                                     \
      {                                                                 \
      std::cerr << "ERROR: bad file is in list!" << std::endl;          \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->ResetFileNameList();                                          \
    if (node->GetNumberOfFileNames() != 0)                              \
      {                                                                 \
      std::cerr << "ERROR: " << node->GetNumberOfFileNames() << " files left in list after reset!" << std::endl;          \
      return EXIT_FAILURE;                                              \
      }                                                                 \
                                                                        \
    node->ResetURIList();                                                            \
    std::cout << "Number of uri's after resetting list = " << node->GetNumberOfURIs() << std::endl; \
    node->AddURI("http://www.nowhere.com/filename.txt");                \
    if ( node->GetNumberOfURIs()  != 1)                                 \
      {                                                                 \
      std::cerr << "Error adding one uri, number of uris is incorrect: " <<  node->GetNumberOfURIs()<< std::endl;                \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    const char *uri = node->GetNthURI(0);                               \
    if (uri == NULL || strcmp(uri, "http://www.nowhere.com/filename.txt") != 0)        \
      {                                                                 \
      std::cerr << "0th URI " << uri << " is incorrect." << std::endl;                \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    node->ResetNthURI(0, "http://www.nowhere.com/newfilename.txt");     \
    node->ResetNthURI(100, "ftp://not.in.list");                        \
    node->ResetNthURI(100, NULL);                                       \
    const char *dataDirName = "/test-ing/a/dir ect.ory";                \
    node->SetDataDirectory(dataDirName);                                \
    node->SetFileName("/tmp/file.txt");                                 \
    node->SetDataDirectory(dataDirName);                                \
    if (strcmp("/test-ing/a/dir ect.ory/file.txt", node->GetFileName()) != 0) \
      {                                                                 \
      std::cerr << "Setting DataDirectory to " << dataDirName << " failed, got new file name of " << node->GetFileName() << std::endl; \
      return EXIT_FAILURE;                                              \
      }                                                                 \
    else                                                                \
      {                                                                 \
    std::cout << "Resetting Data Directory to " << dataDirName << " succeeded, got new file name of " << node->GetFileName() << std::endl; \
      }                                                                 \
    const char *uriPrefix = "http://www.somewhere.com/";                \
    node->SetURIPrefix(uriPrefix);                                      \
                                                                        \
    const char *defaultExt = node->GetDefaultWriteFileExtension();      \
    std::cout << "Default write extension = " << (defaultExt == NULL ? "null" : defaultExt) << std::endl; \
                                                                        \
    std::cout << "Is null file path relative? " << node->IsFilePathRelative(NULL) << std::endl; \
    std::cout << "Is absolute file path relative? " << node->IsFilePathRelative("/spl/tmp/file.txt") << std::endl; \
    std::cout << "Is relative file path relative? " << node->IsFilePathRelative("tmp/file.txt") << std::endl; \
  }

// ----------------------------------------------------------------------------
#include <vtkMatrix4x4.h>
/// For testing nodes in Libs/MRML that are transform nodes. Calls the basic
/// storable mrml methods macro first.
#define EXERCISE_BASIC_TRANSFORM_MRML_METHODS( className, node )    \
  {                                                                  \
    EXERCISE_BASIC_STORABLE_MRML_METHODS( className, node );         \
    std::cout << "IsLinear = " << node->IsLinear()<< std:: endl;        \
    vtkAbstractTransform* gtp = node->GetTransformToParent();        \
    if (gtp == NULL)                                                 \
      {                                                                 \
      std::cout << "Warning: transform node has a null transform to parent" << std::endl; \
      }                                                                 \
    std::cout << "IsTransformToWorldLinear = " << node->IsTransformToWorldLinear() << std::endl; \
    vtkSmartPointer < className > t = vtkSmartPointer < className >::New(); \
    std::cout << "IsTransformToNodeLinear = " << node->IsTransformToNodeLinear(t) << std::endl; \
    vtkSmartPointer<vtkGeneralTransform> g =  vtkSmartPointer<vtkGeneralTransform>::New(); \
    node->GetTransformToWorld(g);                                    \
    node->GetTransformToNode(t, g);                                     \
    vtkSmartPointer<vtkMatrix4x4> m =  vtkSmartPointer<vtkMatrix4x4>::New(); \
    int retval = node->GetMatrixTransformToWorld(m);                    \
    if (retval == 0)                                                    \
      {                                                                 \
      std::cout << "Warning: can't get matrix transform to world." << std::endl; \
      }                                                                 \
    retval = node->GetMatrixTransformToNode(t, m);                      \
    if (retval == 0)                                                    \
      {                                                                 \
      std::cout << "Warning: can't get matrix transform to node." << std::endl; \
      }                                                                 \
    std::cout << "IsTransformNodeMyParent = " << node->IsTransformNodeMyParent(t) << std::endl; \
    std::cout << "IsTransformNodeMyChild = " << node->IsTransformNodeMyChild(t) << std::endl; \
  }

//---------------------------------------------------------------------------
class VTK_MRML_EXPORT vtkMRMLNodeCallback : public vtkCallbackCommand
{
public:
  static vtkMRMLNodeCallback *New() {return new vtkMRMLNodeCallback;}
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Execute(vtkObject* caller, unsigned long eid, void *callData);
  virtual void ResetNumberOfEvents();

  void SetMRMLNode(vtkMRMLNode*);
  std::string GetErrorString();

  int GetNumberOfModified();
  int GetNumberOfEvents(unsigned long event);
  int GetTotalNumberOfEvents();
  std::vector<unsigned long> GetReceivedEvents();

protected:
  vtkMRMLNodeCallback();
  ~vtkMRMLNodeCallback();

  void SetErrorString(const char* error);

  vtkMRMLNode* Node;
  std::string ErrorString;
  std::map<unsigned long, unsigned int> ReceivedEvents;
};

#endif
