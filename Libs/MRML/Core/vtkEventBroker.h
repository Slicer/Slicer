/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEventBroker.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#ifndef __vtkEventBroker_h
#define __vtkEventBroker_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>
class vtkTimerLog;

// STD includes
#include <deque>
#include <vector>
#include <set>
#include <map>
#include <fstream>

class vtkCollection;
class vtkCallbackCommand;
class vtkObservation;

/// \brief Class that manages adding and deleting of observers with events.
///
/// This class keeps track of observers and events added to each vtk object.
/// It caches tags returned by AddObserver method so that observers can be
/// removed properly.
/// See also:
/// http://wiki.na-mic.org/Wiki/index.php/Slicer3:EventBroker
/// http://en.wikipedia.org/wiki/Observer_pattern
//
/// Other interesting observer implementations:
/// http://xlobject.sourceforge
/// http://sigslot.sourceforge.net/
/// http://doc.trolltech.com/4.3/signalsandslots.html
class VTK_MRML_EXPORT vtkEventBroker : public vtkObject
{
public:
  vtkTypeMacro(vtkEventBroker, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef std::set< vtkObservation * > ObservationVector;

  ///
  /// Return the singleton instance with no reference counting.
  static vtkEventBroker* GetInstance();

  ///
  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkEventBroker object per process.  Clients that
  /// call this must call Delete on the object so that the reference
  /// counting will work. The single instance will be unreferenced when
  /// the program exits.
  static vtkEventBroker* New();

  ///
  /// the static function used by the command callback (used by vtkObservation)
  /// - each observation has a vtkCallbackCommand method which stores
  ///   the pointer to the vtkObservation class as the clientData
  /// - the vtkObservation has a pointer to vtkEventBroker,
  ///   which knows how to process that event for that Observation
  static void Callback(vtkObject *caller,
      unsigned long eid, void *clientData, void *callData);

  /// Adding and Removing Observation objects
  ///
  /// Request that an observer be added to the subject (or "observee")
  /// - this is the "Attach" operation
  vtkObservation *AddObservation (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify, float priority=0.0f);

  ///
  /// Scripted version of observation
  /// - creates an observation that will be invoked using the ScriptHandler method
  vtkObservation *AddObservation (vtkObject *subject, const char *event, const char *script);

  ///
  /// Remove observation from the broker and event queue
  void RemoveObservation (vtkObservation *observation);

  ///
  /// Remove all observations that match
  /// - various signatures provided as helpers
  /// - when specifying the tag, a 0 matches all tags
  void RemoveObservations (ObservationVector observations);
  void RemoveObservations (vtkObject *observer);
  void RemoveObservations (vtkObject *subject, vtkObject *observer);
  void RemoveObservations (vtkObject *subject, unsigned long event, vtkObject *observer);
  void RemoveObservations (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify);
  void RemoveObservationsForSubjectByTag (vtkObject *subject, unsigned long tag);
  /// Fast retrieve of all observations of a given subject
  ObservationVector GetSubjectObservations(vtkObject *subject);
  /// If event is != 0 , only observations matching the events are returned
  /// If observer is != 0 , only observations matching the observer are returned
  /// If notify is != 0, only observations matching the callback are returned
  /// If maxReturnedObservations is != 0, only up to this number of observations are are returned
  ObservationVector GetObservations (vtkObject *subject,
                                                  unsigned long event = 0,
                                                  vtkObject *observer = nullptr,
                                                  vtkCallbackCommand *notify = nullptr,
                                                  unsigned int maxReturnedObservations = 0);
  /// Returns true if such an observation exists (arguments are same as for GetObservations)
  bool GetObservationExist (vtkObject *subject,
                                                  unsigned long event = 0,
                                                  vtkObject *observer = nullptr,
                                                  vtkCallbackCommand *notify = nullptr);
  ObservationVector GetObservationsForSubjectByTag (vtkObject *subject, unsigned long tag);

  /// Description
  /// Accessors for intropsection
  /// Note: vtkCollection object is allocated internally
  /// and must be freed by the caller
  vtkCollection *GetObservationsForSubject (vtkObject *subject);
  vtkCollection *GetObservationsForObserver (vtkObject *observer);
  vtkCollection *GetObservationsForCallback (vtkCallbackCommand* callback);

  ///
  /// Accessors for Observations
  int GetNumberOfObservations();
  vtkObservation *GetNthObservation(int n);

  ///
  /// Process any event that comes from either subject or observer
  void ProcessEvent (vtkObservation *observation, vtkObject *caller, unsigned long eid, void *callData);

  /// Event Logging
  ///
  /// Turn on event tracing (requires TraceFile)
  vtkBooleanMacro (EventLogging, int);
  vtkSetMacro (EventLogging, int);
  vtkGetMacro (EventLogging, int);

  ///
  /// Current level of indent (event nesting)
  /// shows what is called by what when in synchronous mode
  vtkSetMacro (EventNestingLevel, int);
  vtkGetMacro (EventNestingLevel, int);

  ///
  /// File to write event logs to when EventLoging is turned on
  vtkSetStringMacro (LogFileName);
  vtkGetStringMacro (LogFileName);

  ///
  /// Timer log class for calculating elapsed time for event invocations
  virtual void SetTimerLog(vtkTimerLog* timerLog);
  vtkGetObjectMacro (TimerLog, vtkTimerLog);

  ///
  /// Open and close the log file
  void OpenLogFile ();
  void CloseLogFile ();

  ///
  /// actually write to the log file (also manages state of the LogFile ivar
  /// based on the filename and the EventLogging variable)
  void LogEvent (vtkObservation *observation);

  /// Graph File
  ///
  /// Write out the current list of observations in graphviz format (.dot)
  int GenerateGraphFile ( const char *graphFile );


  /// Event Queue processing modes
  ///
  /// In synchronous mode, observations are invoked immediately when the
  /// event takes place.  In asynchronous mode, observations are added
  /// to the event queue for later invocation.
  enum EventMode {
    Synchronous,
    Asynchronous
  };
  vtkGetMacro(EventMode, int);
  void SetEventMode(int eventMode)
  {
    if (eventMode != this->EventMode)
     {
     this->EventMode = eventMode;
     this->ProcessEventQueue();
     this->Modified();
     }
  };

  void SetEventModeToSynchronous() {this->SetEventMode(vtkEventBroker::Synchronous);};
  void SetEventModeToAsynchronous() {this->SetEventMode(vtkEventBroker::Asynchronous);};
  const char * GetEventModeAsString() {
    if (this->EventMode == vtkEventBroker::Synchronous) return ("Synchronous");
    if (this->EventMode == vtkEventBroker::Asynchronous) return ("Asynchronous");
    return "Undefined";
  }


  /// Event queue processing

  ///
  /// Event queue handling routines
  /// Note:
  /// - assume here that the information in a vtkObservation contains enough
  /// information for the observer to handle the event (that is, we don't pass
  /// the callData field of the event back)
  /// TODO: if the callData is needed, we will need another class/struct to
  /// go into the event queue that saves them
  void QueueObservation (vtkObservation *observation, unsigned long eid,
                         void *callData);
  int GetNumberOfQueuedObservations ();
  vtkObservation *GetNthQueuedObservation (int n);
  vtkObservation *DequeueObservation ();
  void InvokeObservation (vtkObservation *observation, unsigned long eid,
                          void *callData);
  void ProcessEventQueue ();

  ///
  /// two modes -
  ///  - CompressCallDataOn: only keep the most recent call data.  this means that if the
  ///    observation is in the queue, replace the call data with the current value
  ///  - CompressCallDataOff: maintain the list of all call data values, but only
  ///    one unique entry for each
  ///  Compression is ON by default
  vtkBooleanMacro (CompressCallData, int);
  vtkGetMacro (CompressCallData, int);
  vtkSetMacro (CompressCallData, int);

  ///
  /// Sets the method pointer to be used for processing script observations
  void SetScriptHandler ( void (*scriptHandler) (const char* script, void *clientData), void *clientData )
    {
    this->ScriptHandler = scriptHandler;
    this->ScriptHandlerClientData = clientData;
    }

protected:
  vtkEventBroker();
  ~vtkEventBroker() override;
  vtkEventBroker(const vtkEventBroker&);
  void operator=(const vtkEventBroker&);

  ///
  /// Singleton management functions.
  static void classInitialize();
  static void classFinalize();

  ///
  /// Attach adds the observers to the object.
  /// Detach removes the observers
  /// These routines manage the internal datastructures and should
  /// be the only methods used to modified the internal Observations member
  /// Please note that they don't update the SubjectMap nor the ObserverMap.
  void AttachObservation (vtkObservation *observation);
  void DetachObservation (vtkObservation *observation);

  friend class vtkEventBrokerInitialize;
  typedef vtkEventBroker Self;


  ///
  typedef std::map< vtkObject*, ObservationVector > ObjectToObservationVectorMap;

  /// maps to manage quick lookup by object
  ObjectToObservationVectorMap SubjectMap;
  ObjectToObservationVectorMap ObserverMap;

  /// The event queue of triggered but not-yet-invoked observations
  std::deque< vtkObservation * > EventQueue;

  void (*ScriptHandler) (const char* script, void* clientData);
  void *ScriptHandlerClientData;

  int EventLogging;
  int EventNestingLevel;
  char *LogFileName;
  vtkTimerLog *TimerLog;

  int EventMode;
  int CompressCallData;

  std::ofstream LogFile;
private:
  /// DetachObservations is a fast (but dangerous) method to delete all the
  /// observations. It leaves the event broker in an inconsistent state:
  ///  - SubjectMap and ObserverMap are not being updated.
  void DetachObservations();
  /// vtkObservation can call these methods
  friend class vtkObservation;
};

/// Utility class to make sure qSlicerModuleManager is initialized before it is used.
class VTK_MRML_EXPORT vtkEventBrokerInitialize
{
public:
  typedef vtkEventBrokerInitialize Self;

  vtkEventBrokerInitialize();
  ~vtkEventBrokerInitialize();
private:
  static unsigned int Count;
};

/// This instance will show up in any translation unit that uses
/// vtkEventBroker.  It will make sure vtkEventBroker is initialized
/// before it is used.
static vtkEventBrokerInitialize vtkEventBrokerInitializer;

#endif
