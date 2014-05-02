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

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLCameraNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCameraDisplayableManager :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLCameraDisplayableManager* New();
  vtkTypeMacro(vtkMRMLCameraDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void RemoveMRMLObservers();

  void UpdateCameraNode();

  vtkMRMLCameraNode* GetCameraNode();

//  void AddCameraObservers();
//  void RemoveCameraObservers();

  /// Events
  enum
  {
    ActiveCameraChangedEvent   = 30000
  };

protected:

  vtkMRMLCameraDisplayableManager();
  virtual ~vtkMRMLCameraDisplayableManager();

  virtual void Create();

  virtual void OnMRMLSceneEndClose();
  virtual void OnMRMLSceneStartImport();
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneEndRestore();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  virtual void ProcessMRMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData);
  virtual void OnMRMLNodeModified(vtkMRMLNode* node);

  void SetAndObserveCameraNode(vtkMRMLCameraNode * newCameraNode);
  virtual void AdditionalInitializeStep();
  void SetCameraToRenderer();
  void SetCameraToInteractor();

private:

  vtkMRMLCameraDisplayableManager(const vtkMRMLCameraDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLCameraDisplayableManager&);                     // Not Implemented

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
