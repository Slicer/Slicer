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
  vtkTypeMacro(vtkMRMLTestCustomDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // For testing
  static int NodeAddedCountThreeDView;
  static int NodeAddedCountSliceView;

protected:

  vtkMRMLTestCustomDisplayableManager();
  ~vtkMRMLTestCustomDisplayableManager() override;

  virtual void AdditionnalInitializeStep();
  void OnInteractorStyleEvent(int eventid) override;

  void Create() override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;

private:

  vtkMRMLTestCustomDisplayableManager(const vtkMRMLTestCustomDisplayableManager&) = delete;
  void operator=(const vtkMRMLTestCustomDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

#endif
