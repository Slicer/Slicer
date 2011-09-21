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

  /// Convenient method equivalent to call SetAndObserveDisplayableManagerFactory, SetRenderer,
  /// then instantiate and add all displayable managers registered within the \a factory.
  /// \sa SetAndObserveDisplayableManagerFactory SetRenderer
  /// \sa AddDisplayableManager InstantiateDisplayableManager
  void Initialize(vtkMRMLDisplayableManagerFactory * factory, vtkRenderer * renderer);

  ///
  /// Set and observe DisplayableManager factory
  void SetAndObserveDisplayableManagerFactory(vtkMRMLDisplayableManagerFactory * factory);

  ///
  /// Add a DisplayableManager and initialize it if required
  void AddDisplayableManager(vtkMRMLAbstractDisplayableManager * displayableManager);

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
  void SetRenderer(vtkRenderer* newRenderer);

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

  /// Returns true if the displayableManagerName corresponds to a valid
  /// displayable manager, false otherwise.
  /// To be valid, ad displayableManagerName must be the name of a VTK
  /// class that derives from vtkMRMLAbstractDisplayableManager or be a valid
  /// python script file name.
  static bool IsADisplayableManager(const char* displayableManagerName);

  /// Returns a new instance of a displayable manager defined by its name:
  /// VTK class name or python file name.
  /// You are responsible of the returned pointer.
  static vtkMRMLAbstractDisplayableManager* InstantiateDisplayableManager(
    const char* displayableManagerName);

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

