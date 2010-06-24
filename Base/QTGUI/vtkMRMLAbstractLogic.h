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
/// Superclass for all slicer logic classes (application, views, slices).
/// There must be a corresponding vtkSlicerGUI subclass corresponding 
/// to each logic class that handles all GUI interaction (no GUI code
/// goes in the logic class).

#ifndef __vtkMRMLAbstractLogic_h
#define __vtkMRMLAbstractLogic_h

#include "vtkCommand.h"

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkUnsignedLongArray.h"

#include "vtkMRMLScene.h"
#include "vtkObserverManager.h"

#include "qSlicerBaseQTGUIExport.h"

//BTX

#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveNoModifyMRMLNodeMacro
#define vtkSetAndObserveNoModifyMRMLNodeMacro(node,value)  { \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events)); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

//ETX

class Q_SLICER_BASE_QTGUI_EXPORT vtkMRMLAbstractLogic : public vtkObject 
{
  public:

  /// Typedef for member functions of SlicerLogic that can be used as
  /// scheduled tasks.
  //BTX
  typedef void (vtkMRMLAbstractLogic::*TaskFunctionPointer)(void *clientdata);
  //ETX
  
  /// The Usual vtk class functions
  static vtkMRMLAbstractLogic *New();
  vtkTypeRevisionMacro(vtkMRMLAbstractLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Description
  /// All logic classes need to know about the current mrml scene
  vtkGetObjectMacro (MRMLScene, vtkMRMLScene);

  /// 
  /// API for setting or setting and observing MRMLScene
  void SetMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    this->RegisterNodes();
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    this->RegisterNodes();
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLSceneEvents ( vtkMRMLScene *mrml, vtkIntArray *events )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &this->MRMLScene), mrml, events );
    this->RegisterNodes();
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };

  virtual void ProcessLogicEvents( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };

  virtual void ProcessLogicEvents() {};

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
  vtkMRMLAbstractLogic(const vtkMRMLAbstractLogic&);
  void operator=(const vtkMRMLAbstractLogic&);

  /// Register node classes into the mrml scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes();

  vtkMRMLScene *MRMLScene;

  char *Name;

  //BTX
  /// a shared set of functions that call the 
  /// virtual ProcessMRMLEvents and ProcessLogicEvents methods in the
  /// subclasses (if they are defined)
  static void MRMLCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);
  static void LogicCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);

  //ETX

  /// 
  /// Holder for MRML and Logic callbacks
  vtkCallbackCommand *LogicCallbackCommand;
  vtkCallbackCommand *MRMLCallbackCommand;

  /// 
  /// Flag to avoid event loops
  int InLogicCallbackFlag;
  int InMRMLCallbackFlag;


  vtkObserverManager *MRMLObserverManager;

};

#endif

