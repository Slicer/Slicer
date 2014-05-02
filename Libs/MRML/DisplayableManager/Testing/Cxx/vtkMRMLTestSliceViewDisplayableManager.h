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
  void PrintSelf(ostream& os, vtkIndent indent);

  // For testing
  static int NodeAddedCount;

protected:

  vtkMRMLTestSliceViewDisplayableManager();
  virtual ~vtkMRMLTestSliceViewDisplayableManager();

  virtual void AdditionnalInitializeStep();

  virtual void Create();

  //  virtual void OnMRMLSceneAboutToBeClosedEvent(){}
  //  virtual void OnMRMLSceneClosedEvent(){}
  //  virtual void OnMRMLSceneAboutToBeImportedEvent(){}
  //  virtual void OnMRMLSceneImportedEvent(){}
  //  virtual void OnMRMLSceneRestoredEvent(){}
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  //  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* /*node*/){}

private:

  vtkMRMLTestSliceViewDisplayableManager(const vtkMRMLTestSliceViewDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLTestSliceViewDisplayableManager&);                     // Not Implemented

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
