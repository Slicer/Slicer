/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
///  Helper file that contains definition of requests that are sent from
/// various threads and executed on the main thread.

#ifndef __vtkSlicerApplicationLogicRequests_h
#define __vtkSlicerApplicationLogicRequests_h

#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTableNode.h>

//----------------------------------------------------------------------------
class DataRequest
{
public:
  DataRequest()
  {
    m_UID = 0;
  }

  DataRequest(int uid)
  {
    m_UID = uid;
  }

  virtual ~DataRequest()  = default;

  virtual void Execute(vtkSlicerApplicationLogic*) {};

  int GetUID()const{return m_UID;}

protected:
  vtkMTimeType m_UID;
};

//----------------------------------------------------------------------------
class ReadDataRequestFile : public DataRequest
{
public:
  ReadDataRequestFile(const std::string& node, const std::string& filename,
    int displayData, int deleteFile, int uid = 0)
    : DataRequest(uid)
  {
    m_TargetNode = node;
    m_Filename = filename;
    m_DisplayData = displayData;
    m_DeleteFile = deleteFile;
  }

  void Execute(vtkSlicerApplicationLogic* appLogic) override
  {
    // This method needs to read the data into the specific type of node and set up an
    // appropriate storage and display node.

    vtkMRMLNode *nd = appLogic->GetMRMLScene()->GetNodeByID(m_TargetNode.c_str());
    vtkDebugWithObjectMacro(appLogic, "ProcessReadNodeData: read data request node id = " << nd->GetID());

    vtkSmartPointer<vtkMRMLStorageNode> storageNode;
#ifdef Slicer_BUILD_CLI_SUPPORT
    vtkMRMLCommandLineModuleNode *clp = vtkMRMLCommandLineModuleNode::SafeDownCast(nd);
#endif

    bool useURI = appLogic->GetMRMLScene()->GetCacheManager()->IsRemoteReference(m_Filename.c_str());

    vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(nd);
    if (storableNode)
      {
      int numStorageNodes = storableNode->GetNumberOfStorageNodes();
      for (int n = 0; n < numStorageNodes; n++)
        {
        vtkMRMLStorageNode *testStorageNode = storableNode->GetNthStorageNode(n);
        if (testStorageNode)
          {
          if (useURI && testStorageNode->GetURI() != nullptr)
            {
            if (m_Filename.compare(testStorageNode->GetURI()) == 0)
              {
              // found a storage node for the remote file
              vtkDebugWithObjectMacro(appLogic, "ProcessReadNodeData: found a storage node with the right URI: " << testStorageNode->GetURI());
              storageNode = testStorageNode;
              break;
              }
            }
          else if (testStorageNode->GetFileName() != nullptr &&
            m_Filename.compare(testStorageNode->GetFileName()) == 0)
            {
            // found the right storage node for a local file
            vtkDebugWithObjectMacro(appLogic, "ProcessReadNodeData: found a storage node with the right filename: " << testStorageNode->GetFileName());
            storageNode = testStorageNode;
            break;
            }
          }
        }

      // if there wasn't already a matching storage node on the node, make one
      bool createdNewStorageNode = false;
      if (storageNode.GetPointer() == nullptr)
        {
        // Read the data into the referenced node
        if (itksys::SystemTools::FileExists(m_Filename.c_str()))
          {
          // file is there on disk
          storableNode->AddDefaultStorageNode(m_Filename.c_str());
          storageNode = storableNode->GetStorageNode();
          createdNewStorageNode = (storageNode != nullptr);
          }
        }

      // Have the storage node read the data into the current node
      if (storageNode.GetPointer() != nullptr)
        {
        try
          {
          vtkDebugWithObjectMacro(appLogic, "ProcessReadNodeData: about to call read data, " \
            "storage node's read state is " << storageNode->GetReadStateAsString());
          if (useURI)
            {
            storageNode->SetURI(m_Filename.c_str());
            vtkDebugWithObjectMacro(appLogic, "ProcessReadNodeData: calling ReadData on the storage node " \
              << storageNode->GetID() << ", uri = " << storageNode->GetURI());
            storageNode->ReadData(nd, /*temporary*/true);
            if (createdNewStorageNode)
              {
              storageNode->SetURI(nullptr); // clear temporary URI
              }
            }
          else
            {
            storageNode->SetFileName(m_Filename.c_str());
            vtkDebugWithObjectMacro(appLogic, "ProcessReadNodeData: calling ReadData on the storage node " \
              << storageNode->GetID() << ", filename = " << storageNode->GetFileName());
            storageNode->ReadData(nd, /*temporary*/true);
            if (createdNewStorageNode)
              {
              storageNode->SetFileName(nullptr); // clear temp file name
              }
            }
          }
        catch (itk::ExceptionObject& exc)
          {
          vtkErrorWithObjectMacro(appLogic, "Exception while reading " << m_Filename << ", " << exc);
          }
        catch (...)
          {
          vtkErrorWithObjectMacro(appLogic, "Unknown exception while reading " << m_Filename);
          }
        }
      }
#ifdef Slicer_BUILD_CLI_SUPPORT
    // if the node was a CommandLineModule node, then read the file
    // (no storage node for these, yet)
    if (clp)
      {
      clp->ReadParameterFile(m_Filename);
      }
#endif

    // Delete the file if requested
    if (m_DeleteFile)
     {
      int removed;
      // is it a shared memory location?
      if (m_Filename.find("slicer:") != std::string::npos)
        {
        removed = 1;
        }
      else
        {
        removed = itksys::SystemTools::RemoveFile(m_Filename.c_str());
        }
      if (!removed)
        {
        vtkGenericWarningMacro("Unable to delete temporary file " << m_Filename);
        }
      }


    // Get the right type of display node. Only create a display node
    // if one does not exist already
    //
    vtkMRMLDisplayableNode *displayableNode =
      vtkMRMLDisplayableNode::SafeDownCast(nd);
    if (displayableNode)
      {
      // Create a default display node if no display node exists for the node yet.
      displayableNode->CreateDefaultDisplayNodes();
      }

    // Cause the any observers to fire (we may have avoided calling
    // modified on the node)
    //
    nd->Modified();

    // If scalar volume, set the volume as the active volume and
    // propagate selection.
    //
    // Models are always displayed when loaded above.
    //
    // Tensors? Vectors?
    if (m_DisplayData)
      {
      if (vtkMRMLLabelMapVolumeNode::SafeDownCast(nd) != nullptr)
        {
        appLogic->GetSelectionNode()->SetActiveLabelVolumeID(m_TargetNode.c_str());
        appLogic->PropagateVolumeSelection();
        }
      else if (vtkMRMLScalarVolumeNode::SafeDownCast(nd) != nullptr)
        {
        appLogic->GetSelectionNode()->SetActiveVolumeID(m_TargetNode.c_str());
        // make sure win/level gets calculated
        vtkMRMLDisplayNode* displayNode = vtkMRMLScalarVolumeNode::SafeDownCast(nd)->GetDisplayNode();
        if (displayNode)
          {
          displayNode->Modified();
          }
        appLogic->PropagateVolumeSelection();
        }
      else if (vtkMRMLTableNode::SafeDownCast(nd) != nullptr)
        {
        appLogic->GetSelectionNode()->SetActiveTableID(m_TargetNode.c_str());
        appLogic->PropagateTableSelection();
        }
      }
  }

protected:
  std::string m_TargetNode;
  std::string m_Filename;
  int m_DisplayData;
  int m_DeleteFile;
};

