/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLAbstractLogic_h
#define __vtkMRMLAbstractLogic_h

// MRMLLogic includes
class vtkMRMLApplicationLogic;

// MRML includes
#include <vtkObserverManager.h>
class vtkMRMLNode;
class vtkMRMLScene;

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>
class vtkIntArray;
class vtkFloatArray;

#include "vtkMRMLLogicExport.h"


//----------------------------------------------------------------------------
// Convenient macros

//----------------------------------------------------------------------------
// It removes all the event observations associated with the old value.
#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  {                                    \
  vtkObject *_oldNode = (node);                                               \
  this->GetMRMLNodesObserverManager()->SetObject(                             \
    vtkObjectPointer(&(node)), (value));                                      \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveMRMLNodeMacro
/// \brief Set and observe a MRML node.
/// Replace the existing value of \a node with \a value. Unobserve the old node
/// and observe the ModifiedEvent of the new. When the new node is modified,
/// vtkMRMLAbstractLogic::ProcessMRMLNodesEvents is called which propagate the
/// call to vtkMRMLAbstractLogic::OnMRMLNodeModified(vtkMRMLNode*)
/// automatically.
/// \note Can't be used with objects other than vtkMRMLNodes
/// \code
/// void vtkMRMLMyLogic::SetMyNode(vtkMRMLNode* newNode)
/// {
///   vtkSetAndObserveMRMLNodeMacro(this->MyNode, newNode);
///   this->OnMRMLNodeModified(this->MyNode);
/// }
/// \endcode
/// \sa vtkMRMLAbstractLogic::ProcessMRMLNodesEvents(),
/// vtkMRMLAbstractLogic::OnMRMLNodeModified()
#define vtkSetAndObserveMRMLNodeMacro(node,value) {                           \
  vtkObject *_oldNode = (node);                                               \
  this->GetMRMLNodesObserverManager()->SetAndObserveObject(                   \
    vtkObjectPointer(&(node)), (value));                                      \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events) {              \
  vtkObject *_oldNode = (node);                                               \
  this->GetMRMLNodesObserverManager()->SetAndObserveObjectEvents(             \
     vtkObjectPointer(&(node)), (value), (events));                           \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

#ifndef vtkObserveMRMLNodeMacro
#define vtkObserveMRMLNodeMacro(node)                                         \
{                                                                             \
  this->GetMRMLNodesObserverManager()->ObserveObject( (node) );               \
};
#endif


#ifndef vtkObserveMRMLNodeEventsMacro
#define vtkObserveMRMLNodeEventsMacro(node, events)                           \
{                                                                             \
  this->GetMRMLNodesObserverManager()->AddObjectEvents ( (node), (events) );  \
};
#endif

#ifndef vtkUnObserveMRMLNodeMacro
#define vtkUnObserveMRMLNodeMacro(node)                                       \
{                                                                             \
  this->GetMRMLNodesObserverManager()->RemoveObjectEvents ( (node) );         \
};
#endif

#ifndef vtkIsObservedMRMLNodeEventMacro
#define vtkIsObservedMRMLNodeEventMacro(node, event)                          \
  (                                                                           \
  this->GetMRMLNodesObserverManager()->GetObservationsCount(node, event) != 0 \
  )
#endif

/// \brief Superclass for MRML logic classes.
///
/// Superclass for all MRML logic classes.
/// When a scene is set, SetMRMLScene(vtkMRMLScene*),
/// - UnobserveMRMLScene() is called if a scene was previously set,
/// - SetMRMLSceneInternal() is called to observe the scene events
/// (e.g. StartImportEvent, EndBatchProcessEvent...)
/// - ObserveMRMLScene() is called to initialize the scene from the logic
/// - UpdateMRMLScene() is called to initialize the logic from the scene
/// Later, when events are fired by the scene, corresponding methods
/// (e.g. OnMRMLSceneNodeAdded, OnMRMLEndBatchProcess...) are called in the
/// logic if the events have been previously observed in SetMRMLSceneInternal()
class VTK_MRML_LOGIC_EXPORT vtkMRMLAbstractLogic : public vtkObject
{
public:
  /// Typedef for member functions of MRMLLogic that can be used as
  /// scheduled tasks.
  typedef void (vtkMRMLAbstractLogic::*TaskFunctionPointer)(void *clientdata);

  static vtkMRMLAbstractLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkMRMLAbstractLogic, vtkObject);

  /// Get access to overall application state
  virtual vtkMRMLApplicationLogic* GetMRMLApplicationLogic()const;
  virtual void SetMRMLApplicationLogic(vtkMRMLApplicationLogic* logic);

  /// Return a reference to the current MRML scene
  vtkMRMLScene * GetMRMLScene()const;

  /// Set and observe the MRMLScene
  void SetMRMLScene(vtkMRMLScene * newScene);

  /// @cond
  /// \deprecated Still here for EMSegment
  /// Set and observe MRML Scene. In order to provide a single method to set
  /// the scene, consider overloading SetMRMLSceneInternal().
  /// \note After each module are ported to Qt, these methods will be removed.
  ///  Use SetMRMLScene() instead.
  /// \sa SetMRMLSceneInternal()
  /// \sa SetAndObserveMRMLSceneInternal() SetAndObserveMRMLSceneEventsInternal()
  void SetAndObserveMRMLScene(vtkMRMLScene * newScene);
  void SetAndObserveMRMLSceneEvents(vtkMRMLScene * newScene, vtkIntArray * events, vtkFloatArray* priorities=nullptr);
  /// @endcond

protected:

  vtkMRMLAbstractLogic();
  ~vtkMRMLAbstractLogic() override;

  /// Receives all the events fired by the scene.
  /// By default, it calls OnMRMLScene*Event based on the event passed.
  virtual void ProcessMRMLSceneEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Receives all the events fired by the nodes.
  /// To listen to a node, you can add an observer using
  /// GetMRMLNodesCallbackCommand() or use the utility macros
  /// vtk[SetAndObserve|Observe]MRMLNode[Event]Macro
  /// ProcessMRMLNodesEvents calls OnMRMLNodeModified when event is
  /// vtkCommand::ModifiedEvent.
  /// \sa ProcessMRMLSceneEvents, ProcessMRMLLogicsEvents,
  /// OnMRMLNodeModified(), vtkSetAndObserveMRMLNodeMacro,
  /// vtkObserveMRMLNodeMacro, vtkSetAndObserveMRMLNodeEventMacro
  virtual void ProcessMRMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Receives all the events fired by the logics.
  /// To listen to a logic, you can add an observer using
  /// GetMRMLLogicsCallbackCommand().
  /// To be reimplemented in subclasses if needed.
  /// \sa GetMRMLLogicsCallbackCommand() ,ProcessMRMLSceneEvents(),
  /// ProcessMRMLNodesEvents()
  virtual void ProcessMRMLLogicsEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Get MRML scene callbackCommand.
  /// You shouldn't have to use it manually, reimplementing
  /// SetMRMLSceneInternal and setting the events to listen should be enough.
  /// \sa SetMRMLSceneInternal()
  vtkCallbackCommand * GetMRMLSceneCallbackCommand();

  /// Get the MRML nodes callbackCommand. The Execute function associated
  /// the the callback calls ProcessMRMLNodesEvents.
  /// Only vtkMRMLNodes can be listened to.
  /// \sa ProcessMRMLNodesEvents()
  vtkCallbackCommand * GetMRMLNodesCallbackCommand();

  /// Get the MRML Logic callback command.
  /// \sa GetMRMLSceneCallbackCommand(), GetMRMLNodesCallbackCommand()
  vtkCallbackCommand * GetMRMLLogicsCallbackCommand();

  /// Get MRML scene observerManager. It points to the scene callback.
  /// \sa GetMRMLSceneCallbackCommand()
  vtkObserverManager * GetMRMLSceneObserverManager()const;

  /// Get MRML nodes observerManager. It points to the nodes callback.
  /// \sa GetMRMLNodesCallbackCommand()
  vtkObserverManager * GetMRMLNodesObserverManager()const;

  /// Get MRML logics observerManager. It points to the logics callback.
  /// \sa GetMRMLLogicsCallbackCommand()
  vtkObserverManager * GetMRMLLogicsObserverManager()const;

  /// Return the event id currently processed or 0 if any.
  int GetProcessingMRMLSceneEvent()const;

  /// Called anytime a scene is not set to the logic anymore (e.g. a new or
  /// no scene is set)
  /// Reimplement the method to delete all the scene specific information
  /// such as a node IDs, pointers...
  /// \sa SetMRMLSceneInternal, ObserveMRMLScene, UpdateFromMRMLScene
  virtual void UnobserveMRMLScene();
  /// Called after a scene is set to the logic and nodes are registered
  /// (RegisterNodes()).
  /// The scene events to observe are already set in SetMRMLSceneInternal().
  /// By default, ObserveMRMLScene() calls UpdateFromMRMLScene().
  /// Override for a custom behavior.
  /// \sa SetMRMLSceneInternal, RegisterNodes, UnobserveMRMLScene
  /// \sa UpdateFromMRMLScene
  virtual void ObserveMRMLScene();
  /// Called every time the scene has been significantly changed.
  /// If the scene BatchProcessState events are observed (in
  /// SetMRMLSceneInternal() ), UpdateFromMRMLScene is called after each
  /// batch process (Close, Import, Restore...). It is also being called by
  /// default when a new scene is set (SetMRMLScene).
  /// \sa SetMRMLSceneInternal, UnobserveMRMLScene, ObserveMRMLScene
  virtual void UpdateFromMRMLScene();

  /// If vtkMRMLScene::StartBatchProcessEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneEndBatchProcess
  virtual void OnMRMLSceneStartBatchProcess(){}
  /// If vtkMRMLScene::EndBatchProcessEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// Internally calls UpdateFromMRMLScene.
  /// Can be reimplemented to change the default behavior.
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneStartBatchProcess
  virtual void OnMRMLSceneEndBatchProcess();
  /// If vtkMRMLScene::StartCloseEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneEndClose
  virtual void OnMRMLSceneStartClose(){}
  /// If vtkMRMLScene::EndCloseEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneStartClose
  virtual void OnMRMLSceneEndClose(){}
  /// If vtkMRMLScene::StartImportEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneEndImport, OnMRMLSceneNew
  virtual void OnMRMLSceneStartImport(){}
  /// If vtkMRMLScene::EndImportEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneStartImport, OnMRMLSceneNew
  virtual void OnMRMLSceneEndImport(){}
  /// If vtkMRMLScene::StartRestoreEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneEndRestore
  virtual void OnMRMLSceneStartRestore(){}
  /// If vtkMRMLScene::EndRestoreEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneStartRestore
  virtual void OnMRMLSceneEndRestore(){}
  /// If vtkMRMLScene::SceneNewEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneStartImport, OnMRMLSceneEndImport
  virtual void OnMRMLSceneNew(){}
  /// If vtkMRMLScene::NodeAddedEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneNodeRemoved, vtkMRMLScene::NodeAboutToBeAdded
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* /*node*/){}
  /// If vtkMRMLScene::NodeRemovedEvent has been set to be observed in
  ///  SetMRMLSceneInternal, it is called when the scene fires the event
  /// \sa ProcessMRMLSceneEvents, SetMRMLSceneInternal
  /// \sa OnMRMLSceneNodeAdded, vtkMRMLScene::NodeAboutToBeRemoved
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* /*node*/){}

  /// Called after the corresponding MRML event is triggered.
  /// \sa ProcessMRMLNodesEvents
  virtual void OnMRMLNodeModified(vtkMRMLNode* /*node*/){}

  /// Called each time a new scene is set. Can be reimplemented in derivated classes.
  /// Doesn't observe the scene by default, that means that
  /// UpdateFromMRMLScene() won't be called by default when a scene is imported,
  /// closed or restored, only when a new scene is set.
  /// \sa SetAndObserveMRMLSceneInternal() SetAndObserveMRMLSceneEventsInternal()
  /// \sa UpdateFromMRMLScene()
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// @cond
  /// Convenient method to set and observe the scene.
  /// \deprecated The ModifiedEvent on the scene is deprecated.
  void SetAndObserveMRMLSceneInternal(vtkMRMLScene *newScene);
  /// @endcond

  /// Typically called by a subclass in the derived SetMRMLSceneInternal to
  /// observe specific node events.
  /// \code
  /// void vtkMRMLMyLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
  /// {
  ///   vtkNew<vtkIntArray> events;
  ///   events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  ///   events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  ///   this->SetAndObserveMRMLSceneEventsInternal(newScene, events);
  /// }
  /// \endcode
  /// \sa SetMRMLSceneInternal()
  void SetAndObserveMRMLSceneEventsInternal(vtkMRMLScene *newScene,
                                            vtkIntArray *events,
                                            vtkFloatArray *priorities=nullptr);

  /// Register node classes into the MRML scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes(){}

  /// Set MRMLSceneCallback flag
  /// True means ProcessMRMLEvent has already been called
  /// In MRMLSceneCallback, loop are avoided by checking the value of the flag
  /// \sa EnterMRMLSceneCallback()
  void SetInMRMLSceneCallbackFlag(int flag);

  /// Return 0 when not processing a MRML scene event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested event:
  /// processing a MRML scene event fires other scene events.
  /// \sa SetInMRMLCallbackFlag()
  int GetInMRMLSceneCallbackFlag()const;

  /// Return true if the MRML callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses
  virtual bool EnterMRMLSceneCallback()const;

  /// Set event id currently processed or 0 if any.
  /// \sa EnterMRMLSceneCallback()
  void SetProcessingMRMLSceneEvent(int event);

  /// Set InMRMLNodesCallback flag.
  /// In InMRMLNodesCallback, loop are avoided by checking the value of the
  /// flag.
  /// \sa EnterMRMLNodesCallback()
  void SetInMRMLNodesCallbackFlag(int flag);

  /// Return 0 when not processing any MRML node event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested events:
  /// processing a MRML node event fires other node events.
  /// \sa SetMRMLNodesCallbackFlag()
  int GetInMRMLNodesCallbackFlag()const;

  /// Return true if the MRML Nodes callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses.
  /// \sa SetInMRMLNodesCallbackFlag()
  virtual bool EnterMRMLNodesCallback()const;

  /// Set InMRMLLogicsCallback flag.
  /// In InMRMLLogicsCallback, loop are avoided by checking the value of the
  /// flag.
  /// \sa EnterMRMLLogicsCallback()
  void SetInMRMLLogicsCallbackFlag(int flag);

  /// Return 0 when not processing any MRML logic event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested events:
  /// processing a MRML logic event fires other node events.
  /// \sa SetMRMLLogicsCallbackFlag()
  int GetInMRMLLogicsCallbackFlag()const;

  /// Return true if the Logics callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses
  virtual bool EnterMRMLLogicsCallback()const;

  /// MRMLSceneCallback is a static function to relay modified events from the MRML Scene
  /// In subclass, MRMLSceneCallback can also be used to relay event from observe MRML node(s)
  static void MRMLSceneCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// MRMLNodesCallback is a static function to relay modified events from the nodes
  static void MRMLNodesCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// MRMLLogicCallback is a static function to relay modified events from the logics
  static void MRMLLogicsCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// Start modifying the logic. Disable Modify events.
  /// Returns the previous state of DisableModifiedEvent flag
  /// that should be passed to EndModify() method
  inline bool StartModify() ;

  /// End modifying the node. Enable Modify events if the
  /// previous state of DisableModifiedEvent flag is 0.
  /// Return the number of pending ModifiedEvent;
  inline int EndModify(bool wasModifying);

  bool GetDisableModifiedEvent()const;
  void SetDisableModifiedEvent(bool onOff);

  /// overrides the vtkObject method so that all changes to the node which would normally
  /// generate a ModifiedEvent can be grouped into an 'atomic' operation.  Typical usage
  /// would be to disable modified events, call a series of Set* operations, and then re-enable
  /// modified events and call InvokePendingModifiedEvent to invoke the event (if any of the Set*
  /// calls actually changed the values of the instance variables).
  void Modified() override;

  /// Invokes any modified events that are 'pending', meaning they were generated
  /// while the DisableModifiedEvent flag was nonzero.
  int InvokePendingModifiedEvent();

  int GetPendingModifiedEventCount()const;

private:

  vtkMRMLAbstractLogic(const vtkMRMLAbstractLogic&) = delete;
  void operator=(const vtkMRMLAbstractLogic&) = delete;

  class vtkInternal;
  vtkInternal * Internal;

};

//---------------------------------------------------------------------------
bool vtkMRMLAbstractLogic::StartModify()
{
  bool disabledModify = this->GetDisableModifiedEvent();
  this->SetDisableModifiedEvent(true);
  return disabledModify;
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractLogic::EndModify(bool previousDisableModifiedEventState)
{
  this->SetDisableModifiedEvent(previousDisableModifiedEventState);
  if (!previousDisableModifiedEventState)
    {
    return this->InvokePendingModifiedEvent();
    }
  return this->GetPendingModifiedEventCount();
}

#endif
