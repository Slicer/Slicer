/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsFiducialDisplayableManager3D.h"

// MarkupsModule/VTKWidgets includes
#include <vtkMarkupsGlyphSource2D.h>

// MRMLDisplayableManager includes
#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkFollower.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientedPolygonalHandleRepresentation3D.h>
#include <vtkPickingManager.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSeedRepresentation.h>
#include <vtkSphereSource.h>

// STD includes
#include <sstream>
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsFiducialDisplayableManager3D);

//---------------------------------------------------------------------------
// vtkMRMLMarkupsFiducialDisplayableManager3D Callback
/// \ingroup Slicer_QtModules_Markups
class vtkMarkupsFiducialWidgetCallback3D : public vtkCommand
{
public:
  static vtkMarkupsFiducialWidgetCallback3D *New()
  { return new vtkMarkupsFiducialWidgetCallback3D; }

  vtkMarkupsFiducialWidgetCallback3D(){}

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void *callData)
  {
    if (event ==  vtkCommand::PlacePointEvent)
      {
      // std::cout << "Warning: PlacePointEvent not supported" << std::endl;
      }
    else if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {
      // sanity checks
      if (!this->DisplayableManager)
        {
        return;
        }
      if (!this->Node)
        {
        return;
        }
      if (!this->Widget)
        {
        return;
        }
      // sanity checks end
      }

    if (event == vtkCommand::EndInteractionEvent)
      {
      // save the state of the node when done moving, then call
      // PropagateWidgetToMRML to update the node one last time
      if (this->Node->GetScene())
        {
        this->Node->GetScene()->SaveStateForUndo(this->Node);
        }
      this->Node->InvokeEvent(vtkMRMLMarkupsNode::PointEndInteractionEvent, callData);
      }
    // the interaction with the widget ended, now propagate the changes to MRML
    this->DisplayableManager->PropagateWidgetToMRML(this->Widget, this->Node);
  }

  void SetWidget(vtkAbstractWidget *w)
    {
    this->Widget = w;
    }
  void SetNode(vtkMRMLMarkupsNode *n)
    {
    this->Node = n;
    }
  void SetDisplayableManager(vtkMRMLMarkupsDisplayableManager3D * dm)
    {
    this->DisplayableManager = dm;
    }

  vtkAbstractWidget * Widget;
  vtkMRMLMarkupsNode * Node;
  vtkMRMLMarkupsDisplayableManager3D * DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLMarkupsFiducialDisplayableManager3D methods

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->Helper->PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new widget.
vtkAbstractWidget * vtkMRMLMarkupsFiducialDisplayableManager3D::CreateWidget(vtkMRMLMarkupsNode* node)
{
  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("CreateWidget: Could not get fiducial node!")
    return 0;
    }

  vtkMRMLMarkupsDisplayNode *displayNode = fiducialNode->GetMarkupsDisplayNode();

  if (!displayNode)
    {
    // std::cout<<"No DisplayNode!"<<std::endl;
    }

  vtkNew<vtkSeedRepresentation> rep;
  vtkNew<vtkOrientedPolygonalHandleRepresentation3D> handle;

  // default to a starburst glyph, update in propagate mrml to widget
  vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
  glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::StarBurst2D);
  glyphSource->Update();
  glyphSource->SetScale(1.0);
  handle->SetHandle(glyphSource->GetOutput());

  rep->SetHandleRepresentation(handle.GetPointer());

  //seed widget
  vtkSeedWidget * seedWidget = vtkSeedWidget::New();
  seedWidget->CreateDefaultRepresentation();

  seedWidget->SetRepresentation(rep.GetPointer());

  if (this->GetInteractor()->GetPickingManager())
    {
    if (!(this->GetInteractor()->GetPickingManager()->GetEnabled()))
      {
      // Managed picking is on by default on the seed widget, but the interactor
      // will need to have it's picking manager turned on once seed widgets are
      // going to be used to avoid dragging seeds that are behind others.
      // Enabling it before setting the interactor on the seed widget seems to
      // work better with tests of two fiducial lists.
      this->GetInteractor()->GetPickingManager()->EnabledOn();
      }
    }
  seedWidget->SetInteractor(this->GetInteractor());
  seedWidget->SetCurrentRenderer(this->GetRenderer());

  vtkDebugMacro("Fids CreateWidget: Created widget for node " << fiducialNode->GetID() << " with a representation");

  seedWidget->CompleteInteraction();

  return seedWidget;
  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node)
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
  vtkMarkupsFiducialWidgetCallback3D *myCallback = vtkMarkupsFiducialWidgetCallback3D::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialDisplayableManager3D::UpdateNthSeedPositionFromMRML(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode)
{
  if (!pointsNode || !widget)
    {
    return false;
    }
  if (n > pointsNode->GetNumberOfMarkups())
    {
    return false;
    }
  vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
  if (!seedWidget)
    {
    return false;
    }
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  if (!seedRepresentation)
    {
    return false;
    }
  bool positionChanged = false;

  // transform fiducial point using parent transforms
  double fidWorldCoord[4];
  pointsNode->GetMarkupPointWorld(n, 0, fidWorldCoord);

  // for 3d managers, compare world positions
  double seedWorldCoord[4];
  seedRepresentation->GetSeedWorldPosition(n,seedWorldCoord);

  if (this->GetWorldCoordinatesChanged(seedWorldCoord, fidWorldCoord))
    {
    vtkDebugMacro("UpdateNthSeedPositionFromMRML: 3D:"
                  << " world coordinates changed:\n\tseed = "
                  << seedWorldCoord[0] << ", "
                  << seedWorldCoord[1] << ", "
                  << seedWorldCoord[2] << "\n\tfid =  "
                  << fidWorldCoord[0] << ", "
                  << fidWorldCoord[1] << ", "
                  << fidWorldCoord[2]);
    seedRepresentation->GetHandleRepresentation(n)->SetWorldPosition(fidWorldCoord);
    positionChanged = true;
    }
  else
    {
    vtkDebugMacro("UpdateNthSeedPositionFromMRML: 3D: world coordinates unchanged!");
    }

  return positionChanged;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::SetNthSeed(int n, vtkMRMLMarkupsFiducialNode* fiducialNode, vtkSeedWidget *seedWidget)
{
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  if (!seedRepresentation)
    {
    vtkErrorMacro("SetNthSeed: no seed representation in widget!");
    return;
    }

  vtkMRMLMarkupsDisplayNode *displayNode = fiducialNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    vtkDebugMacro("SetNthSeed: Could not get display node for node " << (fiducialNode->GetID() ? fiducialNode->GetID() : "null id"));
    return;
    }

  int numberOfHandles = seedRepresentation->GetNumberOfSeeds();
  vtkDebugMacro("SetNthSeed, n = " << n << ", number of handles = " << numberOfHandles);

  // does this handle need to be created?
  bool createdNewHandle = false;
  if (n >= numberOfHandles)
    {
    // create a new handle
    vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
    if (!newhandle)
      {
      vtkErrorMacro("SetNthSeed: error creaing a new handle!");
      }
    else
      {
      // std::cout << "SetNthSeed: created a new handle,number of seeds = " << seedRepresentation->GetNumberOfSeeds() << std::endl;
      createdNewHandle = true;
      newhandle->ManagesCursorOff();
      if (newhandle->GetEnabled() == 0)
        {
        vtkDebugMacro("turning on the new handle");
        newhandle->EnabledOn();
        }
      }
    }

  // update the postion
  bool positionChanged = this->UpdateNthSeedPositionFromMRML(n, seedWidget, fiducialNode);
  if (!positionChanged)
    {
    vtkDebugMacro("Position did not change");
    }

  vtkOrientedPolygonalHandleRepresentation3D *handleRep =
    vtkOrientedPolygonalHandleRepresentation3D::SafeDownCast(seedRepresentation->GetHandleRepresentation(n));
  if (!handleRep)
    {
    vtkErrorMacro("Failed to get an oriented polygonal handle rep for n = "
          << n << ", number of seeds = "
          << seedRepresentation->GetNumberOfSeeds()
          << ", handle rep = "
          << (seedRepresentation->GetHandleRepresentation(n) ? seedRepresentation->GetHandleRepresentation(n)->GetClassName() : "null"));
    return;
    }

  // update the text
  std::string textString = fiducialNode->GetNthFiducialLabel(n);
  if (textString.compare(handleRep->GetLabelText()) != 0)
    {
    handleRep->SetLabelText(textString.c_str());
    }
  // visibility for this handle, hide it if the whole list is invisible,
  // this fid is invisible, or the list isn't visible in this view
  bool fidVisible = true;
  vtkMRMLViewNode *viewNode = this->GetMRMLViewNode();
  if ((viewNode && displayNode->GetVisibility(viewNode->GetID()) == 0) ||
      displayNode->GetVisibility() == 0 ||
      fiducialNode->GetNthFiducialVisibility(n) == 0)
    {
    fidVisible = false;
    }
  if (fidVisible)
    {
    handleRep->VisibilityOn();
    handleRep->HandleVisibilityOn();
    if (textString.compare("") != 0)
      {
      handleRep->LabelVisibilityOn();
      }
    seedWidget->GetSeed(n)->EnabledOn();
    }
  else
    {
    handleRep->VisibilityOff();
    handleRep->HandleVisibilityOff();
    handleRep->LabelVisibilityOff();
    seedWidget->GetSeed(n)->EnabledOff();
    }

  // update locked
  int listLocked = fiducialNode->GetLocked();
  int seedLocked = fiducialNode->GetNthMarkupLocked(n);
  // if the user is placing lots of fiducials at once, add this one as locked
  // so that it can't be moved when placing the next fiducials. They will be
  // unlocked when the interaction node goes back into ViewTransform
  int persistentPlaceMode = 0;
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    persistentPlaceMode =
      (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      && (interactionNode->GetPlaceModePersistence() == 1);
    }
  if (listLocked || seedLocked || persistentPlaceMode)
    {
    seedWidget->GetSeed(n)->ProcessEventsOff();
    }
  else
    {
    seedWidget->GetSeed(n)->ProcessEventsOn();
    }

  // set the glyph type if a new handle was created, or the glyph type changed
  int oldGlyphType = this->Helper->GetNodeGlyphType(displayNode, n);
  if (createdNewHandle ||
      oldGlyphType != displayNode->GetGlyphType())
    {
    vtkDebugMacro("3D: DisplayNode glyph type = "
          << displayNode->GetGlyphType()
          << " = " << displayNode->GetGlyphTypeAsString()
          << ", is 3d glyph = "
          << (displayNode->GlyphTypeIs3D() ? "true" : "false"));
    if (displayNode->GlyphTypeIs3D())
      {
      if (displayNode->GetGlyphType() == vtkMRMLMarkupsDisplayNode::Sphere3D)
        {
        // std::cout << "3d sphere" << std::endl;
        vtkNew<vtkSphereSource> sphereSource;
        sphereSource->SetRadius(0.5);
        sphereSource->SetPhiResolution(10);
        sphereSource->SetThetaResolution(10);
        sphereSource->Update();
        handleRep->SetHandle(sphereSource->GetOutput());
        }
      else
        {
        // the 3d diamond isn't supported yet, use a 2d diamond for now
        vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
        glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::Diamond2D);
        glyphSource->Update();
        glyphSource->SetScale(1.0);
        handleRep->SetHandle(glyphSource->GetOutput());
        }
      }//if (displayNode->GlyphTypeIs3D())
    else
      {
      // 2D
      vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
      glyphSource->SetGlyphType(displayNode->GetGlyphType());
      glyphSource->Update();
      glyphSource->SetScale(1.0);
      handleRep->SetHandle(glyphSource->GetOutput());
      }
    // TBD: keep with the assumption of one glyph type per markups node,
    // but they may have different glyphs during update
    this->Helper->SetNodeGlyphType(displayNode, displayNode->GetGlyphType(), n);
    }  // end of glyph type

  // update the text display properties if there is text
  if (textString.compare("") != 0)
    {
    // scale the text
    double textscale[3] = {displayNode->GetTextScale(), displayNode->GetTextScale(), displayNode->GetTextScale()};
    handleRep->SetLabelTextScale(textscale);
    if (handleRep->GetLabelTextActor())
      {
      // set the colours
      if (fiducialNode->GetNthFiducialSelected(n))
        {
        double *color = displayNode->GetSelectedColor();
        handleRep->GetLabelTextActor()->GetProperty()->SetColor(color);
        // std::cout << "Set label text actor property color to selected col " << color[0] << ", " << color[1] << ", " << color[2] << std::endl;
        }
      else
        {
        double *color = displayNode->GetColor();
        handleRep->GetLabelTextActor()->GetProperty()->SetColor(color);
        // std::cout << "Set label text actor property color to col " << color[0] << ", " << color[1] << ", " << color[2] << std::endl;
        }

      handleRep->GetLabelTextActor()->GetProperty()->SetOpacity(displayNode->GetOpacity());
      }
    }

  vtkProperty *prop = NULL;
  prop = handleRep->GetProperty();
  if (prop)
    {
    if (fiducialNode->GetNthFiducialSelected(n))
      {
      // use the selected color
      double *color = displayNode->GetSelectedColor();
      prop->SetColor(color);
      // std::cout << "Set glyph property color to selected col " << color[0] << ", " << color[1] << ", " << color[2] << std::endl;
      }
    else
      {
      // use the unselected color
      double *color = displayNode->GetColor();
      prop->SetColor(color);
      // std::cout << "Set glyph property color to col " << color[0] << ", " << color[1] << ", " << color[2] << std::endl;
      }

    // material properties
    prop->SetOpacity(displayNode->GetOpacity());
    prop->SetAmbient(displayNode->GetAmbient());
    prop->SetDiffuse(displayNode->GetDiffuse());
    prop->SetSpecular(displayNode->GetSpecular());
    }

  handleRep->SetUniformScale(displayNode->GetGlyphScale());
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLMarkupsFiducialDisplayableManager3D::PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget)
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
  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get fiducial node!")
    return;
    }

  // disable processing of modified events
  this->Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkMRMLMarkupsDisplayNode *displayNode = fiducialNode->GetMarkupsDisplayNode();

  if (!displayNode)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Could not get display node for node " << (fiducialNode->GetID() ? fiducialNode->GetID() : "null id"));
    }

  // iterate over the fiducials in this markup
  int numberOfFiducials = fiducialNode->GetNumberOfMarkups();

  vtkDebugMacro("Fids PropagateMRMLToWidget, node num markups = " << numberOfFiducials);

  for (int n = 0; n < numberOfFiducials; n++)
    {
    // std::cout << "Fids PropagateMRMLToWidget: n = " << n << std::endl;
    this->SetNthSeed(n, fiducialNode, seedWidget);
    }

  // update lock status
  this->Helper->UpdateLocked(node, this->GetInteractionNode());

  // update visibility of widget as a whole
  // std::cout << "PropagateMRMLToWidget: calling UpdateWidgetVisibility" << std::endl;
  this->UpdateWidgetVisibility(node);

  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  seedRepresentation->NeedToRenderOn();
  seedWidget->Modified();

  // enable processing of modified events
  this->Updating = 0;
}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLMarkupsFiducialDisplayableManager3D::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node)
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

  if (seedWidget->GetWidgetState() != vtkSeedWidget::MovingSeed)
    {
    // ignore events not caused by seed movement
    return;
    }

  // cast to the specific mrml node
  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get fiducial node!")
   return;
   }

  // disable processing of modified events
  this->Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  int numberOfSeeds = seedRepresentation->GetNumberOfSeeds();

  bool positionChanged = false;
  for (int n = 0; n < numberOfSeeds; n++)
    {
    double worldCoordinates1[4];
    seedRepresentation->GetSeedWorldPosition(n,worldCoordinates1);
    vtkDebugMacro("PropagateWidgetToMRML: 3d: widget seed " << n
          << " world coords = " << worldCoordinates1[0] << ", "
          << worldCoordinates1[1] << ", "<< worldCoordinates1[2]);

    // was there a change?
    double currentCoordinates[4];
    fiducialNode->GetNthFiducialWorldCoordinates(n,currentCoordinates);
    vtkDebugMacro("PropagateWidgetToMRML: fiducial " << n
          << " current world coordinates = " << currentCoordinates[0]
          << ", " << currentCoordinates[1] << ", "
          << currentCoordinates[2]);

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
      positionChanged = true;
      vtkDebugMacro("PropagateWidgetToMRML: position changed, setting fiducial coordinates");
      fiducialNode->SetNthFiducialWorldCoordinates(n,worldCoordinates1);
      }
    }

  // did any of the positions change?
  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToMRML: position changed, calling point modified on the fiducial node");
    fiducialNode->Modified();
    fiducialNode->GetScene()->InvokeEvent(vtkMRMLMarkupsNode::PointModifiedEvent,fiducialNode);
    }
  // This displayableManager should now consider ModifiedEvent again
  this->Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a markupsMRMLnode
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnClickInRenderWindow: x = " << x << ", y = " << y << ", incorrect displayable manager, focus = " << this->Focus << ", jumping out");
    return;
    }

  // place the seed where the user clicked
  vtkDebugMacro("OnClickInRenderWindow: placing seed at " << x << ", " << y);
  // switch to updating state to avoid events mess
  this->Updating = 1;

  double displayCoordinates1[2];
  displayCoordinates1[0] = x;
  displayCoordinates1[1] = y;

  double worldCoordinates1[4];

  this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

  // Is there an active markups node that's a fiducial node?
  vtkMRMLMarkupsFiducialNode *activeFiducialNode = NULL;

  vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
  if (selectionNode)
    {
    const char *activeMarkupsID = selectionNode->GetActivePlaceNodeID();
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(activeMarkupsID);
    if (mrmlNode &&
        mrmlNode->IsA("vtkMRMLMarkupsFiducialNode"))
      {
      activeFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
      }
    else
      {
      vtkDebugMacro("OnClickInRenderWindow: active markup id = "
            << (activeMarkupsID ? activeMarkupsID : "null")
            << ", mrml node is "
            << (mrmlNode ? mrmlNode->GetID() : "null")
            << ", not a vtkMRMLMarkupsFiducialNode");
      }
    }

  bool newNode = false;
  if (!activeFiducialNode)
    {
    newNode = true;
    // create the MRML node
    activeFiducialNode = vtkMRMLMarkupsFiducialNode::New();
    activeFiducialNode->SetName("F");
    }

  // add a fiducial: this will trigger an update of the widgets
  int fiducialIndex = activeFiducialNode->AddMarkupWithNPoints(1);
  if (fiducialIndex == -1)
    {
    vtkErrorMacro("OnClickInRenderWindow: unable to add a fiducial to active fiducial list!");
    if (newNode)
      {
      activeFiducialNode->Delete();
      }
    return;
    }
  // set values on it
  activeFiducialNode->SetNthFiducialWorldCoordinates(fiducialIndex,worldCoordinates1);
  // std::cout << "OnClickInRenderWindow: Setting " << fiducialIndex << "th fiducial label from " << activeFiducialNode->GetNthFiducialLabel(fiducialIndex);

  // reset updating state
  this->Updating = 0;

  // if this was a one time place, go back to view transform mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
    {
    vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    }

  // save for undo and add the node to the scene after any reset of the
  // interaction node so that don't end up back in place mode
  this->GetMRMLScene()->SaveStateForUndo();

  // is there a node associated with this?
  if (associatedNodeID)
    {
    activeFiducialNode->SetNthFiducialAssociatedNodeID(fiducialIndex, associatedNodeID);
    }

  if (newNode)
    {
    // create a display node and add node and display node to scene
    vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::New();
    this->GetMRMLScene()->AddNode(displayNode);
    // let the logic know that it needs to set it to defaults
    displayNode->InvokeEvent(vtkMRMLMarkupsDisplayNode::ResetToDefaultsEvent);

    activeFiducialNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    this->GetMRMLScene()->AddNode(activeFiducialNode);

    // have to reset the fid id since the fiducial node generates a scene
    // unique id only if the node was in the scene when the point was added
    if (!activeFiducialNode->ResetNthMarkupID(0))
      {
      vtkWarningMacro("Failed to reset the unique ID on the first fiducial in a new list: " << activeFiducialNode->GetNthMarkupID(0));
      }

    // save it as the active markups list
    if (selectionNode)
      {
      selectionNode->SetActivePlaceNodeID(activeFiducialNode->GetID());
      }
    // clean up
    displayNode->Delete();
    activeFiducialNode->Delete();
    }
}

