/*=auto=========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEventBroker.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCollection.h"
#include "vtkCallbackCommand.h"
#include "vtkEventBroker.h"
#include "vtkObservation.h"


vtkCxxRevisionMacro(vtkEventBroker, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkEventBroker);

//----------------------------------------------------------------------------
vtkEventBroker::vtkEventBroker()
{
  this->EventMode = vtkEventBroker::Synchronous;
  this->LogFile = NULL;
}

//----------------------------------------------------------------------------
vtkEventBroker::~vtkEventBroker()
{
  std::vector< vtkObservation *>::iterator iter; 
  for(iter=this->Observations.begin(); iter != this->Observations.end(); iter++)  
    { 
    this->DetachObservation (*iter);
    (*iter)->Delete();
    }
}


//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::AddObservation (
  vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify)
{
  vtkObservation *observation = vtkObservation::New();
  observation->SetEventBroker( this );
  this->Observations.push_back( observation );
  observation->AssignSubject( subject );
  observation->SetEvent( event );
  observation->AssignObserver( observer );
  observation->SetCallbackCommand( notify );

  this->AttachObservation( observation );
  return (observation);
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::AddObservation (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify, char *comment)
{
  vtkObservation *observation = this->AddObservation (subject, event, observer, notify);
  observation->SetComment (comment);
  return (observation);
};

//----------------------------------------------------------------------------
void vtkEventBroker::AttachObservation ( vtkObservation *observation )
{
  //
  // set up to be notified if either of the objects is delete
  // and when the specified event takes place
  // (remove any old notifications first)
  //

  this->DetachObservation ( observation );

  unsigned long tag;

  tag = observation->GetSubject()->AddObserver( vtkCommand::DeleteEvent, observation->GetObservationCallbackCommand() );
  observation->SetSubjectDeleteEventTag( tag );

  tag = observation->GetObserver()->AddObserver( vtkCommand::DeleteEvent, observation->GetObservationCallbackCommand() );
  observation->SetObserverDeleteEventTag( tag );

  tag = observation->GetSubject()->AddObserver( observation->GetEvent(), observation->GetObservationCallbackCommand() );
  observation->SetEventTag( tag );

}

//----------------------------------------------------------------------------
void vtkEventBroker::DetachObservation ( vtkObservation *observation )
{
  //
  // if the observation is attached, detach it
  //

  if ( observation->GetSubjectDeleteEventTag() )
    {
    observation->GetSubject()->RemoveObserver( observation->GetSubjectDeleteEventTag() );
    observation->SetSubjectDeleteEventTag( 0 );
    }

  if ( observation->GetObserverDeleteEventTag() )
    {
    observation->GetObserver()->RemoveObserver( observation->GetObserverDeleteEventTag() );
    observation->SetObserverDeleteEventTag( 0 );
    }

  if ( observation->GetEventTag() )
    {
    observation->GetObserver()->RemoveObserver( observation->GetEventTag() );
    observation->SetEventTag( 0 );
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservation ( vtkObservation *observation )
{
  // remove from observation list
  std::vector< vtkObservation *> newObservations;
  std::vector< vtkObservation *>::iterator obsIter; 
  for(obsIter=this->Observations.begin(); obsIter != this->Observations.end(); obsIter++)  
    { 
    if ( *obsIter != observation )
      {
      newObservations.push_back( *obsIter );
      }
    }
  this->Observations = newObservations;

  // remove from event queue
  std::deque< vtkObservation *> newEventQueue;
  std::deque< vtkObservation *>::iterator queueIter; 
  for(queueIter=this->EventQueue.begin(); queueIter != this->EventQueue.end(); queueIter++)  
    { 
    if ( *queueIter != observation )
      {
      newEventQueue.push_back( *queueIter );
      }
    }
  this->EventQueue = newEventQueue;

  this->DetachObservation( observation );
  observation->Delete();
}

//----------------------------------------------------------------------------
vtkCollection *vtkEventBroker::GetObservationsForSubject ( vtkObject *subject )
{
  vtkCollection *collection = vtkCollection::New();
  std::vector< vtkObservation *>::iterator iter; 
  for(iter=this->Observations.begin(); iter != this->Observations.end(); iter++)  
    { 
    if ( (*iter)->GetSubject() == subject )
      {
      collection->AddItem( *iter );
      }
    }
  return collection;
}

//----------------------------------------------------------------------------
vtkCollection *vtkEventBroker::GetObservationsForObserver ( vtkObject *observer )
{
  vtkCollection *collection = vtkCollection::New();
  std::vector< vtkObservation *>::iterator iter; 
  for(iter=this->Observations.begin(); iter != this->Observations.end(); iter++)  
    { 
    if ( (*iter)->GetObserver() == observer )
      {
      collection->AddItem( *iter );
      }
    }
  return collection;
}

//----------------------------------------------------------------------------
int vtkEventBroker::GetNumberOfObservations ( )
{
  return (this->Observations.size());
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::GetNthObservation ( int n )
{
  if ( n < 0 || n >= this->GetNumberOfObservations() )
    {
    return NULL;
    }
  return (this->Observations[n]);
}

//----------------------------------------------------------------------------
void vtkEventBroker::ProcessEvent ( vtkObservation *observation, vtkObject *caller, unsigned long eid, void *callData )
{

  // 
  // for delete events, just clean up and get out
  // - we want to forget we ever had this observation
  //
  if ( eid == vtkCommand::DeleteEvent )
    {
    this->RemoveObservation( observation );
    return;
    }

  //
  // otherwise, we've got an event we care about - either invoke it
  // right away, or put it on the queue for later handling
  //
  if ( this->EventMode == vtkEventBroker::Synchronous )
    {
    this->InvokeObservation( observation );
    }
  else if ( this->EventMode == vtkEventBroker::Asynchronous )
    {
    this->QueueObservation( observation );
    }
  else
    {
    vtkErrorMacro ( "Bad EventMode " << this->EventMode );
    }
}


//----------------------------------------------------------------------------
void vtkEventBroker::QueueObservation ( vtkObservation *observation )
{
  if ( observation->GetInEventQueue())
    {
    return;
    }
  this->EventQueue.push_back( observation );
  observation->SetInEventQueue(1);
}

//----------------------------------------------------------------------------
int vtkEventBroker::GetNumberOfQueuedObservations ()
{
  return( this->EventQueue.size() );
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::GetNthQueuedObservation ( int n )
{
  if ( n < 0 || n >= this->GetNumberOfQueuedObservations() )
    {
    return NULL;
    }
  return (this->EventQueue[n]);
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::DequeueObservation ()
{
  vtkObservation *observation = this->EventQueue.front();
  this->EventQueue.pop_front();
  observation->SetInEventQueue(0);
  return( observation );
}

//----------------------------------------------------------------------------
void vtkEventBroker::InvokeObservation ( vtkObservation *observation )
{
  observation->GetCallbackCommand()->Execute(
                                    observation->GetSubject(),
                                    observation->GetEvent(),
                                    NULL);
}

//----------------------------------------------------------------------------
void vtkEventBroker::ProcessEventQueue ()
{
  while ( this->GetNumberOfQueuedObservations() > 0 )
    {
    this->InvokeObservation( this->DequeueObservation() );
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  
  
  if ( this->CallbackCommand ) os << indent << "CallbackCommand: " << this->CallbackCommand << "\n";
  else os << indent << "CallbackCommand: " << "(none) \n";

  os << indent << "NumberOfObservations: " << this->GetNumberOfObservations() << "\n";
  os << indent << "NumberOfQueueObservations: " << this->GetNumberOfQueuedObservations() << "\n";
  os << indent << "EventMode: " << this->GetEventModeAsString() << "\n";
  os << indent << "EventLogging: " << this->EventLogging << "\n";
  os << indent << "LogFile: " <<
    (this->LogFile ? this->LogFile : "(none)") << "\n";
}

//----------------------------------------------------------------------------
// Description:
// the Callback is a static function to relay events 
//
void 
vtkEventBroker::Callback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
{
  vtkObservation *observation = reinterpret_cast<vtkObservation *>(clientData);
  vtkEventBroker *self = observation->GetEventBroker();

  vtkDebugWithObjectMacro(self, "In vtkEvenBroker Callback");

  self->ProcessEvent(observation, caller, eid, callData);
}

