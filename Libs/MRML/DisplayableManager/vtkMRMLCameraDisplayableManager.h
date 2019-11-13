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

  Based on Slicer/Base/GUI/vtkSlicerViewerWidget.h,
  this file was developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLCameraDisplayableManager_h
#define __vtkMRMLCameraDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLCameraNode;
class vtkMRMLCameraWidget;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCameraDisplayableManager :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLCameraDisplayableManager* New();
  vtkTypeMacro(vtkMRMLCameraDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void RemoveMRMLObservers() override;

  void UpdateCameraNode();

  vtkMRMLCameraNode* GetCameraNode();

//  void AddCameraObservers();
//  void RemoveCameraObservers();

  /// Events
  enum
  {
    ActiveCameraChangedEvent   = 30000
  };

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  vtkMRMLCameraWidget* GetCameraWidget();

protected:

  vtkMRMLCameraDisplayableManager();
  ~vtkMRMLCameraDisplayableManager() override;

  void Create() override;

  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneStartImport() override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneEndRestore() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  void ProcessMRMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData) override;
  void OnMRMLNodeModified(vtkMRMLNode* node) override;

  void SetAndObserveCameraNode(vtkMRMLCameraNode * newCameraNode);
  void AdditionalInitializeStep() override;
  void SetCameraToRenderer();
  void SetCameraToInteractor();

private:

  vtkMRMLCameraDisplayableManager(const vtkMRMLCameraDisplayableManager&) = delete;
  void operator=(const vtkMRMLCameraDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
