/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

#ifndef __vtkMRMLCoreTestingUtilities_h
#define __vtkMRMLCoreTestingUtilities_h

// MRML/Core includes
#include <vtkMRML.h>
class vtkMRMLNode;
class vtkMRMLScene;

// STD includes
#include <sstream>

/// This module provides functions to facilitate writing tests.
///
/// Example:
///
/// \code{.cpp}
/// int current = 40 + 2;
/// int expected = 43;
/// if (!CheckInt(__LINE__, "40 + 2", current, expected))
///   {
///   return false;
///   }
/// \endcode

namespace vtkMRMLCoreTestingUtilities
{

VTK_MRML_EXPORT
bool CheckInt(int line, const std::string& description,
              int current, int expected);

VTK_MRML_EXPORT
bool CheckNotNull(int line, const std::string& description,
                  const void* pointer);

VTK_MRML_EXPORT
bool CheckNull(int line, const std::string& description,
               const void* pointer);

VTK_MRML_EXPORT
bool CheckPointer(int line, const std::string& description,
                  void* current, void* expected);

VTK_MRML_EXPORT
bool CheckString(int line, const std::string& description,
                 const char* current, const char* expected, bool errorIfDifferent = true );

VTK_MRML_EXPORT
bool CheckNodeInSceneByID(int line, vtkMRMLScene* scene,
                          const char* nodeID, vtkMRMLNode* expected);

VTK_MRML_EXPORT
bool CheckNodeIdAndName(int line, vtkMRMLNode* node,
                        const char* expectedID, const char* expectedName);

template<typename Type>
std::string ToString(Type value);

}

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
  if (!vtkMRMLCoreTestingUtilities::CheckNull(__LINE__,#pointer " is NULL", pointerValue)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies that pointer is not NULL
#define CHECK_NOT_NULL(pointer) \
  { \
  const void* pointerValue = (pointer); \
  if (!vtkMRMLCoreTestingUtilities::CheckNotNull(__LINE__,#pointer " is NULL", pointerValue)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies that pointer is NULL,
/// if check fails then it calls methodToCallOnFailure before returning with failure
#define CHECK_NOT_NULL_ADD_REPORT(pointer, methodToCallOnFailure) \
  { \
  const void* pointerValue = (pointer); \
  if (!vtkMRMLCoreTestingUtilities::CheckNotNull(__LINE__,#pointer " is NULL", pointerValue)) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual int value is the same as expected
#define CHECK_INT(actual, expected) \
  { \
  int a = (actual); \
  int e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, a, e)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual int value is the same as expected,
/// if check fails then it calls methodToCallOnFailure before returning with failure
#define CHECK_INT_ADD_REPORT(actual, expected, methodToCallOnFailure) \
  { \
  int a = (actual); \
  int e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, a, e)) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual pointer value is the same as expected
#define CHECK_POINTER(actual, expected) \
  { \
  void* a = (actual); \
  void* e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckPointer(__LINE__,#actual " != " #expected, a, e)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual bool value is the same as expected
#define CHECK_BOOL(actual, expected) \
  { \
  bool a = (actual); \
  bool e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, a?1:0, e?1:0)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is the same as expected.
/// It can handle NULL pointer inputs.
/// Not recommended for comparing std::string values with c_str, because
/// if the string is a temporary object, created as a return value then
/// the string object may be deleted before the comparison happens.
#define CHECK_STRING(actual, expected) \
  { \
  const char* a = (actual); \
  const char* e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a, e)) \
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
  const char* a = (actual); \
  const char* e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a, e)) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is not the same as expected.
/// It can handle NULL pointer inputs.
#define CHECK_STRING_DIFFERENT(actual, expected) \
  { \
  const char* a = (actual); \
  const char* e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a, e, false)) \
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
  vtkMRMLScene* s = (scene); \
  const char* n = (nodeID); \
  vtkMRMLNode* e = (expected); \
  if (!vtkMRMLCoreTestingUtilities::CheckNodeInSceneByID(__LINE__, scene, nodeID, expected)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if the node's ID and name are the same as expected
#define CHECK_NODE_ID_AND_NAME(node, expectedID, expectedName) \
  { \
  vtkMRMLNode* n = (node); \
  const char* ei = (expectedID); \
  const char* en = (expectedName); \
  if (!vtkMRMLCoreTestingUtilities::CheckNodeIdAndName(__LINE__, n, ei, en)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

#include "vtkMRMLCoreTestingUtilities.txx"

#endif
