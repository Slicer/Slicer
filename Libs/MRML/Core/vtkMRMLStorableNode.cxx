/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLStorableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkTagTable.h"

// VTK includes
#include <vtkCallbackCommand.h>

// STD includes
#include <sstream>

const char* vtkMRMLStorableNode::StorageNodeReferenceRole = "storage";
const char* vtkMRMLStorableNode::StorageNodeReferenceMRMLAttributeName = "storageNodeRef";

//----------------------------------------------------------------------------
vtkMRMLStorableNode::vtkMRMLStorableNode()
{
  this->UserTagTable = vtkTagTable::New();
  this->SlicerDataType = "";
  this->AddNodeReferenceRole(this->GetStorageNodeReferenceRole(),
                             this->GetStorageNodeReferenceMRMLAttributeName());

}

//----------------------------------------------------------------------------
vtkMRMLStorableNode::~vtkMRMLStorableNode()
{
  if ( this->UserTagTable )
    {
    this->UserTagTable->Delete();
    this->UserTagTable = NULL;
    }
  this->SlicerDataType.clear();
}

//----------------------------------------------------------------------------
const char* vtkMRMLStorableNode::GetStorageNodeReferenceRole()
{
  return vtkMRMLStorableNode::StorageNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLStorableNode::GetStorageNodeReferenceMRMLAttributeName()
{
  return vtkMRMLStorableNode::StorageNodeReferenceMRMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetAndObserveStorageNodeID(const char *storageNodeID)
{
  this->SetAndObserveNodeReferenceID(this->GetStorageNodeReferenceRole(), storageNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::AddAndObserveStorageNodeID(const char *storageNodeID)
{
  this->AddAndObserveNodeReferenceID(this->GetStorageNodeReferenceRole(), storageNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetAndObserveNthStorageNodeID(int n, const char *storageNodeID)
{
  this->SetAndObserveNthNodeReferenceID(this->GetStorageNodeReferenceRole(), n, storageNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::SetSlicerDataType ( const char *type )
{
  this->SlicerDataType.clear();
  this->SlicerDataType = type;
  if (this->Scene)
    {
    this->Scene->InvokeEvent ( vtkMRMLScene::MetadataAddedEvent );
    }
}


//----------------------------------------------------------------------------
const char* vtkMRMLStorableNode::GetSlicerDataType ()
{
  return ( this->SlicerDataType.c_str() );
}

//----------------------------------------------------------------------------
int vtkMRMLStorableNode::GetNumberOfStorageNodes()
{
  return this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());
}

//----------------------------------------------------------------------------
const char* vtkMRMLStorableNode::GetNthStorageNodeID(int n)
{
  return this->GetNthNodeReferenceID(this->GetStorageNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
const char* vtkMRMLStorableNode::GetStorageNodeID()
{
return this->GetNthStorageNodeID(0);
}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;

  //---write any user tags.
  if ( this->GetUserTagTable() != NULL )
    {
    ss.clear();
    ss.str ( "" );
    int numc = this->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    for (int i=0; i < numc; i++ )
      {
      kwd = this->GetUserTagTable()->GetTagAttribute(i);
      val = this->GetUserTagTable()->GetTagValue (i);
      if (kwd != NULL && val != NULL)
        {
        ss << kwd << "=" << val;
        if ( i < (numc-1) )
          {
          ss << " ";
          }
        }
      }
    if ( ss.str().c_str()!= NULL )
      {
      of << indent << " userTags=\"" << ss.str().c_str() << "\"";
      }
    }

}


//----------------------------------------------------------------------------
void vtkMRMLStorableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    //---Read any user tags
    if (!strcmp (attName, "userTags"))
      {
      if ( this->GetUserTagTable() == NULL )
        {
        this->UserTagTable = vtkTagTable::New();
        }
      std::stringstream ss(attValue);
      std::string kwd = "";
      std::string val = "";
      std::string::size_type i;
      while (!ss.eof())
        {
        std::string tags;
        ss >> tags;
        //--- now pull apart individual tags
        if ( tags.c_str() != NULL )
          {
          i = tags.find("=");
          if ( i != std::string::npos)
            {
            kwd = tags.substr(0, i);
            val = tags.substr(i+1, std::string::npos );
            if ( kwd.c_str() != NULL && val.c_str() != NULL )
              {
              this->GetUserTagTable()->AddOrUpdateTag ( kwd.c_str(), val.c_str(), 0 );
              }
            }
          }
        }
      }
    }

  this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLStorableNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLStorableNode *node = (vtkMRMLStorableNode *) anode;
  if (!node)
    {
    return;
    }

  //---
  //--- Copy any user tags
  //---
  if ( node->GetUserTagTable() != NULL )
    {
    //--- make sure the destination node has a TagTable.
    if ( this->GetUserTagTable() == NULL )
      {
      this->UserTagTable = vtkTagTable::New();
      }

    //--- copy.
    int numc = node->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    int sel;
    for ( int j=0; j < numc; j++ )
      {
      kwd = node->GetUserTagTable()->GetTagAttribute(j);
      val = node->GetUserTagTable()->GetTagValue (j);
      sel = node->GetUserTagTable()->IsTagSelected ( kwd );
      if (kwd != NULL && val != NULL && sel >= 0 )
        {
        this->UserTagTable->AddOrUpdateTag ( kwd, val, sel );
        }
      }
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLStorableNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  this->UserTagTable->PrintSelf(os, indent);

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  for (int i=0; i < numStorageNodes; i++)
    {
    const char * id = this->GetNthNodeReferenceID(this->GetStorageNodeReferenceRole(), i);
    os << indent << "StorageNodeIDs[" << i << "]: " <<
      id << "\n";
    }
}


//-----------------------------------------------------------
void vtkMRMLStorableNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  if (!this->AddToScene)
    {
    return;
    }

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  vtkDebugMacro("UpdateScene: going through the storage node ids: " <<  numStorageNodes);
  for (int i=0; i < numStorageNodes; i++)
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

vtkMRMLStorageNode* vtkMRMLStorableNode::GetNthStorageNode(int n)
{
  return vtkMRMLStorageNode::SafeDownCast(this->GetNthNodeReference(this->GetStorageNodeReferenceRole(), n));
}

vtkMRMLStorageNode* vtkMRMLStorableNode::GetStorageNode()
{
  return this->GetNthStorageNode(0);
}



/*
std::vector<vtkMRMLStorageNode*> vtkMRMLStorableNode::GetStorageNodes()const
{
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
*/

//---------------------------------------------------------------------------
void vtkMRMLStorableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  for (int i=0; i<numStorageNodes; i++)
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

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLStorableNode::CreateDefaultStorageNode()
{
  return NULL;
}

//---------------------------------------------------------------------------
bool vtkMRMLStorableNode::GetModifiedSinceRead()
{
  vtkTimeStamp storedTime = this->GetStoredTime();
  return storedTime < this->StorableModifiedTime;
}

//---------------------------------------------------------------------------
void vtkMRMLStorableNode::StorableModified()
{
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
vtkTimeStamp vtkMRMLStorableNode::GetStoredTime()
{
  vtkTimeStamp storedTime;

  int numStorageNodes = this->GetNumberOfNodeReferences(this->GetStorageNodeReferenceRole());

  for (int i = 0; i < numStorageNodes; ++i)
    {
    vtkMRMLStorageNode *dnode = this->GetNthStorageNode(i);
    if (dnode != NULL && storedTime < dnode->GetStoredTime())
      {
      storedTime = dnode->GetStoredTime();
      }
    }
  return storedTime;
}