//----------------------------------------------------------------------------
class ReadDataRequestScene : public DataRequest
{
public:
  ReadDataRequestScene(const std::vector<std::string>& targetNodes,
    const std::vector<std::string>& sourceNodes,
    const std::string& filename,
    int displayData, int deleteFile,
    int uid = 0)
    : DataRequest(uid)
  {
    m_TargetNodes = targetNodes;
    m_SourceNodes = sourceNodes;
    m_Filename = filename;
    m_DisplayData = displayData;
    m_DeleteFile = deleteFile;
  }

  void Execute(vtkSlicerApplicationLogic* appLogic) override
  {
    if (m_SourceNodes.size() != m_TargetNodes.size())
      {
      // Can't do ID remapping if the two node lists are different
      // sizes. Just import the scene. (This is where we would put to
      // the code to load into a node hierarchy (with a corresponding
      // change in the conditional above)).
      appLogic->GetMRMLScene()->SetURL(m_Filename.c_str());
      appLogic->GetMRMLScene()->Import();

      // Delete the file if requested
      if (m_DeleteFile)
        {
        int removed;
        removed = itksys::SystemTools::RemoveFile(m_Filename.c_str());
        if (!removed)
          {
          std::stringstream information;
          information << "Unable to delete temporary file "
                      << m_Filename << std::endl;
          vtkGenericWarningMacro( << information.str().c_str() );
          }
        }

      return;
      }

    vtkNew<vtkMRMLScene> miniscene;
    miniscene->SetURL(m_Filename.c_str() );
    miniscene->Import();

    // iterate over the list of nodes specified to read
    std::vector<std::string>::const_iterator tit;
    std::vector<std::string>::const_iterator sit;

    tit = m_TargetNodes.begin();
    sit = m_SourceNodes.begin();

    appLogic->GetMRMLScene()->StartState(vtkMRMLScene::ImportState);

    while (sit != m_SourceNodes.end())
      {
      vtkMRMLNode *source = miniscene->GetNodeByID((*sit).c_str());
      vtkMRMLNode *target = appLogic->GetMRMLScene()->GetNodeByID( (*tit).c_str() );

      if (source && target)
        {
        // save old storage info (in case user has custom file name already
        // defined for this node, don't use the one from the miniscene since it
        // was only used to read/write the temp area).
        vtkMRMLStorableNode *storableTarget = vtkMRMLStorableNode::SafeDownCast(target);
        if ( storableTarget )
          {
          const char *oldStorageNodeID = storableTarget->GetStorageNodeID();
          target->Copy(source);
          storableTarget->SetAndObserveStorageNodeID(oldStorageNodeID);
          }
        else
          {
          target->Copy(source);
          }

        // if the source node is a model hierarchy node, then also copy
        // and remap any child nodes of the target that are not in the
        // target list (nodes that had no source equivalent before the
        // module ran).
        vtkMRMLModelHierarchyNode *smhnd
          = vtkMRMLModelHierarchyNode::SafeDownCast(source);
        vtkMRMLModelHierarchyNode *tmhnd
          = vtkMRMLModelHierarchyNode::SafeDownCast(target);
        if (smhnd && tmhnd)
          {
          // get the model node and display node BEFORE we add nodes to
          // the target scene
          vtkMRMLModelNode *smnd = smhnd->GetModelNode();
          vtkMRMLDisplayNode *sdnd = smhnd->GetDisplayNode();

          // add the model and display referenced by source model hierarchy node
          if (smnd)
            {
            // set the model node to be modified, as it was read from a temp
            // location
            //smnd->SetModifiedSinceRead(1);
            // get display node BEFORE we add nodes to the target scene
            vtkMRMLDisplayNode *sdnd1 = smnd->GetDisplayNode();

            vtkMRMLNode *tmodel = appLogic->GetMRMLScene()->CopyNode(smnd);
            vtkMRMLStorableNode::SafeDownCast(tmodel)->SetAndObserveStorageNodeID(nullptr);
            vtkMRMLModelNode *mnd = vtkMRMLModelNode::SafeDownCast( tmodel );
            tmhnd->SetModelNodeID( mnd->GetID() );

            if (sdnd1)
              {
              vtkMRMLNode *tdnd = appLogic->GetMRMLScene()->CopyNode(sdnd1);
              mnd->SetAndObserveDisplayNodeID( tdnd->GetID() );
              }
            }

          if (sdnd)
            {
            vtkMRMLNode *dnd = appLogic->GetMRMLScene()->CopyNode(sdnd);
            tmhnd->SetAndObserveDisplayNodeID( dnd->GetID() );
            }

          // add any children model hierarchy nodes, rinse, repeat
          //
          // keep a map of model hierarchy node ids so that can update the parent node references
          std::map<std::string, std::string> parentNodeIDMapper;
          // hopefully the parents will have been read first, but if not
          // keep a list of model hierarchy nodes that failed to have their parent node reference remapped
          std::vector<vtkMRMLModelHierarchyNode *> childNodesThatNeedParentsIDsRemapped;
          for (int n=0;
               n<miniscene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
               n++)
            {
            vtkMRMLModelHierarchyNode * mhnd = vtkMRMLModelHierarchyNode
              ::SafeDownCast(miniscene->GetNthNodeByClass(n,
                                                  "vtkMRMLModelHierarchyNode"));
            if (mhnd)
              {
              // is this model hierarchy node in our source list
              // already? if so skip it
              std::vector<std::string>::const_iterator ssit
                = std::find(m_SourceNodes.begin(),
                            m_SourceNodes.end(), mhnd->GetID());
              if (ssit == m_SourceNodes.end())
                {
                // not in source list, so we may need to add it,
                // if it's a child, grandchild etc of the top level node that we're importing
                if (strcmp(mhnd->GetTopParentNode()->GetID(), smhnd->GetID()) == 0)
                  {
                  // get the model and display node BEFORE we add nodes
                  // to the target scene
                  vtkMRMLModelNode *smnd1 = mhnd->GetModelNode();
                  vtkMRMLDisplayNode *sdnd1 = mhnd->GetDisplayNode();

                  vtkMRMLNode *tchild = appLogic->GetMRMLScene()->CopyNode(mhnd);
                  // keep track of any node id change in case other nodes use this as a parent
                  parentNodeIDMapper[std::string(mhnd->GetID())] = std::string(tchild->GetID());
                  vtkMRMLModelHierarchyNode *tcmhd
                    = vtkMRMLModelHierarchyNode::SafeDownCast( tchild );
                  // check for a parent node id in the mapper (as long as it doesn't already
                  // point to the source node), default to the top level one though
                  std::string parentNodeID = std::string(tmhnd->GetID());
                  if (tcmhd->GetParentNodeID() != nullptr &&
                      strcmp(tcmhd->GetParentNodeID(),smhnd->GetID()) != 0)
                    {
                    std::map<std::string,std::string>::iterator pIt = parentNodeIDMapper.find(std::string(tcmhd->GetParentNodeID()));
                    if (pIt != parentNodeIDMapper.end())
                      {
                      parentNodeID = pIt->second;
                      vtkDebugWithObjectMacro(appLogic, "Remapped parent node id to " << parentNodeID.c_str());
                      }
                    else
                      {
                      childNodesThatNeedParentsIDsRemapped.push_back(tcmhd);
                      }
                    }
                  tcmhd->SetParentNodeID( parentNodeID.c_str() );

                  if (smnd1)
                    {
                    // set it as modified
                    //smnd1->SetModifiedSinceRead(1);
                    // get display node BEFORE we add nodes to the target scene
                    vtkMRMLDisplayNode *sdnd2 = smnd1->GetDisplayNode();

                    vtkMRMLNode *tmodel = appLogic->GetMRMLScene()->CopyNode(smnd1);
                    vtkMRMLStorableNode::SafeDownCast(tmodel)->SetAndObserveStorageNodeID(nullptr);
                    vtkMRMLModelNode *mnd =vtkMRMLModelNode::SafeDownCast(tmodel);
                    tcmhd->SetModelNodeID( mnd->GetID() );

                    if (sdnd2)
                      {
                      vtkMRMLNode *tdnd = appLogic->GetMRMLScene()->CopyNode(sdnd2);
                      mnd->SetAndObserveDisplayNodeID( tdnd->GetID() );
                      }
                    }

                  if (sdnd1)
                    {
                    vtkMRMLNode *tdnd = appLogic->GetMRMLScene()->CopyNode(sdnd1);
                    tcmhd->SetAndObserveDisplayNodeID( tdnd->GetID() );
                    }
                  }
                }
              }
            }
          if (childNodesThatNeedParentsIDsRemapped.size() > 0)
            {
            // iterate through all the imported hierarchies that failed and double check their parent node ids
            for (unsigned int i = 0; i < childNodesThatNeedParentsIDsRemapped.size(); i++)
              {
              std::map<std::string,std::string>::iterator pIt = parentNodeIDMapper.find(childNodesThatNeedParentsIDsRemapped[i]->GetParentNodeID());
              if (pIt != parentNodeIDMapper.end())
                {
                vtkDebugWithObjectMacro(appLogic, "Remapping child node " << childNodesThatNeedParentsIDsRemapped[i]->GetName() << \
                  " parent node id from " << childNodesThatNeedParentsIDsRemapped[i]->GetParentNodeID() << " to " << pIt->second.c_str());
                childNodesThatNeedParentsIDsRemapped[i]->SetParentNodeID(pIt->second.c_str());
                }
              }
            }
          }
        }
      else if (!source)
        {
        std::stringstream information;
        information << "Node " << (*sit) << " not found in scene file "
                    << m_Filename << std::endl;
        vtkGenericWarningMacro( << information.str().c_str() );
        }
      else if (!target)
        {
        std::stringstream information;
        information << "Node " << (*tit) << " not found in current scene."
                    << std::endl;
        vtkGenericWarningMacro( << information.str().c_str() );
        }

      ++sit;
      ++tit;
      }

    appLogic->GetMRMLScene()->EndState(vtkMRMLScene::ImportState);

    // Delete the file if requested
    if (m_DeleteFile)
      {
      int removed;
      removed = itksys::SystemTools::RemoveFile( m_Filename.c_str() );
      if (!removed)
        {
        std::stringstream information;
        information << "Unable to delete temporary file "
                    << m_Filename << std::endl;
        vtkGenericWarningMacro( << information.str().c_str() );
        }
      }
  }

protected:
  std::vector<std::string> m_TargetNodes;
  std::vector<std::string> m_SourceNodes;
  std::string m_Filename;
  int m_DisplayData;
  int m_DeleteFile;
};

