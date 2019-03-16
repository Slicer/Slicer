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

#include "vtkSlicerCurveRepresentation2D.h"

#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkDiscretizableColorTransferFunction.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPiecewiseFunction.h"
#include "vtkActor2D.h"
#include "vtkAssemblyPath.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkMath.h"
#include "vtkInteractorObserver.h"
#include "vtkLine.h"
#include "vtkCoordinate.h"
#include "vtkGlyph2D.h"
#include "vtkCursor2D.h"
#include "vtkCylinderSource.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkCamera.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"
#include "vtkTubeFilter.h"
#include "vtkStringArray.h"
#include "vtkPlane.h"
#include "vtkVectorText.h"
#include "vtkTextActor.h"
#include "cmath"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkSampleImplicitFunctionFilter.h"

vtkStandardNewMacro(vtkSlicerCurveRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation2D::vtkSlicerCurveRepresentation2D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();

  this->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  this->SlicePlane = vtkSmartPointer<vtkPlane>::New();
  this->SliceDistance->SetImplicitFunction(this->SlicePlane);

  this->WorldToSliceTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransform = vtkSmartPointer<vtkTransform>::New();
  this->WorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->WorldToSliceTransformer->SetInputConnection(this->SliceDistance->GetOutputPort());

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputConnection(this->WorldToSliceTransformer->GetOutputPort());
  this->TubeFilter->SetNumberOfSides(6);
  this->TubeFilter->SetRadius(1);

  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());
  this->LineColorMap = vtkSmartPointer<vtkDiscretizableColorTransferFunction>::New();
  this->LineMapper->SetLookupTable(this->LineColorMap);
  this->LineMapper->SetScalarVisibility(true);

  this->LineActor = vtkSmartPointer<vtkActor2D>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->SliceCurvePointLocator = vtkSmartPointer<vtkCellLocator>::New();
}

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation2D::~vtkSlicerCurveRepresentation2D()
= default;

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->MarkupsDisplayNode
    || !this->MarkupsDisplayNode->GetVisibility()
    || !this->MarkupsDisplayNode->IsDisplayableInView(this->ViewNode->GetID())
    )
  {
    this->VisibilityOff();
    return;
  }
  this->VisibilityOn();

  // Update from slice node
  if (!caller || caller == this->ViewNode.GetPointer())
    {
    vtkMatrix4x4* sliceXYToRAS = this->GetSliceNode()->GetXYToRAS();

    // Update transformation to slice
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(sliceXYToRAS, rasToSliceXY.GetPointer());
    // Project all points to the slice plane (slice Z coordinate = 0)
    rasToSliceXY->SetElement(2, 0, 0);
    rasToSliceXY->SetElement(2, 1, 0);
    rasToSliceXY->SetElement(2, 2, 0);
    this->WorldToSliceTransform->SetMatrix(rasToSliceXY.GetPointer());

    // Update slice plane (for distance computation)
    double normal[3];
    double origin[3];
    const int planeOrientation = 1; // +/-1: orientation of the normal
    for (int i = 0; i < 3; i++)
      {
      normal[i] = planeOrientation * sliceXYToRAS->GetElement(i, 2);
      origin[i] = sliceXYToRAS->GetElement(i, 3);
      }
    vtkMath::Normalize(normal);
    this->SlicePlane->SetNormal(normal);
    this->SlicePlane->SetOrigin(origin);
    this->SlicePlane->Modified();
    }

  // Line display

  this->TubeFilter->SetRadius(this->ViewScaleFactor * this->ControlPointSize * 0.125);

  this->LineActor->SetVisibility(markupsNode->GetNumberOfControlPoints() >= 2);

  bool allControlPointsSelected = this->GetAllControlPointsSelected();
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
  {
    controlPointType = allControlPointsSelected ? Selected : Unselected;
  }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);

  // TODO: use color node or display node properties to set up colormap
  double limit = 10.0;
  double tolerance = 1.0;
  vtkPiecewiseFunction* opacityFunction = this->LineColorMap->GetScalarOpacityFunction();
  if (!opacityFunction)
    {
    opacityFunction = vtkPiecewiseFunction::New();
    this->LineColorMap->SetScalarOpacityFunction(opacityFunction);
    opacityFunction->Delete();
    }
  opacityFunction->RemoveAllPoints();
  opacityFunction->AddPoint(-limit, 0.0);
  opacityFunction->AddPoint(-tolerance, 1.0);
  opacityFunction->AddPoint(tolerance, 1.0);
  opacityFunction->AddPoint(limit, 0.0);
  vtkColorTransferFunction* colorFunction = this->LineColorMap;
  colorFunction->RemoveAllPoints();
  double* lineColorRGB = this->GetControlPointsPipeline(controlPointType)->Property->GetColor();
  colorFunction->AddRGBPoint(-tolerance*2.0, std::min(lineColorRGB[0]*0.5, 1.0), std::min(lineColorRGB[1] * 0.5, 1.0), std::min(lineColorRGB[2] * 2.0, 1.0));
  colorFunction->AddRGBPoint(-tolerance, lineColorRGB[0], lineColorRGB[1], lineColorRGB[2]);
  colorFunction->AddRGBPoint( tolerance, lineColorRGB[0], lineColorRGB[1], lineColorRGB[2]);
  colorFunction->AddRGBPoint( tolerance*2.0, std::min(lineColorRGB[0] * 2.0, 1.0), std::min(lineColorRGB[1] * 0.5, 1.0), std::min(lineColorRGB[2] * 0.5, 1.0));
  this->LineColorMap->SetEnableOpacityMapping(true);
  this->LineColorMap->SetClamping(true);

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
  const int displayPosition[2], const double worldPosition[3],
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1)
  {
    return;
  }
  Superclass::CanInteract(displayPosition, worldPosition, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
  {
    // if mouse is near a control point then select that (ignore the line)
    return;
  }

  this->CanInteractWithCurve(displayPosition, worldPosition, foundComponentType, foundComponentIndex, closestDistance2);
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
  const int vtkNotUsed(displayPosition)[2], const double worldPosition[3],
  int &foundComponentType, int &componentIndex, double &closestDistance2)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 2)
    {
    return;
    }

  this->SliceDistance->Update();
  this->SliceCurvePointLocator->SetDataSet(this->SliceDistance->GetOutput());
  this->SliceCurvePointLocator->Update();

  double closestPointDisplay[3] = { 0.0 };
  vtkIdType cellId = -1;
  int subId = -1;
  double dist2 = VTK_DOUBLE_MAX;
  this->SliceCurvePointLocator->FindClosestPoint(worldPosition, closestPointDisplay, cellId, subId, dist2);

  //if (dist2 < this->ControlPointSize * (1.0 + this->Tolerance))
  if (dist2 < this->MarkupsDisplayNode->GetGlyphScale() * (1.0 + this->Tolerance))
    {
    closestDistance2 = dist2 * this->ViewScaleFactor * this->ViewScaleFactor;
    foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
    componentIndex = markupsNode->GetControlPointIndexFromInterpolatedPointIndex(subId);
    }
}
