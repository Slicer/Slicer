// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationRulerDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkDistanceWidget.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkDistanceRepresentation2D.h>
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

  vtkDistanceWidget * rulerWidget = vtkDistanceWidget::New();

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

  vtkDistanceWidget * rulerWidget = vtkDistanceWidget::SafeDownCast(widget);

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

  if (this->GetSliceNode())
    {
    // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();
    vtkMatrix4x4 * rasToXyMatrix = vtkMatrix4x4::New();

    // we need to invert this matrix
    xyToRasMatrix->Invert(xyToRasMatrix,rasToXyMatrix);

    rasToXyMatrix->MultiplyPoint(rulerNode->GetControlPointCoordinates(0),position1);
    rasToXyMatrix->MultiplyPoint(rulerNode->GetControlPointCoordinates(1),position2);

    }
  else
    {

    this->GetWorldToDisplayCoordinates(rulerNode->GetControlPointCoordinates(0),position1);
    this->GetWorldToDisplayCoordinates(rulerNode->GetControlPointCoordinates(1),position2);

    }

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

  if (this->GetSliceNode())
    {
    // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();
    vtkMatrix4x4 * rasToXyMatrix = vtkMatrix4x4::New();

    // we need to invert this matrix
    xyToRasMatrix->Invert(xyToRasMatrix,rasToXyMatrix);

    rasToXyMatrix->MultiplyPoint(worldCoordinates1, displayCoordinates1);
    rasToXyMatrix->MultiplyPoint(worldCoordinates2, displayCoordinates2);

    //std::cout << this->GetSliceNode()->GetName() << ": "<<position1[0] << "," << position1[1] << "," << position1[2] << "," << position1[3] << std::endl;

    vtkDistanceRepresentation2D::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint1DisplayPosition(displayCoordinates1);
    vtkDistanceRepresentation2D::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint2DisplayPosition(displayCoordinates2);

    }
  else
    {

    vtkDistanceRepresentation2D::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint1WorldPosition(worldCoordinates1);
    vtkDistanceRepresentation2D::SafeDownCast(rulerWidget->GetRepresentation())->SetPoint2WorldPosition(worldCoordinates2);

    }


  // add observer for end interaction
  vtkAnnotationRulerWidgetCallback *myCallback = vtkAnnotationRulerWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
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
  vtkDistanceWidget* rulerWidget = vtkDistanceWidget::SafeDownCast(widget);

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
  vtkDistanceRepresentation2D * rep = vtkDistanceRepresentation2D::SafeDownCast(rulerWidget->GetRepresentation());

  rep->SetPoint1WorldPosition(rulerNode->GetPosition1());

  rep->SetPoint2WorldPosition(rulerNode->GetPosition2());


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
  vtkDistanceWidget* rulerWidget = vtkDistanceWidget::SafeDownCast(widget);

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

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkDistanceRepresentation2D * rep = vtkDistanceRepresentation2D::SafeDownCast(rulerWidget->GetRepresentation());

  double position1[3];
  double position2[3];

  rep->GetPoint1WorldPosition(position1);
  rep->GetPoint2WorldPosition(position2);

  rulerNode->SetPosition1(position1);

  rulerNode->SetPosition2(position2);

  rulerNode->SetDistanceMeasurement(rep->GetDistance());

  rulerNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, rulerNode);

  // enable processing of modified events
  this->m_Updating = 0;

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

    double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4];
    double worldCoordinates2[4];


    if (this->GetSliceNode())
      {
      // the click was inside a 2D SliceView
      // we will get the transformation matrix to convert display coordinates to RAS

      vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();

      double displayCoordinates1[4];
      displayCoordinates1[0] = position1[0];
      displayCoordinates1[1] = position1[1];
      displayCoordinates1[2] = 0;
      displayCoordinates1[3] = 1;

      double displayCoordinates2[4];
      displayCoordinates2[0] = position2[0];
      displayCoordinates2[1] = position2[1];
      displayCoordinates2[2] = 0;
      displayCoordinates2[3] = 1;

      xyToRasMatrix->MultiplyPoint(displayCoordinates1, worldCoordinates1);
      xyToRasMatrix->MultiplyPoint(displayCoordinates2, worldCoordinates2);

      }
    else
      {
      // the click was inside a 3D RenderView
      // we can get the world coordinates by conversion
      this->GetDisplayToWorldCoordinates(position1[0],position1[1],worldCoordinates1);
      this->GetDisplayToWorldCoordinates(position2[0],position2[1],worldCoordinates2);
      }

    vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

    rulerNode->SetPosition1(worldCoordinates1);
    rulerNode->SetPosition2(worldCoordinates2);

    rulerNode->Initialize(this->GetMRMLScene());

    rulerNode->SetName(rulerNode->GetScene()->GetUniqueNameByString("AnnotationRuler"));

    rulerNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
