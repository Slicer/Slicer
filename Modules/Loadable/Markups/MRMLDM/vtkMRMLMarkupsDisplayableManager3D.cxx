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

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager3D.h"

// MRMLDisplayableManager includes
#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkHandleRepresentation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPropCollection.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkWidgetRepresentation.h>
#include <vtkGeneralTransform.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

typedef void (*fp)(void);

#define NUMERIC_ZERO 0.001

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsDisplayableManager3D);

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager3D::vtkMRMLMarkupsDisplayableManager3D()
{
  this->Helper = vtkMRMLMarkupsDisplayableManagerHelper::New();
  this->ClickCounter = vtkMRMLMarkupsClickCounter::New();
  this->DisableInteractorStyleEventsProcessing = 0;
  this->Updating = 0;

  this->Focus = "vtkMRMLMarkupsNode";

  this->LastClickWorldCoordinates[0]=0.0;
  this->LastClickWorldCoordinates[1]=0.0;
  this->LastClickWorldCoordinates[2]=0.0;
  this->LastClickWorldCoordinates[3]=1.0;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager3D::~vtkMRMLMarkupsDisplayableManager3D()
{

  this->DisableInteractorStyleEventsProcessing = 0;
  this->Updating = 0;
  this->Focus = 0;

  this->Helper->Delete();
  this->ClickCounter->Delete();

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "DisableInteractorStyleEventsProcessing = " << this->DisableInteractorStyleEventsProcessing << std::endl;
  if (this->Focus)
    {
    os << indent << "Focus = " << this->Focus << std::endl;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::SetAndObserveNode(vtkMRMLMarkupsNode *markupsNode)
{
  if (!markupsNode)
    {
    return;
    }
  vtkNew<vtkIntArray> nodeEvents;
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::PointModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::NthMarkupModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::MarkupAddedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::MarkupRemovedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLDisplayableNode::DisplayModifiedEvent);

 if (markupsNode)// && !markupsNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent))
   {
   vtkUnObserveMRMLNodeMacro(markupsNode);
   vtkObserveMRMLNodeEventsMacro(markupsNode, nodeEvents.GetPointer());
   }
}
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::SetAndObserveNodes()
{



  // run through all associated nodes
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  for(it = this->Helper->MarkupsNodeList.begin();
      it != this->Helper->MarkupsNodeList.end();
      ++it)
    {
    vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast((*it));
    this->SetAndObserveNode(markupsNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::AddObserversToInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // also observe the interaction node for changes
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    vtkDebugMacro("AddObserversToInteractionNode: interactionNode found");
    vtkIntArray *interactionEvents = vtkIntArray::New();
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModeChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::EndPlacementEvent);
    vtkObserveMRMLNodeEventsMacro(interactionNode, interactionEvents);
    interactionEvents->Delete();
    }
  else { vtkDebugMacro("AddObserversToInteractionNode: No interaction node!"); }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::RemoveObserversFromInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  // find the interaction node
  vtkMRMLInteractionNode *interactionNode =  this->GetInteractionNode();
  if (interactionNode)
    {
    vtkUnObserveMRMLNodeMacro(interactionNode);
    }
}
/*
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::Create()
{

  // hack to force initialization of the renderview
  //this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
  //this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
  this->UpdateFromMRML();
  //this->DebugOn();
}
*/
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::RequestRender()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  if (!this->GetMRMLScene()->IsBatchProcessing())
    {
    this->Superclass::RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::RemoveMRMLObservers()
{
  // run through all associated nodes
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  it = this->Helper->MarkupsNodeList.begin();
  while(it != this->Helper->MarkupsNodeList.end())
    {
    vtkUnObserveMRMLNodeMacro(*it);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == NULL || this->Focus == NULL)
    {
    return;
    }
  // check if there are any of these nodes in the scene
  if (this->GetMRMLScene()->GetNumberOfNodesByClass(this->Focus) < 1)
    {
    return;
    }
  // loop over the nodes for which this manager provides widgets
  this->GetMRMLScene()->InitTraversal();
  vtkMRMLNode *node = this->GetMRMLScene()->GetNextNodeByClass(this->Focus);
  // turn off update from mrml requested, as we're doing it now, and create
  // widget requests a render which checks this flag before calling update
  // from mrml again
  this->SetUpdateFromMRMLRequested(0);
  while (node != NULL)
    {
    vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
    if (markupsNode)
      {
      // do we  have a widget for it?
      if (this->GetWidget(markupsNode) == NULL)
        {
        vtkDebugMacro("UpdateFromMRML: creating a widget for node " << markupsNode->GetID());
        vtkAbstractWidget *widget = this->AddWidget(markupsNode);
        if (widget)
          {
          // update the new widget from the node
          //this->PropagateMRMLToWidget(markupsNode, widget);
          }
        else
          {
          vtkErrorMacro("UpdateFromMRML: failed to create a widget for node " << markupsNode->GetID());
          }
        }
      }
    node = this->GetMRMLScene()->GetNextNodeByClass(this->Focus);
    }
  // set up observers on all the nodes
//  this->SetAndObserveNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnMRMLSceneEndClose();

  if (newScene)
    {
    this->AddObserversToInteractionNode();
    }
  else
    {
    // there's no scene to get the interaction node from, so this won't do anything
    this->RemoveObserversFromInteractionNode();
    }
  vtkDebugMacro("SetMRMLSceneInternal: add observer on interaction node now?");

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D
::ProcessMRMLNodesEvents(vtkObject *caller,unsigned long event,void *callData)
{

  vtkMRMLMarkupsNode * markupsNode = vtkMRMLMarkupsNode::SafeDownCast(caller);
  vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(caller);
  vtkMRMLInteractionNode * interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
  int *nPtr = NULL;
  int n = -1;
  if (callData != 0)
    {
    nPtr = reinterpret_cast<int *>(callData);
    if (nPtr)
      {
      n = *nPtr;
      }
    }
  if (markupsNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLMarkupsNodeModifiedEvent(markupsNode);
        break;
      case vtkMRMLMarkupsNode::PointModifiedEvent:
        this->OnMRMLMarkupsPointModifiedEvent(markupsNode, n);
        break;
      case vtkMRMLMarkupsNode::NthMarkupModifiedEvent:
        this->OnMRMLMarkupsNodeNthMarkupModifiedEvent(markupsNode, n);
        break;
      case vtkMRMLMarkupsNode::MarkupAddedEvent:
        this->OnMRMLMarkupsNodeMarkupAddedEvent(markupsNode);
        break;
      case vtkMRMLMarkupsNode::MarkupRemovedEvent:
        this->OnMRMLMarkupsNodeMarkupRemovedEvent(markupsNode);
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLMarkupsNodeTransformModifiedEvent(markupsNode);
        break;
      case vtkMRMLMarkupsNode::LockModifiedEvent:
        this->OnMRMLMarkupsNodeLockModifiedEvent(markupsNode);
        break;
      case vtkMRMLDisplayableNode::DisplayModifiedEvent:
        // get the display node and process the change
        vtkMRMLNode *displayNode = markupsNode->GetDisplayNode();
        this->OnMRMLMarkupsDisplayNodeModifiedEvent(displayNode);
        break;
      }
    }
  else if (displayNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLMarkupsDisplayNodeModifiedEvent(displayNode);
        break;
      }
    }
  else if (interactionNode)
    {
    if (event == vtkMRMLInteractionNode::EndPlacementEvent)
      {
      // remove all seeds and reset the clickcounter
      this->ClickCounter->Reset();
      this->Helper->RemoveSeeds();
      return;
      }
    else if (event == vtkMRMLInteractionNode::InteractionModeChangedEvent)
      {
      // always update lock if the mode changed, even if this isn't the displayable manager
      // for the markups that is getting placed, but don't update locking on persistence changed event
      this->Helper->UpdateLockedAllWidgetsFromInteractionNode(interactionNode);
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLSceneEndClose()
{
  vtkDebugMacro("OnMRMLSceneEndClose: remove observers?");
  // run through all nodes and remove node and widget
  this->Helper->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::UpdateFromMRMLScene()
{
  this->UpdateFromMRML();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
    {
    return;
    }

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");

  // if the scene is still updating, jump out
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }

  if (node->IsA("vtkMRMLInteractionNode"))
    {
    this->AddObserversToInteractionNode();
    return;
    }

  if (node->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    // have a display node, need to observe it
    vtkObserveMRMLNodeMacro(node);
    return;
    }

  if (!node->IsA(this->Focus))
    {
    // jump out
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Not the correct displayableManager for node " << node->GetID() << ", jumping out!")
    // also delete potential seeds
    this->ClickCounter->Reset();
    this->Helper->RemoveSeeds();
    return;
    }

  vtkMRMLMarkupsNode * markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent:  node " << node->GetID());

  // Node added should not be already managed
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it = std::find(
      this->Helper->MarkupsNodeList.begin(),
      this->Helper->MarkupsNodeList.end(),
      markupsNode);
  if (it != this->Helper->MarkupsNodeList.end())
    {
      vtkErrorMacro("OnMRMLSceneNodeAddedEvent: This node is already associated to the displayable manager!")
      return;
    }

  // There should not be a widget for the new node
  if (this->Helper->GetWidget(markupsNode) != 0)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: A widget is already associated to this node!");
    return;
    }

  //std::cout << "OnMRMLSceneNodeAddedEvent ThreeD -> CreateWidget" << std::endl;

  // Create the Widget and add it to the list.
  vtkAbstractWidget* newWidget = this->AddWidget(markupsNode);
  if (!newWidget)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    return;
    }
  else
    {
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: widget was created, saved to helper Widgets map");
    }

  // tear down widget creation
  //this->OnWidgetCreated(newWidget, markupsNode);

  // Remove all placed seeds
  this->Helper->RemoveSeeds();

  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  this->Helper->UpdateLockedAllWidgetsFromInteractionNode(interactionNode);

  // and render again after seeds were removed
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeRemovedEvent");
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  // Remove the widget and the MRMLnode from the internal lists.
  this->Helper->RemoveWidgetAndNode(markupsNode);

  // Refresh observers
  vtkUnObserveMRMLNodeMacro(markupsNode);

  // and render again after seeds were removed
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLMarkupsNodeModifiedEvent(vtkMRMLNode* node)
{
  //this->DebugOn();

  vtkDebugMacro("OnMRMLMarkupsNodeModifiedEvent");

  if (this->Updating)
    {
    vtkDebugMacro("OnMRMLMarkupsNodeModifiedEvent: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeModifiedEvent: Can not access node.")
    return;
    }

  //std::cout << "OnMRMLMarkupsNodeModifiedEvent ThreeD->PropagateMRMLToWidget" << std::endl;

  vtkAbstractWidget * widget = this->Helper->GetWidget(markupsNode);

  if (widget)
    {
    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLMarkupsDisplayNodeModifiedEvent(vtkMRMLNode* node)
{
  if (!node)
    {
    vtkErrorMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: no node!");
    return;
    }

  //this->DebugOn();

  if (this->Updating)
    {
    vtkDebugMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLMarkupsDisplayNode *markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(node);
  if (!markupsDisplayNode)
    {
    vtkErrorMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: Can not access node.")
    return;
    }

  // find the markups node that has this display node
  vtkMRMLMarkupsNode *markupsNode = this->Helper->GetMarkupsNodeFromDisplayNode(markupsDisplayNode);

  if (!markupsNode)
    {
    return;
    }

  vtkDebugMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: found the markups node "
        << markupsNode->GetID()
        << " associated with the modified display node "
        << markupsDisplayNode->GetID());
  vtkAbstractWidget * widget = this->Helper->GetWidget(markupsNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(markupsNode, widget);

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLMarkupsPointModifiedEvent(vtkMRMLNode *node, int n)
{
  vtkDebugMacro("OnMRMLMarkupsPointModifiedEvent");
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }
  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (widget)
    {
    // Update the standard settings of all widgets.
    this->UpdateNthSeedPositionFromMRML(n, widget, markupsNode);

    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    this->RequestRender();
    }
}



//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLMarkupsNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLMarkupsNodeTransformModifiedEvent");
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeTransformModifiedEvent - Can not access node.")
    return;
    }

  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (widget)
    {
    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLMarkupsNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLMarkupsNodeLockModifiedEvent");
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeLockModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateLocked(markupsNode, this->GetInteractionNode());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{

  vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent");

  if (!caller)
    {
    vtkErrorMacro("OnMRMLDisplayableNodeModifiedEvent: Could not get caller.")
    return;
    }

  vtkMRMLViewNode * viewNode = vtkMRMLViewNode::SafeDownCast(caller);

  if (viewNode)
    {
    // the associated renderWindow is a 3D View
    vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent: This displayableManager handles a ThreeD view.")
    return;
    }

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::UpdateWidgetVisibility(vtkMRMLMarkupsNode* markupsNode)
{
//  std::cout << "UpdateWidgetVisibility" << std::endl;
  if (!markupsNode)
    {
    vtkErrorMacro("UpdateWidgetVisibility: no markups node from which to work!");
    return;
    }

   vtkAbstractWidget* widget = this->Helper->GetWidget(markupsNode);

   if (!widget)
     {
     vtkErrorMacro("UpdateWidgetVisibility: We could not get the widget to the node: " << markupsNode->GetID());
     return;
     }

   // check if the markups node is visible according to the current mrml state
   vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
   bool visibleOnNode = true;
   if (displayNode)
     {
     vtkMRMLViewNode *viewNode = this->GetMRMLViewNode();
     if (viewNode)
       {
       // use the view information to get the visibility
       visibleOnNode = (displayNode->GetVisibility(viewNode->GetID()) == 1 ? true : false);
       }
     else
       {
       visibleOnNode = (displayNode->GetVisibility() == 1 ? true : false);
       }
     }
   // check if the widget is visible according to the widget state
   bool visibleOnWidget = (widget->GetEnabled() == 1 ? true : false);

   // only update the visibility of the widget if it is different than on the node
   // first case: the node says it is not visible, but the widget is
   if (!visibleOnNode && visibleOnWidget)
     {
     // hide the widget immediately
     widget->SetEnabled(0);
     vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
     if (seedWidget)
       {
       seedWidget->CompleteInteraction();
       vtkDebugMacro("UpdateWidgetVisibility: complete interaction");
       }
     }
   // second case: the node says it is visible, but the widget is not
   else if (visibleOnNode && !visibleOnWidget)
     {
     widget->SetEnabled(1);
     //this->PropagateMRMLToWidget(markupsNode, widget);
     vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
     if (seedWidget)
       {
       seedWidget->CompleteInteraction();
       vtkDebugMacro("UpdateWidgetVisibility: complete interaction");
       }
     }

   // it's visible, just update the position
   //  this->UpdatePosition(widget, markupsNode);

}


//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnInteractorStyleEvent(int eventid)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }

  if (!this->IsCorrectDisplayableManager())
    {
    //std::cout << "Markups DisplayableManger: OnInteractorStyleEvent : " << this->Focus << ", not correct displayable manager, returning" << std::endl;
    return;
    }
  vtkDebugMacro("OnInteractorStyleEvent " << this->Focus << " " << eventid);

  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      this->OnClickInRenderWindowGetCoordinates();
      }
//    else { vtkWarningMacro("OnInteractorStyleEvent: not in vtkMRMLInteractionNode::Place interaction mode"); }
    }
  else if (eventid == vtkCommand::LeftButtonPressEvent)
    {
//    vtkWarningMacro("OnInteractorStyleEvent: unhandled left button press event " << eventid);
    }
  else if (eventid == vtkCommand::RightButtonReleaseEvent)
    {
    // if we're in persistent place mode, go back to view transform mode, but
    // leave the persistent flag on
    // Note: this is currently only implemented in 3D as the right click is used
    // in ctkQImageView
    // for zooming.
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
        this->GetInteractionNode()->GetPlaceModePersistence() == 1)
      {
      this->GetInteractionNode()->SwitchToViewTransformMode();
      }
    }
  else
    {
    //vtkWarningMacro("OnInteractorStyleEvent: unhandled event " << eventid);
    //std::cout << "Markups DisplayableManager: OnInteractorStyleEvent: unhandled event " << eventid << std::endl;
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManager3D::GetWidget(vtkMRMLMarkupsNode * node)
{
  return this->Helper->GetWidget(node);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnClickInRenderWindowGetCoordinates()
{

  double x = this->GetInteractor()->GetEventPosition()[0];
  double y = this->GetInteractor()->GetEventPosition()[1];

  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (x < windowWidth && y < windowHeight)
    {
    const char *associatedNodeID = NULL;
    // it's a 3D displayable manager and the click could have been on a node
    vtkMRMLModelDisplayableManager * modelDisplayableManager =
      vtkMRMLModelDisplayableManager::SafeDownCast(this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName("vtkMRMLModelDisplayableManager"));
    double yNew = windowHeight - y - 1;
    if (modelDisplayableManager &&
        modelDisplayableManager->Pick(x,yNew) &&
        strcmp(modelDisplayableManager->GetPickedNodeID(),"") != 0)
      {
      // find the node id, the picked node name is probably the display node
      const char *pickedNodeID = modelDisplayableManager->GetPickedNodeID();
      vtkDebugMacro("Click was on model " << pickedNodeID);
      vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(pickedNodeID);
      vtkMRMLDisplayNode *displayNode = NULL;
      if (mrmlNode)
        {
        vtkDebugMacro("Got a mrml node by name, id = " << mrmlNode->GetID());
        displayNode = vtkMRMLDisplayNode::SafeDownCast(mrmlNode);
        }
      else
        {
        vtkDebugMacro("couldn't find a mrml node with ID " << pickedNodeID);
        }
      if (displayNode)
        {
        vtkDebugMacro("Got display node for picked node name " << displayNode->GetID());
        vtkMRMLDisplayableNode *displayableNode = displayNode->GetDisplayableNode();
        if (displayableNode)
          {
          // start with the assumption that it's a generic displayable node,
          // then look for it to be a slice node that has the string
          // CompositeID in it's Description with a valid mrml node id after it
          associatedNodeID = displayableNode->GetID();
          // it might be a slice node, check the Description field for the string CompositeID
          if (displayableNode->GetDescription())
            {
            std::string desc = displayableNode->GetDescription();
            size_t ptr = desc.find("CompositeID");
            // does it have the string CompositeID in the description with
            // something after it?
            vtkDebugMacro("Desc len = " << desc.length() << ", ptr = " << ptr);
            if (ptr != std::string::npos &&
                (desc.length() > (ptr + 12)))
              {
              std::string compID = desc.substr(ptr + 12);
              vtkDebugMacro("Found composite node id = " << compID.c_str());
              vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(compID.c_str());
              // was this a valid composite node id?
              if (mrmlNode)
                {
                vtkMRMLSliceCompositeNode* sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(mrmlNode);
                if (sliceCompositeNode)
                  {
                  if (sliceCompositeNode->GetBackgroundVolumeID())
                    {
                    associatedNodeID = sliceCompositeNode->GetBackgroundVolumeID();
                    }
                  else if (sliceCompositeNode->GetForegroundVolumeID())
                    {
                    associatedNodeID = sliceCompositeNode->GetForegroundVolumeID();
                    }
                  else if (sliceCompositeNode->GetLabelVolumeID())
                    {
                    associatedNodeID = sliceCompositeNode->GetLabelVolumeID();
                    }
                  vtkDebugMacro("Markups was placed on a 3d slice model, found the volume id " << associatedNodeID);
                  }
                }
              }
            }
          }
        }
      }
    vtkDebugMacro("associatedNodeID set to " << (associatedNodeID ? associatedNodeID : "NULL"));
    this->OnClickInRenderWindow(x, y, associatedNodeID);
    //this->Helper->UpdateLockedAllWidgetsFromNodes();
    }
}


//---------------------------------------------------------------------------
// Placement of widgets through seeds
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// Place a seed for widgets
void vtkMRMLMarkupsDisplayableManager3D::PlaceSeed(double x, double y)
{

  // place the seed
  this->Helper->PlaceSeed(x,y,this->GetInteractor(),this->GetRenderer());

  this->RequestRender();

}

//---------------------------------------------------------------------------
/// Get the handle of a placed seed
vtkHandleWidget * vtkMRMLMarkupsDisplayableManager3D::GetSeed(int index)
{
  return this->Helper->GetSeed(index);
}

//---------------------------------------------------------------------------
// Coordinate conversions
//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLMarkupsDisplayableManager3D::GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates)
{

  // for 3D, we want to convert the coordinates using the pick function

  // ModelDisplayableManager is expected to be instantiated !
  vtkMRMLModelDisplayableManager * modelDisplayableManager =
      vtkMRMLModelDisplayableManager::SafeDownCast(
        this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
          "vtkMRMLModelDisplayableManager"));
  assert(modelDisplayableManager);

  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  double yNew = windowHeight - y - 1;

  if (modelDisplayableManager->Pick(x,yNew))
    {
    double* pickedWorldCoordinates = modelDisplayableManager->GetPickedRAS();
    worldCoordinates[0] = pickedWorldCoordinates[0];
    worldCoordinates[1] = pickedWorldCoordinates[1];
    worldCoordinates[2] = pickedWorldCoordinates[2];
    worldCoordinates[3] = 1;
    }
  else
    {
    // we could not pick so just convert to world coordinates
    vtkInteractorObserver::ComputeDisplayToWorld(this->GetRenderer(),x,y,0,worldCoordinates);
    }

}

//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLMarkupsDisplayableManager3D::GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates)
{

  this->GetDisplayToWorldCoordinates(displayCoordinates[0], displayCoordinates[1], worldCoordinates);

}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLMarkupsDisplayableManager3D::GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates)
{
  vtkInteractorObserver::ComputeWorldToDisplay(this->GetRenderer(),r,a,s,displayCoordinates);
}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLMarkupsDisplayableManager3D::GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates)
{
  if (worldCoordinates == NULL)
    {
    return;
    }
  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);

}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLMarkupsDisplayableManager3D::GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates)
{

  if (viewportCoordinates == NULL)
    {
    return;
    }
  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (windowWidth != 0.0)
    {
    viewportCoordinates[0] = x/windowWidth;
    }
  if (windowHeight != 0.0)
    {
    viewportCoordinates[1] = y/windowHeight;
    }
  vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x
        << ", y = " << y
        << ", returning viewport = "
        << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLMarkupsDisplayableManager3D::GetDisplayToViewportCoordinates(double * displayCoordinates, double * viewportCoordinates)
{
  if (displayCoordinates && viewportCoordinates)
    {
    this->GetDisplayToViewportCoordinates(displayCoordinates[0], displayCoordinates[1], viewportCoordinates);
    }
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLMarkupsDisplayableManager3D::GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2)
{
  bool changed = false;

  if (sqrt( ( displayCoordinates1[0] - displayCoordinates2[0] ) * ( displayCoordinates1[0] - displayCoordinates2[0] )
           + ( displayCoordinates1[1] - displayCoordinates2[1] ) * ( displayCoordinates1[1] - displayCoordinates2[1] ))>1.0)
    {
    changed = true;
    }

  return changed;
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLMarkupsDisplayableManager3D::GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2)
{
  bool changed = false;

  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  // TODO find a better value?
  // - use a smaller number to make fiducial seeding more smooth
  if (distance > VTK_DBL_EPSILON)
    {
    changed = true;
    }

  return changed;
}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager3D::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = this->GetMRMLApplicationLogic()->GetSelectionNode();
  if ( selectionNode == 0 )
    {
    vtkErrorMacro ( "IsCorrectDisplayableManager: No selection node in the scene." );
    return false;
    }
  if ( selectionNode->GetActivePlaceNodeClassName() == 0)
    {
    //vtkErrorMacro ( "IsCorrectDisplayableManager: no active markups");
    return false;
    }
  // the purpose of the displayableManager is hardcoded
  return this->IsManageable(selectionNode->GetActivePlaceNodeClassName());
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager3D::IsManageable(vtkMRMLNode* node)
{
  return node->IsA(this->Focus);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager3D::IsManageable(const char* nodeClassName)
{
  return nodeClassName && !strcmp(nodeClassName, this->Focus);
}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnClickInRenderWindow(double vtkNotUsed(x), double vtkNotUsed(y), const char * vtkNotUsed(associatedNodeID))
{

  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkMRMLMarkupsDisplayableManager3D::CreateWidget(vtkMRMLMarkupsNode* vtkNotUsed(node))
{

  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::OnWidgetCreated(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLMarkupsNode* vtkNotUsed(node))
{

  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::PropagateMRMLToWidget(vtkMRMLMarkupsNode* vtkNotUsed(node), vtkAbstractWidget* vtkNotUsed(widget))
{
  // update the widget visibility according to the mrml settings of visibility
  // status for 2d (taking into account the current slice) and 3d
  //this->UpdateWidgetVisibility(markupsNode);
  // The properties of a widget should be set here, subclasses should call this.
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager3D::PropagateWidgetToMRML(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLMarkupsNode* vtkNotUsed(node))
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}

//---------------------------------------------------------------------------
/// Convert world coordinates to local using mrml parent transform
void vtkMRMLMarkupsDisplayableManager3D::GetWorldToLocalCoordinates(vtkMRMLMarkupsNode *node,
                                                                     double *worldCoordinates,
                                                                     double *localCoordinates)
{
  if (node == NULL)
    {
    vtkErrorMacro("GetWorldToLocalCoordinates: node is null");
    return;
    }

  for (int i=0; i<3; i++)
    {
    localCoordinates[i] = worldCoordinates[i];
    }

  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();
  vtkGeneralTransform *transformToWorld = vtkGeneralTransform::New();
  transformToWorld->Identity();
  if (tnode != 0 && !tnode->IsTransformToWorldLinear())
    {
    tnode->GetTransformFromWorld(transformToWorld);
    }
  else if (tnode != NULL && tnode->IsTransformToWorldLinear())
    {
    vtkNew<vtkMatrix4x4> matrixTransformToWorld;
    matrixTransformToWorld->Identity();
    tnode->GetMatrixTransformToWorld(matrixTransformToWorld.GetPointer());
    matrixTransformToWorld->Invert();
    transformToWorld->Concatenate(matrixTransformToWorld.GetPointer());
  }

  double p[4];
  p[3] = 1;
  int i;
  for (i=0; i<3; i++)
    {
    p[i] = worldCoordinates[i];
    }
  double *xyz = transformToWorld->TransformDoublePoint(p);
  for (i=0; i<3; i++)
    {
    localCoordinates[i] = xyz[i];
    }
  transformToWorld->Delete();
}

//---------------------------------------------------------------------------
/// Create a new widget for this markups node and save it to the helper.
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManager3D::AddWidget(vtkMRMLMarkupsNode *markupsNode)
{
  vtkDebugMacro("AddWidget: calling create widget");
  vtkAbstractWidget* newWidget = this->CreateWidget(markupsNode);
  if (!newWidget)
    {
    vtkErrorMacro("AddWidget: unable to create a new widget for markups node " << markupsNode->GetID());
    return NULL;
    }

  // record the mapping between node and widget in the helper
  this->Helper->RecordWidgetForNode(newWidget,markupsNode);

  vtkDebugMacro("AddWidget: saved to helper ");
//  vtkIndent indent;
//  this->Helper->PrintSelf(std::cout, indent);

  // Refresh observers
  this->SetAndObserveNode(markupsNode);

  // TODO do we need this render call?
  this->RequestRender();

  // ?
  this->OnWidgetCreated(newWidget, markupsNode);

  // now set up the new widget
  this->PropagateMRMLToWidget(markupsNode, newWidget);

  return newWidget;
}
