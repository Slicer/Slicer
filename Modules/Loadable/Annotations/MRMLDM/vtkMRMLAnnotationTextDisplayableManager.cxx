
// Annotation Logic includes
#include <vtkSlicerAnnotationModuleLogic.h>

// Annotation MRML includes
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationTextDisplayNode.h>
#include <vtkMRMLAnnotationTextNode.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationTextDisplayableManager.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionWidget.h>
#include <vtkCoordinate.h>
#include <vtkHandleRepresentation.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// STD includes
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationTextDisplayableManager);

//---------------------------------------------------------------------------
// vtkMRMLAnnotationTextDisplayableManager Callback
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationTextWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationTextWidgetCallback *New()
  { return new vtkAnnotationTextWidgetCallback; }

  vtkAnnotationTextWidgetCallback() = default;

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


      if (this->m_DisplayableManager->Is2DDisplayableManager())
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
    vtkErrorMacro("CreateWidget: Node not set!");
    return nullptr;
    }

  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
    {
    vtkErrorMacro("CreateWidget: Could not get text node!");
    return nullptr;
    }

  vtkCaptionWidget* captionWidget = vtkCaptionWidget::New();
  vtkNew<vtkCaptionRepresentation> captionRep;

  captionRep->SetMoving(1);

  if (!textNode->GetTextLabel())
    {
    textNode->SetTextLabel("New text");
    }
  // turn off text scaling
  if (captionRep->GetCaptionActor2D() &&
      captionRep->GetCaptionActor2D()->GetTextActor())
    {
    vtkDebugMacro("CreateWidget: turning text scaling off");
    captionRep->GetCaptionActor2D()->GetTextActor()->SetTextScaleMode(vtkTextActor::TEXT_SCALE_MODE_NONE);
    captionRep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->SetFontFamilyToTimes();
    captionRep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->SetShadow(0);
    captionRep->GetCaptionActor2D()->GetTextActor()->GetScaledTextProperty()->SetItalic(0);

    captionRep->GetCaptionActor2D()->GetCaptionTextProperty()->SetFontFamilyToTimes();
    captionRep->GetCaptionActor2D()->GetCaptionTextProperty()->SetShadow(0);
    captionRep->GetCaptionActor2D()->GetCaptionTextProperty()->SetItalic(0);
    }
  captionWidget->SetInteractor(this->GetInteractor());

  captionWidget->SetRepresentation(captionRep.GetPointer());

  // disallow the user being able to resize the border around the text
  captionWidget->ResizableOff();

  captionWidget->On();

  // now set it up from the node
  this->PropagateMRMLToWidget(textNode, captionWidget);

  vtkDebugMacro("CreateWidget: Widget was set up");

  return captionWidget;

}

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationTextDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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
  vtkAnnotationTextWidgetCallback *myCallback = vtkAnnotationTextWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  myCallback->Delete();

  // store the current view
  node->SaveView();

  // propagate the widget to the MRML node
//  this->PropagateWidgetToMRML(widget, node);
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationTextDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkCaptionWidget * captionWidget = vtkCaptionWidget::SafeDownCast(widget);

  if (!captionWidget)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get caption widget!");
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  if (!textNode)
   {
   vtkErrorMacro("PropagateMRMLToWidget: Could not get text node!");
   return;
   }

  // disable processing of modified events
  this->m_Updating = 1;

  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = textNode->GetAnnotationTextDisplayNode();
  if (!textDisplayNode)
    {
    // no display node yet, create one
    textNode->CreateAnnotationTextDisplayNode();
    textDisplayNode = textNode->GetAnnotationTextDisplayNode();
    if (!textDisplayNode)
      {
      vtkErrorMacro("PropagateMRMLToWidget: Error creating a new text display node for text node " << textNode->GetID());
      return;
      }
    }

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkCaptionRepresentation * rep = vtkCaptionRepresentation::SafeDownCast(captionWidget->GetRepresentation());
  vtkCaptionActor2D *captionActor = rep->GetCaptionActor2D();


  if (this->Is2DDisplayableManager())
     {
     // turn off the three dimensional leader
     captionActor->ThreeDimensionalLeaderOff();
     }
   else
     {
     // 3D case
     captionActor->SetThreeDimensionalLeader(textDisplayNode->GetUseThreeDimensionalLeader());
     }
  // update widget text
  if (textDisplayNode->GetUseLineWrap())
    {
    std::string wrappedText = textDisplayNode->GetLineWrappedText(textNode->GetText(0));
    captionActor->SetCaption(wrappedText.c_str());
    }
  else
    {
    captionActor->SetCaption(textNode->GetText(0).c_str());
    }
  // TODO: trigger resizing the boundary around it if it's on and text changed


  // update widget textscale
  captionActor->GetTextActor()->GetScaledTextProperty()->SetFontSize(textDisplayNode->GetTextScale());

  if (textNode->GetSelected())
    {
    // update widget selected color
    captionActor->GetTextActor()->GetScaledTextProperty()->SetColor(textDisplayNode->GetSelectedColor());
    }
  else
    {
    // update widget color
    captionActor->GetTextActor()->GetScaledTextProperty()->SetColor(textDisplayNode->GetColor());
    }

  captionActor->SetBorder(textDisplayNode->GetShowBorder());
  captionActor->SetLeader(textDisplayNode->GetShowLeader());
  if (textDisplayNode->GetShowLeader())
    {
    if (textDisplayNode->GetShowArrowHead())
      {
      captionActor->SetLeaderGlyphSize(textDisplayNode->GetLeaderGlyphSize());
      }
    else
      {
      captionActor->SetLeaderGlyphSize(0.0);
      }
    captionActor->SetMaximumLeaderGlyphSize(textDisplayNode->GetMaximumLeaderGlyphSize());
    }
  captionActor->SetPadding(textDisplayNode->GetPadding());
  captionActor->SetAttachEdgeOnly(textDisplayNode->GetAttachEdgeOnly());

  this->UpdatePosition(widget, node);

  // at least one value has changed, so set the widget to modified
  rep->NeedToRenderOn();
  captionWidget->Modified();

  // enable processing of modified events
  this->m_Updating = 0;

}

