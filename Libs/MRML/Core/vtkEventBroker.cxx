/*=auto=========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEventBroker.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// MRML includes
#include "vtkEventBroker.h"
#include "vtkObservation.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkObjectFactory.h>
#include <vtkTimerLog.h>

vtkCxxSetObjectMacro(vtkEventBroker, TimerLog, vtkTimerLog);

//----------------------------------------------------------------------------
// The IO manager singleton.
// This MUST be default initialized to zero by the compiler and is
// therefore not initialized here.  The ClassInitialize and
// ClassFinalize methods handle this instance.
static vtkEventBroker* vtkEventBrokerInstance;

//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int vtkEventBrokerInitialize::Count;

//----------------------------------------------------------------------------
// Implementation of vtkEventBrokerInitialize class.
//----------------------------------------------------------------------------
vtkEventBrokerInitialize::vtkEventBrokerInitialize()
{
  if(++Self::Count == 1)
    {
    vtkEventBroker::classInitialize();
    }
}

//----------------------------------------------------------------------------
vtkEventBrokerInitialize::~vtkEventBrokerInitialize()
{
  if(--Self::Count == 0)
    {
    vtkEventBroker::classFinalize();
    }
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkEventBroker* vtkEventBroker::New()
{
  vtkEventBroker* ret = vtkEventBroker::GetInstance();
  ret->Register(nullptr);
  return ret;
}

//----------------------------------------------------------------------------
// Return the single instance of the vtkEventBroker
vtkEventBroker* vtkEventBroker::GetInstance()
{
  if(!vtkEventBrokerInstance)
    {
    // Try the factory first
    vtkEventBrokerInstance = (vtkEventBroker*)vtkObjectFactory::CreateInstance("vtkEventBroker");
    // if the factory did not provide one, then create it here
    if(!vtkEventBrokerInstance)
      {
      vtkEventBrokerInstance = new vtkEventBroker;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
      vtkEventBrokerInstance->InitializeObjectBase();
#endif
      }
    }
  // return the instance
  return vtkEventBrokerInstance;
}

//----------------------------------------------------------------------------
vtkEventBroker::vtkEventBroker()
{
  this->EventMode = vtkEventBroker::Synchronous;
  this->EventLogging = 0;
  this->EventNestingLevel = 0;
  this->TimerLog = vtkTimerLog::New();
  this->CompressCallData = 0;
  this->LogFileName = nullptr;
  this->ScriptHandler = nullptr;
  this->ScriptHandlerClientData = nullptr;
}

//----------------------------------------------------------------------------
vtkEventBroker::~vtkEventBroker()
{
  /// fast and dangerous but ok because we are in the destructor.
  this->DetachObservations();

  // close the event log if needed
  if ( this->LogFile.is_open() )
    {
    this->CloseLogFile();
    this->EventLoggingOff();
    }

  if (this->TimerLog)
    {
    this->TimerLog->Delete();
    }
  //cout << "vtkEventBroker singleton Deleted" << endl;
}

//----------------------------------------------------------------------------
void vtkEventBroker::DetachObservations()
{
  // for each subject, remove observations in its list
  ObjectToObservationVectorMap::iterator mapiter;
  ObservationVector::iterator oiter;

  for (mapiter = this->SubjectMap.begin(); mapiter != this->SubjectMap.end(); mapiter++)
    {
    // clear out all the observation records
    for(oiter=(mapiter->second).begin(); oiter != (mapiter->second).end(); oiter++)
      {
      this->DetachObservation (*oiter);
      // Ideally the observation should be removed from SubjectMap and
      // ObserverMap. This is what RemoveObservations() does, but it takes
      // time.
      (*oiter)->Delete();
      }
    }
  this->SubjectMap.clear();
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::AddObservation (
  vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify, float priority)
{
  std::vector<vtkObject *>::iterator siter;

  vtkObservation *observation = vtkObservation::New();
  observation->SetEventBroker( this );
  this->SubjectMap[subject].insert( observation );
  this->ObserverMap[observer].insert( observation );
  observation->AssignSubject( subject );
  observation->SetEvent( event );
  observation->AssignObserver( observer );
  observation->SetCallbackCommand( notify );
  observation->SetPriority( priority );

  this->AttachObservation( observation );

  if ( this->EventLogging && this->LogFile.is_open() )
    {
    this->LogFile << "# AddObservation: (subject) " \
          << observation->GetSubject()->GetClassName() << observation->GetSubject()
          << " -> (observer) "
          << observation->GetObserver()->GetClassName()
          << " [ tag "
          << observation->GetEventTag()
          << "];\n" ;
    this->LogFile.flush();
    }

  return (observation);
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::AddObservation (
  vtkObject *subject, const char *event, const char *script)
{
  vtkObservation *observation = vtkObservation::New();
  observation->SetEventBroker( this );
  this->SubjectMap[subject].insert( observation );
  observation->AssignSubject( subject );

  // figure out event either as a predefined string, or
  // as an ascii number
  unsigned long eventID = vtkCommand::GetEventIdFromString( event );
  if ( eventID == vtkCommand::NoEvent )
    {
    eventID = static_cast<unsigned long> (atoi( event ));
    }
  observation->SetEvent( eventID );
  observation->SetScript( script );

  this->AttachObservation( observation );

  if ( this->EventLogging && this->LogFile.is_open() )
    {
    this->LogFile << "# AddObservation: (subject) " \
          << observation->GetSubject()->GetClassName() << observation->GetSubject()
          << " -> (script) "
          << observation->GetScript()
          << " [ tag "
          << observation->GetEventTag()
          << "];\n" ;
    this->LogFile.flush();
    }
  return (observation);
}

//----------------------------------------------------------------------------
void vtkEventBroker::AttachObservation ( vtkObservation *observation )
{
  //
  // set up to be notified if either of the objects is delete
  // and when the specified event takes place
  // (remove any old notifications first)
  //

  this->DetachObservation( observation );

  unsigned long tag;

  tag = observation->GetSubject()->AddObserver( vtkCommand::DeleteEvent, observation->GetObservationCallbackCommand());
  observation->SetSubjectDeleteEventTag( tag );

  if ( observation->GetObserver() != nullptr && (observation->GetSubject() != observation->GetObserver()) )
    {
    // there may be no Observer (e.g. for a Script)
    tag = observation->GetObserver()->AddObserver( vtkCommand::DeleteEvent, observation->GetObservationCallbackCommand() );
    observation->SetObserverDeleteEventTag( tag );
    }

  tag = observation->GetSubject()->AddObserver( observation->GetEvent(), observation->GetObservationCallbackCommand(), observation->GetPriority());
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
    observation->GetSubject()->RemoveObserver( observation->GetEventTag() );
    observation->SetEventTag( 0 );
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservation ( vtkObservation *observation )
{
  if (observation == nullptr)
    {
    return;
    }
  ObservationVector removeList;
  removeList.insert( observation );
  this->RemoveObservations( removeList );
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservations (ObservationVector observations)
{
  // remove passed observations from:
  // - broker's observation maps
  // -- remove each from list of observations per subject
  // -- remove each from list of observations per obserer
  // - current event queue
  // - detach from subject (and observer)
  // - delete the observation

  ObservationVector::iterator inObsIter;

  for(inObsIter=observations.begin(); inObsIter != observations.end(); inObsIter++)
    {
    vtkObservation *inObs = (*inObsIter);
    ObservationVector& subjectObservations = this->SubjectMap[(*inObsIter)->GetSubject()];
    subjectObservations.erase(subjectObservations.find(inObs));
    }

  for(inObsIter=observations.begin(); inObsIter != observations.end(); inObsIter++)
    {
    vtkObservation *inObs = (*inObsIter);
    ObservationVector& observerObservations = this->ObserverMap[inObs->GetObserver()];
    observerObservations.erase(observerObservations.find(inObs));
    }

  // remove from event queue
  std::deque< vtkObservation *>::iterator queueIter;
  for(queueIter=this->EventQueue.begin(); queueIter != this->EventQueue.end();)
    {
    // foreach of the broker's observations see if it is in the list of items to be removed
    if (observations.find(*queueIter)!=observations.end())
      {
      // these event is related to the observations to be deleted
      queueIter=this->EventQueue.erase(queueIter);
      }
    else
      {
      ++queueIter;
      }
    }

  // detach and delete each of the observations
  for(ObservationVector::iterator removeIter=observations.begin(); removeIter != observations.end(); removeIter++)
    {
    (*removeIter)->SetInEventQueue( 0 );
    this->DetachObservation( *removeIter );
    (*removeIter)->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservations (vtkObject *observer)
{
  this->RemoveObservations( this->GetObservations( observer ) );
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservations (vtkObject *subject, vtkObject *observer)
{
  this->RemoveObservations( this->GetObservations( subject, 0, observer ) );
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservations (vtkObject *subject, unsigned long event, vtkObject *observer)
{
  this->RemoveObservations( this->GetObservations( subject, event, observer ) );
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservations (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify)
{
  this->RemoveObservations( this->GetObservations( subject, event, observer, notify ) );
}

//----------------------------------------------------------------------------
void vtkEventBroker::RemoveObservationsForSubjectByTag (vtkObject *subject, unsigned long tag)
{
  this->RemoveObservations( this->GetObservationsForSubjectByTag ( subject, tag ) );
}

//----------------------------------------------------------------------------
vtkEventBroker::ObservationVector vtkEventBroker
::GetSubjectObservations (vtkObject *observer)
{
  // find matching observations to remove
  ObservationVector observationList = this->ObserverMap[observer];

  return( observationList );
}

//----------------------------------------------------------------------------
vtkEventBroker::ObservationVector vtkEventBroker::GetObservations (
  vtkObject *subject, unsigned long event,
  vtkObject *observer, vtkCallbackCommand *notify, unsigned int maxReturnedObservations/*=0*/)
{
  ObservationVector observationList;
  // Special case for fast return
  if (event == 0 && observer == nullptr && notify == nullptr)
    {
    observationList = this->GetSubjectObservations(subject);
    return observationList;
    }
  // find matching observations to remove
  ObservationVector& subjectList = this->SubjectMap[subject];

  for(ObservationVector::iterator obsIter = subjectList.begin();
      obsIter != subjectList.end();
      ++obsIter)
    {
    if ( (observer == nullptr || (*obsIter)->GetObserver() == observer) &&
         //(subject == 0 || (*obsIter)->GetSubject() == subject) &&
         (event == 0 || (*obsIter)->GetEvent() == event) &&
         (notify == nullptr || (*obsIter)->GetCallbackCommand() == notify))
      {
      observationList.insert( *obsIter );
      if (maxReturnedObservations && observationList.size()>=maxReturnedObservations)
        {
        // reached enough number of requested observations
        break;
        }
      }
    }
  return observationList;
}

