// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationRulerDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// Annotation widget includes
#include "Widgets/vtkAnnotationRulerWidget.h"
#include "Widgets/vtkAnnotationRulerRepresentation.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkAbstractWidget.h>

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

  vtkAnnotationRulerWidgetCallback(){}

  virtual void Execute (vtkObject *caller, unsigned long event, void*)
  {
    if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
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

  vtkAnnotationRulerWidget * rulerWidget = vtkAnnotationRulerWidget::New();

  rulerWidget->SetInteractor(this->GetInteractor());
  rulerWidget->SetCurrentRenderer(this->GetRenderer());

  rulerWidget->CreateDefaultRepresentation();

  rulerWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up")

  return rulerWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationRulerDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

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

  vtkAnnotationRulerWidget * rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get ruler widget")
    return;
    }

  vtkMRMLAnnotationRulerNode * rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get rulerNode node")
    return;
    }

  VTK_CREATE(vtkInteractorEventRecorder, recorder);
  recorder->SetInteractor(this->GetInteractor());
  recorder->ReadFromInputStringOn();

  std::ostringstream o;

  double position1[2];
  double position2[2];

  this->GetWorldToDisplayCoordinates(rulerNode->GetControlPointCoordinates(0),position1);
  this->GetWorldToDisplayCoordinates(rulerNode->GetControlPointCoordinates(1),position2);

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


  // widget thinks the interaction ended, now we can place the points from MRML
  double worldCoordinates1[4];
  worldCoordinates1[0] = rulerNode->GetControlPointCoordinates(0)[0];
  worldCoordinates1[1] = rulerNode->GetControlPointCoordinates(0)[1];
  worldCoordinates1[2] = rulerNode->GetControlPointCoordinates(0)[2];
  worldCoordinates1[3] = 1;

  double worldCoordinates2[4];
  worldCoordinates2[0] = rulerNode->GetControlPointCoordinates(1)[0];
  worldCoordinates2[1] = rulerNode->GetControlPointCoordinates(1)[1];
  worldCoordinates2[2] = rulerNode->GetControlPointCoordinates(1)[2];
  worldCoordinates2[3] = 1;

  double displayCoordinates1[4];
  double displayCoordinates2[4];

  //vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPositionsForDistanceCalculation(worldCoordinates1, worldCoordinates2);

  vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));

  if (this->GetSliceNode())
    {

    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);

    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint1DisplayPosition(displayCoordinates1);
    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint2DisplayPosition(displayCoordinates2);

    }
  else
    {

    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint1WorldPosition(worldCoordinates1);
    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint2WorldPosition(worldCoordinates2);

    }


  // add observer for end interaction
  vtkAnnotationRulerWidgetCallback *myCallback = vtkAnnotationRulerWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  //this->m_Updating = 0;
  //this->PropagateWidgetToMRML(widget, node);

}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationRulerDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkAnnotationRulerWidget* rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!rulerWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ruler widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ruler node!")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;


  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAnnotationRulerRepresentation * rep = vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());

  double worldCoordinates1[4];
  worldCoordinates1[0] = rulerNode->GetControlPointCoordinates(0)[0];
  worldCoordinates1[1] = rulerNode->GetControlPointCoordinates(0)[1];
  worldCoordinates1[2] = rulerNode->GetControlPointCoordinates(0)[2];
  worldCoordinates1[3] = 1;

  double worldCoordinates2[4];
  worldCoordinates2[0] = rulerNode->GetControlPointCoordinates(1)[0];
  worldCoordinates2[1] = rulerNode->GetControlPointCoordinates(1)[1];
  worldCoordinates2[2] = rulerNode->GetControlPointCoordinates(1)[2];
  worldCoordinates2[3] = 1;

  double displayCoordinates1[4];
  double displayCoordinates2[4];

  // update the distance measurement
  rep->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));

  // update the location
  if (this->GetSliceNode())
    {
    // change the 2D location
    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);

    rep->SetPoint1DisplayPosition(displayCoordinates1);
    rep->SetPoint2DisplayPosition(displayCoordinates2);

    }
  else
    {
    // change the 3D location
    rep->SetPoint1WorldPosition(worldCoordinates1);
    rep->SetPoint2WorldPosition(worldCoordinates2);
    }

  rep->NeedToRenderOn();
  rulerWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationRulerDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
  vtkAnnotationRulerWidget* rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!rulerWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ruler widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ruler node!")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;
  rulerNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkAnnotationRulerRepresentation * rep = vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());

  double worldCoordinates1[4];
  double worldCoordinates2[4];

  if (this->GetSliceNode())
    {
    // 2D widget was changed

    double displayCoordinates1[4];
    double displayCoordinates2[4];
    rep->GetPoint1DisplayPosition(displayCoordinates1);
    rep->GetPoint2DisplayPosition(displayCoordinates2);

    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates2,worldCoordinates2);

    }
  else
    {
    rep->GetPoint1WorldPosition(worldCoordinates1);
    rep->GetPoint2WorldPosition(worldCoordinates2);
    }

  // save worldCoordinates to MRML
  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);

  // save distance to MRML
  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
  rep->SetDistance(distance);
  rulerNode->SetDistanceMeasurement(distance);

  // enable processing of modified events
  this->m_Updating = 0;
  rulerNode->DisableModifiedEventOff();

  //rulerNode->Modified();

  rulerNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, rulerNode);

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationRulerDisplayableManager::OnClickInRenderWindow(double x, double y)
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

    // switch to updating state to avoid events mess
    this->m_Updating = 1;


    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);

    // convert the coordinates
    double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4];
    double worldCoordinates2[4];

    this->GetDisplayToWorldCoordinates(position1[0],position1[1],worldCoordinates1);
    this->GetDisplayToWorldCoordinates(position2[0],position2[1],worldCoordinates2);

    double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

    rulerNode->SetPosition1(worldCoordinates1);
    rulerNode->SetPosition2(worldCoordinates2);
    rulerNode->SetDistanceMeasurement(distance);

    rulerNode->Initialize(this->GetMRMLScene());

    rulerNode->SetName(rulerNode->GetScene()->GetUniqueNameByString("AnnotationRuler"));

    rulerNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
