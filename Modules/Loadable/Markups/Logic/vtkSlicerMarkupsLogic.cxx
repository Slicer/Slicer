/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  ==============================================================================*/

// Markups includes
#include "vtkSlicerMarkupsLogic.h"
#include "vtkSlicerMarkupsWidget.h"

// Markups MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsAngleNode.h"
#include "vtkMRMLMarkupsClosedCurveNode.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsJsonStorageNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLMarkupsPlaneJsonStorageNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsROIDisplayNode.h"
#include "vtkMRMLMarkupsROIJsonStorageNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLTableStorageNode.h"

// Markups vtk widgets includes
#include "vtkSlicerAngleWidget.h"
#include "vtkSlicerCurveWidget.h"
#include "vtkSlicerLineWidget.h"
#include "vtkSlicerPlaneWidget.h"
#include "vtkSlicerPointsWidget.h"
#include "vtkSlicerROIWidget.h"

// Annotation MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"

// MRML includes
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLTableNode.h"

// vtkAddon includes
#include "vtkAddonMathUtilities.h"

// VTK includes
#include <vtkBitArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTable.h>

// STD includes
#include <cassert>
#include <list>

//----------------------------------------------------------------------------
class vtkSlicerMarkupsLogic::vtkInternal
{
public:

  void UpdatePlacementValidInSelectionNode()
    {
    if (!this->SelectionNode)
      {
      return;
      }
    bool activePlaceNodePlacementValid = false;
    if (this->ActiveMarkupsNode)
      {
      activePlaceNodePlacementValid = !this->ActiveMarkupsNode->GetControlPointPlacementComplete();
      }
    this->SelectionNode->SetActivePlaceNodePlacementValid(activePlaceNodePlacementValid);
    }

  // This keeps the elements that can be registered to a node type
  struct MarkupEntry
    {
    vtkSmartPointer<vtkSlicerMarkupsWidget> MarkupsWidget;
    vtkSmartPointer<vtkMRMLMarkupsNode> MarkupsNode;
    bool CreatePushButton;
    };

  std::map<std::string, std::string> MarkupsTypeStorageNodes;
  vtkMRMLSelectionNode* SelectionNode{ nullptr };

  vtkWeakPointer<vtkMRMLMarkupsNode> ActiveMarkupsNode;

  /// Keeps track of the registered nodes and corresponding widgets
  std::map<std::string, MarkupEntry> MarkupTypeToMarkupEntry;

  /// Keeps track of the order in which the markups were registered
  std::list<std::string> RegisteredMarkupsOrder;

  /// Counter used by GenerateUniqueColor for creating new colors from a color table.
  int UniqueColorNextColorTableIndex{ 0 };
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsLogic);

//----------------------------------------------------------------------------
// call back to be triggered when the default display node is changed, so that
// changes to it can be linked to a modified event on the logic
// \ingroup
class vtkSlicerMarkupsLogicCallback : public vtkCommand
{
public:
  static vtkSlicerMarkupsLogicCallback *New()
  { return new vtkSlicerMarkupsLogicCallback; }

