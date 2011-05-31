// AnnotationModule includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationFiducialDisplayableManager.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationNode.h"
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLInteractionNode.h>
#include "vtkMRMLAnnotationDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkAnnotationGlyphSource2D.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSeedWidget.h>
#include <vtkHandleRepresentation.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkOrientedPolygonalHandleRepresentation3D.h>
#include <vtkAbstractWidget.h>
#include <vtkMatrix4x4.h>
#include <vtkSphereSource.h>
#include <vtkMath.h>


#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyle.h>
#include "vtkSliceViewInteractorStyle.h"

#include <vtkFollower.h>

// std includes
#include <string>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationFiducialDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationFiducialDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLAnnotationFiducialDisplayableManager Callback
/// \ingroup Slicer_QtModules_Annotation
class vtkAnnotationFiducialWidgetCallback : public vtkCommand
{
public:
  static vtkAnnotationFiducialWidgetCallback *New()
  { return new vtkAnnotationFiducialWidgetCallback; }

  vtkAnnotationFiducialWidgetCallback(){}

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
        vtkSeedRepresentation * representation = vtkSeedRepresentation::SafeDownCast(this->m_Widget->GetRepresentation());

        double displayCoordinates1[4];

        // first, we get the current displayCoordinates of the points
        representation->GetSeedDisplayPosition(0,displayCoordinates1);

        // second, we copy these to restrictedDisplayCoordinates
        double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};

        // modify restrictedDisplayCoordinates 1 and 2, if these are outside the viewport of the current renderer
        this->m_DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);

        // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
        if (this->m_DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
          {
          representation->SetSeedDisplayPosition(0,restrictedDisplayCoordinates1);
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
// vtkMRMLAnnotationFiducialDisplayableManager methods

//---------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new text widget.
vtkAbstractWidget * vtkMRMLAnnotationFiducialDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  // 2d glyphs and text need to be scaled by 1/300 to show up properly in the 2d slice windows
  this->SetScaleFactor2D(0.0033);
  
  vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("CreateWidget: Could not get fiducial node!")
    return 0;
    }

  vtkMRMLAnnotationPointDisplayNode *displayNode = fiducialNode->GetAnnotationPointDisplayNode();

  if (!displayNode)
    {
    std::cout<<"No DisplayNode!"<<std::endl;
    }

  VTK_CREATE(vtkSeedRepresentation, rep);
  VTK_CREATE(vtkOrientedPolygonalHandleRepresentation3D, handle);

  // default to a starburst glyph, update in propagate mrml to widget
  VTK_CREATE(vtkAnnotationGlyphSource2D, glyphSource);
  glyphSource->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::StarBurst2D);
  glyphSource->Update();
  glyphSource->SetScale(1.0);
  handle->SetHandle(glyphSource->GetOutput());

  
  rep->SetHandleRepresentation(handle);


  //seed widget
  vtkSeedWidget * seedWidget = vtkSeedWidget::New();
  seedWidget->CreateDefaultRepresentation();

  seedWidget->SetRepresentation(rep);

  seedWidget->SetInteractor(this->GetInteractor());
  seedWidget->SetCurrentRenderer(this->GetRenderer());

  //seedWidget->ProcessEventsOff();
  
  // create a new handle
  vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
  if (!newhandle)
    {
    vtkErrorMacro("CreateWidget: error creaing a new handle!");
    }

  // init the widget from the mrml node
  this->PropagateMRMLToWidget(fiducialNode, seedWidget);
  
  if (this->GetSliceNode())
    {

    bool showWidget = true;
    showWidget = this->IsWidgetDisplayable(this->GetSliceNode(), node);

    if (showWidget)
      {
      seedWidget->On();
      }

    }
  else
    {

    seedWidget->On();

    }

  seedWidget->CompleteInteraction();

  return seedWidget;

  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLAnnotationFiducialDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
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
  vtkAnnotationFiducialWidgetCallback *myCallback = vtkAnnotationFiducialWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

  // store the current view
  node->SaveView();

}


