/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractLogic.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/
///  vtkMRMLAbstractLogic - Superclass for MRML logic classes
/// 
/// Superclass for all MRML logic classes.
/// There must be a corresponding UI class that handles all UI interaction

#ifndef __vtkMRMLAbstractLogic_h
#define __vtkMRMLAbstractLogic_h

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

//BTX

//----------------------------------------------------------------------------
// Convenient macros

//----------------------------------------------------------------------------
#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  {                                         \
  vtkObject *oldNode = (node);                                                     \
  this->GetMRMLObserverManager()->SetObject(vtkObjectPointer(&(node)), (value));   \
  if (oldNode != (node))                                                           \
    {                                                                              \
    this->InvokeEvent(vtkCommand::ModifiedEvent);                                  \
    }                                                                              \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value) {                                        \
  vtkObject *oldNode = (node);                                                             \
  this->GetMRMLObserverManager()->SetAndObserveObject(vtkObjectPointer(&(node)), (value)); \
  if (oldNode != (node))                                                                   \
    {                                                                                      \
    this->InvokeEvent(vtkCommand::ModifiedEvent);                                          \
    }                                                                                      \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveNoModifyMRMLNodeMacro
#define vtkSetAndObserveNoModifyMRMLNodeMacro(node,value) {                                 \
  this->GetMRMLObserverManager()->SetAndObserveObject(vtkObjectPointer(&(node)), (value));  \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events) {                        \
  vtkObject *oldNode = (node);                                                          \
  this->GetMRMLObserverManager()->SetAndObserveObjectEvents(                            \
     vtkObjectPointer(&(node)), (value), (events));                                     \
  if (oldNode != (node))                                                                \
    {                                                                                   \
    this->InvokeEvent(vtkCommand::ModifiedEvent);                                       \
    }                                                                                   \
};
#endif

//ETX

class VTK_MRML_LOGIC_EXPORT vtkMRMLAbstractLogic : public vtkObject 
{
public:
  
  static vtkMRMLAbstractLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractLogic, vtkObject);

  ///
  /// Return a reference to the current MRML scene
  vtkMRMLScene * GetMRMLScene();

  ///
  /// Set MRMLScene
  void SetMRMLScene(vtkMRMLScene * newScene);

  /// Set and observe MRML Scene. In order to provide a single method to set the scene,
  /// please consider overloading SetMRMLSceneInternal() instead.
  /// \note After each module are ported to Qt, these methods will be removed.
  /// \deprecated
  /// \sa SetMRMLSceneInternal()
  /// \sa SetAndObserveMRMLSceneInternal() SetAndObserveMRMLSceneEventsInternal()
  void SetAndObserveMRMLScene(vtkMRMLScene * newScene);
  void SetAndObserveMRMLSceneEvents(vtkMRMLScene * newScene, vtkIntArray * events);

  virtual void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void * callData);

  virtual void ProcessLogicEvents(vtkObject * /*caller*/, unsigned long /*event*/,
                                  void * /*callData*/){ };

  /// Get MRML CallbackCommand
  vtkCallbackCommand * GetMRMLCallbackCommand();

  /// Get MRML ObserverManager
  vtkObserverManager * GetMRMLObserverManager();

  /// Get InMRMLCallbackFlag
  /// \sa SetInMRMLCallbackFlag()
  int GetInMRMLCallbackFlag();

  /// Get Logic CallbackCommand
  vtkCallbackCommand * GetLogicCallbackCommand();

  /// Return the event id currently processed or 0 if any.
  int GetProcessingMRMLEvent();

  /// Get InLogicCallbackFlag
  /// \sa SetInLogicCallbackFlag()
  int GetInLogicCallbackFlag();

protected:

  vtkMRMLAbstractLogic();
  virtual ~vtkMRMLAbstractLogic();

  /// Called after the corresponding MRML event is triggered.
  /// \sa ProcessMRMLEvents
  virtual void OnMRMLSceneAboutToBeClosedEvent(){}
  virtual void OnMRMLSceneClosedEvent(){}
  virtual void OnMRMLSceneAboutToBeImportedEvent(){}
  virtual void OnMRMLSceneImportedEvent(){}
  virtual void OnMRMLSceneRestoredEvent(){}
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* /*node*/){}
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* /*node*/){}

  /// Called each time a new scene is set. Can be reimplemented in derivated classes.
  /// \sa SetAndObserveMRMLSceneInternal() SetAndObserveMRMLSceneEventsInternal()
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  /// Convenient method to set and observe the scene
  void SetAndObserveMRMLSceneInternal(vtkMRMLScene *newScene);
  void SetAndObserveMRMLSceneEventsInternal(vtkMRMLScene *newScene, vtkIntArray *events);
  
  //BTX
  /// Register node classes into the MRML scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes(){}

  /// Set InMRMLCallbackFlag flag
  /// True means ProcessMRMLEvent has already been called
  /// In MRMLCallback, loop are avoided by checking the value of the flag
  void SetInMRMLCallbackFlag(int flag);

  /// Set InLogicCallbackFlag flag
  /// True means ProcesslogicEvent has already been called
  /// In LogicCallback, loop are avoided by checking the value of the flag
  void SetInLogicCallbackFlag(int flag);

  /// Set event id currently processed or 0 if any.
  void SetProcessingMRMLEvent(int event);

  /// MRMLCallback is a static function to relay modified events from the MRML Scene
  /// In subclass, MRMLCallback can also be used to relay event from observe MRML node(s)
  static void MRMLCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);

  /// LogicCallback is a static function to relay modified events from the Logic
  static void LogicCallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData);
  //ETX
  
private:

  vtkMRMLAbstractLogic(const vtkMRMLAbstractLogic&); // Not implemented
  void operator=(const vtkMRMLAbstractLogic&);       // Not implemented

  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif

