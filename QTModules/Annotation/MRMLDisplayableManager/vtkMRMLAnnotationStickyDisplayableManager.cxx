// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationStickyDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationStickyNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// VTK includes
#include <vtkObject.h>
#include <vtkPNGReader.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkLogoWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkLogoRepresentation.h>
#include <vtkAbstractWidget.h>


// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationStickyDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationStickyDisplayableManager, "$Revision: 1.0 $");

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

  vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

  if (!stickyNode)
    {
    vtkErrorMacro("CreateWidget: Could not get sticky node!")
    return 0;
    }

  vtkHandleWidget * h1 = this->m_HandleWidgetList[0];

  // we get display coordinates, we need normalized viewport
  // so we transform
  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double x = position1[0];
  double y = position1[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedViewport(x,y);

  VTK_CREATE(vtkPNGReader,r);

  // a hack to get the icon
  std::stringstream s;
  //s << std::getenv("TMPDIR") << "sticky.png";
  s << "/tmp/" << "sticky.png";
  r->SetFileName(s.str().c_str());

  VTK_CREATE(vtkLogoRepresentation,logoRepresentation);
  logoRepresentation->SetImage(r->GetOutput());
  logoRepresentation->SetPosition(x,y); // here we set normalized viewport
  logoRepresentation->SetPosition2(.1, .1);
  logoRepresentation->GetImageProperty()->SetOpacity(.7);

  vtkLogoWidget * logoWidget = vtkLogoWidget::New();
  logoWidget->SetInteractor(this->GetInteractor());
  logoWidget->SetRepresentation(logoRepresentation);

  // add the callback
  vtkAnnotationStickyWidgetCallback *myCallback = vtkAnnotationStickyWidgetCallback::New();
  myCallback->SetNode(stickyNode);
  myCallback->SetWidget(logoWidget);
  myCallback->SetDisplayableManager(this);
  logoWidget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  logoWidget->On();

  return logoWidget;

  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationStickyDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  //this->m_DisableModifiedEvents = 0;
}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationStickyDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkLogoWidget* logoWidget = vtkLogoWidget::SafeDownCast(widget);

  if (!logoWidget)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get logo widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

  if (!stickyNode)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get sticky node!")
   return;
   }

  // if this flag is true after the checks below, the widget will be set to modified
  bool hasChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
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

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to widget
  //
  if (stickyNode->GetControlPointCoordinates(0)[0] != worldCoordinates[0] || stickyNode->GetControlPointCoordinates(0)[1] != worldCoordinates[1] || stickyNode->GetControlPointCoordinates(0)[2] != worldCoordinates[2])
    {
    // at least one coordinate has changed, so update the widget

    // now we have to transfer again from world coordinates to normalized viewport coordinates
    double * displayCoordinates = this->GetWorldToDisplayCoordinates(stickyNode->GetControlPointCoordinates(0)[0],stickyNode->GetControlPointCoordinates(0)[1],stickyNode->GetControlPointCoordinates(0)[2]);

    double u = displayCoordinates[0];
    double v = displayCoordinates[1];

    this->GetRenderer()->DisplayToNormalizedDisplay(u,v);
    this->GetRenderer()->NormalizedDisplayToViewport(u,v);
    this->GetRenderer()->ViewportToNormalizedViewport(u,v);
    // now we have transformed the world coordinates in the MRML node to normalized viewport coordinates and can really update the widget

    rep->SetPosition(u,v);
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so set the widget to modified
    rep->NeedToRenderOn();
    logoWidget->Modified();
    }

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationStickyDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
   vtkLogoWidget* logoWidget = vtkLogoWidget::SafeDownCast(widget);

   if (!logoWidget)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get logo widget!")
     return;
     }

   // cast to the specific mrml node
   vtkMRMLAnnotationStickyNode* stickyNode = vtkMRMLAnnotationStickyNode::SafeDownCast(node);

   if (!stickyNode)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get sticky node!")
     return;
     }

  // if this flag is true after the checks below, the modified event gets fired
  bool hasChanged = false;

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

  //
  // Check if the position of the widget is different than the saved one in the mrml node
  // If yes, propagate the changes to the mrml node
  //
  if (stickyNode->GetControlPointCoordinates(0)[0] != worldCoordinates[0] || stickyNode->GetControlPointCoordinates(0)[1] != worldCoordinates[1] || stickyNode->GetControlPointCoordinates(0)[2] != worldCoordinates[2])
    {
    // at least one coordinate has changed, so update the mrml property
    stickyNode->SetStickyCoordinates(worldCoordinates);
    hasChanged = true;
    }

  if (hasChanged)
    {
    // at least one value has changed, so fire the modified event
    stickyNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, stickyNode);
    }

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationStickyDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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

    double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);

    // Create the node
    vtkMRMLAnnotationStickyNode *stickyNode = vtkMRMLAnnotationStickyNode::New();

    stickyNode->Initialize(this->GetMRMLScene());

    stickyNode->SetStickyCoordinates(worldCoordinates);

    stickyNode->SetName(stickyNode->GetScene()->GetUniqueNameByString("AnnotationStickyNote"));

    stickyNode->Delete();

    }

  }
