
// Annotation Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationSplineDisplayableManager.h"

// Annotation MRML includes
#include <vtkMRMLAnnotationSplineNode.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationDisplayableManager.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkSplineWidget2.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkSplineRepresentation.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkAbstractWidget.h>

// std includes
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationSplineDisplayableManager);

//---------------------------------------------------------------------------
// vtkMRMLAnnotationSplineDisplayableManager Callback
class vtkAnnotationSplineWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationSplineWidgetCallback *New()
  { return new vtkAnnotationSplineWidgetCallback; }

  vtkAnnotationSplineWidgetCallback(){}

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::EndInteractionEvent)
      {

      // sanity checks
      if (!this->m_DisplayableManager)
        {
        return;
        }
      if (!this->m_Node)
        {
        return;
        }
      if (!this->m_Widget)
        {
        return;
        }
      // sanity checks end

      // the interaction with the widget ended, now propagate the changes to MRML
      this->m_DisplayableManager->PropagateWidgetToMRML(this->m_Widget, this->m_Node);

      }
  }

  void SetWidget(vtkAbstractWidget *w)
  {
    this->m_Widget = w;
  }
  void SetNode(vtkMRMLAnnotationNode *n)
  {
    this->m_Node = n;
  }
  void SetDisplayableManager(vtkMRMLAnnotationDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkMRMLAnnotationNode * m_Node;
  vtkMRMLAnnotationDisplayableManager * m_DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationSplineDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSplineDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationSplineDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!");
    return 0;
    }

  vtkMRMLAnnotationSplineNode* splineNode = vtkMRMLAnnotationSplineNode::SafeDownCast(node);

  if (!splineNode)
    {
    vtkErrorMacro("CreateWidget: Could not get spline node!");
    return 0;
    }

  vtkSplineWidget2 * splineWidget = vtkSplineWidget2::New();

  splineWidget->SetInteractor(this->GetInteractor());
  splineWidget->SetCurrentRenderer(this->GetRenderer());

  splineWidget->On();

  splineWidget->CreateDefaultRepresentation();

  vtkSplineRepresentation * rep = vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation());
  rep->SetNumberOfHandles(5);

  rep->SetHandlePosition(0,splineNode->GetControlPointCoordinates(0));
  rep->SetHandlePosition(1,splineNode->GetControlPointCoordinates(1));
  rep->SetHandlePosition(2,splineNode->GetControlPointCoordinates(2));
  rep->SetHandlePosition(3,splineNode->GetControlPointCoordinates(3));
  rep->SetHandlePosition(4,splineNode->GetControlPointCoordinates(4));

  rep->EndWidgetInteraction(splineNode->GetControlPointCoordinates(4));

  vtkDebugMacro("CreateWidget: Widget was set up");

  return splineWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationSplineDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!");
    return;
    }

  // add observer for end interaction
  vtkAnnotationSplineWidgetCallback *myCallback = vtkAnnotationSplineWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  //this->m_Updating = 0;
  // propagate the widget to the MRML node
  //this->PropagateWidgetToMRML(widget, node);
}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationSplineDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkSplineWidget2 * splineWidget = vtkSplineWidget2::SafeDownCast(widget);

  if (!splineWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get spline widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationSplineNode * splineNode = vtkMRMLAnnotationSplineNode::SafeDownCast(node);

  if (!splineNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get spline node!");
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSplineRepresentation * rep = vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation());

  rep->SetHandlePosition(0,splineNode->GetControlPointCoordinates(0));

  rep->SetHandlePosition(1,splineNode->GetControlPointCoordinates(1));

  rep->SetHandlePosition(2,splineNode->GetControlPointCoordinates(2));

  rep->SetHandlePosition(3,splineNode->GetControlPointCoordinates(3));

  rep->SetHandlePosition(4,splineNode->GetControlPointCoordinates(4));

    rep->NeedToRenderOn();
    splineWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationSplineDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkSplineWidget2 * splineWidget = vtkSplineWidget2::SafeDownCast(widget);

  if (!splineWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get spline widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationSplineNode * splineNode = vtkMRMLAnnotationSplineNode::SafeDownCast(node);

  if (!splineNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get spline node!");
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkSplineRepresentation * rep = vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation());

  double position1[3];
  double position2[3];
  double position3[3];
  double position4[3];
  double position5[3];


  rep->GetHandlePosition(0,position1);
  rep->GetHandlePosition(1,position2);
  rep->GetHandlePosition(2,position3);
  rep->GetHandlePosition(3,position4);
  rep->GetHandlePosition(4,position5);

  splineNode->SetControlPoint(position1, 0);

  splineNode->SetControlPoint(position2,1);

  // at least one coordinate has changed, so update the mrml property
  splineNode->SetControlPoint(position3,2);

  splineNode->SetControlPoint(position4,3);

  // at least one coordinate has changed, so update the mrml property
  splineNode->SetControlPoint(position5,3);


  // at least one value has changed, so fire the modified event
  splineNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, splineNode);

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationSplineDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(5))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);
    vtkHandleWidget *h3 = this->GetSeed(2);
    vtkHandleWidget *h4 = this->GetSeed(3);
    vtkHandleWidget *h5 = this->GetSeed(4);

    double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetWorldPosition();
    double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetWorldPosition();
    double* position3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetWorldPosition();
    double* position4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetWorldPosition();
    double* position5 = vtkHandleRepresentation::SafeDownCast(h5->GetRepresentation())->GetWorldPosition();

    vtkMRMLAnnotationSplineNode *splineNode = vtkMRMLAnnotationSplineNode::New();

    // save the world coordinates of the points to MRML
    splineNode->SetControlPoint(position1,0);
    splineNode->SetControlPoint(position2,1);
    splineNode->SetControlPoint(position3,2);
    splineNode->SetControlPoint(position4,3);
    splineNode->SetControlPoint(position5,4);

    // if this was a one time place, go back to view transform mode
    vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }

    this->GetMRMLScene()->SaveStateForUno();

    // is there a node associated with this?
    if (associatedNodeID)
      {
      splineNode->SetAttribute("AssociatedNodeID", associatedNodeID);
      }

    splineNode->Initialize(this->GetMRMLScene());

    splineNode->SetName(splineNode->GetScene()->GetUniqueNameByString("S"));

    splineNode->Delete();

    // reset updating state
    this->m_Updating = 0;



    }

  }
