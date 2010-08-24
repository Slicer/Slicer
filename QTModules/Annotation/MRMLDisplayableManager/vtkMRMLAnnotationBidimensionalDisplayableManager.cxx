// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationBidimensionalDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationBidimensionalNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkBiDimensionalWidget.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkBiDimensionalRepresentation2D.h>
#include <vtkInteractorEventRecorder.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationBidimensionalDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationBidimensionalDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationBidimensionalDisplayableManager Callback
class vtkAnnotationBidimensionalWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationBidimensionalWidgetCallback *New()
  { return new vtkAnnotationBidimensionalWidgetCallback; }

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if (event == vtkCommand::HoverEvent)
      {
      std::cout << "HoverEvent\n";
      }
    if (event == vtkCommand::EndInteractionEvent)
      {

      // sync MRML Node to widget

      vtkWidgetRepresentation * rep = this->m_Widget->GetRepresentation();
      vtkBiDimensionalRepresentation2D * bidimensionalRep = vtkBiDimensionalRepresentation2D::SafeDownCast(rep);

      double position1[3];
      double position2[3];
      double position3[3];
      double position4[3];

      bidimensionalRep->GetPoint1WorldPosition(position1);
      bidimensionalRep->GetPoint2WorldPosition(position2);
      bidimensionalRep->GetPoint3WorldPosition(position3);
      bidimensionalRep->GetPoint4WorldPosition(position4);

      this->m_Node->SetControlPoint(position1,0);
      this->m_Node->SetControlPoint(position2,1);
      this->m_Node->SetControlPoint(position3,2);
      this->m_Node->SetControlPoint(position4,3);

      this->m_Node->SetBidimensionalMeasurement(bidimensionalRep->GetLength1(),bidimensionalRep->GetLength2());

    }
  }
  vtkAnnotationBidimensionalWidgetCallback(){}
  ~vtkAnnotationBidimensionalWidgetCallback()
  {
    this->m_Widget = 0;
    this->m_Node = 0;
  }

  vtkBiDimensionalWidget * m_Widget;
  vtkMRMLAnnotationBidimensionalNode * m_Node;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationBidimensionalDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationBidimensionalDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
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

  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("CreateWidget: Could not get bidimensional node!")
    return 0;
    }

  vtkBiDimensionalWidget * bidimensionalWidget = vtkBiDimensionalWidget::New();

  bidimensionalWidget->SetInteractor(this->GetInteractor());
  bidimensionalWidget->SetCurrentRenderer(this->GetRenderer());

  bidimensionalWidget->CreateDefaultRepresentation();

  bidimensionalWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up")

  return bidimensionalWidget;

}

//---------------------------------------------------------------------------
/// Propagate MRML properties to an existing text widget.
void vtkMRMLAnnotationBidimensionalDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
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
void vtkMRMLAnnotationBidimensionalDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // here we need the widget
  if (!widget)
    {
    return;
    }

  vtkBiDimensionalWidget * bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  // it has to be a vtkBiDimensionalWidget
  if (!bidimensionalWidget)
    {
    return;
    }

  // we also need the MRML node
  if (!node)
    {
    return;
    }

  vtkMRMLAnnotationBidimensionalNode * bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  // it has to be a vtkMRMLAnnotationBidimensionalNode
  if (!bidimensionalNode)
    {
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
  VTK_CREATE(vtkHandleWidget, h4);
  h4 = this->m_HandleWidgetList[3];


  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

  double* position3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();

  double* position4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetDisplayPosition();

  o << "EnterEvent 2 184 0 0 0 0 0\n";
  o << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o << "LeftButtonPressEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position4[0] << " " << position4[1] << " 0 0 0 0 t\n";
  o << "ExitEvent " << position4[0] << " " << position4[1] << " 0 0 113 1 q\n";

  recorder->SetInputString(o.str().c_str());
  recorder->Play();

  // widget thinks the interaction ended, now we can place the last two points
  vtkBiDimensionalRepresentation2D::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint3DisplayPosition(position3);

  vtkBiDimensionalRepresentation2D::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint4DisplayPosition(position4);

  // finally we add observer for end interaction, what a dirty hack!! if we added it before, it would all be crazy
  vtkAnnotationBidimensionalWidgetCallback *myCallback = vtkAnnotationBidimensionalWidgetCallback::New();
  myCallback->m_Node = bidimensionalNode;
  myCallback->m_Widget = bidimensionalWidget;
  bidimensionalWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  // we do not have to update the coordinates here, we already did during creation of the MRML node

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationBidimensionalDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(4))
    {

    vtkMRMLAnnotationBidimensionalNode *bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::New();

    vtkHandleWidget *h1 = this->m_HandleWidgetList[0];
    vtkHandleWidget *h2 = this->m_HandleWidgetList[1];
    vtkHandleWidget *h3 = this->m_HandleWidgetList[2];
    vtkHandleWidget *h4 = this->m_HandleWidgetList[3];

    double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetWorldPosition();
    double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetWorldPosition();
    double* position3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetWorldPosition();
    double* position4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetWorldPosition();

    // save the world coordinates of the points to MRML
    bidimensionalNode->SetControlPoint(position1,0);
    bidimensionalNode->SetControlPoint(position2,1);
    bidimensionalNode->SetControlPoint(position3,2);
    bidimensionalNode->SetControlPoint(position4,3);

    bidimensionalNode->Initialize(this->GetMRMLScene());

    bidimensionalNode->SetName(bidimensionalNode->GetScene()->GetUniqueNameByString("AnnotationBidimensional"));

    bidimensionalNode->Delete();

    }

  }
