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

#ifndef __vtkMRMLAbstractDisplayableManager_h
#define __vtkMRMLAbstractDisplayableManager_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLInteractionNode;
class vtkMRMLSelectionNode;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLLightBoxRendererManagerProxy;

class vtkRenderer;
class vtkRenderWindowInteractor;

/// \brief Superclass for displayable manager classes.
///
/// A displayable manager is responsible for representing MRML display nodes
/// into a renderer (e.g. 2D view renderer). Displayable managers are the interface
/// between MRML display nodes and vtkRenderer/vtkActors. They are responsible
/// for creating and synchronizing vtkActor, vtkMapper and vtkProperties with
/// MRML display nodes.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractDisplayableManager
  : public vtkMRMLAbstractLogic
{
public:
  static vtkMRMLAbstractDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkMRMLAbstractDisplayableManager, vtkMRMLAbstractLogic);

  /// Return True if Create() method has been invoked
  /// \sa CreateIfPossible() Create()
  bool IsCreated();

  /// Set the LightBoxRendererManager proxy. This proxy provides a
  /// method GetRenderer(int) that returns the renderer for the ith
  /// lightbox pane. The DisplayableManagers use this method to map
  /// coordinates to the proper lightbox pane, e.g. in placing
  /// crosshairs or annotations in the proper renderer.
  virtual void SetLightBoxRendererManagerProxy(vtkMRMLLightBoxRendererManagerProxy *);

  /// Get the LightBoxRendererManagerProxy if one has been provided
  /// \sa SetLightBoxRendererManagerProxy(vtkMRMLLightBoxRendererManagerProxy *)
  virtual vtkMRMLLightBoxRendererManagerProxy* GetLightBoxRendererManagerProxy();

  /// Get the default renderer for this displayable manager.
  vtkRenderer* GetRenderer();

  /// Get the renderer for the ith lightbox pane. This method uses the
  /// LightBoxRendererManagerProxy if one has been configured for the
  /// DisplayableManager. If no LightBoxRendererManagerProxy has been
  /// set, this method returns the default renderer by deferring to
  /// GetRenderer(),
  vtkRenderer* GetRenderer(int idx);

  /// Convenient method to get the WindowInteractor associated with the Renderer
  vtkRenderWindowInteractor* GetInteractor();

  /// Convenient method to get the current InteractionNode
  vtkMRMLInteractionNode* GetInteractionNode();

  /// Convenient method to get the current SelectionNode
  vtkMRMLSelectionNode* GetSelectionNode();

  /// Assemble and return info string to display in Data probe for a given viewer XYZ position.
  /// \return Invalid string by default, meaning no information to display.
  virtual std::string GetDataProbeInfoStringForPosition(double xyz[3]) { return ""; };

