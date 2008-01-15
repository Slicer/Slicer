/*=auto=========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObservation.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkObservation.h"

#include "vtkCallbackCommand.h"
#include "vtkEventBroker.h"

vtkCxxRevisionMacro(vtkObservation, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkObservation);

//----------------------------------------------------------------------------
vtkObservation::vtkObservation()
{
  this->EventBroker = NULL;
  this->InEventQueue = 0;
  this->Subject = NULL;
  this->Event = 0;
  this->Observer = NULL;
  this->CallbackCommand = NULL;
  this->Comment = NULL;
  this->EventTag = 0;
  this->SubjectDeleteEventTag = 0;
  this->ObserverDeleteEventTag = 0;

  this->ObservationCallbackCommand = vtkCallbackCommand::New();
  this->ObservationCallbackCommand->SetCallback( vtkEventBroker::Callback ); 
  this->ObservationCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
}

//----------------------------------------------------------------------------
vtkObservation::~vtkObservation()
{
  if (this->ObservationCallbackCommand != NULL)
    {
    this->ObservationCallbackCommand->Delete();
    }

  if (this->CallbackCommand != NULL)
    {
    this->CallbackCommand->Delete();
    }

  if (this->EventBroker != NULL)
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

  os << indent << "Comment: " <<
    (this->Comment ? this->Comment : "(none)") << "\n";
  os << indent << "EventTag: " << this->EventTag << "\n";
  os << indent << "SubjectDeleteEventTag: " << this->SubjectDeleteEventTag << "\n";
  os << indent << "ObserverDeleteEventTag: " << this->ObserverDeleteEventTag << "\n";
}
