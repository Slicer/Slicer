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
#include <vtkCaptionWidget.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionActor2D.h>
#include <vtkRenderer.h>
#include <vtkHandleRepresentation.h>
#include <vtkAbstractWidget.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>

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
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationTextWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationTextWidgetCallback *New()
  { return new vtkAnnotationTextWidgetCallback; }

  vtkAnnotationTextWidgetCallback(){}

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
        vtkCaptionRepresentation* representation = vtkCaptionRepresentation::SafeDownCast(this->m_Widget->GetRepresentation());

        double displayCoordinates1[4];

        // first, we get the current displayCoordinates of the points
        representation->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
        representation->GetCaptionActor2D()->GetAttachmentPointCoordinate()->GetValue(displayCoordinates1);

        // second, we copy these to restrictedDisplayCoordinates
        double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};

        // modify restrictedDisplayCoordinates 1 and 2, if these are outside the viewport of the current renderer
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);

        // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
          {

          representation->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
          representation->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetValue(displayCoordinates1);

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

  vtkCaptionWidget* captionWidget = vtkCaptionWidget::New();
  VTK_CREATE(vtkCaptionRepresentation, captionRep);

  captionRep->SetMoving(1);
  
  if (textNode->GetTextLabel())
    {
    captionRep->GetCaptionActor2D()->SetCaption(textNode->GetTextLabel());
    }
  else
    {
    captionRep->GetCaptionActor2D()->SetCaption("New text");
    }


  double* tmpPtr = textNode->GetControlPointCoordinates(0);
  double worldCoordinates1[4] = {tmpPtr[0], tmpPtr[1], tmpPtr[2], 1};

  double displayCoordinates1[4];

  if (this->GetSliceNode())
     {

     this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);

     captionRep->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
     captionRep->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetValue(displayCoordinates1);

     // turn off the three dimensional leader
     captionRep->GetCaptionActor2D()->ThreeDimensionalLeaderOff();

     }
   else
     {
     captionRep->SetAnchorPosition(worldCoordinates1);
     // turn on the three dimensional leader
     captionRep->GetCaptionActor2D()->ThreeDimensionalLeaderOn();

     }

  double *captionWorldCoordinates = textNode->GetControlPointCoordinates(1);
  double captionViewportCoordinates[4];
  double captionDisplayCoordinates[4];
  this->GetWorldToDisplayCoordinates(captionWorldCoordinates,captionDisplayCoordinates);
  this->GetDisplayToViewportCoordinates(captionDisplayCoordinates[0], captionDisplayCoordinates[1], captionViewportCoordinates);
  captionRep->SetPosition(captionViewportCoordinates);
  
  captionWidget->SetInteractor(this->GetInteractor());

  captionWidget->SetRepresentation(captionRep);

  captionWidget->On();

  vtkDebugMacro("CreateWidget: Widget was set up")

  return captionWidget;

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

  // store the current view
  node->SaveView();

  // propagate the widget to the MRML node
  //this->PropagateWidgetToMRML(widget, node);
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
  vtkCaptionWidget * captionWidget = vtkCaptionWidget::SafeDownCast(widget);

  if (!captionWidget)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get caption widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get text node!")
   return;
   }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkCaptionRepresentation * rep = vtkCaptionRepresentation::SafeDownCast(captionWidget->GetRepresentation());

  double* tmpPtr = textNode->GetControlPointCoordinates(0);
  double worldCoordinates1[4] = {tmpPtr[0], tmpPtr[1], tmpPtr[2], 1};

  double displayCoordinates1[4];

  if (this->GetSliceNode())
     {

     this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);

     rep->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
     rep->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetValue(displayCoordinates1);

     }
   else
     {
     rep->SetAnchorPosition(worldCoordinates1);

     }

  // update widget text
  rep->GetCaptionActor2D()->SetCaption(textNode->GetText(0).c_str());
  // TODO: trigger resizing the boundary around it if it's on and text changed

  
  if (!textNode->GetAnnotationTextDisplayNode())
    {
    // no display node yet, create one
    textNode->CreateAnnotationTextDisplayNode();
    }

  // update widget textscale
  rep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->SetFontSize(textNode->GetTextScale());

  if (textNode->GetSelected())
    {
    // update widget selected color
    rep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->SetColor(textNode->GetAnnotationTextDisplayNode()->GetSelectedColor());
    }
  else
    {
    // update widget color
    rep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->SetColor(textNode->GetAnnotationTextDisplayNode()->GetColor());
    }

  // at least one value has changed, so set the widget to modified
  rep->NeedToRenderOn();
  captionWidget->Modified();

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
   vtkCaptionWidget * captionWidget = vtkCaptionWidget::SafeDownCast(widget);

   if (!captionWidget)
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

  // disable processing of modified events
  this->m_Updating = 1;
  textNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkCaptionRepresentation * rep = vtkCaptionRepresentation::SafeDownCast(captionWidget->GetRepresentation());

  double worldCoordinates1[4];
  double displayCoordinates1[4];

  // for the caption 
  double worldCoordinates2[4];
  double displayCoordinates2[4];

  bool allowMovement = true;

  if (this->GetSliceNode())
    {

    rep->GetCaptionActor2D()->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
    rep->GetCaptionActor2D()->GetAttachmentPointCoordinate()->GetValue(displayCoordinates1);

    
    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

    if (displayCoordinates1[0] < 0 || displayCoordinates1[0] > this->GetInteractor()->GetRenderWindow()->GetSize()[0])
      {
      allowMovement = false;
      }

    }
  else
    {

    rep->GetAnchorPosition(worldCoordinates1);

    }

  // if movement is not allowed, jump out
  if (!allowMovement)
    {
    return;
    }

  // update mrml coordinates
  textNode->SetTextCoordinates(worldCoordinates1);

  double *captionPosition = rep->GetPosition();
  //rep->GetPosition(displayCoordinates2);
  if (captionPosition)
    {
    displayCoordinates2[0] = captionPosition[0];
    displayCoordinates2[1] = captionPosition[1];
    displayCoordinates2[2] = 0.0;
    displayCoordinates2[3] = 0.0;
    }
  this->GetDisplayToWorldCoordinates(displayCoordinates2,worldCoordinates2);
  textNode->SetCaptionCoordinates(worldCoordinates2);
  
  // update mrml text
  textNode->SetText(0,rep->GetCaptionActor2D()->GetCaption(),1,1);

  if (!textNode->GetAnnotationTextDisplayNode())
    {
    // no display node yet, create one
    textNode->CreateAnnotationTextDisplayNode();
    }

  // update mrml textscale
  textNode->SetTextScale(rep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->GetFontSize());

  // update mrml selected color
  //textNode->GetAnnotationTextDisplayNode()->SetSelectedColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());

  // update mrml color
  //textNode->GetAnnotationTextDisplayNode()->SetColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());


  // save the current view
  textNode->SaveView();

  // enable processing of modified events
  textNode->DisableModifiedEventOff();

  textNode->Modified();
  textNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, textNode);

  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationTextDisplayableManager::OnClickInRenderWindow(double x, double y)
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

    vtkHandleWidget *h1 = this->GetSeed(0);

    // convert the coordinates
    double* displayCoordinates1 = vtkHandleRepresentation::SafeDownCast(h1->GetRepresentation())->GetDisplayPosition();
    // offset the caption
    double displayCoordinates2[4];
    if (this->GetSliceNode())
     {
     // offset by 50
     displayCoordinates2[0] = displayCoordinates1[0] - 50.0;
     displayCoordinates2[1] = displayCoordinates1[1] - 50.0;
     }
    else
      {
      // offest by a bit more
      displayCoordinates2[0] = displayCoordinates1[0] - 100.0;
      displayCoordinates2[1] = displayCoordinates1[1] - 100.0;
      }
    displayCoordinates2[2] = 0.0;
    displayCoordinates2[3] = 0.0;
    
    if (displayCoordinates1)
      {
      vtkDebugMacro("OnClickInRenderWindow: displaycoordinates1 (anchor) = " << displayCoordinates1[0] << ", " << displayCoordinates1[1] << ", 2 (caption) = " << displayCoordinates2[0] << ", " << displayCoordinates2[1] << ", " << displayCoordinates2[2]);
      }
    double worldCoordinates1[4];
    double worldCoordinates2[4];
      
    this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],worldCoordinates1);

    this->RestrictDisplayCoordinatesToViewport(displayCoordinates2);
    vtkDebugMacro("OnClickInRenderWindow: restricted to viewport displaycoordinates2 = " << displayCoordinates2[0] << ", " << displayCoordinates2[1] << ", " << displayCoordinates2[2]);
    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],worldCoordinates2);

    // create the MRML node
    vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
    textNode->SetTextCoordinates(worldCoordinates1);
    textNode->SetCaptionCoordinates(worldCoordinates2);
    
    textNode->SetTextLabel("New text");

    textNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationText"));

    textNode->Initialize(this->GetMRMLScene());

    textNode->Delete();

    // reset updating state
    this->m_Updating = 0;

    }

  }
