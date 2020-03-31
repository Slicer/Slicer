
// Annotation Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationAngleDisplayableManager.h"

// Annotation MRML includes
#include <vtkMRMLAnnotationAngleNode.h>
#include <vtkMRMLAnnotationDisplayableManager.h>
#include <vtkMRMLAnnotationNode.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkAngleRepresentation3D.h>
#include <vtkAngleWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkHandleWidget.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkSphereHandleRepresentation.h>

// Math includes
#define _USE_MATH_DEFINES
#include <math.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationAngleDisplayableManager);

//---------------------------------------------------------------------------
// vtkMRMLAnnotationAngleDisplayableManager Callback
class vtkAnnotationAngleWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationAngleWidgetCallback *New()
  { return new vtkAnnotationAngleWidgetCallback; }

  vtkAnnotationAngleWidgetCallback(){}

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*)
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
// vtkMRMLAnnotationAngleDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new angle widget.
vtkAbstractWidget * vtkMRMLAnnotationAngleDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!");
    return 0;
    }

  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("CreateWidget: Could not get angle node!");
    return 0;
    }

  vtkNew<vtkSphereHandleRepresentation> handle;
  handle->GetProperty()->SetColor(1,0,0);
  handle->SetHandleSize(5);

  vtkNew<vtkAngleRepresentation3D> rep;
  rep->SetHandleRepresentation(handle.GetPointer());
  rep->InstantiateHandleRepresentation();

  vtkAngleWidget *angleWidget = vtkAngleWidget::New();
  angleWidget->CreateDefaultRepresentation();
  angleWidget->SetRepresentation(rep.GetPointer());

  angleWidget->SetInteractor(this->GetInteractor());
  angleWidget->SetCurrentRenderer(this->GetRenderer());

  angleWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up");

  return angleWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationAngleDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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

  vtkNew<vtkInteractorEventRecorder> recorder;
  recorder->SetInteractor(this->GetInteractor());
  recorder->ReadFromInputStringOn();

  std::ostringstream o;
  vtkHandleWidget * h1 = this->GetSeed(0);
  vtkHandleWidget * h2 = this->GetSeed(1);
  vtkHandleWidget * h3 = this->GetSeed(2);

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

  double* position3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();

  o << "EnterEvent 2 184 0 0 0 0 0\n";
  o << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o << "MouseMoveEvent " << position3[0] << " " << position3[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position3[0] << " " << position3[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position3[0] << " " << position3[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position3[0] << " " << position3[1] << " 0 0 0 0 t\n";
  o << "ExitEvent 192 173 0 0 113 1 q\n";

  recorder->SetInputString(o.str().c_str());
  recorder->Play();

  // add observer for end interaction, will not fire now
  vtkAnnotationAngleWidgetCallback *myCallback = vtkAnnotationAngleWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  this->m_Updating = 0;
  this->PropagateWidgetToMRML(widget,node);

}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationAngleDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkAngleWidget* angleWidget = vtkAngleWidget::SafeDownCast(widget);

  if (!angleWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get angle widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get angle node!");
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
  vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());

  this->UpdatePosition(widget, node);

  rep->NeedToRenderOn();
  angleWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate position properties of MRML node to widget.
void vtkMRMLAnnotationAngleDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
{

  if (!widget)
    {
    vtkErrorMacro("UpdatePosition: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("UpdatePosition: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkAngleWidget* angleWidget = vtkAngleWidget::SafeDownCast(widget);

  if (!angleWidget)
    {
    vtkErrorMacro("UpdatePosition: Could not get angle widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("UpdatePosition: Could not get angle node!");
    return;
    }

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());
  if (!rep)
    {
    vtkErrorMacro("UpdatePosition: Could not get angle representation!");
    return;
    }
  double position1[3];
  double position2[3];
  double position3[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);
  rep->GetCenterWorldPosition(position3);

  rep->SetPoint1WorldPosition(angleNode->GetPosition1());

  rep->SetPoint2WorldPosition(angleNode->GetPosition2());

  rep->SetCenterWorldPosition(angleNode->GetPositionCenter());

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationAngleDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
  vtkAngleWidget* angleWidget = vtkAngleWidget::SafeDownCast(widget);

  if (!angleWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get angle widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get angle node!");
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
  vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());

  double position1[3];
  double position2[3];
  double position3[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);
  rep->GetCenterWorldPosition(position3);

  angleNode->SetPosition1(position1);

  angleNode->SetPosition2(position2);

  angleNode->SetPositionCenter(position3);

  double angleInDegrees = rep->GetAngle() / M_PI * 180.0;
  angleNode->SetAngleMeasurement(angleInDegrees);

  angleNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, angleNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Handle clicks in 3D Render Window
void vtkMRMLAnnotationAngleDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  // check if the user already clicked enough to create this widget
  if (this->m_ClickCounter->HasEnoughClicks(3))
    {

    // we can't set coordinates here to MRML, we will do it later

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    // create MRML node
    vtkMRMLAnnotationAngleNode *angleNode = vtkMRMLAnnotationAngleNode::New();

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
      angleNode->SetAttribute("AssociatedNodeID", associatedNodeID);
      }

    angleNode->Initialize(this->GetMRMLScene());

    angleNode->SetName(angleNode->GetScene()->GetUniqueNameByString("A"));

    angleNode->Delete();

    // reset updating state
    this->m_Updating = 0;
    } // has enough clicks

}
