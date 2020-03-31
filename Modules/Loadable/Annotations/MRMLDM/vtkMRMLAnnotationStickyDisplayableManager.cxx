
// Annotation Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationStickyDisplayableManager.h"

// Annotation MRML includes
#include <vtkMRMLAnnotationDisplayableManager.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationStickyNode.h>

#include <vtkMRMLInteractionNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkLogoRepresentation.h>
#include <vtkLogoWidget.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkObject.h>
#include <vtkPNGReader.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationStickyDisplayableManager);

//---------------------------------------------------------------------------
// vtkMRMLAnnotationStickyDisplayableManager Callback
class vtkAnnotationStickyWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationStickyWidgetCallback *New()
  { return new vtkAnnotationStickyWidgetCallback; }

  vtkAnnotationStickyWidgetCallback(){}

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
// vtkMRMLAnnotationStickyDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationStickyDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationStickyDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!");
    return 0;
    }

  vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

  if (!stickyNode)
    {
    vtkErrorMacro("CreateWidget: Could not get sticky node!");
    return 0;
    }

  vtkHandleWidget * h1 = this->GetSeed(0);

  // we get display coordinates, we need normalized viewport
  // so we transform
  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double x = position1[0];
  double y = position1[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedViewport(x,y);

  vtkNew<vtkPNGReader> r;

  // a hack to get the icon
  std::stringstream s;
  //s << std::getenv("TMPDIR") << "sticky.png";
  s << "/tmp/" << "sticky.png";
  r->SetFileName(s.str().c_str());

  vtkNew<vtkLogoRepresentation> logoRepresentation;
  logoRepresentation->SetImage(r->GetOutput());
  logoRepresentation->SetPosition(x,y); // here we set normalized viewport
  logoRepresentation->SetPosition2(.1, .1);
  logoRepresentation->GetImageProperty()->SetOpacity(.7);

  vtkLogoWidget * logoWidget = vtkLogoWidget::New();
  logoWidget->SetInteractor(this->GetInteractor());
  logoWidget->SetRepresentation(logoRepresentation.GetPointer());

  logoWidget->On();

  return logoWidget;

  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationStickyDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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

  // add the callback
  vtkAnnotationStickyWidgetCallback *myCallback = vtkAnnotationStickyWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  this->m_Updating = 0;
  // propagate the widget to the MRML node
  this->PropagateWidgetToMRML(widget, node);
}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationStickyDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkLogoWidget* logoWidget = vtkLogoWidget::SafeDownCast(widget);

  if (!logoWidget)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get logo widget!");
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

  if (!stickyNode)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get sticky node!");
   return;
   }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Updating in progress.. Exit now.");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkLogoRepresentation * rep = vtkLogoRepresentation::SafeDownCast(logoWidget->GetRepresentation());

  // now we have to transfer again from world coordinates to normalized viewport coordinates
  double * displayCoordinates = this->GetWorldToDisplayCoordinates(
        stickyNode->GetControlPointCoordinates(0)[0],
        stickyNode->GetControlPointCoordinates(0)[1],
        stickyNode->GetControlPointCoordinates(0)[2]);

  double u = displayCoordinates[0];
  double v = displayCoordinates[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(u,v);
  this->GetRenderer()->NormalizedDisplayToViewport(u,v);
  this->GetRenderer()->ViewportToNormalizedViewport(u,v);
  // now we have transformed the world coordinates in the MRML node to normalized viewport coordinates and can really update the widget

  rep->SetPosition(u,v);


  // at least one value has changed, so set the widget to modified
  rep->NeedToRenderOn();
  logoWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationStickyDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
   vtkLogoWidget* logoWidget = vtkLogoWidget::SafeDownCast(widget);

   if (!logoWidget)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get logo widget!");
     return;
     }

   // cast to the specific mrml node
   vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

   if (!stickyNode)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get sticky node!");
     return;
     }

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.");
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkLogoRepresentation * rep = vtkLogoRepresentation::SafeDownCast(logoWidget->GetRepresentation());

  double * position1 = rep->GetPosition();

  double x = position1[0];
  double y = position1[1];

  // we have normalized viewport coordinates but we need world coordinates
  this->GetRenderer()->NormalizedViewportToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToDisplay(x,y);

  double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);
  // now we have world coordinates :)

  stickyNode->SetStickyCoordinates(worldCoordinates);

  stickyNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, stickyNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationStickyDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (this->m_ClickCounter->HasEnoughClicks(1))
    {

    // switch to updating state to avoid events mess
    this->m_Updating = 1;

    double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);

    // Create the node
    vtkMRMLAnnotationStickyNode *stickyNode = vtkMRMLAnnotationStickyNode::New();

    // if this was a one time place, go back to view transform mode
    vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }

    this->GetMRMLScene()->SaveStateForUno();

    // is there a node associated with this?
    if (associatedNodeID)
      {
      stickyNode->SetAttribute("AssociatedNodeID", associatedNodeID);
      }

    stickyNode->Initialize(this->GetMRMLScene());

    stickyNode->SetStickyCoordinates(worldCoordinates);

    stickyNode->SetName(stickyNode->GetScene()->GetUniqueNameByString("N"));

    stickyNode->Delete();

    // reset updating state
    this->m_Updating = 0;


    }

  }
