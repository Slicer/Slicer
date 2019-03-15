/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObserverManager.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkUnsignedLongArray.h>

// MRML includes
#include "vtkEventBroker.h"
#include "vtkObservation.h"
#include "vtkObserverManager.h"

vtkStandardNewMacro(vtkObserverManager);

//----------------------------------------------------------------------------
vtkObserverManager::vtkObserverManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->Owner = nullptr;
}

//----------------------------------------------------------------------------
vtkObserverManager::~vtkObserverManager()
{
  // Remove all the observations associated to the callback
  std::map< vtkObject*, vtkUnsignedLongArray*>::iterator iter;
  for(iter=this->ObserverTags.begin(); iter != this->ObserverTags.end(); iter++)
    {
    this->RemoveObjectEvents(iter->first);
    vtkUnsignedLongArray *objTags = iter->second;
    objTags->Delete();
    }

  if (this->CallbackCommand)
    {
    if (this->Owner &&
        this->CallbackCommand->GetReferenceCount() > 1)
      {
      // It is possible to externally use the callback command, it is not
      // recommended though.
      vtkWarningMacro( << "The callback is not deleted because there are still some observation. "
                       << "They seem to not have been registered into the event broker.");
      }
    this->CallbackCommand->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkObserverManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  std::map< vtkObject*, vtkUnsignedLongArray*>::iterator iter;
  for(iter=this->ObserverTags.begin(); iter != this->ObserverTags.end(); iter++)
    {
    vtkUnsignedLongArray *objTags = iter->second;
    os << indent << "Observer tags:             " << objTags << "\n";
    }
}

//----------------------------------------------------------------------------
vtkObject* vtkObserverManager::GetObserver()
{
  return this->GetOwner() ? this->GetOwner() : this;
}

//----------------------------------------------------------------------------
void vtkObserverManager::SetObject(vtkObject **nodePtr, vtkObject *node)
{
  vtkDebugMacro (<< "SetObject of " << node);

  // SetObject only, therefore we need to remove all event observers
  this->RemoveObjectEvents(*nodePtr);

  if (*nodePtr == node)
    {
    return;
    }
  vtkObject *nodePtrOld = *nodePtr;

  *nodePtr  = node ;

  if ( node )
    {
    vtkDebugMacro (<< "registering " << node << " with " << this << "\n");
    node->Register(this);
    }

  if (nodePtrOld)
    {
    (nodePtrOld)->UnRegister(this);
    }

}

//----------------------------------------------------------------------------
void vtkObserverManager::SetAndObserveObject(vtkObject **nodePtr, vtkObject *node, float priority, bool logWarningIfSameObservationExists)
{
  vtkDebugMacro (<< "SetAndObserveObject of " << node);
  if (*nodePtr == node)
    {
    if (node == nullptr)
      {
      return;
      }
    // The node is the same - check if the events are the same, too,
    // because then there is no need to update the observations.
    vtkNew<vtkIntArray> existingEvents;
    vtkNew<vtkFloatArray> existingPriorities;
    this->GetObjectEvents(*nodePtr, existingEvents.GetPointer(), existingPriorities.GetPointer());
    if (existingEvents->GetNumberOfTuples()==1 && existingEvents->GetValue(0) == vtkCommand::ModifiedEvent
      && existingPriorities->GetNumberOfTuples()==1 && existingPriorities->GetValue(0) == priority)
      {
      if (logWarningIfSameObservationExists)
        {
        vtkWarningMacro( << "The same object is already observed with the same priority. The observation is kept as is.");
        }
      return;
      }
    }
  this->SetObject(nodePtr, node);
  this->ObserveObject(node, priority);
}


//----------------------------------------------------------------------------
void vtkObserverManager::SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events, vtkFloatArray *priorities, bool logWarningIfSameObservationExists)
{
  vtkDebugMacro (<< "SetAndObserveObjectEvents of " << node);
  if (*nodePtr == node)
    {
    if (node == nullptr)
      {
      return;
      }
    // The node is the same - check if the events are the same, too,
    // because then there is no need to update the observations.
    int numberOfEvents = events ? events->GetNumberOfTuples() : 0;
    int numberOfPriorities = priorities ? priorities->GetNumberOfTuples() : 0;
    vtkNew<vtkIntArray> existingEvents;
    vtkNew<vtkFloatArray> existingPriorities;
    this->GetObjectEvents(*nodePtr, existingEvents.GetPointer(), existingPriorities.GetPointer());
    // There must be same number of existing events and priorities in the same order,
    // if there is a mismatch then we update the event observations. This event comparison method
    // is simple and fast, but it may update the event observations when it is not necessary
    // (when only the order is different) - which is an acceptable tradeoff.
    if (existingEvents->GetNumberOfTuples() == numberOfEvents && existingEvents->GetNumberOfTuples() == existingPriorities->GetNumberOfTuples())
      {
      bool eventsEqual = true;
      for (int i=0; i<numberOfEvents; i++)
        {
        if (existingEvents->GetValue(i) != events->GetValue(i))
          {
          eventsEqual = false;
          break;
          }
        float priority = ( i<numberOfPriorities ? priorities->GetValue(i) : 0.0 /* default priority */ );
        if (existingPriorities->GetValue(i) != priority)
          {
          eventsEqual = false;
          break;
          }
        }
      if (eventsEqual)
        {
        if (logWarningIfSameObservationExists)
          {
          vtkWarningMacro( << "The same object is already observed with the same events and priorities. The observation is kept as is.");
          }
        return;
        }
      }
    }

  this->SetObject(nodePtr, node);
  this->AddObjectEvents(node, events, priorities);
}

