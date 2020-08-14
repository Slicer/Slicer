/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// Slicer logic includes
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkMRMLSliceLogic.h"

/// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLClipModelsNode.h>
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
vtkCxxSetObjectMacro(vtkSlicerModelsLogic, ColorLogic, vtkMRMLColorLogic);

//----------------------------------------------------------------------------
vtkSlicerModelsLogic::vtkSlicerModelsLogic()
{
  this->ColorLogic = nullptr;
}

//----------------------------------------------------------------------------
vtkSlicerModelsLogic::~vtkSlicerModelsLogic()
{
  if (this->ColorLogic != nullptr)
    {
    this->ColorLogic->Delete();
    this->ColorLogic = nullptr;
    }
}

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

  vtkNew<vtkMRMLModelDisplayNode> display;
  this->GetMRMLScene()->AddNode(display.GetPointer());

  vtkNew<vtkMRMLModelNode> model;
  model->SetAndObservePolyData(polyData);
  model->SetAndObserveDisplayNodeID(display->GetID());
  this->GetMRMLScene()->AddNode(model.GetPointer());

  return model.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerModelsLogic::AddModel(vtkAlgorithmOutput* polyData)
{
  if (this->GetMRMLScene() == nullptr)
    {
    return nullptr;
    }

  vtkNew<vtkMRMLModelDisplayNode> display;
  this->GetMRMLScene()->AddNode(display.GetPointer());

  vtkNew<vtkMRMLModelNode> model;
  model->SetPolyDataConnection(polyData);
  model->SetAndObserveDisplayNodeID(display->GetID());
  this->GetMRMLScene()->AddNode(model.GetPointer());

  return model.GetPointer();
}

//----------------------------------------------------------------------------
int vtkSlicerModelsLogic::AddModels (const char* dirname, const char* suffix,
  int coordinateSystem /*=vtkMRMLStorageNode::CoordinateSystemLPS*/)
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
        if (this->AddModel(fullPath.c_str(), coordinateSystem) == nullptr)
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
  int coordinateSystem /*=vtkMRMLStorageNode::CoordinateSystemLPS*/)
{
  if (this->GetMRMLScene() == nullptr ||
      filename == nullptr)
    {
    return nullptr;
    }
  vtkNew<vtkMRMLModelNode> modelNode;
  vtkNew<vtkMRMLModelDisplayNode> displayNode;
  vtkNew<vtkMRMLModelStorageNode> mStorageNode;
  vtkSmartPointer<vtkMRMLStorageNode> storageNode;

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != nullptr)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    vtkDebugMacro("AddModel: file name is remote: " << filename);
    }
  const char *localFile=nullptr;
  if (useURI)
    {
    mStorageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    mStorageNode->SetFileName(filename);
    localFile = filename;
    }
  const std::string fname(localFile?localFile:"");
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  std::string name = itksys::SystemTools::GetFilenameName(fname);
  vtkDebugMacro("AddModel: got model name = " << name.c_str());

  // check to see which node can read this type of file
  if (mStorageNode->SupportedFileType(name.c_str()))
    {
    storageNode = mStorageNode.GetPointer();
    mStorageNode->SetCoordinateSystem(coordinateSystem);
    }

  /* don't read just yet, need to add to the scene first for remote reading
  if (mStorageNode->ReadData(modelNode) != 0)
    {
    storageNode = mStorageNode;
    }
  */
  if (storageNode != nullptr)
    {
    std::string baseName = storageNode->GetFileNameWithoutExtension(fname.c_str());
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(baseName.c_str()));
    modelNode->SetName(uname.c_str());

    this->GetMRMLScene()->AddNode(storageNode.GetPointer());
    this->GetMRMLScene()->AddNode(displayNode.GetPointer());

    // Set the scene so that SetAndObserve[Display|Storage]NodeID can find the
    // node in the scene (so that DisplayNodes return something not empty)
    modelNode->SetScene(this->GetMRMLScene());
    modelNode->SetAndObserveStorageNodeID(storageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    this->GetMRMLScene()->AddNode(modelNode.GetPointer());

    // now set up the reading
    vtkDebugMacro("AddModel: calling read on the storage node");
    int retval = storageNode->ReadData(modelNode.GetPointer());
    if (retval != 1)
      {
      vtkErrorMacro("AddModel: error reading " << filename);
      this->GetMRMLScene()->RemoveNode(modelNode.GetPointer());
      return nullptr;
      }
    }
  else
    {
    vtkErrorMacro("Couldn't read file: " << filename);
    return nullptr;
    }

  return modelNode.GetPointer();
}

//----------------------------------------------------------------------------
int vtkSlicerModelsLogic::SaveModel (const char* filename, vtkMRMLModelNode *modelNode,
  int coordinateSystem/*=-1*/)
{
   if (modelNode == nullptr || filename == nullptr)
    {
    vtkErrorMacro("SaveModel: unable to proceed, filename is " <<
                  (filename == nullptr ? "null" : filename) <<
                  ", model node is " <<
                  (modelNode == nullptr ? "null" : modelNode->GetID()));
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
    storageNode = vtkMRMLModelStorageNode::New();
    storageNode->SetScene(this->GetMRMLScene());
    this->GetMRMLScene()->AddNode(storageNode);
    modelNode->SetAndObserveStorageNodeID(storageNode->GetID());
    storageNode->Delete();
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

  int res = storageNode->WriteData(modelNode);

  return res;
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerModelsLogic:             " << this->GetClassName() << "\n";
  if (this->ColorLogic)
    {
    os << indent << "ColorLogic: ";
    this->ColorLogic->PrintSelf(os, indent);
    }
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
        if (displayNode)
          {
          displayNode->SetVisibility(flag);
          }
        }
      }
    }
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}
