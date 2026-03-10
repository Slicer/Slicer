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

  // Share the LineColorMap LUT with the direction arrow mapper (initialized in base class)
  this->LineDirectionArrowPipeline->Mapper->SetLookupTable(this->LineColorMap);
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

  vtkPolyData* curveWorld = markupsNode->GetCurveWorld();
  if (!curveWorld)
  {
    this->VisibilityOff();
    return;
  }

  this->VisibilityOn();

  // Physical line diameter in pixels: absolute (mm->px) or relative to line thickness.
  double lineDiameterPx =
    (this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ? this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel
                                                                                                    : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness());
  double markerSizePx = vtkMRMLMarkupsDisplayNode::LineDirectionMarkerBaseScaleFactor * this->MarkupsDisplayNode->GetLineDirectionMarkerScale() * lineDiameterPx;

  // Line display
  this->TubeFilter->SetRadius(lineDiameterPx * 0.5);

  // Hide the line actor if it doesn't intersect the current slice
  this->SliceDistance->Update();
  vtkPolyData* representation = vtkPolyData::SafeDownCast(this->SliceDistance->GetOutput());
  const char* arrayName = this->SliceDistance->GetScalarArrayName();
  vtkFloatArray* distanceArray = nullptr;
  if (representation)
  {
    distanceArray = vtkFloatArray::SafeDownCast(representation->GetPointData()->GetArray(this->SliceDistance->GetScalarArrayName()));
  }
  bool isRepresentationIntersectingSlice = this->IsRepresentationIntersectingSlice(representation, arrayName);
  this->LineActor->SetVisibility(markupsNode->GetNumberOfDefinedControlPoints(true) == 2 && isRepresentationIntersectingSlice);

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

  vtkColorTransferFunction* colorMap = nullptr;
  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
  {
    // Update the line color mapping from the colorNode stored in the markups display node
    colorMap = this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction();
  }
  else
  {
    // if there is no line color node, build the color mapping from few variables
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdateDistanceColorMap(this->LineColorMap, this->LineActor->GetProperty()->GetColor());
    colorMap = this->LineColorMap;
  }

  this->LineMapper->SetLookupTable(colorMap);
  this->LineDirectionArrowPipeline->Mapper->SetLookupTable(colorMap);

  // Compute fading scalars for the 2D line projection
  this->ComputeIntersectionFadingScalars(curveWorld, this->SlicePlane, this->Line, distanceArray, this->LineSliceIntersectionWorldPoints);

  // Direction markers update (2D)
  if (isRepresentationIntersectingSlice && this->MarkupsDisplayNode->GetLineDirectionVisibility() && this->MarkupsDisplayNode->GetLineDirectionVisibility2D()
      && markupsNode->GetNumberOfDefinedControlPoints(true) == 2)
  {
    // Spacing in mm, relative to cone height (100% = touching).
    double markerSpacingMm = this->MarkupsDisplayNode->GetLineDirectionMarkerSpacingScale() * markerSizePx * this->ViewScaleFactorMmPerPixel;
    double intersectionExclR2 = markerSizePx * markerSizePx * 0.75;
    // Rebuild the sampled world positions/tangents only when spacing or world geometry changed.
    vtkMTimeType curveMTime = curveWorld->GetMTime();
    bool lineDirectionFirstToLastControlPoint = this->MarkupsDisplayNode->GetLineDirectionFirstToLastControlPoint();
    bool updateDirectionMarkers = markerSpacingMm != this->LineDirectionMarkerLastSpacing || curveMTime != this->LineDirectionMarkerLastGeometryMTime
                                  || lineDirectionFirstToLastControlPoint != this->LineDirectionFirstToLastControlPoint;
    if (updateDirectionMarkers)
    {
      vtkMRMLMarkupsNode::BuildLineDirectionMarkers(curveWorld->GetPoints(),
                                                    /*closedCurve=*/false,
                                                    markerSpacingMm,
                                                    this->LineDirectionMarkerCachedWorldPositions,
                                                    this->LineDirectionMarkerCachedWorldTangents,
                                                    lineDirectionFirstToLastControlPoint);
      this->LineDirectionMarkerLastSpacing = markerSpacingMm;
      this->LineDirectionMarkerLastGeometryMTime = curveMTime;
      this->LineDirectionFirstToLastControlPoint = lineDirectionFirstToLastControlPoint;
    }
    vtkMTimeType slicePlaneMTime = this->SlicePlane->GetMTime();
    vtkMTimeType markupsDisplayMTime = this->GetMarkupsDisplayNode()->GetMTime();
    bool hoverUpdate = caller == this->GetMarkupsNode() && event == vtkMRMLDisplayableNode::DisplayModifiedEvent;
    if (updateDirectionMarkers || slicePlaneMTime != this->LineDirectionMarkerLastSlicePlaneMTime || markupsDisplayMTime != this->LineDirectionMarkerLastMarkupsDisplayMTime)
    {
      this->LineDirectionArrowPipeline->Points->Reset();
      this->LineDirectionArrowPipeline->Normals->Reset();
      this->LineDirectionArrowPipeline->SliceDistances->Reset();

      for (vtkIdType pointIndex = 0; pointIndex < this->LineDirectionMarkerCachedWorldPositions->GetNumberOfPoints(); ++pointIndex)
      {
        double worldPos[3];
        this->LineDirectionMarkerCachedWorldPositions->GetPoint(pointIndex, worldPos);

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
        this->LineDirectionMarkerCachedWorldTangents->GetTuple(pointIndex, worldTangent);
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

        // Skip arrows that overlap with slice intersection points (only when intersection points are visible)
        bool tooCloseToIntersection = false;
        if (this->MarkupsDisplayNode->GetLineSliceIntersectionPointVisibility())
        {
          for (vtkIdType intersectionPointIndex = 0; intersectionPointIndex < this->LineSliceIntersectionWorldPoints->GetNumberOfPoints(); ++intersectionPointIndex)
          {
            double isectWorld[3];
            this->LineSliceIntersectionWorldPoints->GetPoint(intersectionPointIndex, isectWorld);
            double isectDisplay[4] = { 0.0 };
            this->GetWorldToDisplayCoordinates(isectWorld, isectDisplay);
            double isectPos[3] = { isectDisplay[0], isectDisplay[1], 0.0 };
            if (vtkMath::Distance2BetweenPoints(arrowPos, isectPos) < intersectionExclR2)
            {
              tooCloseToIntersection = true;
              break;
            }
          }
        }
        if (tooCloseToIntersection)
        {
          continue;
        }

        this->LineDirectionArrowPipeline->Points->InsertNextPoint(arrowPos);
        this->LineDirectionArrowPipeline->Normals->InsertNextTuple(sliceTangent);
        this->LineDirectionArrowPipeline->SliceDistances->InsertNextValue(static_cast<float>(sliceDist));
      }

      this->LineDirectionArrowPipeline->Points->Modified();
      this->LineDirectionArrowPipeline->Normals->Modified();
      this->LineDirectionArrowPipeline->SliceDistances->Modified();
      this->LineDirectionArrowPipeline->PointsPoly->Modified();
      this->LineDirectionArrowPipeline->Glypher->SetScaleFactor(markerSizePx);

      // Update slice intersection point markers (same size as direction arrows, same color LUT)
      this->UpdateSliceIntersectionPointDisplay(markerSizePx, colorMap);

      this->LineDirectionMarkerLastSlicePlaneMTime = slicePlaneMTime;
      this->LineDirectionMarkerLastMarkupsDisplayMTime = markupsDisplayMTime;
    }
    else if (hoverUpdate)
    {
      double fadingEnd = this->MarkupsDisplayNode->GetLineColorFadingEnd();
      double sideOffset = fadingEnd * 0.5;
      if (sideOffset <= 0.0)
      {
        sideOffset = 0.01;
      }

      double enteringColor[3];
      colorMap->GetColor(+sideOffset, enteringColor);
      this->LineSliceIntersectionEnteringPipeline->Property->SetColor(enteringColor);

      double exitingColor[3];
      colorMap->GetColor(-sideOffset, exitingColor);
      this->LineSliceIntersectionExitingPipeline->Property->SetColor(exitingColor);
    }
    this->LineDirectionArrowPipeline->Actor->SetVisibility(this->LineDirectionArrowPipeline->Points->GetNumberOfPoints() > 0);
  }
  else
  {
    this->LineDirectionArrowPipeline->Actor->SetVisibility(false);
    this->LineSliceIntersectionEnteringPipeline->Actor->SetVisibility(false);
    this->LineSliceIntersectionExitingPipeline->Actor->SetVisibility(false);
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
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerLineRepresentation2D::ReleaseGraphicsResources(vtkWindow* win)
{
  this->LineActor->ReleaseGraphicsResources(win);
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
  count += this->Superclass::RenderOpaqueGeometry(viewport);
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
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerLineRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->LineActor->GetVisibility() && this->LineActor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  if (this->Superclass::HasTranslucentPolygonalGeometry())
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
