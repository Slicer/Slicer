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
#include "vtkConeSource.h"
#include "vtkDoubleArray.h"
#include "vtkGlyph3DMapper.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSlicerLineRepresentation3D.h"
#include "vtkTextActor.h"
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

  // Direction marker (arrow) glyph pipeline
  this->DirectionConeSource = vtkSmartPointer<vtkConeSource>::New();
  this->DirectionConeSource->SetHeight(1.0);
  this->DirectionConeSource->SetRadius(0.4);
  this->DirectionConeSource->SetResolution(6);
  this->DirectionConeSource->SetDirection(1, 0, 0);

  this->DirectionArrowNormals = vtkSmartPointer<vtkDoubleArray>::New();
  this->DirectionArrowNormals->SetNumberOfComponents(3);
  this->DirectionArrowNormals->SetName("Normals");

  this->DirectionArrowPoints = vtkSmartPointer<vtkPoints>::New();

  this->DirectionArrowPointsPoly = vtkSmartPointer<vtkPolyData>::New();
  this->DirectionArrowPointsPoly->SetPoints(this->DirectionArrowPoints);
  this->DirectionArrowPointsPoly->GetPointData()->AddArray(this->DirectionArrowNormals);
  this->DirectionArrowPointsPoly->GetPointData()->SetActiveNormals("Normals");

  this->DirectionArrowMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  this->DirectionArrowMapper->SetSourceConnection(this->DirectionConeSource->GetOutputPort());
  this->DirectionArrowMapper->SetInputData(this->DirectionArrowPointsPoly);
  this->DirectionArrowMapper->SetOrientationArray("Normals");
  this->DirectionArrowMapper->SetOrientationModeToDirection();
  this->DirectionArrowMapper->SetScaleModeToNoDataScaling();
  this->DirectionArrowMapper->SetScaleFactor(1.0); // updated dynamically in UpdateFromMRMLInternal

  this->DirectionArrowActor = vtkSmartPointer<vtkActor>::New();
  this->DirectionArrowActor->SetMapper(this->DirectionArrowMapper);
  this->DirectionArrowActor->SetVisibility(false);
}

