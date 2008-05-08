/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerLogic - superclass for slicer logic classes
// .SECTION Description
// Superclass for all slicer logic classes (application, views, slices).
// There must be a corresponding vtkSlicerGUI subclass corresponding 
// to each logic class that handles all GUI interaction (no GUI code
// goes in the logic class).

#ifndef __vtkSlicerLogic_h
#define __vtkSlicerLogic_h

#include "vtkCommand.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkUnsignedLongArray.h"

#include "vtkMRMLScene.h"
#include "vtkObserverManager.h"

#include <string>

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

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerLogic : public vtkObject 
{
  public:

  // Typedef for member functions of SlicerLogic that can be used as
  // scheduled tasks.
  //BTX
  typedef void (vtkSlicerLogic::*TaskFunctionPointer)(void *clientdata);
  //ETX
  
  // The Usual vtk class functions
  static vtkSlicerLogic *New();
  vtkTypeRevisionMacro(vtkSlicerLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description
  // All logic classes need to know about the current mrml scene
  vtkGetObjectMacro (MRMLScene, vtkMRMLScene);

  // Description:
  // API for setting or setting and observing MRMLScene
  void SetMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLSceneEvents ( vtkMRMLScene *mrml, vtkIntArray *events )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &this->MRMLScene), mrml, events );
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

  // Description:
  // Name of this node
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);
  
  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  void SetInLogicCallbackFlag (int flag) {
    this->InLogicCallbackFlag = flag;
  }
  vtkGetMacro(InLogicCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);

  // Additional functionality:

  // Overload in modules that observe events, used during Loadable Module
  // discovery. CLIENT MUST DELETE!
  virtual vtkIntArray* NewObservableEvents() { return vtkIntArray::New(); };

protected:
  vtkSlicerLogic();
  virtual ~vtkSlicerLogic();
  vtkSlicerLogic(const vtkSlicerLogic&);
  void operator=(const vtkSlicerLogic&);

  vtkMRMLScene *MRMLScene;

  char *Name;

  //BTX
  // a shared set of functions that call the 
  // virtual ProcessMRMLEvents and ProcessLogicEvents methods in the
  // subclasses (if they are defined)
  static void MRMLCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);
  static void LogicCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);

  //ETX

  // Description:
  // Holder for MRML and Logic callbacks
  vtkCallbackCommand *LogicCallbackCommand;

  // Description:
  // Flag to avoid event loops
  int InLogicCallbackFlag;
  int InMRMLCallbackFlag;


  vtkObserverManager *MRMLObserverManager;

};

#endif

