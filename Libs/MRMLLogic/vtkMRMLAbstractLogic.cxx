/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractLogic.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractLogic);
vtkCxxRevisionMacro(vtkMRMLAbstractLogic, "$Revision: 13525 $");

//----------------------------------------------------------------------------
class vtkMRMLAbstractLogic::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  vtkMRMLScene *       MRMLScene;

  vtkObserverManager * MRMLObserverManager;
  int                  InMRMLCallbackFlag;
  int                  ProcessingMRMLEvent;

  vtkCallbackCommand * LogicCallbackCommand;
  int                  InLogicCallbackFlag;
  
  bool                 DisableModifiedEvent;
  int                  ModifiedEventPending;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkInternal::vtkInternal()
{
  this->MRMLScene = 0;

  this->MRMLObserverManager = vtkObserverManager::New();
  this->InMRMLCallbackFlag = false;
  this->ProcessingMRMLEvent = 0;

  this->InLogicCallbackFlag = false;
  this->LogicCallbackCommand = vtkCallbackCommand::New();
  
  this->DisableModifiedEvent = false;
  this->ModifiedEventPending = 0;
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkInternal::~vtkInternal()
{
  this->MRMLObserverManager->AssignOwner(0);
  this->MRMLObserverManager->Delete();

  this->LogicCallbackCommand->Delete();
}

//----------------------------------------------------------------------------
// vtkMRMLAbstractLogic methods

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkMRMLAbstractLogic()
{
  this->Internal = new vtkInternal;

  // Setup MRML callback
  vtkObserverManager * observerManager = this->Internal->MRMLObserverManager;
  observerManager->AssignOwner(this);
  observerManager->GetCallbackCommand()->SetClientData(reinterpret_cast<void *>(this));
  observerManager->GetCallbackCommand()->SetCallback(vtkMRMLAbstractLogic::MRMLCallback);

  // Setup Logic callback
  this->Internal->LogicCallbackCommand->SetClientData(reinterpret_cast<void *> (this));
  this->Internal->LogicCallbackCommand->SetCallback(vtkMRMLAbstractLogic::LogicCallback);
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::~vtkMRMLAbstractLogic()
{
  this->SetAndObserveMRMLScene(0);
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ClassName:   " << this->GetClassName() << "\n";
  os << indent << "MRMLScene:   " << this->GetMRMLScene() << "\n";
}


//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::MRMLCallback(vtkObject *caller,unsigned long eid,
                                        void *clientData, void *callData)
{
  vtkMRMLAbstractLogic *self = reinterpret_cast<vtkMRMLAbstractLogic *>(clientData);

  if (self && !self->EnterMRMLCallback())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "vtkMRMLAbstractLogic ******* MRMLCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic MRMLCallback");

  self->SetInMRMLCallbackFlag(self->GetInMRMLCallbackFlag() + 1);
  int oldProcessingEvent = self->GetProcessingMRMLEvent();
  self->SetProcessingMRMLEvent(eid);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetProcessingMRMLEvent(oldProcessingEvent);
  self->SetInMRMLCallbackFlag(self->GetInMRMLCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
// Description:
// the LogicCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
void vtkMRMLAbstractLogic::LogicCallback(vtkObject *caller, unsigned long eid,
                                         void *clientData, void *callData)
{
  vtkMRMLAbstractLogic *self = reinterpret_cast<vtkMRMLAbstractLogic *>(clientData);

  if (self && !self->EnterLogicCallback())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "vtkMRMLAbstractLogic ******* LogicCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic LogicCallback");

  self->SetInLogicCallbackFlag(self->GetInLogicCallbackFlag() + 1);
  self->ProcessLogicEvents(caller, eid, callData);
  self->SetInLogicCallbackFlag(self->GetInLogicCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
vtkMRMLScene * vtkMRMLAbstractLogic::GetMRMLScene()
{
  return this->Internal->MRMLScene;
}

//----------------------------------------------------------------------------
vtkCallbackCommand * vtkMRMLAbstractLogic::GetMRMLCallbackCommand()
{
  return this->Internal->MRMLObserverManager->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkObserverManager * vtkMRMLAbstractLogic::GetMRMLObserverManager()
{
  return this->Internal->MRMLObserverManager;
}

//----------------------------------------------------------------------------
vtkCallbackCommand * vtkMRMLAbstractLogic::GetLogicCallbackCommand()
{
  return this->Internal->LogicCallbackCommand;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetMRMLScene(vtkMRMLScene * newScene)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  vtkObject *oldValue = this->Internal->MRMLScene;

  this->SetMRMLSceneInternal(newScene);

  this->RegisterNodes();

  if (oldValue != this->Internal->MRMLScene)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  this->Internal->MRMLObserverManager->SetObject(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLScene(vtkMRMLScene *newScene)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  vtkObject *oldValue = this->Internal->MRMLScene;

  this->SetAndObserveMRMLSceneInternal(newScene);

  this->RegisterNodes();

  if ( oldValue != this->Internal->MRMLScene )
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneInternal(vtkMRMLScene *newScene)
{
  this->Internal->MRMLObserverManager->SetAndObserveObject(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneEvents(vtkMRMLScene *newScene, vtkIntArray *events)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  vtkObject *oldValue = this->Internal->MRMLScene;

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);

  this->RegisterNodes();

  if ( oldValue != this->Internal->MRMLScene )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneEventsInternal(vtkMRMLScene *newScene,
                                                                vtkIntArray *events)
{
  this->Internal->MRMLObserverManager->SetAndObserveObjectEvents(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene, events);
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetProcessingMRMLEvent()
{
  vtkDebugMacro("returning Internal->ProcessingMRMLEvent of "
                << this->Internal->ProcessingMRMLEvent);
  return this->Internal->ProcessingMRMLEvent;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetInLogicCallbackFlag(int flag)
{
  this->Internal->InLogicCallbackFlag = flag;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetProcessingMRMLEvent(int event)
{
  this->Internal->ProcessingMRMLEvent = event;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetInLogicCallbackFlag()
{
  return this->Internal->InLogicCallbackFlag;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetInMRMLCallbackFlag(int flag)
{
  this->Internal->InMRMLCallbackFlag = flag;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetInMRMLCallbackFlag()
{
  return this->Internal->InMRMLCallbackFlag;
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractLogic::EnterMRMLCallback()const
{
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractLogic::EnterLogicCallback()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractLogic::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  assert(vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene());
#ifndef _DEBUG
  (void)caller;
#endif

  vtkMRMLNode * node = 0;

  switch(event)
    {
    case vtkMRMLScene::SceneAboutToBeClosedEvent:
      this->OnMRMLSceneAboutToBeClosedEvent();
      break;
    case vtkMRMLScene::SceneClosedEvent:
      this->OnMRMLSceneClosedEvent();
      break;
    case vtkMRMLScene::SceneAboutToBeImportedEvent:
      this->OnMRMLSceneAboutToBeImportedEvent();
      break;
    case vtkMRMLScene::SceneImportedEvent:
      this->OnMRMLSceneImportedEvent();
      break;
    case vtkMRMLScene::SceneRestoredEvent:
      this->OnMRMLSceneRestoredEvent();
      break;
    case vtkMRMLScene::NodeAddedEvent:
      node = reinterpret_cast<vtkMRMLNode*>(callData);
      assert(node);
      this->OnMRMLSceneNodeAddedEvent(node);
      break;
    case vtkMRMLScene::NodeRemovedEvent:
      node = reinterpret_cast<vtkMRMLNode*>(callData);
      assert(node);
      this->OnMRMLSceneNodeRemovedEvent(node);
      break;
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLAbstractLogic::GetDisableModifiedEvent()const
{
  return this->Internal->DisableModifiedEvent;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetDisableModifiedEvent(bool onOff)
{
  this->Internal->DisableModifiedEvent = onOff;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractLogic::Modified() 
{
  if (this->GetDisableModifiedEvent())
    {
    ++this->Internal->ModifiedEventPending;
    return;
    }
  this->Superclass::Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractLogic::InvokePendingModifiedEvent ()
{
  if ( this->Internal->ModifiedEventPending )
    {
    int oldModifiedEventPending = this->Internal->ModifiedEventPending;
    this->Internal->ModifiedEventPending = 0;
    this->Superclass::Modified();
    return oldModifiedEventPending;
    }
  return this->Internal->ModifiedEventPending;
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetPendingModifiedEventCount()const
{
  return this->Internal->ModifiedEventPending;
}
