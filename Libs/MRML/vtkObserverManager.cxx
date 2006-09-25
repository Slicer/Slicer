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

#include "vtkCallbackCommand.h"

vtkCxxRevisionMacro(vtkObserverManager, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkObserverManager);

//----------------------------------------------------------------------------
vtkObserverManager::vtkObserverManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
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
  this->RemoveObjectEvents(*nodePtr);

  if (*nodePtr)
    {
    (*nodePtr)->Delete();
    }

  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);
    }
}

//----------------------------------------------------------------------------
void vtkObserverManager::SetAndObserveObject(vtkObject **nodePtr, vtkObject *node)
{
  this->RemoveObjectEvents(*nodePtr);

  if (*nodePtr)
    {
    (*nodePtr)->Delete();
    }

  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);

    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    this->AddObjectEvents(*nodePtr, events);
    events->Delete();   
  }


}

//----------------------------------------------------------------------------
void vtkObserverManager::SetAndObserveObjectEvents(vtkObject **nodePtr, vtkObject *node, vtkIntArray *events)
{
   this->RemoveObjectEvents(*nodePtr);

   if (*nodePtr)
     {
     (*nodePtr)->Delete();
     }
   
  *nodePtr  = node ;

  if ( *nodePtr  )
    {
    (*nodePtr)->Register(this);
    this->AddObjectEvents(*nodePtr, events);
    }
}

//----------------------------------------------------------------------------
void vtkObserverManager::RemoveObjectEvents(vtkObject *nodePtr) 
{
  if (nodePtr)
    {
    std::map< vtkObject*, vtkUnsignedLongArray*>::iterator it =  this->ObserverTags.find(nodePtr); 
    if (it != this->ObserverTags.end()) 
      { 
      vtkUnsignedLongArray* objTags = it->second;
      for (int i=0; i < objTags->GetNumberOfTuples(); i++)
        {
        (nodePtr)->RemoveObserver(objTags->GetValue(i) );
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

    for (int i=0; i<events->GetNumberOfTuples(); i++)
      {
      unsigned long tag = nodePtr->AddObserver(events->GetValue(i), this->CallbackCommand );
      objTags->InsertNextValue(tag);
      }
    }

}

