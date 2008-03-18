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
}

//----------------------------------------------------------------------------
vtkMRMLStorableNode::~vtkMRMLStorableNode()
{
  this->SetAndObserveStorageNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

   std::stringstream ss;
  unsigned int n;
  for (n=0; n < this->StorageNodeIDs.size(); n++) 
    {
    ss << this->StorageNodeIDs[n];
    if (n < StorageNodeIDs.size()-1)
      {
      ss << " ";
      }
    }
  if (this->StorageNodeIDs.size() > 0) 
    {
    of << indent << " storageNodeRef=\"" << ss.str().c_str() << "\"";
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
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        this->AddStorageNodeID(id.c_str());
        }

      //this->Scene->AddReferencedNodeID(this->StorageNodeID, this);
      }
    }  
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::UpdateReferenceID(const char *oldID, const char *newID)
{ 
  Superclass::UpdateReferenceID(oldID, newID);
  for (unsigned int i=0; i<this->StorageNodeIDs.size(); i++)
    {
    if ( std::string(oldID) == this->StorageNodeIDs[i])
      {
      this->SetNthStorageNodeID(i, newID);
      }
    }
}
//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLStorableNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLStorableNode *node = (vtkMRMLStorableNode *) anode;
  if (!node)
    {
    return;
    }
      
      
  this->SetAndObserveStorageNodeID(NULL);

  int ndnodes = node->GetNumberOfStorageNodes();
  for (int i=0; i<ndnodes; i++)
    {
    this->SetAndObserveNthStorageNodeID(i, node->StorageNodeIDs[i].c_str());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
   for (unsigned int i=0; i<this->StorageNodeIDs.size(); i++)
    {
    os << indent << "StorageNodeIDs[" << i << "]: " <<
      this->StorageNodeIDs[i] << "\n";
    }
}

//-----------------------------------------------------------
void vtkMRMLStorableNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  vtkDebugMacro("UpdateScene: going through the storage node ids: " <<  this->StorageNodeIDs.size());
  for (unsigned int i=0; i < this->StorageNodeIDs.size(); i++)
    {
    vtkDebugMacro("UpdateScene: getting storage node at i = " << i);
    vtkMRMLStorageNode *pnode = this->GetNthStorageNode(i);

    std::string fname = std::string("(null)");
    if (pnode)
      {      
      if (pnode->GetFileName() != NULL)
        {
        fname = std::string(pnode->GetFileName());
        }
      else if (pnode->GetURI() != NULL)
        {
        fname = std::string(pnode->GetURI());
        }
      vtkDebugMacro("UpdateScene: calling ReadData, fname = " << fname.c_str());
      if (pnode->ReadData(this) == 0)
        {
        scene->SetErrorCode(1);      
        std::string msg = std::string("Error reading file ") + fname;
        scene->SetErrorMessage(msg);
        }
      else
        {
        vtkDebugMacro("UpdateScene: read data called and succeeded reading " << fname.c_str());
        }
      }
    else
      {
      vtkErrorMacro("UpdateScene: error getting " << i << "th storage node, id = " << (this->GetNthStorageNodeID(i) == NULL ? "null" : this->GetNthStorageNodeID(i)));
      }
    }
}

