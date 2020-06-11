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
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsStorageNode.h"

// Annotation MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"

// MRML includes
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSceneViewNode.h"

// vtkAddon includes
#include "vtkAddonMathUtilities.h"

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkDelaunay2D.h>
#include <vtkDiskSource.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkMassProperties.h>
#include <vtkStringArray.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <cassert>

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
  this->AutoCreateDisplayNodes = true;
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::~vtkSlicerMarkupsLogic() = default;

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

//  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*> (callData);

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
      int controlPointIndex = -1;
      int viewGroup = -1;
      vtkMRMLSliceNode* sliceNode = nullptr;
      if (callData != nullptr)
        {
        vtkMRMLInteractionEventData* eventData = reinterpret_cast<vtkMRMLInteractionEventData*>(callData);
        if (eventData->GetComponentType() == vtkMRMLMarkupsDisplayNode::ComponentControlPoint)
          {
          controlPointIndex = eventData->GetComponentIndex();
          }
        if (eventData->GetViewNode())
          {
          viewGroup = eventData->GetViewNode()->GetViewGroup();
          sliceNode = vtkMRMLSliceNode::SafeDownCast(eventData->GetViewNode());
          }
        }
      // Jump current slice node to the plane of the control point (do not center)
      if (sliceNode)
        {
        vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(markupsDisplayNode->GetDisplayableNode());
        if (markupsNode)
          {
          double worldPos[3] = { 0.0 };
          markupsNode->GetNthControlPointPositionWorld(controlPointIndex, worldPos);
          sliceNode->JumpSliceByOffsetting(worldPos[0], worldPos[1], worldPos[2]);
          }
        }
      // Jump centered in all other slices in the view group
      this->JumpSlicesToNthPointInMarkup(markupsDisplayNode->GetDisplayableNode()->GetID(), controlPointIndex,
        true /* centered */, viewGroup, sliceNode);
      }
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
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ObserveMRMLScene()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // add known markup types to the selection node
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (selectionNode)
    {
    // got into batch process mode so that an update on the mouse mode tool
    // bar is triggered when leave it
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsMouseModePlace.png", "Fiducial");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsLineNode", ":/Icons/MarkupsLineMouseModePlace.png", "Line");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsAngleNode", ":/Icons/MarkupsAngleMouseModePlace.png", "Angle");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsCurveNode", ":/Icons/MarkupsCurveMouseModePlace.png", "Open Curve");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsClosedCurveNode", ":/Icons/MarkupsClosedCurveMouseModePlace.png", "Closed Curve");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsPlaneNode", ":/Icons/MarkupsPlaneMouseModePlace.png", "Plane");

    // trigger an update on the mouse mode toolbar
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
    }

 this->Superclass::ObserveMRMLScene();
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != nullptr);

  vtkMRMLScene *scene = this->GetMRMLScene();

  // Nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsLineNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsAngleNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsCurveNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsPlaneNode>::New());

  // Display nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialDisplayNode>::New());

  // Storage Nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsJsonStorageNode>::New());
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
  // but make sure that the scen eis not batch processing before responding to
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
  this->SetActiveListID(markupsNode);
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
void vtkSlicerMarkupsLogic::SetActiveListID(vtkMRMLMarkupsNode *markupsNode)
{
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str()));
  if (!selectionNode)
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::SetActiveListID: No selection node in the scene.");
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
  std::string id;

  if (!scene && !this->GetMRMLScene())
    {
    vtkErrorMacro("AddNewMarkupsNode: no scene to add a markups node to!");
    return id;
    }

  vtkMRMLScene *addToThisScene;
  if (scene)
    {
    addToThisScene = scene;
    }
  else
    {
    addToThisScene = this->GetMRMLScene();
    }

  // create and add the node
  vtkMRMLMarkupsFiducialNode *mnode = vtkMRMLMarkupsFiducialNode::New();
  addToThisScene->AddNode(mnode);

  // add a display node
  std::string displayID = this->AddNewDisplayNodeForMarkupsNode(mnode);

  if (displayID.compare("") != 0)
    {
    // get the node id to return
    id = std::string(mnode->GetID());
    if (name != nullptr)
      {
      mnode->SetName(name);
      }
    // if adding to this scene (could be adding to a scene view during conversion)
    // make it active so mouse mode tool bar clicks will add new fids to
    // this list
    if (addToThisScene == this->GetMRMLScene())
      {
      this->SetActiveListID(mnode);
      }
    }
  // clean up
  mnode->Delete();

  return id;
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::AddFiducial(double r, double a, double s)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("AddFiducial: no scene defined!");
    return -1;
    }

  // get the active list id
  std::string listID = this->GetActiveListID();

  // is there no active fiducial list?
  if (listID.size() == 0)
    {
    vtkDebugMacro("AddFiducial: no list is active, adding one first!");
    std::string newListID = this->AddNewFiducialNode();
    if (newListID.size() == 0)
      {
      vtkErrorMacro("AddFiducial: failed to add a new fiducial list to the scene.");
      return -1;
      }
    // try to get the id again
    listID = this->GetActiveListID();
    if (listID.size() == 0)
      {
      vtkErrorMacro("AddFiducial: failed to create a new list to add to!");
      return -1;
      }
    }

  // get the active list
  vtkMRMLNode *listNode = this->GetMRMLScene()->GetNodeByID(listID.c_str());
  if (!listNode)
    {
    vtkErrorMacro("AddFiducial: failed to get the active list with id " << listID);
    return -1;
    }
  vtkMRMLMarkupsFiducialNode *fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(listNode);
  if (!fiducialNode)
    {
    vtkErrorMacro("AddFiducial: active list is not a fiducial list: " << listNode->GetClassName());
    return -1;
    }
  vtkDebugMacro("AddFiducial: adding a fiducial to the list " << listID);
  // add the point to the active fiducial list
  return fiducialNode->AddFiducial(r,a,s);
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
char* vtkSlicerMarkupsLogic::LoadMarkups(const char* fileName, const char* nodeName/*=nullptr*/)
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
    return this->LoadMarkupsFromJson(fileName, nodeName);
    }
  else if (extension == std::string(".fcsv"))
    {
    return this->LoadMarkupsFromFcsv(fileName, nodeName);
    }
  else
    {
    vtkErrorMacro("vtkSlicerMarkupsLogic::LoadMarkups failed: unrecognized file extension in " << fileName);
    return nullptr;
    }
}

