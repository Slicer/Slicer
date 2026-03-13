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
#include "vtkConeSource.h"
#include "vtkDiscretizableColorTransferFunction.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGlyph2D.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkSampleImplicitFunctionFilter.h"
#include "vtkSlicerLineRepresentation2D.h"
#include "vtkTextActor.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLProceduralColorNode.h"

vtkStandardNewMacro(vtkSlicerLineRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerLineRepresentation2D::vtkSlicerLineRepresentation2D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();

  this->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  this->SliceDistance->SetImplicitFunction(this->SlicePlane);

  this->WorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->WorldToSliceTransformer->SetInputData(this->Line);

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputConnection(this->WorldToSliceTransformer->GetOutputPort());
  this->TubeFilter->SetNumberOfSides(6);
  this->TubeFilter->SetRadius(1);

  this->LineColorMap = vtkSmartPointer<vtkDiscretizableColorTransferFunction>::New();

  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());
  this->LineMapper->SetLookupTable(this->LineColorMap);
  this->LineMapper->SetScalarVisibility(true);

  this->LineActor = vtkSmartPointer<vtkActor2D>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  // Direction marker (arrow) glyph pipeline (2D)
  this->DirectionArrowNormals = vtkSmartPointer<vtkDoubleArray>::New();
  this->DirectionArrowNormals->SetNumberOfComponents(3);
  this->DirectionArrowNormals->SetName("Normals");

  this->DirectionArrowPoints = vtkSmartPointer<vtkPoints>::New();

  this->DirectionArrowPointsPoly = vtkSmartPointer<vtkPolyData>::New();
  this->DirectionArrowPointsPoly->SetPoints(this->DirectionArrowPoints);
  this->DirectionArrowPointsPoly->GetPointData()->AddArray(this->DirectionArrowNormals);
  this->DirectionArrowPointsPoly->GetPointData()->SetActiveNormals("Normals");

  // Signed slice-distance per marker drives per-arrow opacity fading via the line color LUT.
  this->DirectionArrowSliceDistances = vtkSmartPointer<vtkFloatArray>::New();
  this->DirectionArrowSliceDistances->SetName("SliceDistance");
  this->DirectionArrowPointsPoly->GetPointData()->AddArray(this->DirectionArrowSliceDistances);
  this->DirectionArrowPointsPoly->GetPointData()->SetActiveScalars("SliceDistance");

  vtkNew<vtkConeSource> coneSource;
  coneSource->SetHeight(1.0);
  coneSource->SetRadius(0.4);
  coneSource->SetResolution(6);
  coneSource->SetDirection(1, 0, 0);

  this->DirectionGlypher = vtkSmartPointer<vtkGlyph2D>::New();
  this->DirectionGlypher->SetInputData(this->DirectionArrowPointsPoly);
  this->DirectionGlypher->SetSourceConnection(coneSource->GetOutputPort());
  this->DirectionGlypher->SetVectorModeToUseNormal();
  this->DirectionGlypher->OrientOn();
  this->DirectionGlypher->SetScaleModeToDataScalingOff(); // scale only via SetScaleFactor(), not by scalar value
  this->DirectionGlypher->SetScaleFactor(1.0);            // updated dynamically in UpdateFromMRMLInternal

  this->DirectionArrowMapper2D = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->DirectionArrowMapper2D->SetInputConnection(this->DirectionGlypher->GetOutputPort());
  this->DirectionArrowMapper2D->SetLookupTable(this->LineColorMap); // shared with LineMapper; fades arrows like the line
  this->DirectionArrowMapper2D->SetScalarVisibility(true);

  this->DirectionArrowActor2D = vtkSmartPointer<vtkActor2D>::New();
  this->DirectionArrowActor2D->SetMapper(this->DirectionArrowMapper2D);
  this->DirectionArrowActor2D->GetProperty()->SetColor(1, 0, 0); // Red arrows by default
  this->DirectionArrowActor2D->SetVisibility(false);

  this->DirectionMarkerCachedWorldPositions = vtkSmartPointer<vtkPoints>::New();
  this->DirectionMarkerCachedWorldTangents = vtkSmartPointer<vtkDoubleArray>::New();
  this->DirectionMarkerCachedWorldTangents->SetNumberOfComponents(3);
}

