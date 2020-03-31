
// Annotation MRML includes
#include <vtkMRMLAnnotationControlPointsNode.h>
#include <vtkMRMLAnnotationDisplayNode.h>
#include <vtkMRMLAnnotationFiducialNode.h>
#include <vtkMRMLAnnotationLineDisplayNode.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationPointDisplayNode.h>
#include <vtkMRMLAnnotationRulerNode.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationClickCounter.h"
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkMRMLAnnotationDisplayableManagerHelper.h"

// Annotation VTKWidget
#include <vtkAnnotationGlyphSource2D.h>

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractSliceViewDisplayableManager.h>
#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLInteractionEventData.h>
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
#include <vtkHandleWidget.h>
#include <vtkLineWidget2.h>
#include <vtkLineRepresentation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkPropCollection.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkWidgetRepresentation.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

typedef void (*fp)();

#define NUMERIC_ZERO 0.001

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationDisplayableManager);

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::vtkMRMLAnnotationDisplayableManager()
{
  this->Helper = vtkMRMLAnnotationDisplayableManagerHelper::New();
  this->m_ClickCounter = vtkMRMLAnnotationClickCounter::New();
  this->DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;

  this->m_Focus = "vtkMRMLAnnotationNode";

  // by default, multiply the display node scale by this when setting scale on elements in 2d windows
  this->ScaleFactor2D = 0.01667;

  this->LastClickWorldCoordinates[0]=0;
  this->LastClickWorldCoordinates[1]=0;
  this->LastClickWorldCoordinates[2]=0;
  this->LastClickWorldCoordinates[3]=1;
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::~vtkMRMLAnnotationDisplayableManager()
{

  this->DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;
  this->m_Focus = nullptr;

  this->Helper->Delete();
  this->m_ClickCounter->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "DisableInteractorStyleEventsProcessing = " << this->DisableInteractorStyleEventsProcessing << std::endl;
  if (this->GetSliceNode() &&
      this->GetSliceNode()->GetID())
    {
    os << indent << "Slice node id = " << this->GetSliceNode()->GetID() << std::endl;
    }
  else
    {
    os << indent << "No slice node" << std::endl;
    }
  if (this->m_Focus)
    {
    os << indent << "Focus = " << this->m_Focus << std::endl;
    }
  os << indent << "ScaleFactor2D = " << this->ScaleFactor2D << std::endl;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::SetAndObserveNode(vtkMRMLAnnotationNode *annotationNode)
{
  if (!annotationNode)
    {
    return;
    }
  vtkNew<vtkIntArray> nodeEvents;
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLAnnotationNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLDisplayableNode::DisplayModifiedEvent);

 if (annotationNode)// && !annotationNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent))
   {
   vtkUnObserveMRMLNodeMacro(annotationNode);
   vtkObserveMRMLNodeEventsMacro(annotationNode, nodeEvents.GetPointer());
   }
}
//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::SetAndObserveNodes()
{



  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  for(it = this->Helper->AnnotationNodeList.begin();
      it != this->Helper->AnnotationNodeList.end();
      ++it)
    {
    vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast((*it));
    this->SetAndObserveNode(annotationNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::AddObserversToInteractionNode()
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
      vtkNew<vtkIntArray> interactionEvents;
      interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModeChangedEvent);
      interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent);
      interactionEvents->InsertNextValue(vtkMRMLInteractionNode::EndPlacementEvent);
      vtkObserveMRMLNodeEventsMacro(interactionNode, interactionEvents.GetPointer());
    }
  else { vtkDebugMacro("AddObserversToInteractionNode: No interaction node!"); }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::RemoveObserversFromInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  // find the interaction node
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    vtkUnObserveMRMLNodeMacro(interactionNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::AddObserversToSelectionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  vtkMRMLSelectionNode* selectionNode = this->GetSelectionNode();
  if (selectionNode)
    {
    vtkDebugMacro("AddObserversToSelectionNode: selectionNode found");
    vtkNew<vtkIntArray> selectionEvents;
    selectionEvents->InsertNextValue(vtkMRMLSelectionNode::UnitModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(selectionNode, selectionEvents.GetPointer());
    }
  else
    {
    vtkWarningMacro("AddObserversToSelectionNode: No selection node!");
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::RemoveObserversFromSelectionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  vtkMRMLSelectionNode* selectionNode = this->GetSelectionNode();
  if (selectionNode)
    {
    vtkUnObserveMRMLNodeMacro(selectionNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::Create()
{

  // hack to force initialization of the renderview
  //this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
  //this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
  this->UpdateFromMRML();
  //this->DebugOn();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::RequestRender()
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
void vtkMRMLAnnotationDisplayableManager::RemoveMRMLObservers()
{
  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
    vtkUnObserveMRMLNodeMacro(*it);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == nullptr || this->m_Focus == nullptr)
    {
    return;
    }

  // turn off update from mrml requested, as we're doing it now, and create
  // widget requests a render which checks this flag before calling update
  // from mrml again
  this->SetUpdateFromMRMLRequested(false);

  // loop over the nodes for which this manager provides widgets
  std::vector<vtkMRMLNode*> nodes;
  this->GetMRMLScene()->GetNodesByClass(this->m_Focus, nodes);
  for (std::vector< vtkMRMLNode* >::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    {
    vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(*nodeIt);
    if (annotationNode)
      {
      // do we  have a widget for it?
      if (this->GetWidget(annotationNode) == nullptr)
        {
        vtkDebugMacro("UpdateFromMRML: adding node " << annotationNode->GetID());
        if (this->AddAnnotation(annotationNode))
          {
          // update the new widget from the node
          this->PropagateMRMLToWidget(annotationNode, this->GetWidget(annotationNode));
          }
        }
      }
    }
  // set up observers on all the nodes
//  this->SetAndObserveNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnMRMLSceneEndClose();

  if (newScene)
    {
    this->RemoveObserversFromInteractionNode();
    this->RemoveObserversFromSelectionNode();
    this->AddObserversToInteractionNode();
    this->AddObserversToSelectionNode();
    }
  else
    {
    // there's no scene to get the interaction node from, so this won't do anything
    this->RemoveObserversFromInteractionNode();
    this->RemoveObserversFromSelectionNode();
    }
  vtkDebugMacro("SetMRMLSceneInternal: add observer on interaction node now?");

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager
::ProcessMRMLNodesEvents(vtkObject *caller,unsigned long event,void *callData)
{

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(caller);
  vtkMRMLInteractionNode * interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
  vtkMRMLSelectionNode * selectionNode =
    vtkMRMLSelectionNode::SafeDownCast(caller);
  if (annotationNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      case vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent:
        this->OnMRMLAnnotationControlPointModifiedEvent(annotationNode);
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLAnnotationNodeTransformModifiedEvent(annotationNode);
        break;
      case vtkMRMLAnnotationNode::LockModifiedEvent:
        this->OnMRMLAnnotationNodeLockModifiedEvent(annotationNode);
        break;
      case vtkMRMLDisplayableNode::DisplayModifiedEvent:
        this->OnMRMLAnnotationDisplayNodeModifiedEvent(annotationNode->GetDisplayNode());
        break;
      }
    }
  else if (interactionNode)
    {
    if (event == vtkMRMLInteractionNode::EndPlacementEvent)
      {
      // remove all seeds and reset the clickcounter
      this->m_ClickCounter->Reset();
      this->Helper->RemoveSeeds();
      return;
      }
    else if (event == vtkMRMLInteractionNode::InteractionModeChangedEvent)
      {
      // always update lock if the mode changed, even if this isn't the displayable manager
      // for the annotation that is getting placed, but don't update locking on persistence changed event
      this->Helper->UpdateLockedAllWidgetsFromInteractionNode(interactionNode);
      }
    }
  else if (selectionNode)
    {
    if (event == vtkMRMLSelectionNode::UnitModifiedEvent)
      {
      this->OnMRMLSelectionNodeUnitModifiedEvent(selectionNode);
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneEndClose()
{
  vtkDebugMacro("OnMRMLSceneEndClose: remove observers?");
  // run through all nodes and remove node and widget
  this->Helper->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromMRMLRequested(true);
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::UpdateFromMRMLScene()
{
  this->UpdateFromMRML();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
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
    this->RemoveObserversFromInteractionNode();
    this->AddObserversToInteractionNode();
    return;
    }

  if (!this->IsManageable(node))
    {
    // jump out
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Not the correct displayableManager for node " << node->GetID() << ", jumping out!");
    // also delete potential seeds
    this->m_ClickCounter->Reset();
    this->Helper->RemoveSeeds();
    return;
    }

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent:  node " << node->GetID());

  if (!this->AddAnnotation(annotationNode))
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: failed to add annotation node " << node->GetName());
    return;
    }

  // Remove all placed seeds
  this->Helper->RemoveSeeds();

  this->Helper->UpdateLockedAllWidgetsFromInteractionNode(this->GetInteractionNode());

  // and render again after seeds were removed
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeRemovedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

  // Remove the widget and the MRMLnode from the internal lists.
  this->Helper->RemoveWidgetAndNode(annotationNode);

  // Refresh observers
  vtkUnObserveMRMLNodeMacro(annotationNode);

  // and render again after seeds were removed
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node)
{
  //this->DebugOn();

  vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent");

  if (this->m_Updating)
    {
    vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent: Updating in progress.. Exit now.");
    return;
    }

  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeModifiedEvent: Can not access node.");
    return;
    }

  //std::cout << "OnMRMLAnnotationNodeModifiedEvent ThreeD->PropagateMRMLToWidget" << std::endl;

  vtkAbstractWidget * widget = this->Helper->GetWidget(annotationNode);

  if(this->Is2DDisplayableManager())
    {
    // force a OnMRMLSliceNodeModified() call to hide/show widgets according to the selected slice
    this->OnMRMLSliceNodeModifiedEvent(this->GetSliceNode());
    }
  else
    {
    // in 3D, always update the widget according to the mrml settings of lock and visibility status
    this->Helper->UpdateVisible(annotationNode);
    }

  if (widget)
    {
    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(annotationNode, widget);
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationDisplayNodeModifiedEvent(vtkMRMLNode* node)
{
  //this->DebugOn();

  if (this->m_Updating)
    {
    vtkDebugMacro("OnMRMLAnnotationDisplayNodeModifiedEvent: Updating in progress, returning.");
    return;
    }

  if (!node)
    {
    vtkDebugMacro("OnMRMLAnnotationDisplayNodeModifiedEvent: null node");
    return;
    }
  vtkMRMLAnnotationDisplayNode *annotationDisplayNode = vtkMRMLAnnotationDisplayNode::SafeDownCast(node);
  if (!annotationDisplayNode)
    {
    vtkErrorMacro("OnMRMLAnnotationDisplayNodeModifiedEvent: Cannot convert node to annotation display node: " << (node->GetID() ? node->GetID() : "null id"));
    return;
    }

  // find the annotation node that has this display node
  vtkMRMLAnnotationNode *annotationNode = this->Helper->GetAnnotationNodeFromDisplayNode(annotationDisplayNode);

  if (!annotationNode)
    {
    return;
    }

  vtkDebugMacro("OnMRMLAnnotationDisplayNodeModifiedEvent: found the annotation node "
                << annotationNode->GetID() << " associated with the modified display node "
                << annotationDisplayNode->GetID());
  vtkAbstractWidget * widget = this->Helper->GetWidget(annotationNode);

  if (widget)
    {

    if(this->Is2DDisplayableManager())
      {
      // force a OnMRMLSliceNodeModified() call to hide/show widgets according to the selected slice
      this->OnMRMLSliceNodeModifiedEvent(this->GetSliceNode());
      }
    else
      {
      // in 3D, always update the widget according to the mrml settings of lock and visibility status
      this->Helper->UpdateWidget(annotationNode);
      }

    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(annotationNode, widget);

    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationControlPointModifiedEvent(vtkMRMLNode *node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeControlPointModifiedEvent");
  if (!node)
    {
    return;
    }
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }
  vtkAbstractWidget *widget = this->Helper->GetWidget(annotationNode);
  if (widget)
    {
    // Update the standard settings of all widgets.
    this->UpdatePosition(widget, node);

    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(annotationNode, widget);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeTransformModifiedEvent");
  if (!node)
    {
    return;
    }
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeTransformModifiedEvent - Can not access node.");
    return;
    }


  if(this->Is2DDisplayableManager())
    {
    // force a OnMRMLSliceNodeModified() call to hide/show widgets according to the selected slice
    this->OnMRMLSliceNodeModifiedEvent(this->GetSliceNode());
    }
  else
    {
    // in 3D, always update the widget according to the mrml settings of lock and visibility status
    this->Helper->UpdateWidget(annotationNode);
    }

  vtkAbstractWidget *widget = this->Helper->GetWidget(annotationNode);
  if (widget)
    {
    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(annotationNode, widget);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeLockModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeLockModifiedEvent - Can not access node.");
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateLocked(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{

  vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent");

  if (!caller)
    {
    vtkErrorMacro("OnMRMLDisplayableNodeModifiedEvent: Could not get caller.");
    return;
    }

  if (this->GetSliceNode())
    {
    // the associated renderWindow is a 2D SliceView
    // this is the entry point for all events fired by one of the three sliceviews
    // (f.e. change slice number, zoom etc.)
    // now we call the handle for specific sliceNode actions
    this->OnMRMLSliceNodeModifiedEvent(this->GetSliceNode());

    // and exit
    return;
    }

  vtkMRMLViewNode * viewNode = vtkMRMLViewNode::SafeDownCast(caller);
  if (viewNode)
    {
    // the associated renderWindow is a 3D View
    vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent: This displayableManager handles a ThreeD view.");
    return;
    }

}

//---------------------------------------------------------------------------
vtkMRMLSliceNode * vtkMRMLAnnotationDisplayableManager::GetSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->GetMRMLDisplayableNode());
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::Is2DDisplayableManager()
{
  return GetSliceNode() != nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode* sliceNode)
{

  if (!sliceNode)
    {
    vtkErrorMacro("OnMRMLSliceNodeModifiedEvent: Could not get the sliceNode.");
    return;
    }

  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {

    // we loop through all nodes
    vtkMRMLAnnotationNode * annotationNode = *it;

    // check if the annotation is displayable according to the current selected Slice
    bool visibleOnSlice = this->IsWidgetDisplayable(sliceNode, annotationNode);

    this->Helper->UpdateVisible(annotationNode, visibleOnSlice);

    if (visibleOnSlice)
      {
      // it's visible, but if just update the position, don't get updates
      //necessary when switch into and out of lightbox
      vtkDebugMacro("OnMRMLSliceNodeModifiedEvent: visible, propagate mrml to widget");

      // If visible, turn off projection
      // TODO: Find a way to generalize it (turn projectionOff ?). Difficult to know which one were turned on before
      // to turn them back on when out of slice plane

      vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode);
      vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNode);
      if (rulerNode)
        {
        vtkLineWidget2* overLine = vtkLineWidget2::SafeDownCast(this->Helper->GetOverLineProjectionWidget(rulerNode));
        vtkLineWidget2* underLine = vtkLineWidget2::SafeDownCast(this->Helper->GetUnderLineProjectionWidget(rulerNode));
        if (overLine)
          {
          overLine->Off();
          }
        if (underLine)
          {
          underLine->Off();
          }
        }

      if (fiducialNode)
        {
        vtkSeedWidget* fiducialSeed = vtkSeedWidget::SafeDownCast(this->Helper->GetPointProjectionWidget(fiducialNode));
        if (fiducialSeed)
          {
          fiducialSeed->Off();
          }
        }

      this->PropagateMRMLToWidget(annotationNode, this->Helper->GetWidget(annotationNode));
      }

    else
      {
      // if the widget is not shown on the slice, show at least the intersection

      // only implemented for ruler yet

      vtkMRMLAnnotationRulerNode* rulerNode =
        vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode);
      vtkMRMLAnnotationFiducialNode* fiducialNode =
        vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNode);

      if (rulerNode &&
          (rulerNode->GetAnnotationLineDisplayNode()))
        {

        double transformedP1[4];
        rulerNode->GetControlPointWorldCoordinates(0, transformedP1);

        double transformedP2[4];
        rulerNode->GetControlPointWorldCoordinates(1, transformedP2);

        // now get the displayCoordinates for the transformed worldCoordinates
        double displayP1[4];
        double displayP2[4];
        this->GetWorldToDisplayCoordinates(transformedP1,displayP1);
        this->GetWorldToDisplayCoordinates(transformedP2,displayP2);

        //std::cout << this->GetSliceNode()->GetName() << " ras1: " << p1[0] << "," << p1[1] << "," << p1[2] << std::endl;
        //std::cout << this->GetSliceNode()->GetName() << " ras2: " << p2[0] << "," << p2[1] << "," << p2[2] << std::endl;

        //std::cout << this->GetSliceNode()->GetName() << " display1: " << displayP1[0] << "," << displayP1[1] << "," << displayP1[2] << std::endl;
        //std::cout << this->GetSliceNode()->GetName() << " display2: " << displayP2[0] << "," << displayP2[1] << "," << displayP2[2] << std::endl;

        // get line between p1 and p2
        // g(x) = p1 + r*(p2-p1)
        //
        // compute intersection with slice plane
        // if !=0: mark the intersection

        if(rulerNode->GetAnnotationLineDisplayNode()->GetVisibility2D())
          {
          //double this->GetSliceNode()->GetSliceOffset() = p1[2] + (p2[2]-p1[2])*t;
          // t = (this->GetSliceNode()->GetSliceOffset() - p1[2]) / (p2[2]-p1[2])
          //double t = (this->GetSliceNode()->GetSliceOffset()-displayP1[2]) / (displayP2[2]-displayP1[2]);
          double t = (-displayP1[2]) / (displayP2[2]-displayP1[2]);

          // p2-p1
          double P2minusP1[3];
          vtkMath::Subtract(displayP2,displayP1,P2minusP1);

          // (p2-p1)*t
          vtkMath::MultiplyScalar(P2minusP1,t);

          // p1 + ((p2-p1)*t)
          double P1plusP2minusP1[3];
          vtkMath::Add(displayP1,P2minusP1,P1plusP2minusP1);

          // Since we have the position of the intersection now,
          // we want to show it using a marker inside the sliceViews.
          //
          // We query the list if we already have a marker for this special widget.
          // If not, we create a new marker.
          // In any case, we will move the marker (either the newly created or the old).

          vtkSeedWidget* marker = vtkSeedWidget::SafeDownCast(this->Helper->GetIntersectionWidget(rulerNode));

          if (!marker)
            {
            // we create a new marker.

            vtkNew<vtkPointHandleRepresentation2D> handle;
            handle->GetProperty()->SetColor(0,1,0);
            handle->SetHandleSize(3);

            vtkNew<vtkSeedRepresentation> rep;
            rep->SetHandleRepresentation(handle.GetPointer());

            marker = vtkSeedWidget::New();

            marker->CreateDefaultRepresentation();

            marker->SetRepresentation(rep.GetPointer());

            marker->SetInteractor(this->GetInteractor());
            marker->SetCurrentRenderer(this->GetRenderer());

            marker->ProcessEventsOff();

            marker->On();
            marker->CompleteInteraction();

            // we save the marker in our WidgetIntersection list associated to this node
            this->Helper->WidgetIntersections[rulerNode] = marker;
            }

          // remove all old markers associated with this node
          marker->DeleteSeed(0);

          // the third component of the displayCoordinates is the distance to the slice
          // now make sure that they have different signs
          if ((displayP1[2] > 0 && displayP2[2] > 0) ||
              (displayP1[2] < 0 && displayP2[2] < 0))
            {
            // jump out because they do not have different signs
            ++it;
            continue;
            }

          // .. and create a new one at the intersection location
          vtkSmartPointer<vtkHandleWidget> newhandle = marker->CreateNewHandle();
          vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(P1plusP2minusP1);
          marker->On();
          marker->CompleteInteraction();
          }

        // Display projection on 2D viewers
        vtkLineWidget2* overLine = vtkLineWidget2::SafeDownCast(this->Helper->GetOverLineProjectionWidget(rulerNode));
        vtkLineWidget2* underLine = vtkLineWidget2::SafeDownCast(this->Helper->GetUnderLineProjectionWidget(rulerNode));
        vtkMRMLAnnotationLineDisplayNode* lineDisplayNode = rulerNode->GetAnnotationLineDisplayNode();

        if (lineDisplayNode)
          {
          if ((lineDisplayNode->GetSliceProjection() & lineDisplayNode->ProjectionOn) &&
              lineDisplayNode->GetVisibility() && lineDisplayNode->GetVisibility2D())
            {
            double overLineWidth = lineDisplayNode->GetOverLineThickness();
            double underLineWidth = lineDisplayNode->GetUnderLineThickness();
            double intersectionPoint[3] = {displayP2[0], displayP2[1], displayP2[2] };
            bool lineIntersectPlane = false;

            if (!overLine)
              {
              vtkNew<vtkPointHandleRepresentation3D> handle;
              handle->GetProperty()->SetOpacity(0.0);
              handle->GetSelectedProperty()->SetOpacity(0.0);
              handle->SetHandleSize(0);

              vtkNew<vtkLineRepresentation> rep;
              rep->SetHandleRepresentation(handle.GetPointer());
              rep->GetPoint1Representation()->GetProperty()->SetOpacity(0.0);
              rep->GetPoint1Representation()->GetSelectedProperty()->SetOpacity(0.0);
              rep->GetPoint2Representation()->GetProperty()->SetOpacity(0.0);
              rep->GetPoint2Representation()->GetSelectedProperty()->SetOpacity(0.0);
              rep->GetLineHandleRepresentation()->GetProperty()->SetOpacity(0.0);
              rep->GetLineHandleRepresentation()->GetSelectedProperty()->SetOpacity(0.0);
              rep->GetLineProperty()->SetLineWidth(overLineWidth);
              rep->GetLineHandleRepresentation()->DragableOff();
              rep->GetLineHandleRepresentation()->PickableOff();

              overLine = vtkLineWidget2::New();
              overLine->CreateDefaultRepresentation();
              overLine->SetRepresentation(rep.GetPointer());
              overLine->SetInteractor(this->GetInteractor());
              overLine->SetCurrentRenderer(this->GetRenderer());
              overLine->ProcessEventsOff();
              overLine->ManagesCursorOff();
              this->Helper->WidgetOverLineProjections[rulerNode] = overLine;
              }

            if (lineDisplayNode->GetSliceProjection() & lineDisplayNode->ProjectionThickerOnTop)
              {
              if (!underLine)
                {
                vtkNew<vtkPointHandleRepresentation3D> handle;
                handle->GetProperty()->SetOpacity(0.0);
                handle->GetSelectedProperty()->SetOpacity(0.0);
                handle->SetHandleSize(0);

                vtkNew<vtkLineRepresentation> rep;
                rep->SetHandleRepresentation(handle.GetPointer());
                rep->GetPoint1Representation()->GetProperty()->SetOpacity(0.0);
                rep->GetPoint1Representation()->GetSelectedProperty()->SetOpacity(0.0);
                rep->GetPoint2Representation()->GetProperty()->SetOpacity(0.0);
                rep->GetPoint2Representation()->GetSelectedProperty()->SetOpacity(0.0);
                rep->GetLineHandleRepresentation()->GetProperty()->SetOpacity(0.0);
                rep->GetLineHandleRepresentation()->GetSelectedProperty()->SetOpacity(0.0);
                rep->GetLineProperty()->SetLineWidth(underLineWidth);
                rep->GetLineHandleRepresentation()->DragableOff();
                rep->GetLineHandleRepresentation()->PickableOff();

                underLine = vtkLineWidget2::New();
                underLine->CreateDefaultRepresentation();
                underLine->SetRepresentation(rep.GetPointer());
                underLine->SetInteractor(this->GetInteractor());
                underLine->SetCurrentRenderer(this->GetRenderer());
                underLine->ProcessEventsOff();
                underLine->ManagesCursorOff();
                this->Helper->WidgetUnderLineProjections[rulerNode] = underLine;
                }

              if ((displayP1[2] * displayP2[2]) < 0)
                {
                // Point 1 and Point 2 are in different side of the plane
                // Calculate plane intersection and set it as second point
                // of top line, set it as first point of under line
                lineIntersectPlane = true;

                double t = (-displayP1[2]) / (displayP2[2]-displayP1[2]);
                double P2minusP1[3];
                vtkMath::Subtract(displayP2,displayP1,P2minusP1);
                vtkMath::MultiplyScalar(P2minusP1,t);
                vtkMath::Add(displayP1,P2minusP1,intersectionPoint);
                }
              else
                {
                underLine->Off();
                }
              }
            else
              {
              if (underLine)
                {
                underLine->Off();
                }
              }

            vtkLineRepresentation* overLineRep = vtkLineRepresentation::SafeDownCast(overLine->GetRepresentation());

            if (overLineRep)
              {
              overLine->Off();
              if (underLine)
                {
                underLine->Off();
                }

              double lineOpacity = lineDisplayNode->GetProjectedOpacity();
              double lineColor[3];

              if (lineDisplayNode->GetSliceProjection() & lineDisplayNode->ProjectionUseRulerColor)
                {
                lineDisplayNode->GetColor(lineColor);
                }
              else
                {
                lineDisplayNode->GetProjectedColor(lineColor);
                }

              short linePattern = 0xFFFF;

              if (lineDisplayNode->GetSliceProjection() & lineDisplayNode->ProjectionDashed)
                {
                linePattern = 0xFF00;
                }

              // TODO: Modify pattern to have spaced dot line when further from plane, smaller space when closer
              // 0x0001 + (0x0001 + 1) = 0x0003 (0000 0000 0000 0011)
              // 0x0003 + (0x0003 + 1) = 0x0007 (0000 0000 0000 0111)
              // 0x0007 + (0x0007 + 1) = 0x000F (0000 0000 0000 1111)
              // etc...

              if (lineDisplayNode->GetSliceProjection() & lineDisplayNode->ProjectionColoredWhenParallel)
                {
                vtkMatrix4x4 *sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
                double slicePlaneNormal[3], rulerVector[3];
                slicePlaneNormal[0] = sliceToRAS->GetElement(0,2);
                slicePlaneNormal[1] = sliceToRAS->GetElement(1,2);
                slicePlaneNormal[2] = sliceToRAS->GetElement(2,2);
                rulerVector[0] = transformedP2[0] - transformedP1[0];
                rulerVector[1] = transformedP2[1] - transformedP1[1];
                rulerVector[2] = transformedP2[2] - transformedP1[2];

                vtkMath::Normalize(slicePlaneNormal);
                vtkMath::Normalize(rulerVector);

                static const double LineInPlaneError = 0.005;
                if (fabs(vtkMath::Dot(slicePlaneNormal, rulerVector)) < LineInPlaneError)
                  {
                  sliceNode->GetLayoutColor(lineColor);
                  linePattern = 0xFFFF;
                  }
                }

              // Should be reset when widget is turned off, otherwise handle is rendered
              overLineRep->GetPoint1Representation()->GetSelectedProperty()->SetOpacity(0.0);
              overLineRep->GetPoint2Representation()->GetSelectedProperty()->SetOpacity(0.0);
              overLineRep->GetLineProperty()->SetColor(lineColor);
              overLineRep->GetLineProperty()->SetOpacity(lineOpacity);
              overLineRep->GetLineProperty()->SetLineStipplePattern(linePattern);
              overLineRep->GetLineProperty()->SetLineWidth(displayP1[2] > 0 ? underLineWidth : overLineWidth);
              overLine->On();
              overLineRep->SetPoint1DisplayPosition(displayP1);
              overLineRep->SetPoint2DisplayPosition(intersectionPoint);

              if (lineIntersectPlane)
                {
                vtkLineRepresentation* underLineRep = vtkLineRepresentation::SafeDownCast(underLine->GetRepresentation());
                if (underLineRep)
                  {
                  underLineRep->GetPoint1Representation()->GetSelectedProperty()->SetOpacity(0.0);
                  underLineRep->GetPoint2Representation()->GetSelectedProperty()->SetOpacity(0.0);
                  underLineRep->GetLineProperty()->SetColor(lineColor);
                  underLineRep->GetLineProperty()->SetOpacity(lineOpacity);
                  underLineRep->GetLineProperty()->SetLineStipplePattern(linePattern);
                  underLineRep->GetLineProperty()->SetLineWidth(displayP2[2] > 0 ? underLineWidth : overLineWidth);
                  underLine->On();
                  underLineRep->SetPoint1DisplayPosition(intersectionPoint);
                  underLineRep->SetPoint2DisplayPosition(displayP2);
                  }
                }
              }
            }
          else
            {
            if (overLine)
              {
              overLine->Off();
              }
            if (underLine)
              {
              underLine->Off();
              }
            }
          }
        }
      else if (fiducialNode &&
               fiducialNode->GetAnnotationPointDisplayNode())
        {
        double transformedP1[4];
        fiducialNode->GetFiducialWorldCoordinates(transformedP1);

        double displayP1[4];
        this->GetWorldToDisplayCoordinates(transformedP1, displayP1);

        vtkSeedWidget* projectionSeed =
          vtkSeedWidget::SafeDownCast(this->Helper->GetPointProjectionWidget(fiducialNode));

        vtkMRMLAnnotationPointDisplayNode* pointDisplayNode =
          vtkMRMLAnnotationPointDisplayNode::SafeDownCast(fiducialNode->GetAnnotationPointDisplayNode());

        if ((pointDisplayNode->GetSliceProjection() & pointDisplayNode->ProjectionOn) &&
            pointDisplayNode->GetVisibility() && pointDisplayNode->GetVisibility2D())
          {
          double glyphScale = fiducialNode->GetAnnotationPointDisplayNode()->GetGlyphScale()*2;
          int glyphType = fiducialNode->GetAnnotationPointDisplayNode()->GetGlyphType();
          if (glyphType == vtkMRMLAnnotationPointDisplayNode::Sphere3D)
            {
            // 3D Sphere glyph is represented in 2D by a Circle2D glyph
            glyphType = vtkMRMLAnnotationPointDisplayNode::Circle2D;
            }

          double pointOpacity = pointDisplayNode->GetProjectedOpacity();
          double pointColor[3];
          pointDisplayNode->GetProjectedColor(pointColor);

          if (pointDisplayNode->GetSliceProjection() & pointDisplayNode->ProjectionUseFiducialColor)
            {
            pointDisplayNode->GetColor(pointColor);
            }

          if (!projectionSeed)
            {
            vtkNew<vtkAnnotationGlyphSource2D> glyph;
            glyph->SetGlyphType(glyphType);
            glyph->SetScale(glyphScale);
            glyph->SetScale2(glyphScale);
            glyph->SetColor(pointColor);

            vtkNew<vtkPointHandleRepresentation2D> handle;
            handle->SetCursorShape(glyph->GetOutput());

            vtkNew<vtkSeedRepresentation> rep;
            rep->SetHandleRepresentation(handle.GetPointer());

            projectionSeed = vtkSeedWidget::New();
            projectionSeed->CreateDefaultRepresentation();
            projectionSeed->SetRepresentation(rep.GetPointer());
            projectionSeed->SetInteractor(this->GetInteractor());
            projectionSeed->SetCurrentRenderer(this->GetRenderer());
            projectionSeed->CreateNewHandle();
            projectionSeed->ProcessEventsOff();
            projectionSeed->ManagesCursorOff();
            projectionSeed->On();
            projectionSeed->CompleteInteraction();
            this->Helper->WidgetPointProjections[fiducialNode] = projectionSeed;
            }

          vtkSeedRepresentation* projectionSeedRep =
            vtkSeedRepresentation::SafeDownCast(projectionSeed->GetRepresentation());

          if (projectionSeedRep)
            {
            projectionSeed->Off();

            if (projectionSeed->GetSeed(0))
              {
              vtkPointHandleRepresentation2D* handleRep =
                vtkPointHandleRepresentation2D::SafeDownCast(projectionSeed->GetSeed(0)->GetRepresentation());

              if (handleRep)
                {
                vtkNew<vtkAnnotationGlyphSource2D> glyphSource;
                glyphSource->SetGlyphType(glyphType);
                glyphSource->SetScale(glyphScale);
                glyphSource->SetScale2(glyphScale);

                if (pointDisplayNode->GetSliceProjection() & pointDisplayNode->ProjectionOutlinedBehindSlicePlane)
                  {
                  static const double threshold = 0.5;
                  static const double notInPlaneOpacity = 0.6;
                  static const double inPlaneOpacity = 1.0;
                  if (displayP1[2] < 0)
                    {
                    glyphSource->FilledOff();
                    pointOpacity = notInPlaneOpacity;

                    if (displayP1[2] > -threshold)
                      {
                      pointOpacity = inPlaneOpacity;
                      }
                    }
                  else if (displayP1[2] > 0)
                    {
                    glyphSource->FilledOn();
                    pointOpacity = notInPlaneOpacity;

                    if (displayP1[2] < threshold)
                      {
                      pointOpacity = inPlaneOpacity;
                      }
                    }
                  }
                else
                  {
                  glyphSource->FilledOn();
                  }
                glyphSource->SetColor(pointColor);
                handleRep->GetProperty()->SetColor(pointColor);
                handleRep->GetProperty()->SetOpacity(pointOpacity);
                handleRep->SetCursorShape(glyphSource->GetOutput());
                handleRep->SetDisplayPosition(displayP1);
                projectionSeed->On();
                projectionSeed->CompleteInteraction();
                }
              }
            }
          }
        else
          {
          if (projectionSeed)
            {
            projectionSeed->Off();
            }
          }
        }
      }

    ++it;
    }

}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::IsWidgetDisplayable(vtkMRMLSliceNode* sliceNode, vtkMRMLAnnotationNode* node)
{

  if (!sliceNode)
    {
    vtkErrorMacro("IsWidgetDisplayable: Could not get the sliceNode.");
    return false;
    }

  if (!node)
    {
    vtkErrorMacro("IsWidgetDisplayable: Could not get the annotation node.");
    return false;
    }

  bool showWidget = true;
  bool inViewport = false;

  // allow annotations to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = node->GetDisplayNode();
  if (displayNode && !displayNode->IsDisplayableInView(sliceNode->GetID()))
    {
    return false;
    }

  // down cast the node as a control points node to get the coordinates
  vtkMRMLAnnotationControlPointsNode * controlPointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);

  if (!controlPointsNode)
    {
    vtkErrorMacro("IsWidgetDisplayable: Could not get the controlpoints node.");
    return false;
    }

  if (this->IsInLightboxMode())
    {
    /// BUG mantis issue 1690: if in lightbox mode, don't show rulers
    if (!strcmp(this->m_Focus, "vtkMRMLAnnotationRulerNode"))
      {
      return false;
      }
    }

  int numberOfControlPoints =  controlPointsNode->GetNumberOfControlPoints();
  // the text node saves it's second control point in viewport coordinates, so
  // don't check it
  if (node->IsA("vtkMRMLAnnotationTextNode"))
    {
    numberOfControlPoints = 1;
    }
  for (int i=0; i < numberOfControlPoints; i++)
    {
    // we loop through all controlpoints of each node
    double transformedWorldCoordinates[4];
    controlPointsNode->GetControlPointWorldCoordinates(i, transformedWorldCoordinates);

    // now get the displayCoordinates for the transformed worldCoordinates
    double displayCoordinates[4];
    this->GetWorldToDisplayCoordinates(transformedWorldCoordinates,displayCoordinates);

    if (this->IsInLightboxMode())
      {
      //
      // Lightbox specific code
      //

      // get the corresponding lightbox index for this display coordinate and
      // check if it's in the range of the current number of light boxes being
      // displayed in the grid rows/columns.
      int lightboxIndex = this->GetLightboxIndex(controlPointsNode);
      int numberOfLightboxes = sliceNode->GetLayoutGridColumns() * sliceNode->GetLayoutGridRows();
      //std::cout << "IsWidgetDisplayable: " << sliceNode->GetName()
      //          << ": lightbox mode, index = " << lightboxIndex
      //          << ", rows = " << sliceNode->GetLayoutGridRows()
      //          << ", cols = " << sliceNode->GetLayoutGridColumns()
      //          << ", number of light boxes = " << numberOfLightboxes << std::endl;
      if (lightboxIndex < 0 ||
          lightboxIndex >= numberOfLightboxes)
        {
        showWidget = false;
        }
      else
        {
        // get the right renderer index by checking the z coordinate
        vtkRenderer* currentRenderer = this->GetRenderer(lightboxIndex);

        // now we get the widget..
        vtkAbstractWidget* widget = this->GetWidget(node);

        // TODO this code blocks the movement of the widget in lightbox mode
        if (widget &&
            (widget->GetCurrentRenderer() != currentRenderer ||
             widget->GetRepresentation()->GetRenderer() != currentRenderer))
          {
          vtkDebugMacro("IsWidgetDisplayable: updating renderer on widget and representation");
          // if the widget is on, need to turn it off to set the renderer
          bool toggleOffOn = false;
          if (widget->GetEnabled())
            {
            // turn it off..
            widget->Off();
            toggleOffOn = true;
            }
          // ..place it and its representation to the right renderer..

          widget->SetCurrentRenderer(currentRenderer);
          widget->GetRepresentation()->SetRenderer(currentRenderer);

          if (toggleOffOn)
            {
            // ..and turn it on again!
            widget->On();
            }
          // if it's a seed widget, go to complete interaction state
          vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
          if (seedWidget)
            {
            vtkDebugMacro("SeedWidget: Complete interaction");
            seedWidget->CompleteInteraction();
            }

          // we need to render again
          this->RequestRender();
          }

        }
      //
      // End of Lightbox specific code
      //
      }

    // check if the annotation is close enough to the slice to be shown
    if (showWidget)
      {
      if (this->IsInLightboxMode())
        {
        // get the volume's spacing to determine the distance between the slice
        // location and the annotation
        // default to spacing 1.0 in case can't get volume slice spacing from
        // the logic as that will be a multiplicative no-op
        double spacing = 1.0;
        vtkMRMLSliceLogic *sliceLogic = nullptr;
        vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
        if (mrmlAppLogic)
          {
          sliceLogic = mrmlAppLogic->GetSliceLogic(this->GetSliceNode());
          }
        if (sliceLogic)
          {
          double *volumeSliceSpacing = sliceLogic->GetLowestVolumeSliceSpacing();
          if (volumeSliceSpacing != nullptr)
            {
            vtkDebugMacro("Slice node " << this->GetSliceNode()->GetName()
                          << ": volumeSliceSpacing = "
                          << volumeSliceSpacing[0] << ", "
                          << volumeSliceSpacing[1] << ", "
                          << volumeSliceSpacing[2]);
            spacing = volumeSliceSpacing[2];
            }
          }
        vtkDebugMacro("displayCoordinates: "
                      << displayCoordinates[0] << ","
                      << displayCoordinates[1] << ","
                      << displayCoordinates[2]
                      << "\n\tworld coords: "
                      << transformedWorldCoordinates[0] << ","
                      << transformedWorldCoordinates[1] << ","
                      << transformedWorldCoordinates[2]);
        // calculate the distance from the annotation in world space to the
        // plane defined by the slice node normal and origin (using same
        // convention as the vtkMRMLThreeDReformatDisplayableManager)
        vtkMatrix4x4 *sliceToRAS = this->GetSliceNode()->GetSliceToRAS();
        double slicePlaneNormal[3], slicePlaneOrigin[3];
        slicePlaneNormal[0] = sliceToRAS->GetElement(0,2);
        slicePlaneNormal[1] = sliceToRAS->GetElement(1,2);
        slicePlaneNormal[2] = sliceToRAS->GetElement(2,2);
        slicePlaneOrigin[0] = sliceToRAS->GetElement(0,3);
        slicePlaneOrigin[1] = sliceToRAS->GetElement(1,3);
        slicePlaneOrigin[2] = sliceToRAS->GetElement(2,3);
        double distanceToPlane = slicePlaneNormal[0]*(transformedWorldCoordinates[0]-slicePlaneOrigin[0]) +
          slicePlaneNormal[1]*(transformedWorldCoordinates[1]-slicePlaneOrigin[1]) +
          slicePlaneNormal[2]*(transformedWorldCoordinates[2]-slicePlaneOrigin[2]);
        // this gives the distance to light box plane 0, but have to offset by
        // number of light box planes (as determined by the light box index) times the volume
        // slice spacing
        int lightboxIndex = this->GetLightboxIndex(controlPointsNode);
        double lightboxOffset = lightboxIndex * spacing;
        double distanceToSlice = distanceToPlane - lightboxOffset;
        double maxDistance = 0.5;
        vtkDebugMacro("\n\tdistance to plane = " << distanceToPlane
                      << "\n\tlightboxIndex = " << lightboxIndex
                      << "\n\tlightboxOffset = " << lightboxOffset
                      << "\n\tdistance to slice = " << distanceToSlice);
        // check that it's within 0.5mm
        if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
          {
          vtkDebugMacro("Distance to slice is greater than max distance, not showing the widget");
          showWidget = false;
          break;
          }
        }
      else
        {
        // the third coordinate of the displayCoordinates is the distance to the slice
        float distanceToSlice = displayCoordinates[2];
        float maxDistance = 0.5 + (sliceNode->GetDimensions()[2] - 1);
        vtkDebugMacro("Slice node " << this->GetSliceNode()->GetName()
                      << ": distance to slice = " << distanceToSlice
                      << ", maxDistance = " << maxDistance
                      << "\n\tslice node dimensions[2] = " << sliceNode->GetDimensions()[2]);
        if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
          {
          // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
          // hence, we do not want to show this widget
          showWidget = false;
          // we don't even need to continue parsing the controlpoints, because we know the widget will not be shown
          break;
          }
        }
      }
    // -----------------------------------------
    // special cases when the slices get panned:

    // if all of the controlpoints are outside the viewport coordinates, the widget should not be shown
    // if one controlpoint is inside the viewport coordinates, the widget should be shown

    // we need to check if we are inside the viewport
    double coords[2] = {displayCoordinates[0], displayCoordinates[1]};

    vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(coords[0],coords[1]);
    if (!pokedRenderer)
      {
      vtkErrorMacro("IsWidgetDisplayable: Could not find the poked renderer!");
      return false;
      }

    pokedRenderer->DisplayToNormalizedDisplay(coords[0],coords[1]);
    pokedRenderer->NormalizedDisplayToViewport(coords[0],coords[1]);
    pokedRenderer->ViewportToNormalizedViewport(coords[0],coords[1]);

    if ((coords[0]>0.0) && (coords[0]<1.0) && (coords[1]>0.0) && (coords[1]<1.0))
      {
      // current point is inside of view
      inViewport = true;
      }

    } // end of for loop through control points


  return showWidget && inViewport;

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.");
    return;
    }

  if (!this->IsCorrectDisplayableManager())
    {
    //std::cout << "Annotation DisplayableManger: OnInteractorStyleEvent : " << this->m_Focus << ", not correct displayable manager, returning" << std::endl;
    return;
    }
  vtkDebugMacro("OnInteractorStyleEvent " << this->m_Focus << " " << eventid);

  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      //std::cout << "OnInteractorStyleEvent got a left button release "
      // << eventid << ", and are in place mode, calling OnClickInRenderWindowGetCoordinates" << std::endl;
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
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
        this->GetInteractionNode()->GetPlaceModePersistence() == 1)
      {
      this->GetInteractionNode()->SwitchToViewTransformMode();
      }
    }
  else
    {
    //vtkWarningMacro("OnInteractorStyleEvent: unhandled event " << eventid);
    //std::cout << "Annotation DisplayableManager: OnInteractorStyleEvent: unhandled event " << eventid << std::endl;
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManager::GetWidget(vtkMRMLAnnotationNode * node)
{
  return this->Helper->GetWidget(node);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnClickInRenderWindowGetCoordinates()
{

  double x = this->GetInteractor()->GetEventPosition()[0];
  double y = this->GetInteractor()->GetEventPosition()[1];

  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (x < windowWidth && y < windowHeight)
    {
    const char *associatedNodeID = nullptr;
    // is this a 2d manager that has a volume in the background?
    if (this->Is2DDisplayableManager())
      {
      if (this->GetSliceNode())
        {
        // find the slice composite node in the scene with the matching layout
        // name
        vtkMRMLSliceLogic *sliceLogic = nullptr;
        vtkMRMLSliceCompositeNode* sliceCompositeNode = nullptr;
        vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
        if (mrmlAppLogic)
          {
          sliceLogic = mrmlAppLogic->GetSliceLogic(this->GetSliceNode());
          }
        if (sliceLogic)
          {
          sliceCompositeNode = sliceLogic->GetSliceCompositeNode(this->GetSliceNode());
          }
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
          }
        }
      }
    else
      {
      // it's a 3D displayable manager and the click could have been on a node
      vtkMRMLModelDisplayableManager * modelDisplayableManager =
        vtkMRMLModelDisplayableManager::SafeDownCast(
          this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
            "vtkMRMLModelDisplayableManager"));
      double yNew = windowHeight - y - 1;
      if (modelDisplayableManager &&
          modelDisplayableManager->Pick(x,yNew) &&
          strcmp(modelDisplayableManager->GetPickedNodeID(),"") != 0)
        {
        // find the node id, the picked node name is probably the display node
        const char *pickedNodeID = modelDisplayableManager->GetPickedNodeID();
        vtkDebugMacro("Click was on model " << pickedNodeID);
        vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(pickedNodeID);
        vtkMRMLDisplayNode *displayNode = nullptr;
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
                    vtkDebugMacro("Annotation was placed on a 3d slice model, found the volume id " << associatedNodeID);
                    }
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
void vtkMRMLAnnotationDisplayableManager::PlaceSeed(double x, double y)
{

  // place the seed
  this->Helper->PlaceSeed(x,y,this->GetInteractor(),this->GetRenderer());

  this->RequestRender();

}

//---------------------------------------------------------------------------
/// Get the handle of a placed seed
vtkHandleWidget * vtkMRMLAnnotationDisplayableManager::GetSeed(int index)
{
  return this->Helper->GetSeed(index);
}

//---------------------------------------------------------------------------
// Coordinate conversions
//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLAnnotationDisplayableManager::GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates)
{

  if (this->Is2DDisplayableManager())
    {
    // 2D case
    double xyz[3];
    vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(), this->GetSliceNode(), x, y, xyz);
    vtkMRMLAbstractSliceViewDisplayableManager::ConvertXYZToRAS(this->GetSliceNode(), xyz, worldCoordinates);
    }
  else
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
}

//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLAnnotationDisplayableManager::GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates)
{

  this->GetDisplayToWorldCoordinates(displayCoordinates[0], displayCoordinates[1], worldCoordinates);

}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLAnnotationDisplayableManager::GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates)
{

  if (this->Is2DDisplayableManager())
    {
    // 2D case
    double ras[3];
    ras[0] = r;
    ras[1] = a;
    ras[2] = s;
    vtkMRMLAbstractSliceViewDisplayableManager::ConvertRASToXYZ(this->GetSliceNode(), ras, displayCoordinates);
    }
  else
    {
    vtkInteractorObserver::ComputeWorldToDisplay(this->GetRenderer(),r,a,s,displayCoordinates);
    }


}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLAnnotationDisplayableManager::GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates)
{
  if (worldCoordinates == nullptr)
    {
    return;
    }
  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);

}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLAnnotationDisplayableManager::GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates)
{

  if (viewportCoordinates == nullptr)
    {
    return;
    }
  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (this->Is2DDisplayableManager())
    {
    // 2D case

    int numberOfColumns = this->GetSliceNode()->GetLayoutGridColumns();
    int numberOfRows = this->GetSliceNode()->GetLayoutGridRows();

    float tempX = x / windowWidth;
    float tempY = (windowHeight - y) / windowHeight;

    float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

    vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x,y);


    double displayCoordinates[4];
    displayCoordinates[0] = x - pokedRenderer->GetOrigin()[0];
    displayCoordinates[1] = y - pokedRenderer->GetOrigin()[1];
    displayCoordinates[2] = z;
    displayCoordinates[3] = 1;

    if (windowWidth != 0.0)
      {
      viewportCoordinates[0] = displayCoordinates[0]/windowWidth;
      }
    if (windowHeight != 0.0)
      {
      viewportCoordinates[1] = displayCoordinates[1]/windowHeight;
      }
    vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y
                  << ", display coords calc as "
                  << displayCoordinates[0] << ", "
                  << displayCoordinates[1]
                  << ", returning viewport = "
                  << viewportCoordinates[0] << ", "
                  << viewportCoordinates[1]);
    }
  else
    {
    if (windowWidth != 0.0)
      {
      viewportCoordinates[0] = x/windowWidth;
      }
    if (windowHeight != 0.0)
      {
      viewportCoordinates[1] = y/windowHeight;
      }
    vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y
                  << ", returning viewport = " << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
    }
}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLAnnotationDisplayableManager::GetDisplayToViewportCoordinates(double * displayCoordinates, double * viewportCoordinates)
{
  if (displayCoordinates && viewportCoordinates)
    {
    this->GetDisplayToViewportCoordinates(displayCoordinates[0], displayCoordinates[1], viewportCoordinates);
    }
}

//---------------------------------------------------------------------------
/// Check if the displayCoordinates are inside the viewport and if not, correct the displayCoordinates
void vtkMRMLAnnotationDisplayableManager::RestrictDisplayCoordinatesToViewport(double* displayCoordinates)
{
  double coords[2] = {displayCoordinates[0], displayCoordinates[1]};

  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(coords[0],coords[1]);
  if (!pokedRenderer)
    {
    vtkErrorMacro("RestrictDisplayCoordinatesToViewport: Could not find the poked renderer!");
    return;
    }

  pokedRenderer->DisplayToNormalizedDisplay(coords[0],coords[1]);
  pokedRenderer->NormalizedDisplayToViewport(coords[0],coords[1]);
  pokedRenderer->ViewportToNormalizedViewport(coords[0],coords[1]);

  if (coords[0]<0.001)
    {
    coords[0] = 0.001;
    }
  else if (coords[0]>0.999)
    {
    coords[0] = 0.999;
    }

  if (coords[1]<0.001)
    {
    coords[1] = 0.001;
    }
  else if (coords[1]>0.999)
    {
    coords[1] = 0.999;
    }

  pokedRenderer->NormalizedViewportToViewport(coords[0],coords[1]);
  pokedRenderer->ViewportToNormalizedDisplay(coords[0],coords[1]);
  pokedRenderer->NormalizedDisplayToDisplay(coords[0],coords[1]);

//  if (this->GetDisplayCoordinatesChanged(displayCoordinates,coords))
//    {
  displayCoordinates[0] = coords[0];
  displayCoordinates[1] = coords[1];
//    }
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLAnnotationDisplayableManager::GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2)
{
  bool changed = false;

  if (sqrt( ( displayCoordinates1[0] - displayCoordinates2[0] ) * ( displayCoordinates1[0] - displayCoordinates2[0] )
           + ( displayCoordinates1[1] - displayCoordinates2[1] ) * ( displayCoordinates1[1] - displayCoordinates2[1] ))>1.0)
    {
    changed = true;
    }
  else
    {
    // if in lightbox mode, the third element in the vector may have changed
    if (this->IsInLightboxMode())
      {
      // one of the arguments may be coming from a widget, the other should be
      // the index into the light box array
      double dist = sqrt( (displayCoordinates1[2] - displayCoordinates2[2]) * (displayCoordinates1[2] - displayCoordinates2[2]));
      if (dist > 1.0)
        {
        changed = true;
        }
      }
    }
  return changed;
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLAnnotationDisplayableManager::GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2)
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
bool vtkMRMLAnnotationDisplayableManager::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
  if ( selectionNode == nullptr )
    {
    vtkErrorMacro ( "IsCorrectDisplayableManager: No selection node in the scene." );
    return false;
    }
  if ( selectionNode->GetActivePlaceNodeClassName() == nullptr)
    {
    //vtkErrorMacro ( "IsCorrectDisplayableManager: no active annotation");
    return false;
    }
  // the purpose of the displayableManager is hardcoded
  return this->IsManageable(selectionNode->GetActivePlaceNodeClassName());

}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::IsManageable(vtkMRMLNode* node)
{
  return node->IsA(this->m_Focus);
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::IsManageable(const char* nodeClassName)
{
  return nodeClassName && !strcmp(nodeClassName, this->m_Focus);
}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnClickInRenderWindow(double vtkNotUsed(x), double vtkNotUsed(y), const char * vtkNotUsed(associatedNodeID))
{

  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkMRMLAnnotationDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* vtkNotUsed(node))
{

  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnWidgetCreated(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLAnnotationNode* vtkNotUsed(node))
{

  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* vtkNotUsed(node), vtkAbstractWidget* vtkNotUsed(widget))
{

  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateMRMLToWidget should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLAnnotationNode* vtkNotUsed(node))
{

  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}

//---------------------------------------------------------------------------
/// Convert world coordinates to local using mrml parent transform
void vtkMRMLAnnotationDisplayableManager::GetWorldToLocalCoordinates(vtkMRMLAnnotationNode *node,
                                                                     double *worldCoordinates,
                                                                     double *localCoordinates)
{
  if (node == nullptr)
    {
    vtkErrorMacro("GetWorldToLocalCoordinates: node is null");
    return;
    }

  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();
  if (tnode != nullptr && tnode->IsTransformToWorldLinear())
    {
    vtkNew<vtkMatrix4x4> transformToWorld;
    transformToWorld->Identity();
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
    transformToWorld->Invert();

    double p[4];
    p[3] = 1;
    int i;
    for (i=0; i<3; i++)
      {
      p[i] = worldCoordinates[i];
      }
    double *xyz = transformToWorld->MultiplyDoublePoint(p);
    for (i=0; i<3; i++)
      {
      localCoordinates[i] = xyz[i];
      }
    }
  else
    {
    for (int i=0; i<3; i++)
      {
      localCoordinates[i] = worldCoordinates[i];
      }
    }

}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::IsInLightboxMode()
{
  bool flag = false;
  if (this->Is2DDisplayableManager() &&
      this->GetSliceNode())
    {
    int numberOfColumns = this->GetSliceNode()->GetLayoutGridColumns();
    int numberOfRows = this->GetSliceNode()->GetLayoutGridRows();
    if (numberOfColumns > 1 ||
        numberOfRows > 1)
      {
      flag = true;
      }
    }
  return flag;
}



//---------------------------------------------------------------------------
int vtkMRMLAnnotationDisplayableManager::GetLightboxIndex(vtkMRMLAnnotationNode *node, int controlPointIndex)
{
  int index = -1;

  if (!node)
    {
    return index;
    }
  if (!this->IsInLightboxMode())
    {
    return index;
    }

  // down cast the node as a controlpoints node to get the coordinates
  vtkMRMLAnnotationControlPointsNode * controlPointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);

  if (!controlPointsNode)
    {
    vtkErrorMacro("GetLightboxIndex: Could not get the controlpoints node.");
    return index;
    }

  if (controlPointIndex < 0 ||
      controlPointIndex >= controlPointsNode->GetNumberOfControlPoints())
    {
    return index;
    }

  double transformedWorldCoordinates[4];
  controlPointsNode->GetControlPointWorldCoordinates(controlPointIndex, transformedWorldCoordinates);
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates,displayCoordinates);

  index = (int)(floor(displayCoordinates[2]+0.5));


  return index;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::AddAnnotation(vtkMRMLAnnotationNode *annotationNode)
{
  if (!annotationNode || !this->GetMRMLScene())
    {
    return false;
    }

    // Node added should not be already managed
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it = std::find(
      this->Helper->AnnotationNodeList.begin(),
      this->Helper->AnnotationNodeList.end(),
      annotationNode);
  if (it != this->Helper->AnnotationNodeList.end())
    {
    vtkErrorMacro("AddAnnotation: This node is already associated to the displayable manager!");
    return false;
    }

  // There should not be a widget for the new node
  if (this->Helper->GetWidget(annotationNode) != nullptr)
    {
    vtkErrorMacro("AddAnnotation: A widget is already associated to this node!");
    return false;
    }

  vtkDebugMacro("AddAnnotation: ThreeD -> CreateWidget");

  // Create the Widget and add it to the list.
  vtkAbstractWidget* newWidget = this->CreateWidget(annotationNode);
  if (!newWidget)
    {
    vtkErrorMacro("AddAnnotation: Widget was not created!");
    return false;
    }
  this->Helper->Widgets[annotationNode] = newWidget;

  // Add the node to the list.
  this->Helper->AnnotationNodeList.push_back(annotationNode);

  // Refresh observers
  this->SetAndObserveNode(annotationNode);

  // TODO do we need this render call?
  this->RequestRender();

  // tear down widget creation
  this->OnWidgetCreated(newWidget, annotationNode);

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    return false;
    }
  if (!this->IsCorrectDisplayableManager())
    {
    return false;
    }
  if (eventData->GetType() != vtkCommand::LeftButtonPressEvent
    && eventData->GetType() != vtkCommand::LeftButtonReleaseEvent
    && eventData->GetType() != vtkCommand::RightButtonPressEvent
    && eventData->GetType() != vtkCommand::RightButtonReleaseEvent)
    {
    return false;
    }
  if (this->GetInteractionNode()->GetCurrentInteractionMode() != vtkMRMLInteractionNode::Place)
    {
    return false;
    }
  closestDistance2 = 0.0;
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  this->OnInteractorStyleEvent(eventData->GetType());
  return true;
}