//---------------------------------------------------------------------------
char* vtkSlicerMarkupsLogic::LoadMarkupsFiducials(const char* fileName, const char* fidsName/*=nullptr*/)
{

  return this->LoadMarkups(fileName, fidsName);
}

//---------------------------------------------------------------------------
char* vtkSlicerMarkupsLogic::LoadMarkupsFromJson(const char* fileName, const char* nodeName/*=nullptr*/)
{
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkups: null file or markups class name, cannot load");
    return nullptr;
    }

  vtkDebugMacro("LoadMarkups, file name = " << fileName << ", nodeName = " << (nodeName ? nodeName : "null"));

  // make a storage node and fiducial node and set the file name
  vtkMRMLMarkupsJsonStorageNode* storageNode = vtkMRMLMarkupsJsonStorageNode::SafeDownCast(
    this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsJsonStorageNode"));
  if (!storageNode)
    {
    vtkErrorMacro("LoadMarkups: failed to instantiate markups storage node by class vtkMRMLMarkupsJsonStorageNode");
    return nullptr;
    }

  vtkMRMLMarkupsNode* markupsNode = storageNode->AddNewMarkupsNodeFromFile(fileName, nodeName);
  if (!markupsNode)
    {
    return nullptr;
    }

  return markupsNode->GetID();
}

