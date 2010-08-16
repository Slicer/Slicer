// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationAngleDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationAngleNode.h"

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

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
      {
      std::cout << "HoverEvent\n";
      }
    if (event == vtkCommand::EndInteractionEvent)
      {
      //std::cout << "EndInteraction" << std::endl;
      vtkAngleRepresentation3D *rep = vtkAngleRepresentation3D::SafeDownCast(this->m_Widget->GetRepresentation());

      double position1[3];
      double position2[3];
      double position3[3];

      rep->GetPoint1WorldPosition(position1);
      rep->GetPoint2WorldPosition(position2);
      rep->GetCenterWorldPosition(position3);

      this->m_Node->SetPosition1(position1);
      this->m_Node->SetPosition2(position2);
      this->m_Node->SetPositionCenter(position3);

      this->m_Node->SetAngleMeasurement(rep->GetAngle() / M_PI * 180.0);
      this->m_Node->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent,this->m_Node);
      }
  }
  vtkAnnotationAngleWidgetCallback(){}

  void SetWidget(vtkAngleWidget *w)
  {
    this->m_Widget = w;
  }
  void SetNode(vtkMRMLAnnotationAngleNode *n)
  {
    this->m_Node = n;
  }

  vtkAngleWidget * m_Widget;
  vtkMRMLAnnotationAngleNode * m_Node;
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
  //angleWidget->Modified();
  //angleWidget->ProcessEventsOff();

  // add observer for end interaction
  vtkAnnotationAngleWidgetCallback *myCallback = vtkAnnotationAngleWidgetCallback::New();
  myCallback->m_Node = angleNode;
  myCallback->m_Widget = angleWidget;
  angleWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  angleWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up")

  return angleWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing angle widget.
void vtkMRMLAnnotationAngleDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  vtkMRMLAnnotationAngleNode* angleNode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);

  vtkAbstractWidget* angleWidget = this->GetWidget(angleNode);
  if (!angleWidget) {
    vtkErrorMacro("Widget was not found!");
    return;
  }

  // not yet


}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationAngleDisplayableManager::OnWidgetCreated()
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
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

    // create MRML node
    vtkMRMLAnnotationAngleNode *angleNode = vtkMRMLAnnotationAngleNode::New();
    angleNode->Initialize(this->GetMRMLScene());

    angleNode->SetName(angleNode->GetScene()->GetUniqueNameByString("AnnotationAngle"));

    angleNode->Delete();

    } // has enough clicks

}


