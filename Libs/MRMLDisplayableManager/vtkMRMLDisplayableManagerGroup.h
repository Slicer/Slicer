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

/// DisplayableManagerGroup is a collection of DisplayableManager
/// 
/// It also provides method allowing to either call RenderRequest
/// or SetAndObserveMRMLDisplayableNode on all member of the group.
/// 

#ifndef __vtkMRMLDisplayableManagerGroup_h
#define __vtkMRMLDisplayableManagerGroup_h

// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLDisplayableManagerFactory;
class vtkMRMLAbstractDisplayableManager;
class vtkMRMLNode;
class vtkRenderer;
class vtkRenderWindowInteractor;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerGroup : public vtkObject 
{
public:

  static vtkMRMLDisplayableManagerGroup *New();
  vtkTypeRevisionMacro(vtkMRMLDisplayableManagerGroup,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Set and observe DisplayableManager factory
  void SetAndObserveDisplayableManagerFactory(vtkMRMLDisplayableManagerFactory * factory);

  ///
  /// Add a DisplayableManager and initialize it if required
  void AddAndInitialize(vtkMRMLAbstractDisplayableManager * displayableManager);

  ///
  /// Return the number of DisplayableManager already added to the group
  int GetDisplayableManagerCount();

  ///
  /// Return a DisplayableManager given its class name
  vtkMRMLAbstractDisplayableManager*
      GetDisplayableManagerByClassName(const char* className);

  /// Set Renderer and Interactor
  /// No-op if already initialized.
  /// \sa IsInitialized
  void Initialize(vtkRenderer* newRenderer);

  ///
  /// Return True if Group has already been initialized
  bool IsInitialized();

  ///
  /// Convenient method to get the WindowInteractor associated with the Renderer
  vtkRenderWindowInteractor* GetInteractor();

  /// Invoke vtkCommand::UpdateEvent
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  /// \sa vtkMRMLAbstractDisplayableManager::RequestRender()
  void RequestRender();

  ///
  /// Get Renderer
  vtkRenderer* GetRenderer();

  /// Set / Get MRML Displayable Node
  vtkMRMLNode* GetMRMLDisplayableNode();
  void SetMRMLDisplayableNode(vtkMRMLNode* newMRMLDisplayableNode);

protected:

  vtkMRMLDisplayableManagerGroup();
  virtual ~vtkMRMLDisplayableManagerGroup();

  //BTX
  typedef vtkMRMLDisplayableManagerGroup Self;
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);
  /// Trigger upon a DisplayableManager is either registered or unregisterer from
  /// the associated factory
  void onDisplayableManagerFactoryRegisteredEvent(const char* displayableManagerName);
  void onDisplayableManagerFactoryUnRegisteredEvent(const char* displayableManagerName);
  //ETX

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

private:

  vtkMRMLDisplayableManagerGroup(const vtkMRMLDisplayableManagerGroup&);
  void operator=(const vtkMRMLDisplayableManagerGroup&);

};

#endif

