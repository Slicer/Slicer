/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// Slicer logic includes
#include "vtkSlicerModelsLogic.h"
#include "vtkMRMLSliceLogic.h"

/// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLClipModelsNode.h>
#include "vtkMRMLMessageCollection.h"
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelStorageNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTransformNode.h>

/// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkGeneralTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkTagTable.h>

/// ITK includes
#include <itksys/Directory.hxx>
#include <itksys/SystemTools.hxx>

/// STD includes
#include <cassert>

vtkStandardNewMacro(vtkSlicerModelsLogic);

//----------------------------------------------------------------------------
vtkSlicerModelsLogic::vtkSlicerModelsLogic()=default;

//----------------------------------------------------------------------------
vtkSlicerModelsLogic::~vtkSlicerModelsLogic()=default;

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::ObserveMRMLScene()
{
  if (this->GetMRMLScene() &&
    this->GetMRMLScene()->GetFirstNodeByClass("vtkMRMLClipModelsNode") == nullptr)
    {
    // vtkMRMLClipModelsNode is a singleton
    this->GetMRMLScene()->AddNode(vtkSmartPointer<vtkMRMLClipModelsNode>::New());
    }
  this->Superclass::ObserveMRMLScene();
}

//-----------------------------------------------------------------------------
void vtkSlicerModelsLogic::OnMRMLSceneEndImport()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("OnMRMLSceneEndImport: Unable to access MRML scene");
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
  if (!shNode)
    {
    vtkErrorMacro("OnMRMLSceneEndImport: Unable to access subject hierarchy node");
    return;
    }

  // Convert model hierarchy nodes into subject hierarchy folders
  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator mhIt;
  vtkCollection* mhNodes = scene->GetNodesByClass("vtkMRMLModelHierarchyNode");
  std::string newFolderName = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewItemNamePrefix()
    + vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder();
  std::map<std::string, vtkIdType> mhNodeIdToShItemIdMap;
  std::map<std::string, std::string> mhNodeIdToParentNodeIdMap;
  for (mhNodes->InitTraversal(mhIt); (node = (vtkMRMLNode*)mhNodes->GetNextItemAsObject(mhIt)) ;)
    {
    // Get direct child hierarchy nodes
    vtkMRMLModelHierarchyNode* mhNode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
    std::vector<vtkMRMLHierarchyNode*> childHierarchyNodes = mhNode->GetChildrenNodes();

    vtkIdType folderItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (childHierarchyNodes.size() > 0)
      {
      // Create new folder for the model hierarchy if there are children
      // (otherwise it's a leaf node with an associated model).
      // Have it directly under the scene for now. Rebuild hierarchy later
      // when we have all the items created.
      folderItemID = shNode->CreateFolderItem(shNode->GetSceneItemID(),
        (mhNode->GetName() ? mhNode->GetName() : shNode->GenerateUniqueItemName(newFolderName)) );
      }
    else if (!mhNode->GetAssociatedNodeID())
      {
      // If there are no children but there is no associated node, then something is wrong
      vtkWarningMacro("OnMRMLSceneEndImport: Invalid model hierarchy node found with neither "
        << "children nor associated node: " << mhNode->GetID());
      continue;
      }

    // Remember subject hierarchy item for current model hierarchy node
    // (even if has no actual folder, as this map will be used to remove the hierarchy nodes)
    mhNodeIdToShItemIdMap[mhNode->GetID()] = folderItemID;

    // Remember parent for current model hierarchy node if not leaf
    // (i.e. has a corresponding folder item and so need to be reparented in subject hierarchy)
    if (mhNode->GetParentNodeID() && !mhNode->GetAssociatedNodeID())
      {
      mhNodeIdToParentNodeIdMap[mhNode->GetID()] = mhNode->GetParentNodeID();
      }

    // Move all the direct children of the model hierarchy node under the folder if one was created
    if (folderItemID)
      {
      for (std::vector<vtkMRMLHierarchyNode*>::iterator it = childHierarchyNodes.begin();
        it != childHierarchyNodes.end(); ++it)
        {
        vtkMRMLNode* associatedNode = (*it)->GetAssociatedNode();
        if (associatedNode)
          {
          vtkIdType associatedItemID = shNode->GetItemByDataNode(associatedNode);
          if (associatedItemID)
            {
            shNode->SetItemParent(associatedItemID, folderItemID, true);
            }
          }
        }
      // Request plugin search for the folder that triggers creation of a model display node
      shNode->RequestOwnerPluginSearch(folderItemID);
      }
    } // for all model hierarchy nodes
  mhNodes->Delete();

  // Set up hierarchy between the created folder items
  for (std::map<std::string, std::string>::iterator it = mhNodeIdToParentNodeIdMap.begin();
    it != mhNodeIdToParentNodeIdMap.end(); ++it)
    {
    // Get SH item IDs for the nodes
    vtkIdType currentItemID = mhNodeIdToShItemIdMap[it->first];
    vtkIdType parentItemID = mhNodeIdToShItemIdMap[it->second];

    // Set parent in subject hierarchy
    shNode->SetItemParent(currentItemID, parentItemID, true);
    }

  // Remove model hierarchy nodes from the scene
  for (std::map<std::string, vtkIdType>::iterator it = mhNodeIdToShItemIdMap.begin();
    it != mhNodeIdToShItemIdMap.end(); ++it)
    {
    scene->RemoveNode(scene->GetNodeByID(it->first));
    }
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerModelsLogic::AddModel(vtkPolyData* polyData)
{
  if (this->GetMRMLScene() == nullptr)
    {
    return nullptr;
    }
  vtkMRMLModelNode* model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLModelNode"));
  if (!model)
    {
    return nullptr;
    }
  model->SetAndObservePolyData(polyData);
  model->CreateDefaultDisplayNodes();
  return model;
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerModelsLogic::AddModel(vtkAlgorithmOutput* polyData)
{
  if (this->GetMRMLScene() == nullptr)
    {
    return nullptr;
    }
  vtkMRMLModelNode* model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLModelNode"));
  if (!model)
    {
    return nullptr;
    }
  model->SetPolyDataConnection(polyData);
  model->CreateDefaultDisplayNodes();
  return model;
}

//----------------------------------------------------------------------------
int vtkSlicerModelsLogic::AddModels (const char* dirname, const char* suffix,
  int coordinateSystem /*=vtkMRMLStorageNode::CoordinateSystemLPS*/,
  vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  std::string ssuf = suffix;
  itksys::Directory dir;
  dir.Load(dirname);

  int nfiles = dir.GetNumberOfFiles();
  int res = 1;
  for (int i=0; i<nfiles; i++) {
    const char* filename = dir.GetFile(i);
    std::string sname = filename;
    if (!itksys::SystemTools::FileIsDirectory(filename))
      {
      if ( sname.find(ssuf) != std::string::npos )
        {
        std::string fullPath = std::string(dir.GetPath())
            + "/" + filename;
        if (this->AddModel(fullPath.c_str(), coordinateSystem, userMessages) == nullptr)
          {
          res = 0;
          }
        }
      }
  }
  return res;
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerModelsLogic::AddModel(const char* filename,
  int coordinateSystem/*=vtkMRMLStorageNode::CoordinateSystemLPS*/,
  vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  if (this->GetMRMLScene() == nullptr || filename == nullptr)
    {
    vtkErrorToMessageCollectionMacro(userMessages, "vtkSlicerModelsLogic::AddModel",
      "Invalid scene or filename");
    return nullptr;
    }

  // Determine local filename
  vtkNew<vtkMRMLModelStorageNode> storageNode;
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != nullptr)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    vtkDebugMacro("AddModel: file name is remote: " << filename);
    }
  std::string localFile;
  if (useURI)
    {
    storageNode->SetURI(filename);
    // reset filename to the local file name
    const char* localFilePtr = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(filename);
    if (localFilePtr)
      {
      localFile = localFilePtr;
      }
    }
  else
    {
    storageNode->SetFileName(filename);
    localFile = filename;
    }

  // Check if we can read this type of file.
  // The model name is based on the file name (itksys call should work even if
  // file is not on disk yet).
  std::string name = itksys::SystemTools::GetFilenameName(localFile);
  vtkDebugMacro("AddModel: got model name = " << name.c_str());
  if (!storageNode->SupportedFileType(name.c_str()))
    {
    vtkErrorToMessageCollectionMacro(userMessages, "vtkSlicerModelsLogic::AddModel",
      "Could not find a suitable storage node for file '" << filename << "'.");
    return nullptr;
    }

  // Create model node
  std::string baseName = storageNode->GetFileNameWithoutExtension(localFile.c_str());
  std::string uniqueName(this->GetMRMLScene()->GetUniqueNameByString(baseName.c_str()));
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLModelNode", uniqueName.c_str()));
  if (!modelNode)
    {
    return nullptr;
    }

  // Read the model file
  vtkDebugMacro("AddModel: calling read on the storage node");
  storageNode->SetCoordinateSystem(coordinateSystem);
  int success = storageNode->ReadData(modelNode);
  if (!success)
    {
    vtkErrorMacro("AddModel: error reading " << filename);
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    this->GetMRMLScene()->RemoveNode(modelNode);
    return nullptr;
    }

  // Associate with storage node
  this->GetMRMLScene()->AddNode(storageNode);
  modelNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // Add display node
  modelNode->CreateDefaultDisplayNodes();

  return modelNode;
}

