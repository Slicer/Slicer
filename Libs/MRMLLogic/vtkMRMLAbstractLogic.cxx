/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractLogic.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
//#include "vtkMRMLApplicationLogic.h"

// MRML includes
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

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

  vtkMRMLScene *           MRMLScene;
  vtkMRMLApplicationLogic* MRMLApplicationLogic;

  vtkObserverManager * MRMLSceneObserverManager;

  int                  InMRMLSceneCallbackFlag;
  int                  ProcessingMRMLSceneEvent;

  vtkObserverManager * MRMLNodesObserverManager;
  int                  InMRMLNodesCallbackFlag;

  bool                 DisableModifiedEvent;
  int                  ModifiedEventPending;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkInternal::vtkInternal()
{
  this->MRMLApplicationLogic = 0;
  this->MRMLScene = 0;

  this->MRMLSceneObserverManager = vtkObserverManager::New();
  this->InMRMLSceneCallbackFlag = false;
  this->ProcessingMRMLSceneEvent = 0;

  this->MRMLNodesObserverManager = vtkObserverManager::New();
  this->InMRMLNodesCallbackFlag = false;

  this->DisableModifiedEvent = false;
  this->ModifiedEventPending = 0;
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkInternal::~vtkInternal()
{
  this->MRMLSceneObserverManager->AssignOwner(0);
  this->MRMLSceneObserverManager->Delete();

  this->MRMLNodesObserverManager->AssignOwner(0);
  this->MRMLNodesObserverManager->Delete();
}

//----------------------------------------------------------------------------
// vtkMRMLAbstractLogic methods

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkMRMLAbstractLogic()
{
  this->Internal = new vtkInternal;

  // Setup MRML scene callback
  vtkObserverManager * sceneObserverManager = this->Internal->MRMLSceneObserverManager;
  sceneObserverManager->AssignOwner(this);
  sceneObserverManager->GetCallbackCommand()->SetClientData(reinterpret_cast<void *>(this));
  sceneObserverManager->GetCallbackCommand()->SetCallback(vtkMRMLAbstractLogic::MRMLSceneCallback);

  // Setup MRML nodes callback
  vtkObserverManager * nodesObserverManager = this->Internal->MRMLNodesObserverManager;
  nodesObserverManager->AssignOwner(this);
  nodesObserverManager->GetCallbackCommand()->SetClientData(reinterpret_cast<void *> (this));
  nodesObserverManager->GetCallbackCommand()->SetCallback(vtkMRMLAbstractLogic::MRMLNodesCallback);
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
vtkMRMLApplicationLogic* vtkMRMLAbstractLogic::GetMRMLApplicationLogic()const
{
  return this->Internal->MRMLApplicationLogic;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* logic)
{
  if (logic == this->Internal->MRMLApplicationLogic)
    {
    return;
    }
  this->Internal->MRMLApplicationLogic = logic;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::MRMLSceneCallback(vtkObject*caller, unsigned long eid,
                                             void* clientData, void* callData)
{
  vtkMRMLAbstractLogic *self = reinterpret_cast<vtkMRMLAbstractLogic *>(clientData);
  assert(vtkMRMLScene::SafeDownCast(caller));
  assert(caller == self->GetMRMLScene());

  if (self && !self->EnterMRMLSceneCallback())
    {
#ifdef _DEBUG
    vtkWarningWithObjectMacro(self, "vtkMRMLAbstractLogic ******* MRMLSceneCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic MRMLSceneCallback");

  self->SetInMRMLSceneCallbackFlag(self->GetInMRMLSceneCallbackFlag() + 1);
  int oldProcessingEvent = self->GetProcessingMRMLSceneEvent();
  self->SetProcessingMRMLSceneEvent(eid);
  self->ProcessMRMLSceneEvents(caller, eid, callData);
  self->SetProcessingMRMLSceneEvent(oldProcessingEvent);
  self->SetInMRMLSceneCallbackFlag(self->GetInMRMLSceneCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
// Description:
// the LogicCallback is a static function to relay modified events from the
// observed mrml node back into the gui layer for further processing
//
void vtkMRMLAbstractLogic::MRMLNodesCallback(vtkObject* caller, unsigned long eid,
                                             void* clientData, void* callData)
{
  vtkMRMLAbstractLogic *self = reinterpret_cast<vtkMRMLAbstractLogic *>(clientData);
  assert(vtkMRMLNode::SafeDownCast(caller));

  if (self && !self->EnterMRMLNodesCallback())
    {
#ifdef _DEBUG
    vtkWarningWithObjectMacro(self, "vtkMRMLAbstractLogic ******* MRMLNodesCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic MRMLNodesCallback");

  self->SetInMRMLNodesCallbackFlag(self->GetInMRMLNodesCallbackFlag() + 1);
  self->ProcessMRMLNodesEvents(caller, eid, callData);
  self->SetInMRMLNodesCallbackFlag(self->GetInMRMLNodesCallbackFlag() - 1);
}

//----------------------------------------------------------------------------
vtkMRMLScene * vtkMRMLAbstractLogic::GetMRMLScene()const
{
  return this->Internal->MRMLScene;
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkMRMLAbstractLogic::GetMRMLSceneCallbackCommand()
{
  return this->GetMRMLSceneObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkMRMLAbstractLogic::GetMRMLSceneObserverManager()const
{
  return this->Internal->MRMLSceneObserverManager;
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkMRMLAbstractLogic::GetMRMLNodesObserverManager()const
{
  return this->Internal->MRMLNodesObserverManager;
}

//----------------------------------------------------------------------------
vtkCallbackCommand* vtkMRMLAbstractLogic::GetMRMLNodesCallbackCommand()
{
  return this->GetMRMLNodesObserverManager()->GetCallbackCommand();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetMRMLScene(vtkMRMLScene * newScene)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  this->SetMRMLSceneInternal(newScene);

  this->RegisterNodes();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  this->GetMRMLSceneObserverManager()->SetObject(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLScene(vtkMRMLScene *newScene)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  this->SetAndObserveMRMLSceneInternal(newScene);

  this->RegisterNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneInternal(vtkMRMLScene *newScene)
{
  this->GetMRMLSceneObserverManager()->SetAndObserveObject(
    vtkObjectPointer(&this->Internal->MRMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneEvents(vtkMRMLScene *newScene, vtkIntArray *events)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);

  this->RegisterNodes();

  this->InvokeEvent (vtkCommand::ModifiedEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneEventsInternal(vtkMRMLScene *newScene,
                                                                vtkIntArray *events)
{
  this->GetMRMLSceneObserverManager()->SetAndObserveObjectEvents(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene, events);
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetProcessingMRMLSceneEvent()const
{
  return this->Internal->ProcessingMRMLSceneEvent;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetInMRMLNodesCallbackFlag(int flag)
{
  this->Internal->InMRMLNodesCallbackFlag = flag;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetProcessingMRMLSceneEvent(int event)
{
  this->Internal->ProcessingMRMLSceneEvent = event;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetInMRMLNodesCallbackFlag()const
{
  return this->Internal->InMRMLNodesCallbackFlag;
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetInMRMLSceneCallbackFlag(int flag)
{
  this->Internal->InMRMLSceneCallbackFlag = flag;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractLogic::GetInMRMLSceneCallbackFlag()const
{
  return this->Internal->InMRMLSceneCallbackFlag;
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractLogic::EnterMRMLSceneCallback()const
{
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractLogic::EnterMRMLNodesCallback()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractLogic
::ProcessMRMLSceneEvents(vtkObject *caller, unsigned long event, void *callData)
{
  assert(vtkMRMLScene::SafeDownCast(caller));
  assert(caller == this->GetMRMLScene());
#ifndef _NDEBUG
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
    case vtkMRMLScene::NewSceneEvent:
      this->OnMRMLSceneNewEvent();
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
    default:
      break;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractLogic
::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkMRMLNode * node = vtkMRMLNode::SafeDownCast(caller);
  assert(node);
  switch(event)
    {
    case vtkCommand::ModifiedEvent:
      this->OnMRMLNodeModified(node);
      break;
    default:
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
