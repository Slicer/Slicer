
// Annotation Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// Annotation MRML includes
#include <vtkMRMLAnnotationLineDisplayNode.h>
#include <vtkMRMLAnnotationPointDisplayNode.h>
#include <vtkMRMLAnnotationRulerNode.h>
#include <vtkMRMLAnnotationTextDisplayNode.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationClickCounter.h"
#include "vtkMRMLAnnotationDisplayableManagerHelper.h"
#include "vtkMRMLAnnotationRulerDisplayableManager.h"

// Annotation VTKWidgets includes
#include <vtkAnnotationRulerRepresentation.h>
#include <vtkAnnotationRulerRepresentation3D.h>
#include <vtkAnnotationRulerWidget.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkAxisActor2D.h>
#include <vtkCubeSource.h>
#include <vtkFollower.h>
#include <vtkGlyph3D.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkLineRepresentation.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkPickingManager.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

// std includes
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationRulerDisplayableManager);

//---------------------------------------------------------------------------
// vtkMRMLAnnotationRulerDisplayableManager Callback
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationRulerWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationRulerWidgetCallback *New()
  { return new vtkAnnotationRulerWidgetCallback; }

  vtkAnnotationRulerWidgetCallback() = default;

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
      if (event == vtkCommand::EndInteractionEvent)
        {
        // save the state of the node when done moving, then call
        // PropagateWidgetToMRML to update the node one last time
        if (this->m_Node->GetScene())
          {
          this->m_Node->GetScene()->SaveStateForUndo();
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
    vtkErrorMacro("CreateWidget: Node not set!");
    return nullptr;
    }

  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);
  vtkMRMLAnnotationLineDisplayNode* lineDisplayNode =
    rulerNode ? rulerNode->GetAnnotationLineDisplayNode() : nullptr;

  if (!rulerNode)
    {
    vtkErrorMacro("CreateWidget: Could not get ruler node!");
    return nullptr;
    }

  vtkAnnotationRulerWidget * rulerWidget = vtkAnnotationRulerWidget::New();

  if (this->GetInteractor()->GetPickingManager())
    {
    if (!(this->GetInteractor()->GetPickingManager()->GetEnabled()))
      {
      // if the picking manager is not already turned on for this
      // interactor, enable it
      this->GetInteractor()->GetPickingManager()->EnabledOn();
      }
    }

  rulerWidget->SetInteractor(this->GetInteractor());
  rulerWidget->SetCurrentRenderer(this->GetRenderer());
  rulerWidget->SetIs2DWidget(this->Is2DDisplayableManager());

  if (this->Is2DDisplayableManager())
    {

    // this is a 2D displayableManager
    vtkNew<vtkPointHandleRepresentation2D> handle;
//    handle->GetProperty()->SetColor(1,0,0);

    vtkAnnotationRulerRepresentation * dRep =
        vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());
    dRep->SetHandleRepresentation(handle.GetPointer());
    dRep->InstantiateHandleRepresentation();

    vtkAxisActor2D *axis = dRep->GetAxis();
    axis->SetNumberOfMinorTicks(0);
    axis->SetTickLength(6);
    axis->SetTitlePosition(0.2);

    dRep->RulerModeOn();
    dRep->SetRulerDistance(
      this->ApplyUnit(lineDisplayNode ? lineDisplayNode->GetTickSpacing() : 10.));

    bool showWidget = true;
    showWidget = this->IsWidgetDisplayable(this->GetSliceNode(), node);

    rulerWidget->SetWidgetStateToManipulate();

    if (showWidget)
      {
      rulerWidget->On();
      }
    }
  else
    {

    // this is a 3D displayableManager
    vtkNew<vtkPointHandleRepresentation3D> handle2;
//    handle2->GetProperty()->SetColor(1,1,0);

    vtkAnnotationRulerRepresentation3D * dRep2 =
        vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

    dRep2->SetHandleRepresentation(handle2.GetPointer());
    dRep2->InstantiateHandleRepresentation();
    dRep2->RulerModeOn();
    dRep2->SetRulerDistance(
      this->ApplyUnit(lineDisplayNode ? lineDisplayNode->GetTickSpacing() : 10.));

    // change ticks to a stretched cube
    vtkNew<vtkCubeSource> cubeSource;
    cubeSource->SetXLength(1.0);
    cubeSource->SetYLength(0.1);
    cubeSource->SetZLength(1.0);
    cubeSource->Update();
    //dRep2->UpdateGlyphPolyData(cubeSource->GetOutput());

    rulerWidget->SetWidgetStateToManipulate();
    rulerWidget->On();
    }

  this->PropagateMRMLToWidget(rulerNode, rulerWidget);

  vtkDebugMacro("CreateWidget: Widget was set up");

  return rulerWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationRulerDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!");
    return;
    }

  vtkAnnotationRulerWidget * rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get ruler widget");
    return;
    }

  vtkMRMLAnnotationRulerNode * rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("OnWidgetCreated: Could not get rulerNode node");
    return;
    }

  // widget thinks the interaction ended, now we can place the points from MRML
  double worldCoordinates1[4]={0,0,0,1};
  rulerNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double worldCoordinates2[4]={0,0,0,1};
  rulerNode->GetControlPointWorldCoordinates(1, worldCoordinates2);

  //vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation())->SetPositionsForDistanceCalculation(worldCoordinates1, worldCoordinates2);

  std::string format = this->GetLabelFormat(rulerNode);
  double distance = this->ApplyUnit(this->GetDistance(worldCoordinates1, worldCoordinates2));

  if (this->GetSliceNode())
    {

    double displayCoordinates1[4]={0,0,0,1};
    double displayCoordinates2[4]={0,0,0,1};

    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);

    vtkAnnotationRulerRepresentation * rulerRepresentation =
        vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());

    rulerRepresentation->SetPoint1DisplayPosition(displayCoordinates1);
    rulerRepresentation->SetPoint2DisplayPosition(displayCoordinates2);

    // set a specific format for the measurement text
    rulerRepresentation->SetLabelFormat(format.c_str());

    rulerRepresentation->SetDistance(distance);

    }
  else
    {

    vtkAnnotationRulerRepresentation3D * rulerRepresentation3D =
        vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

    rulerRepresentation3D->SetPoint1WorldPosition(worldCoordinates1);
    rulerRepresentation3D->SetPoint2WorldPosition(worldCoordinates2);

    // set a specific format for the measurement text
    rulerRepresentation3D->SetLabelFormat(format.c_str());

    rulerRepresentation3D->SetDistance(distance);

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
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationRulerDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkAnnotationRulerWidget* rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!rulerWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ruler widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ruler node!");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;


  // get the points, to calculate the distance between them
  double worldCoordinates1[4]={0,0,0,1};
  rulerNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double worldCoordinates2[4]={0,0,0,1};
  rulerNode->GetControlPointWorldCoordinates(1, worldCoordinates2);

  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = rulerNode->GetAnnotationTextDisplayNode();
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = rulerNode->GetAnnotationPointDisplayNode();
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = rulerNode->GetAnnotationLineDisplayNode();

  double distance = this->ApplyUnit(this->GetDistance(worldCoordinates1,worldCoordinates2));

  // update the location
  if (this->Is2DDisplayableManager())
    {
    // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
    vtkAnnotationRulerRepresentation * rep = vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());

    if (textDisplayNode)
      {
      // TODO: get the font size/scale working
      vtkTextProperty *textProp = rep->GetAxis()->GetTitleTextProperty();
      textProp->SetFontSize(textDisplayNode->GetTextScale());
      // opacity
      textProp->SetOpacity(textDisplayNode->GetOpacity());
      // to match with the 3d ruler that doesn't have labels and turns off the measurement
      // with the label visibility is off, toggle the title visibility on the label visibility
      // for the 2d case (also: no text visibility is exposed in the current GUI)
      rep->GetAxis()->SetTitleVisibility(textDisplayNode->GetVisibility() && textDisplayNode->GetVisibility2D());
      }
    // update the distance measurement
    rep->SetDistance(distance);

    // set the color
    vtkHandleRepresentation *pointrep1 = rep->GetPoint1Representation();
    vtkHandleRepresentation *pointrep2 = rep->GetPoint2Representation();
    vtkPointHandleRepresentation2D *handle1 = nullptr;
    vtkPointHandleRepresentation2D *handle2 = nullptr;
    if (pointrep1 && pointrep2)
      {
      handle1 = vtkPointHandleRepresentation2D::SafeDownCast(pointrep1);
      handle2 = vtkPointHandleRepresentation2D::SafeDownCast(pointrep2);
      }
    if (handle1 && handle2 && pointDisplayNode)
      {
      if (rulerNode->GetSelected())
        {
        handle1->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
        handle2->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
        }
      else
        {
        handle1->GetProperty()->SetColor(pointDisplayNode->GetColor());
        handle2->GetProperty()->SetColor(pointDisplayNode->GetColor());
        }
      // opacity
      handle1->GetProperty()->SetOpacity(pointDisplayNode->GetOpacity());
      handle2->GetProperty()->SetOpacity(pointDisplayNode->GetOpacity());
      // scale
      handle1->SetHandleSize(pointDisplayNode->GetGlyphScale());
      handle2->SetHandleSize(pointDisplayNode->GetGlyphScale());

      }
    if (textDisplayNode)
      {
      if (rulerNode->GetSelected())
        {
        rep->GetAxis()->GetTitleTextProperty()->SetColor(textDisplayNode->GetSelectedColor());
        rep->GetAxis()->GetLabelTextProperty()->SetColor(textDisplayNode->GetSelectedColor());
        }
      else
        {
        rep->GetAxis()->GetTitleTextProperty()->SetColor(textDisplayNode->GetColor());
        rep->GetAxis()->GetLabelTextProperty()->SetColor(textDisplayNode->GetColor());
        }
      rep->GetAxis()->SetTitleVisibility(textDisplayNode->GetVisibility() && textDisplayNode->GetVisibility2D());
      // TODO: get this working
      rep->GetAxis()->GetTitleTextProperty()->SetFontSize(textDisplayNode->GetTextScale());
      }
    if (lineDisplayNode)
      {
      if (rep && rep->GetLineProperty())
        {
        if (rulerNode->GetSelected())
          {
          rep->GetLineProperty()->SetColor(lineDisplayNode->GetSelectedColor());
          }
        else
          {
          rep->GetLineProperty()->SetColor(lineDisplayNode->GetColor());
          }
        rep->GetLineProperty()->SetLineWidth(lineDisplayNode->GetLineThickness());
        rep->GetLineProperty()->SetOpacity(lineDisplayNode->GetOpacity());
       }
      rep->GetAxis()->SetTitlePosition(lineDisplayNode->GetLabelPosition());
      if (textDisplayNode && rep->GetAxis()->GetLabelTextProperty())
        {
        rep->GetAxis()->GetLabelTextProperty()->SetOpacity(textDisplayNode->GetOpacity());
        }
      if (textDisplayNode)
        {
        if (textDisplayNode->GetVisibility() == 0 || textDisplayNode->GetVisibility2D() == 0)
          {
          // override label visibility if all text is off
          rep->GetAxis()->SetLabelVisibility(0);
          }
        else
          {
          rep->GetAxis()->SetLabelVisibility(lineDisplayNode->GetLabelVisibility());
          }
        }
      else
        {
        rep->GetAxis()->SetLabelVisibility(lineDisplayNode->GetLabelVisibility());
        }
      rep->SetRulerMode(lineDisplayNode->GetMaxTicks() ? 1 : 0);
      rep->SetNumberOfRulerTicks(lineDisplayNode->GetMaxTicks());
      rep->SetRulerDistance(this->ApplyUnit(lineDisplayNode->GetTickSpacing()));
      }
    rep->NeedToRenderOn();
    }
  else
    {
    /// 3d case

    // now get the widget properties (coordinates, measurement etc.) and if the mrml node
    // has changed, propagate the changes
    vtkAnnotationRulerRepresentation3D * rep = vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

    // update the distance measurement
    rep->SetDistance(distance);

    if (lineDisplayNode)
      {
      rep->SetLabelPosition(lineDisplayNode->GetLabelPosition());
      }
    if (textDisplayNode)
      {
      double textScale = textDisplayNode->GetTextScale();
      rep->SetLabelScale(textScale,textScale,textScale);
      }
    // set the color
    vtkHandleRepresentation *pointrep1 = rep->GetPoint1Representation();
    vtkHandleRepresentation *pointrep2 = rep->GetPoint2Representation();
    vtkPointHandleRepresentation3D *handle1 = nullptr;
    vtkPointHandleRepresentation3D *handle2 = nullptr;
    if (pointrep1 && pointrep2)
      {
      handle1 = vtkPointHandleRepresentation3D::SafeDownCast(pointrep1);
      handle2 = vtkPointHandleRepresentation3D::SafeDownCast(pointrep2);
      }
    if (handle1 && handle2 && pointDisplayNode)
      {
      if (rulerNode->GetSelected())
        {
        handle1->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
        handle2->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
        }
      else
        {
        handle1->GetProperty()->SetColor(pointDisplayNode->GetColor());
        handle2->GetProperty()->SetColor(pointDisplayNode->GetColor());
        }
      }
    if (pointDisplayNode)
      {
      // opacity
      handle1->GetProperty()->SetOpacity(pointDisplayNode->GetOpacity());
      handle2->GetProperty()->SetOpacity(pointDisplayNode->GetOpacity());
      // scale
      handle1->SetHandleSize(pointDisplayNode->GetGlyphScale());
      handle2->SetHandleSize(pointDisplayNode->GetGlyphScale());
      // use the point scale and opacity and color for the ticks
      rep->SetGlyphScale(pointDisplayNode->GetGlyphScale());
      rep->GetGlyphActor()->GetProperty()->SetOpacity(pointDisplayNode->GetOpacity());
      if (rulerNode->GetSelected())
        {
        rep->GetGlyphActor()->GetProperty()->SetColor(pointDisplayNode->GetSelectedColor());
        }
      else
        {
        rep->GetGlyphActor()->GetProperty()->SetColor(pointDisplayNode->GetColor());
        }
      }
    if (textDisplayNode)
      {
      if (rulerNode->GetSelected())
        {
        rep->GetLabelProperty()->SetColor(textDisplayNode->GetSelectedColor());
        }
      else
        {
        rep->GetLabelProperty()->SetColor(textDisplayNode->GetColor());
        }
      // if the text node says not to show text, use the label property to set it invisible,
      // otherwise, use the text displaynode's opacity setting (since there are no tick
      // labels in the 3D case, don't sue the line display node label visibility).
      if (!textDisplayNode->GetVisibility() || !textDisplayNode->GetVisibility3D())
        {
        rep->GetLabelProperty()->SetOpacity(0);
        }
      else
        {
        vtkProperty *labelProp = rep->GetLabelProperty();
        labelProp->SetOpacity(textDisplayNode->GetOpacity());
        }
      }
    if (lineDisplayNode && rep->GetLineProperty())
      {
      if (rulerNode->GetSelected())
        {
        rep->GetLineProperty()->SetColor(lineDisplayNode->GetSelectedColor());
        }
      else
        {
        rep->GetLineProperty()->SetColor(lineDisplayNode->GetColor());
        }
      rep->GetLineProperty()->SetLineWidth(lineDisplayNode->GetLineThickness());
      rep->GetLineProperty()->SetOpacity(lineDisplayNode->GetOpacity());
      // vtkProperty2D only defines opacity, color and line width
      //rep->GetLineProperty()->SetAmbient(lineDisplayNode->GetAmbient());
      //rep->GetLineProperty()->SetDiffuse(lineDisplayNode->GetDiffuse());
      //rep->GetLineProperty()->SetSpecular(lineDisplayNode->GetSpecular());

      //double thickness = lineDisplayNode->GetLineThickness();
      rep->SetRulerDistance(this->ApplyUnit(lineDisplayNode->GetTickSpacing()));
      rep->SetMaxTicks(lineDisplayNode->GetMaxTicks());
      }

    rep->NeedToRenderOn();
    }

  // update the label format
  vtkDistanceRepresentation *rep = vtkDistanceRepresentation::SafeDownCast(rulerWidget->GetRepresentation());
  if (rep)
    {
    rep->SetLabelFormat(this->GetLabelFormat(rulerNode).c_str());
    }

  // update the position
  this->UpdatePosition(widget, node);

  rulerWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Get the label from the application and the ruler widget