//----------------------------------------------------------------------------
class ReadDataRequestUpdateParentTransform : public DataRequest
{
public:
  ReadDataRequestUpdateParentTransform(const std::string& updatedNode,
    const std::string& parentTransformNode, int uid = 0)
    : DataRequest(uid)
  {
    m_UpdatedNode = updatedNode;
    m_ParentTransformNode = parentTransformNode;
  }

  void Execute(vtkSlicerApplicationLogic* appLogic) override
  {
    vtkMRMLScene* scene = appLogic->GetMRMLScene();
    vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(
      scene->GetNodeByID(m_UpdatedNode));
    if (node)
      {
      node->SetAndObserveTransformNodeID(m_ParentTransformNode.c_str());
      }
  }

protected:
  std::string m_UpdatedNode;
  std::string m_ParentTransformNode;
};

//----------------------------------------------------------------------------
class ReadDataRequestUpdateSubjectHierarchyLocation : public DataRequest
{
public:
  ReadDataRequestUpdateSubjectHierarchyLocation(const std::string& updatedNode,
    const std::string& siblingNode, int uid = 0)
    : DataRequest(uid)
  {
    m_UpdatedNode = updatedNode;
    m_SubjectHierarchySiblingNode = siblingNode;
  }

  void Execute(vtkSlicerApplicationLogic* appLogic) override
  {
    vtkMRMLScene* scene = appLogic->GetMRMLScene();

    vtkMRMLNode *siblingNode = scene->GetNodeByID(m_SubjectHierarchySiblingNode);
    vtkMRMLNode *updatedNode = scene->GetNodeByID(m_UpdatedNode);
    vtkMRMLSubjectHierarchyNode *shnd = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
    vtkIdType siblingNodeShItemID = shnd->GetItemByDataNode(siblingNode);
    vtkIdType updatedNodeShItemID = shnd->GetItemByDataNode(updatedNode);

    if (updatedNodeShItemID && siblingNodeShItemID)
      {
      vtkIdType parentItemID = shnd->GetItemParent(siblingNodeShItemID);
      shnd->SetItemParent(updatedNodeShItemID, parentItemID);
      shnd->SetItemLevel(updatedNodeShItemID, shnd->GetItemLevel(siblingNodeShItemID));
      }
  }

protected:
  std::string m_UpdatedNode;
  std::string m_SubjectHierarchySiblingNode;
};

