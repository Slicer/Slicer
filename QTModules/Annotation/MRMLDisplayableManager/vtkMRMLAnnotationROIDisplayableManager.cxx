// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationROIDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkAbstractWidget.h>

// std includes
#include <string>
#include <math.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationROIDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationROIDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationROIDisplayableManager Callback
class vtkAnnotationROIWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationROIWidgetCallback *New()
  { return new vtkAnnotationROIWidgetCallback; }

  vtkAnnotationROIWidgetCallback(){}


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
// vtkMRMLAnnotationROIDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROIDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationROIDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("CreateWidget: Could not get ROI node!")
    return 0;
    }

  vtkBoxWidget2* boxWidget = vtkBoxWidget2::New();

  boxWidget->SetInteractor(this->GetInteractor());
  boxWidget->SetCurrentRenderer(this->GetRenderer());

  boxWidget->CreateDefaultRepresentation();

  vtkBoxRepresentation * boxRepresentation = vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  double * origin = roiNode->GetXYZ();
  double * radius = roiNode->GetRadiusXYZ();

  double d = sqrt((origin[0]-radius[0])*(origin[0]-radius[0])+(origin[1]-radius[1])*(origin[1]-radius[1])+(origin[2]-radius[2])*(origin[2]-radius[2]));

  double bounds[6];
  bounds[0]=origin[0]-d;
  bounds[1]=origin[0]+d;
  bounds[2]=origin[1]-d;
  bounds[3]=origin[1]+d;
  bounds[4]=origin[2]-d;
  bounds[5]=origin[2]+d;

  boxRepresentation->PlaceWidget(bounds);


  //boxWidget->ProcessEventsOff();

  boxWidget->On();

  boxRepresentation->EndWidgetInteraction(origin);

  return boxWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationROIDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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

  vtkAnnotationROIWidgetCallback *myCallback = vtkAnnotationROIWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

 // this->m_Updating = 0;
  // just propagate the bounds to the MRML node, because before we use only origin and radius
  //this->PropagateWidgetToMRML(widget,node);
}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationROIDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkBoxWidget2* boxWidget = vtkBoxWidget2::SafeDownCast(widget);

  if (!boxWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get box widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ROI node!")
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
  vtkBoxRepresentation * rep = vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  rep->PlaceWidget(roiNode->GetBounds());

  rep->NeedToRenderOn();
  boxWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationROIDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
  vtkBoxWidget2* boxWidget = vtkBoxWidget2::SafeDownCast(widget);

  if (!boxWidget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get box widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);

  if (!roiNode)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Could not get ROI node!")
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
  vtkBoxRepresentation * rep = vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());

  double * bounds = rep->GetBounds();


  roiNode->SetBounds(bounds);

  roiNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, roiNode);

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationROIDisplayableManager::OnClickInRenderWindow(double x, double y)
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

    double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetWorldPosition();
    double* position2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetWorldPosition();

    // create the MRML node
    vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::New();

    roiNode->SetXYZ(position1);
    roiNode->SetRadiusXYZ(position2);

    roiNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationROI"));

    roiNode->Initialize(this->GetMRMLScene());

    roiNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