//----------------------------------------------------------------------
vtkSlicerLineRepresentation2D::~vtkSlicerLineRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::UpdateFromMRMLInternal(vtkMRMLNode* caller, unsigned long event, void* callData /*=nullptr*/)
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

  // Direction markers update (2D)
  // Build a 2-point world-space polyline from the control points and sample it.
  if (this->MarkupsDisplayNode->GetLineDirectionVisibility() && this->MarkupsDisplayNode->GetLineDirectionVisibility2D() && markupsNode->GetNumberOfDefinedControlPoints(true) == 2)
  {
    double p0World[3], p1World[3];
    markupsNode->GetNthControlPointPositionWorld(0, p0World);
    markupsNode->GetNthControlPointPositionWorld(1, p1World);

    // Physical line diameter in pixels: absolute (mm→px) or relative to line thickness.
    double lineDiameterPx = (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter
                               ? this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel
                               : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
    double markerSizePx = vtkMRMLMarkupsDisplayNode::DirectionMarkerBaseScaleFactor * this->MarkupsDisplayNode->GetDirectionMarkerScale() * lineDiameterPx;
    // Spacing in mm, relative to cone height (100% = touching).
    double markerSpacingMm = this->MarkupsDisplayNode->GetDirectionMarkerSpacingScale() * markerSizePx * this->ViewScaleFactorMmPerPixel;

    // Rebuild the sampled world positions/tangents only when spacing or world geometry changed.
    // The projection loop below (world → display) always runs because it depends on slice plane.
    // Use GetCurveWorld() MTime rather than markupsNode->GetMTime() so that parent transform
    // changes (e.g. middle-click translate) also invalidate the cache.
    vtkPolyData* curveWorld = markupsNode->GetCurveWorld();
    vtkMTimeType worldMTime = curveWorld ? curveWorld->GetMTime() : markupsNode->GetMTime();
    if (markerSpacingMm != this->DirectionMarkerLastSpacing || worldMTime != this->DirectionMarkerLastNodeMTime)
    {
      vtkNew<vtkPoints> lineWorldPoints;
      lineWorldPoints->InsertNextPoint(p0World);
      lineWorldPoints->InsertNextPoint(p1World);
      vtkMRMLMarkupsNode::BuildDirectionMarkers(
        lineWorldPoints, /*closedCurve=*/false, markerSpacingMm, this->DirectionMarkerCachedWorldPositions, this->DirectionMarkerCachedWorldTangents);
      this->DirectionMarkerLastSpacing = markerSpacingMm;
      this->DirectionMarkerLastNodeMTime = worldMTime;
    }

    this->DirectionArrowPoints->Reset();
    this->DirectionArrowNormals->Reset();
    this->DirectionArrowSliceDistances->Reset();

    for (vtkIdType i = 0; i < this->DirectionMarkerCachedWorldPositions->GetNumberOfPoints(); ++i)
    {
      double worldPos[3];
      this->DirectionMarkerCachedWorldPositions->GetPoint(i, worldPos);

      // Signed distance to the slice plane; drives opacity fading through LineColorMap.
      double sliceDist = this->SlicePlane->EvaluateFunction(worldPos);

      double displayPos[4] = { 0.0 };
      this->GetWorldToDisplayCoordinates(worldPos, displayPos);
      // Skip arrows that are completely outside the fading range (avoids projecting far-off markers).
      double limit = this->MarkupsDisplayNode->GetLineColorFadingEnd();
      if (std::abs(sliceDist) > limit)
      {
        continue;
      }

      double worldTangent[3];
      this->DirectionMarkerCachedWorldTangents->GetTuple(i, worldTangent);
      double sliceTangent[3];
      this->WorldToSliceTransform->TransformVector(worldTangent, sliceTangent);
      sliceTangent[2] = 0.0;
      double norm2D = std::sqrt(sliceTangent[0] * sliceTangent[0] + sliceTangent[1] * sliceTangent[1]);
      if (norm2D < 1e-6)
      {
        continue;
      }
      sliceTangent[0] /= norm2D;
      sliceTangent[1] /= norm2D;

      double arrowPos[3] = { displayPos[0], displayPos[1], 0.0 };
      this->DirectionArrowPoints->InsertNextPoint(arrowPos);
      this->DirectionArrowNormals->InsertNextTuple(sliceTangent);
      this->DirectionArrowSliceDistances->InsertNextValue(static_cast<float>(sliceDist));
    }

    this->DirectionArrowPoints->Modified();
    this->DirectionArrowNormals->Modified();
    this->DirectionArrowSliceDistances->Modified();
    this->DirectionArrowPointsPoly->Modified();
    this->DirectionGlypher->SetScaleFactor(markerSizePx);
    this->DirectionArrowActor2D->SetVisibility(this->DirectionArrowPoints->GetNumberOfPoints() > 0);
  }
  else
  {
    this->DirectionArrowActor2D->SetVisibility(false);
  }

  // Line display

  double diameter =
    (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ? this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel
                                                                                                    : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
  this->TubeFilter->SetRadius(diameter * 0.5);

  this->LineActor->SetVisibility(markupsNode->GetNumberOfDefinedControlPoints(true) == 2);

  // Hide the line actor if it doesn't intersect the current slice
  this->SliceDistance->Update();
  if (!this->IsRepresentationIntersectingSlice(vtkPolyData::SafeDownCast(this->SliceDistance->GetOutput()), this->SliceDistance->GetScalarArrayName()))
  {
    this->LineActor->SetVisibility(false);
  }

  // Compute arc-length-from-intersection scalars for accurate line projection.
  // The standard SliceDistance filter only samples perpendicular distance at the curve
  // vertices. For a line (only 2 vertices), this causes incorrect fading because
  // intermediate points near the slice intersection are missing. Here we find the exact
  // intersection of the curve with the slice plane, insert it into the polydata, and
  // compute fading based on arc-length distance along the curve from the nearest
  // intersection point.
  if (this->LineActor->GetVisibility())
  {
    vtkPolyData* worldCurve = markupsNode->GetCurveWorld();
    if (worldCurve && worldCurve->GetNumberOfPoints() >= 2)
    {
      this->ComputeIntersectionFadingScalars(worldCurve, this->SlicePlane, this->Line);
    }
  }

  if (markupsNode->GetNumberOfDefinedControlPoints(true) == 2)
  {
    double p1[3] = { 0.0 };
    double p2[3] = { 0.0 };
    this->GetNthControlPointDisplayPosition(0, p1);
    this->GetNthControlPointDisplayPosition(1, p2);
    double textPos[2] = { (p1[0] + p2[0]) / 2.0, (p1[1] + p2[1]) / 2.0 };
    /*
    l1 = vtkMath::Normalize(vector1);
    l2 = vtkMath::Normalize(vector2);
    length = l1 < l2 ? l1 : l2;
    const double angleTextPlacementRatio = 0.7;
    const double lText = length * angleTextPlacementRatio;
    double vector3[3] = { vector1[0] + vector2[0],
                          vector1[1] + vector2[1],
                          vector1[2] + vector2[2] };
    vtkMath::Normalize(vector3);
    double textPos[3] = { lText * vector3[0] + c[0],
                          lText * vector3[1] + c[1],
                          lText * vector3[2] + c[2] };
                          */
    this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]), static_cast<int>(textPos[1]));
    this->TextActor->SetVisibility(this->MarkupsDisplayNode->GetPropertiesLabelVisibility() //
                                   && this->AnyPointVisibilityOnSlice                       //
                                   && markupsNode->GetNumberOfDefinedControlPoints(true) == 2);
  }
  else
  {
    this->TextActor->SetVisibility(false);
  }

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
  {
    controlPointType = this->GetAllControlPointsSelected() ? Selected : Unselected;
  }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
  {
    // Update the line color mapping from the colorNode stored in the markups display node
    this->LineMapper->SetLookupTable(this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction());
    this->DirectionArrowMapper2D->SetLookupTable(this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction());
  }
  else
  {
    // if there is no line color node, build the color mapping from few variables
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdateDistanceColorMap(this->LineColorMap, this->LineActor->GetProperty()->GetColor());
    this->LineMapper->SetLookupTable(this->LineColorMap);
    // DirectionArrowMapper2D already holds a pointer to LineColorMap (set in constructor);
    // UpdateDistanceColorMap updated it in place, so no reassignment needed unless external LUT changed.
    this->DirectionArrowMapper2D->SetLookupTable(this->LineColorMap);
  }
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::CanInteract(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
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

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::GetActors(vtkPropCollection* pc)
{
  this->LineActor->GetActors(pc);
  this->DirectionArrowActor2D->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::ReleaseGraphicsResources(vtkWindow* win)
{
  this->LineActor->ReleaseGraphicsResources(win);
  this->DirectionArrowActor2D->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerLineRepresentation2D::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;
  if (this->LineActor->GetVisibility())
  {
    count += this->LineActor->RenderOverlay(viewport);
  }
  if (this->DirectionArrowActor2D->GetVisibility())
  {
    count += this->DirectionArrowActor2D->RenderOverlay(viewport);
  }
  count += this->Superclass::RenderOverlay(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerLineRepresentation2D::RenderOpaqueGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->LineActor->GetVisibility())
  {
    count += this->LineActor->RenderOpaqueGeometry(viewport);
  }
  if (this->DirectionArrowActor2D->GetVisibility())
  {
    count += this->DirectionArrowActor2D->RenderOpaqueGeometry(viewport);
  }
  count = this->Superclass::RenderOpaqueGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerLineRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->LineActor->GetVisibility())
  {
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
  }
  if (this->DirectionArrowActor2D->GetVisibility())
  {
    count += this->DirectionArrowActor2D->RenderTranslucentPolygonalGeometry(viewport);
  }
  count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerLineRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  if (this->LineActor->GetVisibility() && this->LineActor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  if (this->DirectionArrowActor2D->GetVisibility() && this->DirectionArrowActor2D->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------
double* vtkSlicerLineRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
  {
    os << indent << "Line Actor Visibility: " << this->LineActor->GetVisibility() << "\n";
  }
  else
  {
    os << indent << "Line Actor: (none)\n";
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::SetMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  if (this->MarkupsNode != markupsNode)
  {
    if (markupsNode)
    {
      this->SliceDistance->SetInputConnection(markupsNode->GetCurveWorldConnection());
    }
    else
    {
      this->SliceDistance->SetInputData(this->Line);
    }
  }
  this->Superclass::SetMarkupsNode(markupsNode);
}