//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLAnnotationFiducialDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
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
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get seed widget!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get fiducial node!")
    return;
    }

  // disable processing of modified events
  this->m_Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

    
  vtkMRMLAnnotationPointDisplayNode *displayNode = fiducialNode->GetAnnotationPointDisplayNode();

  if (!displayNode)
    {
    vtkErrorMacro("Could not get display Node!")
    }

  vtkOrientedPolygonalHandleRepresentation3D *handleRep = vtkOrientedPolygonalHandleRepresentation3D::SafeDownCast(seedRepresentation->GetHandleRepresentation(0));
  if (handleRep)
    {
    // set the glyph type
    vtkDebugMacro("DisplayNode glyph type = " << displayNode->GetGlyphType() << " = " << displayNode->GetGlyphTypeAsString() << ", is 3d glyph = " << (displayNode->GlyphTypeIs3D() ? "true" : "false") << ", is 2d disp manager = " << this->Is2DDisplayableManager());
    if (displayNode->GlyphTypeIs3D())
      {
      if (this->Is2DDisplayableManager())
        {
        // map the 3d sphere to a filled circle, the 3d diamond to a filled
        // diamond
        VTK_CREATE(vtkAnnotationGlyphSource2D, glyphSource);
        if (displayNode->GetGlyphType() == vtkMRMLAnnotationPointDisplayNode::Sphere3D)
          {
          glyphSource->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Circle2D);
          }
        else if (displayNode->GetGlyphType() == vtkMRMLAnnotationPointDisplayNode::Diamond3D)
          {
          glyphSource->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Diamond2D);
          }
        else
          {
          glyphSource->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::StarBurst2D);
          }
        glyphSource->Update();
        glyphSource->SetScale(1.0);
        handleRep->SetHandle(glyphSource->GetOutput());
        }
      else
        {
        if (displayNode->GetGlyphType() == vtkMRMLAnnotationPointDisplayNode::Sphere3D)
          {
          VTK_CREATE(vtkSphereSource, sphereSource);
          sphereSource->SetRadius(0.5);
          sphereSource->SetPhiResolution(10);
          sphereSource->SetThetaResolution(10);
          sphereSource->Update();
          handleRep->SetHandle(sphereSource->GetOutput());
          }
        else
          {
          // the 3d diamond isn't supported yet, use a 2d diamond for now
          VTK_CREATE(vtkAnnotationGlyphSource2D, glyphSource);
          glyphSource->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Diamond2D);
          glyphSource->Update();
          glyphSource->SetScale(1.0);
          handleRep->SetHandle(glyphSource->GetOutput());
          }
        }
      }
    else
      {
      // 2D
      VTK_CREATE(vtkAnnotationGlyphSource2D, glyphSource);
      glyphSource->SetGlyphType(displayNode->GetGlyphType());
      glyphSource->Update();
      glyphSource->SetScale(1.0);
      handleRep->SetHandle(glyphSource->GetOutput());
      }
    
    if (fiducialNode->GetSelected())
      {
      // use the selected color
      handleRep->GetProperty()->SetColor(displayNode->GetSelectedColor());
      }
    else
      {
      // use the unselected color
      handleRep->GetProperty()->SetColor(displayNode->GetColor());
      }
    // material properties
    handleRep->GetProperty()->SetOpacity(displayNode->GetOpacity());
    handleRep->GetProperty()->SetAmbient(displayNode->GetAmbient());
    handleRep->GetProperty()->SetDiffuse(displayNode->GetDiffuse());
    handleRep->GetProperty()->SetSpecular(displayNode->GetSpecular());
    