  vtkSlicerMarkupsLogicCallback() = default;

  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*) override
  {
    if (event == vtkCommand::ModifiedEvent)
      {
      if (!this->markupsLogic)
        {
        return;
        }
      // trigger a modified event on the logic so that settings panel
      // observers can update
      this->markupsLogic->InvokeEvent(vtkCommand::ModifiedEvent);
      }
  }
  void SetLogic(vtkSlicerMarkupsLogic *logic)
  {
    this->markupsLogic = logic;
  }
  vtkSlicerMarkupsLogic * markupsLogic;
};

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::vtkSlicerMarkupsLogic()
{
  this->Internal = new vtkInternal();
  this->AutoCreateDisplayNodes = true;
  this->RegisterJsonStorageNodeForMarkupsType("ROI", "vtkMRMLMarkupsROIJsonStorageNode");
  this->RegisterJsonStorageNodeForMarkupsType("Plane", "vtkMRMLMarkupsPlaneJsonStorageNode");
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::~vtkSlicerMarkupsLogic()
{
  this->SetAndObserveSelectionNode(nullptr);
  this->Internal->MarkupsTypeStorageNodes.clear();
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ProcessMRMLNodesEvents(vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData)
{
  vtkDebugMacro("ProcessMRMLNodesEvents: Event " << event);

  vtkMRMLMarkupsDisplayNode *markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(caller);
  if (markupsDisplayNode)
    {
    if (event == vtkMRMLMarkupsDisplayNode::ResetToDefaultsEvent)
      {
      vtkDebugMacro("ProcessMRMLNodesEvents: calling SetDisplayNodeToDefaults");
      this->SetDisplayNodeToDefaults(markupsDisplayNode);
      }
    else if (event == vtkMRMLMarkupsDisplayNode::JumpToPointEvent)
      {
      int componentIndex = -1;
      int componentType = -1;
      int viewGroup = -1;
      vtkMRMLSliceNode* sliceNode = nullptr;
      if (callData != nullptr)
        {
        vtkMRMLInteractionEventData* eventData = reinterpret_cast<vtkMRMLInteractionEventData*>(callData);
        componentIndex = eventData->GetComponentIndex();
        componentType = eventData->GetComponentType();
        if (eventData->GetViewNode())
          {
          viewGroup = eventData->GetViewNode()->GetViewGroup();
          sliceNode = vtkMRMLSliceNode::SafeDownCast(eventData->GetViewNode());
          }
        }
      if (componentType == vtkMRMLMarkupsDisplayNode::ComponentControlPoint)
        {
        // Jump current slice node to the plane of the control point (do not center)
        if (sliceNode)
          {
          vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(markupsDisplayNode->GetDisplayableNode());
          if (markupsNode)
            {
            double worldPos[3] = { 0.0 };
            markupsNode->GetNthControlPointPositionWorld(componentIndex, worldPos);
            sliceNode->JumpSliceByOffsetting(worldPos[0], worldPos[1], worldPos[2]);
            }
          }
          // Jump centered in all other slices in the view group
          this->JumpSlicesToNthPointInMarkup(markupsDisplayNode->GetDisplayableNode()->GetID(), componentIndex,
            true /* centered */, viewGroup, sliceNode);
        }
      else if (callData != nullptr && (componentType == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle
        || componentType == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle
        || componentType == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle))
        {
        // Jump to the location of the current handle position.
        vtkMRMLInteractionEventData* eventData = reinterpret_cast<vtkMRMLInteractionEventData*>(callData);
        double position_World[3] = { 0.0, 0.0, 0.0 };
        eventData->GetWorldPosition(position_World);
        this->JumpSlicesToLocation(position_World[0], position_World[1], position_World[2], true /* centered */, viewGroup, sliceNode);
        }
      }
    }

  // Update all measurements if units changed.
  // This makes display format, precision, etc. changes reflected immediately.
  if (event == vtkMRMLSelectionNode::UnitModifiedEvent && this->GetMRMLScene())
    {
    // units modified, update all measurements
    std::vector<vtkMRMLNode*> nodes;
    this->GetMRMLScene()->GetNodesByClass("vtkMRMLMarkupsNode", nodes);
    for (vtkMRMLNode* node : nodes)
      {
      vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
      if (!markupsNode)
        {
        continue;
        }
      markupsNode->UpdateAllMeasurements();
      }
    }

  // Update the observer to the active place node.
  if (caller == this->Internal->SelectionNode && event == vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent && this->GetMRMLScene())
    {
    vtkMRMLMarkupsNode* activeMarkupsNode = nullptr;
    std::string activeMarkupsNodeID = this->GetActiveListID();
    if (!activeMarkupsNodeID.empty())
      {
      activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeMarkupsNodeID.c_str()));
      }
    if (this->Internal->ActiveMarkupsNode.GetPointer() != activeMarkupsNode)
      {
      // Active placement mode changed, add an observer to the markups node so that
      // we get notified about any control point number or state changes,
      // so that we can update the PlacementValid value in the selection node.
      vtkUnObserveMRMLNodeMacro(this->Internal->ActiveMarkupsNode);
      vtkNew<vtkIntArray> events;
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      events->InsertNextValue(vtkMRMLMarkupsNode::PointPositionDefinedEvent);
      events->InsertNextValue(vtkMRMLMarkupsNode::PointPositionUndefinedEvent);
      events->InsertNextValue(vtkMRMLMarkupsNode::PointPositionMissingEvent);
      events->InsertNextValue(vtkMRMLMarkupsNode::PointPositionNonMissingEvent);
      vtkObserveMRMLNodeEventsMacro(activeMarkupsNode, events.GetPointer());
      this->Internal->ActiveMarkupsNode = activeMarkupsNode;

      this->Internal->UpdatePlacementValidInSelectionNode();
      }
    }

  if (caller == this->Internal->ActiveMarkupsNode.GetPointer() && this->GetMRMLScene())
    {
    // Markup control points are placed, update the selection node to indicate if placement of more control points is allowed.
    this->Internal->UpdatePlacementValidInSelectionNode();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());

  vtkMRMLSelectionNode* selectionNode = nullptr;
  if (this->GetMRMLScene())
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
    }
  this->SetAndObserveSelectionNode(selectionNode);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ObserveMRMLScene()
{
  if (this->GetMRMLScene())
    {
    this->UpdatePlaceNodeClassNamesInSelectionNode();
    }
  this->Superclass::ObserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode failed: invalid scene");
    return;
    }
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode failed: invalid selection node");
    return;
    }

  for (const std::string& markupType : this->Internal->RegisteredMarkupsOrder)
    {
    this->Internal->MarkupTypeToMarkupEntry[markupType];
    auto markupEntryIt = this->Internal->MarkupTypeToMarkupEntry.find(markupType);
    if (markupEntryIt == this->Internal->MarkupTypeToMarkupEntry.end())
      {
      vtkWarningMacro("vtkSlicerMarkupsLogic::UpdatePlaceNodeClassNamesInSelectionNode failed to add " << markupType << " to selection node");
      continue;
      }
    const char* markupsClassName = markupEntryIt->second.MarkupsNode->GetClassName();
    if (selectionNode->PlaceNodeClassNameInList(markupsClassName) < 0)
      {
      vtkSmartPointer<vtkMRMLMarkupsNode> markupsNode = vtkSmartPointer<vtkMRMLMarkupsNode>::Take(
        vtkMRMLMarkupsNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass(markupsClassName)));
      if (!markupsNode)
        {
        vtkErrorMacro("vtkSlicerMarkupsLogic::ObserveMRMLScene: Failed to create markups node by class " << markupsClassName);
        continue;
        }
      selectionNode->AddNewPlaceNodeClassNameToList(markupsNode->GetClassName(), markupsNode->GetAddIcon(), markupsNode->GetMarkupType());
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAndObserveSelectionNode(vtkMRMLSelectionNode* selectionNode)
{
  vtkNew<vtkIntArray> selectionEvents;
  selectionEvents->InsertNextValue(vtkMRMLSelectionNode::UnitModifiedEvent);
  selectionEvents->InsertNextValue(vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->Internal->SelectionNode, selectionNode, selectionEvents.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != nullptr);

  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::RegisterNodes failed: invalid scene");
    return;
    }

  // Generic markups nodes
  scene->RegisterAbstractNodeClass("vtkMRMLMarkupsNode", "Markup");
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsJsonStorageNode>::New());

  // NOTE: the order of registration determines the order of the create push buttons in the GUI

  vtkNew<vtkMRMLMarkupsFiducialNode> fiducialNode;
  vtkNew<vtkSlicerPointsWidget> pointsWidget;
  this->RegisterMarkupsNode(fiducialNode, pointsWidget);
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialStorageNode>::New());

  vtkNew<vtkMRMLMarkupsLineNode> lineNode;
  vtkNew<vtkSlicerLineWidget> lineWidget;
  this->RegisterMarkupsNode(lineNode, lineWidget);

  vtkNew<vtkMRMLMarkupsAngleNode> angleNode;
  vtkNew<vtkSlicerAngleWidget> angleWidget;
  this->RegisterMarkupsNode(angleNode, angleWidget);

  vtkNew<vtkMRMLMarkupsCurveNode> curveNode;
  vtkNew<vtkSlicerCurveWidget> curveWidget;
  this->RegisterMarkupsNode(curveNode, curveWidget);

  vtkNew<vtkMRMLMarkupsClosedCurveNode> closedCurveNode;
  vtkNew<vtkSlicerCurveWidget> closedCurveWidget;
  this->RegisterMarkupsNode(closedCurveNode, closedCurveWidget);

  vtkNew<vtkMRMLMarkupsPlaneNode> planeNode;
  vtkNew<vtkSlicerPlaneWidget> planeWidget;
  this->RegisterMarkupsNode(planeNode, planeWidget);
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsPlaneDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsPlaneJsonStorageNode>::New());

  vtkNew<vtkMRMLMarkupsROINode> roiNode;
  vtkNew<vtkSlicerROIWidget> roiWidget;
  this->RegisterMarkupsNode(roiNode, roiWidget);
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsROIDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsROIJsonStorageNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != nullptr);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Have a markups display node");
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLMarkupsDisplayNode::ResetToDefaultsEvent);
    events->InsertNextValue(vtkMRMLMarkupsDisplayNode::JumpToPointEvent);
    vtkUnObserveMRMLNodeMacro(node);
    vtkObserveMRMLNodeEventsMacro(node, events.GetPointer());
    }
  // a node could have been added by a node selector's create new node method,
  // but make sure that the scene is not batch processing before responding to
  // the event
  if (!node->IsA("vtkMRMLMarkupsNode"))
    {
    return;
    }
  if (this->GetMRMLScene() &&
      (this->GetMRMLScene()->IsImporting() ||
       this->GetMRMLScene()->IsRestoring() ||
       this->GetMRMLScene()->IsBatchProcessing()))
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  if (markupsNode->GetDisplayNode() == nullptr && this->AutoCreateDisplayNodes)
    {
    // add a display node
    int modifyFlag = markupsNode->StartModify();
    std::string displayNodeID = this->AddNewDisplayNodeForMarkupsNode(markupsNode);
    markupsNode->EndModify(modifyFlag);
    vtkDebugMacro("Added a display node with id " << displayNodeID.c_str()
                  << " for markups node with id " << markupsNode->GetID());
    }
  // make it active for adding to via the mouse
  this->SetActiveList(markupsNode);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  // remove observer
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    vtkDebugMacro("OnMRMLSceneNodeRemoved: Have a markups display node");
    vtkUnObserveMRMLNodeMacro(node);
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetSelectionNodeID()
{
  std::string selectionNodeID = std::string("");

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("GetSelectionNodeID: no scene defined!");
    return selectionNodeID;
    }

  // try the application logic first
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (mrmlAppLogic)
    {
    vtkMRMLSelectionNode *selectionNode = mrmlAppLogic->GetSelectionNode();
    if (selectionNode)
      {
      char *id = selectionNode->GetID();
      if (id)
        {
        selectionNodeID = std::string(id);
        }
      }
    }
  else
    {
    // try a default string
    selectionNodeID = std::string("vtkMRMLSelectionNodeSingleton");
    // check if it's in the scene
    if (this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str()) == nullptr)
      {
      vtkErrorMacro("GetSelectionNodeID: no selection node in scene with id " << selectionNodeID);
      // reset it
      selectionNodeID = std::string("");
      }
    }
  return selectionNodeID;
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetActiveListID()
{
  std::string listID = std::string("");

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("GetActiveListID: no scene defined!");
    return listID;
    }

  // get the selection node
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("GetActiveListID: unable to get the selection node that governs active lists.");
    return listID;
    }

  const char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  // is there no active fiducial list?
  if (activePlaceNodeID == nullptr)
    {
    vtkDebugMacro("GetListID: no active place node");
    return listID;
    }

  listID = std::string(activePlaceNodeID);
  return listID;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetActiveListID(vtkMRMLMarkupsNode* markupsNode)
{
  this->SetActiveList(markupsNode);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetActiveList(vtkMRMLMarkupsNode *markupsNode)
{
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::SetActiveList: No selection node in the scene.");
    return;
    }

  if (markupsNode == nullptr)
    {
    // If fiducial node was placed then reset node ID and deactivate placement
    const char *activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    if (activePlaceNodeClassName && strcmp(activePlaceNodeClassName, "vtkMRMLMarkupsFiducialNode") == 0)
      {
      selectionNode->SetReferenceActivePlaceNodeID(nullptr);
      vtkSmartPointer<vtkCollection> interactionNodes = vtkSmartPointer<vtkCollection>::Take
        (this->GetMRMLScene()->GetNodesByClass("vtkMRMLInteractionNode"));
      for(int interactionNodeIndex = 0; interactionNodeIndex < interactionNodes->GetNumberOfItems(); ++interactionNodeIndex)
        {
        vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(interactionNodes->GetItemAsObject(interactionNodeIndex));
        if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
          {
          interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
          }
        }
      }
    return;
    }

  // check if need to update the current type of node that's being placed
  const char *activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  if (!activePlaceNodeClassName ||
      (activePlaceNodeClassName &&
       strcmp(activePlaceNodeClassName, markupsNode->GetClassName()) != 0))
    {
    // call the set reference to make sure the event is invoked
    selectionNode->SetReferenceActivePlaceNodeClassName(markupsNode->GetClassName());
    }
  // set this markup node active if it's not already
  const char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  if (!activePlaceNodeID ||
      (activePlaceNodeID && strcmp(activePlaceNodeID, markupsNode->GetID()) != 0))
    {
    selectionNode->SetReferenceActivePlaceNodeID(markupsNode->GetID());
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::AddNewDisplayNodeForMarkupsNode(vtkMRMLNode *mrmlNode)
{
  std::string id;
  if (!mrmlNode || !mrmlNode->GetScene())
    {
    vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: unable to add a markups display node!");
    return id;
    }

  // is there already a display node?
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(mrmlNode);
  if (displayableNode && displayableNode->GetDisplayNode() != nullptr)
    {
    return displayableNode->GetDisplayNodeID();
    }

  // create the display node
  displayableNode->CreateDefaultDisplayNodes();
  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(displayableNode->GetDisplayNode());
  if (!displayNode)
    {
    vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: error creating new display node");
    return id;
    }

  // set it from the defaults
  this->SetDisplayNodeToDefaults(displayNode);
  vtkDebugMacro("AddNewDisplayNodeForMarkupsNode: set display node to defaults");

  // get the node id to return
  id = std::string(displayNode->GetID());

  return id;
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::AddNewFiducialNode(const char *name, vtkMRMLScene *scene)
{
  vtkMRMLMarkupsNode* markupsNode = this->AddNewMarkupsNode("vtkMRMLMarkupsFiducialNode", name ? name : "", scene);
  if (!markupsNode)
    {
    return "";
    }

  // If adding to this scene (could be adding to a scene view during conversion)
  // make it active so mouse mode tool bar clicks will add new fids to this list.
  if (scene == nullptr || scene == this->GetMRMLScene())
    {
    this->SetActiveList(markupsNode);
    }

  const char* nodeId = markupsNode->GetID();
  return (nodeId ? nodeId : "");
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerMarkupsLogic::AddNewMarkupsNode(
  std::string className, std::string nodeName/*=std::string()*/, vtkMRMLScene* scene/*=nullptr*/)
{
  if (!scene)
    {
    scene = this->GetMRMLScene();
    }
  if (!scene)
    {
    vtkErrorMacro("AddNewMarkupsNode: no scene to add a markups node to");
    return nullptr;
    }

  vtkSmartPointer<vtkMRMLNode> node = vtkSmartPointer<vtkMRMLNode>::Take(
    scene->CreateNodeByClass(className.c_str()));
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("AddNewMarkupsNode: failed to instantiate class " << className);
    return nullptr;
    }

  // Set node name
  if (nodeName.empty())
    {
    nodeName = scene->GenerateUniqueName(markupsNode->GetDefaultNodeNamePrefix());
    }
  markupsNode->SetName(nodeName.c_str());

  // Add the new node and display node to the scene
  scene->AddNode(markupsNode);
  markupsNode->CreateDefaultDisplayNodes();

  // Special case: for ROI nodes, we create alternating colors.
  // If it turns out to be a well-liked feature then we can enable this for all markup types
  if (className == "vtkMRMLMarkupsROINode")
    {
    markupsNode->GetDisplayNode()->SetSelectedColor(this->GenerateUniqueColor().GetData());
    }

  return markupsNode;
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::AddControlPoint(double r, double a, double s)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("AddControlPoint: no scene defined!");
    return -1;
    }

  // get the active list id
  std::string listID = this->GetActiveListID();

  // is there no active point list?
  if (listID.size() == 0)
    {
    vtkDebugMacro("AddControlPoint: no point list is active, adding one first!");
    std::string newListID = this->AddNewFiducialNode();
    if (newListID.size() == 0)
      {
      vtkErrorMacro("AddControlPoint: failed to add a new point list to the scene.");
      return -1;
      }
    // try to get the id again
    listID = this->GetActiveListID();
    if (listID.size() == 0)
      {
      vtkErrorMacro("AddControlPoint: failed to create a new point list to add to!");
      return -1;
      }
    }

  // get the active list
  vtkMRMLNode *listNode = this->GetMRMLScene()->GetNodeByID(listID.c_str());
  if (!listNode)
    {
    vtkErrorMacro("AddControlPoint: failed to get the active point list with id " << listID);
    return -1;
    }
  vtkMRMLMarkupsFiducialNode *fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(listNode);
  if (!fiducialNode)
    {
    vtkErrorMacro("AddControlPoint: active list is not a point list: " << listNode->GetClassName());
    return -1;
    }
  vtkDebugMacro("AddControlPoint: adding a control point to the list " << listID);
  // add a control point to the active point list
  return fiducialNode->AddControlPoint(vtkVector3d(r,a,s), std::string());
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToLocation(double x, double y, double z, bool centered,
                                                 int viewGroup /* =-1 */, vtkMRMLSliceNode* exclude /* =nullptr */)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("JumpSlicesToLocation: No scene defined");
    return;
    }

  // save the whole state as iterating over all slice nodes
  int jumpMode = centered ? vtkMRMLSliceNode::CenteredJumpSlice: vtkMRMLSliceNode::OffsetJumpSlice;
  vtkMRMLSliceNode::JumpAllSlices(this->GetMRMLScene(), x, y, z, jumpMode, viewGroup, exclude);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToNthPointInMarkup(const char *id, int n, bool centered,
                                                         int viewGroup /* =-1 */, vtkMRMLSliceNode* exclude /* =nullptr */)
{
  if (!id)
    {
    return;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("JumpSlicesToNthPointInMarkup: No scene defined");
    return;
    }
  // get the markups node
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(id);
  if (mrmlNode == nullptr)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (markupNode)
    {
    double point[4];
    markupNode->GetNthControlPointPositionWorld(n, point);
    this->JumpSlicesToLocation(point[0], point[1], point[2], centered, viewGroup, exclude);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::FocusCamerasOnNthPointInMarkup(const char *id, int n)
{

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("FocusCamerasOnNthPointInMarkup: No scene defined");
    return;
    }

  std::vector<vtkMRMLNode *> cameraNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  vtkMRMLNode *node;
  for (unsigned int i = 0; i < cameraNodes.size(); ++i)
    {
    node = cameraNodes[i];
    if (node)
      {
      this->FocusCameraOnNthPointInMarkup(node->GetID(), id, n);
      }
    }
}
//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::FocusCameraOnNthPointInMarkup(
  const char *cameraNodeID, const char *markupNodeID, int n)
{
  if (!cameraNodeID || !markupNodeID)
    {
    return;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: No scene defined");
    return;
    }

  // get the camera node
  vtkMRMLNode *mrmlNode1 = this->GetMRMLScene()->GetNodeByID(cameraNodeID);
  if (mrmlNode1 == nullptr)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find node with id " << cameraNodeID);
    return;
    }
  vtkMRMLCameraNode *cameraNode = vtkMRMLCameraNode::SafeDownCast(mrmlNode1);
  if (!cameraNode)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find camera with id " << cameraNodeID);
    return;
    }

  // get the markups node
  vtkMRMLNode *mrmlNode2 = this->GetMRMLScene()->GetNodeByID(markupNodeID);
  if (mrmlNode2 == nullptr)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find node with id " << markupNodeID);
    return;
    }
  vtkMRMLMarkupsNode *markup = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode2);
  if (!markup)
    {
    vtkErrorMacro("FocusCameraOnNthPointInMarkup: unable to find markup with id " << markupNodeID);
    return;
    }

  double point[4];
  markup->GetNthControlPointPositionWorld(n, point);
  // and focus the camera there
  cameraNode->SetFocalPoint(point[0], point[1], point[2]);
}

