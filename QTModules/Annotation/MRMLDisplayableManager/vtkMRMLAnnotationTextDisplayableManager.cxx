// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationTextDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationDisplayableManager.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkAbstractWidget.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationTextDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationTextDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationTextDisplayableManager Callback
class vtkAnnotationTextWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationTextWidgetCallback *New()
  { return new vtkAnnotationTextWidgetCallback; }

  vtkAnnotationTextWidgetCallback(){}

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
// vtkMRMLAnnotationTextDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationTextDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
    {
    vtkErrorMacro("CreateWidget: Could not get text node!")
    return 0;
    }

  vtkTextWidget* textWidget = vtkTextWidget::New();
  VTK_CREATE(vtkTextRepresentation, textRep);

  textRep->SetMoving(1);

  if (textNode->GetTextLabel())
    {
    textRep->SetText(textNode->GetTextLabel());
    }
  else
    {
    textRep->SetText("New text");
    }

  textWidget->SetRepresentation(textRep);

  textWidget->SetInteractor(this->GetInteractor());

  // we get display coordinates
  // we need normalized viewport coordinates, so we transform
  vtkHandleWidget * h1 = this->GetSeed(0);

  double* position1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();

  double x = position1[0];
  double y = position1[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedViewport(x,y);

  // we set normalized viewport coordinates
  textRep->SetPosition(x,y);

  textWidget->On();

  return textWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationTextDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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

  // add the callback
  vtkAnnotationTextWidgetCallback *myCallback = vtkAnnotationTextWidgetCallback::New();
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
void vtkMRMLAnnotationTextDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkTextWidget * textWidget = vtkTextWidget::SafeDownCast(widget);

  if (!textWidget)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get text widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get text node!")
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
  vtkTextRepresentation * rep = vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation());

  // now we have to transfer again from world coordinates to normalized viewport coordinates
  double * displayCoordinates = this->GetWorldToDisplayCoordinates(textNode->GetTextCoordinates()[0], textNode->GetTextCoordinates()[1], textNode->GetTextCoordinates()[2]);

  double u = displayCoordinates[0];
  double v = displayCoordinates[1];

  this->GetRenderer()->DisplayToNormalizedDisplay(u,v);
  this->GetRenderer()->NormalizedDisplayToViewport(u,v);
  this->GetRenderer()->ViewportToNormalizedViewport(u,v);
  // now we have transformed the world coordinates in the MRML node to normalized viewport coordinates and can really update the widget

  // update widget position
  rep->SetPosition(u,v);

  // update widget text
  rep->SetText(textNode->GetText(0).c_str());

  // update widget textscale
  rep->GetTextActor()->GetScaledTextProperty()->SetFontSize(textNode->GetTextScale());

  if (textNode->GetSelected())
    {
    // update widget selected color
    rep->GetTextActor()->GetScaledTextProperty()->SetColor(textNode->GetAnnotationTextDisplayNode()->GetSelectedColor());
    }
  else
    {
    // update widget color
    rep->GetTextActor()->GetScaledTextProperty()->SetColor(textNode->GetAnnotationTextDisplayNode()->GetColor());
    }

  // at least one value has changed, so set the widget to modified
  rep->NeedToRenderOn();
  textWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationTextDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
   vtkTextWidget * textWidget = vtkTextWidget::SafeDownCast(widget);

   if (!textWidget)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get text widget!")
     return;
     }

   // cast to the specific mrml node
   vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

   if (!textNode)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get text node!")
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
  vtkTextRepresentation * rep = vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation());

  double * position1 = rep->GetPosition();

  double x = position1[0];
  double y = position1[1];

  // we have normalized viewport coordinates but we need world coordinates
  this->GetRenderer()->NormalizedViewportToViewport(x,y);
  this->GetRenderer()->ViewportToNormalizedDisplay(x,y);
  this->GetRenderer()->NormalizedDisplayToDisplay(x,y);

  double* worldCoordinates = this->GetDisplayToWorldCoordinates(x,y);
  // now we have world coordinates :)

  // update mrml coordinates
  textNode->SetTextCoordinates(worldCoordinates);

  // update mrml text
  textNode->SetText(0,rep->GetText(),1,1);

  if (!textNode->GetAnnotationTextDisplayNode())
    {
    // no display node yet, create one
    textNode->CreateAnnotationTextDisplayNode();
    }

  // update mrml textscale
  textNode->SetTextScale(rep->GetTextActor()->GetScaledTextProperty()->GetFontSize());

  // update mrml selected color
  //textNode->GetAnnotationTextDisplayNode()->SetSelectedColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());

  // update mrml color
  //textNode->GetAnnotationTextDisplayNode()->SetColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());

  // fire the modified event
  textNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, textNode);

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationTextDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
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

    // create the MRML node
    vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
    textNode->SetTextCoordinates(worldCoordinates);
    textNode->SetTextLabel("New text");

    textNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationText"));

    textNode->Initialize(this->GetMRMLScene());

    textNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
