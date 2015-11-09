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
                 const char* current, const char* expected);

VTK_MRML_EXPORT
bool CheckNodeInSceneByID(int line, vtkMRMLScene* scene,
                          const char* nodeID, vtkMRMLNode* expected);

VTK_MRML_EXPORT
bool CheckNodeIdAndName(int line, vtkMRMLNode* node,
                        const char* expectedID, const char* expectedName);

template<typename Type>
std::string ToString(Type value);

}

#include "vtkMRMLCoreTestingUtilities.txx"

#endif
