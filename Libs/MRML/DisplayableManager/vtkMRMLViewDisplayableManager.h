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

#ifndef __vtkMRMLViewDisplayableManager_h
#define __vtkMRMLViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLViewDisplayableManager : 
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLViewDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLViewDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLViewDisplayableManager();
  virtual ~vtkMRMLViewDisplayableManager();

  virtual void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData);
  virtual void ProcessWidgetsEvents(vtkObject *caller, unsigned long event, void *callData);

  virtual void AdditionalInitializeStep();

  virtual void Create();
  
private:

  vtkMRMLViewDisplayableManager(const vtkMRMLViewDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLViewDisplayableManager&);                     // Not Implemented
  
  class vtkInternal;
  vtkInternal * Internal;

};

#endif
