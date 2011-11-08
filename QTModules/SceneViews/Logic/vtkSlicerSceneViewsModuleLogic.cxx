// SlicerLogic includes
#include "vtkSlicerSceneViewsModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLSceneViewStorageNode.h>
#include <vtkMRMLHierarchyNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerSceneViewsModuleLogic, "$Revision: 1.0$")
vtkStandardNewMacro(vtkSlicerSceneViewsModuleLogic)

//-----------------------------------------------------------------------------
// vtkSlicerSceneViewsModuleLogic methods
//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic::vtkSlicerSceneViewsModuleLogic()
{
  this->m_LastAddedSceneViewNode = 0;
  this->ActiveHierarchyNodeID = NULL;
}

//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic::~vtkSlicerSceneViewsModuleLogic()
{
  // let go of pointer to the last added node
  if (this->m_LastAddedSceneViewNode)
    {
    this->m_LastAddedSceneViewNode = 0;
    }

  if (this->ActiveHierarchyNodeID != NULL)
    {
    delete [] this->ActiveHierarchyNodeID;
    this->ActiveHierarchyNodeID = NULL;
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkDebugMacro("SetMRMLSceneInternal - listening to scene events");
  
  vtkIntArray *events = vtkIntArray::New();
//  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneImportedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
//  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeRestoredEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);
  events->Delete();
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::AddMissingHierarchyNodes()
{
  vtkDebugMacro("AddMissingHierarchyNodes");

  if (!this->GetMRMLScene())
    {
    return;
    }
  
  // don't do anything if the scene is still updating
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetIsUpdating())
    {
    vtkDebugMacro("AddMissingHierarchyNodes: updating, returning");
    return;
    }

  vtkSmartPointer<vtkCollection> sceneViewNodes;
  sceneViewNodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLSceneViewNode"));
  unsigned int numNodes = sceneViewNodes->GetNumberOfItems();
  vtkDebugMacro("AddMissingHierarchyNodes: have " << numNodes << " scene view nodes");
  for (unsigned int n = 0; n < numNodes; n++)
    {
    vtkMRMLSceneViewNode *sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(sceneViewNodes->GetItemAsObject(n));
    vtkMRMLHierarchyNode *hierarchyNode =  NULL;
    hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(sceneViewNode->GetScene(), sceneViewNode->GetID());
    if (!hierarchyNode)
      {
      vtkDebugMacro("AddMissingHierarchyNodes: missing a hierarchy node for scene view node " << sceneViewNode->GetID() << ", adding one");
      int retval = this->AddHierarchyNodeForNode(sceneViewNode);
      if (!retval)
        {
        vtkErrorMacro("AddMissingHierarchyNodes: failed to add a missing a hierarchy node for scene view node " << sceneViewNode->GetID());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");

  // don't do anything if the scene is still updating
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetIsUpdating())
    {
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: updating, returning");
    return;
    }
  
  vtkMRMLSceneViewNode * sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(node);
  if (!sceneViewNode)
    {
    return;
    }
  
  int retval = this->AddHierarchyNodeForNode(sceneViewNode);
  vtkMRMLHierarchyNode* hierarchyNode = NULL;
  if (!retval)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: error adding a hierarchy node for scene view node");
    return;
    }
  hierarchyNode =  vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(sceneViewNode->GetScene(), sceneViewNode->GetID());
  if (!hierarchyNode)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: No hierarchyNode found.")
    return;
    }
  hierarchyNode->SetAssociatedNodeID(sceneViewNode->GetID());
  sceneViewNode->Modified();

  // we pass the hierarchy node along - it includes the pointer to the actual sceneViewNode
  this->AddNodeCompleted(hierarchyNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneImportedEvent()
{
  vtkDebugMacro("OnMRMLSceneImportedEvent");

  // this may have been an imported scene with old style snapshot nodes and no
  // hierarchies, so fill in some hierarchies
  this->AddMissingHierarchyNodes();
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneRestoredEvent()
{
  vtkDebugMacro("OnMRMLSceneRestoredEvent");
}
//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLNodeModified(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLNodeModifiedEvent " << node->GetID());

  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetIsUpdating())
    {
    vtkDebugMacro("OnMRMLNodeModifiedEvent: updating, returning");
    return;
    }
  
  vtkMRMLSceneViewNode * sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(node);
  if (!sceneViewNode)
    {
    return;
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::OnMRMLSceneClosedEvent()
{
  if (this->m_LastAddedSceneViewNode)
    {
    this->m_LastAddedSceneViewNode = 0;
    }

  if (this->GetActiveHierarchyNodeID())
    {
    // this is important: otherwise adding a new scene view node might end up
    // setting it's parent to itself
    this->SetActiveHierarchyNodeID(NULL);
    }
}


//-----------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::RegisterNodes()
{

  if (!this->GetMRMLScene())
    {
    std::cerr << "RegisterNodes: no scene on which to register nodes" << std::endl;
    return;
    }


  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::New();
  this->GetMRMLScene()->RegisterNodeClass(viewNode);
  viewNode->Delete();

  vtkMRMLSceneViewStorageNode *storageNode = vtkMRMLSceneViewStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass ( storageNode );
  storageNode->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  if (!screenshot)
    {
    vtkErrorMacro("CreateSceneView: No screenshot was set.")
    return;
    }

  vtkStdString nameString = vtkStdString(name);

  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::New();
  newSceneViewNode->SetScene(this->GetMRMLScene());
  if (strcmp(nameString,""))
    {
    // a name was specified
    newSceneViewNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    newSceneViewNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneView"));
    }

  vtkStdString descriptionString = vtkStdString(description);

  newSceneViewNode->SetSceneViewDescription(descriptionString);
  newSceneViewNode->SetScreenShotType(screenshotType);

  // make a new vtk image data, as the set macro is taking the pointer
  vtkSmartPointer<vtkImageData> copyScreenShot = vtkSmartPointer<vtkImageData>::New();
  copyScreenShot->DeepCopy(screenshot);
  newSceneViewNode->SetScreenShot(copyScreenShot);
  newSceneViewNode->StoreScene();
  //newSceneViewNode->HideFromEditorsOff();
  // mark it modified since read so that the screen shot will get saved to disk
  newSceneViewNode->ModifiedSinceReadOn();
  
  this->GetMRMLScene()->AddNode(newSceneViewNode);

  // put it in a hierarchy
  if (!this->AddHierarchyNodeForNode(newSceneViewNode))
    {
    vtkErrorMacro("CreateSceneView: Error adding a hierarchy node for new scene view node " << newSceneViewNode->GetID());
    }

  newSceneViewNode->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::ModifySceneView(vtkStdString id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  if (!screenshot)
    {
    vtkErrorMacro("ModifySceneView: No screenshot was set.")
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id.c_str()));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewName: Could not get sceneView node!")
    return;
    }

  vtkStdString nameString = vtkStdString(name);
  if (strcmp(nameString,""))
    {
    // a name was specified
    viewNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    viewNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneView"));
    }

  vtkStdString descriptionString = vtkStdString(description);
  viewNode->SetSceneViewDescription(descriptionString);
  viewNode->SetScreenShotType(screenshotType);
  viewNode->SetScreenShot(screenshot);
  viewNode->ModifiedSinceReadOn();

  viewNode->Modified();
  // TODO: Listen to the node directly, probably in OnMRMLSceneNodeAddedEvent
  this->OnMRMLNodeModified(viewNode);
}

//---------------------------------------------------------------------------
vtkStdString vtkSlicerSceneViewsModuleLogic::GetSceneViewName(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return 0;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewName: Could not get sceneView node!")
    return 0;
    }

  return vtkStdString(viewNode->GetName());
}

//---------------------------------------------------------------------------
vtkStdString vtkSlicerSceneViewsModuleLogic::GetSceneViewDescription(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return 0;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewDescription: Could not get sceneView node!")
    return 0;
    }

  return viewNode->GetSceneViewDescription();
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshotType(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return -1;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshotType: Could not get sceneView node!")
    return -1;
    }

  return viewNode->GetScreenShotType();
}

//---------------------------------------------------------------------------
vtkImageData* vtkSlicerSceneViewsModuleLogic::GetSceneViewScreenshot(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return 0;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshot: Could not get sceneView node!")
    return 0;
    }

  return viewNode->GetScreenShot();
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::RestoreSceneView(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("RestoreSceneView: Could not get sceneView node!")
    return;
    }

  this->GetMRMLScene()->SaveStateForUndo();
  viewNode->RestoreScene();
}

