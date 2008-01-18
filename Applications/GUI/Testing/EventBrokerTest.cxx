/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date: 2007-12-20 16:55:07 -0500 (Thu, 20 Dec 2007) $
Version:   $Revision: 5304 $

=========================================================================auto=*/

#include "EventBrokerTestCLP.h"

#include "vtkEventBroker.h"
#include "vtkObservation.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkImageEllipsoidSource.h"
#include "vtkImageViewer.h"

//----------------------------------------------------------------------------
void Callback(vtkObject *caller, unsigned long eid, void *clientData, void *callData)
{
  std::cerr << "Got an event " << eid << " from " << caller->GetClassName() << "\n";

  vtkImageViewer *viewer = reinterpret_cast<vtkImageViewer *>(clientData);
  viewer->Render();
}

int main(int argc, char * argv[])
{
  PARSE_ARGS;

  vtkEventBroker *broker = vtkEventBroker::New();
  vtkCallbackCommand *callback = vtkCallbackCommand::New();
  vtkImageEllipsoidSource *ellip = vtkImageEllipsoidSource::New();
  vtkImageViewer *viewer = vtkImageViewer::New();

  viewer->SetColorWindow(200);
  viewer->SetColorLevel(100);
  viewer->SetInput( ellip->GetOutput() );

  callback->SetCallback( Callback ); 
  callback->SetClientData( reinterpret_cast<void *> (viewer) );

  vtkObservation *observation = broker->AddObservation( ellip, vtkCommand::ModifiedEvent, viewer, callback);

  if ( logFileName != "" )
    {
    broker->SetLogFileName( logFileName.c_str() );
    broker->EventLoggingOn();
    }

  std::cerr << "Three synchonous events:\n";
  broker->SetEventModeToSynchronous();
  ellip->Modified();
  ellip->Modified();
  ellip->Modified();

  std::cerr << "Three asynchonous events:\n";
  broker->SetEventModeToAsynchronous();
  ellip->Modified();
  ellip->Modified();
  ellip->Modified();
  broker->ProcessEventQueue();

  if ( graphFile != "" )
    {
    broker->GenerateGraphFile( graphFile.c_str() );
    }

  viewer->Delete();
  ellip->Delete();
  callback->Delete();
  broker->Delete();

  return (EXIT_SUCCESS);
}
