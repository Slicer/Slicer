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
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLViewDisplayableManager_h
#define __vtkMRMLViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

// MRML includes
class vtkMRMLCameraNode;

/// \brief Displayable manager that configures a vtkRenderer from a
/// vtkMRMLViewNode and its associated active vtkMRMLCameraNode.
/// It essentially configures:
///  - the axis cube
///  - the axis labels
///  - the background colors (gradient)
/// When the view is rotated, the "closest" axis label is hidden.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLViewDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLViewDisplayableManager* New();
  vtkTypeMacro(vtkMRMLViewDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLViewDisplayableManager();
  ~vtkMRMLViewDisplayableManager() override;

  /// Receives events from the view and the camera nodes.
  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;
  void ProcessWidgetsEvents(vtkObject *caller, unsigned long event, void *callData) override;

  void AdditionalInitializeStep() override;

  /// Observe the View node and initialize the renderer accordingly.
  void Create() override;

  /// Called each time the view node is modified.
  /// Internally update the renderer from the view node.
  /// \sa UpdateFromMRMLViewNode()
  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Update the renderer from the view node properties.
  /// \sa UpdateFromCameraNode
  void UpdateFromViewNode();

  /// Set, Observe and Update from the camera node.
  /// \sa UpdateFromMRMLCameraNode()
  void SetAndObserveCameraNode(vtkMRMLCameraNode* cameraNodeToObserve);

  /// Each time the camera is modified, the closest label to the camera is hidden.
  /// \sa SetAndObserveFromMRMLCameraNode, UpdateFromViewNode()
  void UpdateFromCameraNode();

private:

  vtkMRMLViewDisplayableManager(const vtkMRMLViewDisplayableManager&) = delete;
  void operator=(const vtkMRMLViewDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
