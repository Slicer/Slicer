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
#include "vtkArcSource.h"
#include "vtkCellLocator.h"
#include "vtkDiscretizableColorTransferFunction.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkSampleImplicitFunctionFilter.h"
#include "vtkSlicerAngleRepresentation2D.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsAngleNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLProceduralColorNode.h"


vtkStandardNewMacro(vtkSlicerAngleRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation2D::vtkSlicerAngleRepresentation2D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();
  this->Arc = vtkSmartPointer<vtkArcSource>::New();
  this->Arc->SetResolution(30);

  this->LineSliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  this->ArcSliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  this->LineSliceDistance->SetImplicitFunction(this->SlicePlane);
  this->ArcSliceDistance->SetImplicitFunction(this->SlicePlane);

  this->LineWorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ArcWorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->LineWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->ArcWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->LineWorldToSliceTransformer->SetInputConnection(this->LineSliceDistance->GetOutputPort());
  this->ArcWorldToSliceTransformer->SetInputConnection(this->ArcSliceDistance->GetOutputPort());

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputConnection(this->LineWorldToSliceTransformer->GetOutputPort());
  this->TubeFilter->SetNumberOfSides(6);
  this->TubeFilter->SetRadius(1);

  this->ArcTubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->ArcTubeFilter->SetInputConnection(this->ArcWorldToSliceTransformer->GetOutputPort());
  this->ArcTubeFilter->SetNumberOfSides(6);
  this->ArcTubeFilter->SetRadius(1);

  this->ColorMap = vtkSmartPointer<vtkDiscretizableColorTransferFunction>::New();

  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());
  this->LineMapper->SetLookupTable(this->ColorMap);
  this->LineMapper->SetScalarVisibility(true);

  this->ArcMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ArcMapper->SetInputConnection(this->ArcTubeFilter->GetOutputPort());
  this->ArcMapper->SetLookupTable(this->ColorMap);
  this->ArcMapper->SetScalarVisibility(true);

  this->LineActor = vtkSmartPointer<vtkActor2D>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->ArcActor = vtkSmartPointer<vtkActor2D>::New();
  this->ArcActor->SetMapper(this->ArcMapper);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);
}

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation2D::~vtkSlicerAngleRepresentation2D() = default;

