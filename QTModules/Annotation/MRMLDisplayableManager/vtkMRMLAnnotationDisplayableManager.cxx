// AnnotationModule includes
#include "vtkMRMLAnnotationDisplayableManager.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkMRMLAnnotationRulerNode.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

#include "vtkMRMLModelDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkMath.h>
#include <vtkRendererCollection.h>
#include <vtkPropCollection.h>
#include <vtkWidgetRepresentation.h>
#include <vtkHandleRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkSeedWidget.h>
#include <vtkSeedRepresentation.h>
#include <vtkProperty2D.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

typedef void (*fp)(void);

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationDisplayableManager, "$Revision: 1.2 $");

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::vtkMRMLAnnotationDisplayableManager()
{
  this->Helper = vtkMRMLAnnotationDisplayableManagerHelper::New();
  this->m_ClickCounter = vtkMRMLAnnotationClickCounter::New();
  this->m_DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;

  this->m_Focus = "vtkMRMLAnnotationNode";

  // by default, this displayableManager handles a ThreeDView
  this->m_SliceNode = 0;

}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::~vtkMRMLAnnotationDisplayableManager()
{

  this->m_DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;
  this->m_Focus = 0;

  this->Helper->Delete();
  this->m_ClickCounter->Delete();

  this->m_SliceNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::SetAndObserveNodes()
{
  VTK_CREATE(vtkIntArray, nodeEvents);
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLAnnotationNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLAnnotationNode::CancelPlacementEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);


  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
    //vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast((*it).GetPointer());

    vtkSetAndObserveMRMLNodeEventsMacro(*it, *it, nodeEvents);

    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::Create()
{

  // hack to force initialization of the renderview
  this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
  this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelForwardEvent);

  //this->DebugOn();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::RemoveMRMLObservers()
{
  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
    vtkSetAndObserveMRMLNodeEventsMacro(*it, *it, NULL);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == NULL || this->m_Focus == NULL)
    {
    return;
    }
  // check if there are any of these nodes in the scene
  if (this->GetMRMLScene()->GetNumberOfNodesByClass(this->m_Focus) < 1)
    {
    return;
    }
  // loop over the nodes for which this manager provides widgets
  this->GetMRMLScene()->InitTraversal();
  vtkMRMLNode *node = this->GetMRMLScene()->GetNextNodeByClass(this->m_Focus);
  // turn off update from mrml requested, as we're doing it now, and create
  // widget requests a render which checks this flag before calling update
  // from mrml again
  this->SetUpdateFromMRMLRequested(0);
  while (node != NULL)
    {
    vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
    if (annotationNode)
      {
      // do we  have a widget for it?
      if (this->GetWidget(annotationNode) == NULL)
        {
        vtkDebugMacro("UpdateFromMRML: creating a widget for node " << annotationNode->GetID());
        vtkAbstractWidget *widget = this->CreateWidget(annotationNode);
        if (widget)
          {
          // update the new widget from the node
          this->PropagateMRMLToWidget(annotationNode, widget);
          }
        }
      }
    node = this->GetMRMLScene()->GetNextNodeByClass(this->m_Focus);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnMRMLSceneClosedEvent();

  vtkDebugMacro("SetMRMLSceneInternal: add observer on interaction node now?");

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
                                                            unsigned long event,
                                                            void *callData)
{
  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(caller);
  if (annotationNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLAnnotationNodeTransformModifiedEvent(annotationNode);
        break;
      case vtkMRMLAnnotationNode::LockModifiedEvent:
        this->OnMRMLAnnotationNodeLockModifiedEvent(annotationNode);
        break;
      case vtkMRMLAnnotationNode::CancelPlacementEvent:
        this->Helper->RemoveSeeds();
        break;
      }
    }
  else
    {
    this->Superclass::ProcessMRMLEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneAboutToBeClosedEvent()
{

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneClosedEvent()
{
  vtkDebugMacro("OnMRMLSceneClosedEvent: remove observers?");
  // run through all nodes and remove node and widget
  this->Helper->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneAboutToBeImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneImportedEvent()
{
  this->UpdateFromMRML();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
    {
    return;
    }
  
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");

  // if the scene is still updating, jump out
  if (this->GetMRMLScene()->GetIsUpdating())
    {
//    return;
    }
  
  if (!node->IsA(this->m_Focus))
    {
    // jump out
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Not the correct displayableManager for node " << node->GetID() << ", jumping out!")
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
  
  // Node added should not be already managed
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it = std::find(
      this->Helper->AnnotationNodeList.begin(),
      this->Helper->AnnotationNodeList.end(),
      annotationNode);
  if (it != this->Helper->AnnotationNodeList.end())
    {
      vtkErrorMacro("OnMRMLSceneNodeAddedEvent: This node is already associated to the displayable manager!")
      return;
    }

  // There should not be a widget for the new node
  if (this->Helper->GetWidget(annotationNode) != 0)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: A widget is already associated to this node!");
    return;
    }

  //std::cout << "OnMRMLSceneNodeAddedEvent ThreeD -> CreateWidget" << std::endl;

  // Create the Widget and add it to the list.
  vtkAbstractWidget* newWidget = this->CreateWidget(annotationNode);
  if (!newWidget) {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    // Exit here, if this is not the right displayableManager
    return;
  }
  this->Helper->Widgets[annotationNode] = newWidget;

  // Add the node to the list.
  this->Helper->AnnotationNodeList.push_back(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

  // TODO do we need this render call?
  this->RequestRender();

  // tear down widget creation
  this->OnWidgetCreated(newWidget, annotationNode);

  // Remove all placed seeds
  this->Helper->RemoveSeeds();

  // and render again after seeds were removed
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
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
  this->SetAndObserveNodes();

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
    vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeModifiedEvent: Can not access node.")
    return;
    }

  //std::cout << "OnMRMLAnnotationNodeModifiedEvent ThreeD->PropagateMRMLToWidget" << std::endl;

  vtkAbstractWidget * widget = this->Helper->GetWidget(annotationNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(annotationNode, widget);

  if(this->m_SliceNode)
    {
    // force a OnMRMLSliceNodeModified() call to hide/show widgets according to the selected slice
    this->OnMRMLSliceNodeModifiedEvent(this->m_SliceNode);

    // Update the standard settings of all widgets if the widget is displayable in the current geoemtry
    if (this->IsWidgetDisplayable(this->m_SliceNode, annotationNode))
      {
      // in 2D, the widget is displayable at this point so check its visibility or lock status from MRML
      this->Helper->UpdateWidget(annotationNode);
      }
    }
  else
    {
    // in 3D, always update the widget according to the mrml settings of lock and visibility status
    this->Helper->UpdateWidget(annotationNode);
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeTransformModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeTransformModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateWidget(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeLockModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeLockModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateWidget(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{

  vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent");

  if (!caller)
    {
    vtkErrorMacro("OnMRMLDisplayableNodeModifiedEvent: Could not get caller.")
    return;
    }

  vtkMRMLSliceNode * sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);

  if (sliceNode)
    {
    // the associated renderWindow is a 2D SliceView
    // this is the entry point for all events fired by one of the three sliceviews
    // (f.e. change slice number, zoom etc.)

    // we remember that this instance of the displayableManager deals with 2D
    // this is important for widget creation etc. and save the actual SliceNode
    // because during Slicer startup the SliceViews fire events, it will be always set correctly
    this->m_SliceNode = sliceNode;

    // now we call the handle for specific sliceNode actions
    this->OnMRMLSliceNodeModifiedEvent(sliceNode);

    // and exit
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
vtkMRMLSliceNode * vtkMRMLAnnotationDisplayableManager::GetSliceNode()
{

  return this->m_SliceNode;

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode* sliceNode)
{

  if (!sliceNode)
    {
    vtkErrorMacro("OnMRMLSliceNodeModifiedEvent: Could not get the sliceNode.")
    return;
    }

  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
    // by default, we want to show the associated widget
    bool showWidget = true;

    // we loop through all nodes
    vtkMRMLAnnotationNode * annotationNode = *it;

    // check if the widget is displayable
    showWidget = this->IsWidgetDisplayable(sliceNode, annotationNode);

    // now this is the magical part
    // we know if all points of a widget are on the activeSlice of the sliceNode (including the tolerance)
    // thus we will only enable the widget if they are
    vtkAbstractWidget* widget = this->Helper->GetWidget(annotationNode);

    if (!widget)
      {
      vtkErrorMacro("OnMRMLSliceNodeModifiedEvent: We could not get the widget to the node: " << annotationNode->GetID());
      return;
      }

    // check if the widget is visible according to its mrml node
    if (annotationNode->GetVisible())
      {
      // only then update the visibility according to the geometry
      // and only adjust the widget settings if there was a change
      if ((widget->GetEnabled() && !showWidget) || (!widget->GetEnabled() && showWidget))
        {
        widget->SetEnabled(showWidget);
        }

      }

    // if the widget is not displayable, show at least the intersection
    if (!showWidget)
      {

      vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode);
      if (rulerNode)
        {

        double* tmpPtr = rulerNode->GetControlPointCoordinates(0);
        double p1[3] = { tmpPtr[0], tmpPtr[1], tmpPtr[2] };

        tmpPtr = rulerNode->GetControlPointCoordinates(1);
        double p2[3] = { tmpPtr[0], tmpPtr[1], tmpPtr[2] };

        //std::cout << this->m_SliceNode->GetName() << " original ras1: " << p1[0] << "," << p1[1] << "," << p1[2] << std::endl;
        //std::cout << this->m_SliceNode->GetName() << " original ras2: " << p2[0] << "," << p2[1] << "," << p2[2] << std::endl;

        VTK_CREATE(vtkMatrix4x4, transformMatrix);

        transformMatrix->Identity();

        if (annotationNode->GetTransformNodeID())
          {
          // if annotation is under transform, get the transformation matrix

          vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(annotationNode->GetTransformNodeID()));

          if (transformNode)
            {
            transformNode->GetMatrixTransformToWorld(transformMatrix);
            }

          }


        double extendedP1[4];
        extendedP1[0] = p1[0];
        extendedP1[1] = p1[1];
        extendedP1[2] = p1[2];
        extendedP1[3] = 1;

        double transformedP1[4] = {0,0,0,0};

        double displayP1[4] = {0,0,0,0};

        double extendedP2[4];
        extendedP2[0] = p2[0];
        extendedP2[1] = p2[1];
        extendedP2[2] = p2[2];
        extendedP2[3] = 1;

        double transformedP2[4] = {0,0,0,0};

        double displayP2[4] = {0,0,0,0};


        // now multiply with the transformMatrix
        // if there is a valid transform, the coordinates get transformed else the transformMatrix is the identity matrix and
        // does not change the coordinates
        transformMatrix->MultiplyPoint(extendedP1,transformedP1);
        transformMatrix->MultiplyPoint(extendedP2,transformedP2);

        // now get the displayCoordinates for the transformed worldCoordinates
        this->GetWorldToDisplayCoordinates(transformedP1,displayP1);
        this->GetWorldToDisplayCoordinates(transformedP2,displayP2);

        //std::cout << this->m_SliceNode->GetName() << " ras1: " << p1[0] << "," << p1[1] << "," << p1[2] << std::endl;
        //std::cout << this->m_SliceNode->GetName() << " ras2: " << p2[0] << "," << p2[1] << "," << p2[2] << std::endl;

        //std::cout << this->m_SliceNode->GetName() << " display1: " << displayP1[0] << "," << displayP1[1] << "," << displayP1[2] << std::endl;
        //std::cout << this->m_SliceNode->GetName() << " display2: " << displayP2[0] << "," << displayP2[1] << "," << displayP2[2] << std::endl;

        // get line between p1 and p2
        // g(x) = p1 + r*(p2-p1)
        //
        // compute intersection with slice plane
        // if !=0: mark the intersection

        //double this->m_SliceNode->GetSliceOffset() = p1[2] + (p2[2]-p1[2])*t;
        // t = (this->m_SliceNode->GetSliceOffset() - p1[2]) / (p2[2]-p1[2])
        double t = (this->m_SliceNode->GetSliceOffset()-displayP1[2]) / (displayP2[2]-displayP1[2]);

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

          VTK_CREATE(vtkPointHandleRepresentation2D, handle);
          handle->GetProperty()->SetColor(0,1,0);
          handle->SetHandleSize(3);

          VTK_CREATE(vtkSeedRepresentation, rep);
          rep->SetHandleRepresentation(handle);

          marker = vtkSeedWidget::New();

          marker->CreateDefaultRepresentation();

          marker->SetRepresentation(rep);

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
        VTK_CREATE(vtkHandleWidget, newhandle);
        newhandle = marker->CreateNewHandle();
        vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(P1plusP2minusP1);

        marker->On();
        marker->CompleteInteraction();

        //std::cout << this->m_SliceNode->GetName() << ": " << P1plusP2minusP1[0] << "," << P1plusP2minusP1[1] << "," << P1plusP2minusP1[2] << std::endl;

        }

      }

    this->PropagateMRMLToWidget(annotationNode,widget);

    ++it;
    }

}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::IsWidgetDisplayable(vtkMRMLSliceNode* sliceNode, vtkMRMLAnnotationNode* node)
{

  if (!sliceNode)
    {
    vtkErrorMacro("IsWidgetDisplayable: Could not get the sliceNode.")
    return 0;
    }

  if (!node)
    {
    vtkErrorMacro("IsWidgetDisplayable: Could not get the annotation node.")
    return 0;
    }

  VTK_CREATE(vtkMatrix4x4, transformMatrix);

  transformMatrix->Identity();

  if (node->GetTransformNodeID())
    {
    // if annotation is under transform, get the transformation matrix

    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(node->GetTransformNodeID()));

    if (transformNode)
      {
      transformNode->GetMatrixTransformToWorld(transformMatrix);
      }

    }

  bool showWidget = true;

  // down cast the node as a controlpoints node to get the coordinates
  vtkMRMLAnnotationControlPointsNode * controlPointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);

  if (!controlPointsNode)
    {
    vtkErrorMacro("IsWidgetDisplayable: Could not get the controlpoints node.")
    return 0;
    }

  for (int i=0; i<controlPointsNode->GetNumberOfControlPoints(); i++)
    {
    // we loop through all controlpoints of each node
    double* worldCoordinates = controlPointsNode->GetControlPointCoordinates(i);

    double extendedWorldCoordinates[4];
    extendedWorldCoordinates[0] = worldCoordinates[0];
    extendedWorldCoordinates[1] = worldCoordinates[1];
    extendedWorldCoordinates[2] = worldCoordinates[2];
    extendedWorldCoordinates[3] = 1;

    double transformedWorldCoordinates[4];

    double displayCoordinates[4];

    // now multiply with the transformMatrix
    // if there is a valid transform, the coordinates get transformed else the transformMatrix is the identity matrix and
    // does not change the coordinates
    transformMatrix->MultiplyPoint(extendedWorldCoordinates,transformedWorldCoordinates);

    // now get the displayCoordinates for the transformed worldCoordinates
    this->GetWorldToDisplayCoordinates(transformedWorldCoordinates,displayCoordinates);

    //
    // Lightbox specific code
    //

    // get the right renderer index by checking the z coordinate
    int rendererIndex = (int)(displayCoordinates[2]+0.5);

    // get all renderers associated with this renderWindow
    // when lightbox mode is enabled, there will be different renderers associated with the renderWindow of the sliceView
    vtkRendererCollection* rendererCollection = this->GetInteractor()->GetRenderWindow()->GetRenderers();

    // check if lightbox is enabled
    if (sliceNode->GetLayoutGridRows() > 1 || sliceNode->GetLayoutGridColumns() > 1)
      {

      // check if the rendererIndex is valid for the current lightbox view
      if (rendererIndex >= 0 && rendererIndex < rendererCollection->GetNumberOfItems())
        {

        vtkRenderer* currentRenderer = vtkRenderer::SafeDownCast(rendererCollection->GetItemAsObject(rendererIndex));

        // now we get the widget..
        vtkAbstractWidget* widget = this->GetWidget(node);

        // TODO this code blocks the movement of the widget in lightbox mode

        // ..and turn it off..
        widget->Off();
        // ..place it and its representation to the right renderer..
        widget->SetCurrentRenderer(currentRenderer);
        widget->GetRepresentation()->SetRenderer(currentRenderer);
        // ..and turn it on again!
        widget->On();
        // if it's a seed widget, go to complete interaction state
        vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
        if (seedWidget)
          {
          vtkDebugMacro("SeedWidget: Complete interaction");
          seedWidget->CompleteInteraction();
          }
        
        // we need to render again
        currentRenderer->Render();

        }

      }

    //
    // End of Lightbox specific code
    //

    // the third coordinate of the displayCoordinates is the distance to the slice
    float distanceToSlice = displayCoordinates[2];

    if (distanceToSlice < -0.5 || distanceToSlice >= (0.5+this->m_SliceNode->GetDimensions()[2]-1))
      {
      // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
      // hence, we do not want to show this widget
      showWidget = false;
      // we don't even need to continue parsing the controlpoints, because we know the widget will not be shown
      break;
      }

    } // end of for loop through control points

  return showWidget;

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  if (this->m_DisableInteractorStyleEventsProcessing == 1)
    {
    vtkDebugMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }
  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      vtkDebugMacro("OnInteractorStyleEvent got a left button release " << eventid << ", and are in place mode, calling OnClickInRenderWindowGetCoordinates");
      this->OnClickInRenderWindowGetCoordinates();
      }
    //else { vtkWarningMacro("OnInteractorStyleEvent: not in vtkMRMLInteractionNode::Place interaction mode"); }
    }
  else if (eventid == vtkCommand::LeftButtonPressEvent)
    {
    //vtkWarningMacro("OnInteractorStyleEvent: unhandled left button press event " << eventid);
    }
  else
    {
    //vtkWarningMacro("OnInteractorStyleEvent: unhandled event " << eventid);
    //std::cout << "AnnotationDisplayableManager: OnInteractorStyleEvent: unhandled event " << eventid << std::endl;
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
    this->OnClickInRenderWindow(x, y);
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

  if (this->GetSliceNode())
    {
    // 2D case

    // we will get the transformation matrix to convert display coordinates to RAS

    double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
    double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

    int numberOfColumns = this->GetSliceNode()->GetLayoutGridColumns();
    int numberOfRows = this->GetSliceNode()->GetLayoutGridRows();

    float tempX = x / windowWidth;
    float tempY = (windowHeight - y) / windowHeight;

    float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

    vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x,y);

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();

    double displayCoordinates[4];
    displayCoordinates[0] = x - pokedRenderer->GetOrigin()[0];
    displayCoordinates[1] = y - pokedRenderer->GetOrigin()[1];
    displayCoordinates[2] = z;
    displayCoordinates[3] = 1;

    xyToRasMatrix->MultiplyPoint(displayCoordinates, worldCoordinates);

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

  if (this->GetSliceNode())
    {
    // 2D case

    // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();
    VTK_CREATE(vtkMatrix4x4, rasToXyMatrix);

    // we need to invert this matrix
    xyToRasMatrix->Invert(xyToRasMatrix,rasToXyMatrix);

    double worldCoordinates[4];
    worldCoordinates[0] = r;
    worldCoordinates[1] = a;
    worldCoordinates[2] = s;
    worldCoordinates[3] = 1;

    rasToXyMatrix->MultiplyPoint(worldCoordinates,displayCoordinates);

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
  if (worldCoordinates == NULL)
    {
    return;
    }
  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);

}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLAnnotationDisplayableManager::GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates)
{

  if (viewportCoordinates == NULL)
    {
    return;
    }
  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];
  
  if (this->GetSliceNode())
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
    vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y << ", display coords calc as " << displayCoordinates[0] << ", " << displayCoordinates[1] << ", returning viewport = " << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
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
    vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y << ", returning viewport = " << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
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
    vtkErrorMacro("RestrictDisplayCoordinatesToViewport: Could not find the poked renderer!")
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

  if (fabs(displayCoordinates1[0]-displayCoordinates2[0])>0.1 || fabs(displayCoordinates1[1]-displayCoordinates2[1])>0.1) {
    changed = true;
  }

  return changed;
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLAnnotationDisplayableManager::GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2)
{
  bool changed = false;

  if (fabs(worldCoordinates1[0]-worldCoordinates2[0])>0.1 ||
      fabs(worldCoordinates1[1]-worldCoordinates2[1])>0.1 ||
      fabs(worldCoordinates1[2]-worldCoordinates2[2])>0.1) {
    changed = true;
  }

  return changed;
}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
//---------------------------------------------------------------------------
bool vtkMRMLAnnotationDisplayableManager::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLSelectionNode"));
  if ( selectionNode == NULL )
    {
    vtkErrorMacro ( "OnClickInThreeDRenderWindow: No selection node in the scene." );
    return false;
    }
  if ( selectionNode->GetActiveAnnotationID() == NULL)
    {
    return false;
    }
  // the purpose of the displayableManager is hardcoded
  return !strcmp(selectionNode->GetActiveAnnotationID(), this->m_Focus);

}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnClickInRenderWindow(double vtkNotUsed(x), double vtkNotUsed(y))
{

  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkMRMLAnnotationDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* vtkNotUsed(node))
{

  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
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