//    handleRep->SetHandle(glyphSource->GetOutput());

    // the following check is only needed since we require a different uniform scale depending on 2D and 3D
    if (this->Is2DDisplayableManager())
      {
      
      handleRep->SetUniformScale(displayNode->GetGlyphScale()*this->GetScaleFactor2D());
      
      }
    else
      {
      
      handleRep->SetUniformScale(displayNode->GetGlyphScale());
      
      }

    // update the text
    if (fiducialNode->GetNumberOfTexts() > 0)
      {
      // create a string
      vtkStdString textString;
      for (int i = 0; i < fiducialNode->GetNumberOfTexts(); i++)
        {
        if (i > 0)
          {
          textString.append("\n");
          }
        textString.append(fiducialNode->GetText(i));
        }
      handleRep->SetLabelText(textString.c_str());

      // get the text display node
      vtkMRMLAnnotationTextDisplayNode *textDisplayNode = fiducialNode->GetAnnotationTextDisplayNode();
      if (textDisplayNode)
        {
        // scale the text
        double textscale[3] = {textDisplayNode->GetTextScale(), textDisplayNode->GetTextScale(), textDisplayNode->GetTextScale()};
        if (this->Is2DDisplayableManager())
          {
          // scale it down for the 2d windows
          textscale[0] *= this->GetScaleFactor2D();
          textscale[1] *= this->GetScaleFactor2D();
          textscale[2] *= this->GetScaleFactor2D();
          }
        handleRep->SetLabelTextScale(textscale);
        if (handleRep->GetLabelTextActor())
          {
          // set the colours
          if (fiducialNode->GetSelected())
            {
            handleRep->GetLabelTextActor()->GetProperty()->SetColor(textDisplayNode->GetSelectedColor());
            }        
          else
            {
            handleRep->GetLabelTextActor()->GetProperty()->SetColor(textDisplayNode->GetColor());
            }
          handleRep->GetLabelTextActor()->GetProperty()->SetOpacity(textDisplayNode->GetOpacity());
          }
        }
      handleRep->LabelVisibilityOn();
      }
    else
      {
      handleRep->LabelVisibilityOff();
      }
    }

  // now update the position
  this->UpdatePosition(widget, node);
  
  seedRepresentation->NeedToRenderOn();
  seedWidget->Modified();

  seedWidget->CompleteInteraction();

  // enable processing of modified events
  this->m_Updating = 0;


}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLAnnotationFiducialDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
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
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get seed widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get fiducial node!")
   return;
   }

  // disable processing of modified events
  this->m_Updating = 1;
  fiducialNode->DisableModifiedEventOn();

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  double worldCoordinates1[4];

  if (this->Is2DDisplayableManager())
    {
    // 2D widget was changed

    double displayCoordinates1[4];
    seedRepresentation->GetSeedDisplayPosition(0,displayCoordinates1);
    vtkDebugMacro("PropagateWidgetToMRML: 2d displayable manager: widget display coords = " << displayCoordinates1[0] << ", " << displayCoordinates1[1] << ", " << displayCoordinates1[2]);
    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);
    vtkDebugMacro("PropagateWidgetToMRML: 2d: widget world coords = " << worldCoordinates1[0] << ", " << worldCoordinates1[1] << ", "<< worldCoordinates1[2]);
    }
  else
    {

    seedRepresentation->GetSeedWorldPosition(0,worldCoordinates1);
vtkDebugMacro("PropagateWidgetToMRML: 3d: widget world coords = " << worldCoordinates1[0] << ", " << worldCoordinates1[1] << ", "<< worldCoordinates1[2]);
    }

  // was there a change?
  bool positionChanged = false;
  double *currentCoordinates = fiducialNode->GetFiducialCoordinates();
  if (currentCoordinates)
    {
    vtkDebugMacro("PropagateWidgetToMRML: fiducial current world coordinates = " << currentCoordinates[0] << ", " << currentCoordinates[1] << ", " << currentCoordinates[2]);
    double currentCoords[3];
    currentCoords[0] = currentCoordinates[0];
    currentCoords[1] = currentCoordinates[1];
    currentCoords[2] = currentCoordinates[2];
    double newCoords[3];
    newCoords[0] = worldCoordinates1[0];
    newCoords[1] = worldCoordinates1[1];
    newCoords[2] = worldCoordinates1[2];
    if (this->GetWorldCoordinatesChanged(currentCoords, newCoords))
      {
      vtkDebugMacro("PropagateWidgetToMRML: position changed.");
      positionChanged = true;
      }
    }
  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToMRML: position changed, setting fiducial coordinates");
    fiducialNode->SetFiducialCoordinates(worldCoordinates1);

    fiducialNode->SaveView();
    }
  seedWidget->CompleteInteraction();

  // enable processing of modified events
  fiducialNode->DisableModifiedEventOff();

  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToMRML: position changed, calling modified on the fiducial node");
    fiducialNode->Modified();
    fiducialNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, fiducialNode);
    }
  // This displayableManager should now consider ModifiedEvent again
  this->m_Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a annotationMRMLnode
void vtkMRMLAnnotationFiducialDisplayableManager::OnClickInRenderWindow(double x, double y)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnClickInRenderWindow: x = " << x << ", y = " << y << ", incorrect displayable manager, focus = " << this->m_Focus << ", jumping out");
    return;
    }

  // place the seed where the user clicked
  vtkDebugMacro("OnClickInRenderWindow: placing seed at " << x << ", " << y);
  // switch to updating state to avoid events mess
  this->m_Updating = 1;

  double displayCoordinates1[2];
  displayCoordinates1[0] = x;
  displayCoordinates1[1] = y;


  double worldCoordinates1[4];

  this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

  // create the MRML node
  vtkMRMLAnnotationFiducialNode *fiducialNode = vtkMRMLAnnotationFiducialNode::New();

  fiducialNode->SetFiducialCoordinates(worldCoordinates1);

  fiducialNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationFiducial"));

  fiducialNode->Initialize(this->GetMRMLScene());

  fiducialNode->Delete();

  // reset updating state
  this->m_Updating = 0;

  // if this was a one time place, go back to view transform mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
    {
    vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    }

}