//---------------------------------------------------------------------------
/// observe key press events
void vtkMRMLMarkupsFiducialDisplayableManager3D::AdditionnalInitializeStep()
{
  // don't add the key press event, as it triggers a crash on start up
  //vtkDebugMacro("Adding an observer on the key press event");
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnInteractorStyleEvent(int eventid)
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
    vtkDebugMacro("OnInteractorStyleEvent 3D: key press event position = "
              << this->GetInteractor()->GetEventPosition()[0] << ", "
              << this->GetInteractor()->GetEventPosition()[1]
              << ", key sym = " << (keySym == NULL ? "null" : keySym));
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
void vtkMRMLMarkupsFiducialDisplayableManager3D::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
{
//  vtkWarningMacro("UpdatePosition, node is " << (node == NULL ? "null" : node->GetID()));
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsNode *pointsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
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

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  bool positionChanged = false;
  int numberOfFiducials = pointsNode->GetNumberOfMarkups();
  for (int n = 0; n < numberOfFiducials; n++)
    {
    if (this->UpdateNthSeedPositionFromMRML(n, seedWidget, pointsNode))
      {
      positionChanged = true;
      }
    }
  // did any of the positions change?
  if (positionChanged && this->Updating == 0)
    {
    // not already updating from propagate mrml to widget, so trigger a render
    vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
    seedRepresentation->NeedToRenderOn();
    seedWidget->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnMRMLSceneEndClose()
{
  // clear out the map of glyph types
  this->Helper->ClearNodeGlyphTypes();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode* node, int n)
{
  int numberOfMarkups = node->GetNumberOfMarkups();
  if (n < 0 || n >= numberOfMarkups)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: n = " << n << " is out of range 0-" << numberOfMarkups);
    return;
    }

  vtkAbstractWidget *widget = this->Helper->GetWidget(node);
  if (!widget)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: a markup was added to a node that doesn't already have a widget! Returning..");
    return;
    }

  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);
  if (!seedWidget)
   {
   vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: Could not get seed widget!")
   return;
   }
  this->SetNthSeed(n, vtkMRMLMarkupsFiducialNode::SafeDownCast(node), seedWidget);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode)
{
  vtkDebugMacro("OnMRMLMarkupsNodeMarkupAddedEvent");

  if (!markupsNode)
    {
    return;
    }
  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (!widget)
    {
    // TBD: create a widget?
    vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: a markup was added to a node that doesn't already have a widget! Returning..");
    return;
    }

  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);
  if (!seedWidget)
   {
   vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: Could not get seed widget!")
   return;
   }

  // this call will create a new handle and set it
  int n = markupsNode->GetNumberOfMarkups() - 1;
  this->SetNthSeed(n, vtkMRMLMarkupsFiducialNode::SafeDownCast(markupsNode), seedWidget);

  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  seedRepresentation->NeedToRenderOn();
  seedWidget->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager3D::OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * markupsNode)
{
  vtkDebugMacro("OnMRMLMarkupsNodeMarkupRemovedEvent");

  if (!markupsNode)
    {
    return;
    }
  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (!widget)
    {
    // TBD: create a widget?
    vtkErrorMacro("OnMRMLMarkupsNodeMarkupRemovedEvent: a markup was removed from a node that doesn't already have a widget! Returning..");
    return;
    }

  // for now, recreate the widget
  this->Helper->RemoveWidgetAndNode(markupsNode);
  this->AddWidget(markupsNode);
}