//-----------------------------------------------------------
void vtkMRMLStorableNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  for (unsigned int i=0; i<this->StorageNodeIDs.size(); i++)
    {
    if (this->Scene->GetNodeByID(this->StorageNodeIDs[i]) == NULL)
      {
      this->SetAndObserveNthStorageNodeID(i, NULL);
      }    
    }
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLStorableNode::GetNthStorageNode(int n)
{
  vtkMRMLStorageNode* node = NULL;
  if (this->GetScene() && this->GetNthStorageNodeID(n) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->GetNthStorageNodeID(n));
    node = vtkMRMLStorageNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetStorageNodeID(const char *storageNodeID)
{
  if (this->StorageNodeIDs.empty() && storageNodeID == NULL)
    {
    return;
    }
  if (this->StorageNodeIDs.size() == 1 && storageNodeID != NULL && this->StorageNodeIDs[0] == std::string(storageNodeID) )
    {
    return;
    }
  this->StorageNodeIDs.clear();
  if (storageNodeID != NULL)
    {
    this->StorageNodeIDs.push_back(std::string(storageNodeID));
    }
  if (storageNodeID) 
    { 
    this->Scene->AddReferencedNodeID(storageNodeID, this); 
    } 
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetNthStorageNodeID(int n, const char *storageNodeID)
{
  if (this->StorageNodeIDs.empty() && storageNodeID == NULL)
    {
    return;
    }
  if ((int)this->StorageNodeIDs.size() <= n)
    {
    return;
    }
  if (storageNodeID != NULL && this->StorageNodeIDs[n] == std::string(storageNodeID) )
    {
    return;
    }
  if (storageNodeID != NULL)
    {
    this->StorageNodeIDs[n] = std::string(storageNodeID);
    }
  if (storageNodeID) 
    { 
    this->Scene->AddReferencedNodeID(storageNodeID, this); 
    } 
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::AddStorageNodeID(const char *storageNodeID)
{
  if (storageNodeID == NULL)
    {
    return;
    }

  this->StorageNodeIDs.push_back(std::string(storageNodeID));
  this->Scene->AddReferencedNodeID(storageNodeID, this); 
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetAndObserveStorageNodeID(const char *storageNodeID)
{
  for (unsigned int i=0; i<this->StorageNodes.size(); i++)
    {
    if (this->StorageNodes[i])
      {
      vtkSetAndObserveMRMLObjectMacro(this->StorageNodes[i], NULL);
      //vtkSetMRMLObjectMacro(this->StorageNodes[i], NULL);
      }
    }
  this->StorageNodes.clear();

  this->SetStorageNodeID(storageNodeID);

  vtkMRMLStorageNode *dnode = this->GetStorageNode();
  this->AddAndObserveStorageNode(dnode);

  this->Modified(); 

}


//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetAndObserveNthStorageNodeID(int n, const char *storageNodeID)
{
  if (n >= (int)this->StorageNodes.size())
    {
    this->AddAndObserveStorageNodeID(storageNodeID);
    return;
    }
  vtkSetAndObserveMRMLObjectMacro(this->StorageNodes[n], NULL);
  //vtkSetMRMLObjectMacro(this->StorageNodes[n], NULL);
    
  this->SetNthStorageNodeID(n, storageNodeID);

  vtkMRMLStorageNode *dnode = this->GetNthStorageNode(n);
  if (dnode) 
    {
    vtkSetAndObserveMRMLObjectMacro(this->StorageNodes[n], dnode);
    //vtkSetMRMLObjectMacro(this->StorageNodes[n], dnode);
    }
  this->Modified(); 

}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::AddAndObserveStorageNodeID(const char *storageNodeID)
{

  this->AddStorageNodeID(storageNodeID);

  vtkMRMLStorageNode *dnode = vtkMRMLStorageNode::SafeDownCast(this->GetScene()->GetNodeByID(storageNodeID));
  this->AddAndObserveStorageNode(dnode);
  this->Modified(); 
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::AddAndObserveStorageNode(vtkMRMLStorageNode *dnode)
{
  if (dnode) 
    {
    vtkMRMLStorageNode *pnode = vtkMRMLStorageNode::New();
    vtkSetAndObserveMRMLObjectMacro(pnode, dnode);
    //vtkSetMRMLObjectMacro(pnode, dnode);
    this->StorageNodes.push_back(pnode);
    //pnode->Delete();
    }
}

/*
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

*/

//---------------------------------------------------------------------------
void vtkMRMLStorableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  for (unsigned int i=0; i<this->StorageNodes.size(); i++)
    {
    vtkMRMLStorageNode *dnode = this->GetNthStorageNode(i);
    if (dnode != NULL && dnode == vtkMRMLStorageNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
      {
      vtkDebugMacro("Got a modified event on a storage node, id = " << dnode->GetID());
      // read?
      }
    }
  return;
}