//----------------------------------------------------------------------
bool vtkSlicerAngleRepresentation2D::GetTransformationReferencePoint(double referencePointWorld[3])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfDefinedControlPoints(true) < 2)
    {
    return false;
    }
  markupsNode->GetNthControlPointPositionWorld(1, referencePointWorld);
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::BuildArc()
{
  vtkMRMLMarkupsAngleNode* markupsNode = vtkMRMLMarkupsAngleNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || markupsNode->GetNumberOfDefinedControlPoints(true) != 3)
    {
    this->TextActor->SetVisibility(false);
    return;
    }

  double angle = markupsNode->GetAngleDegrees();
  bool longArc = (angle > 180.0 || angle < 0.0) && (markupsNode->GetAngleMeasurementMode() == vtkMRMLMarkupsAngleNode::OrientedPositive);

  double p1[3] = {0.0};
  double c[3] = {0.0};
  double p2[3] = {0.0};
  markupsNode->GetNthControlPointPositionWorld(0, p1);
  markupsNode->GetNthControlPointPositionWorld(1, c);
  markupsNode->GetNthControlPointPositionWorld(2, p2);

  // Compute the angle (only if necessary since we don't want
  // fluctuations in angle value as the camera moves, etc.)
  if (((fabs(p1[0] - c[0]) < 0.001) &&
       (fabs(p1[1] - c[1]) < 0.001) &&
       (fabs(p1[2] - c[2]) < 0.001)) ||
      ((fabs(p2[0] - c[0]) < 0.001) &&
       (fabs(p2[1] - c[1]) < 0.001) &&
       (fabs(p2[2] - c[2]) < 0.001)))
    {
    return;
    }

  double vector1[3] = { p1[0] - c[0], p1[1] - c[1], p1[2] - c[2] };
  double vector2[3] = { p2[0] - c[0], p2[1] - c[1], p2[2] - c[2] };
  double l1 = vtkMath::Normalize(vector1);
  double l2 = vtkMath::Normalize(vector2);

  // Place the label and place the arc
  double length = l1 < l2 ? l1 : l2;
  double anglePlacementRatio = 0.5;
  double angleTextPlacementRatio = 0.7;
  if (markupsNode->GetAngleMeasurementMode() == vtkMRMLMarkupsAngleNode::OrientedPositive)
    {
    // Reduce arc size if angle>180deg (it just takes too much space).
    // arcLengthAdjustmentFactor is a sigmoid function that is 1.0 for angle<180 and 0.5 for angle>180,
    // with a smooth transition.
    double arcLengthAdjustmentFactor = 0.5 + 0.5 / (1 + exp((angle - 180.0) * 0.1));
    anglePlacementRatio *= arcLengthAdjustmentFactor;
    angleTextPlacementRatio *= arcLengthAdjustmentFactor;
    }
  const double lArc = length * anglePlacementRatio;
  double arcp1[3] = { lArc * vector1[0] + c[0],
                      lArc * vector1[1] + c[1],
                      lArc * vector1[2] + c[2] };
  double arcp2[3] = { lArc * vector2[0] + c[0],
                      lArc * vector2[1] + c[1],
                      lArc * vector2[2] + c[2] };

  this->Arc->SetPoint1(arcp1);
  this->Arc->SetPoint2(arcp2);
  this->Arc->SetCenter(c);
  this->Arc->SetNegative(longArc);
  this->Arc->Update();

  this->GetNthControlPointDisplayPosition(0, p1);
  this->GetNthControlPointDisplayPosition(1, c);
  this->GetNthControlPointDisplayPosition(2, p2);

  for (int i = 0; i < 3; ++i)
    {
    vector1[i] = p1[i] - c[i];
    vector2[i] = p2[i] - c[i];
    }

  l1 = vtkMath::Normalize(vector1);
  l2 = vtkMath::Normalize(vector2);
  length = l1 < l2 ? l1 : l2;
  const double lText = length * angleTextPlacementRatio;
  double vector3[3] = { vector1[0] + vector2[0],
                        vector1[1] + vector2[1],
                        vector1[2] + vector2[2] };
  vtkMath::Normalize(vector3);
  double textPos[3] = { lText * (longArc ? -1.0 : 1.0) * vector3[0] + c[0],
                        lText * (longArc ? -1.0 : 1.0) * vector3[1] + c[1],
                        lText * (longArc ? -1.0 : 1.0) * vector3[2] + c[2]};

  this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]),
                                      static_cast<int>(textPos[1]));
  this->TextActor->SetVisibility(
    this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && this->AnyPointVisibilityOnSlice
    && markupsNode->GetNumberOfDefinedControlPoints(true) == 3);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
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

  // Update from slice node
  this->BuildArc();

  // Update lines display properties

  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  this->TubeFilter->SetRadius(diameter * 0.5);
  this->ArcTubeFilter->SetRadius(diameter * 0.5);

  int numberOfDefinedControlPoints = markupsNode->GetNumberOfDefinedControlPoints(true);
  this->LineActor->SetVisibility(numberOfDefinedControlPoints >= 2);
  this->ArcActor->SetVisibility(numberOfDefinedControlPoints == 3);

  int controlPointType = Unselected;
  if (this->MarkupsDisplayNode->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = Active;
    }
  else if ((numberOfDefinedControlPoints > 0 && !markupsNode->GetNthControlPointSelected(0)) ||
           (numberOfDefinedControlPoints > 1 && !markupsNode->GetNthControlPointSelected(1)) ||
           (numberOfDefinedControlPoints > 2 && !markupsNode->GetNthControlPointSelected(2)))
    {
    controlPointType = Unselected;
    }
  else
    {
    controlPointType = Selected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
    {
    // Update the line color mapping from the colorNode stored in the markups display node
    vtkColorTransferFunction* colormap = this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction();
    this->LineMapper->SetLookupTable(colormap);
    this->ArcMapper->SetLookupTable(colormap);
    }
  else
    {
    // if there is no line color node, build the color mapping from few variables
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdateDistanceColorMap(this->ColorMap, this->LineActor->GetProperty()->GetColor());
    this->LineMapper->SetLookupTable(this->ColorMap);
    this->ArcMapper->SetLookupTable(this->ColorMap);
    }
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfDefinedControlPoints(true) < 1
    || !this->GetVisibility() || !interactionEventData )
    {
    return;
    }
  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    // if mouse is near a control point then select that (ignore the line)
    return;
    }

  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  double maxPickingDistanceFromControlPoint2 = this->GetMaximumControlPointPickingDistance2();

  vtkIdType numberOfPoints = markupsNode->GetNumberOfDefinedControlPoints(true);

  double pointDisplayPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointDisplayPos2[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos2[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  vtkMatrix4x4::Invert(this->GetSliceNode()->GetXYToRAS(), rasToxyMatrix.GetPointer());
  for (int i = 0; i < numberOfPoints-1; i++)
    {
    if (!this->PointsVisibilityOnSlice->GetValue(i))
      {
      continue;
      }
    if (!this->PointsVisibilityOnSlice->GetValue(i+1))
      {
      i++; // skip one more, as the next iteration would use (i+1)-th point
      continue;
      }
    markupsNode->GetNthControlPointPositionWorld(i, pointWorldPos1);
    rasToxyMatrix->MultiplyPoint(pointWorldPos1, pointDisplayPos1);
    markupsNode->GetNthControlPointPositionWorld(i+1, pointWorldPos2);
    rasToxyMatrix->MultiplyPoint(pointWorldPos2, pointDisplayPos2);

    double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
    double distance2 = vtkLine::DistanceToLine(displayPosition3, pointDisplayPos1, pointDisplayPos2, relativePositionAlongLine);
    if (distance2 < maxPickingDistanceFromControlPoint2 && distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
      foundComponentIndex = i;
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->LineActor->GetActors(pc);
  this->ArcActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->LineActor->ReleaseGraphicsResources(win);
  this->ArcActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerAngleRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count +=  this->LineActor->RenderOverlay(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    count +=  this->ArcActor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerAngleRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    count += this->ArcActor->RenderOpaqueGeometry(viewport);
    }
  count += this->Superclass::RenderOpaqueGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerAngleRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    count += this->ArcActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerAngleRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->LineActor->GetVisibility() && this->LineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ArcActor->GetVisibility() && this->ArcActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerAngleRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
    {
    os << indent << "Line Actor Visibility: " << this->LineActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Line Actor: (none)\n";
    }

  if (this->ArcActor)
    {
    os << indent << "Arc Visibility: " << this->ArcActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Arc Visibility: (none)\n";
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode)
{
  if (this->MarkupsNode != markupsNode)
    {
    if (markupsNode)
      {
      this->LineSliceDistance->SetInputConnection(markupsNode->GetCurveWorldConnection());
      this->ArcSliceDistance->SetInputConnection(this->Arc->GetOutputPort());
      }
    else
      {
      this->LineSliceDistance->SetInputData(this->Line);
      this->ArcSliceDistance->SetInputConnection(this->Arc->GetOutputPort());
      }
    }
  this->Superclass::SetMarkupsNode(markupsNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::UpdateInteractionPipeline()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfDefinedControlPoints(true) < 3)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  Superclass::UpdateInteractionPipeline();
}
