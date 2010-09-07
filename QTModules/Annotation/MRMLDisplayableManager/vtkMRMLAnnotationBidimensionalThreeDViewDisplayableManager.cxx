// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationThreeDViewDisplayableManager.h"

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
#include <vtkAbstractWidget.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager Callback
class vtkAnnotationBidimensionalWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationBidimensionalWidgetCallback *New()
  { return new vtkAnnotationBidimensionalWidgetCallback; }

  vtkAnnotationBidimensionalWidgetCallback(){}

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
  void SetDisplayableManager(vtkMRMLAnnotationThreeDViewDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkMRMLAnnotationNode * m_Node;
  vtkMRMLAnnotationThreeDViewDisplayableManager * m_DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

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
/// Tear down the widget creation
void vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  // here we need the widget
  if (!widget)
    {
    return;
    }

  // we also need the MRML node
  if (!node)
    {
    return;
    }

  vtkBiDimensionalWidget * bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get bidimensional widget.")
    return;
    }

  VTK_CREATE(vtkInteractorEventRecorder, recorder);
  recorder->SetInteractor(this->GetInteractor());
  recorder->ReadFromInputStringOn();

  std::ostringstream o;

  vtkHandleWidget * h1 = this->GetSeed(0);
  vtkHandleWidget * h2 = this->GetSeed(1);
  vtkHandleWidget * h3 = this->GetSeed(2);
  vtkHandleWidget * h4 = this->GetSeed(3);


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
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  // we want to manually propagate the widget to the MRML node now
  // in the future, the callback handles the update of the MRML node
  this->m_Updating = 0;
  this->PropagateWidgetToMRML(widget, node);

}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{

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
  vtkBiDimensionalWidget* bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get bidimensional widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get bidimensional node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkBiDimensionalRepresentation2D * rep = vtkBiDimensionalRepresentation2D::SafeDownCast(bidimensionalWidget->GetRepresentation());

  double position1[3];
  double position2[3];
  double position3[3];
  double position4[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);
  rep->GetPoint3WorldPosition(position3);
  rep->GetPoint4WorldPosition(position4);

  // at least one coordinate has changed, so update the widget
  rep->SetPoint1WorldPosition(bidimensionalNode->GetControlPointCoordinates(0));

  rep->SetPoint2WorldPosition(bidimensionalNode->GetControlPointCoordinates(1));

  rep->SetPoint3WorldPosition(bidimensionalNode->GetControlPointCoordinates(2));

  rep->SetPoint4WorldPosition(bidimensionalNode->GetControlPointCoordinates(3));

  rep->NeedToRenderOn();
  bidimensionalWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{

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
  vtkBiDimensionalWidget* bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get bidimensional widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get bidimensional node!")
    return;
    }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkBiDimensionalRepresentation2D * rep = vtkBiDimensionalRepresentation2D::SafeDownCast(bidimensionalWidget->GetRepresentation());

  double position1[3];
  double position2[3];
  double position3[3];
  double position4[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);
  rep->GetPoint3WorldPosition(position3);
  rep->GetPoint4WorldPosition(position4);

  bidimensionalNode->SetControlPoint(position1,0);

  bidimensionalNode->SetControlPoint(position2,1);

  bidimensionalNode->SetControlPoint(position3,2);

  bidimensionalNode->SetControlPoint(position4,3);

  bidimensionalNode->SetBidimensionalMeasurement(rep->GetLength1(), rep->GetLength2());

  bidimensionalNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, bidimensionalNode);

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    vtkMRMLAnnotationBidimensionalNode *bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::New();

    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);
    vtkHandleWidget *h3 = this->GetSeed(2);
    vtkHandleWidget *h4 = this->GetSeed(3);

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

    // reset updating state
    this->m_Updating = 0;

    }

  }
