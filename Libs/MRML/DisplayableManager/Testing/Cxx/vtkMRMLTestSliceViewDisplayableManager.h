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

#ifndef __vtkMRMLTestSliceViewDisplayableManager_h
#define __vtkMRMLTestSliceViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"

class vtkMRMLCameraNode;

class vtkMRMLTestSliceViewDisplayableManager :
  public vtkMRMLAbstractSliceViewDisplayableManager
{

public:
  static vtkMRMLTestSliceViewDisplayableManager* New();
  vtkTypeMacro(vtkMRMLTestSliceViewDisplayableManager,vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // For testing
  static int NodeAddedCount;

protected:

  vtkMRMLTestSliceViewDisplayableManager();
  ~vtkMRMLTestSliceViewDisplayableManager() override;

  virtual void AdditionnalInitializeStep();

  void Create() override;

  //  virtual void OnMRMLSceneAboutToBeClosedEvent(){}
  //  virtual void OnMRMLSceneClosedEvent(){}
  //  virtual void OnMRMLSceneAboutToBeImportedEvent(){}
  //  virtual void OnMRMLSceneImportedEvent(){}
  //  virtual void OnMRMLSceneRestoredEvent(){}
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  //  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* /*node*/){}

private:

  vtkMRMLTestSliceViewDisplayableManager(const vtkMRMLTestSliceViewDisplayableManager&) = delete;
  void operator=(const vtkMRMLTestSliceViewDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