//----------------------------------------------------------------------------
bool vtkEventBroker::GetObservationExist (
  vtkObject *subject, unsigned long event,
  vtkObject *observer, vtkCallbackCommand *notify)
{
  // request up to 1 element to see if any observation exist
  ObservationVector observationList = GetObservations (subject, event, observer, notify, 1);
  return (observationList.size()>0);
}

//----------------------------------------------------------------------------
vtkEventBroker::ObservationVector vtkEventBroker::GetObservationsForSubjectByTag (vtkObject *subject, unsigned long tag)
{
  // find matching observations to remove
  // - all tags match 0
  ObservationVector& subjectList = this->SubjectMap[subject];
  ObservationVector observationList;
  for (ObservationVector::iterator obsIter = subjectList.begin();
       obsIter != subjectList.end(); obsIter++)
    {
    vtkObservation *obs = *obsIter;
    if ( ( obs->GetSubject() == subject ) &&
         ( (tag == 0) || (obs->GetEventTag() == tag) ) )
      {
      observationList.insert( obs );
      }
    }
  return ( observationList );
}

//----------------------------------------------------------------------------
vtkCollection *vtkEventBroker::GetObservationsForSubject ( vtkObject *subject )
{
  vtkCollection *collection = vtkCollection::New();
  ObservationVector& subjectList = this->SubjectMap[subject];
  for(ObservationVector::iterator iter=subjectList.begin();
      iter != subjectList.end(); iter++)
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
  ObservationVector& observerList = this->ObserverMap[observer];
  for (ObservationVector::iterator iter = observerList.begin();
       iter != observerList.end(); iter++)
    {
    if ( (*iter)->GetObserver() == observer )
      {
      collection->AddItem( *iter );
      }
    }
  return collection;
}