//---------------------------------------------------------------------------
/// observe key press events
void vtkMRMLAnnotationFiducialDisplayableManager::AdditionnalInitializeStep()
{
  // don't add the key press event, as it triggers a crash on start up
  //vtkDebugMacro("Adding an observer on the key press event");
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  this->Superclass::OnInteractorStyleEvent(eventid);

  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }
  
  if (eventid == vtkCommand::KeyPressEvent)
    {
    char *keySym = this->GetInteractor()->GetKeySym();
    vtkDebugMacro("OnInteractorStyleEvent " << (this->Is2DDisplayableManager() ? this->GetSliceNode()->GetName() : "3D") << ": key press event position = " << this->GetInteractor()->GetEventPosition()[0] << ", " << this->GetInteractor()->GetEventPosition()[1] << ", key sym = " << (keySym == NULL ? "null" : keySym));
    if (!keySym)
      {
      return;
      }
    if (strcmp(keySym, "p") == 0)
      {
      if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
        {
        this->OnClickInRenderWindowGetCoordinates();
        }
      else
        {
        vtkDebugMacro("Fiducial DisplayableManager: key press p, but not in Place mode! Returning.");
        return;
        }
      }
    }
  else if (eventid == vtkCommand::KeyReleaseEvent)
    {
    vtkDebugMacro("Got a key release event");
    }  
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialDisplayableManager::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
{
//  vtkWarningMacro("UpdatePosition, node is " << (node == NULL ? "null" : node->GetID()));
  if (!node)
    {
    return;
    }
  vtkMRMLAnnotationControlPointsNode *pointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    vtkErrorMacro("UpdatePosition - Can not access control points node from node with id " << node->GetID());
    return;
    }
  // get the widget
  if (!widget)
    {
    vtkErrorMacro("UpdatePosition: no widget associated with points node " << pointsNode->GetID());
    return;
    }
  // cast to a seed widget
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("UpdatePosition: Could not get seed widget!")
   return;
   }

  // disable processing of modified events
  //this->m_Updating = 1;
  bool positionChanged = false;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  if (this->Is2DDisplayableManager())
    {
    // for 2d managers, compare the display positions
    double displayCoordinates1[4];
    double displayCoordinatesBuffer1[4];

    seedRepresentation->GetSeedDisplayPosition(0,displayCoordinatesBuffer1);

    this->GetWorldToDisplayCoordinates(pointsNode->GetControlPointCoordinates(0),displayCoordinates1);

    if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
      {
      // only update when really changed
      vtkDebugMacro("UpdatePosition: " << this->GetSliceNode()->GetName() << ": display coordinates changed:\n\tseed display = " << displayCoordinatesBuffer1[0] << ", " << displayCoordinatesBuffer1[1] << "\n\tfid display =  " << displayCoordinates1[0] << ", " << displayCoordinates1[1] );
      seedRepresentation->SetSeedDisplayPosition(0,displayCoordinates1);
      positionChanged = true;
      }
    else
      {
      vtkDebugMacro("UpdatePosition: " <<  this->GetSliceNode()->GetName() << ": display coordinates unchanged!");
      }
    }
  else
    {
    // for 3d managers, compare world positions
    double seedWorldCoord[4];
    seedRepresentation->GetSeedWorldPosition(0,seedWorldCoord);
    double *fidWorldCoord = pointsNode->GetControlPointCoordinates(0);
    if (this->GetWorldCoordinatesChanged(seedWorldCoord, fidWorldCoord))
      {
      vtkDebugMacro("UpdatePosition: " << (this->Is2DDisplayableManager() ? this->GetSliceNode()->GetName() : "3D") << ": world coordinates changed:\n\tseed = " << seedWorldCoord[0] << ", " << seedWorldCoord[1] << ", " << seedWorldCoord[2] << "\n\tfid =  " << fidWorldCoord[0] << ", " << fidWorldCoord[1] << ", " << fidWorldCoord[2]);
      seedRepresentation->GetHandleRepresentation(0)->SetWorldPosition(fidWorldCoord);
      positionChanged = true;
      }
    else
      {
      vtkDebugMacro("UpdatePosition: " << (this->Is2DDisplayableManager() ? this->GetSliceNode()->GetName() : "3D") << ": world coordinates unchanged!");
      }
    }
  if (positionChanged && this->m_Updating == 0)
    {
    // not already updating from propagate mrml to widget, so trigger a render
    seedRepresentation->NeedToRenderOn();
    seedWidget->Modified();
    seedWidget->CompleteInteraction();
    }
  // enable processing of modified events
  //this->m_Updating = 0;
}
