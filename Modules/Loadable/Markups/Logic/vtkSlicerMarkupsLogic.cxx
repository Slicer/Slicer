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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
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

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>

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

  vtkSlicerMarkupsLogicCallback(){}

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*)
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
  this->DefaultMarkupsDisplayNode = vtkMRMLMarkupsDisplayNode::New();
  // link an observation of the modified event on the display node to trigger
  // a modified event on the logic so any settings panel can get updated
  // first, create the callback
  vtkNew<vtkSlicerMarkupsLogicCallback> myCallback;
  myCallback->SetLogic(this);
  this->DefaultMarkupsDisplayNode->AddObserver(vtkCommand::ModifiedEvent, myCallback.GetPointer());
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::~vtkSlicerMarkupsLogic()
{
  this->DefaultMarkupsDisplayNode->RemoveObserver(vtkCommand::ModifiedEvent);
  this->DefaultMarkupsDisplayNode->Delete();
  this->DefaultMarkupsDisplayNode = NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ProcessMRMLNodesEvents(vtkObject *caller,
                                                   unsigned long event,
                                                   void *vtkNotUsed(callData))
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
  vtkMRMLSelectionNode *selectionNode = NULL;
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str());
  if (mrmlNode)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(mrmlNode);
    }
  if (selectionNode)
    {
    // got into batch process mode so that an update on the mouse mode tool
    // bar is triggered when leave it
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsMouseModePlace.png", "Fiducial");

    // trigger an upate on the mouse mode toolbar
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
    }

 this->Superclass::ObserveMRMLScene();
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  vtkMRMLScene *scene = this->GetMRMLScene();

  // Nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New());

  // Display nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New());

  // Storage Nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialStorageNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
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
  if (markupsNode->GetDisplayNode() == NULL)
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
    if (this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str()) == NULL)
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
  vtkMRMLSelectionNode *selectionNode = NULL;;
  std::string selectionNodeID = this->GetSelectionNodeID();
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str());
  if (!node)
    {
    vtkErrorMacro("GetActiveListID: no selection node to govern active lists.");
    return listID;
    }
  selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);

  if (!selectionNode)
    {
    vtkErrorMacro("GetActiveListID: unable to get the selection node that governs active lists.");
    return listID;
    }

  char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  // is there no active fiducial list?
  if (activePlaceNodeID == NULL)
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
  std::string selectionNodeID = this->GetSelectionNodeID();
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str());
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }
  if (selectionNode)
    {
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
      selectionNode->SetActivePlaceNodeID(markupsNode->GetID());
      }
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
  if (displayableNode && displayableNode->GetDisplayNode() != NULL)
    {
    return displayableNode->GetDisplayNodeID();
    }

  // create the display node
  vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::New();
  // set it from the defaults
  this->SetDisplayNodeToDefaults(displayNode);
  vtkDebugMacro("AddNewDisplayNodeForMarkupsNode: set display node to defaults");

  // add it to the scene
  //mrmlNode->GetScene()->AddNode(displayNode);
  vtkMRMLNode *n = mrmlNode->GetScene()->InsertBeforeNode(mrmlNode, displayNode);
  if (!n)
    {
    vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: error on insert before node");
    return id;
    }

  // get the node id to return
  id = std::string(displayNode->GetID());

  // cast to markups node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (markupsNode)
    {
    // observe the display node
    markupsNode->DisableModifiedEventOn();
    markupsNode->AddAndObserveDisplayNodeID(id.c_str());
    markupsNode->DisableModifiedEventOff();
    }

  // clean up
  displayNode->Delete();

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
    if (name != NULL)
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
void vtkSlicerMarkupsLogic::JumpSlicesToLocation(double x, double y, double z, bool centered)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("JumpSlicesToLocation: No scene defined");
    return;
    }

  // save the whole state as iterating over all slice nodes
  this->GetMRMLScene()->SaveStateForUndo();

  // jump all the slice nodes in the scene
  int numSliceNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n = 0; n < numSliceNodes; ++n)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(n,"vtkMRMLSliceNode");
    if (!mrmlNode)
      {
      vtkErrorMacro("JumpSlicesToLocation: could not get slice node " << n << " from scene");
      return;
      }
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::SafeDownCast(mrmlNode);
    if (sliceNode)
      {
      if (centered)
        {
        sliceNode->JumpSliceByCentering(x,y,z);
        }
      else
        {
        sliceNode->JumpSliceByOffsetting(x,y,z);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToNthPointInMarkup(const char *id, int n, bool centered)
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
  if (mrmlNode == NULL)
    {
    return;
    }
  vtkMRMLMarkupsNode *markup = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (markup)
    {
    double point[4];
    // get the first point for now
    markup->GetMarkupPointWorld(n, 0, point);
    this->JumpSlicesToLocation(point[0], point[1], point[2], centered);
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
  if (mrmlNode1 == NULL)
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
  if (mrmlNode2 == NULL)
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
    // get the first point for now
    markup->GetMarkupPointWorld(n, 0, point);

    // and focus the camera there
    cameraNode->SetFocalPoint(point[0], point[1], point[2]);
}

//---------------------------------------------------------------------------
char * vtkSlicerMarkupsLogic::LoadMarkupsFiducials(const char *fileName, const char *fidsName)
{
  char *nodeID = NULL;
  std::string idList;
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkupsFiducials: null file name, cannot load");
    return nodeID;
    }

  vtkDebugMacro("LoadMarkupsFiducials, file name = " << fileName << ", fidsName = " << (fidsName ? fidsName : "null"));

  // turn on batch processing
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // make a storage node and fiducial node and set the file name
  vtkNew<vtkMRMLMarkupsFiducialStorageNode> storageNode;
  storageNode->SetFileName(fileName);
  vtkNew<vtkMRMLMarkupsFiducialNode> fidNode;
  fidNode->SetName(fidsName);

  // add the nodes to the scene and set up the observation on the storage node
  this->GetMRMLScene()->AddNode(storageNode.GetPointer());
  this->GetMRMLScene()->AddNode(fidNode.GetPointer());
  fidNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // read the file
  if (storageNode->ReadData(fidNode.GetPointer()))
    {
    nodeID = fidNode->GetID();
    }

  // turn off batch processing
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);

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
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupVisibility(i, flag);
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
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupVisibility(i, !(node->GetNthMarkupVisibility(i)));
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
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupLocked(i, flag);
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
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupLocked(i, !(node->GetNthMarkupLocked(i)));
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
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupSelected(i, flag);
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
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupSelected(i, !(node->GetNthMarkupSelected(i)));
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphType(int glyphType)
{
  this->DefaultMarkupsDisplayNode->SetGlyphType(glyphType);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphTypeFromString(const char *glyphType)
{
  if (glyphType == NULL)
    {
    vtkErrorMacro("SetDefaultMarkupsDisplayNodeGlyphTypeFromString: null glyph type string!");
    return;
    }
  this->DefaultMarkupsDisplayNode->SetGlyphTypeFromString(glyphType);
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeGlyphType()
{
  return this->DefaultMarkupsDisplayNode->GetGlyphType();
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeGlyphTypeAsString()
{
  std::string glyphString;
  const char *glyphType = this->DefaultMarkupsDisplayNode->GetGlyphTypeAsString();
  if (glyphType)
    {
    glyphString = std::string(glyphType);
    }
  return glyphString;
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeGlyphScale()
{
  return this->DefaultMarkupsDisplayNode->GetGlyphScale();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphScale(double scale)
{
  this->DefaultMarkupsDisplayNode->SetGlyphScale(scale);
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeTextScale()
{
  return this->DefaultMarkupsDisplayNode->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeTextScale(double scale)
{
  this->DefaultMarkupsDisplayNode->SetTextScale(scale);
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeOpacity()
{
  return this->DefaultMarkupsDisplayNode->GetOpacity();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeOpacity(double opacity)
{
  this->DefaultMarkupsDisplayNode->SetOpacity(opacity);
}

//---------------------------------------------------------------------------
double *vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeColor()
{
  return this->DefaultMarkupsDisplayNode->GetColor();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeColor(double *color)
{
  if (!color)
    {
    return;
    }
  this->DefaultMarkupsDisplayNode->SetColor(color);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeColor(double r, double g, double b)
{
  this->DefaultMarkupsDisplayNode->SetColor(r,g,b);
}

//---------------------------------------------------------------------------
double *vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSelectedColor()
{
  return this->DefaultMarkupsDisplayNode->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSelectedColor(double *color)
{
  if (!color)
    {
    return;
    }
  this->DefaultMarkupsDisplayNode->SetSelectedColor(color);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSelectedColor(double r, double g, double b)
{
  this->DefaultMarkupsDisplayNode->SetSelectedColor(r,g,b);
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSliceProjection()
{
  return this->DefaultMarkupsDisplayNode->GetSliceProjection();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjection(int projection)
{
  this->DefaultMarkupsDisplayNode->SetSliceProjection(projection);
}

//---------------------------------------------------------------------------
double *vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSliceProjectionColor()
{
  return this->DefaultMarkupsDisplayNode->GetSliceProjectionColor();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjectionColor(double *color)
{
  if (!color)
    {
    return;
    }
  this->DefaultMarkupsDisplayNode->SetSliceProjectionColor(color);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjectionColor(double r, double g, double b)
{
  this->DefaultMarkupsDisplayNode->SetSliceProjectionColor(r,g,b);
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSliceProjectionOpacity()
{
  return this->DefaultMarkupsDisplayNode->GetSliceProjectionOpacity();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjectionOpacity(double opacity)
{
  this->DefaultMarkupsDisplayNode->SetSliceProjectionOpacity(opacity);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDisplayNodeToDefaults(vtkMRMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }

  displayNode->SetSelectedColor(this->GetDefaultMarkupsDisplayNodeSelectedColor());
  displayNode->SetColor(this->GetDefaultMarkupsDisplayNodeColor());
  displayNode->SetOpacity(this->GetDefaultMarkupsDisplayNodeOpacity());
  displayNode->SetGlyphType(this->GetDefaultMarkupsDisplayNodeGlyphType());
  displayNode->SetGlyphScale(this->GetDefaultMarkupsDisplayNodeGlyphScale());
  displayNode->SetTextScale(this->GetDefaultMarkupsDisplayNodeTextScale());
  displayNode->SetSliceProjection(this->GetDefaultMarkupsDisplayNodeSliceProjection());
  displayNode->SetSliceProjectionColor(this->GetDefaultMarkupsDisplayNodeSliceProjectionColor());
  displayNode->SetSliceProjectionOpacity(this->GetDefaultMarkupsDisplayNodeSliceProjectionOpacity());
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::MoveNthMarkupToNewListAtIndex(int n, vtkMRMLMarkupsNode *markupsNode,
                                                          vtkMRMLMarkupsNode *newMarkupsNode, int newIndex)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: at least one of the markup list nodes are null!");
    return false;
    }

  if (n < 0 || n >= markupsNode->GetNumberOfMarkups())
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfMarkups());
    return false;
    }

  // get the markup
  Markup newMarkup;
  markupsNode->CopyMarkup(markupsNode->GetNthMarkup(n), &newMarkup);

  // add it to the destination list
  bool insertVal = newMarkupsNode->InsertMarkup(newMarkup, newIndex);
  if (!insertVal)
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: failed to insert new markup at " << newIndex << ", markup is still on source list.");
    return false;
    }

  // remove it from the source list
  markupsNode->RemoveMarkup(n);

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::CopyNthMarkupToNewList(int n, vtkMRMLMarkupsNode *markupsNode,
                                                   vtkMRMLMarkupsNode *newMarkupsNode)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("CopyNthMarkupToNewList: at least one of the markup list nodes are null!");
    return false;
    }

  if (n < 0 || n >= markupsNode->GetNumberOfMarkups())
    {
    vtkErrorMacro("CopyNthMarkupToNewList: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfMarkups());
    return false;
    }

  // get the markup
  Markup newMarkup;
  markupsNode->CopyMarkup(markupsNode->GetNthMarkup(n), &newMarkup);

  // add it to the destination list
  newMarkupsNode->AddMarkup(newMarkup);

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
      vtkMRMLNode *mrmlNode = NULL;
      vtkMRMLHierarchyNode *hierarchyNode = NULL;
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
      vtkMRMLMarkupsFiducialNode *markupsNode = NULL;
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
        markupsNode->SetNthMarkupLabel(fidIndex, std::string(annotNode->GetName()));
        char *desc = annotNode->GetDescription();
        if (desc)
          {
          markupsNode->SetNthMarkupDescription(fidIndex,std::string(desc));
          }
        markupsNode->SetNthMarkupSelected(fidIndex, annotNode->GetSelected());
        markupsNode->SetNthMarkupVisibility(fidIndex,
                                            annotNode->GetDisplayVisibility());
        markupsNode->SetNthMarkupLocked(fidIndex, annotNode->GetLocked());
        const char *assocNodeID = annotNode->GetAttribute("AssociatedNodeID");
        if (assocNodeID)
          {
          markupsNode->SetNthMarkupAssociatedNodeID(fidIndex, assocNodeID);
          }

        // get the display nodes
        vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = NULL;
        vtkMRMLAnnotationTextDisplayNode *textDisplayNode = NULL;
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

  int numberOfMarkups = markupsNode->GetNumberOfMarkups();
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
  for (int n = 0; n < numberOfMarkups; ++n)
    {
    std::string oldLabel = markupsNode->GetNthMarkupLabel(n);
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
      markupsNode->SetNthMarkupLabel(n, std::string(buff));
      }
    else
      {
      // no number in the format, so just rename it
      markupsNode->SetNthMarkupLabel(n, formatString);
      }
    }
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::StartPlaceMode(bool persistent)
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
  vtkMRMLInteractionNode *interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
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
  vtkMRMLSliceCompositeNode* node = 0;
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
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetSliceIntersectionVisibility(flag);
    }
}
