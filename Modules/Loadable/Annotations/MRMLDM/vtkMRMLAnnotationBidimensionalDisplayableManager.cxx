
// Annotation Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// Annotation MRML includes
#include <vtkMRMLAnnotationBidimensionalNode.h>
#include <vtkMRMLAnnotationLineDisplayNode.h>
#include <vtkMRMLAnnotationPointDisplayNode.h>
#include <vtkMRMLAnnotationTextDisplayNode.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationClickCounter.h"
#include "vtkMRMLAnnotationDisplayableManagerHelper.h"
#include "vtkMRMLAnnotationBidimensionalDisplayableManager.h"

// Annotation VTKWidgets includes
#include <vtkAnnotationBidimensionalRepresentation.h>
#include <vtkAnnotationBidimensionalWidget.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkMath.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

// STD includes
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationBidimensionalDisplayableManager);

//---------------------------------------------------------------------------
// vtkMRMLAnnotationBidimensionalDisplayableManager Callback
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationBidimensionalWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationBidimensionalWidgetCallback *New()
  { return new vtkAnnotationBidimensionalWidgetCallback; }

  vtkAnnotationBidimensionalWidgetCallback() = default;

  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*) override
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
    vtkErrorMacro("CreateWidget: Node not set!");
    return nullptr;
    }

  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("CreateWidget: Could not get bidimensional node!");
    return nullptr;
    }

  vtkAnnotationBidimensionalWidget * bidimensionalWidget = vtkAnnotationBidimensionalWidget::New();

  bidimensionalWidget->SetInteractor(this->GetInteractor());
  bidimensionalWidget->SetCurrentRenderer(this->GetRenderer());

  bidimensionalWidget->CreateDefaultRepresentation();

  bidimensionalWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up");

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
    vtkErrorMacro("OnWidgetCreated: Could not get bidimensional widget.");
    return;
    }

  vtkMRMLAnnotationBidimensionalNode * bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get bidimensional node");
    return;
    }

  bidimensionalWidget->SetWidgetStateToManipulate();

  // widget thinks the interaction ended, now we can place the points from MRML
  double worldCoordinates1[4];
  bidimensionalNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double worldCoordinates2[4];
  bidimensionalNode->GetControlPointWorldCoordinates(1, worldCoordinates2);

  double worldCoordinates3[4];
  bidimensionalNode->GetControlPointWorldCoordinates(2, worldCoordinates3);

  double worldCoordinates4[4];
  bidimensionalNode->GetControlPointWorldCoordinates(3, worldCoordinates4);

  double displayCoordinates1[4];
  double displayCoordinates2[4];
  double displayCoordinates3[4];
  double displayCoordinates4[4];

  vtkAnnotationBidimensionalRepresentation * bidimensionalRepr =
      vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation());

  bidimensionalRepr->SetDistance1(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));
  bidimensionalRepr->SetDistance2(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4)));

  if (this->GetSliceNode())
    {

    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);
    this->GetWorldToDisplayCoordinates(worldCoordinates3,displayCoordinates3);
    this->GetWorldToDisplayCoordinates(worldCoordinates4,displayCoordinates4);

    bidimensionalRepr->SetPoint1DisplayPosition(displayCoordinates1);
    bidimensionalRepr->SetPoint2DisplayPosition(displayCoordinates2);
    bidimensionalRepr->SetPoint3DisplayPosition(displayCoordinates3);
    bidimensionalRepr->SetPoint4DisplayPosition(displayCoordinates4);

    }
  else
    {

    bidimensionalRepr->SetPoint1WorldPosition(worldCoordinates1);
    bidimensionalRepr->SetPoint2WorldPosition(worldCoordinates2);
    bidimensionalRepr->SetPoint3WorldPosition(worldCoordinates3);
    bidimensionalRepr->SetPoint4WorldPosition(worldCoordinates4);

    }

  // set a specific format for the measurement text
  bidimensionalRepr->SetLabelFormat("%.1f");

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
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkBiDimensionalWidget* bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get bidimensional widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get bidimensional node!");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkAnnotationBidimensionalRepresentation * rep = vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation());

  double worldCoordinates1[4];
  bidimensionalNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double worldCoordinates2[4];
  bidimensionalNode->GetControlPointWorldCoordinates(1, worldCoordinates2);

  double worldCoordinates3[4];
  bidimensionalNode->GetControlPointWorldCoordinates(2, worldCoordinates3);

  double worldCoordinates4[4];
  bidimensionalNode->GetControlPointWorldCoordinates(3, worldCoordinates4);



  // update the distance measurement
  rep->SetDistance1(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2)));
  rep->SetDistance2(sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4)));

  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = bidimensionalNode->GetAnnotationTextDisplayNode();
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = bidimensionalNode->GetAnnotationPointDisplayNode();
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = bidimensionalNode->GetAnnotationLineDisplayNode();

  if (textDisplayNode)
    {
    vtkTextProperty *textProperty = rep->GetTextProperty();
    if (textProperty)
      {
      if (bidimensionalNode->GetSelected())
        {
        textProperty->SetColor(textDisplayNode->GetSelectedColor());
        }
      else
        {
        textProperty->SetColor(textDisplayNode->GetColor());
        }
      float textScale = textDisplayNode->GetTextScale();
      textProperty->SetFontSize(textScale);
      }
    }
  if (pointDisplayNode)
    {
    vtkPointHandleRepresentation2D *handle =  vtkPointHandleRepresentation2D::SafeDownCast(rep->GetPoint1Representation());
    if (handle)
      {
      if (bidimensionalNode->GetSelected())
        {
        handle->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
        }
      else
        {
        handle->GetProperty()->SetColor(pointDisplayNode->GetColor());
        }
      }
    handle =  vtkPointHandleRepresentation2D::SafeDownCast(rep->GetPoint2Representation());
    if (bidimensionalNode->GetSelected())
      {
      handle->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
      }
    else
      {
      handle->GetProperty()->SetColor(pointDisplayNode->GetColor());
      }
    handle =  vtkPointHandleRepresentation2D::SafeDownCast(rep->GetPoint3Representation());
    if (bidimensionalNode->GetSelected())
      {
      handle->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
      }
    else
      {
      handle->GetProperty()->SetColor(pointDisplayNode->GetColor());
      }
    handle =  vtkPointHandleRepresentation2D::SafeDownCast(rep->GetPoint4Representation());
    if (bidimensionalNode->GetSelected())
      {
      handle->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
      }
    else
      {
      handle->GetProperty()->SetColor(pointDisplayNode->GetColor());
      }
    }
  if (lineDisplayNode)
    {
    vtkProperty2D *lineProp = rep->GetLineProperty();
    if (lineProp)
      {
      if (bidimensionalNode->GetSelected())
        {
        lineProp->SetColor(lineDisplayNode->GetSelectedColor());
        }
      else
        {
        lineProp->SetColor(lineDisplayNode->GetColor());
        }
      }
    vtkProperty2D *selLineProp = rep->GetSelectedLineProperty();
    if (selLineProp)
      {
      if (bidimensionalNode->GetSelected())
        {
        selLineProp->SetColor(lineDisplayNode->GetSelectedColor());
        }
      else
        {
        selLineProp->SetColor(lineDisplayNode->GetColor());
        }
      }
    }

  this->UpdatePosition(widget, node);

  rep->NeedToRenderOn();
  bidimensionalWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate position properties of MRML node to widget.
void vtkMRMLAnnotationBidimensionalDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
{
    if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkBiDimensionalWidget* bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get bidimensional widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get bidimensional node!");
    return;
    }

  vtkAnnotationBidimensionalRepresentation * rep = vtkAnnotationBidimensionalRepresentation::SafeDownCast(bidimensionalWidget->GetRepresentation());

  double worldCoordinates1[4];
  bidimensionalNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double worldCoordinates2[4];
  bidimensionalNode->GetControlPointWorldCoordinates(1, worldCoordinates2);

  double worldCoordinates3[4];
  bidimensionalNode->GetControlPointWorldCoordinates(2, worldCoordinates3);

  double worldCoordinates4[4];
  bidimensionalNode->GetControlPointWorldCoordinates(3, worldCoordinates4);

  double displayCoordinates1[4];
  double displayCoordinates2[4];
  double displayCoordinates3[4];
  double displayCoordinates4[4];
  double displayCoordinatesBuffer1[4];
  double displayCoordinatesBuffer2[4];
  double displayCoordinatesBuffer3[4];
  double displayCoordinatesBuffer4[4];

  // update the location
  if (this->Is2DDisplayableManager())
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
}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationBidimensionalDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkBiDimensionalWidget* bidimensionalWidget = vtkBiDimensionalWidget::SafeDownCast(widget);

  if (!bidimensionalWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get bidimensional widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationBidimensionalNode* bidimensionalNode = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node);

  if (!bidimensionalNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get bidimensional node!");
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


  bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates1,0);
  bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates2,1);
  bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates3,2);
  bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates4,3);

  // save distance to MRML
  double distance1 = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
  double distance2 = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates3,worldCoordinates4));
  rep->SetDistance1(distance1);
  rep->SetDistance2(distance2);

  bidimensionalNode->SetBidimensionalMeasurement(distance1,distance2);

  bidimensionalNode->SaveView();

  // enable processing of modified events
  bidimensionalNode->DisableModifiedEventOff();

  bidimensionalNode->Modified();
  bidimensionalNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, bidimensionalNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationBidimensionalDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  if (!this->GetSliceNode())
    {
    // we only support bidimensional measurement creation in 2D viewers
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(3))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);
    vtkHandleWidget *h3 = this->GetSeed(2);
    //vtkHandleWidget *h4 = this->GetSeed(3);

    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates3 = vtkHandleRepresentation::SafeDownCast(h3->GetRepresentation())->GetDisplayPosition();
    //double* displayCoordinates4 = vtkHandleRepresentation::SafeDownCast(h4->GetRepresentation())->GetDisplayPosition();

    //
    // calculate fourth point
    //

    // find equation for line between displayCoordinate1 and displayCoordinate2
    double x1 = displayCoordinates1[0];
    double y1 = displayCoordinates1[1];
    //std::cout << "X1: " << x1 << " Y1: " << y1 << std::endl;
    double x2 = displayCoordinates2[0];
    double y2 = displayCoordinates2[1];
    //std::cout << "X2: " << x2 << " Y2: " << y2 << std::endl;
    double x3 = displayCoordinates3[0];
    double y3 = displayCoordinates3[1];
    //std::cout << "X3: " << x3 << " Y3: " << y3 << std::endl;

    // special cases: x2==1 || x1==0
    if (x2==1)
      {
      x2=1.001;
      }
    else if (x1==0)
      {
      x1=0.001;
      }

    //error double b1 = (y2*x1 - y1)/(-1+x1);
    //error2 double b1 = (x2*y1 - y2*x1)/(x2-1);
    double b1 = (y2*x1 - y1*x2)/(x1-x2);
    //std::cout << "B1: " << b1 << std::endl;
    //double m1 = (y1/x1)-((y2*x1-y1)/(x1-x1*x1));
    double m1 = (y1-b1)/x1;
    //std::cout << "M1: " << m1 << std::endl;

    // avoid unlikely case of m1==0
    if (m1==0)
      {
      m1 = 0.00001;
      }

    double m2 = -1*(1/m1);
    //std::cout << "M2: " << m2 << std::endl;

    // avoid unlikely case of m1=m2
    if (m1==m2)
      {
      m2=m2+0.001;
      }

    // get the crosssection (xS,yS) between orthogonal lines
    double xS = (y3-m2*x3-b1)/(m1-m2);
    double yS = m1*xS+b1;
    //std::cout << "xS: " << xS << std::endl;
    //std::cout << "yS: " << yS << std::endl;

    // now we can get the fourth coordinate (TAAAAAADAAAAAAA)
    double displayCoordinates4[2];
    displayCoordinates4[0] = 2*xS-x3;
    displayCoordinates4[1] = 2*yS-y3;
    //std::cout << "X4: " << displayCoordinates4[0] << std::endl;
    //std::cout << "Y4: " << displayCoordinates4[1] << std::endl;

    //
    // end of calculation of fourth point
    //




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
    // but check the order first
    if (y1 > y2)
      {
      // top to bottom
      bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates1,0);
      bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates2,1);

      if (x3 > x2 || x3 > x1)
        {
        // right side
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates3,2);
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates4,3);
        }
      else
        {
        // left side
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates3,3);
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates4,2);
        }

      }
    else
      {
      // bottom to top
      bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates2,0);
      bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates1,1);

      if (x3 > x2 || x3 > x1)
        {
        // right side
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates3,3);
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates4,2);
        }
      else
        {
        // left side
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates3,2);
        bidimensionalNode->SetControlPointWorldCoordinates(worldCoordinates4,3);
        }

      }


    bidimensionalNode->SetBidimensionalMeasurement(distance1,distance2);

    bidimensionalNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("B"));

    // if this was a one time place, go back to view transform mode
    vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }

    this->GetMRMLScene()->SaveStateForUndo();

    // is there a node associated with this?
    if (associatedNodeID)
      {
      bidimensionalNode->SetAttribute("AssociatedNodeID", associatedNodeID);
      }

    bidimensionalNode->Initialize(this->GetMRMLScene());

    bidimensionalNode->Delete();

    // reset updating state
    this->m_Updating = 0;
    }

  }