//---------------------------------------------------------------------------
char* vtkSlicerMarkupsLogic::LoadMarkups(const char* fileName, const char* nodeName/*=nullptr*/, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::LoadMarkups failed: invalid fileName");
    return nullptr;
    }

  // get file extension
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fileName);
  if( extension.empty() )
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::LoadMarkups failed: no file extension specified: " << fileName);
    return nullptr;
    }

  //
  if (extension == std::string(".json"))
    {
    return this->LoadMarkupsFromJson(fileName, nodeName, userMessages);
    }
  else if (extension == std::string(".fcsv"))
    {
    return this->LoadMarkupsFromFcsv(fileName, nodeName, userMessages);
    }
  else
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::LoadMarkups failed: unrecognized file extension in " << fileName);
    return nullptr;
    }
}

//---------------------------------------------------------------------------
char* vtkSlicerMarkupsLogic::LoadMarkupsFiducials(const char* fileName, const char* fidsName/*=nullptr*/, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  return this->LoadMarkups(fileName, fidsName, userMessages);
}

//---------------------------------------------------------------------------
char* vtkSlicerMarkupsLogic::LoadMarkupsFromJson(const char* fileName, const char* nodeName/*=nullptr*/, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkups: null file or markups class name, cannot load");
    return nullptr;
    }

  vtkDebugMacro("LoadMarkups, file name = " << fileName << ", nodeName = " << (nodeName ? nodeName : "null"));

  std::vector<std::string> markupsTypes;
  // make a storage node and fiducial node and set the file name
  vtkMRMLMarkupsJsonStorageNode* tempStorageNode = vtkMRMLMarkupsJsonStorageNode::SafeDownCast(
    this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsJsonStorageNode"));
  if (!tempStorageNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups storage node by class vtkMRMLMarkupsJsonStorageNode");
    return nullptr;
    }
  tempStorageNode->GetMarkupsTypesInFile(fileName, markupsTypes);
  if (userMessages)
    {
    userMessages->AddMessages(tempStorageNode->GetUserMessages());
    }
  this->GetMRMLScene()->RemoveNode(tempStorageNode);

  vtkMRMLMarkupsNode* importedMarkupsNode = nullptr;
  for(unsigned int markupsIndex = 0; markupsIndex < markupsTypes.size(); ++markupsIndex)
    {
    std::string markupsType = markupsTypes[markupsIndex];
    vtkMRMLMarkupsJsonStorageNode* storageNode = this->AddNewJsonStorageNodeForMarkupsType(markupsType);
    if (!storageNode)
      {
      vtkErrorMacro("LoadMarkupsFromJson: Could not create storage node for markup type: " << markupsType);
      continue;
      }

    if (markupsTypes.size() == 1)
      {
      // If a single markups node is stored in this file then save the filename in the storage node.
      // (If multiple markups node are loaded from the same file then the filename should not be saved
      // because that would make multiple nodes overwrite the same file during saving.)
      storageNode->SetFileName(fileName);
      }

    vtkMRMLMarkupsNode* markupsNode = storageNode->AddNewMarkupsNodeFromFile(fileName, nodeName, markupsIndex);
    if (!importedMarkupsNode)
      {
      importedMarkupsNode = markupsNode;
      }
    if (!markupsNode)
      {
      if (userMessages)
        {
        userMessages->AddMessages(storageNode->GetUserMessages());
        }
      this->GetMRMLScene()->RemoveNode(storageNode);
      }
    }

  if (!importedMarkupsNode)
    {
    return nullptr;
    }

  return importedMarkupsNode->GetID();
}

