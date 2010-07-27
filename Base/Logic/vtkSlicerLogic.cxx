/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

#include "vtkSlicerLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerLogic);

//----------------------------------------------------------------------------
vtkSlicerLogic::vtkSlicerLogic()
{
  this->Name = NULL;
  this->MRMLScene = NULL;
  this->InMRMLCallbackFlag = 0;
  this->InLogicCallbackFlag = 0;

  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->AssignOwner( this );
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkSlicerLogic::MRMLCallback);

  this->LogicCallbackCommand = vtkCallbackCommand::New();
  this->LogicCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->LogicCallbackCommand->SetCallback(vtkSlicerLogic::LogicCallback);

  this->MRMLCallbackCommand = this->MRMLObserverManager->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkSlicerLogic::~vtkSlicerLogic()
{
  this->SetName(NULL);

  this->SetAndObserveMRMLScene (NULL);

  if (this->MRMLObserverManager)
    {
    this->MRMLObserverManager->AssignOwner( NULL );
    this->MRMLObserverManager->Delete();
    }

  if (this->LogicCallbackCommand)
    {
    this->LogicCallbackCommand->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerLogic:             " << this->GetClassName() << "\n";
  os << indent << "MRMLScene: " << this->GetMRMLScene() << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLScene* vtkSlicerLogic::GetMRMLScene()
{
  return this->MRMLScene;
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetMRMLScene(vtkMRMLScene *newScene)
{
  vtkObject *oldValue = this->MRMLScene;

  this->SetMRMLSceneInternal(newScene);

  this->RegisterNodes();

  if (oldValue != this->MRMLScene)
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  this->MRMLObserverManager->SetObject(vtkObjectPointer(&this->MRMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRMLScene(vtkMRMLScene *newScene)
{
  vtkObject *oldValue = this->MRMLScene;

  this->SetAndObserveMRMLSceneInternal(newScene);

  this->RegisterNodes();

  if (oldValue != this->MRMLScene)
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRMLSceneInternal(vtkMRMLScene* newScene)
{
  this->MRMLObserverManager->SetAndObserveObject(vtkObjectPointer(&this->MRMLScene), newScene);
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRMLSceneEvents(vtkMRMLScene *newScene, vtkIntArray *events)
{
  vtkObject *oldValue = this->MRMLScene;

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);

  this->RegisterNodes();

  if (oldValue != this->MRMLScene)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRMLSceneEventsInternal(vtkMRMLScene* newScene,
                                                          vtkIntArray *events)
{
  this->MRMLObserverManager->SetAndObserveObjectEvents(
      vtkObjectPointer(&this->MRMLScene), newScene, events);
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::MRMLCallback(vtkObject *caller, unsigned long eid,
                                  void *clientData, void *callData)
{
  vtkSlicerLogic *self = reinterpret_cast<vtkSlicerLogic *>(clientData);

  if (self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerLogic *********MRMLCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerLogic MRMLCallback");

  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::LogicCallback(vtkObject *caller, unsigned long eid,
                                   void *clientData, void *callData)
{
  vtkSlicerLogic *self = reinterpret_cast<vtkSlicerLogic *>(clientData);

  if (self->GetInLogicCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerLogic *********LogicCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerLogic LogicCallback");

  self->SetInLogicCallbackFlag(1);
  self->ProcessLogicEvents(caller, eid, callData);
  self->SetInLogicCallbackFlag(0);
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::RegisterNodes()
{
}
