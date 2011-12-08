/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractLogic.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/
///  vtkMRMLAbstractLogic - Superclass for MRML logic classes
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
#ifndef __vtkMRMLAbstractLogic_h
#define __vtkMRMLAbstractLogic_h

// MRMLLogic includes
class vtkMRMLApplicationLogic;

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkObserverManager.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkIntArray.h>
#include <vtkUnsignedLongArray.h>

#include "vtkMRMLLogicWin32Header.h"


//----------------------------------------------------------------------------
// Convenient macros

//----------------------------------------------------------------------------
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


class VTK_MRML_LOGIC_EXPORT vtkMRMLAbstractLogic : public vtkObject
{
public:
  /// Typedef for member functions of MRMLLogic that can be used as
  /// scheduled tasks.
  typedef void (vtkMRMLAbstractLogic::*TaskFunctionPointer)(void *clientdata);

  static vtkMRMLAbstractLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractLogic, vtkObject);

  /// Get access to overall application state
  virtual vtkMRMLApplicationLogic* GetMRMLApplicationLogic()const;
  virtual void SetMRMLApplicationLogic(vtkMRMLApplicationLogic* logic);

  /// Return a reference to the current MRML scene
  vtkMRMLScene * GetMRMLScene()const;

  /// Set and observe the MRMLScene
  void SetMRMLScene(vtkMRMLScene * newScene);

  /// <HACK> Still here for EMSegment </HACK>
  /// Set and observe MRML Scene. In order to provide a single method to set
  /// the scene, consider overloading SetMRMLSceneInternal().
  /// \note After each module are ported to Qt, these methods will be removed.
  ///  Use SetMRMLScene() instead.
  /// \deprecated
  /// \sa SetMRMLSceneInternal()
  /// \sa SetAndObserveMRMLSceneInternal() SetAndObserveMRMLSceneEventsInternal()
  void SetAndObserveMRMLScene(vtkMRMLScene * newScene);
  void SetAndObserveMRMLSceneEvents(vtkMRMLScene * newScene, vtkIntArray * events);

protected:

  vtkMRMLAbstractLogic();
  virtual ~vtkMRMLAbstractLogic();

  /// Receives all the events fired by the scene.
  /// By default, it calls OnMRMLScene*Event based on the event passed.
  virtual void ProcessMRMLSceneEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Receives all the events fired by the nodes.
  /// To listen to a node, you can add an observer using
  /// GetMRMLNodesCallbackCommand() or use the utility macros
  /// vtkSet[AndObserve]MRMLNode[Event]Macro
  /// ProcessMRMLNodesEvents calls OnMRMLNodeModified when event is
  /// vtkCommand::ModifiedEvent.
  virtual void ProcessMRMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  /// Get MRML scene callbackCommand
  /// You shouldn't have to use it manually, reimplementing
  /// SetMRMLSceneInternal and setting the events to listen should be enough.
  vtkCallbackCommand * GetMRMLSceneCallbackCommand();

  /// Get the MRML nodes callbackCommand. The Execute function associated
  /// the the callback calls ProcessMRMLNodesEvents.
  /// Only vtkMRMLNodes can be listened to.
  vtkCallbackCommand * GetMRMLNodesCallbackCommand();

  /// Get MRML scene observerManager. It points to the scene callback.
  /// \sa GetMRMLSceneCallbackCommand()
  vtkObserverManager * GetMRMLSceneObserverManager()const;

  /// Get MRML nodes observerManager. It points to the noes callback.
  /// \sa GetMRMLSceneCallbackCommand()
  vtkObserverManager * GetMRMLNodesObserverManager()const;

  /// Return 0 when not processing a MRML scene event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested event:
  /// processing a MRML scene event fires other scene events.
  /// \sa SetInMRMLCallbackFlag()
  int GetInMRMLSceneCallbackFlag()const;

  /// Return the event id currently processed or 0 if any.
  int GetProcessingMRMLSceneEvent()const;

  /// Return 0 when not processing any MRML node event, >0 otherwise.
  /// Values can be higher than 1 when receiving nested events:
  /// processing a MRML node event fires other node events.
  /// \sa SetMRMLNodesCallbackFlag()
  int GetInMRMLNodesCallbackFlag()const;

  /// Called anytime a scene is not set to the logic anymore (e.g. a new or
  /// no scene is set)
  /// Reimplement the method to delete all the scene specific information
  /// such as a node IDs, pointers...
  /// \sa SetMRMLSceneInternal, ObserveMRMLScene, UpdateFromMRMLScene
  virtual void UnobserveMRMLScene();
  /// Called after a scene is set to the logic. The scene events to observe
  /// are already set in SetMRMLSceneInternal.
  /// By default, ObserveMRMLScene() calls RegisterNodes and
  /// UpdateFromMRMLScene. Overide for a custom behavior.
  /// \sa SetMRMLSceneInternal, RegisterNodes, UnobserveMRMLScene
  /// \sa UpdateFromMRMLScene
  virtual void ObserveMRMLScene();
  /// Called everytime the scene has been significantly changed.
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
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  /// Convenient method to set and observe the scene
  void SetAndObserveMRMLSceneInternal(vtkMRMLScene *newScene);
  void SetAndObserveMRMLSceneEventsInternal(vtkMRMLScene *newScene, vtkIntArray *events);

  /// Register node classes into the MRML scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes(){}

  /// Set InMRMLCallbackFlag flag
  /// True means ProcessMRMLEvent has already been called
  /// In MRMLCallback, loop are avoided by checking the value of the flag
  void SetInMRMLSceneCallbackFlag(int flag);

  /// Return true if the MRML callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses
  virtual bool EnterMRMLSceneCallback()const;

  /// Set InLogicCallbackFlag flag
  /// True means ProcesslogicEvent has already been called
  /// In LogicCallback, loop are avoided by checking the value of the flag
  void SetInMRMLNodesCallbackFlag(int flag);

  /// Return true if the Logic callback must be executed, false otherwise.
  /// By default, it returns true, you can reimplement it in subclasses
  virtual bool EnterMRMLNodesCallback()const;

  /// Set event id currently processed or 0 if any.
  void SetProcessingMRMLSceneEvent(int event);

  /// MRMLCallback is a static function to relay modified events from the MRML Scene
  /// In subclass, MRMLCallback can also be used to relay event from observe MRML node(s)
  static void MRMLSceneCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// LogicCallback is a static function to relay modified events from the Logic
  static void MRMLNodesCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

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
  virtual void Modified();

  /// Invokes any modified events that are 'pending', meaning they were generated
  /// while the DisableModifiedEvent flag was nonzero.
  int InvokePendingModifiedEvent();

  int GetPendingModifiedEventCount()const;

private:

  vtkMRMLAbstractLogic(const vtkMRMLAbstractLogic&); // Not implemented
  void operator=(const vtkMRMLAbstractLogic&);       // Not implemented

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
