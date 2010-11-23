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
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>

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

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*)
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

      if (this->m_DisplayableManager->GetSliceNode())
        {

        // if this is a 2D SliceView displayableManager, restrict the widget to the renderer

        // we need the widgetRepresentation
        vtkAnnotationRulerRepresentation* representation = vtkAnnotationRulerRepresentation::SafeDownCast(this->m_Widget->GetRepresentation());

        double displayCoordinates1[4];
        double displayCoordinates2[4];

        // first, we get the current displayCoordinates of the points
        representation->GetPoint1DisplayPosition(displayCoordinates1);
        representation->GetPoint2DisplayPosition(displayCoordinates2);

        // second, we copy these to restrictedDisplayCoordinates
        double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};
        double restrictedDisplayCoordinates2[4] = {displayCoordinates2[0], displayCoordinates2[1], displayCoordinates2[2], displayCoordinates2[3]};

        // modify restrictedDisplayCoordinates 1 and 2, if these are outside the viewport of the current renderer
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates2);

        // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
          {
          representation->SetPoint1DisplayPosition(restrictedDisplayCoordinates1);
          }


        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates2,restrictedDisplayCoordinates2))
          {
          representation->SetPoint2DisplayPosition(restrictedDisplayCoordinates2);
          }

        }

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

  {
  std::stringstream o;

  double position1[4];
  double position2[4];

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
  }

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

  node->SaveView();

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
  double displayCoordinatesBuffer1[4];
  double displayCoordinatesBuffer2[4];

  // update the distance measurement
  rep->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));

  // update the location
  if (this->GetSliceNode())
    {
    // change the 2D location
    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);

    // only update the position, if coordinates really change
    rep->GetPoint1DisplayPosition(displayCoordinatesBuffer1);
    rep->GetPoint2DisplayPosition(displayCoordinatesBuffer2);

    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rep->SetPoint1DisplayPosition(displayCoordinates1);
      }
    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rep->SetPoint2DisplayPosition(displayCoordinates2);
      }

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

  bool allowMovement = true;

  if (this->GetSliceNode())
    {
    // 2D widget was changed

    double displayCoordinates1[4];
    double displayCoordinates2[4];
    rep->GetPoint1DisplayPosition(displayCoordinates1);
    rep->GetPoint2DisplayPosition(displayCoordinates2);

    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates2,worldCoordinates2);

    if (displayCoordinates1[0] < 0 || displayCoordinates1[0] > this->GetInteractor()->GetRenderWindow()->GetSize()[0])
      {
      allowMovement = false;
      }

    if (displayCoordinates1[1] < 0 || displayCoordinates1[1] > this->GetInteractor()->GetRenderWindow()->GetSize()[1])
      {
      allowMovement = false;
      }

    if (displayCoordinates2[0] < 0 || displayCoordinates2[0] > this->GetInteractor()->GetRenderWindow()->GetSize()[0])
      {
      allowMovement = false;
      }

    if (displayCoordinates2[1] < 0 || displayCoordinates2[1] > this->GetInteractor()->GetRenderWindow()->GetSize()[1])
      {
      allowMovement = false;
      }

    }
  else
    {
    rep->GetPoint1WorldPosition(worldCoordinates1);
    rep->GetPoint2WorldPosition(worldCoordinates2);
    }

  // if movement is not allowed, jump out
  if (!allowMovement)
    {
    return;
    }

  // save worldCoordinates to MRML
  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);

  // save distance to MRML
  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
  rep->SetDistance(distance);
  rulerNode->SetDistanceMeasurement(distance);

  // save the current view
  rulerNode->SaveView();

  // enable processing of modified events
  this->m_Updating = 0;
  rulerNode->DisableModifiedEventOff();

  rulerNode->Modified();
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
    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4];
    double worldCoordinates2[4];

    this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],worldCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],worldCoordinates2);

    double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

    rulerNode->SetPosition1(worldCoordinates1);
    rulerNode->SetPosition2(worldCoordinates2);
    rulerNode->SetDistanceMeasurement(distance);

    rulerNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationRuler"));

    rulerNode->Initialize(this->GetMRMLScene());

    rulerNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