//----------------------------------------------------------------------------
int vtkSlicerModelsLogic::SaveModel (const char* filename, vtkMRMLModelNode *modelNode,
  int coordinateSystem/*=-1*/, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
   if (modelNode == nullptr || filename == nullptr)
     {
     vtkErrorToMessageCollectionMacro(userMessages, "vtkSlicerModelsLogic::SaveModel",
       "Failed to save model node " << ((modelNode && modelNode->GetID()) ? modelNode->GetID() : "(null)")
       << " into file '" << (filename ? filename : "(null)") << "'.");
     return 0;
     }

  vtkMRMLModelStorageNode *storageNode = nullptr;
  vtkMRMLStorageNode *snode = modelNode->GetStorageNode();
  if (snode != nullptr)
    {
    storageNode = vtkMRMLModelStorageNode::SafeDownCast(snode);
    }
  if (storageNode == nullptr)
    {
    storageNode = vtkMRMLModelStorageNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLModelStorageNode"));
    modelNode->SetAndObserveStorageNodeID(storageNode->GetID());
    }

  if (coordinateSystem >= 0)
    {
    storageNode->SetCoordinateSystem(coordinateSystem);
    }

  // check for a remote file
  if ((this->GetMRMLScene()->GetCacheManager() != nullptr) &&
      this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename))
    {
    storageNode->SetURI(filename);
    }
  else
    {
    storageNode->SetFileName(filename);
    }

  int success = storageNode->WriteData(modelNode);
  if (!success)
    {
    vtkErrorMacro("vtkSlicerModelsLogic::SaveModel: error saving " << filename);
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    }
  return success;
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerModelsLogic:             " << this->GetClassName() << "\n";
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::TransformModel(vtkMRMLTransformNode *tnode,
                                          vtkMRMLModelNode *modelNode,
                                          int transformNormals,
                                          vtkMRMLModelNode *modelOut)
{
  if (!modelNode || !modelOut || !tnode)
    {
    return;
    }

  vtkNew<vtkPolyData> poly;
  modelOut->SetAndObservePolyData(poly.GetPointer());

  poly->DeepCopy(modelNode->GetPolyData());

  vtkMRMLTransformNode *mtnode = modelNode->GetParentTransformNode();

  vtkAbstractTransform *transform = tnode->GetTransformToParent();
  modelOut->ApplyTransform(transform);

  if (transformNormals)
    {
    // fix normals
    //--- NOTE: This filter recomputes normals for polygons and
    //--- triangle strips only. Normals are not computed for lines or vertices.
    //--- Triangle strips are broken up into triangle polygons.
    //--- Polygons are not automatically re-stripped.
    vtkNew<vtkPolyDataNormals> normals;
    normals->SetInputData(poly.GetPointer());
    //--- NOTE: This assumes a completely closed surface
    //---(i.e. no boundary edges) and no non-manifold edges.
    //--- If these constraints do not hold, the AutoOrientNormals
    //--- is not guaranteed to work.
    normals->AutoOrientNormalsOn();
    //--- Flipping modifies both the normal direction
    //--- and the order of a cell's points.
    normals->FlipNormalsOn();
    normals->SplittingOff();
    //--- enforce consistent polygon ordering.
    normals->ConsistencyOn();

    normals->Update();
    modelOut->SetPolyDataConnection(normals->GetOutputPort());
   }

  modelOut->SetAndObserveTransformNodeID(mtnode == nullptr ? nullptr : mtnode->GetID());

  return;
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::SetAllModelsVisibility(int flag)
{
  if (this->GetMRMLScene() == nullptr)
    {
    return;
    }

  int numModels = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");

  // go into batch processing mode
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);
  for (int i = 0; i < numModels; i++)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLModelNode");
    // Exclude volume slice model nodes.
    // Exclude vtkMRMLModelNode subclasses by comparing classname.
    // Doing so will avoid updating annotation and fiber bundle node
    // visibility since they derive from vtkMRMLModelNode
    // See https://github.com/Slicer/Slicer/issues/2576
    if (mrmlNode != nullptr
        && !vtkMRMLSliceLogic::IsSliceModelNode(mrmlNode)
        && strcmp(mrmlNode->GetClassName(), "vtkMRMLModelNode") == 0)
      {
      vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(mrmlNode);
      if (modelNode)
        {
        // have a "real" model node, set the display visibility
        modelNode->SetDisplayVisibility(flag);
        }
      }

    if (flag != 2 && mrmlNode != nullptr
        && !vtkMRMLSliceLogic::IsSliceModelNode(mrmlNode) )
      {
      vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(mrmlNode);
      int ndnodes = modelNode->GetNumberOfDisplayNodes();
      for (int i=0; i<ndnodes; i++)
        {
        vtkMRMLDisplayNode *displayNode = modelNode->GetNthDisplayNode(i);
        if (displayNode && displayNode->IsShowModeDefault())
          {
          displayNode->SetVisibility(flag);
          }
        }
      }
    }
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}
