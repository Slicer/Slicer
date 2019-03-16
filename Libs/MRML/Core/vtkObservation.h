/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObservation.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#ifndef __vtkObservation_h
#define __vtkObservation_h

// MRML includes
#include "vtkMRML.h"
class vtkEventBroker;

// VTK includes
#include <vtkObject.h>
class vtkCallbackCommand;

// STD includes
#include <deque>

/// \brief Stores information about the relationship between a Subject and an Observer.
///
/// The Observation is a record of
/// - a subject  (vtkObject)
/// - an event type (unsigned long)
/// - an objserver (vtkObject)
/// - a callback (vtkCallbackCommand)
/// - optional comment strings
/// This class can be used by the vtkEventBroker to keep track of the registered observers
/// that it manages, and it can be used by the event queue to keep track of which
/// events have been triggered so it can invoke them later
//
/// \note This class does not add or remove observers itself; it just keeps track of them
/// for the event broker.
class VTK_MRML_EXPORT vtkObservation : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkObservation *New();
  vtkTypeMacro(vtkObservation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Accessors
  /// - note that AssignObject methods do not register the pointers
  /// - it is assumed that the EventBroker will attach DeleteEvent
  ///   observers to these objects and will thereby know when they
  ///   are no longer valid
  virtual void SetEventBroker(vtkEventBroker* eventBroker);
  vtkGetObjectMacro (EventBroker, vtkEventBroker);
  vtkGetMacro (InEventQueue, int);
  vtkSetMacro (InEventQueue, int);
  vtkGetObjectMacro (ObservationCallbackCommand, vtkCallbackCommand);
  vtkGetObjectMacro (Subject, vtkObject);
  void AssignSubject(vtkObject* subject) {this->Subject = subject;};
  vtkGetMacro (Event, unsigned long);
  vtkSetMacro (Event, unsigned long);
  virtual void SetCallbackCommand(vtkCallbackCommand* callbackCommand);
  vtkGetObjectMacro (CallbackCommand, vtkCallbackCommand);
  vtkSetStringMacro (Script);
  vtkGetStringMacro (Script);
  vtkGetObjectMacro (Observer, vtkObject);
  void AssignObserver(vtkObject* observer) {this->Observer = observer;};
  vtkSetStringMacro (Comment);
  vtkGetStringMacro (Comment);
  vtkSetMacro(Priority, float);
  vtkGetMacro(Priority, float);

  vtkGetMacro (EventTag, unsigned long);
  vtkSetMacro (EventTag, unsigned long);
  vtkGetMacro (SubjectDeleteEventTag, unsigned long);
  vtkSetMacro (SubjectDeleteEventTag, unsigned long);
  vtkGetMacro (ObserverDeleteEventTag, unsigned long);
  vtkSetMacro (ObserverDeleteEventTag, unsigned long);

  /// Description
  /// Elapsed time of last invocation and total elaspsed time
  /// (in synchronous mode, elapsed time include children)
  vtkGetMacro (LastElapsedTime, double);
  vtkSetMacro (LastElapsedTime, double);
  vtkGetMacro (TotalElapsedTime, double);
  vtkSetMacro (TotalElapsedTime, double);

  struct CallType
  {
    inline CallType(unsigned long eventID, void* callData);
    unsigned long EventID;
    void* CallData;
  };
  std::deque<CallType> *GetCallDataList() {return &(this->CallDataList);};

protected:
  vtkObservation();
  ~vtkObservation() override;
  vtkObservation(const vtkObservation&);
  void operator=(const vtkObservation&);

  ///
  /// EventBroker that 'owns' this observation
  vtkEventBroker *EventBroker;

  ///
  /// Holder for callback that this object wants called when either
  /// the subject or the observer triggers and event
  vtkCallbackCommand *ObservationCallbackCommand;

  ///
  /// Flag that tells the broker that this observation
  /// is already in the event queue and doesn't need
  /// to be re-added
  int InEventQueue;

  ///
  /// Holder for Subject
  vtkObject *Subject;

  ///
  /// Holder for Event
  unsigned long Event;

  ///
  /// Holder for Observer
  vtkObject *Observer;

  ///
  /// Holder for callback that the Observer wants to have run when Event happens
  vtkCallbackCommand *CallbackCommand;

  ///
  /// data passed to the observation by the subject
  std::deque<CallType> CallDataList;

  ///
  /// Holder for script as an alternative to the callback command
  char *Script;

  ///
  /// Holder for comment string
  char *Comment;

  /// Priority of the observer
  float Priority;

  ///
  /// keep track of the tags returned by vtkObject::AddObserver so this
  /// observation will be easy to remove when the time comes
  unsigned long EventTag;
  unsigned long SubjectDeleteEventTag;
  unsigned long ObserverDeleteEventTag;

  double LastElapsedTime;
  double TotalElapsedTime;

};

//----------------------------------------------------------------------------
vtkObservation::CallType::CallType(unsigned long eventID, void* callData)
  : EventID(eventID)
  , CallData(callData)
{
}


#endif
