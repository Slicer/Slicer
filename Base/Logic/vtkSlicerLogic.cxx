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
  this->MRMLScene = NULL;
  this->InMRMLCallbackFlag = 0;
  this->InLogicCallbackFlag = 0;

  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(vtkSlicerLogic::MRMLCallback);

  this->LogicCallbackCommand = vtkCallbackCommand::New();
  this->LogicCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->LogicCallbackCommand->SetCallback(vtkSlicerLogic::LogicCallback);

  this->Events = vtkIntArray::New();
}

//----------------------------------------------------------------------------
vtkSlicerLogic::~vtkSlicerLogic()
{

  this->SetAndObserveMRMLScene (NULL);
      
  if (this->MRMLCallbackCommand)
    {
    this->MRMLCallbackCommand->Delete();
    this->MRMLCallbackCommand = NULL;
    }

  if (this->LogicCallbackCommand)
    {
    this->LogicCallbackCommand->Delete();
    this->LogicCallbackCommand = NULL;
    }
  if (this->Events)
    {
    this->Events->Delete();
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
    vtkErrorWithObjectMacro(self, "In vtkSlicerLogic *********MRMLCallback called recursively?");
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
    vtkErrorWithObjectMacro(self, "In vtkSlicerLogic *********LogicCallback called recursively?");
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerLogic LogicCallback");

  self->SetInLogicCallbackFlag(1);
  self->ProcessLogicEvents();
  self->SetInLogicCallbackFlag(0);
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetMRML(vtkObject **nodePtr, vtkObject *node)
{
  if ( *nodePtr  )
    {
    for (int i=0; i<this->Events->GetNumberOfTuples(); i++)
      {
      (*nodePtr)->RemoveObservers(this->Events->GetValue(i), this->MRMLCallbackCommand );
      }
    (*nodePtr)->Delete();
    }
  
  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRML(vtkObject **nodePtr, vtkObject *node)
{
  if ( *nodePtr  )
    {
    for (int i=0; i<this->Events->GetNumberOfTuples(); i++)
      {
      (*nodePtr)->RemoveObservers(this->Events->GetValue(i), this->MRMLCallbackCommand );
      }
    this->Events->Reset();
    (*nodePtr)->Delete();
    }
  
  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);
    this->Events->InsertNextValue(vtkCommand::ModifiedEvent);
    (*nodePtr)->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRMLEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events)
{
  if ( *nodePtr  )
    {
    for (int i=0; i<this->Events->GetNumberOfTuples(); i++)
      {
      (*nodePtr)->RemoveObservers(this->Events->GetValue(i), this->MRMLCallbackCommand );
      }
    this->Events->Reset();
    (*nodePtr)->Delete();
    }
  
  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);
    for (int i=0; i<events->GetNumberOfTuples(); i++)
      {
      this->Events->InsertNextValue(events->GetValue(i));
      (*nodePtr)->AddObserver(events->GetValue(i), this->MRMLCallbackCommand );
      }
    }
}