//----------------------------------------------------------------------
vtkSlicerLineRepresentation3D::~vtkSlicerLineRepresentation3D() = default;

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::GetActors(vtkPropCollection* pc)
{
  this->Superclass::GetActors(pc);
  this->LineActor->GetActors(pc);
  this->LineOccludedActor->GetActors(pc);
  this->DirectionArrowActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::ReleaseGraphicsResources(vtkWindow* win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
  this->LineOccludedActor->ReleaseGraphicsResources(win);
  this->DirectionArrowActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerLineRepresentation3D::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;
  count = this->Superclass::RenderOverlay(viewport);
  if (this->LineActor->GetVisibility())
  {
    count += this->LineActor->RenderOverlay(viewport);
  }
  if (this->LineOccludedActor->GetVisibility())
  {
    count += this->LineOccludedActor->RenderOverlay(viewport);
  }
  if (this->DirectionArrowActor->GetVisibility())
  {
    count += this->DirectionArrowActor->RenderOverlay(viewport);
  }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerLineRepresentation3D::RenderOpaqueGeometry(vtkViewport* viewport)
{
  int count = 0;
  count = this->Superclass::RenderOpaqueGeometry(viewport);

  if (this->LineActor->GetVisibility())
  {
    double diameter =
      (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ? this->MarkupsDisplayNode->GetLineDiameter()
                                                                                                      : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
    this->TubeFilter->SetRadius(diameter * 0.5);
    count += this->LineActor->RenderOpaqueGeometry(viewport);
  }
  if (this->LineOccludedActor->GetVisibility())
  {
    count += this->LineOccludedActor->RenderOpaqueGeometry(viewport);
  }
  if (this->DirectionArrowActor->GetVisibility())
  {
    if (this->MarkupsDisplayNode)
    {
      // ControlPointSize may have been recalculated by the base class due to camera zoom/pan.
      // Propagate the updated size to the direction arrow scale factor immediately.
      double lineDiameter =
        (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ? this->MarkupsDisplayNode->GetLineDiameter()
                                                                                                        : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
      double markerSizeWorld = vtkMRMLMarkupsDisplayNode::DirectionMarkerBaseScaleFactor * this->MarkupsDisplayNode->GetDirectionMarkerScale() * lineDiameter;
      double markerSpacingWorld = this->MarkupsDisplayNode->GetDirectionMarkerSpacingScale() * markerSizeWorld;
      this->DirectionArrowMapper->SetScaleFactor(markerSizeWorld);
      // Rebuild marker positions only when spacing or line geometry changed.
      if (this->Line->GetNumberOfPoints() >= 2)
      {
        vtkMTimeType lineMTime = this->Line->GetMTime();
        if (markerSpacingWorld != this->DirectionMarkerLastSpacing || lineMTime != this->DirectionMarkerLastLineMTime)
        {
          vtkMRMLMarkupsNode::BuildDirectionMarkers(this->Line->GetPoints(), /*closedCurve=*/false, markerSpacingWorld, this->DirectionArrowPoints, this->DirectionArrowNormals);
          this->DirectionArrowPointsPoly->Modified();
          this->DirectionMarkerLastSpacing = markerSpacingWorld;
          this->DirectionMarkerLastLineMTime = lineMTime;
        }
      }
    }
    count += this->DirectionArrowActor->RenderOpaqueGeometry(viewport);
  }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerLineRepresentation3D::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
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
  if (this->DirectionArrowActor->GetVisibility())
  {
    this->DirectionArrowActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->DirectionArrowActor->RenderTranslucentPolygonalGeometry(viewport);
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
  if (this->DirectionArrowActor->GetVisibility() && this->DirectionArrowActor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------
double* vtkSlicerLineRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors({ this->LineActor, this->DirectionArrowActor });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation3D::UpdateFromMRMLInternal(vtkMRMLNode* caller, unsigned long event, void* callData /*=nullptr*/)
{
  Superclass::UpdateFromMRMLInternal(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
  {
    this->VisibilityOff();
    return;
  }

  this->VisibilityOn();

  // Line geometry – only rebuild when the number or position of control points changed
  if (!event || event == vtkMRMLMarkupsNode::PointModifiedEvent || event == vtkMRMLMarkupsNode::PointAddedEvent || event == vtkMRMLMarkupsNode::PointRemovedEvent
      || event == vtkMRMLMarkupsNode::PointPositionDefinedEvent || event == vtkMRMLMarkupsNode::PointPositionUndefinedEvent
      || event == vtkMRMLMarkupsNode::PointPositionMissingEvent || event == vtkMRMLMarkupsNode::PointPositionNonMissingEvent
      || event == vtkMRMLTransformableNode::TransformModifiedEvent)
  {
    this->BuildLine(this->Line, false);
  }

  // Direction markers update
  bool directionMarkersWasModified = false;
  if (this->MarkupsDisplayNode->GetLineDirectionVisibility() && this->MarkupsDisplayNode->GetLineDirectionVisibility3D() && this->Line->GetNumberOfPoints() >= 2)
  {
    double lineDiameter =
      (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ? this->MarkupsDisplayNode->GetLineDiameter()
                                                                                                      : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
    double markerSizeWorld = vtkMRMLMarkupsDisplayNode::DirectionMarkerBaseScaleFactor * this->MarkupsDisplayNode->GetDirectionMarkerScale() * lineDiameter;
    double markerSpacingWorld = this->MarkupsDisplayNode->GetDirectionMarkerSpacingScale() * markerSizeWorld;
    this->DirectionArrowMapper->SetScaleFactor(markerSizeWorld);

    // Only rebuild marker positions when spacing or line geometry actually changed.
    // UpdateFromMRMLInternal is called on mouse hover, selection changes, etc.
    vtkMTimeType lineMTime = this->Line->GetMTime();
    if (markerSpacingWorld != this->DirectionMarkerLastSpacing || lineMTime != this->DirectionMarkerLastLineMTime)
    {
      vtkMRMLMarkupsNode::BuildDirectionMarkers(this->Line->GetPoints(), /*closedCurve=*/false, markerSpacingWorld, this->DirectionArrowPoints, this->DirectionArrowNormals);
      directionMarkersWasModified = true;
      this->DirectionArrowPointsPoly->Modified();
      this->DirectionMarkerLastSpacing = markerSpacingWorld;
      this->DirectionMarkerLastLineMTime = lineMTime;
    }
    this->DirectionArrowActor->SetVisibility(markupsNode->GetNumberOfDefinedControlPoints(true) == 2);
  }
  else
  {
    this->DirectionArrowActor->SetVisibility(false);
  }

  // Line display
  this->UpdateRelativeCoincidentTopologyOffsets(this->LineMapper, this->LineOccludedMapper);
  this->UpdateRelativeCoincidentTopologyOffsets(this->DirectionArrowMapper, nullptr);

  double diameter =
    (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ? this->MarkupsDisplayNode->GetLineDiameter()
                                                                                                    : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
  this->TubeFilter->SetRadius(diameter * 0.5);

  this->LineActor->SetVisibility(markupsNode->GetNumberOfDefinedControlPoints(true) == 2);
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
  {
    controlPointType = this->GetAllControlPointsSelected() ? Selected : Unselected;
  }
  vtkProperty* pipelineProperty = this->GetControlPointsPipeline(controlPointType)->Property;
  this->LineActor->SetProperty(pipelineProperty);
  if (this->DirectionArrowActor->GetVisibility())
  {
    this->DirectionArrowActor->SetProperty(pipelineProperty);
    if (!directionMarkersWasModified)
    {
      this->DirectionArrowPointsPoly->Modified();
    }
  }
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);
  this->LineOccludedActor->SetVisibility(this->MarkupsDisplayNode            //
                                         && this->LineActor->GetVisibility() //
                                         && this->MarkupsDisplayNode->GetOccludedVisibility());

  if (markupsNode->GetNumberOfDefinedControlPoints(true) == 2 && this->MarkupsDisplayNode->GetPropertiesLabelVisibility())
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
void vtkSlicerLineRepresentation3D::CanInteract(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfDefinedControlPoints(true) < 1 //
      || !interactionEventData)
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
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
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