//----------------------------------------------------------------------------
void vtkObserverManager::RemoveObjectEvents(vtkObject *nodePtr)
{
  if (nodePtr)
    {
    std::map< vtkObject*, vtkUnsignedLongArray*>::iterator it =  this->ObserverTags.find(nodePtr);
    vtkEventBroker *broker = vtkEventBroker::GetInstance();
    if (it != this->ObserverTags.end())
      {
      vtkUnsignedLongArray* objTags = it->second;
      for (int i=0; i < objTags->GetNumberOfTuples(); i++)
        {
        broker->RemoveObservationsForSubjectByTag( nodePtr, objTags->GetValue(i) );
        }
      objTags->Reset();
      }
    }
}


//----------------------------------------------------------------------------
void vtkObserverManager::ObserveObject(vtkObject* node, float priority)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkNew<vtkFloatArray> priorities;
  priorities->InsertNextValue(priority);
  this->AddObjectEvents(node, events.GetPointer(), priorities.GetPointer());
}

//----------------------------------------------------------------------------
void vtkObserverManager::AddObjectEvents(vtkObject *nodePtr, vtkIntArray *events, vtkFloatArray *priorities)
{
  // check whether no priorities are provided or the same number of
  // events and priorities are provided
  if ( !((events && priorities && events->GetNumberOfTuples() == priorities->GetNumberOfTuples()) || (events && !priorities)))
    {
    vtkWarningMacro(<< "Number of events (" << events->GetNumberOfTuples()
                    << ") doesn't match number of priorities ("
                    << priorities->GetNumberOfTuples());
    return;
    }

  if (nodePtr)
    {
    vtkUnsignedLongArray* objTags = nullptr;
    std::map< vtkObject*, vtkUnsignedLongArray*>::iterator it =  this->ObserverTags.find(nodePtr);
    if (it != this->ObserverTags.end())
      {
      objTags = it->second;
      }
    else
      {
      objTags = vtkUnsignedLongArray::New();
      this->ObserverTags[nodePtr] = objTags;
      }

    vtkEventBroker *broker = vtkEventBroker::GetInstance();
    vtkObject *observer = this->GetObserver();
    if (events)
      {
      for (int i=0; i<events->GetNumberOfTuples(); i++)
        {
#ifndef NDEBUG
        // Make sure we are not adding an already existing connection. It's
        // not a big issue but it just shows poor design.
        if (this->GetObservationsCount(nodePtr, events->GetValue(i)) > 0)
          {
          vtkWarningMacro(<< "Observation " << events->GetValue(i)
                          << " between " << nodePtr->GetClassName()
                          << " and " << observer->GetClassName()
                          << " already exists.");
          }
#endif
        vtkObservation *observation=nullptr;
        if (!priorities)
          {
          observation = broker->AddObservation (nodePtr, events->GetValue(i), observer, this->CallbackCommand );
          }
        else
          {
          observation = broker->AddObservation (nodePtr, events->GetValue(i), observer, this->CallbackCommand, priorities->GetValue(i) );
          }

        unsigned long tag = observation->GetEventTag();

        objTags->InsertNextValue(tag);
        }
      }
    }

}

//----------------------------------------------------------------------------
int vtkObserverManager::GetObservationsCount(vtkObject *nodePtr, unsigned long event)
{
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  vtkObject *observer = this->GetObserver();
  vtkEventBroker::ObservationVector observations =
    broker->GetObservations(nodePtr, event, observer, this->CallbackCommand);
  return static_cast<int>(observations.size());
}

//----------------------------------------------------------------------------
void vtkObserverManager::GetObjectEvents(vtkObject *nodePtr, vtkIntArray *events, vtkFloatArray *priorities)
{
  events->Resize(0);
  priorities->Resize(0);
  if (nodePtr == nullptr)
    {
    // node is invalid
    return;
    }
  std::map< vtkObject*, vtkUnsignedLongArray*>::iterator it =  this->ObserverTags.find(nodePtr);
  if (it == this->ObserverTags.end())
    {
    // no observations are found
    return;
    }
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  vtkUnsignedLongArray* objTags = it->second;
  for (int i=0; i < objTags->GetNumberOfTuples(); i++)
    {
    vtkEventBroker::ObservationVector observations = broker->GetObservationsForSubjectByTag ( nodePtr, objTags->GetValue(i) );
    for (vtkEventBroker::ObservationVector::iterator observationIt = observations.begin(); observationIt != observations.end(); ++observationIt)
      {
      events->InsertNextValue((*observationIt)->GetEvent());
      priorities->InsertNextValue((*observationIt)->GetPriority());
      }
    }
}
