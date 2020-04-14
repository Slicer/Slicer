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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLVolumeRenderingDisplayableManager_h
#define __vtkMRMLVolumeRenderingDisplayableManager_h

// VolumeRendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLDisplayableManagerExport.h"

// MRML DisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

class vtkSlicerVolumeRenderingLogic;
class vtkMRMLVolumeNode;
class vtkVolumeMapper;
class vtkVolume;

#define VTKIS_VOLUME_PROPS 100

/// \ingroup Slicer_QtModules_VolumeRendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVolumeRenderingDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLVolumeRenderingDisplayableManager *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void OnMRMLSceneStartClose() override;
  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneEndRestore() override;
  void OnMRMLSceneEndBatchProcess() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void UnobserveMRMLScene() override;

  /// Update actors based on volumes in the scene
  void UpdateFromMRML() override;

  /// Utility functions mainly used for testing
  vtkVolumeMapper* GetVolumeMapper(vtkMRMLVolumeNode* volumeNode);
  vtkVolume* GetVolumeActor(vtkMRMLVolumeNode* volumeNode);

  /// Find display node managed by the displayable manager at a specified world RAS position.
  /// \return Non-zero in case a node is found at the position, 0 otherwise
  int Pick3D(double ras[3]) override;

  /// Get the MRML ID of the picked node, returns empty string if no pick
  const char* GetPickedNodeID() override;

public:
  static int DefaultGPUMemorySize;

protected:
  vtkMRMLVolumeRenderingDisplayableManager();
  ~vtkMRMLVolumeRenderingDisplayableManager() override;

  /// Initialize the displayable manager
  void Create() override;

  /// Observe graphical resources created event
  void ObserveGraphicalResourcesCreatedEvent();

  int ActiveInteractionModes() override;

  void ProcessMRMLNodesEvents(vtkObject * caller, unsigned long event, void * callData) override;

  void OnInteractorStyleEvent(int eventID) override;

protected:
  vtkSlicerVolumeRenderingLogic *VolumeRenderingLogic{nullptr};

protected:
  vtkMRMLVolumeRenderingDisplayableManager(const vtkMRMLVolumeRenderingDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLVolumeRenderingDisplayableManager&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
