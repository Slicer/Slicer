// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationBidimensionalDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// Annotation widget includes
#include "Widgets/vtkAnnotationBidimensionalWidget.h"
#include "Widgets/vtkAnnotationBidimensionalRepresentation.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkMath.h>
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
        vtkAnnotationBidimensionalRepresentation* representation = vtkAnnotationBidimensionalRepresentation::SafeDownCast(this->m_Widget->GetRepresentation());

        double displayCoordinates1[4];
        double displayCoordinates2[4];
        double displayCoordinates3[4];
        double displayCoordinates4[4];

        // first, we get the current displayCoordinates of the points
        representation->GetPoint1DisplayPosition(displayCoordinates1);
        representation->GetPoint2DisplayPosition(displayCoordinates2);
        representation->GetPoint3DisplayPosition(displayCoordinates3);
        representation->GetPoint4DisplayPosition(displayCoordinates4);


        // second, we copy these to restrictedDisplayCoordinates
        double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};
        double restrictedDisplayCoordinates2[4] = {displayCoordinates2[0], displayCoordinates2[1], displayCoordinates2[2], displayCoordinates2[3]};
        double restrictedDisplayCoordinates3[4] = {displayCoordinates3[0], displayCoordinates3[1], displayCoordinates3[2], displayCoordinates3[3]};
        double restrictedDisplayCoordinates4[4] = {displayCoordinates4[0], displayCoordinates4[1], displayCoordinates4[2], displayCoordinates4[3]};

        // modify restrictedDisplayCoordinates 1-4, if these are outside the viewport of the current renderer
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates2);
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates3);
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates4);

        // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
          {
          representation->SetPoint1DisplayPosition(restrictedDisplayCoordinates1);
          }

        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates2,restrictedDisplayCoordinates2))
          {
          representation->SetPoint2DisplayPosition(restrictedDisplayCoordinates2);
          }

        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates3,restrictedDisplayCoordinates3))
          {
          representation->SetPoint3DisplayPosition(restrictedDisplayCoordinates3);
          }

        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates4,restrictedDisplayCoordinates4))
          {
          representation->SetPoint4DisplayPosition(restrictedDisplayCoordinates4);
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
// vtkMRMLAnnotationBidimensionalDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new bidimensional widget.
vtkAbstractWidget * vtkMRMLAnnotationBidimensionalDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
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

  vtkAnnotationBidimensionalWidget * bidimensionalWidget = vtkAnnotationBidimensionalWidget::New();

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

  vtkAnnotationBidimensionalWidget * bidimensionalWidget = vtkAnnotationBidimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get bidimensional widget.")
    return;
    }

  vtkMRMLAnnotationBidimensionalNode * bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get bidimensional node")
    return;
    }

  VTK_CREATE(vtkInteractorEventRecorder, recorder);
  recorder->SetInteractor(this->GetInteractor());
  recorder->ReadFromInputStringOn();

  {
  std::ostringstream o;

  double position1[4];
  double position2[4];
  double position3[4];
  double position4[4];

  this->GetWorldToDisplayCoordinates(bidimensionalNode->GetControlPointCoordinates(0),position1);
  this->GetWorldToDisplayCoordinates(bidimensionalNode->GetControlPointCoordinates(1),position2);
  this->GetWorldToDisplayCoordinates(bidimensionalNode->GetControlPointCoordinates(2),position3);
  this->GetWorldToDisplayCoordinates(bidimensionalNode->GetControlPointCoordinates(3),position4);

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
  }

  // widget thinks the interaction ended, now we can place the points from MRML
  double worldCoordinates1[4];
  worldCoordinates1[0] = bidimensionalNode->GetControlPointCoordinates(0)[0];
  worldCoordinates1[1] = bidimensionalNode->GetControlPointCoordinates(0)[1];
  worldCoordinates1[2] = bidimensionalNode->GetControlPointCoordinates(0)[2];
  worldCoordinates1[3] = 1;

  double worldCoordinates2[4];
  worldCoordinates2[0] = bidimensionalNode->GetControlPointCoordinates(1)[0];
  worldCoordinates2[1] = bidimensionalNode->GetControlPointCoordinates(1)[1];
  worldCoordinates2[2] = bidimensionalNode->GetControlPointCoordinates(1)[2];
  worldCoordinates2[3] = 1;

  double worldCoordinates3[4];
  worldCoordinates3[0] = bidimensionalNode->GetControlPointCoordinates(2)[0];
  worldCoordinates3[1] = bidimensionalNode->GetControlPointCoordinates(2)[1];
  worldCoordinates3[2] = bidimensionalNode->GetControlPointCoordinates(2)[2];
  worldCoordinates3[3] = 1;

  double worldCoordinates4[4];
  worldCoordinates4[0] = bidimensionalNode->GetControlPointCoordinates(3)[0];
  worldCoordinates4[1] = bidimensionalNode->GetControlPointCoordinates(3)[1];
  worldCoordinates4[2] = bidimensionalNode->GetControlPointCoordinates(3)[2];
  worldCoordinates4[3] = 1;

  double displayCoordinates1[4];
  double displayCoordinates2[4];
  double displayCoordinates3[4];
  double displayCoordinates4[4];

  vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetDistance1(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));
  vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetDistance2(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4)));

  if (this->GetSliceNode())
    {

    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);
    this->GetWorldToDisplayCoordinates(worldCoordinates3,displayCoordinates3);
    this->GetWorldToDisplayCoordinates(worldCoordinates4,displayCoordinates4);

    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint1DisplayPosition(displayCoordinates1);
    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint2DisplayPosition(displayCoordinates2);
    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint3DisplayPosition(displayCoordinates3);
    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint4DisplayPosition(displayCoordinates4);

    }
  else
    {

    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint1WorldPosition(worldCoordinates1);
    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint2WorldPosition(worldCoordinates2);
    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint3WorldPosition(worldCoordinates3);
    vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation())->SetPoint4WorldPosition(worldCoordinates4);

    }


  // finally we add observer for end interaction, what a dirty hack!! if we added it before, it would all be crazy
  vtkAnnotationBidimensionalWidgetCallback *myCallback = vtkAnnotationBidimensionalWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  node->SaveView();

  // we want to manually propagate the widget to the MRML node now
  // in the future, the callback handles the update of the MRML node
  //this->m_Updating = 0;
  //this->PropagateWidgetToMRML(widget, node);

}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationBidimensionalDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAnnotationBidimensionalRepresentation * rep = vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation());

  double worldCoordinates1[4];
  worldCoordinates1[0] = bidimensionalNode->GetControlPointCoordinates(0)[0];
  worldCoordinates1[1] = bidimensionalNode->GetControlPointCoordinates(0)[1];
  worldCoordinates1[2] = bidimensionalNode->GetControlPointCoordinates(0)[2];
  worldCoordinates1[3] = 1;

  double worldCoordinates2[4];
  worldCoordinates2[0] = bidimensionalNode->GetControlPointCoordinates(1)[0];
  worldCoordinates2[1] = bidimensionalNode->GetControlPointCoordinates(1)[1];
  worldCoordinates2[2] = bidimensionalNode->GetControlPointCoordinates(1)[2];
  worldCoordinates2[3] = 1;

  double worldCoordinates3[4];
  worldCoordinates3[0] = bidimensionalNode->GetControlPointCoordinates(2)[0];
  worldCoordinates3[1] = bidimensionalNode->GetControlPointCoordinates(2)[1];
  worldCoordinates3[2] = bidimensionalNode->GetControlPointCoordinates(2)[2];
  worldCoordinates3[3] = 1;

  double worldCoordinates4[4];
  worldCoordinates4[0] = bidimensionalNode->GetControlPointCoordinates(3)[0];
  worldCoordinates4[1] = bidimensionalNode->GetControlPointCoordinates(3)[1];
  worldCoordinates4[2] = bidimensionalNode->GetControlPointCoordinates(3)[2];
  worldCoordinates4[3] = 1;

  double displayCoordinates1[4];
  double displayCoordinates2[4];
  double displayCoordinates3[4];
  double displayCoordinates4[4];
  double displayCoordinatesBuffer1[4];
  double displayCoordinatesBuffer2[4];
  double displayCoordinatesBuffer3[4];
  double displayCoordinatesBuffer4[4];

  // update the distance measurement
  rep->SetDistance1(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));
  rep->SetDistance2(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4)));

  // update the location
  if (this->GetSliceNode())
    {
    // change the 2D location
    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);
    this->GetWorldToDisplayCoordinates(worldCoordinates3,displayCoordinates3);
    this->GetWorldToDisplayCoordinates(worldCoordinates4,displayCoordinates4);


    // only update the position, if coordinates really change
    rep->GetPoint1DisplayPosition(displayCoordinatesBuffer1);
    rep->GetPoint2DisplayPosition(displayCoordinatesBuffer2);
    rep->GetPoint3DisplayPosition(displayCoordinatesBuffer3);
    rep->GetPoint4DisplayPosition(displayCoordinatesBuffer4);

    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rep->SetPoint1DisplayPosition(displayCoordinates1);
      }
    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rep->SetPoint2DisplayPosition(displayCoordinates2);
      }
    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rep->SetPoint3DisplayPosition(displayCoordinates3);
      }
    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rep->SetPoint4DisplayPosition(displayCoordinates4);
      }

    }
  else
    {
    // change the 3D location
    rep->SetPoint1WorldPosition(worldCoordinates1);
    rep->SetPoint2WorldPosition(worldCoordinates2);
    rep->SetPoint3WorldPosition(worldCoordinates3);
    rep->SetPoint4WorldPosition(worldCoordinates4);
    }

  rep->NeedToRenderOn();
  bidimensionalWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationBidimensionalDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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

  // disable processing of modified events
  this->m_Updating = 1;
  bidimensionalNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkAnnotationBidimensionalRepresentation * rep = vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation());

  double worldCoordinates1[4];
  double worldCoordinates2[4];
  double worldCoordinates3[4];
  double worldCoordinates4[4];

  if (this->GetSliceNode())
    {
    // 2D widget was changed

    double displayCoordinates1[4];
    double displayCoordinates2[4];
    double displayCoordinates3[4];
    double displayCoordinates4[4];
    rep->GetPoint1DisplayPosition(displayCoordinates1);
    rep->GetPoint2DisplayPosition(displayCoordinates2);
    rep->GetPoint3DisplayPosition(displayCoordinates3);
    rep->GetPoint4DisplayPosition(displayCoordinates4);

    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates2,worldCoordinates2);
    this->GetDisplayToWorldCoordinates(displayCoordinates3,worldCoordinates3);
    this->GetDisplayToWorldCoordinates(displayCoordinates4,worldCoordinates4);

    }
  else
    {
    rep->GetPoint1WorldPosition(worldCoordinates1);
    rep->GetPoint2WorldPosition(worldCoordinates2);
    rep->GetPoint3WorldPosition(worldCoordinates3);
    rep->GetPoint4WorldPosition(worldCoordinates4);
    }


  bidimensionalNode->SetControlPoint(worldCoordinates1,0);
  bidimensionalNode->SetControlPoint(worldCoordinates2,1);
  bidimensionalNode->SetControlPoint(worldCoordinates3,2);
  bidimensionalNode->SetControlPoint(worldCoordinates4,3);

  // save distance to MRML
  double distance1 = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
  double distance2 = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4));
  rep->SetDistance1(distance1);
  rep->SetDistance2(distance2);

  bidimensionalNode->SetBidimensionalMeasurement(distance1,distance2);

  bidimensionalNode->SaveView();

  // enable processing of modified events
  this->m_Updating = 0;
  bidimensionalNode->DisableModifiedEventOff();

  bidimensionalNode->Modified();
  bidimensionalNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, bidimensionalNode);


}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationBidimensionalDisplayableManager::OnClickInRenderWindow(double x, double y)
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

    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);
    vtkHandleWidget *h3 = this->GetSeed(2);
    vtkHandleWidget *h4 = this->GetSeed(3);

    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4];
    double worldCoordinates2[4];
    double worldCoordinates3[4];
    double worldCoordinates4[4];

    this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],worldCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],worldCoordinates2);
    this->GetDisplayToWorldCoordinates(displayCoordinates3[0],displayCoordinates3[1],worldCoordinates3);
    this->GetDisplayToWorldCoordinates(displayCoordinates4[0],displayCoordinates4[1],worldCoordinates4);

    double distance1 = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
    double distance2 = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4));

    vtkMRMLAnnotationBidimensionalNode *bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::New();

    // save the world coordinates of the points to MRML
    bidimensionalNode->SetControlPoint(worldCoordinates1,0);
    bidimensionalNode->SetControlPoint(worldCoordinates2,1);
    bidimensionalNode->SetControlPoint(worldCoordinates3,2);
    bidimensionalNode->SetControlPoint(worldCoordinates4,3);
    bidimensionalNode->SetBidimensionalMeasurement(distance1,distance2);

    bidimensionalNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationBidimensional"));

    bidimensionalNode->Initialize(this->GetMRMLScene());

    bidimensionalNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
