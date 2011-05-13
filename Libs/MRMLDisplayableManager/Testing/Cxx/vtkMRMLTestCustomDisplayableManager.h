/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLTestCustomDisplayableManager_h
#define __vtkMRMLTestCustomDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

class vtkMRMLCameraNode;

class vtkMRMLTestCustomDisplayableManager : 
  public vtkMRMLAbstractDisplayableManager
{

public:
  static vtkMRMLTestCustomDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLTestCustomDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  // For testing
  static int NodeAddedCountThreeDView;
  static int NodeAddedCountSliceView;

protected:

  vtkMRMLTestCustomDisplayableManager();
  virtual ~vtkMRMLTestCustomDisplayableManager();

  virtual void AdditionnalInitializeStep();
  virtual void OnInteractorStyleEvent(int eventid);

  virtual void Create();

//  virtual void OnMRMLSceneAboutToBeClosedEvent(){}
//  virtual void OnMRMLSceneClosedEvent(){}
//  virtual void OnMRMLSceneAboutToBeImportedEvent(){}
//  virtual void OnMRMLSceneImportedEvent(){}
//  virtual void OnMRMLSceneRestoredEvent(){}
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
//  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* /*node*/){}
  
private:

  vtkMRMLTestCustomDisplayableManager(const vtkMRMLTestCustomDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLTestCustomDisplayableManager&);                     // Not Implemented
  
  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif
