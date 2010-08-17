// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationRulerDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationRulerNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkDistanceWidget.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkInteractorEventRecorder.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationRulerDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationRulerDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationRulerDisplayableManager Callback
class vtkAnnotationRulerWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationRulerWidgetCallback *New()
  { return new vtkAnnotationRulerWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
    {
      std::cout << "HoverEvent\n";
    }
    if (event == vtkCommand::EndInteractionEvent)
      {
      std::cout << "EndInteraction" << std::endl;
      vtkDistanceRepresentation2D *rep = vtkDistanceRepresentation2D::SafeDownCast(this->m_Widget->GetRepresentation());

      double position1[3];
      double position2[3];

      rep->GetPoint1WorldPosition(position1);
      rep->GetPoint2WorldPosition(position2);

      this->m_Node->SetPosition1(position1);
      this->m_Node->SetPosition2(position2);

      this->m_Node->SetDistanceMeasurement(rep->GetDistance());
      this->m_Node->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent,this->m_Node);
      }
  }
  vtkAnnotationRulerWidgetCallback(){}
  ~vtkAnnotationRulerWidgetCallback()
  {
    this->m_Widget = 0;
    this->m_Node = 0;
  }

  vtkDistanceWidget * m_Widget;
  vtkMRMLAnnotationRulerNode * m_Node;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationRulerDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationRulerDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
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

  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("CreateWidget: Could not get spline node!")
    return 0;
    }

  vtkDistanceWidget * rulerWidget = vtkDistanceWidget::New();

  rulerWidget->SetInteractor(this->GetInteractor());
  rulerWidget->SetCurrentRenderer(this->GetRenderer());

  // add observer for end interaction
  vtkAnnotationRulerWidgetCallback *myCallback = vtkAnnotationRulerWidgetCallback::New();
  myCallback->m_Node = rulerNode;
  myCallback->m_Widget = rulerWidget;
  rulerWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  rulerWidget->CreateDefaultRepresentation();

  rulerWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up")

  return rulerWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationRulerDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // nothing yet
}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationRulerDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

  o << "EnterEvent 2 184 0 0 0 0 0\n";
  o << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o << "ExitEvent 192 173 0 0 113 1 q\n";

  recorder->SetInputString(o.str().c_str());
  recorder->Play();
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationRulerDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(2))
    {

    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

    // we can't set coordinates here to MRML, we will do it later

    rulerNode->Initialize(this->GetMRMLScene());

    rulerNode->SetName(rulerNode->GetScene()->GetUniqueNameByString("AnnotationRuler"));

    rulerNode->Delete();

    }

  }