std::string vtkMRMLAnnotationRulerDisplayableManager
::GetLabelFormat(vtkMRMLAnnotationRulerNode* rulerNode)
{
  if (!rulerNode)
    {
    return "";
    }

  std::string format = "%-#6.3g mm";
  if (this->GetMRMLScene())
    {
    vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));

    if (selectionNode)
      {
      vtkMRMLUnitNode* lengthUnit = vtkMRMLUnitNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          selectionNode->GetUnitNodeID("length")));
      if (lengthUnit)
        {
        format = lengthUnit->GetDisplayStringFormat();
        }
      }
    }

  if (rulerNode->GetName())
    {
    format = std::string(rulerNode->GetName()) + std::string(" :") + format;
    }
  return format;
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationRulerDisplayableManager::GetDistance(
  const double* worldCoordinates1, const double* worldCoordinates2)
{
  double distance =
    sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));
  return distance;
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationRulerDisplayableManager
::ApplyUnit(double lengthInMM)
{
  double length = lengthInMM;
  if (this->GetMRMLScene())
    {
    vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));

    if (selectionNode)
      {
      vtkMRMLUnitNode* lengthUnit = vtkMRMLUnitNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          selectionNode->GetUnitNodeID("length")));
      if (lengthUnit)
        {
        length = lengthUnit->GetDisplayValueFromValue(lengthInMM);
        }
      }
    }
  return length;
}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationRulerDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
  vtkAnnotationRulerWidget* rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!rulerWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ruler widget!");
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ruler node!");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;
  rulerNode->DisableModifiedEventOn();

  double worldCoordinates1[4]={0,0,0,1};
  double worldCoordinates2[4]={0,0,0,1};

  bool allowMovement = true;

  if (this->GetSliceNode())
    {
    // 2D widget was changed

    // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
    vtkAnnotationRulerRepresentation * rep = vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());


    double displayCoordinates1[4]={0,0,0,1};
    double displayCoordinates2[4]={0,0,0,1};
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

    // Compute and update representation
    double distance = this->ApplyUnit(this->GetDistance(worldCoordinates1,worldCoordinates2));
    rep->SetDistance(distance);

    }
  else
    {
    // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
    vtkAnnotationRulerRepresentation3D * rep = vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

    rep->GetPoint1WorldPosition(worldCoordinates1);
    rep->GetPoint2WorldPosition(worldCoordinates2);
    }

  // if movement is not allowed, jump out
  if (!allowMovement)
    {
    return;
    }

  // save worldCoordinates to MRML if changed
  double p1[4]={0,0,0,1};
  double p2[4]={0,0,0,1};
  rulerNode->GetPositionWorldCoordinates1(p1);
  rulerNode->GetPositionWorldCoordinates2(p2);
  if (this->GetWorldCoordinatesChanged(worldCoordinates1, p1))
    {
    rulerNode->SetPositionWorldCoordinates1(worldCoordinates1);
    }
  if (this->GetWorldCoordinatesChanged(worldCoordinates2, p2))
    {
    rulerNode->SetPositionWorldCoordinates2(worldCoordinates2);
    }

  // save the current view
  rulerNode->SaveView();

  // enable processing of modified events
  rulerNode->DisableModifiedEventOff();

  rulerNode->Modified();
  rulerNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, rulerNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationRulerDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (!this->m_ClickCounter->HasEnoughClicks(2))
    {
    this->GetDisplayToWorldCoordinates(x, y, this->LastClickWorldCoordinates);
    }
  else
   {
    // switch to updating state to avoid events mess
    this->m_Updating = 1;


    //vtkHandleWidget *h1 = this->GetSeed(0);
    vtkHandleWidget *h2 = this->GetSeed(1);

    // convert the coordinates
    //double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4] = {this->LastClickWorldCoordinates[0],
                                  this->LastClickWorldCoordinates[1],
                                  this->LastClickWorldCoordinates[2],
                                  1};
    double worldCoordinates2[4] = {0,0,0,1};

    //this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],worldCoordinates1);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],worldCoordinates2);

    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

    rulerNode->SetPositionWorldCoordinates1(worldCoordinates1);
    rulerNode->SetPositionWorldCoordinates2(worldCoordinates2);

    rulerNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("M"));



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
      rulerNode->SetAttribute("AssociatedNodeID", associatedNodeID);
      }
    rulerNode->Initialize(this->GetMRMLScene());

    rulerNode->Delete();

    // reset updating state
    this->m_Updating = 0;
    }

  }

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
{
    if (!widget)
    {
    vtkErrorMacro("UpdatePosition: Widget was null!");
    return;
    }

  if (!node)
    {
    vtkErrorMacro("UpdatePosition: MRML node was null!");
    return;
    }

  // cast to the specific widget
  vtkAnnotationRulerWidget* rulerWidget = vtkAnnotationRulerWidget::SafeDownCast(widget);

  if (!rulerWidget)
    {
    vtkErrorMacro("UpdatePosition: Could not get ruler widget!");
    return;
    }

  // cast to the specific mrml node
    // cast to the specific mrml node
  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("UpdatePosition: Could not get ruler node!");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  double worldCoordinates1[4]={0,0,0,1};
  rulerNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double worldCoordinates2[4]={0,0,0,1};
  rulerNode->GetControlPointWorldCoordinates(1, worldCoordinates2);


  // update the location
  if (this->Is2DDisplayableManager())
    {
    // get the 2d representation
    vtkAnnotationRulerRepresentation * rulerRepresentation =
        vtkAnnotationRulerRepresentation::SafeDownCast(rulerWidget->GetRepresentation());

    // change the 2D location
    double displayCoordinates1[4]={0,0,0,1};
    double displayCoordinates2[4]={0,0,0,1};
    double displayCoordinatesBuffer1[4]={0,0,0,1};
    double displayCoordinatesBuffer2[4]={0,0,0,1};

    this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);
    this->GetWorldToDisplayCoordinates(worldCoordinates2,displayCoordinates2);

    // only update the position, if coordinates really change
    rulerRepresentation->GetPoint1DisplayPosition(displayCoordinatesBuffer1);
    rulerRepresentation->GetPoint2DisplayPosition(displayCoordinatesBuffer2);

    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      rulerRepresentation->SetPoint1DisplayPosition(displayCoordinates1);
      }
    if (this->GetDisplayCoordinatesChanged(displayCoordinates2,displayCoordinatesBuffer2))
      {
      rulerRepresentation->SetPoint2DisplayPosition(displayCoordinates2);
      }
    }
  else
    {
    /// 3d case
    // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
    vtkAnnotationRulerRepresentation3D * rulerRepresentation3D =
        vtkAnnotationRulerRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());
    // change the 3D location
    rulerRepresentation3D->SetPoint1WorldPosition(worldCoordinates1);
    rulerRepresentation3D->SetPoint2WorldPosition(worldCoordinates2);

    rulerRepresentation3D->NeedToRenderOn();
    }

  rulerWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerDisplayableManager
::OnMRMLSelectionNodeUnitModifiedEvent(vtkMRMLSelectionNode* selectionNode)
{
  if (!selectionNode)
    {
    return;
    }

  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  for (it = this->Helper->AnnotationNodeList.begin();
    it != this->Helper->AnnotationNodeList.end(); ++it)
    {
    vtkMRMLAnnotationRulerNode* rulerNode =
      vtkMRMLAnnotationRulerNode::SafeDownCast((*it));
    if (rulerNode)
      {
      // The unit has changed, modify the ruler node to refresh the label.
      rulerNode->Modified();
      }
    }
}
