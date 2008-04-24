/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEventBroker.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkEventBroker - class that manages adding and deleting of obserevers with events
// .SECTION Description
// Class that manages adding and deleting of obserevers with events
// This class keeps track of obserevers and events added to each vtk object 
// it caches tags returned by AddObserver method so that obserevers can be removed properly
// See also: 
// http://wiki.na-mic.org/Wiki/index.php/Slicer3:EventBroker
// http://en.wikipedia.org/wiki/Observer_pattern
//
// other interesting observer implementations:
// http://xlobject.sourceforge
// http://sigslot.sourceforge.net/
// http://doc.trolltech.com/4.3/signalsandslots.html

#ifndef __vtkEventBroker_h
#define __vtkEventBroker_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRML.h"

#include <deque>
#include <vector>
#include <map>
#include <string>
#include <fstream>

class vtkCollection;
class vtkCallbackCommand;
class vtkObservation;

class VTK_MRML_EXPORT vtkEventBroker : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  vtkTypeRevisionMacro(vtkEventBroker,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This is a singleton pattern New.  There will only be ONE
  // reference to a vtkEventBroker object per process.  Clients that
  // call this must call Delete on the object so that the reference
  // counting will work.   The single instance will be unreferenced when
  // the program exits.
  static vtkEventBroker* New();

  // Description:
  // Get the singleton
  static vtkEventBroker* GetInstance();

  // Description:
  // the static function used by the command callback (used by vtkObservation)
  // - each observation has a vtkCallbackCommand method which stores 
  //   the pointer to the vtkObservation class as the clientData
  // - the vtkObservation has a pointer to vtkEventBroker,
  //   which knows how to process that event for that Observation
  static void Callback(vtkObject *caller, 
      unsigned long eid, void *clientData, void *callData);

  //// Adding and Removing Observation objects

  // Description:
  // Request that an observer be added to the subject (or "observee")
  // - this is the "Attach" operation
  vtkObservation *AddObservation (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify);

  // Description:
  // Scripted version of observation
  // - creates an observation that will be inoked using the ScriptHandler method
  vtkObservation *AddObservation (vtkObject *subject, const char *event, const char *script);

  // Description:
  // Attach adds the observers to the object.
  // Detach removes the observers
  void AttachObservation (vtkObservation *observation);
  void DetachObservation (vtkObservation *observation);
 
  // Description:
  // Remove observation from the broker and event queue
  void RemoveObservation (vtkObservation *observation);

  // Description:
  // Remove all observations that match
  // - various signatures provided as helpers
  //BTX
  void RemoveObservations (std::vector< vtkObservation *>observations);
  //ETX
  void RemoveObservations (vtkObject *observer);
  void RemoveObservations (vtkObject *subject, vtkObject *observer);
  void RemoveObservations (vtkObject *subject, unsigned long event, vtkObject *observer);
  void RemoveObservations (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify);
  void RemoveObservationsForSubjectByTag (vtkObject *subject, unsigned long tag);
  //BTX
  std::vector< vtkObservation *> GetObservations (vtkObject *observer);
  std::vector< vtkObservation *> GetObservations (vtkObject *subject, vtkObject *observer);
  std::vector< vtkObservation *> GetObservations (vtkObject *subject, unsigned long event, vtkObject *observer);
  std::vector< vtkObservation *> GetObservations (vtkObject *subject, unsigned long event, vtkObject *observer, vtkCallbackCommand *notify);
  std::vector< vtkObservation *> GetObservationsForSubjectByTag (vtkObject *subject, unsigned long tag);
  //ETX

  // Description
  // Accessors for intropsection
  // Note: vtkCollection object is allocated internally 
  // and must be freed by the caller
  vtkCollection *GetObservationsForSubject (vtkObject *subject);
  vtkCollection *GetObservationsForObserver (vtkObject *observer);

  // Description:
  // Accessors for Observations
  int GetNumberOfObservations();
  vtkObservation *GetNthObservation(int n);

  // Description:
  // Process any event that comes from either subject or observer
  void ProcessEvent (vtkObservation *observation, vtkObject *caller, unsigned long eid, void *callData);

  //// Event Logging
  
  //
  // TODO: these are not implemented yet
  //
  // Description:
  // Turn on event tracing (requires TraceFile)
  vtkBooleanMacro (EventLogging, int);
  vtkSetMacro (EventLogging, int);
  vtkGetMacro (EventLogging, int);

  // Description:
  // File to write event logs to when EventLoging is turned on
  vtkSetStringMacro (LogFileName);
  vtkGetStringMacro (LogFileName);

  // Description:
  // Open and close the log file
  void OpenLogFile ();
  void CloseLogFile ();

  // Description:
  // actually write to the log file (also manages state of the LogFile ivar
  // based on the filename and the EventLogging variable)
  void LogEvent (vtkObservation *observation);

  //// Graph File

  // Description:
  // Write out the current list of observations in graphviz format (.dot)
  int GenerateGraphFile ( const char *graphFile );


  //// Event Queue processing modes

  // Description:
  // In synchronous mode, observations are invoked immediately when the
  // event takes place.  In asynchronous mode, observations are added
  // to the event queue for later invocation.
  //BTX
  enum EventMode {
    Synchronous,
    Asynchronous
  };
  //ETX
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


  //// Event queue processing
  
  // Description:
  // Event queue handling routines
  // Note:
  // - assume here that the information in a vtkObservation contains enough
  // information for the observer to handle the event (that is, we don't pass 
  // the callData field of the event back)
  // TODO: if the callData is needed, we will need another class/struct to 
  // go into the event queue that saves them
  void QueueObservation (vtkObservation *observation, void *callData); 
  int GetNumberOfQueuedObservations (); 
  vtkObservation *GetNthQueuedObservation (int n); 
  vtkObservation *DequeueObservation (); 
  void InvokeObservation (vtkObservation *observation, void *callData); 
  void ProcessEventQueue (); 

  // Description:
  // two modes - 
  //  - CompressCallDataOn: only keep the most recent call data.  this means that if the
  //    observation is in the queue, replace the call data with the current value
  //  - CompressCallDataOff: maintain the list of all call data values, but only
  //    one unique entry for each
  //  Compression is ON by default
  vtkBooleanMacro (CompressCallData, int);
  vtkGetMacro (CompressCallData, int);
  vtkSetMacro (CompressCallData, int);

  // Description:
  // Sets the method pointer to be used for processing script observations
  //BTX
  void SetScriptHandler ( void (*scriptHandler) (const char* script) )
    {
    this->ScriptHandler = scriptHandler;
    }
  //ETX

protected:

  vtkEventBroker();
  virtual ~vtkEventBroker();
  vtkEventBroker(const vtkEventBroker&);
  void operator=(const vtkEventBroker&);

  //BTX
  std::vector< vtkObservation * > Observations;
  std::deque< vtkObservation * > EventQueue;
  // TODO: cache relationships for fast access when compressing events
  // -- not clear which indices would actually speed things up
  //std::map< vtkObject*, int > ObservationsBySubject;
  //ETX
  
  //BTX
  void (*ScriptHandler) (const char* script);
  //ETX

  int EventLogging;
  char *LogFileName;

  int EventMode;
  int CompressCallData;

  //BTX
  std::ofstream LogFile;
  //ETX
  //

  static vtkEventBroker* Instance;

};

#endif

