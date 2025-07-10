// SlicerLogic includes
#include "vtkSlicerSceneViewsModuleLogic.h"

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// Sequences MRML includes
#include <vtkMRMLSequenceNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLSceneViewStorageNode.h>
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLTextNode.h>
#include <vtkMRMLVectorVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <iostream>
#include <sstream>

const int MAXIMUM_NUMBER_OF_NODES_WITHOUT_BATCH_PROCESSING = 25;

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSceneViewsModuleLogic);

//-----------------------------------------------------------------------------
// vtkSlicerSceneViewsModuleLogic methods
//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic::vtkSlicerSceneViewsModuleLogic() = default;

//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic::~vtkSlicerSceneViewsModuleLogic() = default;

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkDebugMacro("SetMRMLSceneInternal - listening to scene events");

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneEndImport()
{
  vtkDebugMacro("OnMRMLSceneEndImport");
  if (!this->GetMRMLScene())
  {
    return;
  }

  if (this->GetApplicationLogic())
  {
    this->GetApplicationLogic()->PauseRender();
  }

  this->ConvertSceneViewNodesToSequenceBrowserNodes(this->GetMRMLScene());

  if (this->GetApplicationLogic())
  {
    this->GetApplicationLogic()->ResumeRender();
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneEndRestore()
{
  vtkDebugMacro("OnMRMLSceneEndRestore");
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLNodeModified(vtkMRMLNode* vtkNotUsed(node)) {}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneEndClose() {}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::ConvertSceneViewNodesToSequenceBrowserNodes(vtkMRMLScene* scene)
{
  if (!scene)
  {
    return;
  }

  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLSceneViewNode", nodes);
  if (nodes.empty())
  {
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->AddNewSceneViewSequenceBrowserNode();
  MRMLNodeModifyBlocker blocker(sequenceBrowser);
  for (vtkMRMLNode* node : nodes)
  {
    vtkMRMLSceneViewNode* sceneView = vtkMRMLSceneViewNode::SafeDownCast(node);
    if (!sceneView)
    {
      continue;
    }

    this->ConvertSceneViewNodeToSequenceBrowserNode(sceneView, sequenceBrowser);
  }
}

//-----------------------------------------------------------------------------
vtkMRMLSequenceNode* vtkSlicerSceneViewsModuleLogic::GetOrAddSceneViewSequenceNode(
  vtkMRMLSequenceBrowserNode* sequenceBrowser,
  vtkMRMLNode* proxyNode)
{
  vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(this->GetModuleLogic("Sequences"));
  if (!sequencesLogic)
  {
    vtkErrorMacro("GetOrAddSceneViewSequenceNode: Failed to get sequences logic.");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLSequenceNode> sequenceNode = sequenceBrowser->GetSequenceNode(proxyNode);
  if (!sequenceNode)
  {
    sequenceNode = vtkSmartPointer<vtkMRMLSequenceNode>::Take(
      vtkMRMLSequenceNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSequenceNode")));

    const char* proxyNodeName = proxyNode->GetName();
    std::string baseName = (proxyNodeName && strlen(proxyNodeName) > 0) ? proxyNodeName : "Default";
    std::stringstream nodenameStringstream;
    nodenameStringstream << "SceneViewSequence_" << baseName;
    std::string uniqueName = this->GetMRMLScene()->GenerateUniqueName(nodenameStringstream.str());
    sequenceNode->SetName(uniqueName.c_str());

    sequenceNode->HideFromEditorsOn();
    this->GetMRMLScene()->AddNode(sequenceNode);
    sequencesLogic->AddSynchronizedNode(sequenceNode, proxyNode, sequenceBrowser);
    sequenceNode->SetAttribute(vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeName(),
                               vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeValue());
    sequenceBrowser->SetMissingItemMode(sequenceNode, vtkMRMLSequenceBrowserNode::MissingItemDisplayHidden);
  }

  return sequenceNode;
}

//-----------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkSlicerSceneViewsModuleLogic::ConvertSceneViewNodeToSequenceBrowserNode(
  vtkMRMLSceneViewNode* sceneViewNode,
  vtkMRMLSequenceBrowserNode* sequenceBrowser)
{
  vtkMRMLScene* snapshotScene = sceneViewNode->GetStoredScene();

  std::vector<vtkMRMLNode*> snapshotNodes;
  snapshotScene->GetNodesByClass("vtkMRMLNode", snapshotNodes);

  if (!sequenceBrowser)
  {
    sequenceBrowser = this->AddNewSceneViewSequenceBrowserNode();
  }
  if (!sequenceBrowser)
  {
    vtkErrorMacro("ConvertSceneViewNodeToSequenceBrowserNode: Failed to get or create sequence browser node.");
    return nullptr;
  }

  // We don't want to waste time updating proxy nodes, so we block the modified events
  std::map<vtkMRMLNode*, int> wasDisabledModifiedEvents;

  wasDisabledModifiedEvents[sequenceBrowser] = sequenceBrowser->GetDisableModifiedEvent();
  sequenceBrowser->DisableModifiedEventOn();

  std::vector<vtkMRMLNode*> proxyNodes;
  std::map<vtkMRMLNode*, vtkMRMLNode*> proxyNodeToSnapshotMap;
  for (vtkMRMLNode* snapshotNode : snapshotNodes)
  {
    if (!snapshotNode->IsA("vtkMRMLDisplayNode"))
    {
      continue;
    }

    vtkMRMLNode* proxyNode = this->GetMRMLScene()->GetNodeByID(snapshotNode->GetID());
    if (!proxyNode)
    {
      proxyNode = snapshotNode->CreateNodeInstance();
      this->GetMRMLScene()->AddNode(proxyNode);
      proxyNode->Copy(snapshotNode);
    }

    proxyNodes.push_back(proxyNode);
    proxyNodeToSnapshotMap[proxyNode] = snapshotNode;
  }

  const char* sceneViewName = sceneViewNode->GetName();
  const char* sceneViewDescription = sceneViewNode->GetDescription();
  int sceneViewScreenshotType = sceneViewNode->GetScreenShotType();
  vtkImageData* sceneViewScreenshot = sceneViewNode->GetScreenShot();

  vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(this->GetModuleLogic("Sequences"));
  for (vtkMRMLNode* proxyNode : proxyNodes)
  {
    vtkSmartPointer<vtkMRMLSequenceNode> sequenceNode = this->GetOrAddSceneViewSequenceNode(sequenceBrowser, proxyNode);
    int disabledModifiedEvent = sequenceNode->GetDisableModifiedEvent();
    wasDisabledModifiedEvents[sequenceNode] = disabledModifiedEvent;
    sequenceNode->DisableModifiedEventOn();
  }

  this->CreateSceneView(proxyNodes,
                        sceneViewName ? sceneViewName : "",
                        sceneViewDescription ? sceneViewDescription : "",
                        sceneViewScreenshotType,
                        sceneViewScreenshot,
                        sequenceBrowser);

  int index = this->GetNumberOfSceneViews() - 1;
  for (vtkMRMLNode* proxyNode : proxyNodes)
  {
    vtkMRMLNode* snapshotNode = proxyNodeToSnapshotMap[proxyNode];
    if (!snapshotNode)
    {
      continue;
    }

    vtkMRMLNode* dataNode = this->GetNthSceneViewDataNode(index, proxyNode);
    if (!dataNode)
    {
      continue;
    }
    dataNode->GetScene()->StartState(vtkMRMLScene::BatchProcessState);
    dataNode->CopyContent(snapshotNode);
    dataNode->GetScene()->EndState(vtkMRMLScene::BatchProcessState);
  }

  // Restore the disabled modified event state
  for (auto wasDisabledModifiedEntry : wasDisabledModifiedEvents)
  {
    vtkMRMLNode* node = wasDisabledModifiedEntry.first;
    node->SetDisableModifiedEvent(wasDisabledModifiedEntry.second);
  }
  this->GetMRMLScene()->RemoveNode(sceneViewNode);
  return sequenceBrowser;
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
  {
    std::cerr << "RegisterNodes: no scene on which to register nodes" << std::endl;
    return;
  }

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  this->GetMRMLScene()->RegisterNodeClass(sceneViewNode);
  // SceneSnapshot backward compatibility
#if MRML_APPLICATION_SUPPORT_VERSION < MRML_VERSION_CHECK(4, 0, 0)
  this->GetMRMLScene()->RegisterNodeClass(sceneViewNode, "SceneSnapshot");
#endif

  vtkNew<vtkMRMLSceneViewStorageNode> storageNode;
  this->GetMRMLScene()->RegisterNodeClass(storageNode);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::GetDisplayNodes(std::vector<vtkMRMLNode*>& nodes)
{
  std::vector<vtkMRMLNode*> displayNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLDisplayNode", displayNodes);
  nodes.insert(nodes.end(), displayNodes.begin(), displayNodes.end());

  std::vector<vtkMRMLNode*> volumePropertyNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLVolumePropertyNode", volumePropertyNodes);
  nodes.insert(nodes.end(), volumePropertyNodes.begin(), volumePropertyNodes.end());

  std::vector<vtkMRMLNode*> clipNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLClipNode", clipNodes);
  nodes.insert(nodes.end(), clipNodes.begin(), clipNodes.end());

  std::vector<vtkMRMLNode*> crosshairNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLCrosshairNode", crosshairNodes);
  nodes.insert(nodes.end(), crosshairNodes.begin(), crosshairNodes.end());
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::GetViewNodes(std::vector<vtkMRMLNode*>& nodes)
{
  std::vector<vtkMRMLNode*> viewNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLAbstractViewNode", viewNodes);
  nodes.insert(nodes.end(), viewNodes.begin(), viewNodes.end());

  std::vector<vtkMRMLNode*> cameraNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  nodes.insert(nodes.end(), cameraNodes.begin(), cameraNodes.end());

  std::vector<vtkMRMLNode*> sliceCompositeNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceCompositeNode", sliceCompositeNodes);
  nodes.insert(nodes.end(), sliceCompositeNodes.begin(), sliceCompositeNodes.end());

  std::vector<vtkMRMLNode*> layoutNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLLayoutNode", layoutNodes);
  nodes.insert(nodes.end(), layoutNodes.begin(), layoutNodes.end());
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::CreateSceneView(std::string name,
                                                     std::string description /*=""*/,
                                                     int screenshotType /*=ScreenShotType3D*/,
                                                     vtkImageData* screenshot /*=nullptr*/,
                                                     bool saveDisplayNodes /*=true*/,
                                                     bool saveViewNodes /*=true*/,
                                                     vtkMRMLSequenceBrowserNode* sequenceBrowser /*=nullptr*/)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("CreateSceneView: No scene set.");
    return;
  }

  std::vector<vtkMRMLNode*> savedNodes;

  if (saveDisplayNodes)
  {
    this->GetDisplayNodes(savedNodes);
  }

  if (saveViewNodes)
  {
    this->GetViewNodes(savedNodes);
  }

  this->CreateSceneView(savedNodes, name, description, screenshotType, screenshot, sequenceBrowser);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::CreateSceneView(vtkCollection* savedNodes,
                                                     std::string name,
                                                     std::string description /*=""*/,
                                                     int screenshotType /*ScreenShotType3D*/,
                                                     vtkImageData* screenshot /*=nullptr*/,
                                                     vtkMRMLSequenceBrowserNode* sequenceBrowser /*=nullptr*/)
{
  std::vector<vtkMRMLNode*> savedNodesVector;
  for (int i = 0; i < savedNodes->GetNumberOfItems(); ++i)
  {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(savedNodes->GetItemAsObject(i));
    if (node)
    {
      savedNodesVector.push_back(node);
    }
  }
  this->CreateSceneView(savedNodesVector, name, description, screenshotType, screenshot, sequenceBrowser);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::CreateSceneView(std::vector<vtkMRMLNode*> savedNodes,
                                                     std::string name,
                                                     std::string description /*=""*/,
                                                     int screenshotType /*=ScreenShotType3D*/,
                                                     vtkImageData* screenshot /*nullptr*/,
                                                     vtkMRMLSequenceBrowserNode* sequenceBrowser /*=nullptr*/)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("CreateSceneView: No scene set.");
    return;
  }

  if (!sequenceBrowser)
  {
    sequenceBrowser = this->GetSceneViewSequenceBrowserNode(true);
  }

  if (!sequenceBrowser)
  {
    vtkErrorMacro("CreateSceneView: Failed to get or create sequence browser node.");
    return;
  }

  MRMLNodeModifyBlocker blocker(sequenceBrowser);
  if (this->GetApplicationLogic())
  {
    this->GetApplicationLogic()->PauseRender();
  }

  bool wasRecordingActive = sequenceBrowser->GetRecordingActive();
  sequenceBrowser->RecordingActiveOn();

  std::vector<vtkMRMLSequenceNode*> sequenceNodes;
  sequenceBrowser->GetSynchronizedSequenceNodes(sequenceNodes, true);
  for (vtkMRMLSequenceNode* sequenceNode : sequenceNodes)
  {
    sequenceBrowser->SetRecording(sequenceNode, false);
  }

  vtkMRMLVolumeNode* screenshotNode = this->GetSceneViewScreenshotProxyNode(sequenceBrowser);
  if (screenshotNode)
  {
    savedNodes.push_back(screenshotNode);
  }

  // Remove duplicates by converting to a set
  std::set<vtkMRMLNode*> savedNodesSet(savedNodes.begin(), savedNodes.end());

  if (savedNodesSet.size() > MAXIMUM_NUMBER_OF_NODES_WITHOUT_BATCH_PROCESSING)
  {
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);
  }

  vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(this->GetModuleLogic("Sequences"));
  if (!sequencesLogic)
  {
    vtkErrorMacro("CreateSceneView: Failed to get sequences logic.");
    return;
  }

  for (vtkMRMLNode* node : savedNodesSet)
  {
    if (!node)
    {
      continue;
    }

    // If the node is involved in scene views, we should not save it in the scene view sequences
    if (this->IsSceneViewNode(node))
    {
      continue;
    }

    vtkSmartPointer<vtkMRMLSequenceNode> sequenceNode = this->GetOrAddSceneViewSequenceNode(sequenceBrowser, node);
    sequenceBrowser->SetRecording(sequenceNode, true);
  }
  sequenceBrowser->SetRecordingActive(wasRecordingActive);
  sequenceBrowser->SaveProxyNodesState();

  std::vector<vtkMRMLSequenceNode*> savedSequenceNodes;
  sequenceBrowser->GetSynchronizedSequenceNodes(savedSequenceNodes, true);
  for (vtkMRMLSequenceNode* sequenceNode : savedSequenceNodes)
  {
    sequenceNode->AddDefaultStorageNode();
  }

  int index = this->GetNumberOfSceneViews() - 1;
  this->ModifyNthSceneView(index, name, description, screenshotType, screenshot);

  if (savedNodesSet.size() > MAXIMUM_NUMBER_OF_NODES_WITHOUT_BATCH_PROCESSING)
  {
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
  }
  if (this->GetApplicationLogic())
  {
    this->GetApplicationLogic()->ResumeRender();
  }
  this->InvokeEvent(vtkSlicerSceneViewsModuleLogic::SceneViewsModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::UpdateNthSceneView(int sceneViewIndex,
                                                        bool updateExistingNodes /*=true*/,
                                                        bool saveDisplayNodes /*=true*/,
                                                        bool saveViewNodes /*=true*/)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("CreateSceneView: No scene set.");
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(sceneViewIndex);
  if (!sequenceBrowser)
  {
    vtkErrorMacro("UpdateNthSceneView: Failed to get sequence browser node.");
    return;
  }

  std::vector<vtkMRMLNode*> savedNodes;
  if (saveDisplayNodes)
  {
    this->GetDisplayNodes(savedNodes);
  }
  if (saveViewNodes)
  {
    this->GetViewNodes(savedNodes);
  }
  this->UpdateNthSceneView(savedNodes, sceneViewIndex, updateExistingNodes);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::UpdateNthSceneView(vtkCollection* savedNodes,
                                                        int sceneViewIndex,
                                                        bool updateExistingNodes /*=true*/)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("UpdateNthSceneView: No scene set.");
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(sceneViewIndex);
  if (!sequenceBrowser)
  {
    vtkErrorMacro("UpdateNthSceneView: Failed to get sequence browser node.");
    return;
  }

  std::vector<vtkMRMLNode*> savedNodesVector;
  for (int i = 0; i < savedNodes->GetNumberOfItems(); ++i)
  {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(savedNodes->GetItemAsObject(i));
    if (node)
    {
      savedNodesVector.push_back(node);
    }
  }
  this->UpdateNthSceneView(savedNodesVector, sceneViewIndex, updateExistingNodes);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::UpdateNthSceneView(std::vector<vtkMRMLNode*> savedNodes,
                                                        int sceneViewIndex,
                                                        bool updateExistingNodes /*=true*/)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("UpdateNthSceneView: No scene set.");
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(sceneViewIndex);
  if (!sequenceBrowser)
  {
    vtkErrorMacro("UpdateNthSceneView: Failed to get sequence browser node.");
    return;
  }

  MRMLNodeModifyBlocker blocker(sequenceBrowser);
  if (this->GetApplicationLogic())
  {
    this->GetApplicationLogic()->PauseRender();
  }

  std::vector<vtkMRMLSequenceNode*> sequenceNodes;
  sequenceBrowser->GetSynchronizedSequenceNodes(sequenceNodes, true);

  vtkMRMLVolumeNode* screenshotNode = this->GetSceneViewScreenshotProxyNode(sequenceBrowser);

  std::vector<vtkMRMLNode*> existingNodes;
  if (updateExistingNodes)
  {
    // Get existing nodes from the sequence browser
    sequenceBrowser->GetAllProxyNodes(existingNodes);
  }

  // Remove duplicates by converting to a set
  std::set<vtkMRMLNode*> savedNodesSet(savedNodes.begin(), savedNodes.end());
  savedNodesSet.insert(existingNodes.begin(), existingNodes.end());

  if (savedNodesSet.size() > MAXIMUM_NUMBER_OF_NODES_WITHOUT_BATCH_PROCESSING)
  {
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);
  }

  vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(this->GetModuleLogic("Sequences"));
  if (!sequencesLogic)
  {
    vtkErrorMacro("UpdateNthSceneView: Failed to get sequences logic.");
    return;
  }

  int sequenceIndex = this->SceneViewIndexToSequenceBrowserIndex(sceneViewIndex);
  vtkMRMLSequenceNode* screenshotSequenceNode = sequenceBrowser->GetSequenceNode(screenshotNode);
  std::string value = screenshotSequenceNode->GetNthIndexValue(sequenceIndex);

  std::string name = this->GetNthSceneViewName(sceneViewIndex);
  std::string description = this->GetNthSceneViewDescription(sceneViewIndex);
  int screenshotType = this->GetNthSceneViewScreenshotType(sceneViewIndex);
  vtkSmartPointer<vtkImageData> screenshot = this->GetNthSceneViewScreenshot(sceneViewIndex);

  for (vtkMRMLNode* node : savedNodesSet)
  {
    if (!node)
    {
      continue;
    }
    // If the node is involved in scene views, we should not save it in the scene view sequences
    if (this->IsSceneViewNode(node))
    {
      continue;
    }

    vtkSmartPointer<vtkMRMLSequenceNode> sequenceNode = this->GetOrAddSceneViewSequenceNode(sequenceBrowser, node);
    sequenceNode->SetDataNodeAtValue(node, value);
  }

  // Restore existing scene view name
  this->ModifyNthSceneView(sceneViewIndex, name, description, screenshotType, screenshot);

  std::vector<vtkMRMLSequenceNode*> savedSequenceNodes;
  sequenceBrowser->GetSynchronizedSequenceNodes(savedSequenceNodes, true);
  for (vtkMRMLSequenceNode* sequenceNode : savedSequenceNodes)
  {
    sequenceNode->AddDefaultStorageNode();
  }

  if (savedNodesSet.size() > MAXIMUM_NUMBER_OF_NODES_WITHOUT_BATCH_PROCESSING)
  {
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
  }

  this->RestoreSceneView(sceneViewIndex);

  if (this->GetApplicationLogic())
  {
    this->GetApplicationLogic()->ResumeRender();
  }
  this->InvokeEvent(vtkSlicerSceneViewsModuleLogic::SceneViewsModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::ModifyNthSceneView(int index,
                                                        std::string name,
                                                        std::string description,
                                                        int screenshotType,
                                                        vtkImageData* screenshot)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("ModifyNthSceneView: No scene set.");
    return;
  }

  vtkMRMLVolumeNode* screenshotNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotNode)
  {
    vtkErrorMacro("ModifyNthSceneView: Failed to get scene view at index: " << index);
    return;
  }

  MRMLNodeModifyBlocker blocker(screenshotNode);
  this->SetNthSceneViewScreenshot(index, screenshot);
  this->SetNthSceneViewName(index, name);
  this->SetNthSceneViewDescription(index, description);
  this->SetNthSceneViewScreenshotType(index, screenshotType);
  this->InvokeEvent(vtkSlicerSceneViewsModuleLogic::SceneViewsModifiedEvent);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshotProxyNode(
  vtkMRMLSequenceBrowserNode* sequenceBrowser /*=nullptr*/)
{
  if (!sequenceBrowser)
  {
    sequenceBrowser = this->GetSceneViewSequenceBrowserNode(false);
  }

  if (!sequenceBrowser)
  {
    // No scene view sequence browser node exists, so no scene views are available
    return nullptr;
  }

  vtkMRMLVolumeNode* screenshotNode = vtkMRMLVolumeNode::SafeDownCast(
    sequenceBrowser->GetNodeReference(vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshotReferenceRole()));
  if (screenshotNode && !screenshotNode->GetImageData())
  {
    // Initialize the screenshot image data to ensure that the correct sequence nodes are created.
    vtkNew<vtkImageData> tempScreenshot;
    tempScreenshot->SetDimensions(1, 1, 1);
    tempScreenshot->AllocateScalars(VTK_CHAR, 3);
    screenshotNode->SetAndObserveImageData(tempScreenshot);
  }

  return screenshotNode;
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetNthSceneViewName(int index, std::string name)
{
  vtkMRMLVolumeNode* screenshotProxyNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotProxyNode)
  {
    vtkErrorMacro("SetNthSceneViewName: Failed to get name proxy node.");
    return;
  }

  this->SetNthNodeAttribute(screenshotProxyNode, index, this->GetSceneViewNameAttributeName(), name);
}

//---------------------------------------------------------------------------
std::string vtkSlicerSceneViewsModuleLogic::GetNthSceneViewName(int index)
{
  vtkMRMLVolumeNode* screenshotNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotNode)
  {
    // No name node is available
    return "";
  }

  return this->GetNthNodeAttribute(screenshotNode, index, this->GetSceneViewNameAttributeName());
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetNthSceneViewDescription(int index, std::string description)
{
  vtkMRMLVolumeNode* screenshotNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotNode)
  {
    vtkErrorMacro("SetNthSceneViewName: Failed to get name proxy node.");
    return;
  }

  this->SetNthNodeAttribute(screenshotNode, index, this->GetSceneViewDescriptionAttributeName(), description);
}

//---------------------------------------------------------------------------
std::string vtkSlicerSceneViewsModuleLogic::GetNthSceneViewDescription(int index)
{
  vtkMRMLVolumeNode* screenshotNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotNode)
  {
    vtkErrorMacro("SetNthSceneViewName: Failed to get name proxy node.");
    return "";
  }

  return this->GetNthNodeAttribute(screenshotNode, index, this->GetSceneViewDescriptionAttributeName());
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetNthSceneViewScreenshotType(int index, int type)
{
  vtkMRMLVolumeNode* screenshotNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotNode)
  {
    vtkErrorMacro("SetNthSceneViewName: Failed to get name proxy node.");
    return;
  }
  this->SetNthNodeAttribute(
    screenshotNode, index, this->GetSceneViewScreenshotTypeAttributeName(), this->GetScreenShotTypeAsString(type));
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::GetNthSceneViewScreenshotType(int index)
{
  vtkMRMLVolumeNode* screenshotNode = this->GetNthSceneViewScreenshotProxyNode(index);
  if (!screenshotNode)
  {
    vtkErrorMacro("SetNthSceneViewName: Failed to get name proxy node.");
    return -1;
  }
  std::string screenshotTypeString =
    this->GetNthNodeAttribute(screenshotNode, index, this->GetSceneViewScreenshotTypeAttributeName());
  return this->GetScreenShotTypeFromString(screenshotTypeString);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetNthSceneViewScreenshot(int index, vtkImageData* screenshot)
{
  vtkMRMLVolumeNode* screenshotDataNode = this->GetNthSceneViewScreenshotDataNode(index);
  if (!screenshotDataNode)
  {
    vtkErrorMacro("GetNthSceneViewScreenshot: Failed to get screenshot data node.");
    return;
  }

  if (screenshot)
  {
    screenshotDataNode->SetAndObserveImageData(screenshot);
  }
  else
  {
    vtkNew<vtkImageData> tempScreenshot;
    tempScreenshot->SetDimensions(1, 1, 1);
    tempScreenshot->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    screenshotDataNode->SetAndObserveImageData(tempScreenshot);
  }
}

//---------------------------------------------------------------------------
vtkImageData* vtkSlicerSceneViewsModuleLogic::GetNthSceneViewScreenshot(int index)
{
  vtkMRMLVolumeNode* screenshotDataNode = this->GetNthSceneViewScreenshotDataNode(index);
  if (!screenshotDataNode)
  {
    vtkErrorMacro("GetNthSceneViewScreenshot: Failed to get screenshot data node.");
    return nullptr;
  }

  vtkImageData* screenshot = screenshotDataNode->GetImageData();
  if (screenshot)
  {
    int dimensions[3] = { 0, 0, 0 };
    screenshot->GetDimensions(dimensions);
    if (dimensions[0] <= 1 && dimensions[1] <= 1 && dimensions[2] <= 1)
    {
      // Screenshot is not valid.
      return nullptr;
    }
  }
  return screenshot;
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::SceneViewIndexToSequenceBrowserIndex(int sceneViewIndex)
{
  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(sceneViewIndex);
  if (!sequenceBrowser)
  {
    vtkErrorMacro("SceneViewIndexToSequenceBrowserIndex: Failed to get scene view sequence browser node.");
    return false;
  }

  int sequenceBrowserIndex = 0;
  std::vector<vtkMRMLNode*> nodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLSequenceBrowserNode", nodes);
  for (vtkMRMLNode* node : nodes)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
    if (!sequenceBrowserNode || !this->IsSceneViewNode(sequenceBrowserNode))
    {
      continue;
    }

    if (sequenceBrowserNode == sequenceBrowser)
    {
      break;
    }

    sequenceBrowserIndex += sequenceBrowserNode->GetNumberOfItems();
  }

  return sceneViewIndex - sequenceBrowserIndex;
}

//---------------------------------------------------------------------------
bool vtkSlicerSceneViewsModuleLogic::RestoreSceneView(int sceneIndex)
{
  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(sceneIndex);
  if (!sequenceBrowser)
  {
    vtkErrorMacro("RestoreSceneView: Failed to get scene view sequence browser node.");
    return false;
  }

  int sequenceBrowserIndex = this->SceneViewIndexToSequenceBrowserIndex(sceneIndex);
  if (sequenceBrowserIndex < 0 || sequenceBrowserIndex >= sequenceBrowser->GetNumberOfItems())
  {
    vtkErrorMacro("RestoreSceneView: Invalid item number.");
    return -1;
  }

  if (sequenceBrowser->GetSelectedItemNumber() != sequenceBrowserIndex)
  {
    sequenceBrowser->SetSelectedItemNumber(sequenceBrowserIndex);
  }
  else
  {
    vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(this->GetModuleLogic("Sequences"));
    sequencesLogic->UpdateProxyNodesFromSequences(sequenceBrowser);
  }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerSceneViewsModuleLogic::RestoreSceneView(std::string name)
{
  int sceneIndex = this->GetSceneViewIndexByName(name);
  if (sceneIndex < 0)
  {
    vtkErrorMacro("RestoreSceneView: Failed to get scene view index.");
    return false;
  }

  return this->RestoreSceneView(sceneIndex);
}

//---------------------------------------------------------------------------
std::string vtkSlicerSceneViewsModuleLogic::MoveSceneViewUp(std::string vtkNotUsed(sceneViewName))
{
  // reset stringHolder
  this->m_StringHolder = "";

  vtkErrorMacro("MoveSceneViewUp: operation not supported!");
  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
std::string vtkSlicerSceneViewsModuleLogic::MoveSceneViewDown(std::string vtkNotUsed(sceneViewName))
{
  // reset stringHolder
  this->m_StringHolder = "";

  vtkErrorMacro("MoveSceneViewDown: operation not supported!");
  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::GetSceneViewIndexByName(std::string name)
{
  for (int i = 0; i < this->GetNumberOfSceneViews(); ++i)
  {
    if (this->GetNthSceneViewName(i).compare(name) == 0)
    {
      return i;
    }
  }
  return -1;
}

//---------------------------------------------------------------------------
bool vtkSlicerSceneViewsModuleLogic::RemoveSceneView(int sceneIndex)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("RemoveSceneView: No scene set.");
    return true;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(sceneIndex);
  if (!sequenceBrowser)
  {
    vtkErrorMacro("RemoveSceneView: Invalid sequence browser node.");
    return false;
  }

  int sequenceBrowserIndex = this->SceneViewIndexToSequenceBrowserIndex(sceneIndex);
  if (sequenceBrowserIndex < 0 || sequenceBrowserIndex >= sequenceBrowser->GetNumberOfItems())
  {
    vtkErrorMacro("RestoreSceneView: Invalid item number.");
    return -1;
  }

  if (sequenceBrowserIndex < 0 || sequenceBrowserIndex >= sequenceBrowser->GetNumberOfItems())
  {
    vtkErrorMacro("RemoveSceneView: Invalid item number.");
    return false;
  }

  vtkMRMLVolumeNode* screenshotNode = this->GetSceneViewScreenshotProxyNode();
  if (!screenshotNode)
  {
    // No name node is available
    return false;
  }

  if (sequenceBrowser->GetNumberOfItems() == 1)
  {
    // Removing the last item in a sequence will cause an update of the scene nodes.
    // To avoid this, remove the sequence browser and all synchronized sequence nodes.
    // If a scene view is added later, then a new sequence browser will be created.
    std::vector<vtkMRMLSequenceNode*> sequenceNodes;
    sequenceBrowser->GetSynchronizedSequenceNodes(sequenceNodes, true);
    this->GetMRMLScene()->RemoveNode(sequenceBrowser);
    for (vtkMRMLSequenceNode* sequenceNode : sequenceNodes)
    {
      this->GetMRMLScene()->RemoveNode(sequenceNode);
    }
    return true;
  }

  vtkMRMLSequenceNode* sequenceNode = sequenceBrowser->GetSequenceNode(screenshotNode);
  if (!sequenceNode)
  {
    // No sequence node is available
    return false;
  }

  std::vector<MRMLNodeModifyBlocker> blockers;
  blockers.emplace_back(sequenceBrowser);

  std::string value = sequenceNode->GetNthIndexValue(sequenceBrowserIndex);

  std::vector<vtkMRMLSequenceNode*> sequenceNodes;
  sequenceBrowser->GetSynchronizedSequenceNodes(sequenceNodes, true);

  for (vtkMRMLSequenceNode* sequenceNode : sequenceNodes)
  {
    blockers.emplace_back(sequenceNode);
  }
  for (vtkMRMLSequenceNode* sequenceNode : sequenceNodes)
  {
    if (sequenceNode->GetDataNodeAtValue(value))
    {
      sequenceNode->RemoveDataNodeAtValue(value);
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeName()
{
  return "SceneView";
}

//-----------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeValue()
{
  return "SceneView";
}

//-----------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::GetSceneViewNameAttributeName()
{
  return "SceneViewName";
}

//-----------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::GetSceneViewDescriptionAttributeName()
{
  return "SceneViewDescription";
}

//-----------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshotTypeAttributeName()
{
  return "ScreenshotType";
}

//-----------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshotReferenceRole()
{
  return "SceneViewScreenshot";
}

//---------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkSlicerSceneViewsModuleLogic::AddNewSceneViewSequenceBrowserNode()
{
  vtkSmartPointer<vtkMRMLSequenceBrowserNode> sequenceBrowserNode = vtkSmartPointer<vtkMRMLSequenceBrowserNode>::Take(
    vtkMRMLSequenceBrowserNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLSequenceBrowserNode")));
  sequenceBrowserNode->SetName("SceneViews");
  sequenceBrowserNode->HideFromEditorsOn();
  sequenceBrowserNode->SetAttribute(vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeName(),
                                    vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeValue());
  sequenceBrowserNode->SetIndexDisplayMode(vtkMRMLSequenceBrowserNode::IndexDisplayAsIndex);
  this->GetMRMLScene()->AddNode(sequenceBrowserNode);

  vtkSmartPointer<vtkMRMLVectorVolumeNode> screenshotNode = vtkSmartPointer<vtkMRMLVectorVolumeNode>::Take(
    vtkMRMLVectorVolumeNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLVectorVolumeNode")));
  screenshotNode->HideFromEditorsOn();
  screenshotNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneViewScreenshot"));
  this->GetMRMLScene()->AddNode(screenshotNode);
  sequenceBrowserNode->AddNodeReferenceID(vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshotReferenceRole(),
                                          screenshotNode->GetID());

  vtkSmartPointer<vtkMRMLSequenceNode> sequenceNode =
    this->GetOrAddSceneViewSequenceNode(sequenceBrowserNode, screenshotNode);
  return sequenceBrowserNode;
}

//---------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkSlicerSceneViewsModuleLogic::GetNthSceneViewSequenceBrowserNode(int index)
{
  std::vector<vtkMRMLNode*> nodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLSequenceBrowserNode", nodes);
  int currentIndex = 0;
  for (vtkMRMLNode* node : nodes)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
    if (!sequenceBrowserNode)
    {
      continue;
    }

    const char* attributeValue =
      sequenceBrowserNode->GetAttribute(vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeName());
    if (!attributeValue
        || strcmp(attributeValue, vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeValue()) != 0)
    {
      continue;
    }

    int numberOfItems = sequenceBrowserNode->GetNumberOfItems();
    if (currentIndex + numberOfItems > index)
    {
      return sequenceBrowserNode;
    }

    currentIndex += numberOfItems;
  }
  return nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkSlicerSceneViewsModuleLogic::GetSceneViewSequenceBrowserNode(bool addMissingNodes)
{
  if (!this->GetMRMLScene())
  {
    return nullptr;
  }

  std::vector<vtkMRMLNode*> nodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLSequenceBrowserNode", nodes);

  for (vtkMRMLNode* node : nodes)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
    if (!sequenceBrowserNode)
    {
      continue;
    }

    if (this->IsSceneViewNode(sequenceBrowserNode))
    {
      return sequenceBrowserNode;
    }
  }

  vtkSmartPointer<vtkMRMLSequenceBrowserNode> sequenceBrowserNode;
  if (addMissingNodes)
  {
    sequenceBrowserNode = this->AddNewSceneViewSequenceBrowserNode();
  }

  return sequenceBrowserNode;
}

//---------------------------------------------------------------------------
std::string vtkSlicerSceneViewsModuleLogic::GetScreenShotTypeAsString(int type)
{
  switch (type)
  {
    case ScreenShotType3D:
      return "3D";
    case ScreenShotTypeRed:
      return "Red";
    case ScreenShotTypeYellow:
      return "Yellow";
    case ScreenShotTypeGreen:
      return "Green";
    case ScreenShotTypeFullLayout:
      return "FullLayout";
    default:
      return "Unknown";
  }
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::GetScreenShotTypeFromString(const std::string& type)
{
  if (type == this->GetScreenShotTypeAsString(ScreenShotType3D))
  {
    return ScreenShotType3D;
  }
  if (type == this->GetScreenShotTypeAsString(ScreenShotTypeRed))
  {
    return ScreenShotTypeRed;
  }
  if (type == this->GetScreenShotTypeAsString(ScreenShotTypeYellow))
  {
    return ScreenShotTypeYellow;
  }
  if (type == this->GetScreenShotTypeAsString(ScreenShotTypeGreen))
  {
    return ScreenShotTypeGreen;
  }
  if (type == this->GetScreenShotTypeAsString(ScreenShotTypeFullLayout))
  {
    return ScreenShotTypeFullLayout;
  }
  return -1;
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerSceneViewsModuleLogic::GetNthSceneViewScreenshotDataNode(int sceneViewIndex)
{
  vtkMRMLNode* screenshotProxyNode = this->GetNthSceneViewScreenshotProxyNode(sceneViewIndex);
  return vtkMRMLVolumeNode::SafeDownCast(this->GetNthSceneViewDataNode(sceneViewIndex, screenshotProxyNode));
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerSceneViewsModuleLogic::GetNthSceneViewScreenshotProxyNode(int index)
{
  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(index);
  if (!sequenceBrowser)
  {
    // No scene view sequence browser node exists, so no scene views are available
    return nullptr;
  }

  return this->GetSceneViewScreenshotProxyNode(sequenceBrowser);
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkSlicerSceneViewsModuleLogic::GetNthSceneViewDataNode(int index, vtkMRMLNode* proxyNode)
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("GetNthSceneViewDataNode: No scene set.");
    return nullptr;
  }

  if (!proxyNode)
  {
    vtkErrorMacro("Invalid proxy node");
    return nullptr;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowser = this->GetNthSceneViewSequenceBrowserNode(index);
  int sequenceBrowserIndex = this->SceneViewIndexToSequenceBrowserIndex(index);
  if (!sequenceBrowser || sequenceBrowserIndex < 0)
  {
    vtkErrorMacro("GetNthSceneViewDataNode: Failed to get scene view sequence browser node." << proxyNode->GetID());
    return nullptr;
  }

  vtkMRMLSequenceNode* sequenceNode = sequenceBrowser->GetSequenceNode(proxyNode);
  if (!sequenceNode)
  {
    vtkErrorMacro("GetNthSceneViewDataNode: Failed to get sequence node for proxy node ID:" << proxyNode->GetID());
    return nullptr;
  }

  vtkMRMLVolumeNode* screenshotNode = this->GetSceneViewScreenshotProxyNode(sequenceBrowser);
  vtkMRMLSequenceNode* screenshotSequences = sequenceBrowser->GetSequenceNode(screenshotNode);
  if (!screenshotSequences || sequenceBrowserIndex < 0
      || sequenceBrowserIndex >= screenshotSequences->GetNumberOfDataNodes())
  {
    vtkErrorMacro("GetNthSceneViewDataNode: Invalid sequenceBrowserIndex.");
    return nullptr;
  }

  std::string value = screenshotSequences->GetNthIndexValue(sequenceBrowserIndex);
  return sequenceNode->GetDataNodeAtValue(value);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetNthNodeAttribute(vtkMRMLNode* proxyNode,
                                                         int index,
                                                         std::string attributeName,
                                                         std::string attributeValue)
{
  if (!proxyNode)
  {
    vtkErrorMacro("SetNthNodeAttribute: Failed to get proxy node.");
    return;
  }

  vtkMRMLNode* node = this->GetNthSceneViewDataNode(index, proxyNode);
  if (!node)
  {
    vtkErrorMacro("SetNthNodeAttribute: Failed to get data node.");
    return;
  }

  node->SetAttribute(attributeName.c_str(), attributeValue.c_str());
}

//---------------------------------------------------------------------------
std::string vtkSlicerSceneViewsModuleLogic::GetNthNodeAttribute(vtkMRMLNode* proxyNode,
                                                                int index,
                                                                std::string attributeName)
{
  if (!proxyNode)
  {
    vtkErrorMacro("GetNthNodeAttribute: Failed to get proxy node.");
    return "";
  }

  vtkMRMLNode* node = this->GetNthSceneViewDataNode(index, proxyNode);
  if (!node)
  {
    vtkErrorMacro("GetNthNodeAttribute: Failed to get data node.");
    return "";
  }

  const char* attributeValue = node->GetAttribute(attributeName.c_str());
  if (!attributeValue)
  {
    return "";
  }

  return attributeValue;
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::GetNumberOfSceneViews()
{
  if (!this->GetMRMLScene())
  {
    return 0;
  }

  int sceneViewCount = 0;
  std::vector<vtkMRMLNode*> nodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLSequenceBrowserNode", nodes);
  for (vtkMRMLNode* node : nodes)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
    if (!sequenceBrowserNode)
    {
      continue;
    }

    const char* attributeValue =
      sequenceBrowserNode->GetAttribute(vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeName());
    if (!attributeValue
        || strcmp(attributeValue, vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeValue()) != 0)
    {
      continue;
    }

    sceneViewCount += sequenceBrowserNode->GetNumberOfItems();
  }

  return sceneViewCount;
}

//---------------------------------------------------------------------------
bool vtkSlicerSceneViewsModuleLogic::IsSceneViewNode(vtkMRMLNode* node)
{
  if (!node)
  {
    return false;
  }

  const char* attributeValue = node->GetAttribute(vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeName());
  return attributeValue
         && strcmp(attributeValue, vtkSlicerSceneViewsModuleLogic::GetSceneViewNodeAttributeValue()) == 0;
}
