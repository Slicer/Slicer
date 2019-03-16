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

#ifndef __vtkMRMLScriptedDisplayableManager_h
#define __vtkMRMLScriptedDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerExport.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLScriptedDisplayableManager :
  public vtkMRMLAbstractDisplayableManager
{

public:
  static vtkMRMLScriptedDisplayableManager* New();
  vtkTypeMacro(vtkMRMLScriptedDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetPythonSource(const std::string& pythonSource);

protected:
  vtkMRMLScriptedDisplayableManager();
  ~vtkMRMLScriptedDisplayableManager() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  void ProcessMRMLSceneEvents(vtkObject *caller, unsigned long event, void *callData) override;
  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  void Create() override;

  void RemoveMRMLObservers() override;

  void UpdateFromMRML() override;

  void OnInteractorStyleEvent(int eventid) override;

  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

private:
  vtkMRMLScriptedDisplayableManager(const vtkMRMLScriptedDisplayableManager&) = delete;
  void operator=(const vtkMRMLScriptedDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
