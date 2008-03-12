/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLStorableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLStorableNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

//----------------------------------------------------------------------------
vtkMRMLStorableNode::vtkMRMLStorableNode()
{
  this->StorageNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLStorableNode::~vtkMRMLStorableNode()
{
  if (this->StorageNodeID) 
    {
    delete [] this->StorageNodeID;
    this->StorageNodeID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->StorageNodeID != NULL) 
    {
    of << indent << " storageNodeRef=\"" << this->StorageNodeID << "\"";
    }
}



//----------------------------------------------------------------------------
void vtkMRMLStorableNode::ReadXMLAttributes(const char** atts)
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
void vtkMRMLStorableNode::UpdateReferenceID(const char *oldID, const char *newID)
{ 
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->StorageNodeID && !strcmp(oldID, this->StorageNodeID))
    {
    this->SetStorageNodeID(newID);
    return;
    }
}
//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLStorableNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLStorableNode *node = (vtkMRMLStorableNode *) anode;
  if (node)
    {
    this->SetStorageNodeID(node->StorageNodeID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);  
  os << indent << "StorageNodeID: " <<
    (this->StorageNodeID ? this->StorageNodeID : "(none)") << "\n";
  
}

//-----------------------------------------------------------
void vtkMRMLStorableNode::UpdateScene(vtkMRMLScene *scene)
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
    if (node && node->ReadData(this) == 0)
      {
      scene->SetErrorCode(1);
      std::string fname;
      if (node->GetFileName() != NULL)
        {
        fname = std::string(node->GetFileName());
        }
      else
        {
        fname = std::string("(null)");
        }
      std::string msg = std::string("Error reading file ") + fname;
      scene->SetErrorMessage(msg);
      }
//    this->SetAndObservePolyData(this->GetPolyData());
    }
}

//-----------------------------------------------------------
void vtkMRMLStorableNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  if (this->StorageNodeID != NULL && this->Scene->GetNodeByID(this->StorageNodeID) == NULL)
    {
    this->SetStorageNodeID(NULL);
    }

}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLStorableNode::GetStorageNode()
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
void vtkMRMLStorableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

