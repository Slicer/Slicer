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
#include "vtkMRMLSequenceStorageNode.h"
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
    this->UserTagTable = nullptr;
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
bool vtkMRMLStorableNode::HasStorageNodeID(const char* storageNodeID)
{
  return this->HasNodeReferenceID(this->GetStorageNodeReferenceRole(), storageNodeID);
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

  std::stringstream ss;

  //---write any user tags.
  if ( this->GetUserTagTable() != nullptr )
    {
    ss.clear();
    ss.str ( "" );
    int numc = this->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    for (int i=0; i < numc; i++ )
      {
      kwd = this->GetUserTagTable()->GetTagAttribute(i);
      val = this->GetUserTagTable()->GetTagValue (i);
      if (kwd != nullptr && val != nullptr)
        {
        ss << kwd << "=" << val;
        if ( i < (numc-1) )
          {
          ss << " ";
          }
        }
      }
    if ( ss.str().c_str()!= nullptr )
      {
      of << " userTags=\"" << ss.str().c_str() << "\"";
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
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    //---Read any user tags
    if (!strcmp (attName, "userTags"))
      {
      if ( this->GetUserTagTable() == nullptr )
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
        if ( tags.c_str() != nullptr )
          {
          i = tags.find("=");
          if ( i != std::string::npos)
            {
            kwd = tags.substr(0, i);
            val = tags.substr(i+1, std::string::npos );
            if ( kwd.c_str() != nullptr && val.c_str() != nullptr )
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
void vtkMRMLStorableNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLStorableNode *node = (vtkMRMLStorableNode *) anode;
  if (!node)
    {
    return;
    }

  //---
  //--- Copy any user tags
  //---
  if ( node->GetUserTagTable() != nullptr )
    {
    //--- make sure the destination node has a TagTable.
    if ( this->GetUserTagTable() == nullptr )
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
      if (kwd != nullptr && val != nullptr && sel >= 0 )
        {
        this->UserTagTable->AddOrUpdateTag ( kwd, val, sel );
        }
      }
    }
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
      if (pnode->GetFileName() != nullptr)
        {
        fname = std::string(pnode->GetFileName());
        }
      else if (pnode->GetURI() != nullptr)
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
      vtkErrorMacro("UpdateScene: error getting " << i << "th storage node, id = " << (this->GetNthStorageNodeID(i) == nullptr ? "null" : this->GetNthStorageNodeID(i)));
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
  vtkMRMLStorageNode* node = nullptr;
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
    if (dnode != nullptr && dnode == vtkMRMLStorageNode::SafeDownCast(caller) &&
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
  return nullptr;
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
    if (dnode != nullptr && storedTime < dnode->GetStoredTime())
      {
      storedTime = dnode->GetStoredTime();
      }
    }
  return storedTime;
}

//---------------------------------------------------------------------------
std::string vtkMRMLStorableNode::GetDefaultStorageNodeClassName(const char* vtkNotUsed(filename) /* =nullptr */)
{
  std::string defaultStorageNodeClassName;
  vtkSmartPointer<vtkMRMLStorageNode> defaultStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(this->CreateDefaultStorageNode());
  if (defaultStorageNode && defaultStorageNode->GetClassName())
    {
    defaultStorageNodeClassName = defaultStorageNode->GetClassName();
    }
  return defaultStorageNodeClassName;
}

//---------------------------------------------------------------------------
bool vtkMRMLStorableNode::AddDefaultStorageNode(const char* filename /* =nullptr */)
{
  vtkMRMLStorageNode* storageNode = this->GetStorageNode();
  if (storageNode)
    {
    // storage node exists already, no need to add a new one
    return true;
    }
  std::string defaultStorageNodeClassName = this->GetDefaultStorageNodeClassName(filename);
  if (defaultStorageNodeClassName.empty())
    {
    // node can be stored in the scene
    return true;
    }
  if (!this->GetScene())
  {
    vtkErrorMacro("vtkMRMLStorableNode::AddDefaultStorageNode failed: node is not in a scene " << (this->GetID() ? this->GetID() : "(unknown)"));
    return false;
  }
  vtkSmartPointer<vtkMRMLNode> newStorageNode = vtkSmartPointer<vtkMRMLNode>::Take(this->GetScene()->CreateNodeByClass(defaultStorageNodeClassName.c_str()));
  storageNode = vtkMRMLStorageNode::SafeDownCast(newStorageNode);
  if (!storageNode)
    {
    vtkErrorMacro("vtkMRMLStorableNode::AddDefaultStorageNode failed: failed to create storage node for node "
      << (this->GetID() ? this->GetID() : "(unknown)"));
    return false;
    }
  storageNode->SetFileName(filename);
  this->GetScene()->AddNode(storageNode);
  this->SetAndObserveStorageNodeID(storageNode->GetID());
  return storageNode;
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLStorableNode:: CreateDefaultSequenceStorageNode()
{
  return vtkMRMLSequenceStorageNode::New();
}