//---------------------------------------------------------------------------
/// Propagate position properties of MRML node to widget.
void vtkMRMLAnnotationTextDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
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
  vtkCaptionWidget * captionWidget = vtkCaptionWidget::SafeDownCast(widget);

  if (!captionWidget)
   {
   vtkErrorMacro("UpdatePosition: Could not get caption widget!");
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

  vtkCaptionRepresentation * rep = vtkCaptionRepresentation::SafeDownCast(captionWidget->GetRepresentation());
  vtkCaptionActor2D *captionActor = rep->GetCaptionActor2D();

  double worldCoordinates1[4];
  textNode->GetControlPointWorldCoordinates(0, worldCoordinates1);

  double *captionViewportCoordinates = textNode->GetCaptionCoordinates();
  vtkDebugMacro("UpdatePosition: caption coordinates from node = " << captionViewportCoordinates[0] << ", " << captionViewportCoordinates[1]);
  if ((this->Is2DDisplayableManager() && strcmp(this->GetSliceNode()->GetName(), "Red") == 0) || !this->Is2DDisplayableManager())
     {
     vtkDebugMacro("UpdatePosition: caption coordinates from node " << textNode->GetID() << " = "
                   << captionViewportCoordinates[0]
                   << ", " << captionViewportCoordinates[1]
                   << (this->Is2DDisplayableManager() ? "(2D)" : "(3D)"));
     }
  //these two calls should do the same thing, the caption actor is updated
  //from the rep position
  rep->SetPosition(captionViewportCoordinates);
  //captionActor->SetPosition(captionViewportCoordinates[0], captionViewportCoordinates[1]);

   if (this->Is2DDisplayableManager())
     {
     double displayCoordinates1[4];
     this->GetWorldToDisplayCoordinates(worldCoordinates1,displayCoordinates1);

     captionActor->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
     captionActor->GetAttachmentPointCoordinate()->SetValue(displayCoordinates1);
     }
   else
     {
     // 3D case
     rep->SetAnchorPosition(worldCoordinates1);
     }

}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationTextDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
   vtkCaptionWidget * captionWidget = vtkCaptionWidget::SafeDownCast(widget);

   if (!captionWidget)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get text widget!");
     return;
     }

   // cast to the specific mrml node
   vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);

   if (!textNode)
     {
     vtkErrorMacro("PropagateWidgetToMRML: Could not get text node!");
     return;
     }

  // disable processing of modified events
  this->m_Updating = 1;
  textNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and save it to the mrml node
  vtkCaptionRepresentation * rep = vtkCaptionRepresentation::SafeDownCast(captionWidget->GetRepresentation());
  vtkCaptionActor2D *captionActor = rep->GetCaptionActor2D();

  double worldCoordinates1[4];
  double displayCoordinates1[4];

  // for the caption