//----------------------------------------------------------------------------
class ReadDataRequestAddNodeReference : public DataRequest
{
public:
  ReadDataRequestAddNodeReference(const std::string& referencingNode,
    const std::string& referencedNode, const std::string& role, int uid = 0)
    : DataRequest(uid)
  {
    m_ReferencingNode = referencingNode;
    m_ReferencedNode = referencedNode;
    m_Role = role;
  }

  void Execute(vtkSlicerApplicationLogic* appLogic) override
  {
    vtkMRMLScene* scene = appLogic->GetMRMLScene();

    vtkMRMLNode *referencingNode = scene->GetNodeByID(m_ReferencingNode);
    vtkMRMLNode *referencedNode = scene->GetNodeByID(m_ReferencedNode);
    if (referencingNode && referencedNode)
      {
      referencingNode->AddNodeReferenceID(m_Role.c_str(), m_ReferencedNode.c_str());
      }
  }

protected:
  std::string m_ReferencingNode;
  std::string m_ReferencedNode;
  std::string m_Role;
};

//----------------------------------------------------------------------------
class WriteDataRequestFile: public DataRequest
{
public:
  WriteDataRequestFile(
      const std::string& vtkNotUsed(node),
      const std::string& vtkNotUsed(filename),
      int uid = 0)
    : DataRequest(uid)
  {
  }
protected:
  std::string m_SourceNode;
  std::string m_Filename;
};

//----------------------------------------------------------------------------
class WriteDataRequestScene: public DataRequest
{
public:
  WriteDataRequestScene(const std::vector<std::string>& targetNodes,
                   const std::vector<std::string>& sourceNodes,
                   const std::string& filename,
                   int uid = 0)
    : DataRequest(uid)
  {
    m_TargetNodes = targetNodes;
    m_SourceNodes = sourceNodes;
    m_Filename = filename;
  }

  void Execute(vtkSlicerApplicationLogic* appLogic) override
  {
    if (m_SourceNodes.size() != m_TargetNodes.size())
      {
      // Can't do ID remapping if the two node lists are different
      // sizes. Just commit the scene. (This is where we would put to
      // the code to load into a node hierarchy (with a corresponding
      // change in the conditional above)).
      appLogic->GetMRMLScene()->SetURL( m_Filename.c_str() );
      appLogic->GetMRMLScene()->Commit();
      return;
      }
  }

protected:
  std::vector<std::string> m_TargetNodes;
  std::vector<std::string> m_SourceNodes;
  std::string m_Filename;
};


#endif // __vtkSlicerApplicationLogicRequests_h
