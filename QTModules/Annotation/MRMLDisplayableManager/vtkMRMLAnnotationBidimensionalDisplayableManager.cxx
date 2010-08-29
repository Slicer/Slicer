// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationBidimensionalDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

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
vtkStandardNewMacro (vtkMRMLAnnotationBidimensionalDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationBidimensionalDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationBidimensionalDisplayableManager Callback
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
  void SetDisplayableManager(vtkMRMLAnnotationDisplayableManager * dm)
  {
    this->m_DisplayableManager = dm;
  }

  vtkAbstractWidget * m_Widget;
  vtkMRMLAnnotationNode * m_Node;
  vtkMRMLAnnotationDisplayableManager * m_DisplayableManager;
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
  myCallback->SetNode(bidimensionalNode);
  myCallback->SetWidget(bidimensionalWidget);
  myCallback->SetDisplayableManager(this);
  bidimensionalWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  // we want to manually propagate the widget to the MRML node now
  // in the future, the callback handles the update of the MRML node
  this->PropagateWidgetToMRML(bidimensionalWidget, bidimensionalNode);

}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationBidimensionalDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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

  // if this flag is true after the checks below, the widget will be set to modified
  bool hasChanged = false;

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

  // Check if the MRML node has position set at all
   if (!bidimensionalNode->GetControlPointCoordinates(0))
     {
     bidimensionalNode->SetControlPoint(position1,0);
     hasChanged = true;
     }

   if (!bidimensionalNode->GetControlPointCoordinates(1))
     {
     bidimensionalNode->SetControlPoint(position2,1);
     hasChanged = true;
     }

   if (!bidimensionalNode->GetControlPointCoordinates(2))
     {
     bidimensionalNode->SetControlPoint(position3,2);
     hasChanged = true;
     }

   if (!bidimensionalNode->GetControlPointCoordinates(3))
     {
     bidimensionalNode->SetControlPoint(position4,3);
     hasChanged = true;
     }

   if (!bidimensionalNode->GetBidimensionalMeasurement().size()!=2)
     {
     bidimensionalNode->SetBidimensionalMeasurement(rep->GetLength1(), rep->GetLength2());
     hasChanged = true;
     }

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to widget
  //
  if (bidimensionalNode->GetControlPointCoordinates(0)[0] != position1[0] || bidimensionalNode->GetControlPointCoordinates(0)[1] != position1[1] || bidimensionalNode->GetControlPointCoordinates(0)[2] != position1[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetPoint1WorldPosition(bidimensionalNode->GetControlPointCoordinates(0));
    hasChanged = true;
    }

  if (bidimensionalNode->GetControlPointCoordinates(1)[0] != position2[0] || bidimensionalNode->GetControlPointCoordinates(1)[1] != position2[1] || bidimensionalNode->GetControlPointCoordinates(1)[2] != position2[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetPoint2WorldPosition(bidimensionalNode->GetControlPointCoordinates(1));
    hasChanged = true;
    }

  if (bidimensionalNode->GetControlPointCoordinates(2)[0] != position3[0] || bidimensionalNode->GetControlPointCoordinates(2)[1] != position3[1] || bidimensionalNode->GetControlPointCoordinates(2)[2] != position3[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetPoint3WorldPosition(bidimensionalNode->GetControlPointCoordinates(2));
    hasChanged = true;
    }

  if (bidimensionalNode->GetControlPointCoordinates(3)[0] != position4[0] || bidimensionalNode->GetControlPointCoordinates(3)[1] != position4[1] || bidimensionalNode->GetControlPointCoordinates(3)[2] != position4[2])
    {
    // at least one coordinate has changed, so update the widget
    rep->SetPoint4WorldPosition(bidimensionalNode->GetControlPointCoordinates(3));
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so set the widget to modified
    rep->NeedToRenderOn();
    bidimensionalWidget->Modified();
    }

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationBidimensionalDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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

  // if this flag is true after the checks below, the modified event gets fired
  bool hasChanged = false;

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

  // Check if the MRML node has position set at all
  if (!bidimensionalNode->GetControlPointCoordinates(0))
    {
    bidimensionalNode->SetControlPoint(position1,0);
    hasChanged = true;
    }

  if (!bidimensionalNode->GetControlPointCoordinates(1))
    {
    bidimensionalNode->SetControlPoint(position2,1);
    hasChanged = true;
    }

  if (!bidimensionalNode->GetControlPointCoordinates(2))
    {
    bidimensionalNode->SetControlPoint(position3,2);
    hasChanged = true;
    }

  if (!bidimensionalNode->GetControlPointCoordinates(3))
    {
    bidimensionalNode->SetControlPoint(position4,3);
    hasChanged = true;
    }

  if (!bidimensionalNode->GetBidimensionalMeasurement().size()!=2)
    {
    bidimensionalNode->SetBidimensionalMeasurement(rep->GetLength1(), rep->GetLength2());
    hasChanged = true;
    }


  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the mrml node
  //
  if (bidimensionalNode->GetControlPointCoordinates(0)[0] != position1[0] || bidimensionalNode->GetControlPointCoordinates(0)[1] != position1[1] || bidimensionalNode->GetControlPointCoordinates(0)[2] != position1[2])
    {
    // at least one coordinate has changed, so update the mrml property
    bidimensionalNode->SetControlPoint(position1,0);
    hasChanged = true;
    }

  if (bidimensionalNode->GetControlPointCoordinates(1)[0] != position2[0] || bidimensionalNode->GetControlPointCoordinates(1)[1] != position2[1] || bidimensionalNode->GetControlPointCoordinates(1)[2] != position2[2])
    {
    // at least one coordinate has changed, so update the mrml property
    bidimensionalNode->SetControlPoint(position2,1);
    hasChanged = true;
    }

  if (bidimensionalNode->GetControlPointCoordinates(2)[0] != position3[0] || bidimensionalNode->GetControlPointCoordinates(2)[1] != position3[1] || bidimensionalNode->GetControlPointCoordinates(2)[2] != position3[2])
    {
    // at least one coordinate has changed, so update the mrml property
    bidimensionalNode->SetControlPoint(position3,2);
    hasChanged = true;
    }

  if (bidimensionalNode->GetControlPointCoordinates(3)[0] != position4[0] || bidimensionalNode->GetControlPointCoordinates(3)[1] != position4[1] || bidimensionalNode->GetControlPointCoordinates(3)[2] != position4[2])
    {
    // at least one coordinate has changed, so update the mrml property
    bidimensionalNode->SetControlPoint(position4,3);
    hasChanged = true;
    }

  //
  // Check if the measurement value of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the mrml node
  if (bidimensionalNode->GetBidimensionalMeasurement()[0] != rep->GetLength1() || bidimensionalNode->GetBidimensionalMeasurement()[1] != rep->GetLength2())
    {
    // the measurement has changes, so update the mrml property
    bidimensionalNode->SetBidimensionalMeasurement(rep->GetLength1(), rep->GetLength2());
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so fire the modified event
    bidimensionalNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, bidimensionalNode);
    }

  // enable processing of modified events
  this->m_Updating = 0;

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

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

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

    // reset updating state
    this->m_Updating = 0;

    }

  }