protected:

  vtkMRMLAbstractDisplayableManager();
  virtual ~vtkMRMLAbstractDisplayableManager();

  /// Get MRML Displayable Node
  vtkMRMLNode * GetMRMLDisplayableNode();

  /// Access to SetRenderer, SetMRMLDisplayableNode and CreateIfPossible methods
  friend class vtkMRMLDisplayableManagerGroup;

  virtual void SetMRMLDisplayableManagerGroup(vtkMRMLDisplayableManagerGroup* group);
  virtual void SetRenderer(vtkRenderer* newRenderer);

  /// Sub-class could overload that function and perform additional initialization steps
  /// \note Called by SetRenderer()
  /// \note Initialization occurs before the MRMLDisplayableNode is set and observed
  /// \warning That function should NOT be used directly !
  /// \sa SetRenderer
  virtual void AdditionalInitializeStep(){}

  /// Subclass can overload this method to specify under which InteractionNode modes
  /// this Displayable Manger InteractorStyle events.
  /// By default events only arrive when in Place mode (good for annotations)
  /// but if you want a continuous read out of, for example, slice positions while
  /// the mouse moves set this to include Place and ViewTransform
  virtual int ActiveInteractionModes();

  virtual void ProcessMRMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Receives all the events fired by any graphical object interacted by the
  /// user (typically vtk widgets).
  /// A typical use case is to listen to mrml nodes (using
  /// GetMRMLNodesCallbackCommand()) and update the graphical
  /// objects like mappers, actors, widgets... in ProcessMRMLNodesEvent, and to
  /// listen to user interactions (using (using
  /// GetWidgetsCallbackCommand()) like widgets and update the mrml nodes in
  /// ProcessWidgetsEvents.
  /// To listen to a widget (or any vtk Object), you can add an observer using
  /// GetWidgetsCallbackCommand().
  /// ProcessWidgetsEvents doesn't do anything by default, you need to reimplement
  /// it.
  virtual void ProcessWidgetsEvents(vtkObject* caller,
                                    unsigned long event,
                                    void * callData);

  /// WidgetsCallback is a static function to relay modified events from the vtk widgets
  static void WidgetsCallback(vtkObject *caller, unsigned long eid,
                              void *clientData, void *callData);

  /// Get vtkWidget callbackCommand
  vtkCallbackCommand * GetWidgetsCallbackCommand();

  /// Get widget observerManager
  vtkObserverManager * GetWidgetsObserverManager()const;

  /// Called by SetMRMLScene - Used to initialize the Scene
  /// Observe all the events of the scene and call OnMRMLSceneEndClose()
  /// or OnMRMLSceneEndImport() if the new scene is valid
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// ProcessMRMLNodesEvents calls OnMRMLDisplayableNodeModifiedEvent when the
  /// displayable node (e.g. vtkMRMLSliceNode, vtkMRMLViewNode) is Modified.
  /// Could be overloaded in DisplayableManager subclass.
  virtual void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// \brief Allow to specify additonal events that the DisplayableNode will observe
  /// \warning Should be called within AdditionalInitializeStep() method
  /// \sa AdditionalInitializeStep()
  void AddMRMLDisplayableManagerEvent(int eventId);

  /// Set MRML DisplayableNode
  /// Called by vtkMRMLDisplayableManagerFactory
  void SetAndObserveMRMLDisplayableNode(vtkMRMLNode * newMRMLDisplayableNode);

  /// Get associated DisplayableManager group
  vtkMRMLDisplayableManagerGroup * GetMRMLDisplayableManagerGroup();

  /// Invoke Create() and set Created flag to True
  /// A no-op if IsCreated() return True
  void CreateIfPossible();

  /// Called after a valid MRML DisplayableNode is set.
  /// By default it simulates a ModifiedEvent event on the displayable node
  /// so that ProcessMRMLNodesEvents(displayableNode, ModifiedEvent) is called.
  /// \note GetRenderer() and GetMRMLDisplayableNode() will return valid object
  virtual void Create();

  /// Remove MRML observers
  virtual void RemoveMRMLObservers();

  /// Specify if UodateFromMRML() should be called
  /// \sa UpdateFromMRML()
  void SetUpdateFromMRMLRequested(bool requested);

  /// Called from RequestRender method if UpdateFromMRMLRequested is true
  /// \sa RequestRender() SetUpdateFromMRMLRequested()
  virtual void UpdateFromMRML(){}

  /// Invoke vtkCommand::UpdateEvent and then call
  /// vtkMRMLThreeDViewDisplayableManagerFactory::RequestRender() which will also
  /// invoke vtkCommand::UpdateEvent.
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  void RequestRender();

  /// Usually used inside AdditionalInitializeStep()
  /// Allows to add observer to the current interactor style that will call the
  /// virtual method OnInteractorStyleEvent accordingly.
  /// \sa AdditionalInitializeStep RemoveInteractorStyleObservableEvent
  void AddInteractorStyleObservableEvent(int eventid, float priority=0.0);

  /// \sa AddInteractorStyleObservableEvent
  void RemoveInteractorStyleObservableEvent(int eventid);

  /// Usually used inside AdditionalInitializeStep()
  /// Allows to add observer to the current interactor that will call the
  /// virtual method OnInteractorEvent accordingly.
  /// \sa AdditionalInitializeStep RemoveInteractorObservableEvent
  void AddInteractorObservableEvent(int eventid, float priority=0.0);

  /// \sa AddInteractorObservableEvent
  void RemoveInteractorObservableEvent(int eventid);

  /// Called after interactor style event specified using AddInteractorStyleObservableEvent
  /// are invoked.
  /// \note The following events are observed by default:
  /// <ul>
  ///   <li>vtkCommand::LeftButtonPressEvent</li>
  ///   <li>vtkCommand::LeftButtonReleaseEvent</li>
  ///   <li>vtkCommand::RightButtonPressEvent</li>
  ///   <li>vtkCommand::RightButtonReleaseEvent</li>
  ///   <li>vtkCommand::MiddleButtonPressEvent</li>
  ///   <li>vtkCommand::MiddleButtonReleaseEvent</li>
  ///   <li>vtkCommand::MouseWheelBackwardEvent</li>
  ///   <li>vtkCommand::MouseWheelForwardEvent</li>
  ///   <li>vtkCommand::EnterEvent</li>
  ///   <li>vtkCommand::LeaveEvent</li>
  /// </ul>
  /// \sa AddInteractorStyleObservableEvent RemoveInteractorStyleObservableEvent
  virtual void OnInteractorStyleEvent(int eventid);

  /// Called after interactor event specified using
  /// AddInteractorObservableEvent are invoked
  /// \sa AddInteractorObservableEvent RemoveInteractorObservableEvent
  virtual void OnInteractorEvent(int eventid);

  /// Set the Abort flag on the InteractorStyle event callback
  void SetInteractorStyleAbortFlag(int f);
  int GetInteractorStyleAbortFlag();
  void InteractorStyleAbortFlagOn();
  void InteractorStyleAbortFlagOff();

  /// Set the Abort flag on the Interactor event callback
  void SetInteractorAbortFlag(int f);
  int GetInteractorAbortFlag();
  void InteractorAbortFlagOn();
  void InteractorAbortFlagOff();

private:

  vtkMRMLAbstractDisplayableManager(const vtkMRMLAbstractDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLAbstractDisplayableManager&);                    // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
