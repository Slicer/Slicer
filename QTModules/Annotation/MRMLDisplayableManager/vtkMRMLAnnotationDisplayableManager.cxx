// AnnotationModule includes
#include "vtkMRMLAnnotationDisplayableManager.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationControlPointsNode.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

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
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);


  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
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
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneAboutToBeImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Not the correct displayableManager, jumping out!")
    return;
    }

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

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
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    // Exit here, if this is not the right displayableManager
    return;
  }
  this->Helper->Widgets[annotationNode] = newWidget;

  // Add the node to the list.
  this->Helper->AnnotationNodeList.push_back(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

  this->RequestRender();

  // for the following, deactivate tracking of mouse clicks b/c it might be simulated

  this->m_DisableInteractorStyleEventsProcessing = 1;
  // tear down widget creation
  this->OnWidgetCreated(newWidget, annotationNode);
  this->m_DisableInteractorStyleEventsProcessing = 0;

  // Remove all placed seeds
  this->Helper->RemoveSeeds();

  this->RequestRender();

  if(this->m_SliceNode)
    {
    // force a OnMRMLSliceNodeModified() call to hide/show widgets according to the selected slice
    this->OnMRMLSliceNodeModifiedEvent(this->m_SliceNode);
    }

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
  /*
  // Remove the node from the list.
  vtkInternal::AnnotationNodeListIt it = std::find(
      this->Internal->AnnotationNodeList.begin(),
      this->Internal->AnnotationNodeList.end(),
      annotationNode);
  if (it == this->Internal->AnnotationNodeList.end())
    {
    return;
    }
  this->Internal->AnnotationNodeList.erase(it);
  */

  // Remove the widget from the list.
  this->Helper->RemoveWidget(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

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

  // Update the standard settings of all widgets.
  this->Helper->UpdateWidget(annotationNode);

  vtkAbstractWidget * widget = this->Helper->GetWidget(annotationNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(annotationNode, widget);

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
void vtkMRMLAnnotationDisplayableManager::OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode * sliceNode)
{

  if (!sliceNode)
    {
    vtkErrorMacro("OnMRMLSliceNodeModifiedEvent: Could not get the sliceNode.")
    return;
    }

  // set which coordinate index of the annotation node should be used as a reference
  // to be compared against the active slice of the sliceNode
  // default is the Z coordinate
  int variableCoordinateIndex = 2;

  if (!strcmp(sliceNode->GetName(),"redAxial"))
    {
    // axial: the z coordinate
    variableCoordinateIndex = 2;
    }
  else if (!strcmp(sliceNode->GetName(),"yellowSagittal"))
    {
    // sagittal: the x coordinate
    variableCoordinateIndex = 0;
    }
  else if (!strcmp(sliceNode->GetName(),"greenCoronal"))
    {
    // coronal: the y coordinate
    variableCoordinateIndex = 1;
    }

  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
    // by default, we want to show the associated widget
    bool showWidget = true;

    // we loop through all nodes and down cast them as a controlpoints node to get the coordinate
    vtkMRMLAnnotationNode * node = *it;
    vtkMRMLAnnotationControlPointsNode * controlPointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);

    for (int i=0; i<controlPointsNode->GetNumberOfControlPoints(); i++)
      {
      // we loop through all controlpoints of each node
      double * worldCoordinates = controlPointsNode->GetControlPointCoordinates(i);

      // compare the activeSlice value against the coordinate at the variableCoordinateIndex
      // for this, we normalize the difference by getting the absolute value
      double normalizedDifference = abs(sliceNode->GetSliceOffset() - worldCoordinates[variableCoordinateIndex]);

      if (normalizedDifference > 1.2)
        {
        // if the absolute value is more than 2, we know that at least one coordinate of the widget is outside the current activeSlice
        // hence, we do not want to show this widget
        showWidget = false;
        // we don't even need to continue parsing the controlpoints, because we know the widget will not be shown
        break;
        }

      } // end of for loop through control points

    // now this is the magical part
    // we know if all points of a widget are on the activeSlice of the sliceNode (including the tolerance)
    // thus we will only enable the widget if they are
    vtkAbstractWidget * widget = this->Helper->GetWidget(controlPointsNode);

    if (!widget)
      {
      vtkErrorMacro("OnMRMLSliceNodeModifiedEvent: We could not get the widget to the node: " << node->GetID());
      return;
      }

    widget->SetEnabled(showWidget);

    ++it;
    }

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
      this->OnClickInRenderWindowGetCoordinates();
      }
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
  //int rawY = this->GetInteractor()->GetEventPosition()[1];
  //this->GetInteractor()->SetEventPositionFlipY(x, rawY);
  double y = this->GetInteractor()->GetEventPosition()[1];


  this->OnClickInRenderWindow(x, y);
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

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();

    double displayCoordinates[4];
    displayCoordinates[0] = x;
    displayCoordinates[1] = y;
    displayCoordinates[2] = 0;
    displayCoordinates[3] = 1;

    xyToRasMatrix->MultiplyPoint(displayCoordinates, worldCoordinates);

    }
  else
    {
    vtkInteractorObserver::ComputeDisplayToWorld(this->GetRenderer(),x,y,0,worldCoordinates);
    }
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
    vtkMatrix4x4 * rasToXyMatrix = vtkMatrix4x4::New();

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

  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);

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
void vtkMRMLAnnotationDisplayableManager::OnClickInRenderWindow(double x, double y)
{
  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInThreeDRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{
  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{
  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateMRMLToWidget should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}
