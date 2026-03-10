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
#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkConeSource.h"
#include "vtkDiscretizableColorTransferFunction.h"
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
#include "vtkSlicerCurveRepresentation2D.h"
#include "vtkTextActor.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLProceduralColorNode.h"

vtkStandardNewMacro(vtkSlicerCurveRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation2D::vtkSlicerCurveRepresentation2D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();

  this->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  this->SliceDistance->SetImplicitFunction(this->SlicePlane);

  this->WorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->WorldToSliceTransformer->SetInputData(this->Line);

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();

  this->SliceCurvePointsCleaner = vtkSmartPointer<vtkCleanPolyData>::New();
  this->SliceCurvePointsCleaner->PointMergingOn();
  this->SliceCurvePointsCleaner->SetInputConnection(this->WorldToSliceTransformer->GetOutputPort());
  this->TubeFilter->SetInputConnection(this->SliceCurvePointsCleaner->GetOutputPort());
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

  this->SliceCurvePointLocator = vtkSmartPointer<vtkCellLocator>::New();

  // Direction marker (arrow) glyph pipeline (2D)
  this->DirectionArrowPoints = vtkSmartPointer<vtkPoints>::New();

  this->DirectionArrowNormals = vtkSmartPointer<vtkDoubleArray>::New();
  this->DirectionArrowNormals->SetNumberOfComponents(3);
  this->DirectionArrowNormals->SetName("Normals");

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

  this->DirectionArrowActor2D = vtkSmartPointer<vtkActor2D>::New();
  this->DirectionArrowActor2D->SetMapper(this->DirectionArrowMapper2D);
  this->DirectionArrowActor2D->GetProperty()->SetColor(1, 0, 0); // Red arrows by default
  this->DirectionArrowActor2D->SetVisibility(false);

  this->DirectionMarkerCachedWorldPositions = vtkSmartPointer<vtkPoints>::New();
  this->DirectionMarkerCachedWorldTangents = vtkSmartPointer<vtkDoubleArray>::New();
  this->DirectionMarkerCachedWorldTangents->SetNumberOfComponents(3);
}

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation2D::~vtkSlicerCurveRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::UpdateFromMRMLInternal(vtkMRMLNode* caller, unsigned long event, void* callData /*=nullptr*/)
{
  Superclass::UpdateFromMRMLInternal(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsCurveNode* markupsNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !this->IsDisplayable())
  {
    this->VisibilityOff();
    return;
  }
  this->VisibilityOn();

  // Direction markers update (2D)
  // Sample in world space then filter by proximity to the current slice plane.
  vtkPolyData* curveWorld = markupsNode->GetCurveWorld();
  if (this->MarkupsDisplayNode->GetLineDirectionVisibility() && this->MarkupsDisplayNode->GetLineDirectionVisibility2D() && curveWorld && curveWorld->GetNumberOfPoints() > 1)
  {
    // Physical line diameter in pixels: absolute (mm→px) or relative to line thickness.
    double lineDiameterPx = (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter
                               ? this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel
                               : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
    double markerSizePx = vtkMRMLMarkupsDisplayNode::DirectionMarkerBaseScaleFactor * this->MarkupsDisplayNode->GetDirectionMarkerScale() * lineDiameterPx;
    // Spacing in mm, relative to cone height (100% = touching).
    double markerSpacingMm = this->MarkupsDisplayNode->GetDirectionMarkerSpacingScale() * markerSizePx * this->ViewScaleFactorMmPerPixel;

    // Rebuild the sampled world positions/tangents only when spacing or curve geometry changed.
    // The projection loop below (world → display) always runs because it depends on slice plane.
    vtkMTimeType curveMTime = curveWorld->GetMTime();
    if (markerSpacingMm != this->DirectionMarkerLastSpacing || curveMTime != this->DirectionMarkerLastCurveMTime)
    {
      vtkMRMLMarkupsNode::BuildDirectionMarkers(
        curveWorld->GetPoints(), this->CurveClosed, markerSpacingMm, this->DirectionMarkerCachedWorldPositions, this->DirectionMarkerCachedWorldTangents);
      this->DirectionMarkerLastSpacing = markerSpacingMm;
      this->DirectionMarkerLastCurveMTime = curveMTime;
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

      // Project the world tangent onto the slice plane using WorldToSliceTransform.
      double worldTangent[3];
      this->DirectionMarkerCachedWorldTangents->GetTuple(i, worldTangent);
      double sliceTangent[3];
      this->WorldToSliceTransform->TransformVector(worldTangent, sliceTangent);
      sliceTangent[2] = 0.0;
      double norm2D = std::sqrt(sliceTangent[0] * sliceTangent[0] + sliceTangent[1] * sliceTangent[1]);
      if (norm2D < 1e-6)
      {
        continue; // tangent is perpendicular to the slice; no meaningful 2D direction
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
    this->DirectionArrowActor2D->GetProperty()->SetColor(this->LineActor->GetProperty()->GetColor());
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

  this->LineActor->SetVisibility(markupsNode->GetNumberOfControlPoints() >= 2);

  // Hide the line actor if it doesn't intersect the current slice
  this->SliceDistance->Update();
  if (!this->IsRepresentationIntersectingSlice(vtkPolyData::SafeDownCast(this->SliceDistance->GetOutput()), this->SliceDistance->GetScalarArrayName()))
  {
    this->LineActor->SetVisibility(false);
  }

  // Compute fading scalars for accurate curve projection on the 2D slice.
  if (this->LineActor->GetVisibility())
  {
    vtkPolyData* worldCurve = markupsNode->GetCurveWorld();
    if (worldCurve && worldCurve->GetNumberOfPoints() >= 2)
    {
      this->ComputeIntersectionFadingScalars(worldCurve, this->SlicePlane, this->Line);
    }
  }

  bool allControlPointsSelected = this->GetAllControlPointsSelected();
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
  {
    controlPointType = allControlPointsSelected ? Selected : Unselected;
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
    // ArrowGlyphMapper2D already holds a pointer to LineColorMap (set in constructor);
    // UpdateDistanceColorMap updated it in place, so no reassignment needed unless external LUT changed.
    this->DirectionArrowMapper2D->SetLookupTable(this->LineColorMap);
  }

  bool allNodesHidden = true;
  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
  {
    if (markupsNode->GetNthControlPointPositionVisibility(controlPointIndex) //
        && (markupsNode->GetNthControlPointVisibility(controlPointIndex)))
    {
      allNodesHidden = false;
      break;
    }
  }

  // Display center position
  // It would be cleaner to use a dedicated actor for this instead of using the control points actors
  // as it would give flexibility in what glyph we use, it would not interfere with active control point display, etc.
  if (this->CurveClosed && markupsNode->GetNumberOfControlPoints() > 2 && this->CenterVisibilityOnSlice && !allNodesHidden)
  {
    double centerPosWorld[3], centerPosDisplay[3], orient[3] = { 0 };
    markupsNode->GetCenterOfRotationWorld(centerPosWorld);
    this->GetWorldToSliceCoordinates(centerPosWorld, centerPosDisplay);
    int centerControlPointType = allControlPointsSelected ? Selected : Unselected;
    if (this->MarkupsDisplayNode->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentCenterPoint)
    {
      centerControlPointType = Active;
      this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->SetNumberOfPoints(0);
      this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);
    }
    this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->InsertNextPoint(centerPosDisplay);
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(orient);

    this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->Modified();
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->Modified();
    if (centerControlPointType == Active)
    {
      this->GetControlPointsPipeline(centerControlPointType)->Actor->VisibilityOn();
      this->GetControlPointsPipeline(centerControlPointType)->LabelsActor->VisibilityOff();
    }
  }

  // Properties label display

  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility()   //
      && this->AnyPointVisibilityOnSlice                         //
      && markupsNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
  {
    int controlPointIndex = 0;
    int numberOfDefinedControlPoints = markupsNode->GetNumberOfDefinedControlPoints(); // excluding previewed point
    if (numberOfDefinedControlPoints > 0)
    {
      // there is at least one placed point
      controlPointIndex = markupsNode->GetNthControlPointIndexByPositionStatus((numberOfDefinedControlPoints - 1) / 2, vtkMRMLMarkupsNode::PositionDefined);
    }
    else
    {
      // we only have a preview point
      controlPointIndex = markupsNode->GetNthControlPointIndexByPositionStatus(0, vtkMRMLMarkupsNode::PositionPreview);
    }
    double textPos[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointDisplayPosition(controlPointIndex, textPos);
    this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]), static_cast<int>(textPos[1]));
    this->TextActor->SetVisibility(true);
  }
  else
  {
    this->TextActor->SetVisibility(false);
  }
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::CanInteract(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1 //
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

  this->CanInteractWithCurve(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::GetActors(vtkPropCollection* pc)
{
  this->LineActor->GetActors(pc);
  this->DirectionArrowActor2D->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::ReleaseGraphicsResources(vtkWindow* win)
{
  this->LineActor->ReleaseGraphicsResources(win);
  this->DirectionArrowActor2D->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerCurveRepresentation2D::RenderOverlay(vtkViewport* viewport)
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
int vtkSlicerCurveRepresentation2D::RenderOpaqueGeometry(vtkViewport* viewport)
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
  count += this->Superclass::RenderOpaqueGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerCurveRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
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
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerCurveRepresentation2D::HasTranslucentPolygonalGeometry()
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
double* vtkSlicerCurveRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
  {
    os << indent << "Line Actor Visibility: " << this->LineActor->GetVisibility() << "\n";
  }
  else
  {
    os << indent << "Line Actor: (none)\n";
  }
  if (this->DirectionArrowActor2D)
  {
    os << indent << "Arrow Glyph Actor 2D Visibility: " << this->DirectionArrowActor2D->GetVisibility() << "\n";
  }
  else
  {
    os << indent << "Arrow Glyph Actor 2D: (none)\n";
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::SetMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
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

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::CanInteractWithCurve(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& componentIndex, double& closestDistance2)
{
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 2 //
      || !this->GetVisibility() || !interactionEventData                                                    //
      || !this->MarkupsDisplayNode)
  {
    return;
  }

  this->SliceDistance->Update();
  // No points to find. Trying to run the locator would result in a crash.
  vtkDataSet* sliceDistanceData = this->SliceDistance->GetOutput();
  if (!sliceDistanceData || sliceDistanceData->GetNumberOfPoints() == 0)
  {
    return;
  }

  this->SliceCurvePointLocator->SetDataSet(this->SliceDistance->GetOutput());
  this->SliceCurvePointLocator->Update();

  double closestPointWorld[3] = { 0.0 };
  vtkIdType cellId = -1;
  int subId = -1;
  double dist2World = VTK_DOUBLE_MAX;
  if (interactionEventData->IsWorldPositionValid())
  {
    const double* worldPosition = interactionEventData->GetWorldPosition();
    this->SliceCurvePointLocator->FindClosestPoint(worldPosition, closestPointWorld, cellId, subId, dist2World);
    double lineVisibilityDistance = this->MarkupsDisplayNode->GetLineColorFadingEnd() * 1.2;
    if (dist2World > lineVisibilityDistance * lineVisibilityDistance)
    {
      // line not visible at this distance
      return;
    }
  }

  double closestPointDisplay[3] = { 0.0 };
  this->GetWorldToSliceCoordinates(closestPointWorld, closestPointDisplay);

  double maxPickingDistanceFromControlPoint2 = this->GetMaximumControlPointPickingDistance2();
  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };
  double dist2 = vtkMath::Distance2BetweenPoints(displayPosition3, closestPointDisplay);
  if (dist2 < maxPickingDistanceFromControlPoint2)
  {
    closestDistance2 = dist2;
    foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
    componentIndex = markupsNode->GetControlPointIndexFromInterpolatedPointIndex(subId);
  }
}
