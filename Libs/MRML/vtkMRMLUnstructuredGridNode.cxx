/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLUnstructuredGridNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"

vtkMRMLUnstructuredGridNode::vtkMRMLUnstructuredGridNode()
{
  this->StorageNodeID = NULL;
  this->UnstructuredGrid = NULL;
}

vtkMRMLUnstructuredGridNode::~vtkMRMLUnstructuredGridNode()
{
  if ( this->UnstructuredGrid)
    {
    this->UnstructuredGrid->Delete();
    }
  if (this->StorageNodeID) 
    {
    delete [] this->StorageNodeID;
    this->StorageNodeID = NULL;
    }

}

//------------------------------------------------------------------------------
vtkMRMLUnstructuredGridNode* vtkMRMLUnstructuredGridNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUnstructuredGridNode");
  if(ret)
    {
    return (vtkMRMLUnstructuredGridNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUnstructuredGridNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLUnstructuredGridNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUnstructuredGridNode");
  if(ret)
    {
    return (vtkMRMLUnstructuredGridNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUnstructuredGridNode;
}


//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

   if (this->StorageNodeID != NULL) 
    {
    of << indent << "storageNodeRef=\"" << this->StorageNodeID << "\" ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);

  if (this->StorageNodeID && !strcmp(oldID, this->StorageNodeID))
    {
    this->SetStorageNodeID(newID);
    return;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::ReadXMLAttributes(const char** atts)
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
      //this->Scene->AddReferencedNodeID(this->StorageNodeID, this);
      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLUnstructuredGridNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLUnstructuredGridNode *node = (vtkMRMLUnstructuredGridNode *) anode;

  this->SetStorageNodeID(node->StorageNodeID);
}

//-------------------------------
void vtkMRMLUnstructuredGridNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
  this->UnstructuredGrid->Print(os);
  os << indent << "StorageNodeID: " <<
    (this->StorageNodeID ? this->StorageNodeID : "(none)") << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::SetAndObserveUnstructuredGrid(vtkUnstructuredGrid *unstructuredGrid)
{
if (this->UnstructuredGrid != NULL)
    {
    this->UnstructuredGrid->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  unsigned long mtime1, mtime2;
  mtime1 = this->GetMTime();
  this->SetUnstructuredGrid(unstructuredGrid);
  mtime2 = this->GetMTime();

  if (this->UnstructuredGrid != NULL)
    {
    this->UnstructuredGrid->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  if (mtime1 != mtime2)
    {
    this->InvokeEvent( vtkCommand::ModifiedEvent , this);
    }
}

//-----------------------------------------------------------
void vtkMRMLUnstructuredGridNode::UpdateReferences()
{
 Superclass::UpdateReferences();

 if (this->StorageNodeID != NULL && this->Scene->GetNodeByID(this->StorageNodeID) == NULL)
    {
    this->SetStorageNodeID(NULL);
    }
}

//-----------------------------------------------------------
void vtkMRMLUnstructuredGridNode::UpdateScene(vtkMRMLScene *scene)
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
    if (node->ReadData(this) == 0)
      {
      scene->SetErrorCode(1);
      std::string msg = std::string("Error reading model file ") + std::string(node->GetFileName());
      scene->SetErrorMessage(msg);
      }
    this->SetAndObservePolyData(this->GetPolyData());
    this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());
    }

  mnode = scene->GetNodeByID(this->DisplayNodeID);
  if (mnode) 
    {
    vtkMRMLUnstructuredGridDisplayNode *node  = dynamic_cast < vtkMRMLUnstructuredGridDisplayNode *>(mnode);
    if (node)
      {
      // set input/output to/from display pipeline
      node->SetUnstructuredGrid(this->GetUnstructuredGrid());
      this->SetAndObservePolyData(node->GetPolyData());
      }
    }
}

vtkMRMLStorageNode* vtkMRMLUnstructuredGridNode::GetStorageNode()
{
  vtkMRMLStorageNode* node = NULL;
  if (this->GetScene() && this->GetStorageNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->StorageNodeID);
    node = vtkMRMLStorageNode::SafeDownCast(snode);
    }
  return node;
}

//---------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

