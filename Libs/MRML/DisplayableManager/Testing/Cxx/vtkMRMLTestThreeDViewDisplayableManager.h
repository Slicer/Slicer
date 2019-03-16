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

#ifndef __vtkMRMLTestThreeDViewDisplayableManager_h
#define __vtkMRMLTestThreeDViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

class vtkMRMLCameraNode;

class vtkMRMLTestThreeDViewDisplayableManager :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLTestThreeDViewDisplayableManager* New();
  vtkTypeMacro(vtkMRMLTestThreeDViewDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // For testing
  static int NodeAddedCount;

protected:

  vtkMRMLTestThreeDViewDisplayableManager();
  ~vtkMRMLTestThreeDViewDisplayableManager() override;

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

  vtkMRMLTestThreeDViewDisplayableManager(const vtkMRMLTestThreeDViewDisplayableManager&) = delete;
  void operator=(const vtkMRMLTestThreeDViewDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
