/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

// VTK includes
#include <vtkActor2D.h>
#include <vtkCellLocator.h>
#include <vtkContourTriangulator.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineFilter.h>
#include <vtkPassThrough.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// Markups VTK widgets includes
#include <vtkSlicerROIRepresentation2D.h>

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsROIDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include <vtkMRMLSliceLogic.h>

vtkStandardNewMacro(vtkSlicerROIRepresentation2D);
//----------------------------------------------------------------------
vtkSlicerROIRepresentation2D::vtkSlicerROIRepresentation2D()
{
  this->ROISource = nullptr;

  this->ROIPipelineInputFilter = vtkSmartPointer<vtkPassThrough>::New();

  this->ROIToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->ROIToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ROIToWorldTransformFilter->SetInputConnection(this->ROIPipelineInputFilter->GetOutputPort());
  this->ROIToWorldTransformFilter->SetTransform(this->ROIToWorldTransform);

  this->ROIOutlineCutter = vtkSmartPointer<vtkCutter>::New();
  this->ROIOutlineCutter->SetInputConnection(this->ROIToWorldTransformFilter->GetOutputPort());
  this->ROIOutlineCutter->SetCutFunction(this->SlicePlane);

  this->ROIOutlineWorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ROIOutlineWorldToSliceTransformFilter->SetInputConnection(this->ROIOutlineCutter->GetOutputPort());
  this->ROIOutlineWorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);

  this->ROIOutlineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ROIOutlineMapper->SetInputConnection(this->ROIOutlineWorldToSliceTransformFilter->GetOutputPort());
  this->ROIOutlineProperty = vtkSmartPointer<vtkProperty2D>::New();
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(Unselected)->Property);
  this->ROIOutlineActor = vtkSmartPointer<vtkActor2D>::New();
  this->ROIOutlineActor->SetMapper(this->ROIOutlineMapper);
  this->ROIOutlineActor->SetProperty(this->ROIOutlineProperty);

  this->ROIIntersectionTriangulator = vtkSmartPointer<vtkContourTriangulator>::New();
  this->ROIIntersectionTriangulator->SetInputConnection(this->ROIOutlineWorldToSliceTransformFilter->GetOutputPort());

  this->ROIMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ROIMapper->SetInputConnection(this->ROIIntersectionTriangulator->GetOutputPort());
  this->ROIProperty = vtkSmartPointer<vtkProperty2D>::New();
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(Unselected)->Property);
  this->ROIActor = vtkSmartPointer<vtkActor2D>::New();
  this->ROIActor->SetMapper(this->ROIMapper);
  this->ROIActor->SetProperty(this->ROIProperty);
}

