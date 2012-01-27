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
#include <vtkObjectFactory.h>
#include <vtkUnsignedLongArray.h>

// MRML includes
#include "vtkEventBroker.h"
#include "vtkObservation.h"
#include "vtkObserverManager.h"

vtkCxxRevisionMacro(vtkObserverManager, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkObserverManager);

//----------------------------------------------------------------------------
vtkObserverManager::vtkObserverManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->Owner = NULL;
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
  if (*nodePtr == node)
    {
    return;
    }
  vtkObject *nodePtrOld = *nodePtr;

  this->RemoveObjectEvents(*nodePtr);

  *nodePtr  = node ;

  if ( node  )
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
void vtkObserverManager::SetAndObserveObject(vtkObject **nodePtr, vtkObject *node)
{
  vtkDebugMacro (<< "SetAndObserveObject of " << node);
  if (*nodePtr == node)
    {
    return;
    }
  this->SetObject(nodePtr, node);
  this->ObserveObject(node);
}

//----------------------------------------------------------------------------
void vtkObserverManager::SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events)
{
  vtkDebugMacro (<< "SetAndObserveObjectEvents of " << node);
  if (*nodePtr == node)
    {
    if (node == 0)
      {
      return;
      }
    vtkWarningMacro( << "Setting the same object should be a no-op.");
    }
  this->SetObject(nodePtr, node);
  this->AddObjectEvents(node, events);
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
void vtkObserverManager::ObserveObject(vtkObject* node)
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  this->AddObjectEvents(node, events);
  events->Delete();
}

//----------------------------------------------------------------------------
void vtkObserverManager::AddObjectEvents(vtkObject *nodePtr, vtkIntArray *events) 
{
  if (nodePtr)
    {
    vtkUnsignedLongArray* objTags = NULL; 
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
        vtkObservation *observation = broker->AddObservation (nodePtr, events->GetValue(i), observer, this->CallbackCommand );
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
  std::vector<vtkObservation*> observations =
    broker->GetObservations(nodePtr, event, observer, this->CallbackCommand);
  return static_cast<int>(observations.size());
}
