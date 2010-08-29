// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationAngleDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkAngleRepresentation3D.h>
#include <vtkSphereHandleRepresentation.h>
#include <vtkAngleWidget.h>
#include <vtkHandleWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkAbstractWidget.h>

// Math includes
#define _USE_MATH_DEFINES
#include <math.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationAngleDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationAngleDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationAngleDisplayableManager Callback
class vtkAnnotationAngleWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationAngleWidgetCallback *New()
  { return new vtkAnnotationAngleWidgetCallback; }

  vtkAnnotationAngleWidgetCallback(){}

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
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return 0;
    }

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("CreateWidget: Could not get angle node!")
    return 0;
    }

  VTK_CREATE(vtkSphereHandleRepresentation, handle);
  handle->GetProperty()->SetColor(1,0,0);
  handle->SetHandleSize(5);

  VTK_CREATE(vtkAngleRepresentation3D, rep);
  rep->SetHandleRepresentation(handle);
  rep->InstantiateHandleRepresentation();

  vtkAngleWidget *angleWidget = vtkAngleWidget::New();
  angleWidget->CreateDefaultRepresentation();
  angleWidget->SetRepresentation(rep);

  angleWidget->SetInteractor(this->GetInteractor());
  angleWidget->SetCurrentRenderer(this->GetRenderer());

  angleWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up")

  return angleWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationAngleDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkAngleWidget* angleWidget = vtkAngleWidget::SafeDownCast(widget);

  if (!angleWidget)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get angle widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get angle node!")
    return;
    }

  VTK_CREATE(vtkInteractorEventRecorder, recorder);
  recorder->SetInteractor(this->GetInteractor());
  recorder->ReadFromInputStringOn();

  std::ostringstream o;
  VTK_CREATE(vtkHandleWidget, h1);
  h1 = this->m_HandleWidgetList[0];
  VTK_CREATE(vtkHandleWidget, h2);
  h2 = this->m_HandleWidgetList[1];
  VTK_CREATE(vtkHandleWidget, h3);
  h3 = this->m_HandleWidgetList[2];

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

  // add observer for end interaction
  vtkAnnotationAngleWidgetCallback *myCallback = vtkAnnotationAngleWidgetCallback::New();
  myCallback->SetNode(angleNode);
  myCallback->SetWidget(angleWidget);
  myCallback->SetDisplayableManager(this);
  angleWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationAngleDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkAngleWidget* angleWidget = vtkAngleWidget::SafeDownCast(widget);

  if (!angleWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get angle widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get angle node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // if this flag is true after the checks below, the widget will be set to modified
  bool hasChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());

  double position1[3];
  double position2[3];
  double position3[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);
  rep->GetCenterWorldPosition(position3);

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the widget
  //
  if (angleNode->GetPosition1()[0] != position1[0] || angleNode->GetPosition1()[1] != position1[1] || angleNode->GetPosition1()[2] != position1[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetPoint1WorldPosition(angleNode->GetPosition1());
    hasChanged = true;
    }

  if (angleNode->GetPosition2()[0] != position2[0] || angleNode->GetPosition2()[1] != position2[1] || angleNode->GetPosition2()[2] != position2[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetPoint2WorldPosition(angleNode->GetPosition2());
    hasChanged = true;
    }

  if (angleNode->GetPositionCenter()[0] != position3[0] || angleNode->GetPositionCenter()[1] != position3[1] || angleNode->GetPositionCenter()[2] != position3[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetCenterWorldPosition(angleNode->GetPositionCenter());
    hasChanged = true;
    }


  if (hasChanged)
    {
    // at least one value has changed, so set the widget to modified
    rep->NeedToRenderOn();
    angleWidget->Modified();
    }

  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationAngleDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkAngleWidget* angleWidget = vtkAngleWidget::SafeDownCast(widget);

  if (!angleWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get angle widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  if (!angleNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get angle node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // if this flag is true after the checks below, the modified event gets fired
  bool hasChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(angleWidget->GetRepresentation());

  double position1[3];
  double position2[3];
  double position3[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);
  rep->GetCenterWorldPosition(position3);

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the mrml node
  //
  if (angleNode->GetPosition1()[0] != position1[0] || angleNode->GetPosition1()[1] != position1[1] || angleNode->GetPosition1()[2] != position1[2])
    {
    // at least one coordinate has changed, so update the mrml property
    angleNode->SetPosition1(position1);
    hasChanged = true;
    }

  if (angleNode->GetPosition2()[0] != position2[0] || angleNode->GetPosition2()[1] != position2[1] || angleNode->GetPosition2()[2] != position2[2])
    {
    // at least one coordinate has changed, so update the mrml property
    angleNode->SetPosition2(position2);
    hasChanged = true;
    }

  if (angleNode->GetPositionCenter()[0] != position3[0] || angleNode->GetPositionCenter()[1] != position3[1] || angleNode->GetPositionCenter()[2] != position3[2])
    {
    // at least one coordinate has changed, so update the mrml property
    angleNode->SetPositionCenter(position3);
    hasChanged = true;
    }

  //
  // Check if the measurement value of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the mrml node
  //
  double angleInDegrees = rep->GetAngle() / M_PI * 180.0;

  if (angleNode->GetAngleMeasurement() != angleInDegrees)
    {
    // the angle has changes, so update the mrml property
    angleNode->SetAngleMeasurement(angleInDegrees);
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so fire the modified event
    angleNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, angleNode);
    }

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Handle clicks in 3D Render Window
void vtkMRMLAnnotationAngleDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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
    angleNode->Initialize(this->GetMRMLScene());

    angleNode->SetName(angleNode->GetScene()->GetUniqueNameByString("AnnotationAngle"));

    angleNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    } // has enough clicks

}


