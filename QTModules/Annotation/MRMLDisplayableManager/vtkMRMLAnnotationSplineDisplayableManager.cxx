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
  //splineWidget->SetCurrentRenderer(this->GetRenderer());
/*
  // add observer for end interaction
  vtkAnnotationSplineWidgetCallback *myCallback = vtkAnnotationSplineWidgetCallback::New();
  myCallback->m_Node = splineNode;
  myCallback->m_Widget = splineWidget;
  splineWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();
*/
  splineWidget->On();
  /*
  vtkHandleWidget *h1 = this->m_HandleWidgetList[0];
  vtkHandleWidget *h2 = this->m_HandleWidgetList[1];
  vtkHandleWidget *h3 = this->m_HandleWidgetList[2];
  vtkHandleWidget *h4 = this->m_HandleWidgetList[3];
  vtkHandleWidget *h5 = this->m_HandleWidgetList[4];

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

  double* position3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();

  double* position4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetDisplayPosition();

  double* position5 = vtkHandleRepresentation::SafeDownCast(h5->GetRepresentation())->GetDisplayPosition();


  splineWidget->SetHandlePosition(0,position1);
  splineWidget->SetHandlePosition(1,position2);
  splineWidget->SetHandlePosition(2,position3);
  splineWidget->SetHandlePosition(3,position4);
  splineWidget->SetHandlePosition(4,position5);

  // save the world coordinates of the points to MRML
  splineNode->SetControlPoint(this->GetDisplayToWorldCoordinates(position1[0],position1[1]),0);
  splineNode->SetControlPoint(this->GetDisplayToWorldCoordinates(position2[0],position2[1]),1);
  splineNode->SetControlPoint(this->GetDisplayToWorldCoordinates(position3[0],position3[1]),2);
  splineNode->SetControlPoint(this->GetDisplayToWorldCoordinates(position4[0],position4[1]),3);
  splineNode->SetControlPoint(this->GetDisplayToWorldCoordinates(position5[0],position5[1]),4);
*/
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
void vtkMRMLAnnotationSplineDisplayableManager::OnWidgetCreated()
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  VTK_CREATE(vtkInteractorEventRecorder, recorder);
  recorder->SetInteractor(this->GetInteractor());
  recorder->ReadFromInputStringOn();

  vtkHandleWidget *h1 = this->m_HandleWidgetList[0];
  vtkHandleWidget *h2 = this->m_HandleWidgetList[1];
  vtkHandleWidget *h3 = this->m_HandleWidgetList[2];
  vtkHandleWidget *h4 = this->m_HandleWidgetList[3];
  vtkHandleWidget *h5 = this->m_HandleWidgetList[4];

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

  double* position3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();

  double* position4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetDisplayPosition();

  double* position5 = vtkHandleRepresentation::SafeDownCast(h5->GetRepresentation())->GetDisplayPosition();

  std::ostringstream o;

  // HACK TO DO!!

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationSplineDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{

  std::cout << "sppppline1" << std::endl;

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  std::cout << "sppppline2" << std::endl;

  if (this->m_ClickCounter->HasEnoughClicks(5))
    {

    vtkMRMLAnnotationSplineNode *splineNode = vtkMRMLAnnotationSplineNode::New();

    splineNode->Initialize(this->GetMRMLScene());

    splineNode->SetName(splineNode->GetScene()->GetUniqueNameByString("AnnotationSpline"));

    splineNode->Delete();

    }

  }
