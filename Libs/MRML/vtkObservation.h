/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObservation.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkObservation - class that stores information about 
// the relationship between a Subject and an Observer
// .SECTION Description
// The Observation is a record of 
// - a subject  (vtkObject)
// - an event type (unsigned long)
// - an objserver (vtkObject)
// - a callback (vtkCallbackCommand)
// - optional comment strings
// This class can be used by the vtkEventBroker to keep track of the registered observers
// that it manages, and it can be used by the event queue to keep track of which 
// events have been triggered so it can invoke them later
//
// .NOTES
// This class does not add or remove observers itself; it just keeps track of them
// for the event broker.

#ifndef __vtkObservation_h
#define __vtkObservation_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkEventBroker.h"

class VTK_MRML_EXPORT vtkObservation : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkObservation *New();
  vtkTypeRevisionMacro(vtkObservation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Accessors
  // - note that AssignObject methods do not register the pointers
  // - it is assumed that the EventBroker will attach DeleteEvent
  //   observers to these objects and will thereby know when they
  //   are no longer valid
  vtkSetObjectMacro (EventBroker, vtkEventBroker);
  vtkGetObjectMacro (EventBroker, vtkEventBroker);
  vtkGetMacro (InEventQueue, int);
  vtkSetMacro (InEventQueue, int);
  vtkGetObjectMacro (ObservationCallbackCommand, vtkCallbackCommand);
  vtkGetObjectMacro (Subject, vtkObject);
  void AssignSubject(vtkObject* subject) {this->Subject = subject;};
  vtkGetMacro (Event, unsigned long);
  vtkSetMacro (Event, unsigned long);
  vtkSetObjectMacro (CallbackCommand, vtkCallbackCommand);
  vtkGetObjectMacro (CallbackCommand, vtkCallbackCommand);
  vtkSetStringMacro (Script);
  vtkGetStringMacro (Script);
  vtkGetObjectMacro (Observer, vtkObject);
  void AssignObserver(vtkObject* observer) {this->Observer = observer;};
  vtkSetStringMacro (Comment);
  vtkGetStringMacro (Comment);

  vtkGetMacro (EventTag, unsigned long);
  vtkSetMacro (EventTag, unsigned long);
  vtkGetMacro (SubjectDeleteEventTag, unsigned long);
  vtkSetMacro (SubjectDeleteEventTag, unsigned long);
  vtkGetMacro (ObserverDeleteEventTag, unsigned long);
  vtkSetMacro (ObserverDeleteEventTag, unsigned long);

protected:
  vtkObservation();
  virtual ~vtkObservation();
  vtkObservation(const vtkObservation&);
  void operator=(const vtkObservation&);

  // Description:
  // EventBroker that 'owns' this observation
  vtkEventBroker *EventBroker;

  // Description:
  // Holder for callback that this object wants called when either
  // the subject or the observer triggers and event
  vtkCallbackCommand *ObservationCallbackCommand;

  // Description:
  // Flag that tells the broker that this observation 
  // is already in the event queue and doesn't need
  // to be re-added
  int InEventQueue;

  // Description:
  // Holder for Subject
  vtkObject *Subject;

  // Description:
  // Holder for Event
  unsigned long Event;

  // Description:
  // Holder for Observer
  vtkObject *Observer;

  // Description:
  // Holder for callback that the Observer wants to have run when Event happens
  vtkCallbackCommand *CallbackCommand;

  // Description:
  // Holder for script as an alternative to the callback command
  char *Script;

  // Description:
  // Holder for comment string
  char *Comment;

  // Description:
  // keep track of the tags returned by vtkObject::AddObserver so this
  // observation will be easy to remove when the time comes
  unsigned long EventTag;
  unsigned long SubjectDeleteEventTag;
  unsigned long ObserverDeleteEventTag;
};

#endif

