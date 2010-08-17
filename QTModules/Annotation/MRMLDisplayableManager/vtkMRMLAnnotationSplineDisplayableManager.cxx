// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationSplineDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationSplineNode.h"

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

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationSplineDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationSplineDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationSplineDisplayableManager Callback
class vtkAnnotationSplineWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationSplineWidgetCallback *New()
  { return new vtkAnnotationSplineWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
    {
      std::cout << "HoverEvent\n";
    }
  }
  vtkAnnotationSplineWidgetCallback(){}
  ~vtkAnnotationSplineWidgetCallback()
  {
    this->m_Widget = 0;
    this->m_Node = 0;
  }

  vtkSplineWidget2 * m_Widget;
  vtkMRMLAnnotationSplineNode * m_Node;
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

  vtkMRMLAnnotationSplineNode* splineNode = vtkMRMLAnnotationSplineNode::SafeDownCast(node);

  if (!splineNode)
    {
    vtkErrorMacro("CreateWidget: Could not get spline node!")
    return 0;
    }

  vtkSplineWidget2 * splineWidget = vtkSplineWidget2::New();

  splineWidget->SetInteractor(this->GetInteractor());
  splineWidget->SetCurrentRenderer(this->GetRenderer());

  // add observer for end interaction
  vtkAnnotationSplineWidgetCallback *myCallback = vtkAnnotationSplineWidgetCallback::New();
  myCallback->m_Node = splineNode;
  myCallback->m_Widget = splineWidget;
  splineWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  splineWidget->On();

  splineWidget->CreateDefaultRepresentation();
  vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation())->SetNumberOfHandles(5);

  vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation())->SetHandlePosition(0,splineNode->GetControlPointCoordinates(0));
  vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation())->SetHandlePosition(1,splineNode->GetControlPointCoordinates(1));
  vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation())->SetHandlePosition(2,splineNode->GetControlPointCoordinates(2));
  vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation())->SetHandlePosition(3,splineNode->GetControlPointCoordinates(3));
  vtkSplineRepresentation::SafeDownCast(splineWidget->GetRepresentation())->SetHandlePosition(4,splineNode->GetControlPointCoordinates(4));

  splineWidget->GetRepresentation()->EndWidgetInteraction(splineNode->GetControlPointCoordinates(4));

  vtkDebugMacro("CreateWidget: Widget was set up")

  return splineWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationSplineDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
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
void vtkMRMLAnnotationSplineDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // nothing yet
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationSplineDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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

    vtkHandleWidget *h1 = this->m_HandleWidgetList[0];
    vtkHandleWidget *h2 = this->m_HandleWidgetList[1];
    vtkHandleWidget *h3 = this->m_HandleWidgetList[2];
    vtkHandleWidget *h4 = this->m_HandleWidgetList[3];
    vtkHandleWidget *h5 = this->m_HandleWidgetList[4];

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

    splineNode->Initialize(this->GetMRMLScene());

    splineNode->SetName(splineNode->GetScene()->GetUniqueNameByString("AnnotationSpline"));

    splineNode->Delete();

    }

  }