//----------------------------------------------------------------------
vtkSlicerROIRepresentation2D::~vtkSlicerROIRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::UpdateFromMRMLInternal(vtkMRMLNode* caller,
                                                          unsigned long event,
                                                          void* callData /*=nullptr*/)
{
  Superclass::UpdateFromMRMLInternal(caller, event, callData);

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
  {
    return;
  }

  if (!roiNode || !this->MarkupsDisplayNode || !this->IsDisplayable())
  {
    this->VisibilityOff();
    return;
  }

  switch (roiNode->GetROIType())
  {
    case vtkMRMLMarkupsROINode::ROITypeBox:
    case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
      this->UpdateCubeSourceFromMRML(roiNode);
      break;
    default:
      this->VisibilityOff();
      return;
  }

  this->ROIActor->SetVisibility(roiNode->GetNumberOfControlPoints() > 0
                                && this->MarkupsDisplayNode->GetFillVisibility());
  this->ROIOutlineActor->SetVisibility(roiNode->GetNumberOfControlPoints() > 0
                                       && this->MarkupsDisplayNode->GetOutlineVisibility());

  this->ROIToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsROIDisplayNode::ComponentROI)
  {
    controlPointType = this->GetAllControlPointsSelected() ? vtkSlicerMarkupsWidgetRepresentation::Selected
                                                           : vtkSlicerMarkupsWidgetRepresentation::Unselected;
  }

  double opacity = this->MarkupsDisplayNode->GetOpacity();

  double fillOpacity = opacity * this->MarkupsDisplayNode->GetFillOpacity();
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIProperty->SetOpacity(fillOpacity);

  double outlineOpacity = opacity * this->MarkupsDisplayNode->GetOutlineOpacity();
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIOutlineProperty->SetOpacity(outlineOpacity);

  // Properties label display
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);
  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility() && this->AnyPointVisibilityOnSlice
      && roiNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
  {
    double textPos[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointDisplayPosition(0, textPos);
    this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]), static_cast<int>(textPos[1]));
    this->TextActor->SetVisibility(true);
  }
  else
  {
    this->TextActor->SetVisibility(false);
  }
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::SetROISource(vtkPolyDataAlgorithm* roiSource)
{
  this->ROISource = roiSource;
  if (this->ROISource)
  {
    this->ROIPipelineInputFilter->SetInputConnection(roiSource->GetOutputPort());
  }
  else
  {
    this->ROIPipelineInputFilter->RemoveAllInputConnections(0);
  }
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::UpdateCubeSourceFromMRML(vtkMRMLMarkupsROINode* roiNode)
{
  if (!roiNode)
  {
    return;
  }

  vtkSmartPointer<vtkCubeSource> cubeSource = vtkCubeSource::SafeDownCast(this->ROISource);
  if (!cubeSource)
  {
    cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    this->SetROISource(cubeSource);
  }

  // Add a small amount to each axis so that the ROI is still visible when the slice view is parallel to
  // and intersecting with one of the faces.
  double epsilon = 1e-6;

  vtkMRMLApplicationLogic* applicationLogic = this->GetApplicationLogic();
  vtkMRMLSliceLogic* sliceLogic = applicationLogic ? applicationLogic->GetSliceLogic(this->GetSliceNode()) : nullptr;
  if (sliceLogic)
  {
    // Set the value of epsilon relative to the slice thickness.
    double sliceThicknessMm = sliceLogic->GetLowestVolumeSliceSpacing()[2];
    epsilon = sliceThicknessMm / 10000.0;
  }

  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(sideLengths);
  cubeSource->SetXLength(sideLengths[0] + epsilon);
  cubeSource->SetYLength(sideLengths[1] + epsilon);
  cubeSource->SetZLength(sideLengths[2] + epsilon);
  this->ROIOutlineActor->SetVisibility(vtkMath::Norm(sideLengths) >= epsilon);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::GetActors(vtkPropCollection* pc)
{
  this->ROIActor->GetActors(pc);
  this->ROIOutlineActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::ReleaseGraphicsResources(vtkWindow* win)
{
  this->ROIActor->ReleaseGraphicsResources(win);
  this->ROIOutlineActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerROIRepresentation2D::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
  {
    count += this->ROIActor->RenderOverlay(viewport);
  }
  if (this->ROIOutlineActor->GetVisibility())
  {
    count += this->ROIOutlineActor->RenderOverlay(viewport);
  }
  count += Superclass::RenderOverlay(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerROIRepresentation2D::RenderOpaqueGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
  {
    count += this->ROIActor->RenderOpaqueGeometry(viewport);
  }
  if (this->ROIOutlineActor->GetVisibility())
  {
    count += this->ROIOutlineActor->RenderOpaqueGeometry(viewport);
  }
  count += Superclass::RenderOpaqueGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerROIRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
  {
    count += this->ROIActor->RenderTranslucentPolygonalGeometry(viewport);
  }
  if (this->ROIOutlineActor->GetVisibility())
  {
    count += this->ROIOutlineActor->RenderTranslucentPolygonalGeometry(viewport);
  }
  count += Superclass::RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerROIRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  if (this->ROIActor->GetVisibility() && this->ROIActor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  if (this->ROIOutlineActor->GetVisibility() && this->ROIOutlineActor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------
double* vtkSlicerROIRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::CanInteract(vtkMRMLInteractionEventData* interactionEventData,
                                               int& foundComponentType,
                                               int& foundComponentIndex,
                                               double& closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || !interactionEventData)
  {
    return;
  }

  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
  {
    return;
  }

  this->CanInteractWithROI(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::CanInteractWithROI(vtkMRMLInteractionEventData* interactionEventData,
                                                      int& foundComponentType,
                                                      int& foundComponentIndex,
                                                      double& closestDistance2)
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode || !this->Visibility || !this->ROIActor->GetVisibility() || !this->ROISource)
  {
    return;
  }

  this->ROIOutlineCutter->Update();
  if (this->ROIOutlineCutter->GetOutput() && this->ROIOutlineCutter->GetOutput()->GetNumberOfPoints() == 0)
  {
    return;
  }

  double closestPointWorld[3] = { 0.0, 0.0, 0.0 };
  if (interactionEventData->IsWorldPositionValid())
  {
    const double* worldPosition = interactionEventData->GetWorldPosition();

    double dist2World = VTK_DOUBLE_MAX;

    vtkPolyData* roiSliceIntersection = this->ROIOutlineCutter->GetOutput();
    for (int lineIndex = 0; lineIndex < roiSliceIntersection->GetNumberOfCells(); ++lineIndex)
    {
      vtkLine* line = vtkLine::SafeDownCast(roiSliceIntersection->GetCell(lineIndex));
      if (!line)
      {
        continue;
      }

      double edgePoint0World[3] = { 0.0, 0.0, 0.0 };
      line->GetPoints()->GetPoint(0, edgePoint0World);

      double edgePoint1World[3] = { 0.0, 0.0, 0.0 };
      line->GetPoints()->GetPoint(1, edgePoint1World);

      double t;
      double currentClosestPointWorld[3] = { 0.0, 0.0, 0.0 };
      double currentDist2World =
        vtkLine::DistanceToLine(worldPosition, edgePoint0World, edgePoint1World, t, currentClosestPointWorld);
      if (currentDist2World < dist2World)
      {
        dist2World = currentDist2World;
        closestPointWorld[0] = currentClosestPointWorld[0];
        closestPointWorld[1] = currentClosestPointWorld[1];
        closestPointWorld[2] = currentClosestPointWorld[2];
      }
    }
  }

  double closestPointDisplay[3] = { 0.0, 0.0, 0.0 };
  this->GetWorldToSliceCoordinates(closestPointWorld, closestPointDisplay);

  double pixelTolerance = this->PickingTolerance * this->ScreenScaleFactor;
  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]),
                                 static_cast<double>(displayPosition[1]),
                                 0.0 };
  double dist2Display = vtkMath::Distance2BetweenPoints(displayPosition3, closestPointDisplay);
  if (dist2Display < pixelTolerance * pixelTolerance)
  {
    closestDistance2 = dist2Display;
    foundComponentType = vtkMRMLMarkupsROIDisplayNode::ComponentROI;
    foundComponentIndex = 0;
  }
}
