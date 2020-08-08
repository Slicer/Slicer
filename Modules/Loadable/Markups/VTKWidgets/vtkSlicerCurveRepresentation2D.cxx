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
#include "vtkDiscretizableColorTransferFunction.h"
#include "vtkLine.h"
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
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
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
  this->WorldToSliceTransformer->SetInputConnection(this->SliceDistance->GetOutputPort());

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();

  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->PointMergingOn();
  cleaner->SetInputConnection(this->WorldToSliceTransformer->GetOutputPort());
  this->TubeFilter->SetInputConnection(cleaner->GetOutputPort());
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
}

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation2D::~vtkSlicerCurveRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
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

  // Line display

  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() / this->ViewScaleFactorMmPerPixel : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  this->TubeFilter->SetRadius(diameter * 0.5);

  this->LineActor->SetVisibility(markupsNode->GetNumberOfControlPoints() >= 2);

  bool allControlPointsSelected = this->GetAllControlPointsSelected();
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = allControlPointsSelected ? Selected : Unselected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);

  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
    {
    // Update the line color mapping from the colorNode stored in the markups display node
    this->LineMapper->SetLookupTable(this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction());
    }
  else
    {
    // if there is no line color node, build the color mapping from few varibales
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdateDistanceColorMap(this->LineColorMap, this->LineActor->GetProperty()->GetColor());
    this->LineMapper->SetLookupTable(this->LineColorMap);
    }

  bool allNodesHidden = true;
  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (markupsNode->GetNthControlPointVisibility(controlPointIndex))
      {
      allNodesHidden = false;
      break;
      }
    }

  // Display center position
  // It would be cleaner to use a dedicated actor for this instead of using the control points actors
  // as it would give flexibility in what glyph we use, it would not interfere with active control point display, etc.
  if (this->ClosedLoop && markupsNode->GetNumberOfControlPoints() > 2 && this->CenterVisibilityOnSlice && !allNodesHidden)
    {
    double centerPosWorld[3], centerPosDisplay[3], orient[3] = { 0 };
    markupsNode->GetCenterPosition(centerPosWorld);
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
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
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

  this->CanInteractWithCurve(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->LineActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->LineActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerCurveRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count +=  this->LineActor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerCurveRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  count += this->Superclass::RenderOpaqueGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerCurveRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  if (this->LineActor->GetVisibility())
    {
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
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
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerCurveRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
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
}

//-----------------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode)
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
void vtkSlicerCurveRepresentation2D::CanInteractWithCurve(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &componentIndex, double &closestDistance2)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 2
    || !this->GetVisibility() || !interactionEventData
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
