/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerLogic.h"

#include "vtkCallbackCommand.h"

vtkCxxRevisionMacro(vtkSlicerLogic, "$Revision: 1.9.12.1 $");
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
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
void 
vtkSlicerLogic::MRMLCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
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
// Description:
// the LogicCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
void 
vtkSlicerLogic::LogicCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
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