//----------------------------------------------------------------------------
vtkCollection *vtkEventBroker::GetObservationsForCallback ( vtkCallbackCommand *callback )
{
  vtkCollection *collection = vtkCollection::New();
  ObjectToObservationVectorMap::iterator it;
  for (it = this->ObserverMap.begin(); it != this->ObserverMap.end(); ++it)
    {
    ObservationVector::iterator iter;
    for(iter=it->second.begin(); iter != it->second.end(); iter++)
      {
      if ( *iter && (*iter)->GetCallbackCommand() == callback )
        {
        collection->AddItem( *iter );
        }
      }
    }
  return collection;
}

//----------------------------------------------------------------------------
int vtkEventBroker::GetNumberOfObservations ( )
{
  size_t count = 0;
  ObjectToObservationVectorMap::iterator iter;
  for(iter=this->SubjectMap.begin(); iter != this->SubjectMap.end(); iter++)
    {
    count += iter->second.size();
    }
  return static_cast<int>(count);
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::GetNthObservation ( int n )
{
  if ( n < 0 )
    {
    return nullptr;
    }

  size_t count = 0;
  ObjectToObservationVectorMap::iterator iter;
  for(iter=this->SubjectMap.begin(); iter != this->SubjectMap.end(); iter++)
    {
    if ( static_cast<size_t>(n) < count + iter->second.size())
      {
      ObservationVector::iterator it=iter->second.begin();
      std::advance(it, n-count);
      return (*it);
      }
    else
      {
      count += iter->second.size();
      }
    }
  return (nullptr);
}

//----------------------------------------------------------------------------
int vtkEventBroker::GenerateGraphFile ( const char *graphFile )
{
  std::ofstream file;

  file.open( graphFile, std::ios::out );

  if ( file.fail() )
    {
    vtkErrorMacro( "could not write to " << graphFile );
    return 1;
    }

  file << "strict digraph G {\n";

  vtkObservation *observation;
  int size = this->GetNumberOfObservations();
  for (int count = 0; count < size; count++)
    {
    observation = this->GetNthObservation( count );
    file << "# " << observation->GetReferenceCount() << "\n";
    if ( observation->GetScript() != nullptr )
      {
      file << " " \
          << "\"" << observation->GetScript() << "\""
          << " -> "
          << observation->GetSubject()->GetClassName()
          << " [ label = \""
          << vtkCommand::GetStringFromEventId( observation->GetEvent() )
          << "\" ];\n" ;
      }
    else
      {
      file << " " \
          << observation->GetObserver()->GetClassName()
          << " -> "
          << observation->GetSubject()->GetClassName()
          << " [ label = \""
          << vtkCommand::GetStringFromEventId( observation->GetEvent() )
          << "\" ];\n" ;
      }
    file.flush();
    }

  file << "}\n";
  file.close();
  return 0;
}

//----------------------------------------------------------------------------
void vtkEventBroker::OpenLogFile ()
{
  this->CloseLogFile();
  this->LogFile.open( this->LogFileName, std::ios::out );
  this->LogFile << "strict digraph G {\n";
}

//----------------------------------------------------------------------------
void vtkEventBroker::CloseLogFile ()
{
  if ( this->LogFile.is_open() )
    {
    this->LogFile << "}\n";
    this->LogFile.close();
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::LogEvent ( vtkObservation *observation )
{
  if ( this->LogFileName == nullptr )
    {
    // if we don't have a log file, we can't do anything
    return;
    }

  // start an event log if needed
  if ( this->EventLogging && !this->LogFile.is_open() )
    {
    this->OpenLogFile();
    }

  // close the log if done
  if ( !this->EventLogging && this->LogFile.is_open() )
    {
    this->CloseLogFile();
    return;
    }

  char eventString[BUFSIZ];
  const char *eventStringPointer;
  if ( this->EventLogging && observation != nullptr )
    {

    // indent to indicate nesting
    for (int i = 0; i < this->EventNestingLevel; i++)
      {
      this->LogFile << " ";
      }

    eventStringPointer = vtkCommand::GetStringFromEventId( observation->GetEvent() );
    if ( !strcmp (eventStringPointer, "NoEvent") )
      {
      sprintf (eventString, "%ld", observation->GetEvent());
      eventStringPointer = eventString;
      }

    // log the actual event
    if ( observation->GetScript() != nullptr )
      {
      this->LogFile << " " \
          << observation->GetSubject()->GetClassName()
          << " -> "
          << "\"" << observation->GetScript() << "\""
          << " [ label = \""
          << eventStringPointer
          << "\" ];\n" ;
      }
    else
      {
      const char *observerClass = "No observer class";
      if ( observation->GetObserver() )
        {
        observerClass = observation->GetObserver()->GetClassName();
        }

      this->LogFile << " " \
          << observation->GetSubject()->GetClassName()
          << " -> "
          << observerClass
          << " [ label = \""
          << eventStringPointer
          << "\" ];\n" ;
      }

    // indent to indicate nesting
    for (int i = 0; i < this->EventNestingLevel; i++)
      {
      this->LogFile << " ";
      }
    this->LogFile << " # " << observation->GetLastElapsedTime() << " seconds \n";

    this->LogFile.flush();
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::ProcessEvent ( vtkObservation *observation, vtkObject *caller, unsigned long eid, void *callData )
{
  //
  // we've got an event we care about - either invoke it
  // right away, or put it on the queue for later handling
  // - check first if it's the event the observer asked for (it might
  //   be a delete event that the event broker asked for)
  // - if the observer did ask to observe delete events, pass them through
  //   right away even in async mode - this way things can clean up
  //
  if ( eid == observation->GetEvent() || observation->GetEvent() == vtkCommand::AnyEvent )
    {
    if ( this->EventMode == vtkEventBroker::Synchronous || eid == vtkCommand::DeleteEvent )
      {
      this->InvokeObservation( observation, eid, callData );
      }
    else if ( this->EventMode == vtkEventBroker::Asynchronous )
      {
      this->QueueObservation( observation, eid, callData );
      }
    else
      {
      vtkErrorMacro ( "Bad EventMode " << this->EventMode );
      }
    }

  //
  // for delete events, clean up and get out
  // - first invoke any observations that have the object being
  //   deleted as the subject
  // - then we want to forget we ever had this observation
  //   and any that were connected to the object that is
  //   going to be deleted.
  //
  if ( eid == vtkCommand::DeleteEvent )
    {
    // iterate list of observations for the deleted object (caller) as subject
    ObservationVector::iterator obsIter;
    ObservationVector& subjectList = this->SubjectMap[caller];
    for(obsIter=subjectList.begin(); obsIter != subjectList.end(); ++obsIter)
      {
      if ( (*obsIter)->GetEvent() == vtkCommand::DeleteEvent )
        {
        this->InvokeObservation( observation, eid, callData );
        }
      }
    if ( caller == observation->GetSubject() )
      {
      // Remove all observations for this subject (0 matches all tags)
      this->RemoveObservationsForSubjectByTag (observation->GetSubject(), 0);
      }
    else if ( caller == observation->GetObserver() )
      {
      // Remove all observations for this observer
      this->RemoveObservations (observation->GetObserver());
      }
    else
      {
      vtkErrorMacro("Unknown caller for DeleteEvent");
      }

    return;
    }
}


//----------------------------------------------------------------------------
void vtkEventBroker::QueueObservation ( vtkObservation *observation,
                                        unsigned long eid,
                                        void *callData )
{
  //
  // two modes -
  //  - CompressCallDataOn: only keep the most recent call data.  this means that if the
  //    observation is in the queue, replace the call data with the current value
  //  - CompressCallDataOff: maintain the list of all call data values, but only
  //    one unique entry for each
  // it it's not there, add the current call data to the list so that each unique combination
  // can be invoked.
  // If the event is not currently in the queue, add it and keep a flag.
  //
  vtkObservation::CallType call(eid, callData);
  if ( this->GetCompressCallData() &&
       observation->GetEvent() != vtkCommand::AnyEvent)
    {
    observation->GetCallDataList()->clear();
    observation->GetCallDataList()->push_back( call );
    }
  else
    {
    std::deque< vtkObservation::CallType >::const_iterator dataIter;
    for(dataIter=observation->GetCallDataList()->begin();dataIter != observation->GetCallDataList()->end(); dataIter++)
      {
      if ( call.EventID == dataIter->EventID &&
           call.CallData == dataIter->CallData)
        {
        break;
        }
      }
    if ( dataIter == observation->GetCallDataList()->end() )
      {
      observation->GetCallDataList()->push_back( call );
      }
    }

  if ( !observation->GetInEventQueue() )
    {
    this->EventQueue.push_back( observation );
    observation->SetInEventQueue(1);
    }
}

//----------------------------------------------------------------------------
int vtkEventBroker::GetNumberOfQueuedObservations ()
{
  return static_cast<int>( this->EventQueue.size() );
}

//----------------------------------------------------------------------------
vtkObservation *vtkEventBroker::GetNthQueuedObservation ( int n )
{
  if ( n < 0 || n >= this->GetNumberOfQueuedObservations() )
    {
    return nullptr;
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
void vtkEventBroker::InvokeObservation ( vtkObservation *observation,
                                         unsigned long eid, void *callData )
{
  this->EventNestingLevel++;

  double startTime = this->TimerLog->GetUniversalTime();

  // Register so observation won't be deleted while callback is running
  observation->Register(this);

  // Invoke the observation
  // - run script if available, otherwise run callback command
  //  -- pass back the client data to the script handler (for
  //     example it could be the interpreter to use)
  if ( observation->GetScript() != nullptr )
    {
    if ( this->ScriptHandler )
      {
      (*(this->ScriptHandler)) (
          observation->GetScript(), this->ScriptHandlerClientData );
      }
    }
  else
    {
    observation->GetCallbackCommand()->Execute(
                                      observation->GetSubject(),
                                      eid,
                                      callData );
    }

  // Record timing and write the to the log file if enabled
  double elapsedTime = this->TimerLog->GetUniversalTime() - startTime;
  observation->SetTotalElapsedTime (observation->GetTotalElapsedTime() + elapsedTime);
  observation->SetLastElapsedTime (elapsedTime);
  this->LogEvent (observation);

  // clear reference to observation (may cause delete)
  observation->Delete();
  this->EventNestingLevel--;
}

//----------------------------------------------------------------------------
void vtkEventBroker::ProcessEventQueue ()
{
  //
  // for each observation on the event queue,
  // invoke it with each of the stored callData pointers
  // - register your pointer to the observation in case it
  //   gets deleted during handling of the event
  // - if the observation is no longer in the queue, stop processing events
  // - unregister before after dequeing in case the observation should go away
  //
  while ( this->GetNumberOfQueuedObservations() > 0 )
    {
    vtkObservation *observation = this->EventQueue.front();
    observation->Register( this );
    int finished = 0;
    while ( !finished )
      {
      vtkObservation::CallType call = observation->GetCallDataList()->front();
      observation->GetCallDataList()->pop_front();
      finished = (observation->GetCallDataList()->size() == 0);
      this->InvokeObservation( observation, call.EventID, call.CallData );
      if ( !observation->GetInEventQueue() )
        {
        observation->GetCallDataList()->clear();
        finished = 1;
        break;
        }
      }
    this->DequeueObservation();
    observation->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkEventBroker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "NumberOfObservations: " << this->GetNumberOfObservations() << "\n";
  os << indent << "NumberOfQueueObservations: " << this->GetNumberOfQueuedObservations() << "\n";
  os << indent << "EventMode: " << this->GetEventModeAsString() << "\n";
  os << indent << "EventLogging: " << this->EventLogging << "\n";
  os << indent << "EventNestingLevel: " << this->EventNestingLevel << "\n";
  os << indent << "LogFileName: " <<
    (this->LogFileName ? this->LogFileName : "(none)") << "\n";
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

//----------------------------------------------------------------------------
void vtkEventBroker::classInitialize()
{
  // Allocate the singleton
  vtkEventBrokerInstance = vtkEventBroker::GetInstance();
}

//----------------------------------------------------------------------------
void vtkEventBroker::classFinalize()
{
  vtkEventBrokerInstance->Delete();
  vtkEventBrokerInstance = nullptr;
}
