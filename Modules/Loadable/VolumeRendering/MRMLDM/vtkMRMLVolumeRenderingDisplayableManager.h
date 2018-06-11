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

//TODO: For testing, remove
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include "vtkMRMLVolumeNode.h"

class vtkSlicerVolumeRenderingLogic;

#define VTKIS_VOLUME_PROPS 100

/// \ingroup Slicer_QtModules_VolumeRendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVolumeRenderingDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLVolumeRenderingDisplayableManager *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  virtual void OnMRMLSceneStartClose() VTK_OVERRIDE;
  virtual void OnMRMLSceneEndClose() VTK_OVERRIDE;
  virtual void OnMRMLSceneEndImport() VTK_OVERRIDE;
  virtual void OnMRMLSceneEndRestore() VTK_OVERRIDE;
  virtual void OnMRMLSceneEndBatchProcess() VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void UnobserveMRMLScene() VTK_OVERRIDE;

  /// Update actors based on volumes in the scene
  virtual void UpdateFromMRML() VTK_OVERRIDE;

  /// Utility functions mainly used for testing
  vtkVolumeMapper* GetVolumeMapper(vtkMRMLVolumeNode* volumeNode);
  vtkVolume* GetVolumeActor(vtkMRMLVolumeNode* volumeNode);

public:
  static int DefaultGPUMemorySize;

protected:
  vtkMRMLVolumeRenderingDisplayableManager();
  ~vtkMRMLVolumeRenderingDisplayableManager();

  /// Initialize the displayable manager
  virtual void Create() VTK_OVERRIDE;

  /// Observe graphical resources created event
  void ObserveGraphicalResourcesCreatedEvent();

  virtual int ActiveInteractionModes() VTK_OVERRIDE;

  virtual void ProcessMRMLNodesEvents(vtkObject * caller, unsigned long event, void * callData) VTK_OVERRIDE;

  virtual void OnInteractorStyleEvent(int eventID) VTK_OVERRIDE;

protected:
  vtkSlicerVolumeRenderingLogic *VolumeRenderingLogic;

protected:
  vtkMRMLVolumeRenderingDisplayableManager(const vtkMRMLVolumeRenderingDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLVolumeRenderingDisplayableManager&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