//---------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::MoveSceneViewUp(const char* id)
{
  // reset stringHolder
  this->m_StringHolder = "";

  if (!id)
    {
    return this->m_StringHolder.c_str();
    }

  this->m_StringHolder = id;

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return this->m_StringHolder.c_str();
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("MoveSceneViewUp: Could not get sceneView node! (id = " << id << ")")
    return this->m_StringHolder.c_str();
    }

  // see if it's in a hierarchy
  vtkMRMLHierarchyNode *hNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(this->GetMRMLScene(), id);
  if (!hNode)
    {
    vtkWarningMacro("MoveSceneViewUp: did not find a hierarchy node for node with id " << id);
    return this->m_StringHolder.c_str();
    }
  // where is it in the parent's list?
  int currentIndex = hNode->GetIndexInParent();
  // now move it up one
  hNode->SetIndexInParent(currentIndex - 1);
  // if it succeeded, trigger a modified event on the node to get the GUI to
  // update
  if (hNode->GetIndexInParent() != currentIndex)
    {
    std::cout << "MoveSceneViewUp: calling mod on scene view node" << std::endl;
    viewNode->Modified();
    }
  // the id should be the same now
  this->m_StringHolder = viewNode->GetID();
  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
const char* vtkSlicerSceneViewsModuleLogic::MoveSceneViewDown(const char* id)
{
  // reset stringHolder
  this->m_StringHolder = "";

  if (!id)
    {
    return this->m_StringHolder.c_str();
    }

  this->m_StringHolder = id;


  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return this->m_StringHolder.c_str();
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("MoveSceneViewDown: Could not get sceneView node!")
    return this->m_StringHolder.c_str();
    }

  // see if it's in a hierarchy
  vtkMRMLHierarchyNode *hNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(this->GetMRMLScene(), id);
  if (!hNode)
    {
    vtkWarningMacro("MoveSceneViewDown: Temporarily disabled (did not find a hierarchy node for node with id " << id << ")");
    return this->m_StringHolder.c_str();
    }
  // where is it in the parent's list?
  int currentIndex = hNode->GetIndexInParent();
  // now move it down one
  hNode->SetIndexInParent(currentIndex + 1);
  // the id should be the same now
  this->m_StringHolder = viewNode->GetID();
  return this->m_StringHolder.c_str();
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::RemoveSceneViewNode(vtkMRMLSceneViewNode *sceneViewNode)
{
  if (!sceneViewNode)
    {
    vtkErrorMacro("RemoveSceneViewNode: No node to remove");
    return;
    }

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RemoveSceneViewNode: No MRML Scene found from which to remove the node");
    return;
    }

  // remove the 1-1 IS-A hierarchy node first
  vtkMRMLHierarchyNode *hNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(this->GetMRMLScene(), sceneViewNode->GetID());
  if (hNode)
    {
    // there is a parent node
    this->GetMRMLScene()->RemoveNode(hNode);
    }

  this->GetMRMLScene()->RemoveNode(sceneViewNode);

}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::AddHierarchyNodeForNode(vtkMRMLNode* node)
{
  // check that there isn't already a hierarchy node for this node
  if (node && node->GetScene() && node->GetID())
    {
    vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(node->GetScene(), node->GetID());
    if (hnode != NULL)
      {
      vtkWarningMacro("AddHierarchyNodeForNode: scene view node " << node->GetID() << " already has a hierarchy node, returning.");
      return true;
      }
    }
  if (!this->GetActiveHierarchyNodeID())
    {
    vtkDebugMacro("AddHierarchyNodeForNode: no active hierarchy...");
    // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
    char * toplevelHierarchyNodeID = NULL;
    if (!node)
      {
      // we just add a new toplevel hierarchy node
      toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(0);
      }
    else
      {
      // we need to insert the new toplevel hierarchy before the given node
      toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(node);
      }

    if (!toplevelHierarchyNodeID)
      {
      vtkErrorMacro("AddHierarchyNodeForNode: Toplevel hierarchy node was NULL.")
      return 0;
      }

    this->SetActiveHierarchyNodeID(toplevelHierarchyNodeID);
    }

  // Create a hierarchy node
  vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::New();
  if (hierarchyNode == NULL)
    {
    vtkErrorMacro("AddHierarchyNodeForNode: can't create a new hierarchy node to associate with scene view " << node->GetID());
    return 0;
    }

  hierarchyNode->SetParentNodeID(this->GetActiveHierarchyNodeID());
  hierarchyNode->SetScene(this->GetMRMLScene());

  if (!node)
    {
    // this is a user created hierarchy!

    // we want to see that!
    hierarchyNode->HideFromEditorsOff();

    hierarchyNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneViewHierarchy"));

    this->GetMRMLScene()->AddNode(hierarchyNode);

    // we want it to be the active hierarchy from now on
    this->SetActiveHierarchyNodeID(hierarchyNode->GetID());
    }
  else
    {
    // this is the 1-1 hierarchy node for a given node

    // we do not want to see that!
    hierarchyNode->HideFromEditorsOn();

    hierarchyNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneViewHierarchy"));

    this->GetMRMLScene()->InsertBeforeNode(node,hierarchyNode);
    hierarchyNode->SetAssociatedNodeID(node->GetID());
    vtkDebugMacro("AddHierarchyNodeForNode: added hierarchy node, id = " << (hierarchyNode->GetID() ? hierarchyNode->GetID() : "null") << ", set associated node id on the hierarchy node of " << (hierarchyNode->GetAssociatedNodeID() ? hierarchyNode->GetAssociatedNodeID() : "null"));
    }
  
  hierarchyNode->Delete();
  return 1;
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewsModuleLogic::AddHierarchy()
{
  return this->AddHierarchyNodeForNode(0);
}

//---------------------------------------------------------------------------
char * vtkSlicerSceneViewsModuleLogic::GetTopLevelHierarchyNodeID(vtkMRMLNode* node)
{

  if (this->GetMRMLScene() == NULL)
    {
    return NULL;
    }
  const char *toplevelName = "SceneViewToplevelHierarchyNode";
  char *toplevelNodeID = NULL;
  //vtkMRMLHierarchyNode* toplevelNode = vtkMRMLHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLHierarchyNode"));
  vtkSmartPointer<vtkCollection> col;
  col.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLHierarchyNode"));
  vtkMRMLHierarchyNode* toplevelNode = NULL;
  unsigned int numNodes = col->GetNumberOfItems();
  if (numNodes != 0)
    {
    //std::cout << "Found " << numNodes << " hierarchy nodes" << std::endl;
    // iterate through the hierarchy nodes to find one with a name starting
    // with the top level name
    for (unsigned int n = 0; n < numNodes; n++)
      {
      vtkMRMLNode *thisNode = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(n));
      //std::cout << "\tthisNode name = '" << thisNode->GetName() << "', toplevelname = '" << toplevelName << "', strlen = " << strlen(toplevelName) << std::endl;
      if (thisNode && thisNode->GetName() &&
          strncmp(thisNode->GetName(), toplevelName, strlen(toplevelName)) == 0)
        {
        toplevelNode = vtkMRMLHierarchyNode::SafeDownCast(col->GetItemAsObject(n));
        //std::cout << "\tfound matching hierarchy node at index " << n << ", named " << toplevelNode->GetName() << std::endl;
        break;
        }
      }
    }
  if (!toplevelNode)
    {
    //std::cout << "GetTopLevelHierarchyNode: no top level node, making new" << std::endl;
    // no hierarchy node is currently in the scene, create a new one
    toplevelNode = vtkMRMLHierarchyNode::New();
    toplevelNode->HideFromEditorsOff();
    toplevelNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(toplevelName));

    if (!node)
      {
      this->GetMRMLScene()->AddNode(toplevelNode);
      }
    else
      {
      this->GetMRMLScene()->InsertBeforeNode(node,toplevelNode);
      }
    toplevelNodeID = toplevelNode->GetID();
    toplevelNode->Delete();
    }
  else
    {
    toplevelNodeID = toplevelNode->GetID();
    }
  return toplevelNodeID;
}

//--------------------------------------------------------------------------- 
vtkMRMLHierarchyNode * vtkSlicerSceneViewsModuleLogic::GetActiveHierarchyNode()
{
  if (!this->GetActiveHierarchyNodeID())
    {
    // there was no active hierarchy
    // we then use the toplevel hierarchyNode
    char* toplevelNodeID = this->GetTopLevelHierarchyNodeID();

    if (!toplevelNodeID)
      {
      vtkErrorMacro("SetActiveHierarchyNodeByID: Could not find or create any hierarchy.")
      return NULL;
      }

    this->SetActiveHierarchyNodeID(toplevelNodeID);
    }
  if (this->GetMRMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()) == NULL)
    {
    return NULL;
    }
  return vtkMRMLHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()));
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewsModuleLogic::AddNodeCompleted(vtkMRMLHierarchyNode* hierarchyNode)
{

  if (!hierarchyNode)
    {
    return;
    }

  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(hierarchyNode->GetAssociatedNode());

  if (!sceneViewNode)
    {
    vtkErrorMacro("AddNodeCompleted: Could not get scene view node.")
    return;
    }

  this->m_LastAddedSceneViewNode = sceneViewNode;

}
