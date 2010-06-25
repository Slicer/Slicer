/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractLogic.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkMRMLAbstractLogic()
{
  this->Name = NULL;
  this->MRMLScene = NULL;
  this->InMRMLCallbackFlag = 0;
  this->InLogicCallbackFlag = 0;

  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->AssignOwner( this );
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkMRMLAbstractLogic::MRMLCallback);

  this->LogicCallbackCommand = vtkCallbackCommand::New();
  this->LogicCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->LogicCallbackCommand->SetCallback(vtkMRMLAbstractLogic::LogicCallback);

  this->MRMLCallbackCommand = this->MRMLObserverManager->GetCallbackCommand();
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic::~vtkMRMLAbstractLogic()
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
void vtkMRMLAbstractLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerLogic: " << this->GetClassName() << "\n";
  os << indent << "MRMLScene:   " << this->GetMRMLScene() << "\n";
}


//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
void 
vtkMRMLAbstractLogic::MRMLCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
{
  vtkMRMLAbstractLogic *self = reinterpret_cast<vtkMRMLAbstractLogic *>(clientData);

  if (self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic *********MRMLCallback called recursively?");
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
void vtkMRMLAbstractLogic::LogicCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
{
  vtkMRMLAbstractLogic *self = reinterpret_cast<vtkMRMLAbstractLogic *>(clientData);

  if (self->GetInLogicCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic *********LogicCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkMRMLAbstractLogic LogicCallback");

  self->SetInLogicCallbackFlag(1);
  self->ProcessLogicEvents(caller, eid, callData);
  self->SetInLogicCallbackFlag(0);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetMRMLScene ( vtkMRMLScene *mrml )
{
  vtkObject *oldValue = this->MRMLScene;
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->MRMLScene), mrml );
  this->RegisterNodes();
  if ( oldValue != this->MRMLScene )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
{
  vtkObject *oldValue = this->MRMLScene;
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), mrml );
  this->RegisterNodes();
  if ( oldValue != this->MRMLScene )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::SetAndObserveMRMLSceneEvents ( vtkMRMLScene *mrml, vtkIntArray *events )
{
  vtkObject *oldValue = this->MRMLScene;
  this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &this->MRMLScene), mrml, events );
  this->RegisterNodes();
  if ( oldValue != this->MRMLScene )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}
    
