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
#include "Widgets/vtkAnnotationRulerRepresentation3D.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkAxisActor2D.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkAbstractWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointHandleRepresentation3D.h>

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
    vtkErrorMacro("CreateWidget: Could not get ruler node!")
    return 0;
    }

  vtkAnnotationRulerWidget * rulerWidget = vtkAnnotationRulerWidget::New();

  rulerWidget->SetInteractor(this->GetInteractor());
  rulerWidget->SetCurrentRenderer(this->GetRenderer());

  if (this->GetSliceNode())
    {

    // this is a 2D displayableManager
    VTK_CREATE(vtkPointHandleRepresentation2D, handle);
    handle->GetProperty()->SetColor(1,0,0);

    VTK_CREATE(vtkAnnotationRulerRepresentation, dRep);
    dRep->SetHandleRepresentation(handle);
    dRep->InstantiateHandleRepresentation();
    dRep->GetAxis()->SetNumberOfMinorTicks(4);
    dRep->GetAxis()->SetTickLength(5);
    dRep->GetAxis()->SetTitlePosition(0.2);
    dRep->RulerModeOn();
    dRep->SetRulerDistance(20);

    rulerWidget->SetRepresentation(dRep);

    }
  else
    {

    // this is a 3D displayableManager
    VTK_CREATE(vtkPointHandleRepresentation3D, handle2);
    handle2->GetProperty()->SetColor(1,1,0);

    VTK_CREATE(vtkAnnotationRulerRepresentation3D, dRep2);
    dRep2->SetHandleRepresentation(handle2);
    dRep2->InstantiateHandleRepresentation();
    dRep2->RulerModeOn();
    dRep2->SetRulerDistance(10);

    rulerWidget->SetRepresentation(dRep2);

    }

  rulerWidget->SetWidgetStateToManipulate();
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

  if (this->GetSliceNode())
    {

    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);

    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint1DisplayPosition(displayCoordinates1);
    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint2DisplayPosition(displayCoordinates2);

    // set a specific format for the measurement text
    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetLabelFormat("%.1f");

    vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));

    }
  else
    {

    vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint1WorldPosition(worldCoordinates1);
    vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint2WorldPosition(worldCoordinates2);

    // set a specific format for the measurement text
    vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation())->SetLabelFormat("%.1f");

    vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation())->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));

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

  // update the location
  if (this->GetSliceNode())
    {

    // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
    vtkAnnotationRulerRepresentation * rep = vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());

    // update the distance measurement
    rep->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));

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

    rep->NeedToRenderOn();
    }
  else
    {
    // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
    vtkAnnotationRulerRepresentation3D * rep = vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

    // update the distance measurement
    rep->SetDistance(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));


    // change the 3D location
    rep->SetPoint1WorldPosition(worldCoordinates1);
    rep->SetPoint2WorldPosition(worldCoordinates2);
    rep->NeedToRenderOn();
    }


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

  double worldCoordinates1[4];
  double worldCoordinates2[4];

  bool allowMovement = true;

  double distance;

  if (this->GetSliceNode())
    {
    // 2D widget was changed

    // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
    vtkAnnotationRulerRepresentation * rep = vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());


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

    // save distance to MRML
    distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
    rep->SetDistance(distance);

    }
  else
    {
    // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
    vtkAnnotationRulerRepresentation3D * rep = vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

    rep->GetPoint1WorldPosition(worldCoordinates1);
    rep->GetPoint2WorldPosition(worldCoordinates2);

    // get distance
    distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
    rep->SetDistance(distance);

    }

  // if movement is not allowed, jump out
  if (!allowMovement)
    {
    return;
    }

  // save worldCoordinates to MRML
  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);

  // save distance to mrml
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

    double worldCoordinates1[4] = {0,0,0,0};
    double worldCoordinates2[4] = {0,0,0,0};

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