//---------------------------------------------------------------------------
char * vtkSlicerMarkupsLogic::LoadMarkupsFromFcsv(const char* fileName, const char* nodeName/*=nullptr*/)
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
    this->GetMRMLScene()->RemoveNode(storageNode);
    this->GetMRMLScene()->RemoveNode(markupsNode);
    }

  return nodeID;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllMarkupsVisibility(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllMarkupsVisibility: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointVisibility(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ToggleAllMarkupsVisibility(vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllMarkupsVisibility: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointVisibility(i, !(node->GetNthControlPointVisibility(i)));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllMarkupsLocked(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllMarkupsLocked: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointLocked(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ToggleAllMarkupsLocked(vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllMarkupsLocked: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointLocked(i, !(node->GetNthControlPointLocked(i)));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllMarkupsSelected(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllMarkupsSelected: No list");
    return;
    }

  for (int i = 0; i < node->GetNumberOfControlPoints(); i++)
    {
    node->SetNthControlPointSelected(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ToggleAllMarkupsSelected(vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    vtkDebugMacro("ToggleAllMarkupsSelected: No list");
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
  targetDisplayNode->SetSelectedColor(sourceDisplayNode->GetSelectedColor());
  targetDisplayNode->SetColor(sourceDisplayNode->GetColor());
  targetDisplayNode->SetOpacity(sourceDisplayNode->GetOpacity());
  targetDisplayNode->SetGlyphType(sourceDisplayNode->GetGlyphType());
  targetDisplayNode->SetGlyphScale(sourceDisplayNode->GetGlyphScale());
  targetDisplayNode->SetGlyphSize(sourceDisplayNode->GetGlyphSize());
  targetDisplayNode->SetUseGlyphScale(sourceDisplayNode->GetUseGlyphScale());
  targetDisplayNode->SetTextScale(sourceDisplayNode->GetTextScale());
  targetDisplayNode->SetSliceProjection(sourceDisplayNode->GetSliceProjection());
  targetDisplayNode->SetSliceProjectionColor(sourceDisplayNode->GetSliceProjectionColor());
  targetDisplayNode->SetSliceProjectionOpacity(sourceDisplayNode->GetSliceProjectionOpacity());
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
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: at least one of the markup list nodes are null!");
    return false;
    }

  if (!markupsNode->ControlPointExists(n))
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: source index n " << n
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
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: failed to insert new control point at " << newIndex <<
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
    vtkErrorMacro("CopyNthMarkupToNewList: at least one of the markup list nodes are null!");
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
      vtkMRMLMarkupsFiducialNode *markupsNode = nullptr;
      mrmlNode = scene->GetNodeByID(markupsListID.c_str());
      if (!mrmlNode)
        {
        continue;
        }
      markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
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
        vtkMRMLAnnotationFiducialNode *annotNode;
        annotNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(children->GetItemAsObject(c));
        if (!annotNode)
          {
          continue;
          }
        double coord[3];
        annotNode->GetFiducialCoordinates(coord);
        int fidIndex = markupsNode->AddFiducial(coord[0], coord[1], coord[2]);
        vtkDebugMacro("Added a fiducial at index " << fidIndex);
        markupsNode->SetNthControlPointLabel(fidIndex, std::string(annotNode->GetName()));
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
        vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = nullptr;
        vtkMRMLAnnotationTextDisplayNode *textDisplayNode = nullptr;
        pointDisplayNode = annotNode->GetAnnotationPointDisplayNode();
        textDisplayNode = annotNode->GetAnnotationTextDisplayNode();

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
          vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(annotNode->GetScene(),
                                                           annotNode->GetID());
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
void vtkSlicerMarkupsLogic::RenameAllMarkupsFromCurrentFormat(vtkMRMLMarkupsNode *markupsNode)
{
  if (!markupsNode)
    {
    return;
    }

  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  // get the format string with the list name replaced
  std::string formatString = markupsNode->ReplaceListNameInMarkupLabelFormat();
  bool numberInFormat = false;
  char buff[MARKUPS_BUFFER_SIZE];
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
        vtkWarningMacro("RenameAllMarkupsFromCurrentFormat: more than one number in markup " << n << ", keeping second one: " << oldLabel.c_str());
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
int vtkSlicerMarkupsLogic::GetSliceIntersectionsVisibility()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("GetSliceIntersectionsVisibility: no scene");
    return -1;
    }
  int numVisible = 0;
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceCompositeNode"));
  if (!nodes.GetPointer())
    {
    return -1;
    }
  vtkMRMLSliceCompositeNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node->GetSliceIntersectionVisibility())
      {
      numVisible++;
      }
    }
  if (numVisible == 0)
    {
    return 0;
    }
  else if (numVisible == nodes->GetNumberOfItems())
    {
    return 1;
    }
  else
    {
    return 2;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetSliceIntersectionsVisibility(bool flag)
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceCompositeNode"));
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetSliceIntersectionVisibility(flag);
    }
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
  vtkSmartPointer<vtkPolyData> surface;
  if (inputSurface)
    {
    inputSurface->Reset();
    surface = inputSurface;
    }
  else
    {
    surface = vtkSmartPointer<vtkPolyData>::New();
    }
  vtkPoints* curvePointsWorld = curveNode->GetCurvePointsWorld();
  if (curvePointsWorld == nullptr || curvePointsWorld->GetNumberOfPoints() == 0)
    {
    return 0.0;
    }
  bool success = false;
  if (projectWarp)
    {
    success = vtkSlicerMarkupsLogic::FitSurfaceProjectWarp(curvePointsWorld, surface);
    }
  else
    {
    success = vtkSlicerMarkupsLogic::FitSurfaceDiskWarp(curvePointsWorld, surface);
    }
  if (!success)
    {
    return 0.0;
    }

  vtkNew<vtkMassProperties> metrics;
  metrics->SetInputData(surface);
  double surfaceArea = metrics->GetSurfaceArea();

  return surfaceArea;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::FitSurfaceProjectWarp(vtkPoints* curvePoints, vtkPolyData* surface, double vtkNotUsed(radiusScalingFactor)/*=1.0*/)
{
  if (!curvePoints || !surface)
    {
    vtkGenericWarningMacro("FitSurfaceProjectWarp failed: invalid curvePoints or surface");
    return false;
    }

  vtkIdType numberOfCurvePoints = curvePoints->GetNumberOfPoints();
  if (numberOfCurvePoints < 3)
    {
    // less than 3 points means that the surface is empty
    surface->Initialize();
    return true;
    }

  // Create a polydata containing a single polygon of the curve points
  vtkNew<vtkPolyData> inputSurface;
  inputSurface->SetPoints(curvePoints);
  vtkNew<vtkCellArray> polys;
  polys->InsertNextCell(numberOfCurvePoints);
  for (int i = 0; i < numberOfCurvePoints; i++)
    {
    polys->InsertCellPoint(i);
    }
  polys->Modified();
  inputSurface->SetPolys(polys);

  // Remove duplicate points (it would confuse the triangulator)
  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->SetInputData(inputSurface);
  cleaner->Update();
  inputSurface->DeepCopy(cleaner->GetOutput());
  vtkNew<vtkPoints> cleanedCurvePoints;
  cleanedCurvePoints->DeepCopy(inputSurface->GetPoints());
  numberOfCurvePoints = cleanedCurvePoints->GetNumberOfPoints();

  // The triangulator requires all points to be on the XY plane
  vtkNew<vtkMatrix4x4> transformToBestFitPlaneMatrix;
  if (!vtkAddonMathUtilities::FitPlaneToPoints(inputSurface->GetPoints(), transformToBestFitPlaneMatrix))
    {
    return false;
    }
  vtkNew<vtkTransform> transformToXYPlane;
  transformToXYPlane->SetMatrix(transformToBestFitPlaneMatrix); // set XY plane -> best-fit plane
  transformToXYPlane->Inverse(); // // change the transform to: set best-fit plane -> XY plane
  vtkNew<vtkPoints> pointsOnPlane;
  transformToXYPlane->TransformPoints(cleanedCurvePoints, pointsOnPlane);
  inputSurface->SetPoints(pointsOnPlane);
  for (vtkIdType i = 0; i < numberOfCurvePoints; i++)
    {
    double* pt = pointsOnPlane->GetPoint(i);
    pointsOnPlane->SetPoint(i, pt[0], pt[1], 0.0);
    }

  // Ensure points are in counter-clockwise direction
  // (that indicates to Delaunay2D that it is a polygon to be
  // filled in and not a hole).
  if (vtkSlicerMarkupsLogic::IsPolygonClockwise(pointsOnPlane))
    {
    vtkNew<vtkCellArray> reversePolys;
    reversePolys->InsertNextCell(numberOfCurvePoints);
    for (int i = numberOfCurvePoints - 1; i >= 0 ; i--)
      {
      reversePolys->InsertCellPoint(i);
      }
    reversePolys->Modified();
    inputSurface->SetPolys(reversePolys);
    }

  // Add random points to improve triangulation quality.
  // We already have many points on the boundary but no points inside the polygon.
  // If we passed these points to the triangulator then opposite points on the curve
  // would be connected by triangles, so we would end up with many very skinny triangles,
  // and not smooth surface after warping.
  // By adding random points, the triangulator can create evenly sized triangles.
  vtkIdType numberOfExtraPoints = numberOfCurvePoints * 5; // add 10x more mesh points than contour points
  double bounds[6] = { 0.0 };
  pointsOnPlane->GetBounds(bounds);
  for (vtkIdType i = 0; i < numberOfExtraPoints; i++)
    {
    pointsOnPlane->InsertNextPoint(vtkMath::Random(bounds[0], bounds[1]), vtkMath::Random(bounds[2], bounds[3]), 0.0);
    }

  vtkNew<vtkDelaunay2D> triangulator;
  triangulator->SetInputData(inputSurface);
  triangulator->SetSourceData(inputSurface);
  triangulator->Update();
  // TODO: return with failure if a warning is logged (it may happen when the flattened curve is self-intersecting)
  vtkPolyData* triangulatedSurface = triangulator->GetOutput();
  vtkPoints* triangulatedSurfacePoints = triangulatedSurface->GetPoints();

  vtkNew<vtkPoints> sourceLandmarkPoints; // points on the triangulated surface
  vtkNew<vtkPoints> targetLandmarkPoints; // points on the curve
  // Use only the transformed curve points (first numberOfCurvePoints points)
  int step = 3; // use only every 3rd boundary point for simpler and faster warping
  vtkIdType numberOfRegistrationLandmarkPoints = numberOfCurvePoints / step;
  sourceLandmarkPoints->SetNumberOfPoints(numberOfRegistrationLandmarkPoints);
  targetLandmarkPoints->SetNumberOfPoints(numberOfRegistrationLandmarkPoints);
  for (vtkIdType landmarkPointIndex = 0; landmarkPointIndex < numberOfRegistrationLandmarkPoints; landmarkPointIndex++)
    {
    sourceLandmarkPoints->SetPoint(landmarkPointIndex, triangulatedSurfacePoints->GetPoint(landmarkPointIndex*step));
    targetLandmarkPoints->SetPoint(landmarkPointIndex, cleanedCurvePoints->GetPoint(landmarkPointIndex*step));
    }

  vtkNew<vtkThinPlateSplineTransform> landmarkTransform;
  // Disable regularization to make sure transformation is correct even if source or target points are coplanar
  landmarkTransform->SetRegularizeBulkTransform(false);
  landmarkTransform->SetBasisToR();
  landmarkTransform->SetSourceLandmarks(sourceLandmarkPoints);
  landmarkTransform->SetTargetLandmarks(targetLandmarkPoints);

  vtkNew<vtkTransformPolyDataFilter> polyTransformToCurve;
  polyTransformToCurve->SetTransform(landmarkTransform);
  polyTransformToCurve->SetInputData(triangulatedSurface);

  vtkNew<vtkPolyDataNormals> polyDataNormals;
  polyDataNormals->SetInputConnection(polyTransformToCurve->GetOutputPort());
  polyDataNormals->SplittingOff();
  polyDataNormals->Update();

  surface->DeepCopy(polyDataNormals->GetOutput());
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::IsPolygonClockwise(vtkPoints* points)
{
  vtkIdType numberOfPoints = (points == nullptr ? 0 : points->GetNumberOfPoints());
  if (numberOfPoints < 3)
    {
    return false;
    }

  // Find the bottom-left point (it is on the convex hull) of the polygon,
  // and check sign of cross-product of the edges before and after that point.
  // (https://en.wikipedia.org/wiki/Curve_orientation#Orientation_of_a_simple_polygon)

  double* point0 = points->GetPoint(0);
  double minX = point0[0];
  double minY = point0[1];
  vtkIdType cornerPointIndex = 0;
  for (vtkIdType i = 1; i < numberOfPoints; i++)
    {
    double* p = points->GetPoint(i);
    if ((p[1] < minY) || ((p[1] == minY) && (p[0] < minX)))
      {
      cornerPointIndex = i;
      minX = p[0];
      minY = p[1];
      }
    }

  double p1[3];
  double p2[3];
  double p3[3];
  points->GetPoint((cornerPointIndex - 1) % numberOfPoints, p1);
  points->GetPoint(cornerPointIndex, p2);
  points->GetPoint((cornerPointIndex + 1) % numberOfPoints, p3);
  double det = p2[0] * p3[1] - p2[1] * p3[0] - p1[0] * p3[1] + p1[0] * p2[1] + p1[1] * p3[0] - p1[1] * p2[0];
  return (det < 0);
}


//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor/*=1.0*/)
{
  if (!curvePoints || !surface)
    {
    return false;
    }

  // Transform a unit disk to the curve circumference using thin-plate spline interpolation.
  // It does not guarantee minimum area surface but at least it is a smooth surface that tightly
  // fits at the provided contour at the boundary.
  // A further refinement step could be added during that the surface points are adjusted so that
  // the surface area is minimized.

  // We have a landmark point at every 4.5 degrees (360/80) around the perimeter of the curve.
  // This is accurate enough for reproducing even very complex curves and can be still computed
  // quite quickly.
  const vtkIdType numberOfLandmarkPoints = 80;
  const vtkIdType numberOfCurvePoints = curvePoints->GetNumberOfPoints();

  vtkNew<vtkPoints> sourceLandmarkPoints; // points on the unit disk
  sourceLandmarkPoints->SetNumberOfPoints(numberOfLandmarkPoints); // points on the curve
  vtkNew<vtkPoints> targetLandmarkPoints; // curve points
  targetLandmarkPoints->SetNumberOfPoints(numberOfLandmarkPoints);
  for (vtkIdType landmarkPointIndex = 0; landmarkPointIndex < numberOfLandmarkPoints; ++landmarkPointIndex)
    {
    double angle = double(landmarkPointIndex) / double(numberOfLandmarkPoints) * 2.0 * vtkMath::Pi();
    vtkIdType curvePointIndex = vtkMath::Round(round(double(landmarkPointIndex) / double(numberOfLandmarkPoints) * numberOfCurvePoints));
    sourceLandmarkPoints->SetPoint(landmarkPointIndex, cos(angle), sin(angle), 0);
    targetLandmarkPoints->SetPoint(landmarkPointIndex, curvePoints->GetPoint(curvePointIndex));
    }

  vtkNew<vtkThinPlateSplineTransform> landmarkTransform;
  // Disable regularization to make sure transformation is correct even if source or target points are coplanar
  landmarkTransform->SetRegularizeBulkTransform(false);
  landmarkTransform->SetBasisToR();
  landmarkTransform->SetSourceLandmarks(sourceLandmarkPoints);
  landmarkTransform->SetTargetLandmarks(targetLandmarkPoints);

  vtkNew<vtkDiskSource> unitDisk;
  unitDisk->SetOuterRadius(radiusScalingFactor);
  unitDisk->SetInnerRadius(0.0);
  unitDisk->SetCircumferentialResolution(80);
  unitDisk->SetRadialResolution(15);

  vtkNew<vtkDelaunay2D> triangulator;
  triangulator->SetTolerance(0.01); // get rid of the small triangles near the center of the unit disk
  triangulator->SetInputConnection(unitDisk->GetOutputPort());

  vtkNew<vtkTransformPolyDataFilter> polyTransformToCurve;
  polyTransformToCurve->SetTransform(landmarkTransform);
  polyTransformToCurve->SetInputConnection(triangulator->GetOutputPort());

  vtkNew<vtkPolyDataNormals> polyDataNormals;
  polyDataNormals->SetInputConnection(polyTransformToCurve->GetOutputPort());
  // There are a few triangles in the triangulated unit disk with inconsistent
  // orientation. Enabling consistency check fixes them.
  polyDataNormals->ConsistencyOn();
  polyDataNormals->Update();

  surface->DeepCopy(polyDataNormals->GetOutput());
  return true;
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
