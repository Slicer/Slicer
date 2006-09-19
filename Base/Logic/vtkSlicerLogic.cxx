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
  this->ObserverTags  = vtkUnsignedLongArray::New();

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
  if (this->ObserverTags) 
    { 
    this->ObserverTags->Delete(); 
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
  this->RemoveMRMLEvents(*nodePtr);

  if (*nodePtr)
    {
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
  this->RemoveMRMLEvents(*nodePtr);

  if (*nodePtr)
    {
    (*nodePtr)->Delete();
    }

  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);

    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    this->AddMRMLEvents(*nodePtr, events);
    events->Delete();   
  }


}

//----------------------------------------------------------------------------
void vtkSlicerLogic::SetAndObserveMRMLEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events)
{
   this->RemoveMRMLEvents(*nodePtr);

   if (*nodePtr)
     {
     (*nodePtr)->Delete();
     }
   
  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);
    this->AddMRMLEvents(*nodePtr, events);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::RemoveMRMLEvents(vtkObject *nodePtr) 
{
  if (nodePtr)
    {
    //if (nodePtr->IsA("vtkMRMLSliceNode"))
    //std::cout << "remove\n";

    for (int i=0; i < this->ObserverTags->GetNumberOfTuples(); i++)
      {
      (nodePtr)->RemoveObservers(this->ObserverTags->GetValue(i) );
      }
    this->ObserverTags->Reset();
    this->Events->Reset();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLogic::AddMRMLEvents(vtkObject *nodePtr, vtkIntArray *events) 
{
  if (nodePtr)
    {
    //if (nodePtr->IsA("vtkMRMLSliceNode"))
    //std::cout <<"add\n";

    for (int i=0; i<events->GetNumberOfTuples(); i++)
      {
      unsigned long tag = nodePtr->AddObserver(events->GetValue(i), this->MRMLCallbackCommand );
      this->ObserverTags->InsertNextValue(tag);
      this->Events->InsertNextValue(events->GetValue(i));
      }
    }

}

//----------------------------------------------------------------------------
void vtkSlicerLogic::GetMRMLEvents(vtkIntArray *events) 
{
  events->Reset();
  if (this->Events) 
    {
    for (int i=0; i<this->Events->GetNumberOfTuples(); i++)
      {
      events->InsertNextValue(this->Events->GetValue(i));
      }
    }
}
