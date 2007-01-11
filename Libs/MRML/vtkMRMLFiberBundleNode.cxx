/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiberBundleNode.h"

//------------------------------------------------------------------------------
vtkMRMLFiberBundleNode* vtkMRMLFiberBundleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleNode::vtkMRMLFiberBundleNode()
{
  this->StorageNodeID = NULL;
  this->DisplayNodeID = NULL;
  this->FiberBundleDisplayNode = NULL;
  this->PolyData = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleNode::~vtkMRMLFiberBundleNode()
{
  if (this->StorageNodeID) 
    {
    delete [] this->StorageNodeID;
    this->StorageNodeID = NULL;
    }
  this->SetAndObserveDisplayNodeID( NULL);

  this->SetAndObservePolyData(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

   if (this->StorageNodeID != NULL) 
    {
    of << indent << "storageNodeRef=\"" << this->StorageNodeID << "\" ";
    }
  if (this->DisplayNodeID != NULL) 
    {
    of << indent << "displayNodeRef=\"" << this->DisplayNodeID << "\" ";
    }

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (!strcmp(oldID, this->StorageNodeID))
    {
    this->SetStorageNodeID(newID);
    }
  if (!strcmp(oldID, this->DisplayNodeID))
    {
    this->SetDisplayNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "storageNodeRef")) 
      {
      this->SetStorageNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->StorageNodeID, this);
      }
    else if (!strcmp(attName, "displayNodeRef")) 
      {
      this->SetDisplayNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiberBundleNode *node = (vtkMRMLFiberBundleNode *) anode;

  this->SetStorageNodeID(node->StorageNodeID);
  this->SetDisplayNodeID(node->DisplayNodeID);
  this->SetPolyData(node->PolyData);

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "StorageNodeID: " <<
    (this->StorageNodeID ? this->StorageNodeID : "(none)") << "\n";

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";

  os << "\nPoly Data:\n";
  if (this->PolyData) 
    {
    this->PolyData->PrintSelf(os, indent.GetNextIndent());
    }

}

//-----------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

  if (this->GetStorageNodeID() == NULL) 
    {
    //vtkErrorMacro("No reference StorageNodeID found");
    return;
    }

  vtkMRMLNode* mnode = scene->GetNodeByID(this->StorageNodeID);
  if (mnode) 
    {
    vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(mnode);
    node->ReadData(this);
    this->SetAndObservePolyData(this->GetPolyData());
    this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());
    }
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->DisplayNodeID != NULL && this->Scene->GetNodeByID(this->DisplayNodeID) == NULL)
    {
    this->SetAndObserveDisplayNodeID(NULL);
    }
 if (this->StorageNodeID != NULL && this->Scene->GetNodeByID(this->StorageNodeID) == NULL)
    {
    this->SetStorageNodeID(NULL);
    }
}

//-----------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLFiberBundleNode::GetStorageNode()
{
  vtkMRMLStorageNode* node = NULL;
  if (this->GetScene() && this->GetStorageNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->StorageNodeID);
    node = vtkMRMLStorageNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleNode::GetDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode* node = NULL;
  if (this->GetScene() && this->GetDisplayNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
    node = vtkMRMLFiberBundleDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->FiberBundleDisplayNode, NULL);

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLFiberBundleDisplayNode *dnode = this->GetDisplayNode();

  vtkSetAndObserveMRMLObjectMacro(this->FiberBundleDisplayNode, dnode);

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::SetAndObservePolyData(vtkPolyData *polyData)
{
if (this->PolyData != NULL)
    {
    this->PolyData->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  unsigned long mtime1, mtime2;
  mtime1 = this->GetMTime();
  this->SetPolyData(polyData);
  mtime2 = this->GetMTime();

  if (this->PolyData != NULL)
    {
    this->PolyData->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  if (mtime1 != mtime2)
    {
    this->InvokeEvent( vtkMRMLFiberBundleNode::PolyDataModifiedEvent , this);
    }
}


//---------------------------------------------------------------------------
void vtkMRMLFiberBundleNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLFiberBundleDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLFiberBundleDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("Fiber bundle node invoking display modified event");
    this->InvokeEvent(vtkMRMLFiberBundleNode::DisplayModifiedEvent, NULL);
    }
  else if (this->PolyData == vtkPolyData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->ModifiedSinceRead = true;
    vtkDebugMacro("Fiber bundle node invoking poly data modified event");
    this->InvokeEvent(vtkMRMLFiberBundleNode::PolyDataModifiedEvent, NULL);
    }
  return;
}