//---------------------------------------------------------------------------
char * vtkSlicerMarkupsLogic::LoadMarkupsFromFcsv(const char* fileName, const char* nodeName/*=nullptr*/, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkups: null file or markups class name, cannot load");
    return nullptr;
    }

  vtkDebugMacro("LoadMarkups, file name = " << fileName << ", nodeName = " << (nodeName ? nodeName : "null"));
  // make a storage node and fiducial node and set the file name
  vtkMRMLStorageNode* storageNode = vtkMRMLStorageNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialStorageNode"));
  if (!storageNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups storage node by class vtkMRMLMarkupsFiducialNode");
    return nullptr;
    }

  std::string newNodeName;
  if (nodeName && strlen(nodeName)>0)
    {
    newNodeName = nodeName;
    }
  else
    {
    newNodeName = this->GetMRMLScene()->GetUniqueNameByString(storageNode->GetFileNameWithoutExtension(fileName).c_str());
    }
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode", newNodeName));
  if (!markupsNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups node by class vtkMRMLMarkupsFiducialNode");
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    this->GetMRMLScene()->RemoveNode(storageNode);
    return nullptr;
    }

  storageNode->SetFileName(fileName);
  // add the nodes to the scene and set up the observation on the storage node
  markupsNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // read the file
  char* nodeID = nullptr;
  if (storageNode->ReadData(markupsNode))
    {
    nodeID = markupsNode->GetID();
    }
  else
    {
    if (userMessages)
      {
      userMessages->AddMessages(storageNode->GetUserMessages());
      }
    this->GetMRMLScene()->RemoveNode(storageNode);
    this->GetMRMLScene()->RemoveNode(markupsNode);
    }

  return nodeID;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllControlPointsVisibility(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllControlPointsVisibility: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointVisibility(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ToggleAllControlPointsVisibility(vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllControlPointsVisibility: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointVisibility(i, !(node->GetNthControlPointVisibility(i)));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllControlPointsLocked(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllControlPointsLocked: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointLocked(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ToggleAllControlPointsLocked(vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllControlPointsLocked: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointLocked(i, !(node->GetNthControlPointLocked(i)));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllControlPointsSelected(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllControlPointsSelected: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointSelected(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ToggleAllControlPointsSelected(vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllControlPointsSelected: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointSelected(i, !(node->GetNthControlPointSelected(i)));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::CopyBasicDisplayProperties(vtkMRMLMarkupsDisplayNode *sourceDisplayNode,
                                                       vtkMRMLMarkupsDisplayNode *targetDisplayNode)
{
  if (!sourceDisplayNode || !targetDisplayNode)
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::CopyBasicDisplayProperties failed: invalid input nodes");
    return;
    }
  MRMLNodeModifyBlocker blocker(targetDisplayNode);
  targetDisplayNode->SetSnapMode(sourceDisplayNode->GetSnapMode());
  targetDisplayNode->SetFillVisibility(sourceDisplayNode->GetFillVisibility());
  targetDisplayNode->SetOutlineVisibility(sourceDisplayNode->GetOutlineVisibility());
  targetDisplayNode->SetFillOpacity(sourceDisplayNode->GetFillOpacity());
  targetDisplayNode->SetOutlineOpacity(sourceDisplayNode->GetOutlineOpacity());
  targetDisplayNode->SetTextScale(sourceDisplayNode->GetTextScale());

  targetDisplayNode->SetGlyphType(sourceDisplayNode->GetGlyphType());
  targetDisplayNode->SetGlyphScale(sourceDisplayNode->GetGlyphScale());
  targetDisplayNode->SetGlyphSize(sourceDisplayNode->GetGlyphSize());
  targetDisplayNode->SetUseGlyphScale(sourceDisplayNode->GetUseGlyphScale());

  targetDisplayNode->SetSliceProjection(sourceDisplayNode->GetSliceProjection());
  targetDisplayNode->SetSliceProjectionUseFiducialColor(sourceDisplayNode->GetSliceProjectionUseFiducialColor());
  targetDisplayNode->SetSliceProjectionOutlinedBehindSlicePlane(sourceDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());
  targetDisplayNode->SetSliceProjectionColor(sourceDisplayNode->GetSliceProjectionColor());
  targetDisplayNode->SetSliceProjectionOpacity(sourceDisplayNode->GetSliceProjectionOpacity());

  targetDisplayNode->SetCurveLineSizeMode(sourceDisplayNode->GetCurveLineSizeMode());
  targetDisplayNode->SetLineThickness(sourceDisplayNode->GetLineThickness());
  targetDisplayNode->SetLineDiameter(sourceDisplayNode->GetLineDiameter());

  targetDisplayNode->SetLineColorFadingStart(sourceDisplayNode->GetLineColorFadingStart());
  targetDisplayNode->SetLineColorFadingEnd(sourceDisplayNode->GetLineColorFadingEnd());
  targetDisplayNode->SetLineColorFadingSaturation(sourceDisplayNode->GetLineColorFadingSaturation());
  targetDisplayNode->SetLineColorFadingHueOffset(sourceDisplayNode->GetLineColorFadingHueOffset());

  targetDisplayNode->SetOccludedVisibility(sourceDisplayNode->GetOccludedVisibility());
  targetDisplayNode->SetOccludedOpacity(sourceDisplayNode->GetOccludedOpacity());
  std::string textPropertyStr = vtkMRMLDisplayNode::GetTextPropertyAsString(sourceDisplayNode->GetTextProperty());
  vtkMRMLMarkupsDisplayNode::UpdateTextPropertyFromString(textPropertyStr, targetDisplayNode->GetTextProperty());

  targetDisplayNode->SetSelectedColor(sourceDisplayNode->GetSelectedColor());
  targetDisplayNode->SetColor(sourceDisplayNode->GetColor());
  targetDisplayNode->SetActiveColor(sourceDisplayNode->GetActiveColor());
  targetDisplayNode->SetOpacity(sourceDisplayNode->GetOpacity());

  targetDisplayNode->SetInteractionHandleScale(sourceDisplayNode->GetInteractionHandleScale());
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDisplayNodeToDefaults(vtkMRMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }
  if (!this->GetMRMLScene())
    {
    return;
    }
  vtkMRMLMarkupsDisplayNode* defaultNode = this->GetDefaultMarkupsDisplayNode();
  if (!defaultNode)
    {
    return;
    }
  this->CopyBasicDisplayProperties(defaultNode, displayNode);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDisplayDefaultsFromNode(vtkMRMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }
  if (!this->GetMRMLScene())
    {
    return;
    }
  vtkMRMLMarkupsDisplayNode* defaultNode = this->GetDefaultMarkupsDisplayNode();
  if (!defaultNode)
    {
    return;
    }
  this->CopyBasicDisplayProperties(displayNode, defaultNode);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::MoveNthControlPointToNewListAtIndex(int n, vtkMRMLMarkupsNode *markupsNode,
                                                                vtkMRMLMarkupsNode *newMarkupsNode, int newIndex)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("MoveNthControlPointToNewListAtIndex: at least one of the markup list nodes are null!");
    return false;
    }

  if (!markupsNode->ControlPointExists(n))
    {
    vtkErrorMacro("MoveNthControlPointToNewListAtIndex: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfControlPoints());
    return false;
    }

  // get the control point
  vtkMRMLMarkupsNode::ControlPoint *newControlPoint = new vtkMRMLMarkupsNode::ControlPoint;
  *newControlPoint = *markupsNode->GetNthControlPoint(n);

  // add it to the destination list
  bool insertVal = newMarkupsNode->InsertControlPoint(newControlPoint, newIndex);
  if (!insertVal)
    {
    vtkErrorMacro("MoveNthControlPointToNewListAtIndex: failed to insert new control point at " << newIndex <<
                  ", control point is still on source list.");
    return false;
    }

  // remove it from the source list
  markupsNode->RemoveNthControlPoint(n);

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::CopyNthControlPointToNewList(int n, vtkMRMLMarkupsNode *markupsNode,
                                                         vtkMRMLMarkupsNode *newMarkupsNode)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("CopyNthControlPointToNewList: at least one of the markup list nodes are null!");
    return false;
    }

  if (!markupsNode->ControlPointExists(n))
    {
    vtkErrorMacro("CopyNthControlPointToNewList: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfControlPoints());
    return false;
    }

  // get the control point
  vtkMRMLMarkupsNode::ControlPoint *newControlPoint = new vtkMRMLMarkupsNode::ControlPoint;
  *newControlPoint = *markupsNode->GetNthControlPoint(n);

  // add it to the destination list
  newMarkupsNode->AddControlPoint(newControlPoint, false);

  return true;
}


//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ConvertAnnotationFiducialsToMarkups()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  // there can be annotation fiducials in the main scene, as well as in scene
  // view scenes, so collect all of those in one vector to iterate over
  std::vector<vtkMRMLScene *> scenes;
  scenes.push_back(this->GetMRMLScene());

  vtkCollection *sceneViews = this->GetMRMLScene()->GetNodesByClass("vtkMRMLSceneViewNode");
  int numberOfSceneViews = sceneViews->GetNumberOfItems();
  for (int n = 0; n < numberOfSceneViews; ++n)
    {
    vtkMRMLSceneViewNode *sceneView =
      vtkMRMLSceneViewNode::SafeDownCast(sceneViews->GetItemAsObject(n));
    if (sceneView && sceneView->GetStoredScene())
      {
      scenes.push_back(sceneView->GetStoredScene());
      }
    }

  vtkDebugMacro("ConvertAnnotationFiducialsToMarkups: Have " << scenes.size()
                << " scenes to check for annotation fiducial hierarchies");

  // now iterate over this scene and the scene view scenes to get out the
  // annotation fiducials that need to be converted
  for (unsigned int s = 0; s < scenes.size(); ++s)
    {
    vtkMRMLScene *scene = scenes[s];

    vtkCollection *annotationFiducials = scene->GetNodesByClass("vtkMRMLAnnotationFiducialNode");
    int numberOfAnnotationFids = annotationFiducials->GetNumberOfItems();

    if (numberOfAnnotationFids == 0)
      {
      annotationFiducials->Delete();
      continue;
      }


    // go through all the annotation fiducials and collect their hierarchies
    vtkStringArray *hierarchyNodeIDs = vtkStringArray::New();

    for (int n = 0; n < numberOfAnnotationFids; ++n)
      {
      vtkMRMLNode *mrmlNode =
        vtkMRMLNode::SafeDownCast(annotationFiducials->GetItemAsObject(n));
      if (!mrmlNode)
        {
        continue;
        }
      vtkMRMLHierarchyNode *oneToOneHierarchyNode =
        vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlNode->GetScene(),
                                                         mrmlNode->GetID());
      if (!oneToOneHierarchyNode)
        {
        continue;
        }
      char * parentNodeID = oneToOneHierarchyNode->GetParentNodeID();
      // is it not already in the list of annotation hierarchy node ids?
      vtkIdType id = hierarchyNodeIDs->LookupValue(parentNodeID);
      if (id == -1)
        {
        vtkDebugMacro("Found unique annotation hierarchy node, id = " << parentNodeID);
        hierarchyNodeIDs->InsertNextValue(parentNodeID);
        }
      }

    annotationFiducials->RemoveAllItems();
    annotationFiducials->Delete();

    if (hierarchyNodeIDs->GetNumberOfValues() == 0)
      {
      hierarchyNodeIDs->Delete();
      return;
      }
    else
      {
      vtkDebugMacro("Converting " << hierarchyNodeIDs->GetNumberOfValues()
                    << " annotation hierarchies to markup lists");
      }
    // now iterate over the hierarchies that have fiducials in them and convert
    // them to markups lists
    for (int i = 0; i < hierarchyNodeIDs->GetNumberOfValues(); ++i)
      {
      vtkMRMLNode *mrmlNode = nullptr;
      vtkMRMLHierarchyNode *hierarchyNode = nullptr;
      mrmlNode = scene->GetNodeByID(hierarchyNodeIDs->GetValue(i));
      if (!mrmlNode)
        {
        continue;
        }

      hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(mrmlNode);
      if (!hierarchyNode)
        {
        continue;
        }

      // create a markups fiducial list with this name
      std::string markupsListID = this->AddNewFiducialNode(hierarchyNode->GetName(), scene);
      vtkMRMLMarkupsFiducialNode *markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetNodeByID(markupsListID.c_str()));
      if (!markupsNode)
        {
        continue;
        }
      // now get the fiducials in this annotation hierarchy
      vtkCollection *children = vtkCollection::New();
      hierarchyNode->GetAssociatedChildrenNodes(children, "vtkMRMLAnnotationFiducialNode");
      vtkDebugMacro("Found " << children->GetNumberOfItems() << " annot fids in this hierarchy");
      for (int c = 0; c < children->GetNumberOfItems(); ++c)
        {
        vtkMRMLAnnotationFiducialNode *annotNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(children->GetItemAsObject(c));
        if (!annotNode)
          {
          continue;
          }
        double coord[3];
        annotNode->GetFiducialCoordinates(coord);
        int fidIndex = markupsNode->AddControlPoint(vtkVector3d(coord), std::string(annotNode->GetName()));
        vtkDebugMacro("Added a control point at index " << fidIndex);
        char *desc = annotNode->GetDescription();
        if (desc)
          {
          markupsNode->SetNthControlPointDescription(fidIndex,std::string(desc));
          }
        markupsNode->SetNthControlPointSelected(fidIndex, annotNode->GetSelected());
        markupsNode->SetNthControlPointVisibility(fidIndex,
                                                  annotNode->GetDisplayVisibility());
        markupsNode->SetNthControlPointLocked(fidIndex, annotNode->GetLocked());
        const char *assocNodeID = annotNode->GetAttribute("AssociatedNodeID");
        if (assocNodeID)
          {
          markupsNode->SetNthControlPointAssociatedNodeID(fidIndex, assocNodeID);
          }

        // get the display nodes
        vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = annotNode->GetAnnotationPointDisplayNode();
        vtkMRMLAnnotationTextDisplayNode *textDisplayNode = annotNode->GetAnnotationTextDisplayNode();

        if (c == 0)
          {
          // use the first display node to get display settings
          vtkMRMLMarkupsDisplayNode *markupDisplayNode = markupsNode->GetMarkupsDisplayNode();
          if (!markupDisplayNode || !pointDisplayNode || !textDisplayNode)
            {
            continue;
            }
          markupDisplayNode->SetColor(pointDisplayNode->GetColor());
          markupDisplayNode->SetSelectedColor(pointDisplayNode->GetSelectedColor());
          markupDisplayNode->SetGlyphScale(pointDisplayNode->GetGlyphScale());
          markupDisplayNode->SetTextScale(textDisplayNode->GetTextScale());
          markupDisplayNode->SetOpacity(pointDisplayNode->GetOpacity());
          markupDisplayNode->SetPower(pointDisplayNode->GetPower());
          markupDisplayNode->SetAmbient(pointDisplayNode->GetAmbient());
          markupDisplayNode->SetDiffuse(pointDisplayNode->GetDiffuse());
          markupDisplayNode->SetSpecular(pointDisplayNode->GetSpecular());
          markupDisplayNode->SetSliceProjection(pointDisplayNode->GetSliceProjection());
          markupDisplayNode->SetSliceProjectionColor(pointDisplayNode->GetProjectedColor());
          markupDisplayNode->SetSliceProjectionOpacity(pointDisplayNode->GetProjectedOpacity());
          }
        //
        // clean up the no longer needed annotation nodes
        //
        // remove the 1:1 hierarchy node
        vtkMRMLHierarchyNode *oneToOneHierarchyNode =
          vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(annotNode->GetScene(), annotNode->GetID());
        if (oneToOneHierarchyNode)
          {
          scene->RemoveNode(oneToOneHierarchyNode);
          }

        // remove the display nodes
        if (pointDisplayNode)
          {
          scene->RemoveNode(pointDisplayNode);
          }
        if (textDisplayNode)
          {
          scene->RemoveNode(textDisplayNode);
          }
        // is there a storage node?
        vtkMRMLStorageNode *storageNode = annotNode->GetStorageNode();
        if (storageNode)
          {
          scene->RemoveNode(storageNode);
          }
        // now remove the annotation node
        scene->RemoveNode(annotNode);
        }
      children->RemoveAllItems();
      children->Delete();
      }
    hierarchyNodeIDs->Delete();
    } // end of looping over the scene
  sceneViews->RemoveAllItems();
  sceneViews->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RenameAllControlPointsFromCurrentFormat(vtkMRMLMarkupsNode *markupsNode)
{
  if (!markupsNode)
    {
    return;
    }

  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  // get the format string with the list name replaced
  std::string formatString = markupsNode->ReplaceListNameInControlPointLabelFormat();
  bool numberInFormat = false;
  std::vector<char> buffVector(vtkMRMLMarkupsFiducialStorageNode::GetMaximumLineLength());
  char* buff = &(buffVector[0]);
  if (formatString.find("%d") != std::string::npos ||
      formatString.find("%g") != std::string::npos ||
      formatString.find("%f") != std::string::npos)
    {
    numberInFormat = true;
    }
  for (int n = 0; n < numberOfControlPoints; ++n)
    {
    std::string oldLabel = markupsNode->GetNthControlPointLabel(n);
    std::string oldNumber;
    if (numberInFormat)
      {
      // extract any number from the old label
      // is there more than one number in the old label?
      // - find the start of the first number
      std::string numbers = std::string("0123456789.");
      size_t firstNumber = oldLabel.find_first_of(numbers);
      size_t secondNumber = std::string::npos;
      size_t endOfFirstNumber = std::string::npos;
      size_t keepNumberStart = std::string::npos;
      size_t keepNumberEnd = std::string::npos;
      if (firstNumber != std::string::npos)
        {
        endOfFirstNumber = oldLabel.find_first_not_of(numbers, firstNumber);
        secondNumber = oldLabel.find_first_of(numbers, endOfFirstNumber);
        }
      if (secondNumber != std::string::npos)
        {
        vtkWarningMacro("RenameAllControlPointsFromCurrentFormat: more than one number in markup " << n << ", keeping second one: " << oldLabel.c_str());
        keepNumberStart = secondNumber;
        keepNumberEnd = oldLabel.find_first_not_of(numbers, keepNumberStart);
        }
      else
        {
        // use the first number
        keepNumberStart = firstNumber;
        keepNumberEnd = endOfFirstNumber;
        }
      if (keepNumberStart != std::string::npos)
        {
        oldNumber = oldLabel.substr(keepNumberStart, keepNumberEnd - keepNumberStart);
        if (formatString.find("%d") != std::string::npos)
          {
          // integer
          sprintf(buff,formatString.c_str(),atoi(oldNumber.c_str()));
          }
        else
          {
          // float
          sprintf(buff,formatString.c_str(),atof(oldNumber.c_str()));
          }
        }
      else
        {
        // no number found, use n
        sprintf(buff,formatString.c_str(),n);
        }
      markupsNode->SetNthControlPointLabel(n, std::string(buff));
      }
    else
      {
      // no number in the format, so just rename it
      markupsNode->SetNthControlPointLabel(n, formatString);
      }
    }
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::StartPlaceMode(bool persistent, vtkMRMLInteractionNode* interactionNode)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("StartPlaceMode: no scene");
    return false;
    }

  // set up to place markups fiducials
  vtkMRMLSelectionNode *selectionNode =
    vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  if (!selectionNode)
    {
    vtkErrorMacro ("StartPlaceMode: No selection node in the scene." );
    return false;
    }
  selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode");

  // now go into place mode with the persistece flag set
  if (!interactionNode)
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  if (!interactionNode)
    {
    vtkErrorMacro ("StartPlaceMode: No interaction node in the scene." );
    return false;
    }

  interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
  interactionNode->SetPlaceModePersistence(persistent ? 1 : 0);

  if (interactionNode->GetCurrentInteractionMode()
      != vtkMRMLInteractionNode::Place)
    {
    vtkErrorMacro("StartPlaceMode: Could not set place mode! "
                  << "Tried to set the interaction mode to "
                  << vtkMRMLInteractionNode::Place
                  << ", but it's now "
                  << interactionNode->GetCurrentInteractionMode());
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNode()
{
  if (!this->GetMRMLScene())
    {
    return nullptr;
    }
  vtkMRMLMarkupsDisplayNode* defaultNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(
    this->GetMRMLScene()->GetDefaultNodeByClass("vtkMRMLMarkupsDisplayNode"));
  if (defaultNode)
    {
    return defaultNode;
    }
  vtkSmartPointer<vtkMRMLMarkupsDisplayNode> newDefaultNode =
    vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::Take(vtkMRMLMarkupsDisplayNode::SafeDownCast(
                                                       this->GetMRMLScene()->CreateNodeByClass("vtkMRMLMarkupsDisplayNode")));
  if (!newDefaultNode)
    {
    return nullptr;
    }
  this->GetMRMLScene()->AddDefaultNode(newDefaultNode);
  return newDefaultNode;
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetClosedCurveSurfaceArea(vtkMRMLMarkupsClosedCurveNode* curveNode,
                                                        vtkPolyData* inputSurface /*=nullptr*/, bool projectWarp /*=true*/)
{
  return vtkMRMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(curveNode, inputSurface, projectWarp);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::FitSurfaceProjectWarp(vtkPoints* curvePoints,
  vtkPolyData* surface, double radiusScalingFactor/*=1.0*/, vtkIdType numberOfInternalGridPoints/*=225*/)
{
  return vtkMRMLMarkupsClosedCurveNode::FitSurfaceProjectWarp(curvePoints, surface, radiusScalingFactor, numberOfInternalGridPoints);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds/*nullptr*/)
{
  return vtkMRMLMarkupsClosedCurveNode::IsPolygonClockwise(points, pointIds);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor/*=1.0*/)
{
  return vtkMRMLMarkupsClosedCurveNode::FitSurfaceDiskWarp(curvePoints, surface, radiusScalingFactor);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::GetBestFitPlane(vtkMRMLMarkupsNode* curveNode, vtkPlane* plane)
{
  if (!curveNode || !plane)
    {
    return false;
    }
  vtkPoints* curvePointsWorld = curveNode->GetCurvePointsWorld();
  if (curvePointsWorld == nullptr || curvePointsWorld->GetNumberOfPoints() < 3)
    {
    // not enough points for computing a plane
    return false;
    }
  return vtkAddonMathUtilities::FitPlaneToPoints(curvePointsWorld, plane);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RegisterJsonStorageNodeForMarkupsType(std::string markupsType, std::string storageNodeClassName)
{
  this->Internal->MarkupsTypeStorageNodes[markupsType] = storageNodeClassName;
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetJsonStorageNodeClassNameForMarkupsType(std::string markupsType)
{
  auto markupsStorageNodeIt = this->Internal->MarkupsTypeStorageNodes.find(markupsType);
  if (markupsStorageNodeIt == this->Internal->MarkupsTypeStorageNodes.end())
    {
    return "vtkMRMLMarkupsJsonStorageNode";
    }
  return markupsStorageNodeIt->second;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode* vtkSlicerMarkupsLogic::AddNewJsonStorageNodeForMarkupsType(std::string markupsType)
{
  return vtkMRMLMarkupsJsonStorageNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass(this->GetJsonStorageNodeClassNameForMarkupsType(markupsType)));
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RegisterMarkupsNode(vtkMRMLMarkupsNode* markupsNode,
                                                vtkSlicerMarkupsWidget* markupsWidget,
                                                bool createPushButton)
{
  // Check for nullptr
  if (markupsNode == nullptr)
    {
    vtkErrorMacro("RegisterMarkupsNode failed: Invalid node.");
    return;
    }

  // Register node class, if has not been registered already.
  // This is just a convenience function so that markups node registration can be
  // accomplished with a single registration method.
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RegisterMarkupsNode failed: Invalid scene.");
    return;
    }
  if (!this->GetMRMLScene()->IsNodeClassRegistered(markupsNode->GetClassName()))
    {
    this->GetMRMLScene()->RegisterNodeClass(markupsNode);
    }

  // Check for nullptr
  if (markupsWidget == nullptr)
    {
    vtkErrorMacro("RegisterMarkup: Invalid widget.");
    return;
    }

  // Check that the class is not already registered
  if (this->GetNodeByMarkupsType(markupsNode->GetMarkupType()))
    {
    vtkWarningMacro("RegisterMarkup: Markups node " << markupsNode->GetMarkupType() << " is already registered.");
    return;
    }

  vtkSlicerMarkupsLogic::vtkInternal::MarkupEntry markup;
  markup.MarkupsWidget = markupsWidget;
  markup.MarkupsNode = markupsNode;
  markup.CreatePushButton = createPushButton;

  // Register the markup internally
  this->Internal->MarkupTypeToMarkupEntry[markupsNode->GetMarkupType()] = markup;
  this->Internal->RegisteredMarkupsOrder.push_back(markupsNode->GetMarkupType());

  this->UpdatePlaceNodeClassNamesInSelectionNode();

  this->InvokeEvent(vtkSlicerMarkupsLogic::MarkupRegistered);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::UnregisterMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  // Check for nullptr
  if (markupsNode == nullptr)
    {
    vtkErrorMacro("RegisterMarkup: Invalid node.");
    return;
    }

  // Check that the class is not already registered
  if (!this->GetNodeByMarkupsType(markupsNode->GetMarkupType()))
    {
    vtkWarningMacro("UnregisterMarkup: Markups node " << markupsNode->GetMarkupType() << " is not registered.");
    return;
    }

  // Remove the markup
  this->Internal->MarkupTypeToMarkupEntry.erase(markupsNode->GetMarkupType());
  this->Internal->RegisteredMarkupsOrder.remove(markupsNode->GetMarkupType());

  this->InvokeEvent(vtkSlicerMarkupsLogic::MarkupUnregistered);
}

//----------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::IsMarkupsNodeRegistered(const char* nodeType) const
{
  if (!nodeType)
    {
    return false;
    }
  for (auto markupTypToMarkupEntryIt : this->Internal->MarkupTypeToMarkupEntry)
    {
    if (strcmp(markupTypToMarkupEntryIt.second.MarkupsNode->GetClassName(), nodeType) == 0)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsWidget* vtkSlicerMarkupsLogic::GetWidgetByMarkupsType(const char* markupName) const
{
  if (!markupName)
    {
    vtkErrorMacro("GetWidgetByMarkupsType: Invalid node.");
    return nullptr;
    }

  const auto& markupIt = this->Internal->MarkupTypeToMarkupEntry.find(markupName);
  if (markupIt == this->Internal->MarkupTypeToMarkupEntry.end())
    {
    return nullptr;
    }

  return markupIt->second.MarkupsWidget;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerMarkupsLogic::GetNodeByMarkupsType(const char* markupName) const
{
  if (!markupName)
    {
    vtkErrorMacro("GetNodeByMarkupsType: Invalid node.");
    return nullptr;
    }

  const auto& markupIt = this->Internal->MarkupTypeToMarkupEntry.find(markupName);
  if (markupIt == this->Internal->MarkupTypeToMarkupEntry.end())
    {
    return nullptr;
    }

  return markupIt->second.MarkupsNode;
}

//----------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::GetCreateMarkupsPushButton(const char* markupName) const
{
  if (!markupName)
    {
    vtkErrorMacro("GetCreateMarkupsPushButton: Invalid node.");
    return false;
    }

  const auto& markupIt = this->Internal->MarkupTypeToMarkupEntry.find(markupName);
  if (markupIt == this->Internal->MarkupTypeToMarkupEntry.end())
    {
    return false;
    }

  return markupIt->second.CreatePushButton;
}

//----------------------------------------------------------------------------
const std::list<std::string>& vtkSlicerMarkupsLogic::GetRegisteredMarkupsTypes() const
{
  return this->Internal->RegisteredMarkupsOrder;
}

//----------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::ImportControlPointsFromTable(vtkMRMLMarkupsNode* markupsNode, vtkMRMLTableNode* tableNode,
  int startRow/*=0*/, int numberOfRows/*=-1*/)
{
  if (!markupsNode || !tableNode || !tableNode->GetTable() || startRow < 0)
    {
    vtkGenericWarningMacro("vtkSlicerMarkupsLogic::ImportControlPointsFromTable failed: Invalid markupsNode or tableNode or startRow.");
    return false;
    }
  if (numberOfRows < 0 || numberOfRows > tableNode->GetNumberOfRows() - startRow)
    {
    numberOfRows = tableNode->GetNumberOfRows() - startRow;
    }
  MRMLNodeModifyBlocker blocker(markupsNode);

  vtkTable* table = tableNode->GetTable();

  vtkAbstractArray* arrayX = nullptr;
  vtkAbstractArray* arrayY = nullptr;
  vtkAbstractArray* arrayZ = nullptr;
  bool rasCoordinates = true;
  if (table->GetColumnByName("r") && table->GetColumnByName("a") && table->GetColumnByName("s"))
    {
    arrayX = table->GetColumnByName("r");
    arrayY = table->GetColumnByName("a");
    arrayZ = table->GetColumnByName("s");
    }
  else if (table->GetColumnByName("l") && table->GetColumnByName("p") && table->GetColumnByName("s"))
    {
    rasCoordinates = false;
    arrayX = table->GetColumnByName("l");
    arrayY = table->GetColumnByName("p");
    arrayZ = table->GetColumnByName("s");
    }

  vtkAbstractArray* arrayLabel = table->GetColumnByName("label");
  vtkAbstractArray* arrayDescription = table->GetColumnByName("description");
  vtkAbstractArray* arraySelected = table->GetColumnByName("selected");
  vtkAbstractArray* arrayVisible = table->GetColumnByName("visible");
  vtkAbstractArray* arrayLocked = table->GetColumnByName("locked");
  vtkAbstractArray* arrayDefined= table->GetColumnByName("defined");

  for (int row = startRow; row < startRow + numberOfRows; row++)
    {
    // vtkVariant cannot convert values from VTK_BIT type, therefore we need to handle it
    // as a special case here.
    int positionStatus = vtkMRMLMarkupsNode::PositionDefined;
    if (vtkBitArray::SafeDownCast(arrayDefined))
      {
      if (vtkBitArray::SafeDownCast(arrayDefined)->GetValue(row) == 0)
        {
        positionStatus = vtkMRMLMarkupsNode::PositionUndefined;
        }
      }
    else if (arrayDefined)
      {
      if (arrayDefined->GetVariantValue(row).ToInt() == 0)
        {
        positionStatus = vtkMRMLMarkupsNode::PositionUndefined;
        }
      }

    vtkMRMLMarkupsNode::ControlPoint* controlPoint = new vtkMRMLMarkupsNode::ControlPoint;

    bool validX = false;
    bool validY = false;
    bool validZ = false;
    if (arrayX && arrayY && arrayZ)
      {
      controlPoint->Position[0] = arrayX->GetVariantValue(row).ToDouble(&validX);
      controlPoint->Position[1] = arrayY->GetVariantValue(row).ToDouble(&validY);
      controlPoint->Position[2] = arrayZ->GetVariantValue(row).ToDouble(&validZ);
      if (!rasCoordinates)
        {
        controlPoint->Position[0] = -controlPoint->Position[0];
        controlPoint->Position[1] = -controlPoint->Position[1];
        }
      }
    if (!validX || !validY || !validZ)
      {
      controlPoint->PositionStatus = vtkMRMLMarkupsNode::PositionUndefined;
      }

    if (arrayLabel)
      {
      controlPoint->Label = arrayLabel->GetVariantValue(row).ToString();
      }
    if (arrayDescription)
      {
      controlPoint->Description = arrayDescription->GetVariantValue(row).ToString();
      }

    if (vtkBitArray::SafeDownCast(arraySelected))
      {
      controlPoint->Selected = (vtkBitArray::SafeDownCast(arraySelected)->GetValue(row) != 0);
      }
    else if (arraySelected)
      {
      controlPoint->Selected = (arraySelected->GetVariantValue(row).ToInt() != 0);
      }

    if (vtkBitArray::SafeDownCast(arrayVisible))
      {
      controlPoint->Visibility = (vtkBitArray::SafeDownCast(arrayVisible)->GetValue(row) != 0);
      }
    else if (arrayVisible)
      {
      controlPoint->Visibility = (arrayVisible->GetVariantValue(row).ToInt() != 0);
      }

    if (vtkBitArray::SafeDownCast(arrayLocked))
      {
      controlPoint->Locked = (vtkBitArray::SafeDownCast(arrayLocked)->GetValue(row) != 0);
      }
    else if (arrayLocked)
      {
      controlPoint->Locked = (arrayLocked->GetVariantValue(row).ToInt() != 0);
      }

    controlPoint->PositionStatus = positionStatus;
    controlPoint->AutoCreated = false;
    markupsNode->AddControlPoint(controlPoint);
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::ExportControlPointsToTable(vtkMRMLMarkupsNode* markupsNode, vtkMRMLTableNode* tableNode,
  int coordinateSystem/*=vtkMRMLStorageNode::CoordinateSystemRAS*/)
{
  if (!markupsNode || !tableNode || !tableNode->GetTable())
    {
    vtkGenericWarningMacro("vtkSlicerMarkupsLogic::ExportControlPointsToTable failed: Invalid markupsNode or tableNode.");
    return false;
    }

  bool rasCoordinates = (coordinateSystem != vtkMRMLStorageNode::CoordinateSystemLPS);

  vtkTable* table = tableNode->GetTable();

  vtkAbstractArray* arrayLabel = table->GetColumnByName("label");
  if (!arrayLabel)
    {
    arrayLabel = vtkStringArray::New();
    arrayLabel->SetName("label");
    table->AddColumn(arrayLabel);
    arrayLabel->UnRegister(nullptr);
    }

  // Get/create coordinate arrays
  vtkAbstractArray* arrayCoordinates[3] = { nullptr, nullptr, nullptr };
  std::string columnNames[3] =
    {
    rasCoordinates ? "r" : "l",
    rasCoordinates ? "a" : "p",
    "s"
    };
  for (int coordIndex = 0; coordIndex < 3; coordIndex++)
    {
    arrayCoordinates[coordIndex] = table->GetColumnByName(columnNames[coordIndex].c_str());
    if (arrayCoordinates[coordIndex])
      {
      continue;
      }
    arrayCoordinates[coordIndex] = vtkDoubleArray::New();
    arrayCoordinates[coordIndex]->SetName(columnNames[coordIndex].c_str());
    tableNode->AddColumn(arrayCoordinates[coordIndex]);
    arrayCoordinates[coordIndex]->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayDefined = table->GetColumnByName("defined");
  if (!arrayDefined)
    {
    arrayDefined = vtkBitArray::New();
    arrayDefined->SetName("defined");
    table->AddColumn(arrayDefined);
    arrayDefined->UnRegister(nullptr);
    }

  vtkAbstractArray* arraySelected = table->GetColumnByName("selected");
  if (!arraySelected)
    {
    arraySelected = vtkBitArray::New();
    arraySelected->SetName("selected");
    table->AddColumn(arraySelected);
    arraySelected->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayVisible = table->GetColumnByName("visible");
  if (!arrayVisible)
    {
    arrayVisible = vtkBitArray::New();
    arrayVisible->SetName("visible");
    table->AddColumn(arrayVisible);
    arrayVisible->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayLocked = table->GetColumnByName("locked");
  if (!arrayLocked)
    {
    arrayLocked = vtkBitArray::New();
    arrayLocked->SetName("locked");
    table->AddColumn(arrayLocked);
    arrayLocked->UnRegister(nullptr);
    }

  vtkAbstractArray* arrayDescription = table->GetColumnByName("description");
  if (!arrayDescription)
    {
    arrayDescription = vtkStringArray::New();
    arrayDescription->SetName("description");
    table->AddColumn(arrayDescription);
    arrayDescription->UnRegister(nullptr);
    }

  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    int row = tableNode->AddEmptyRow();
    vtkMRMLMarkupsNode::ControlPoint* controlPoint = markupsNode->GetNthControlPoint(controlPointIndex);
    if (rasCoordinates)
      {
      arrayCoordinates[0]->SetVariantValue(row, vtkVariant(controlPoint->Position[0]));
      arrayCoordinates[1]->SetVariantValue(row, vtkVariant(controlPoint->Position[1]));
      }
    else
      {
      arrayCoordinates[0]->SetVariantValue(row, vtkVariant(-controlPoint->Position[0]));
      arrayCoordinates[1]->SetVariantValue(row, vtkVariant(-controlPoint->Position[1]));
      }
    arrayCoordinates[2]->SetVariantValue(row, vtkVariant(controlPoint->Position[2]));

    arrayLabel->SetVariantValue(row, controlPoint->Label.c_str());
    arrayDescription->SetVariantValue(row, controlPoint->Description.c_str());
    arraySelected->SetVariantValue(row, controlPoint->Selected);
    arrayVisible->SetVariantValue(row, controlPoint->Visibility);
    arrayLocked->SetVariantValue(row, controlPoint->Locked);
    arrayDefined->SetVariantValue(row, controlPoint->PositionStatus==vtkMRMLMarkupsNode::PositionDefined);
    }

  return true;
}

//------------------------------------------------------------------------------
vtkVector3d vtkSlicerMarkupsLogic::GenerateUniqueColor()
{
  double color[3] = { 0.5, 0.5, 0.5 };
  this->GenerateUniqueColor(color);
  return vtkVector3d(color[0], color[1], color[2]);
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::GenerateUniqueColor(double color[3])
{
  double rgba[4] = { 1.0, 1.0, 0.0, 1.0 }; // default is yellow
  vtkMRMLColorTableNode* colorTable = nullptr;
  vtkMRMLScene* scene = this->GetMRMLScene();
    {
    colorTable = vtkMRMLColorTableNode::SafeDownCast(
      scene->GetNodeByID("vtkMRMLColorTableNodeFileMediumChartColors.txt"));
    }
  if (colorTable)
    {
    colorTable->GetColor(this->Internal->UniqueColorNextColorTableIndex, rgba);
    this->Internal->UniqueColorNextColorTableIndex++;
    if (this->Internal->UniqueColorNextColorTableIndex >= colorTable->GetNumberOfColors())
      {
      // reached the end of the color table, start from the beginning
      // (the result is not completely unique colors, but at least enough variety that is
      // sufficient for most cases)
      this->Internal->UniqueColorNextColorTableIndex = 0;
      }
    }
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
}

//------------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::ExportControlPointsToCSV(vtkMRMLMarkupsNode* markupsNode,
  const std::string filename, bool lps/*=true*/)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkSlicerMarkupsLogic::ExportControlPointsToCSV failed: invalid input markupsNode");
    return false;
    }
  vtkNew<vtkMRMLTableNode> tableNode;
  if (!vtkSlicerMarkupsLogic::ExportControlPointsToTable(markupsNode, tableNode,
    lps ? vtkMRMLStorageNode::CoordinateSystemLPS : vtkMRMLStorageNode::CoordinateSystemRAS))
    {
    return false;
    }
  vtkNew<vtkMRMLTableStorageNode> tableStorageNode;
  tableStorageNode->SetFileName(filename.c_str());
  if (!tableStorageNode->WriteData(tableNode))
    {
    return false;
    }
  return true;
}

//------------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::ImportControlPointsFromCSV(
  vtkMRMLMarkupsNode* markupsNode, const std::string filename)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkSlicerMarkupsLogic::ImportControlPointsFromCSV failed: invalid markupsNode");
    return false;
    }
  vtkNew<vtkMRMLTableNode> tableNode;
  vtkNew<vtkMRMLTableStorageNode> tableStorageNode;
  tableStorageNode->SetFileName(filename.c_str());
  if (!tableStorageNode->ReadData(tableNode))
    {
    return false;
    }
  if (!vtkSlicerMarkupsLogic::ImportControlPointsFromTable(markupsNode, tableNode))
    {
    return false;
    }
  return true;
}
