/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __vtkMRMLCoreTestingMacros_h
#define __vtkMRMLCoreTestingMacros_h

// VTK includes
#include <vtkMath.h>

/// Convenience macros for unit tests.
///
/// The macro returns from the current method with EXIT_FAILURE if the check fails.
/// Expressions can be passed as arguments, they are guaranteed to be executed only once.
///
/// Example:
///
/// \code{.cpp}
/// int testedFunction(int a, int b) { return a+b; }
///
/// int MyTest1(int , char * [])
/// {
///
///   int current = 40 + 2;
///   int expected = 42;
///   CHECK_INT(current, expected);
///
///   CHECK_INT(testedFunction(40,2), 42);
///   CHECK_INT(testedFunction(35,5), 40);
///
///   return EXIT_SUCCESS;
/// }
///
/// \endcode

/// Verifies that pointer is NULL
#define CHECK_NULL(pointer) \
  { \
  const void* pointerValue = (pointer); \
  if (!vtkMRMLCoreTestingUtilities::CheckNull(__LINE__,#pointer " is not NULL", pointerValue)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies that pointer is not NULL
#define CHECK_NOT_NULL(pointer) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckNotNull(__LINE__,#pointer " is NULL", (pointer))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies that pointer is NULL,
/// if check fails then it calls methodToCallOnFailure before returning with failure
#define CHECK_NOT_NULL_ADD_REPORT(pointer, methodToCallOnFailure) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckNotNull(__LINE__,#pointer " is NULL", (pointer))) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

#define CHECK_EXIT_SUCCESS(actual) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != EXIT_SUCCESS", (actual), EXIT_SUCCESS)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual int value is the same as expected
#define CHECK_INT(actual, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual int value is the same as expected,
/// if check fails then it calls methodToCallOnFailure before returning with failure
#define CHECK_INT_ADD_REPORT(actual, expected, methodToCallOnFailure) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual pointer value is the same as expected
#define CHECK_POINTER(actual, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckPointer(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual pointer value is the same as expected
#define CHECK_POINTER_DIFFERENT(actual, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckPointer(__LINE__,#actual " == " #expected, (actual), (expected), false)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual bool value is the same as expected
#define CHECK_BOOL(actual, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, (actual)?1:0, (expected)?1:0)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is the same as expected.
/// It can handle NULL pointer inputs.
#define CHECK_STRING(actual, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual std::string value is the same as expected.
/// It is safe to use for comparing std::string values.
/// It cannot handle NULL pointer inputs.
#define CHECK_STD_STRING(actual, expected) \
  { \
  std::string a = (actual); \
  std::string e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a.c_str(), e.c_str())) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is the same as expected,
/// if check fails then it calls methodToCallOnFailure before returning with failure
/// It can handle NULL pointer inputs.
#define CHECK_STRING_ADD_REPORT(actual, expected, methodToCallOnFailure) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is not the same as expected.
/// It can handle NULL pointer inputs.
#define CHECK_STRING_DIFFERENT(actual, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, (actual), (expected), false)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual std::string value is not the same as expected.
/// It is safe to use for comparing std::string values.
/// It cannot handle NULL pointer inputs.
#define CHECK_STD_STRING_DIFFERENT(actual, expected) \
  { \
  std::string a = (actual); \
  std::string e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a.c_str(), e.c_str(), false)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if the node's pointer defined by nodeID in scene is the same as expected
#define CHECK_NODE_IN_SCENE_BY_ID(scene, nodeID, expected) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckNodeInSceneByID(__LINE__, (scene), (nodeID), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if the node's ID and name are the same as expected
#define CHECK_NODE_ID_AND_NAME(node, expectedID, expectedName) \
  { \
  if (!vtkMRMLCoreTestingUtilities::CheckNodeIdAndName(__LINE__, (node), (expectedID), (expectedName))) \
    { \
    return EXIT_FAILURE; \
    } \
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
      return EXIT_FAILURE;                                 \
      }                                                    \
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
#define TEST_SET_GET_DOUBLE( object, variable, value )          \
  {                                                             \
    object->Set##variable( value );                             \
    if( object->Get##variable() != value )                      \
      {                                                         \
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
    double val = vtkMath::Random() * max;       \
    object->Set##variable( val );               \
    if( object->Get##variable() != val )        \
      {                                         \
      std::cerr << "Error in  Set/Get"#variable << ", using random value " << val << std::endl; \
      return EXIT_FAILURE;                      \
      }                                         \
  }

// ----------------------------------------------------------------------------
/// test an object variable via Get
#define TEST_GET_OBJECT(object,variable) \
  if ( !object->Get##variable() )        \
    {                                    \
    std::cerr << "Error in  Get"#variable << ", non null value is expected " << std::endl; \
    return EXIT_FAILURE;                 \
    }                                    \
  object->Get##variable()->Print(std::cout);

// ----------------------------------------------------------------------------
/// test an object variable on the object by setting it to input value using Set, and
/// testing it via the Get
#define TEST_SET_GET_VALUE(object,variable,value) \
  object->Set##variable(value); \
  if (object->Get##variable() != value) \
    {   \
    std::cerr << "Error getting " << #variable << std::endl; \
    std::cerr << "Expected " << value << std::endl; \
    std::cerr << "but got  " << object->Get##variable()<< std::endl; \
    return EXIT_FAILURE; \
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
      {                                                              \
      std::cerr << "Error in Set/Get"#variable << " with " << x << ", " << y << ", " << z << std::endl; \
      return EXIT_FAILURE;                                           \
      }                                                              \
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
  }

#define EXERCISE_BASIC_OBJECT_METHODS( node )                                        \
  {                                                                                  \
  int result = vtkMRMLCoreTestingUtilities::ExerciseBasicObjectMethods(node);        \
  if (result != EXIT_SUCCESS)                                                        \
    {                                                                                \
    return result;                                                                   \
    }                                                                                \
  }

/// Tests all applicable common MRML node methods
#define EXERCISE_ALL_BASIC_MRML_METHODS( node )                                      \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseAllBasicMRMLMethods(node));

//---------------------------------------------------------------------------
// Deprecated macros, for backward compatibility only
// (className is no longer needed; usually EXERCISE_ALL_BASIC_MRML_METHODS can
// be used instead of all the macros below)

#define EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS( className, node )                 \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicTransformableMRMLMethods(node));

#define EXERCISE_BASIC_MRML_METHODS( className, node )                               \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicMRMLMethods(node));

#define EXERCISE_BASIC_STORABLE_MRML_METHODS( className, node )                      \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicStorableMRMLMethods(node));

#define EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( className, node )                   \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicDisplayableMRMLMethods(node));

#define EXERCISE_BASIC_DISPLAY_MRML_METHODS( className, node )                       \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicDisplayMRMLMethods(node));

#define EXERCISE_BASIC_STORAGE_MRML_METHODS( className, node )                       \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicStorageMRMLMethods(node));

#define EXERCISE_BASIC_TRANSFORM_MRML_METHODS( className, node )                     \
  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseBasicTransformMRMLMethods(node));

// Need to include vtkMRMLCoreTestingUtilities.h here because
// vtkMRMLCoreTestingUtilities use some of the macros above, too.
#include "vtkMRMLCoreTestingUtilities.h"
#include "vtkMRMLNode.h"

// Commonly used headers in tests
#include "vtkNew.h"
#include "vtkTestingOutputWindow.h"

#endif
