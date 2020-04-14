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

// VTK includes
#include <vtkCallbackCommand.h>

// STD includes
#include <sstream>
#include <vector>
#include <map>

class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLDisplayableNode;
class vtkMRMLDisplayNode;
class vtkMRMLStorableNode;
class vtkMRMLStorageNode;
class vtkMRMLTransformableNode;
class vtkMRMLTransformNode;

/// This module provides functions to facilitate writing tests.
///
/// Usually these test methods are used by single-line convenience macros
/// defined in vtkMRMLCoreTestingMacros.h.

namespace vtkMRMLCoreTestingUtilities
{

VTK_MRML_EXPORT
bool CheckNodeInSceneByID(int line, vtkMRMLScene* scene,
                          const char* nodeID, vtkMRMLNode* expected);

VTK_MRML_EXPORT
bool CheckNodeIdAndName(int line, vtkMRMLNode* node,
                        const char* expectedID, const char* expectedName);

template<typename Type>
std::string ToString(Type value);

/// Test basic VTK object methods (print, superclass, etc.)
VTK_MRML_EXPORT
int ExerciseBasicObjectMethods(vtkObject* object);

/// Tests all basic MRML methods available for the current class.
/// Internally it calls ExerciseBasicObjectMethods, ExerciseBasicMRMLMethods,
/// ExerciseBasicTransformableMRMLMethods, ExerciseBasicStorableMRMLMethods, etc.
VTK_MRML_EXPORT
int ExerciseAllBasicMRMLMethods(vtkMRMLNode* object);

/// Slicer Libs/MRML/vtkMRMLNode exercises
VTK_MRML_EXPORT
int ExerciseBasicMRMLMethods(vtkMRMLNode* node);

/// For testing nodes in Libs/MRML that are storable. Calls the basic
/// mrml methods macro first.
VTK_MRML_EXPORT
int ExerciseBasicStorableMRMLMethods(vtkMRMLStorableNode* node);

/// For testing nodes in Libs/MRML that are transformable. Calls the basic
/// storable mrml methods macro first.
VTK_MRML_EXPORT
int ExerciseBasicTransformableMRMLMethods(vtkMRMLTransformableNode* node);

/// For testing nodes in Libs/MRML that are displayable. Calls the basic
/// transformable mrml methods macro first.
VTK_MRML_EXPORT
int ExerciseBasicDisplayableMRMLMethods(vtkMRMLDisplayableNode* node);

/// For testing nodes in Libs/MRML that are subclasses of the display node. Calls the basic
/// mrml methods macro first.
VTK_MRML_EXPORT
int ExerciseBasicDisplayMRMLMethods(vtkMRMLDisplayNode* node);

/// For testing nodes in Libs/MRML that are subclasses of the storage node. Calls the basic
/// mrml methods macro first.
VTK_MRML_EXPORT
int ExerciseBasicStorageMRMLMethods(vtkMRMLStorageNode* node);

/// For testing nodes in Libs/MRML that are transform nodes. Calls the basic
/// storable mrml methods macro first.
VTK_MRML_EXPORT
int ExerciseBasicTransformMRMLMethods(vtkMRMLTransformNode* node);

/// Test scene loading and import with a custom scene.
/// This is a utility function because scene import of custom MRML nodes cannot be tested
/// in the base MRML library.
/// If inputScene is provided then that scene will be used for testing scene loading. It is
/// needed when custom node registration is necessary in the scene.
VTK_MRML_EXPORT
int ExerciseSceneLoadingMethods(const char * sceneFilePath, vtkMRMLScene* inputScene = nullptr);

//---------------------------------------------------------------------------
class VTK_MRML_EXPORT vtkMRMLNodeCallback : public vtkCallbackCommand
{
public:
  static vtkMRMLNodeCallback *New() {return new vtkMRMLNodeCallback;}
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Execute(vtkObject* caller, unsigned long eid, void *callData) override;
  virtual void ResetNumberOfEvents();

  void SetMRMLNode(vtkMRMLNode*);
  std::string GetErrorString();

  /// Returns EXIT_SUCCESS if string is empty, EXIT_FAILURE if string is non-empty
  int CheckStatus();

  int GetNumberOfModified();
  int GetNumberOfEvents(unsigned long event);
  int GetTotalNumberOfEvents();
  std::vector<unsigned long> GetReceivedEvents();

protected:
  vtkMRMLNodeCallback();
  ~vtkMRMLNodeCallback() override;

  void SetErrorString(const char* error);

  void SetErrorString(int line, const char* error);

  vtkMRMLNode* Node{nullptr};
  std::string ErrorString;
  std::map<unsigned long, unsigned int> ReceivedEvents;
};

} // namespace vtkMRMLCoreTestingUtilities

#include "vtkMRMLCoreTestingUtilities.txx"

#endif
