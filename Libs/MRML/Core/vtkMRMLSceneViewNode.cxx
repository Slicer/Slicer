/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLSceneViewStorageNode.h"

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <sstream>

vtkCxxSetObjectMacro(vtkMRMLSceneViewNode, ScreenShot, vtkImageData);

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSceneViewNode);

//----------------------------------------------------------------------------
vtkMRMLSceneViewNode::vtkMRMLSceneViewNode()
{
  this->HideFromEditors = 0;

  this->Nodes = NULL;
  this->ScreenShot = NULL;
  this->ScreenShotType = 0;
}

//----------------------------------------------------------------------------
vtkMRMLSceneViewNode::~vtkMRMLSceneViewNode()
{
  if (this->Nodes) 
    {
    this->Nodes->GetNodes()->RemoveAllItems();
    //this->Nodes->Clear(1);
    this->Nodes->Delete();
    }
  if (this->ScreenShot)
    {
    this->ScreenShot->Delete();
    this->ScreenShot = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " screenshotType=\"" << this->GetScreenShotType() << "\"";

  vtkStdString description = this->GetSceneViewDescription();
  vtksys::SystemTools::ReplaceString(description,"\n","[br]");

  of << indent << " sceneViewDescription=\"" << description << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::WriteNodeBodyXML(ostream& of, int nIndent)
{
  this->SetAbsentStorageFileNames();

  vtkMRMLNode * node = NULL;
  int n;
  for (n=0; n < this->Nodes->GetNodes()->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetNodes()->GetItemAsObject(n);
    if (node && !node->IsA("vtkMRMLSceneViewNode") && node->GetSaveWithScene())
      {
      vtkIndent vindent(nIndent+1);
      of << vindent << "<" << node->GetNodeTagName() << "\n";

      node->WriteXML(of, nIndent + 2);

      of << vindent << ">";
      node->WriteNodeBodyXML(of, nIndent+1);
      of << "</" << node->GetNodeTagName() << ">\n";
      }
    }
    
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::ReadXMLAttributes(const char** atts)
{

  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "screenshotType"))
      {
      std::stringstream ss;
      ss << attValue;
      int screenshotType;
      ss >> screenshotType;
      this->SetScreenShotType(screenshotType);
      }
    else if(!strcmp(attName, "sceneViewDescription"))
      {
      // can have spaces in the description, don't use stringstream
      vtkStdString sceneViewDescription = vtkStdString(attValue);
      vtksys::SystemTools::ReplaceString(sceneViewDescription,"[br]","\n");
      this->SetSceneViewDescription(sceneViewDescription);
      }
    }

  // for backward compatibility:
  
  // now read the screenCapture if there's a directory for them
  // TODO: don't do this if there is a storage node already, but the problem
  // is that the storage node will get set after, so GetStorageNode returns
  // null right now
  vtkStdString screenCapturePath;
  if (this->GetScene() &&
      this->GetScene()->GetRootDirectory())
    {
    screenCapturePath += this->GetScene()->GetRootDirectory();
    }
  else
    {
    screenCapturePath += ".";
    }
  screenCapturePath += "/";
  screenCapturePath += "ScreenCaptures/";
  
  vtkStdString screenCaptureFilename;
  screenCaptureFilename += screenCapturePath;
  if (this->GetID())
    {
    screenCaptureFilename += this->GetID();
    }
  else
    {
    screenCaptureFilename += "vtkMRMLSceneViewNodeNoID";
    }
  screenCaptureFilename += ".png";
  
  
  if (vtksys::SystemTools::FileExists(vtksys::SystemTools::ConvertToOutputPath(screenCaptureFilename.c_str()).c_str(),true))
    {
    // create a storage node and use it to read the file
    vtkMRMLStorageNode *storageNode = this->GetStorageNode();
    if (storageNode == NULL)
      {
      // only read the directory if there isn't a storage node already
      storageNode = this->CreateDefaultStorageNode();
      if (storageNode)
        {
        storageNode->SetFileName(vtksys::SystemTools::ConvertToOutputPath(screenCaptureFilename.c_str()).c_str());
        if (this->GetScene())
          {
          this->GetScene()->AddNode(storageNode);
          }
        vtkWarningMacro("ReadXMLAttributes: found the ScreenCapture directory, creating a storage node to read the image file at\n\t" << storageNode->GetFileName() << "\n\tImage data be overwritten if there is a storage node pointing to another file");
        storageNode->ReadData(this);
        storageNode->Delete();
        }
      }
    else
      {
      vtkWarningMacro("ReadXMLAttributes: there is a ScreenCaptures directory with a valid file in it, but waiting to let the extant storage node read it's image file");
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::ProcessChildNode(vtkMRMLNode *node)
{
  int disabledModify = node->StartModify();

  Superclass::ProcessChildNode(node);
  node->SetAddToSceneNoModify(0);

  node->EndModify(disabledModify);

  if (this->Nodes == NULL)
    {
    this->Nodes = vtkMRMLScene::New();
    }  
  node->SetScene(this->Nodes);
  this->Nodes->GetNodes()->vtkCollection::AddItem((vtkObject *)node);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLSceneViewNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSceneViewNode *snode = (vtkMRMLSceneViewNode *) anode;

  this->SetScreenShot(vtkMRMLSceneViewNode::SafeDownCast(anode)->GetScreenShot());
  this->SetScreenShotType(vtkMRMLSceneViewNode::SafeDownCast(anode)->GetScreenShotType());
  this->SetSceneViewDescription(vtkMRMLSceneViewNode::SafeDownCast(anode)->GetSceneViewDescription());

  if (this->Nodes == NULL)
    {
    this->Nodes = vtkMRMLScene::New();
    }
  else
    {
    this->Nodes->GetNodes()->RemoveAllItems();
    }
  vtkMRMLNode *node = NULL;
  if ( snode->Nodes != NULL )
    {
    int n;
    for (n=0; n < snode->Nodes->GetNodes()->GetNumberOfItems(); n++) 
      {
      node = (vtkMRMLNode*)snode->Nodes->GetNodes()->GetItemAsObject(n);
      if (node)
        {
        node->SetScene(this->Nodes);
        this->Nodes->GetNodes()->vtkCollection::AddItem((vtkObject *)node);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::UpdateScene(vtkMRMLScene *scene)
{
  // the superclass update scene ensures that the storage node is read into
  // this storable node
  Superclass::UpdateScene(scene);
  if (!scene)
    {
    return;
    }
  if (this->Nodes)
    {
    this->Nodes->CopyNodeReferences(scene);
    this->Nodes->UpdateNodeChangedIDs();
    this->Nodes->UpdateNodeReferences();
    }
  this->UpdateSnapshotScene(this->Nodes);
}
//----------------------------------------------------------------------------

void vtkMRMLSceneViewNode::UpdateSnapshotScene(vtkMRMLScene *)
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->Nodes->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  // prevent data read in UpdateScene
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetNodes()->GetItemAsObject(n));
    if (node) 
      {
      node->SetAddToSceneNoModify(0);
      }
    }

  // update nodes in the snapshot
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetNodes()->GetItemAsObject(n));
    if (node) 
      {
      node->UpdateScene(this->Nodes);
      }
    }

  /**
  // update nodes in the snapshot
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetNodes()->GetItemAsObject(n));
    if (node)
      {
      node->SetAddToSceneNoModify(1);
      }
    }
    ***/
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::StoreScene()
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    this->Nodes = vtkMRMLScene::New();
    }
  else
    {
    this->Nodes->GetNodes()->RemoveAllItems();
    }

  if (this->GetScene())
    {
    this->Nodes->SetRootDirectory(this->GetScene()->GetRootDirectory());
    }

  /// \todo: GetNumberOfNodes/GetNthNode is slow, fasten by using collection
  /// iterators.
  for (int n=0; n < this->Scene->GetNumberOfNodes(); n++) 
    {
    vtkMRMLNode *node = this->Scene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneViewNode") && !node->IsA("vtkMRMLSnapshotClipNode")  && node->GetSaveWithScene() )
      {
      vtkMRMLNode *newNode = node->CreateNodeInstance();

      newNode->SetScene(this->Nodes);
      newNode->CopyWithoutModifiedEvent(node);
      newNode->SetAddToSceneNoModify(0);
      newNode->SetID(node->GetID());

      this->Nodes->GetNodes()->vtkCollection::AddItem((vtkObject *)newNode);
      //--- Try deleting copy after collection has a reference to it,
      //--- in order to eliminate debug leaks..
      newNode->Delete();

      // sanity check
      assert(newNode->GetScene() == this->Nodes);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::RestoreScene()
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->Nodes->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  this->Scene->StartState(vtkMRMLScene::RestoreState);

  // remove nodes in the scene which are not stored in the snapshot
  std::map<std::string, vtkMRMLNode*> snapshotMap;
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetNodes()->GetItemAsObject(n));
    if (node) 
      {
      /***
      const char *newID = this->Scene->GetChangedID(node->GetID());
      if (newID)
        {
        snapshotMap[newID] = node;
        }
      else
        {
        snapshotMap[node->GetID()] = node;
        }
      ***/
      if (node->GetID()) 
        {
        snapshotMap[node->GetID()] = node;
        }
      }
    }
  std::vector<vtkMRMLNode*> removedNodes;
  unsigned int nnodesScene = this->Scene->GetNumberOfNodes();
  for (n=0; n<nnodesScene; n++)
    {
    node = this->Scene->GetNthNode(n);
    if (node)
      {
      std::map<std::string, vtkMRMLNode*>::iterator iter = snapshotMap.find(std::string(node->GetID()));
      vtkSmartPointer<vtkMRMLHierarchyNode> hnode = vtkMRMLHierarchyNode::SafeDownCast(node);
      // don't remove the scene view nodes, the snapshot clip nodes, hierarchy nodes associated with the
      // sceneview nodes nor top level scene view hierarchy nodes
      if (iter == snapshotMap.end() &&
          !node->IsA("vtkMRMLSceneViewNode") &&
          !(hnode && hnode->GetAssociatedNode() && hnode->GetAssociatedNode()->IsA("vtkMRMLSceneViewNode")) &&
          !(hnode && hnode->GetName() && !strncmp(hnode->GetName(), "SceneViewToplevel", 17)) &&
          !node->IsA("vtkMRMLSnapshotClipNode") &&
          node->GetSaveWithScene())
        {
        removedNodes.push_back(node);
        }
      }
    }
  for(n=0; n<removedNodes.size(); n++)
    {
    this->Scene->RemoveNode(removedNodes[n]);
    }

  std::vector<vtkMRMLNode *> addedNodes;
  for (n=0; n < nnodesSanpshot; n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetNodes()->GetItemAsObject(n);
    if (node)
      {
      /***
      const char *newID = this->Scene->GetChangedID(node->GetID());
      if (newID == NULL)
        {
        newID = node->GetID();
        }
      vtkMRMLNode *snode = this->Scene->GetNodeByID(newID);
      ***/
      
      vtkMRMLNode *snode = this->Scene->GetNodeByID(node->GetID());

      if (snode)
        {
        snode->SetScene(this->Scene);
        // to prevent copying of default info if not stored in sanpshot
        snode->CopyWithSingleModifiedEvent(node);
        // to prevent reading data on UpdateScene()
        snode->SetAddToSceneNoModify(0);
        }
      else 
        {
        addedNodes.push_back(node);
        node->SetAddToSceneNoModify(1);
        this->Scene->AddNode(node);
        // to prevent reading data on UpdateScene()
        // but new nodes should read their data
        //node->SetAddToSceneNoModify(0);
        }
      }
    }

  // update all nodes in the scene

  //this->Scene->UpdateNodeReferences(this->Nodes);

  nnodesScene = this->Scene->GetNumberOfNodes();
  for (n=0; n<nnodesScene; n++) 
    {
    node = this->Scene->GetNthNode(n);
    if(!node->IsA("vtkMRMLSceneViewNode") && !node->IsA("vtkMRMLSnapshotClipNode") && node->GetSaveWithScene())
      {
      node->UpdateScene(this->Scene);
      }
    }

  // reset AddToScene
  for (n=0; n < nnodesSanpshot; n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetNodes()->GetItemAsObject(n);
    if (node)
      {
      node->SetAddToSceneNoModify(1);
      }
    }

  //this->Scene->SetIsClosing(0);
  for(n=0; n<addedNodes.size(); n++)
    {
    //addedNodes[n]->UpdateScene(this->Scene);
    //this->Scene->InvokeEvent(vtkMRMLScene::NodeAddedEvent, addedNodes[n] );
    }

  this->Scene->EndState(vtkMRMLScene::RestoreState);

#ifndef NDEBUG
  // sanity checks
  nnodesScene = this->Scene->GetNumberOfNodes();
  for (n=0; n<nnodesScene; n++)
    {
    node = this->Scene->GetNthNode(n);
    assert(node->GetScene() == this->Scene);
    }
#endif
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::SetAbsentStorageFileNames()
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->Nodes->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetNodes()->GetItemAsObject(n));
    if (node) 
      {
      // for storage nodes replace full path with relative
      vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(node);
      if (snode && (snode->GetFileName() == NULL || std::string(snode->GetFileName()) == "") )
        {
        vtkMRMLNode *node1 = this->Scene->GetNodeByID(snode->GetID());
        if (node1)
          {
          vtkMRMLStorageNode *snode1 = vtkMRMLStorageNode::SafeDownCast(node1);
          if (snode1)
            {
            snode->SetFileName(snode1->GetFileName());
            }
          }
        }
      } //if (node) 
    } //for (n=0; n<nnodesSanpshot; n++) 
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLSceneViewNode::CreateDefaultStorageNode()
{
  return vtkMRMLSceneViewStorageNode::New();
}
