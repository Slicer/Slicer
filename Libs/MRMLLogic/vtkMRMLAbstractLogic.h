/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractLogic.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/
///  vtkMRMLAbstractLogic - superclass for slicer logic classes
/// 
/// Superclass for all slicer logic classes.
/// There must be a corresponding vtkSlicerGUI subclass corresponding 
/// to each logic class that handles all GUI interaction (no GUI code
/// goes in the logic class).

#ifndef __vtkMRMLAbstractLogic_h
#define __vtkMRMLAbstractLogic_h

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkIntArray.h>
#include <vtkUnsignedLongArray.h>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkObserverManager.h"

#include "vtkMRMLLogicWin32Header.h"

//BTX

//----------------------------------------------------------------------------
// Convenient macros

//----------------------------------------------------------------------------
#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  {                                      \
  vtkObject *oldNode = (node);                                                  \
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) ); \
  if ( oldNode != (node) )                                                      \
    {                                                                           \
    this->InvokeEvent (vtkCommand::ModifiedEvent);                              \
    }                                                                           \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value)  {                                       \
  vtkObject *oldNode = (node);                                                             \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
  if ( oldNode != (node) )                                                                 \
    {                                                                                      \
    this->InvokeEvent (vtkCommand::ModifiedEvent);                                         \
    }                                                                                      \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveNoModifyMRMLNodeMacro
#define vtkSetAndObserveNoModifyMRMLNodeMacro(node,value)  {                               \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
};
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events)  {                           \
  vtkObject *oldNode = (node);                                                              \
  this->MRMLObserverManager->SetAndObserveObjectEvents (                                    \
     vtkObjectPointer( &(node)), (value), (events));                                        \
  if ( oldNode != (node) )                                                                  \
    {                                                                                       \
    this->InvokeEvent (vtkCommand::ModifiedEvent);                                          \
    }                                                                                       \
};
#endif

//ETX

class VTK_MRML_LOGIC_EXPORT vtkMRMLAbstractLogic : public vtkObject 
{
public:
  
  /// The Usual vtk class functions
  static vtkMRMLAbstractLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractLogic,vtkObject);

  /// Description
  /// All logic classes need to know about the current mrml scene
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);

  /// 
  /// API for setting and observing MRMLScene
  void SetMRMLScene(vtkMRMLScene *mrml);
  void SetAndObserveMRMLScene(vtkMRMLScene *mrml);
  void SetAndObserveMRMLSceneEvents(vtkMRMLScene *mrml, vtkIntArray *events);

  virtual void ProcessMRMLEvents(vtkObject * /*caller*/, 
                                 unsigned long /*event*/, void * /*callData*/){ };

  virtual void ProcessLogicEvents(vtkObject * /*caller*/, 
                                  unsigned long /*event*/, void * /*callData*/){ };

  virtual void ProcessLogicEvents(){ };

  /// 
  /// Name of this node
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);
  
  /// 
  /// Flags to avoid event loops
  /// NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  void SetInLogicCallbackFlag (int flag) {
    this->InLogicCallbackFlag = flag;
  }
  vtkGetMacro(InLogicCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);

  /// Additional functionality:

  /// Overload in modules that observe events, used during Loadable Module
  /// discovery. CLIENT MUST DELETE!
  virtual vtkIntArray* NewObservableEvents() { return vtkIntArray::New(); };

protected:
  vtkMRMLAbstractLogic();
  virtual ~vtkMRMLAbstractLogic();
  
  /// Register node classes into the mrml scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes(){}

  //BTX
  /// a shared set of functions that call the 
  /// virtual ProcessMRMLEvents and ProcessLogicEvents methods in the
  /// subclasses (if they are defined)
  static void MRMLCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);
  static void LogicCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);

  //ETX

  /// Holder for MRML and Logic callbacks
  vtkCallbackCommand * LogicCallbackCommand;
  vtkCallbackCommand * MRMLCallbackCommand;

  /// Flag to avoid event loops
  int InLogicCallbackFlag;
  int InMRMLCallbackFlag;


  vtkMRMLScene *       MRMLScene;
  vtkObserverManager * MRMLObserverManager;
  char *               Name;
  
private:
  vtkMRMLAbstractLogic(const vtkMRMLAbstractLogic&); // Not implemented
  void operator=(const vtkMRMLAbstractLogic&);       // Not implemented
};

#endif

