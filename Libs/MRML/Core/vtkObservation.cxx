/*=auto=========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObservation.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// MRML includes
#include "vtkObservation.h"
#include "vtkEventBroker.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkObservation);
vtkCxxSetObjectMacro(vtkObservation, CallbackCommand, vtkCallbackCommand);
vtkCxxSetObjectMacro(vtkObservation, EventBroker, vtkEventBroker);

//----------------------------------------------------------------------------
vtkObservation::vtkObservation()
{
  this->EventBroker = nullptr;
  this->InEventQueue = 0;
  this->Subject = nullptr;
  this->Event = 0;
  this->Observer = nullptr;
  this->CallbackCommand = nullptr;
  this->Script = nullptr;
  this->Comment = nullptr;
  this->Priority = 0.0f;
  this->EventTag = 0;
  this->SubjectDeleteEventTag = 0;
  this->ObserverDeleteEventTag = 0;

  this->ObservationCallbackCommand = vtkCallbackCommand::New();
  this->ObservationCallbackCommand->SetCallback( vtkEventBroker::Callback );
  this->ObservationCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );

  this->LastElapsedTime = 0.0;
  this->TotalElapsedTime = 0.0;
}

//----------------------------------------------------------------------------
vtkObservation::~vtkObservation()
{
  this->SetScript( nullptr );

  if (this->ObservationCallbackCommand != nullptr)
    {
    this->ObservationCallbackCommand->Delete();
    }

  if (this->CallbackCommand != nullptr)
    {
    this->CallbackCommand->Delete();
    }

  if (this->EventBroker != nullptr)
    {
    this->EventBroker->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkObservation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);


  if ( this->CallbackCommand ) os << indent << "EventBroker: " << this->EventBroker << "\n";
  else os << indent << "EventBroker: " << "(none) \n";

  if ( this->Subject ) os << indent << "Subject: " << this->Subject << "\n";
  else os << indent << "Subject: " << "(none) \n";

  os << indent << "Event: " << this->Event << "\n";

  if ( this->Observer ) os << indent << "Observer: " << this->Observer << "\n";
  else os << indent << "Observer: " << "(none) \n";

  if ( this->CallbackCommand ) os << indent << "CallbackCommand: " << this->CallbackCommand << "\n";
  else os << indent << "CallbackCommand: " << "(none) \n";

  os << indent << "Script: " <<
    (this->Script ? this->Script : "(none)") << "\n";

  os << indent << "Comment: " <<
    (this->Comment ? this->Comment : "(none)") << "\n";
  os << indent << "EventTag: " << this->EventTag << "\n";
  os << indent << "SubjectDeleteEventTag: " << this->SubjectDeleteEventTag << "\n";
  os << indent << "ObserverDeleteEventTag: " << this->ObserverDeleteEventTag << "\n";

  os << indent << "LastElapsedTime: " << this->LastElapsedTime << "\n";
  os << indent << "TotalElapsedTime: " << this->TotalElapsedTime << "\n";
}
