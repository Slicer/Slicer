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

  vtkCallbackCommand * LogicCallbackCommand;
  int                  InLogicCallbackFlag;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkInternal::vtkInternal()
{
  this->MRMLScene = 0;

  this->MRMLObserverManager = vtkObserverManager::New();
  this->InMRMLCallbackFlag = false;

  this->InLogicCallbackFlag = false;
  this->LogicCallbackCommand = vtkCallbackCommand::New();
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

  if (self && self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "vtkMRMLAbstractLogic ******* MRMLCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic MRMLCallback");

  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
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

  if (self && self->GetInLogicCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "vtkMRMLAbstractLogic ******* LogicCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic LogicCallback");

  self->SetInLogicCallbackFlag(1);
  self->ProcessLogicEvents(caller, eid, callData);
  self->SetInLogicCallbackFlag(0);
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
  this->Internal->MRMLObserverManager->SetObject(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene);

  this->RegisterNodes();

  if (oldValue != this->Internal->MRMLScene)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLScene(vtkMRMLScene *newScene)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }
  vtkObject *oldValue = this->Internal->MRMLScene;
  this->Internal->MRMLObserverManager->SetAndObserveObject(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene);

  this->RegisterNodes();

  if ( oldValue != this->Internal->MRMLScene )
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneEvents(vtkMRMLScene *newScene, vtkIntArray *events)
{
  if (this->Internal->MRMLScene == newScene)
    {
    return;
    }

  vtkObject *oldValue = this->Internal->MRMLScene;
  this->Internal->MRMLObserverManager->SetAndObserveObjectEvents(
      vtkObjectPointer(&this->Internal->MRMLScene), newScene, events );

  this->RegisterNodes();

  if ( oldValue != this->Internal->MRMLScene )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
// NOTE: Do *NOT* use the SetMacro or it call modified itself and generate even more events !
void vtkMRMLAbstractLogic::SetInLogicCallbackFlag(int flag)
{
  this->Internal->InLogicCallbackFlag = flag;
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
    