//  double worldCoordinates2[4];
//  double displayCoordinates2[4];

  bool allowMovement = true;

  if (this->Is2DDisplayableManager())
    {
    if (captionActor)
      {
      captionActor->GetAttachmentPointCoordinate()->SetCoordinateSystemToDisplay();
      captionActor->GetAttachmentPointCoordinate()->GetValue(displayCoordinates1);
      }

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

  // this gets the display coordinate offset between the anchor and the bottom
  // of the caption box
  // GetPosition goes to the BorderRepresentation and returns viewport
  // coordinates, so does GetPosition on the caption actor
  double *captionPosition = rep->GetPosition();

  if (captionPosition)
    {
    vtkDebugMacro("PropagateWidgetToMRML: " << textNode->GetID()
                  << " widget rep caption position viewport = "
                  << captionPosition[0] << ", "
                  << captionPosition[1]);
    if (captionPosition[0] < 0.0 || captionPosition[1] < 0.0 || captionPosition[0] > 1.0 || captionPosition[1] > 1.0)
      {
      vtkErrorMacro("PropagateWidgetToMRML: caption position from widget is not in normalised viewport coordinates: "
                    << captionPosition[0] << ", " << captionPosition[1]);
      }
    else
      {
      textNode->SetCaptionCoordinates(captionPosition);
      }
    }
  // don't update mrml text, it can't be edited in 3D and this may take the
  // line wrapped text and put it into the node
  if (captionActor)
    {
//    textNode->SetText(0,captionActor->GetCaption(),1,1);
    }
  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = textNode->GetAnnotationTextDisplayNode();
  if (!textDisplayNode)
    {
    // no display node yet, create one
    textNode->CreateAnnotationTextDisplayNode();
    textDisplayNode = textNode->GetAnnotationTextDisplayNode();
    if (!textDisplayNode)
      {
      vtkErrorMacro("PropagateWidgetToMRML: error creating a text display node for text node " << textNode->GetID());
      return;
      }
    }

  // update mrml textscale
  if (captionActor)
    {
//    double fontSize = captionActor->GetTextActor()->GetScaledTextProperty()->GetFontSize();
//    std::cout << "WidgetToMRML: fontSize = " << fontSize << std::endl;
//    textDisplayNode->SetTextScale(fontSize);
    }
  // update mrml selected color
  //textDisplayNode->SetSelectedColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());

  // update mrml color
  //textDisplayNode->SetColor(rep->GetTextActor()->GetScaledTextProperty()->GetColor());


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
void vtkMRMLAnnotationTextDisplayableManager::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    return;
    }

  // switch to updating state to avoid events mess
  this->m_Updating = 1;


  double displayCoordinates1[2];
  displayCoordinates1[0] = x;
  displayCoordinates1[1] = y;

  vtkDebugMacro("OnClickInRenderWindow: displaycoordinates1 (anchor) = " << displayCoordinates1[0] << ", " << displayCoordinates1[1]);

  double worldCoordinates1[4];

  this->GetDisplayToWorldCoordinates(displayCoordinates1[0],displayCoordinates1[1],worldCoordinates1);

  this->RestrictDisplayCoordinatesToViewport(displayCoordinates1);

  // create the MRML node
  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::New();
  textNode->SetTextCoordinates(worldCoordinates1);
  double captionCoordinates[2];
  this->BestGuessForNewCaptionCoordinates(captionCoordinates);
  vtkDebugMacro("OnClickInRenderWindow: Best guess for caption coordinates = " << captionCoordinates[0] << ", " << captionCoordinates[1]);
  textNode->SetCaptionCoordinates(captionCoordinates);

  textNode->SetTextLabel("New text");

  textNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("T"));

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
    textNode->SetAttribute("AssociatedNodeID", associatedNodeID);
    }

  textNode->Initialize(this->GetMRMLScene());

  textNode->Delete();

  // reset updating state
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Figure out the best guess at a placement for a new caption coordinate by
//examining the text nodes in the scene
void vtkMRMLAnnotationTextDisplayableManager::BestGuessForNewCaptionCoordinates(double bestGuess[2])
{
  bestGuess[0] = 0.05;
  bestGuess[1] = 0.05;

  if (!this->GetMRMLScene())
    {
    return;
    }

  int numNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLAnnotationTextNode");

  // for now, just hard code the nodes around the viewport
  if (numNodes % 5 == 0)
    {
    bestGuess[0] = 0.8;
    bestGuess[1] = 0.8;
    }
  else if (numNodes % 5 == 1)
    {
    bestGuess[0] = 0.05;
    bestGuess[1] = 0.8;
    }
  else if (numNodes % 5 == 2)
    {
    bestGuess[0] = 0.8;
    bestGuess[1] = 0.05;
    }
  else if (numNodes % 5 == 3)
    {
    bestGuess[0] = 0.2;
    bestGuess[1] = 0.2;
    }
  else if (numNodes % 5 == 4)
    {
    bestGuess[0] = 0.5;
    bestGuess[1] = 0.5;
    }
}
