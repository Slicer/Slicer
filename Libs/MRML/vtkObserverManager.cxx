/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkObserverManager.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkObserverManager.h"
#include "vtkEventBroker.h"
#include "vtkObservation.h"

#include "vtkCallbackCommand.h"

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
   
  if (this->CallbackCommand)
    {
    this->CallbackCommand->Delete();
    }

  std::map< vtkObject*, vtkUnsignedLongArray*>::iterator iter; 
  for(iter=this->ObserverTags.begin(); iter != this->ObserverTags.end(); iter++)  
    { 
    this->RemoveObjectEvents(iter->first); 
    vtkUnsignedLongArray *objTags = iter->second; 
    objTags->Delete(); 
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
  vtkObject *nodePtrOld = *nodePtr;

  this->RemoveObjectEvents(*nodePtr);

  *nodePtr  = node ;

  if ( node )
    {
    node->Register(this);

    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    this->AddObjectEvents(node, events);
    events->Delete();   
    }
  if (nodePtrOld)
    {
    (nodePtrOld)->UnRegister(this);
    }

}

//----------------------------------------------------------------------------
void vtkObserverManager::SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events)
{
  vtkDebugMacro (<< "SetAndObserveObjectEvents of " << node);
  if (*nodePtr == node && node == NULL)
    {
    return;
    }
  
  this->RemoveObjectEvents(*nodePtr);
  
  vtkObject *nodePtrOld = *nodePtr;
  
  *nodePtr  = node ;
  
  if ( node  )
    {
    vtkDebugMacro (<< "registering " << node << " with " << this << "\n");
    node->Register(this);
    this->AddObjectEvents(node, events);
    }
  if (nodePtrOld)
    {
    (nodePtrOld)->UnRegister(this);
    }
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
    vtkObject *observer = this->GetOwner();
    if ( observer == NULL ) 
      {
      observer = this;
      }
    for (int i=0; i<events->GetNumberOfTuples(); i++)
      {

      vtkObservation *observation = broker->AddObservation (nodePtr, events->GetValue(i), observer, this->CallbackCommand );
      unsigned long tag = observation->GetEventTag();

      objTags->InsertNextValue(tag);
      }
    }

}

