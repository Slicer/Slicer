/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSceneViewNode.cxx,v $
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
#include <stack>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSceneViewNode);

//----------------------------------------------------------------------------
vtkMRMLSceneViewNode::vtkMRMLSceneViewNode()
{
  this->HideFromEditors = 0;

  this->SnapshotScene = nullptr;
//  this->ScreenShot = vtkImageData::New();
  this->ScreenShot = nullptr;
  this->ScreenShotType = 0;
}

//----------------------------------------------------------------------------
vtkMRMLSceneViewNode::~vtkMRMLSceneViewNode()
{
  if (this->SnapshotScene)
    {
    this->SnapshotScene->Delete();
    this->SnapshotScene = nullptr;
    }
  if (this->ScreenShot)
    {
    this->ScreenShot->Delete();
    this->ScreenShot = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " screenshotType=\"" << this->GetScreenShotType() << "\"";

  vtkStdString description = this->GetSceneViewDescription();
  vtksys::SystemTools::ReplaceString(description,"\n","<br>");

  of << " sceneViewDescription=\"" << this->XMLAttributeEncodeString(description) << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::WriteNodeBodyXML(ostream& of, int nIndent)
{
  if (!this->SnapshotScene)
    {
    return;
    }

  // first make sure that the scene view scene is to be saved relative to the same place as the main scene
  this->SnapshotScene->SetRootDirectory(this->GetScene()->GetRootDirectory());
  this->SetAbsentStorageFileNames();

  for (int n=0; n < this->SnapshotScene->GetNodes()->GetNumberOfItems(); n++)
    {
    vtkMRMLNode* node = (vtkMRMLNode*)this->SnapshotScene->GetNodes()->GetItemAsObject(n);
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
  while (*atts != nullptr)
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
    if (storageNode == nullptr)
      {
      // only read the directory if there isn't a storage node already
      this->AddDefaultStorageNode(vtksys::SystemTools::ConvertToOutputPath(screenCaptureFilename.c_str()).c_str());
      storageNode = this->GetStorageNode();
      if (storageNode)
        {
        vtkWarningMacro("ReadXMLAttributes: found the ScreenCapture directory, creating a storage node to read the image file at\n\t" << storageNode->GetFileName() << "\n\tImage data be overwritten if there is a storage node pointing to another file");
        storageNode->ReadData(this);
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
  // for the child nodes in the scene view scene, we don't want to invoke any
  // pending modified events when done processing them, so just use the bare
  // DisableModifiedEventOn and SetDisableModifiedEvent calls rather than
  // using StartModify and EndModify
  int disabledModify = this->GetDisableModifiedEvent();
  this->DisableModifiedEventOn();

  int disabledModifyNode = node->GetDisableModifiedEvent();
  node->DisableModifiedEventOn();

  Superclass::ProcessChildNode(node);
  node->SetAddToSceneNoModify(0);

  if (this->SnapshotScene == nullptr)
    {
    this->SnapshotScene = vtkMRMLScene::New();
    }
  this->SnapshotScene->GetNodes()->vtkCollection::AddItem((vtkObject *)node);

  this->SnapshotScene->AddNodeID(node);

  node->SetScene(this->SnapshotScene);

  node->SetDisableModifiedEvent(disabledModifyNode);
  this->SetDisableModifiedEvent(disabledModify);

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

  if (this->SnapshotScene == nullptr)
    {
    this->SnapshotScene = vtkMRMLScene::New();
    }
  else
    {
    this->SnapshotScene->GetNodes()->RemoveAllItems();
    this->SnapshotScene->ClearNodeIDs();
    }
  vtkMRMLNode *node = nullptr;
  if ( snode->SnapshotScene != nullptr )
    {
    int n;
    for (n=0; n < snode->SnapshotScene->GetNodes()->GetNumberOfItems(); n++)
      {
      node = (vtkMRMLNode*)snode->SnapshotScene->GetNodes()->GetItemAsObject(n);
      if (node)
        {
        node->SetScene(this->SnapshotScene);
        this->SnapshotScene->AddNodeID(node);
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
  if (this->SnapshotScene)
    {
    // node references are in (this->SavedScene) already, so they should not be modified
    // but there could have been some node ID changes, so get them and update the
    // references accordingly
    this->SnapshotScene->CopyNodeChangedIDs(scene);
    this->SnapshotScene->UpdateNodeChangedIDs();
    this->SnapshotScene->UpdateNodeReferences();
    }
  this->UpdateStoredScene();
}
//----------------------------------------------------------------------------

void vtkMRMLSceneViewNode::UpdateStoredScene()
{
  if (this->Scene == nullptr)
    {
    return;
    }

  if (this->SnapshotScene == nullptr)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->SnapshotScene->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = nullptr;

  // prevent data read in UpdateScene
  for (n=0; n<nnodesSanpshot; n++)
    {
    node  = vtkMRMLNode::SafeDownCast(this->SnapshotScene->GetNodes()->GetItemAsObject(n));
    if (node)
      {
      node->SetAddToSceneNoModify(0);
      }
    }

  // update nodes in the snapshot
  for (n=0; n<nnodesSanpshot; n++)
    {
    node  = vtkMRMLNode::SafeDownCast(this->SnapshotScene->GetNodes()->GetItemAsObject(n));
    if (node)
      {
      node->UpdateScene(this->SnapshotScene);
      }
    }

  /**
  // update nodes in the snapshot
  for (n=0; n<nnodesSanpshot; n++)
    {
    node  = vtkMRMLNode::SafeDownCast(this->Nodes->GetNodes()->GetItemAsObject(n));
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
  if (this->Scene == nullptr)
    {
    return;
    }

  if (this->SnapshotScene == nullptr)
    {
    this->SnapshotScene = vtkMRMLScene::New();
    }
  else
    {
    this->SnapshotScene->Clear(1);
    }

  if (this->GetScene())
    {
    this->SnapshotScene->SetRootDirectory(this->GetScene()->GetRootDirectory());
    }

  // make sure that any storable nodes in the scene have storage nodes before
  // saving them to the scene view, this prevents confusion on scene view
  // restore with mismatched nodes.
  std::vector<vtkMRMLNode *> nodes;
  int nnodes = this->GetScene()->GetNodesByClass("vtkMRMLStorableNode", nodes);
  for (int i = 0; i < nnodes; ++i)
    {
    vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(nodes[i]);
    if (storableNode)
      {
      if (this->IncludeNodeInSceneView(storableNode) &&
          storableNode->GetSaveWithScene() )
        {
        if (!storableNode->GetStorageNode())
          {
          // No storage node in the main scene, try add one.
          storableNode->AddDefaultStorageNode();
          vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
          if (storageNode)
            {
            std::string fileBaseName = std::string(storableNode->GetName());
            std::string extension = storageNode->GetDefaultWriteFileExtension();
            std::string storageFileName = fileBaseName + std::string(".") + extension;
            storageNode->SetFileName(storageFileName.c_str());
            }
          }
        }
      }
    }

  /// \todo: GetNumberOfNodes/GetNthNode is slow, fasten by using collection
  /// iterators.
  for (int n=0; n < this->Scene->GetNumberOfNodes(); n++)
    {
    vtkMRMLNode *node = this->Scene->GetNthNode(n);
    if (this->IncludeNodeInSceneView(node) &&
        node->GetSaveWithScene() )
      {
      vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(node->CreateNodeInstance());

      newNode->SetScene(this->SnapshotScene);

      int oldMode = newNode->GetDisableModifiedEvent();
      newNode->DisableModifiedEventOn();
      newNode->Copy(node);
      newNode->SetDisableModifiedEvent(oldMode);

      newNode->SetID(node->GetID());

      newNode->SetAddToSceneNoModify(1);
      this->SnapshotScene->AddNode(newNode);
      newNode->SetAddToSceneNoModify(0);

      // sanity check
      assert(newNode->GetScene() == this->SnapshotScene);
      }
    }
  this->SnapshotScene->CopyNodeReferences(this->GetScene());
  this->SnapshotScene->CopyNodeChangedIDs(this->GetScene());
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::AddMissingNodes()
{
  if (this->Scene == nullptr)
    {
    vtkWarningMacro("No scene to add nodes from");
    return;
    }
  if (this->SnapshotScene == nullptr)
    {
    vtkWarningMacro("No scene to add to");
    return;
    }
  unsigned int numNodesInSceneView = this->SnapshotScene->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = nullptr;
  // build the list of nodes in the scene view
  std::map<std::string, vtkMRMLNode*> snapshotMap;
  for (n=0; n<numNodesInSceneView; n++)
    {
    node  = vtkMRMLNode::SafeDownCast(this->SnapshotScene->GetNodes()->GetItemAsObject(n));
    if (node && node->GetID())
      {
      snapshotMap[node->GetID()] = node;
      }
    }
  if (snapshotMap.size() == 0)
    {
    // no missing nodes
    return;
    }

  // add the missing ones from the scene
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  int nodesAdded = 0;
  for (sceneNodes->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    std::map<std::string, vtkMRMLNode*>::iterator iter = snapshotMap.find(std::string(node->GetID()));
    // ignore scene view nodes, the snapshot clip nodes, hierarchy nodes associated with the
    // sceneview nodes nor top level scene view hierarchy nodes
    if (iter == snapshotMap.end() &&
        this->IncludeNodeInSceneView(node) &&
        node->GetSaveWithScene())
      {
      vtkDebugMacro("AddMissingNodes: Adding node with id " << node->GetID());

      vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(node->CreateNodeInstance());

      newNode->SetScene(this->SnapshotScene);

      int oldMode = newNode->GetDisableModifiedEvent();
      newNode->DisableModifiedEventOn();
      newNode->Copy(node);
      newNode->SetDisableModifiedEvent(oldMode);

      newNode->SetID(node->GetID());

      newNode->SetAddToSceneNoModify(1);
      this->SnapshotScene->AddNode(newNode);
      newNode->SetAddToSceneNoModify(0);

      // sanity check
      assert(newNode->GetScene() == this->SnapshotScene);

      nodesAdded++;
      }
    }
  vtkDebugMacro("AddMissingNodes: Added " << nodesAdded << " nodes to this scene view");
  if (nodesAdded > 0)
    {
    // update references for any ids that got changed
    this->SnapshotScene->UpdateNodeReferences();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::RestoreScene(bool removeNodes)
{
  if (this->Scene == nullptr)
    {
    vtkWarningMacro("No scene to restore onto");
    return;
    }
  if (this->SnapshotScene == nullptr)
    {
    vtkWarningMacro("No nodes to restore");
    return;
    }

  unsigned int numNodesInSceneView = this->SnapshotScene->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = nullptr;

  this->Scene->StartState(vtkMRMLScene::RestoreState);

  // remove nodes in the scene which are not stored in the snapshot
  std::map<std::string, vtkMRMLNode*> snapshotMap;
  for (n=0; n<numNodesInSceneView; n++)
    {
    node  = vtkMRMLNode::SafeDownCast(this->SnapshotScene->GetNodes()->GetItemAsObject(n));
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
  // Identify which nodes must be removed from the scene.
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  // Use smart pointer to ensure the nodes still exist when being removed.
  // Indeed, removing a node can have the side effect of removing other nodes.
  std::stack<vtkSmartPointer<vtkMRMLNode> > removedNodes;
  for (sceneNodes->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    std::map<std::string, vtkMRMLNode*>::iterator iter = snapshotMap.find(std::string(node->GetID()));
    // don't remove the scene view nodes, the snapshot clip nodes, hierarchy nodes associated with the
    // sceneview nodes nor top level scene view hierarchy nodes
    if (iter == snapshotMap.end() &&
        this->IncludeNodeInSceneView(node) &&
        node->GetSaveWithScene())
      {
      removedNodes.push(vtkSmartPointer<vtkMRMLNode>(node));
      }
    }
  while(!removedNodes.empty())
    {
    vtkMRMLNode* nodeToRemove = removedNodes.top().GetPointer();
    // Remove the node only if it's not part of the scene.
    bool isNodeInScene = (nodeToRemove->GetScene() == this->Scene);
    // Decrease reference count before removing it from the scene
    // to give the opportunity of the node to be deleted in RemoveNode
    // (standard behavior).
    removedNodes.pop();
    if (isNodeInScene)
      {
      if (removeNodes)
        {
        this->Scene->RemoveNode(nodeToRemove);
        }
      else
        {
        vtkErrorMacro("RestoreScene encountered a node in the scene that needs to be removed to restore the scene view '" << this->GetSceneViewDescription().c_str() << "'.\n\tNot removing node named '" << nodeToRemove->GetName() << "',\n\tReturning without restoring the scene.");
        // signal that done trying to restore the scene
        this->Scene->EndState(vtkMRMLScene::RestoreState);
        // signal that there is an error state
        this->Scene->SetErrorMessage("Unable to restore scene, data in main Slicer scene that is not included in the scene view");
        this->Scene->SetErrorCode(1);
        return;
        }
      }
    }

  std::vector<vtkMRMLNode *> addedNodes;
  for (n=0; n < numNodesInSceneView; n++)
    {
    node = vtkMRMLNode::SafeDownCast(this->SnapshotScene->GetNodes()->GetItemAsObject(n));
    if (node)
      {
      // don't restore certain nodes that might have been in the scene view by mistake
      if (this->IncludeNodeInSceneView(node))
        {
        vtkMRMLNode *snode = this->Scene->GetNodeByID(node->GetID());

        if (snode)
          {
          snode->SetScene(this->Scene);
          // to prevent copying of default info if not stored in snapshot
          MRMLNodeModifyBlocker blocker(snode);
          snode->Copy(node);
          // to prevent reading data on UpdateScene()
          snode->SetAddToSceneNoModify(0);
          }
        else
          {
          vtkMRMLNode *newNode = node->CreateNodeInstance();
          newNode->CopyWithScene(node);

          addedNodes.push_back(newNode);
          newNode->SetAddToSceneNoModify(1);
          this->Scene->AddNode(newNode);
          newNode->Delete();

          // to prevent reading data on UpdateScene()
          // but new nodes should read their data
          //node->SetAddToSceneNoModify(0);
          }
        }
      }
    }

  // update all nodes in the scene

  //this->Scene->UpdateNodeReferences(this->Nodes);

  for (sceneNodes->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    if (this->IncludeNodeInSceneView(node) && node->GetSaveWithScene())
      {
      node->UpdateScene(this->Scene);
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
  for (sceneNodes->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    assert(node->GetScene() == this->Scene);
    }
#endif
}

//----------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLSceneViewNode::GetStoredScene()
{
  return this->SnapshotScene;
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::SetAbsentStorageFileNames()
{
  if (this->Scene == nullptr)
    {
    return;
    }

  if (this->SnapshotScene == nullptr)
    {
    return;
    }

  // TBD: determine if storage nodes in the all scene views need unique file names
  // in order to support reading into scene view nodes on xml read.
  unsigned int numNodesInSceneView = this->SnapshotScene->GetNodes()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = nullptr;

  for (n=0; n<numNodesInSceneView; n++)
    {
    node  = vtkMRMLNode::SafeDownCast(this->SnapshotScene->GetNodes()->GetItemAsObject(n));
    if (node)
      {
      // for storage nodes replace full path with relative
      vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(node);
      if (snode)
        {
        vtkMRMLNode *node1 = this->Scene->GetNodeByID(snode->GetID());
        if (node1)
          {
          vtkMRMLStorageNode *snode1 = vtkMRMLStorageNode::SafeDownCast(node1);
          if (snode1)
            {
            snode->SetFileName(snode1->GetFileName());
            int numberOfFileNames = snode1->GetNumberOfFileNames();
            if (numberOfFileNames > 0)
              {
              snode->ResetFileNameList();
              for (int i = 0; i < numberOfFileNames; ++i)
                {
                snode->AddFileName(snode1->GetNthFileName(i));
                }
              }
            }
          }
        }
      } //if (node)
    } //for (n=0; n<numNodesInSceneView; n++)
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLSceneViewNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLSceneViewStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::SetSceneViewDescription(const vtkStdString& newDescription)
{
  if (this->SceneViewDescription == newDescription)
    {
    return;
    }
  this->SceneViewDescription = newDescription;
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::SetScreenShot(vtkImageData* newScreenShot)
{
  this->StorableModifiedTime.Modified();
  //vtkSetObjectBodyMacro(ScreenShot, vtkImageData, newScreenShot);
  if (!newScreenShot)
    {
    if (this->ScreenShot)
      {
      this->ScreenShot->Delete();
      }
    this->ScreenShot = nullptr;
    }
  else
    {
    if (!this->ScreenShot)
      {
      this->ScreenShot = vtkImageData::New();
      }
    this->ScreenShot->DeepCopy(newScreenShot);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSceneViewNode::SetScreenShotType(int newScreenShotType)
{
  if (this->ScreenShotType == newScreenShotType)
    {
    return;
    }
  this->ScreenShotType = newScreenShotType;
  this->StorableModifiedTime.Modified();
  this->Modified();
}
//----------------------------------------------------------------------------
int vtkMRMLSceneViewNode::GetNodesByClass(const char *className, std::vector<vtkMRMLNode *> &nodes)
{
  if (!this->SnapshotScene)
    {
    return 0;
    }
  return this->SnapshotScene->GetNodesByClass(className, nodes);
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLSceneViewNode::GetNodesByClass(const char *className)
{
  if (!this->SnapshotScene)
    {
    return nullptr;
    }
  return this->SnapshotScene->GetNodesByClass(className);
}

//----------------------------------------------------------------------------
bool vtkMRMLSceneViewNode::IncludeNodeInSceneView(vtkMRMLNode *node)
{
  if (!node)
    {
    return false;
    }
  bool includeInView = true;
  // check for simple node types
  // Storable nodes cannot be restored from scene views, as they would require
  // storage nodes to load content, therefore we do not include them in scene views
  // (except camera, which is a storable node but actually does not require a
  // storage node and it is important to save in scene views).
  if (node->IsA("vtkMRMLSceneViewNode") ||
      node->IsA("vtkMRMLSceneViewStorageNode") ||
      node->IsA("vtkMRMLSnapshotClipNode") ||
      (node->IsA("vtkMRMLStorableNode") && !node->IsA("vtkMRMLCameraNode")) )
    {
    includeInView = false;
    }

  // check for scene view hierarchy nodes
  else if (node->IsA("vtkMRMLHierarchyNode"))
    {
    // check for tagged scene view hierarchy nodes
    if (node->GetAttribute("SceneViewHierarchy") != nullptr)
      {
      includeInView = false;
      }
    // is it a top level singleton node?
    else if (node->GetID() && !strncmp(node->GetID(), "vtkMRMLHierarchyNodeSceneViewTopLevel", 37))
      {
      includeInView = false;
      }
    // backward compatibility: is it an old top level node?
    else if (node->GetName() && !strncmp(node->GetName(), "SceneViewToplevel", 17))
      {
      includeInView = false;
      }
    else
      {
      vtkSmartPointer<vtkMRMLHierarchyNode> hnode = vtkMRMLHierarchyNode::SafeDownCast(node);
      if (hnode)
        {
        // backward compatibility: non tagged hierarchy nodes
        // check for hierarchy nodes that point to scene view nodes
        if (hnode->GetAssociatedNode() && hnode->GetAssociatedNode()->IsA("vtkMRMLSceneViewNode"))
          {
          includeInView = false;
          }
        // getting the node might fail if it's an erroneously included hierarchy
        // node in a scene view node being restored
        else if (hnode->GetAssociatedNodeID() && !strncmp(hnode->GetAssociatedNodeID(), "vtkMRMLSceneViewNode", 20))
          {
          includeInView = false;
          }
        }
      }
    }

  vtkDebugMacro("IncludeNodeInSceneView: node " << node->GetID() << " includeInView = " << includeInView);

  return includeInView;
}

void vtkMRMLSceneViewNode::SetSceneViewRootDir( const char* name)
{
  if (!this->SnapshotScene)
    {
    return;
    }
  this->SnapshotScene->SetRootDirectory(name);
}
