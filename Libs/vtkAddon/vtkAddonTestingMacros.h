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

#ifndef __vtkAddonTestingMacros_h
#define __vtkAddonTestingMacros_h

// vtkAddon includes
#include <vtkAddonTestingUtilities.h>

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

/// Verifies that pointer is nullptr
#define CHECK_NULL(pointer) \
  { \
  const void* pointerValue = (pointer); \
  if (!vtkAddonTestingUtilities::CheckNull(__LINE__,#pointer " is not NULL", pointerValue)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies that pointer is not nullptr
#define CHECK_NOT_NULL(pointer) \
  { \
  if (!vtkAddonTestingUtilities::CheckNotNull(__LINE__,#pointer " is NULL", (pointer))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies that pointer is nullptr,
/// if check fails then it calls methodToCallOnFailure before returning with failure
#define CHECK_NOT_NULL_ADD_REPORT(pointer, methodToCallOnFailure) \
  { \
  if (!vtkAddonTestingUtilities::CheckNotNull(__LINE__,#pointer " is NULL", (pointer))) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

#define CHECK_EXIT_SUCCESS(actual) \
  { \
  if (!vtkAddonTestingUtilities::CheckInt(__LINE__,#actual " != EXIT_SUCCESS", (actual), EXIT_SUCCESS)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual double value is the same as expected
#define CHECK_DOUBLE(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::Check<double>(__LINE__,#actual " != " #expected, (actual), (expected), "CheckDouble")) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual double value is the same as expected, within the specified tolerance
#define CHECK_DOUBLE_TOLERANCE(actual, expected, tolerance) \
    { \
  if (!vtkAddonTestingUtilities::CheckDoubleTolerance(__LINE__,#actual " != " #expected " (tolerance: " #tolerance ")", (actual), (expected), (tolerance))) \
      { \
    return EXIT_FAILURE; \
      } \
    }

/// Verifies if actual int value is the same as expected
#define CHECK_INT(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual int value is the same as expected,
/// if check fails then it calls methodToCallOnFailure before returning with failure
#define CHECK_INT_ADD_REPORT(actual, expected, methodToCallOnFailure) \
  { \
  if (!vtkAddonTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual pointer value is the same as expected
#define CHECK_POINTER(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::CheckPointer(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual pointer value is the same as expected
#define CHECK_POINTER_DIFFERENT(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::CheckPointer(__LINE__,#actual " == " #expected, (actual), (expected), false)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual bool value is the same as expected
#define CHECK_BOOL(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::CheckInt(__LINE__,#actual " != " #expected, (actual)?1:0, (expected)?1:0)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is the same as expected.
/// It can handle nullptr pointer inputs.
#define CHECK_STRING(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::CheckString(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual std::string value is the same as expected.
/// It is safe to use for comparing std::string values.
/// It cannot handle nullptr pointer inputs.
#define CHECK_STD_STRING(actual, expected) \
  { \
  std::string a = (actual); \
  std::string e = (expected); \
  if (!vtkAddonTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a.c_str(), e.c_str())) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is the same as expected,
/// if check fails then it calls methodToCallOnFailure before returning with failure
/// It can handle nullptr pointer inputs.
#define CHECK_STRING_ADD_REPORT(actual, expected, methodToCallOnFailure) \
  { \
  if (!vtkAddonTestingUtilities::CheckString(__LINE__,#actual " != " #expected, (actual), (expected))) \
    { \
    methodToCallOnFailure; \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual const char* value is not the same as expected.
/// It can handle nullptr pointer inputs.
#define CHECK_STRING_DIFFERENT(actual, expected) \
  { \
  if (!vtkAddonTestingUtilities::CheckString(__LINE__,#actual " != " #expected, (actual), (expected), false)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

/// Verifies if actual std::string value is not the same as expected.
/// It is safe to use for comparing std::string values.
/// It cannot handle nullptr pointer inputs.
#define CHECK_STD_STRING_DIFFERENT(actual, expected) \
  { \
  std::string a = (actual); \
  std::string e = (expected); \
  if (!vtkAddonTestingUtilities::CheckString(__LINE__,#actual " != " #expected, a.c_str(), e.c_str(), false)) \
    { \
    return EXIT_FAILURE; \
    } \
  }

// Commonly used headers in tests
#include "vtkNew.h"
#include "vtkTestingOutputWindow.h"

#endif
