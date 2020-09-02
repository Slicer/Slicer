/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

// VTK includes
#include "vtkActor2D.h"
#include "vtkGlyph3D.h"
#include "vtkMatrix4x4.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSlicerLineRepresentation3D.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsDisplayNode.h"

vtkStandardNewMacro(vtkSlicerLineRepresentation3D);

//----------------------------------------------------------------------
vtkSlicerLineRepresentation3D::vtkSlicerLineRepresentation3D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();
  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputData(Line);
  this->TubeFilter->SetNumberOfSides(20);
  this->TubeFilter->SetRadius(1);

  // Mappers
  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  this->LineOccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineOccludedMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  // Actors
  this->LineActor = vtkSmartPointer<vtkActor>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->LineOccludedActor = vtkSmartPointer<vtkActor>::New();
  this->LineOccludedActor->SetMapper(this->LineOccludedMapper);
  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(Unselected)->OccludedProperty);

  this->HideTextActorIfAllPointsOccluded = true;
}

//----------------------------------------------------------------------
vtkSlicerLineRepresentation3D::~vtkSlicerLineRepresentation3D() = default;

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->Superclass::GetActors(pc);
  this->LineActor->GetActors(pc);
  this->LineOccludedActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
  this->LineOccludedActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerLineRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOverlay(viewport);
  if (this->LineActor->GetVisibility())
    {
    count +=  this->LineActor->RenderOverlay(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    count +=  this->LineOccludedActor->RenderOverlay(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerLineRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOpaqueGeometry(viewport);
  if (this->LineActor->GetVisibility())
    {
    double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
      this->MarkupsDisplayNode->GetLineDiameter() : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
    this->TubeFilter->SetRadius(diameter * 0.5);
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    count += this->LineOccludedActor->RenderOpaqueGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerLineRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->LineActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->LineActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->LineOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->LineOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerLineRepresentation3D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->LineActor->GetVisibility() && this->LineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->LineOccludedActor->GetVisibility() && this->LineOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerLineRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors({ this->LineActor });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  // Line geometry

  this->BuildLine(this->Line, false);

  // Line display

  this->UpdateRelativeCoincidentTopologyOffsets(this->LineMapper, this->LineOccludedMapper);

  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  this->TubeFilter->SetRadius(diameter * 0.5);

  this->LineActor->SetVisibility(this->GetAllControlPointsVisible());
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = this->GetAllControlPointsSelected() ? Selected : Unselected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);
  this->LineOccludedActor->SetVisibility(this->MarkupsDisplayNode
    && this->LineActor->GetVisibility()
    && this->MarkupsDisplayNode->GetOccludedVisibility());

  if (markupsNode->GetNumberOfDefinedControlPoints(true) == 2 && this->GetAllControlPointsVisible()
    && this->MarkupsDisplayNode->GetPropertiesLabelVisibility())
    {
    this->TextActor->SetVisibility(true);
    double p1[3] = { 0.0 };
    double p2[3] = { 0.0 };
    markupsNode->GetNthControlPointPositionWorld(0, p1);
    markupsNode->GetNthControlPointPositionWorld(1, p2);
    this->TextActorPositionWorld[0] = (p1[0] + p2[0]) / 2.0;
    this->TextActorPositionWorld[1] = (p1[1] + p2[1]) / 2.0;
    this->TextActorPositionWorld[2] = (p1[2] + p2[2]) / 2.0;
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfDefinedControlPoints(true) < 1
    || !interactionEventData )
    {
    return;
    }
  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    // if mouse is near a control point then select that (ignore the line)
    return;
    }

  this->CanInteractWithLine(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
    {
    os << indent << "Line Visibility: " << this->LineActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Line Visibility: (none)\n";
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::UpdateInteractionPipeline()
{
  if (!this->MarkupsNode || this->MarkupsNode->GetNumberOfDefinedControlPoints(true) < 2)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }
  // Final visibility handled by superclass in vtkSlicerMarkupsWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}
