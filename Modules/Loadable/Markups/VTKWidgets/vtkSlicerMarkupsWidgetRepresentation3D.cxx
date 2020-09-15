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
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkLabelPlacementMapper.h"
#include "vtkLine.h"
#include "vtkGlyph3D.h"
#include "vtkMarkupsGlyphSource2D.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkSlicerMarkupsWidgetRepresentation3D.h"
#include "vtkSphereSource.h"
#include "vtkStringArray.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

// MRML includes
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLViewNode.h>

vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D::ControlPointsPipeline3D()
{
  this->Glypher = vtkSmartPointer<vtkGlyph3D>::New();
  this->Glypher->SetInputData(this->ControlPointsPolyData);
  this->Glypher->SetVectorModeToFollowCameraDirection();
  this->Glypher->OrientOn();
  this->Glypher->ScalingOn();
  this->Glypher->SetScaleModeToDataScalingOff();
  this->Glypher->SetScaleFactor(1.0);
  // By default the Points are rendered as spheres
  this->Glypher->SetSourceConnection(this->GlyphSourceSphere->GetOutputPort());

  // Properties
  this->Property = vtkSmartPointer<vtkProperty>::New();
  this->Property->SetRepresentationToSurface();
  this->Property->SetColor(0.4, 1.0, 1.0);
  this->Property->SetAmbient(0.0);
  this->Property->SetDiffuse(1.0);
  this->Property->SetSpecular(0.0);
  this->Property->SetShading(true);
  this->Property->SetSpecularPower(1.0);
  this->Property->SetPointSize(3.);
  this->Property->SetLineWidth(3.);
  this->Property->SetOpacity(1.);

  this->OccludedProperty = vtkSmartPointer<vtkProperty>::New();
  this->OccludedProperty->DeepCopy(this->Property);
  this->OccludedProperty->SetOpacity(0.0);

  this->OccludedTextProperty = vtkSmartPointer<vtkTextProperty>::New();
  this->OccludedTextProperty->ShallowCopy(this->TextProperty);
  this->OccludedTextProperty->SetOpacity(0.0);

  // Label point filters
  this->ControlPointIndices = vtkSmartPointer<vtkIdTypeArray>::New();
  this->ControlPointIndices->SetName("controlPointIndices");
  this->ControlPointIndices->Allocate(100);
  this->ControlPointIndices->SetNumberOfValues(1);
  this->ControlPointIndices->SetValue(0, 0);
  this->LabelControlPointsPolyData->GetPointData()->AddArray(this->ControlPointIndices);

  this->SelectVisiblePoints = vtkSmartPointer<vtkSelectVisiblePoints>::New();
  this->SelectVisiblePoints->SetInputData(this->LabelControlPointsPolyData);
  this->SelectVisiblePoints->SetTolerance(0.0); // we will set tolerance in world coordinate system
  this->SelectVisiblePoints->SetOutput(vtkNew<vtkPolyData>());

  // The SelectVisiblePoints filter should not be added to any pipeline with SetInputConnection.
  // Updates to SelectVisiblePoints must only happen at the start of the RenderOverlay function.
  this->PointSetToLabelHierarchyFilter->SetInputData(this->SelectVisiblePoints->GetOutput());

  this->OccludedPointSetToLabelHierarchyFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  this->OccludedPointSetToLabelHierarchyFilter->SetTextProperty(this->OccludedTextProperty);
  this->OccludedPointSetToLabelHierarchyFilter->SetLabelArrayName("labels");
  this->OccludedPointSetToLabelHierarchyFilter->SetPriorityArrayName("priority");
  this->OccludedPointSetToLabelHierarchyFilter->SetInputData(this->LabelControlPointsPolyData);

  // Mappers
  this->Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Mapper->SetInputConnection(this->Glypher->GetOutputPort());
  // This turns on resolve coincident topology for everything
  // as it is a class static on the mapper
  vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
  this->Mapper->ScalarVisibilityOff();

  this->OccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->OccludedMapper->SetInputConnection(this->Glypher->GetOutputPort());
  this->OccludedMapper->ScalarVisibilityOff();

  this->LabelsMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  this->LabelsMapper->SetInputConnection(this->PointSetToLabelHierarchyFilter->GetOutputPort());
  this->LabelsMapper->PlaceAllLabelsOn();
  /*
  We could consider showing labels with semi-transparent background:
  this->LabelsMapper->SetShapeToRect();
  this->LabelsMapper->SetStyleToFilled();
  this->LabelsMapper->SetBackgroundOpacity(0.4);
  */

  this->LabelsOccludedMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  this->LabelsOccludedMapper->SetInputConnection(this->OccludedPointSetToLabelHierarchyFilter->GetOutputPort());
  this->LabelsOccludedMapper->PlaceAllLabelsOn();

  // Actors
  this->Actor = vtkSmartPointer<vtkActor>::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetProperty(this->Property);

  this->OccludedActor = vtkSmartPointer<vtkActor>::New();
  this->OccludedActor->SetMapper(this->OccludedMapper);
  this->OccludedActor->SetProperty(this->OccludedProperty);

  this->LabelsActor = vtkSmartPointer<vtkActor2D>::New();
  this->LabelsActor->SetMapper(this->LabelsMapper);
  this->LabelsActor->PickableOff();
  this->LabelsActor->DragableOff();

  this->LabelsOccludedActor = vtkSmartPointer<vtkActor2D>::New();
  this->LabelsOccludedActor->SetMapper(this->LabelsOccludedMapper);
  this->LabelsOccludedActor->PickableOff();
  this->LabelsOccludedActor->DragableOff();
};

vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D::~ControlPointsPipeline3D() = default;

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation3D::vtkSlicerMarkupsWidgetRepresentation3D()
{
  for (int i = 0; i<NumberOfControlPointTypes; i++)
    {
    this->ControlPoints[i] = new ControlPointsPipeline3D;
    }

  this->ControlPoints[Selected]->TextProperty->SetColor(1.0, 0.5, 0.5);
  reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Selected])->Property->SetColor(1.0, 0.5, 0.5);

  this->ControlPoints[Active]->TextProperty->SetColor(0.4, 1.0, 0.); // bright green
  reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Active])->Property->SetColor(0.4, 1.0, 0.);
  reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Active])->Actor->PickableOff();
  reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Active])->Actor->DragableOff();

  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(Unselected)->TextProperty);
  this->TextActorPositionWorld[0] = 0.0;
  this->TextActorPositionWorld[1] = 0.0;
  this->TextActorPositionWorld[2] = 0.0;
  this->TextActorOccluded = false;
  // this requires computation of point occlusion, which is expensive if there are thousands of control points,
  // so disable by default
  this->HideTextActorIfAllPointsOccluded = false;

  this->ControlPointSize = 10; // will be set from the markup's GlyphScale

  this->AccuratePicker = vtkSmartPointer<vtkCellPicker>::New();
  this->AccuratePicker->SetTolerance(.005);

  // Using the minimum value of -65000 creates a lot of rendering artifacts on the occluded objects, as all of the
  // pixels in the occluded object will have the same depth buffer value (0.0).
  // Using a default value of -25000 strikes a balance between rendering the occluded objects on top of other objects,
  // while still providing enough leeway to ensure that occluded actors are rendered correctly relative to themselves
  // and to other occluded actors.
  this->OccludedRelativeOffset = -25000;
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation3D::~vtkSlicerMarkupsWidgetRepresentation3D() = default;

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateNthPointAndLabelFromMRML(int vtkNotUsed(n))
{
  /*
  TODO: implement this for better performance
  if (markupsNode->GetNumberOfControlPoints() - 1 >= this->PointsVisibilityOnSlice->GetNumberOfValues())
    {
    this->PointsVisibilityOnSlice->InsertValue(markupsNode->GetNumberOfControlPoints() - 1, 1);
    }
  else
    {
    this->PointsVisibilityOnSlice->InsertNextValue(1);
    }
  this->UpdateInterpolatedPoints(markupsNode->GetNumberOfControlPoints() - 1);
  */
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateAllPointsAndLabelsFromMRML()
{
  if (!this->MarkupsDisplayNode)
    {
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  int numPoints = markupsNode->GetNumberOfControlPoints();
  std::vector<int> activeControlPointIndices;
  this->MarkupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[controlPointType]);

    if (controlPointType == Project || controlPointType == ProjectBack)
      {
      // no projection display in 3D
      controlPoints->Actor->SetVisibility(false);
      controlPoints->OccludedActor->SetVisibility(false);
      controlPoints->LabelsActor->SetVisibility(false);
      controlPoints->LabelsOccludedActor->SetVisibility(false);
      continue;
      }

    this->UpdateRelativeCoincidentTopologyOffsets(controlPoints->Mapper, controlPoints->OccludedMapper);

    controlPoints->Glypher->SetScaleFactor(this->ControlPointSize);

    controlPoints->ControlPoints->SetNumberOfPoints(0);
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);

    controlPoints->LabelControlPoints->SetNumberOfPoints(0);
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);

    controlPoints->Labels->SetNumberOfValues(0);
    controlPoints->LabelsPriority->SetNumberOfValues(0);
    controlPoints->ControlPointIndices->SetNumberOfValues(0);

    for (int pointIndex = 0; pointIndex < numPoints; ++pointIndex)
      {
      if (!markupsNode->GetNthControlPointVisibility(pointIndex))
        {
        continue;
        }
      bool isPointActive = std::find(activeControlPointIndices.begin(), activeControlPointIndices.end(), pointIndex) != activeControlPointIndices.end();
      switch (controlPointType)
        {
        case Active: if (!isPointActive) continue; break;
        case Unselected: if (isPointActive || markupsNode->GetNthControlPointSelected(pointIndex)) continue; break;
        case Selected: if (isPointActive || !markupsNode->GetNthControlPointSelected(pointIndex)) continue; break;
        }

      double worldPos[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNthControlPointPositionWorld(pointIndex, worldPos);
      double pointNormalWorld[3] = { 0.0, 0.0, 1.0 };
      markupsNode->GetNthControlPointNormalWorld(pointIndex, pointNormalWorld);

      controlPoints->ControlPoints->InsertNextPoint(worldPos);

      /* No offset for 3D actors - we may revisit this in the future
      (we could also use text margins to add some space).
      worldPos[0] += this->ControlPointSize;
      worldPos[1] += this->ControlPointSize;
      worldPos[2] += this->ControlPointSize;
      */
      controlPoints->LabelControlPoints->InsertNextPoint(worldPos);
      controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(pointNormalWorld);
      controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(pointNormalWorld);
      controlPoints->Labels->InsertNextValue(markupsNode->GetNthControlPointLabel(pointIndex));
      controlPoints->LabelsPriority->InsertNextValue(std::to_string(pointIndex));
      controlPoints->ControlPointIndices->InsertNextValue(pointIndex);
      }

    if (controlPoints->ControlPointIndices->GetNumberOfValues() > 0)
      {
      controlPoints->ControlPoints->Modified();
      controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
      controlPoints->ControlPointsPolyData->Modified();

      controlPoints->LabelControlPoints->Modified();
      controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->Modified();
      controlPoints->LabelControlPointsPolyData->Modified();

      controlPoints->Actor->SetVisibility(true);
      controlPoints->OccludedActor->SetVisibility(controlPoints->Actor->GetVisibility()
        && this->MarkupsDisplayNode->GetOccludedVisibility() && this->MarkupsDisplayNode->GetOccludedOpacity() > 0.0);
      // For backward compatibility, we hide labels if text scale is set to 0.
      controlPoints->LabelsActor->SetVisibility(this->MarkupsDisplayNode->GetPointLabelsVisibility()
        && this->MarkupsDisplayNode->GetTextScale() > 0.0);
      controlPoints->LabelsOccludedActor->SetVisibility(controlPoints->LabelsActor->GetVisibility()
        && this->MarkupsDisplayNode->GetOccludedVisibility() && this->MarkupsDisplayNode->GetOccludedOpacity() > 0.0);
      }
    else
      {
      controlPoints->Actor->SetVisibility(false);
      controlPoints->OccludedActor->SetVisibility(false);
      controlPoints->LabelsActor->SetVisibility(false);
      controlPoints->LabelsOccludedActor->SetVisibility(false);
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
    || !this->GetVisibility() || !interactionEventData )
    {
    return;
    }

  double displayPosition3[3] = { 0.0, 0.0, 0.0 };
  // Display position is valid in case of desktop interactions. Otherwise it is a 3D only context such as
  // virtual reality, and then we expect a valid world position in the absence of display position.
  if (interactionEventData->IsDisplayPositionValid())
    {
    const int* displayPosition = interactionEventData->GetDisplayPosition();
    displayPosition3[0] = static_cast<double>(displayPosition[0]);
    displayPosition3[1] = static_cast<double>(displayPosition[1]);
    }
  else if (!interactionEventData->IsWorldPositionValid())
    {
    return;
    }

  closestDistance2 = VTK_DOUBLE_MAX; // in display coordinate system (phyisical in case of virtual reality renderer)
  foundComponentIndex = -1;

  // We can interact with the handle if the mouse is hovering over one of the handles (translation or rotation), in display coordinates.
  // If display coordinates for the interaction event are not valid, world coordinates will be checked instead.
  this->CanInteractWithHandles(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    // if mouse is near a handle then select that (ignore the line + control points)
    return;
    }

  if (markupsNode->GetNumberOfControlPoints() > 2 && this->ClosedLoop)
    {
    // Check if center is selected
    double centerPosWorld[3], centerPosDisplay[3];
    markupsNode->GetCenterPosition(centerPosWorld);
    if (interactionEventData->IsDisplayPositionValid())
      {
      double pixelTolerance = this->ControlPointSize / 2.0 / this->GetViewScaleFactorAtPosition(centerPosWorld)
        + this->PickingTolerance * this->ScreenScaleFactor;
      this->Renderer->SetWorldPoint(centerPosWorld);
      this->Renderer->WorldToDisplay();
      this->Renderer->GetDisplayPoint(centerPosDisplay);
      centerPosDisplay[2] = 0.0;
      double dist2 = vtkMath::Distance2BetweenPoints(centerPosDisplay, displayPosition3);
      if (dist2 < pixelTolerance * pixelTolerance)
        {
        closestDistance2 = dist2;
        foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentCenterPoint;
        foundComponentIndex = 0;
        }
      }
    else
      {
      const double* worldPosition = interactionEventData->GetWorldPosition();
      double worldTolerance = this->ControlPointSize / 2.0 +
        this->PickingTolerance / interactionEventData->GetWorldToPhysicalScale();
      double dist2 = vtkMath::Distance2BetweenPoints(centerPosWorld, worldPosition);
      if (dist2 < worldTolerance * worldTolerance)
        {
        closestDistance2 = dist2;
        foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentCenterPoint;
        foundComponentIndex = 0;
        }
      }
    }

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();
  for (int i = 0; i < numberOfPoints; i++)
    {
    if (!markupsNode->GetNthControlPointVisibility(i))
      {
      continue;
      }
    double centerPosWorld[4] = { 0.0, 0.0, 0.0, 1.0 };
    double centerPosDisplay[4] = { 0.0, 0.0, 0.0, 1.0 };
    markupsNode->GetNthControlPointPositionWorld(i, centerPosWorld);
    this->Renderer->SetWorldPoint(centerPosWorld);

    // Check SelectVisiblePoints output to see if the point is occluded or not.
    // SelectVisiblePoints is very sensitive to when it is executed (it has to check the z buffer after
    // opaque geometry is rendered but 2D labels are not yet), therefore we do not
    // update its output but just use the last output generated for the last rendering.
    bool pointVisible = false;
    if (this->MarkupsDisplayNode
      && this->MarkupsDisplayNode->GetOccludedVisibility()
      && this->MarkupsDisplayNode->GetOccludedOpacity() > 0.0)
      {
      pointVisible = true;
      }
    for (int controlPointType = 0; controlPointType <= Active && !pointVisible; ++controlPointType)
      {
      if ((controlPointType == Unselected && markupsNode->GetNthControlPointSelected(i))
        || (controlPointType == Selected && !markupsNode->GetNthControlPointSelected(i)))
        {
        continue;
        }
      ControlPointsPipeline3D* controlPoints = this->GetControlPointsPipeline(controlPointType);
      vtkPolyData* visiblePointsPoly = controlPoints->SelectVisiblePoints->GetOutput();
      if (!visiblePointsPoly || !visiblePointsPoly->GetPointData())
        {
        continue;
        }
      vtkIdTypeArray* visiblePointIndices = vtkIdTypeArray::SafeDownCast(visiblePointsPoly->GetPointData()->GetAbstractArray("controlPointIndices"));
      if (!visiblePointIndices)
        {
        continue;
        }
      if (visiblePointIndices->LookupValue(i) >= 0)
        {
        // visible
        pointVisible = true;
        break;
        }
      }

    if (interactionEventData->IsDisplayPositionValid())
      {
      double pixelTolerance = this->ControlPointSize / 2.0 / this->GetViewScaleFactorAtPosition(centerPosWorld)
        + this->PickingTolerance * this->ScreenScaleFactor;
      this->Renderer->SetWorldPoint(centerPosWorld);
      this->Renderer->WorldToDisplay();
      this->Renderer->GetDisplayPoint(centerPosDisplay);
      centerPosDisplay[2] = 0.0;
      double dist2 = vtkMath::Distance2BetweenPoints(centerPosDisplay, displayPosition3);
      if (dist2 < pixelTolerance * pixelTolerance && dist2 < closestDistance2 && pointVisible)
        {
        closestDistance2 = dist2;
        foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentControlPoint;
        foundComponentIndex = i;
        }
      }
    else
      {
      const double* worldPosition = interactionEventData->GetWorldPosition();
      double worldTolerance = this->ControlPointSize / 2.0 +
        this->PickingTolerance / interactionEventData->GetWorldToPhysicalScale();
      double dist2 = vtkMath::Distance2BetweenPoints(centerPosWorld, worldPosition);
      if (dist2 < worldTolerance * worldTolerance && dist2 < closestDistance2 && pointVisible)
        {
        closestDistance2 = dist2;
        foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentControlPoint;
        foundComponentIndex = i;
        }
      }
    }

  /* This would probably faster for many points:

  this->BuildLocator();

  double closestDistance2 = VTK_DOUBLE_MAX;
  int closestNode = static_cast<int> (this->Locator->FindClosestPointWithinRadius(
    this->PixelTolerance, displayPos, closestDistance2));

  if (closestNode != this->GetActiveNode())
    {
    this->SetActiveNode(closestNode);
    this->NeedToRender = 1;
    }
  return (this->GetActiveNode() >= 0);
  */
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::CanInteractWithHandles(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  if (!this->InteractionPipeline || !this->InteractionPipeline->Actor->GetVisibility())
    {
    return;
    }

  double displayPosition3[3] = { 0.0, 0.0, 0.0 };
  // Display position is valid in case of desktop interactions. Otherwise it is a 3D only context such as
  // virtual reality, and then we expect a valid world position in the absence of display position.
  if (interactionEventData->IsDisplayPositionValid())
    {
    const int* displayPosition = interactionEventData->GetDisplayPosition();
    displayPosition3[0] = static_cast<double>(displayPosition[0]);
    displayPosition3[1] = static_cast<double>(displayPosition[1]);
    }
  else if (!interactionEventData->IsWorldPositionValid())
    {
    return;
    }

  bool handlePicked = false;
  vtkSlicerMarkupsWidgetRepresentation::HandleInfoList handleInfoList = this->InteractionPipeline->GetHandleInfoList();
  for (vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::HandleInfo handleInfo : handleInfoList)
    {
    if (!handleInfo.IsVisible())
      {
      continue;
      }

    double* handleWorldPos = handleInfo.PositionWorld;
    double handleDisplayPos[3] = { 0 };

    if (interactionEventData->IsDisplayPositionValid())
      {
      double pixelTolerance = this->ControlPointSize / 2.0 / this->GetViewScaleFactorAtPosition(handleWorldPos)
        + this->PickingTolerance * this->ScreenScaleFactor;
      this->Renderer->SetWorldPoint(handleWorldPos);
      this->Renderer->WorldToDisplay();
      this->Renderer->GetDisplayPoint(handleDisplayPos);
      handleDisplayPos[2] = 0.0;
      double dist2 = vtkMath::Distance2BetweenPoints(handleDisplayPos, displayPosition3);
      if (dist2 < pixelTolerance * pixelTolerance && dist2 < closestDistance2)
        {
        closestDistance2 = dist2;
        foundComponentType = handleInfo.ComponentType;
        foundComponentIndex = handleInfo.Index;
        handlePicked = true;
        }
      }
    else
      {
      const double* worldPosition = interactionEventData->GetWorldPosition();
      double worldTolerance = this->ControlPointSize / 2.0 +
        this->PickingTolerance / interactionEventData->GetWorldToPhysicalScale();
      double dist2 = vtkMath::Distance2BetweenPoints(handleWorldPos, worldPosition);
      if (dist2 < worldTolerance * worldTolerance && dist2 < closestDistance2)
        {
        closestDistance2 = dist2;
        foundComponentType = handleInfo.ComponentType;
        foundComponentIndex = handleInfo.Index;
        }
      }
    }

  if (!handlePicked)
    {
    // Detect translation handle shaft
    for (vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::HandleInfo handleInfo : handleInfoList)
      {
      if (!handleInfo.IsVisible() || handleInfo.ComponentType != vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
        {
        continue;
        }
      double* handleWorldPos = handleInfo.PositionWorld;
      double handleDisplayPos[3] = { 0 };

      if (interactionEventData->IsDisplayPositionValid())
        {
        double pixelTolerance = this->ControlPointSize / 2.0 / this->GetViewScaleFactorAtPosition(handleWorldPos)
          + this->PickingTolerance * this->ScreenScaleFactor;
        this->Renderer->SetWorldPoint(handleWorldPos);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(handleDisplayPos);
        handleDisplayPos[2] = 0.0;

        double originWorldPos[4] = { 0.0, 0.0, 0.0, 1.0 };
        this->InteractionPipeline->GetInteractionHandleOriginWorld(originWorldPos);
        double originDisplayPos[4] = { 0.0 };
        this->Renderer->SetWorldPoint(originWorldPos);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(originDisplayPos);
        originDisplayPos[2] = displayPosition3[2]; // Handles are always projected
        double t = 0;
        double lineDistance = vtkLine::DistanceToLine(displayPosition3, originDisplayPos, handleDisplayPos, t);
        double lineDistance2 = lineDistance * lineDistance;
        if (lineDistance < pixelTolerance && lineDistance2 < closestDistance2)
          {
          closestDistance2 = lineDistance2;
          foundComponentType = handleInfo.ComponentType;
          foundComponentIndex = handleInfo.Index;
          }
        }
      else
        {
        const double* worldPosition = interactionEventData->GetWorldPosition();
        double worldTolerance = this->ControlPointSize / 2.0 +
          this->PickingTolerance / interactionEventData->GetWorldToPhysicalScale();
        double originWorldPos[4] = { 0.0, 0.0, 0.0, 1.0 };
        this->InteractionPipeline->GetInteractionHandleOriginWorld(originWorldPos);
        double t;
        double lineDistance = vtkLine::DistanceToLine(worldPosition, originWorldPos, handleWorldPos, t);
        if (lineDistance < worldTolerance && lineDistance < closestDistance2)
          {
          closestDistance2 = lineDistance;
          foundComponentType = handleInfo.ComponentType;
          foundComponentIndex = handleInfo.Index;
          }
        }
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::CanInteractWithLine(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
    || !this->GetVisibility() || !interactionEventData || !interactionEventData->IsWorldPositionValid() )
    {
    return;
    }

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();

  double pointWorldPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos2[4] = { 0.0, 0.0, 0.0, 1.0 };

  double toleranceWorld = this->ControlPointSize * this->ControlPointSize;

  for (int i = 0; i < numberOfPoints - 1; i++)
    {
    markupsNode->GetNthControlPointPositionWorld(i, pointWorldPos1);
    markupsNode->GetNthControlPointPositionWorld(i + 1, pointWorldPos2);

    double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
    const double* worldPosition = interactionEventData->GetWorldPosition();
    double distance2 = vtkLine::DistanceToLine(worldPosition, pointWorldPos1, pointWorldPos2, relativePositionAlongLine);
    if (distance2 < toleranceWorld && distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
      foundComponentIndex = i;
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  this->UpdateViewScaleFactor();
  this->UpdateControlPointSize();

  Superclass::UpdateFromMRML(caller, event, callData);

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  // Use hierarchy information if any, and if overriding is allowed for the current display node
  double hierarchyOpacity = 1.0;
  if (this->MarkupsDisplayNode->GetFolderDisplayOverrideAllowed())
    {
    vtkMRMLDisplayableNode* displayableNode = this->MarkupsDisplayNode->GetDisplayableNode();
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
    }

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    double* color = this->GetWidgetColor(controlPointType);
    double opacity = this->MarkupsDisplayNode->GetOpacity() * hierarchyOpacity;

    ControlPointsPipeline3D* controlPoints = this->GetControlPointsPipeline(controlPointType);
    controlPoints->Property->SetColor(color);
    controlPoints->Property->SetOpacity(opacity);

    controlPoints->TextProperty->ShallowCopy(this->MarkupsDisplayNode->GetTextProperty());
    controlPoints->TextProperty->SetColor(color);
    controlPoints->TextProperty->SetOpacity(opacity);
    controlPoints->TextProperty->SetFontSize(static_cast<int>(this->MarkupsDisplayNode->GetTextProperty()->GetFontSize()
      * this->MarkupsDisplayNode->GetTextScale()));
    controlPoints->TextProperty->SetBackgroundOpacity(opacity * this->MarkupsDisplayNode->GetTextProperty()->GetBackgroundOpacity());

    controlPoints->OccludedProperty->SetColor(color);
    controlPoints->OccludedTextProperty->ShallowCopy(controlPoints->TextProperty);
    if (this->MarkupsDisplayNode->GetOccludedVisibility() && this->MarkupsDisplayNode->GetOccludedOpacity() > 0.0)
      {
      // To prevent some rendering artifacts, and to ensure that the occluded actor does not block point visibility,
      // the maximum opacity of the occluded actor is required to be almost, but not fully opaque.
      double occludedOpacity =
        std::min(0.99, this->MarkupsDisplayNode->GetOccludedOpacity() * opacity);
      controlPoints->OccludedProperty->SetOpacity(occludedOpacity);
      controlPoints->OccludedTextProperty->SetOpacity(occludedOpacity);
      controlPoints->OccludedTextProperty->SetBackgroundOpacity(occludedOpacity
        * this->MarkupsDisplayNode->GetTextProperty()->GetBackgroundOpacity());
      }
    else
      {
      controlPoints->OccludedProperty->SetOpacity(0.0);
      controlPoints->OccludedTextProperty->SetOpacity(0.0);
      }

    if (this->MarkupsDisplayNode->GlyphTypeIs3D())
      {
      this->GetControlPointsPipeline(controlPointType)->Glypher->SetSourceConnection(
        this->GetControlPointsPipeline(controlPointType)->GlyphSourceSphere->GetOutputPort());
      }
    else
      {
      vtkMarkupsGlyphSource2D* glyphSource = this->GetControlPointsPipeline(controlPointType)->GlyphSource2D;
      glyphSource->SetGlyphType(this->GetGlyphTypeSourceFromDisplay(this->MarkupsDisplayNode->GetGlyphType()));
      this->GetControlPointsPipeline(controlPointType)->Glypher->SetSourceConnection(glyphSource->GetOutputPort());
      }
    }

  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(Unselected)->TextProperty);

  /* TODO: implement this for better performance
  if (event == )
    {
    int *nPtr = nullptr;
    int n = -1;
    if (callData != nullptr)
      {
      nPtr = reinterpret_cast<int *>(callData);
      if (nPtr)
        {
        n = *nPtr;
        }
      }
    this->UpdateNthPointAndLabelFromMRML(n);
    }
  else*/
    {
    this->UpdateAllPointsAndLabelsFromMRML();
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::GetActors(vtkPropCollection *pc)
{
  Superclass::GetActors(pc);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
   {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    controlPoints->Actor->GetActors(pc);
    controlPoints->OccludedActor->GetActors(pc);
    controlPoints->LabelsActor->GetActors(pc);
    controlPoints->LabelsOccludedActor->GetActors(pc);
    }
  this->TextActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  Superclass::ReleaseGraphicsResources(win);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    controlPoints->Actor->ReleaseGraphicsResources(win);
    controlPoints->OccludedActor->ReleaseGraphicsResources(win);
    controlPoints->LabelsActor->ReleaseGraphicsResources(win);
    controlPoints->LabelsOccludedActor->ReleaseGraphicsResources(win);
    }
  this->TextActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count = Superclass::RenderOverlay(viewport);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    controlPoints->SelectVisiblePoints->Update();
    if (controlPoints->Actor->GetVisibility())
      {
      count += controlPoints->Actor->RenderOverlay(viewport);
      }
    if (controlPoints->OccludedActor->GetVisibility())
      {
      count += controlPoints->OccludedActor->RenderOverlay(viewport);
      }
    if (controlPoints->LabelsActor->GetVisibility())
      {
      count += controlPoints->LabelsActor->RenderOverlay(viewport);
      }
    if (controlPoints->LabelsOccludedActor->GetVisibility())
      {
      count += controlPoints->LabelsOccludedActor->RenderOverlay(viewport);
      }
    }

  this->TextActorOccluded = false;
  if (this->TextActor->GetVisibility() && this->MarkupsNode)
    {
    // Only show text actor if at least one of the control points are visible
    if (this->HideTextActorIfAllPointsOccluded)
    {
      this->TextActorOccluded = true;
      int numberOfControlPoints = this->MarkupsNode->GetNumberOfControlPoints();
      for (int i = 0; i < numberOfControlPoints; i++)
      {
        if (this->GetNthControlPointViewVisibility(0))
        {
          this->TextActorOccluded = false;
          break;
        }
      }
    }

    // Update displayed properties text position from 3D position
    this->Renderer->SetWorldPoint(this->TextActorPositionWorld);
    this->Renderer->WorldToDisplay();
    double textActorPositionDisplay[3] = { 0.0 };
    this->Renderer->GetDisplayPoint(textActorPositionDisplay);
    this->TextActor->SetDisplayPosition(
      static_cast<int>(textActorPositionDisplay[0]),
      static_cast<int>(textActorPositionDisplay[1]));

    if (!this->TextActorOccluded)
      {
      count +=  this->TextActor->RenderOverlay(viewport);
      }
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  // Recompute glyph size if it is relative to the screen size
  // (it gets smaller/larger as the camera is moved or zoomed)
  bool updateControlPointSize = false;
  if (this->MarkupsDisplayNode->GetUseGlyphScale())
    {
    double newControlPointSize = 1.0;
    double oldScreenSizePixel = this->ScreenSizePixel;
    this->UpdateViewScaleFactor();
    if (this->ScreenSizePixel != oldScreenSizePixel)
      {
      updateControlPointSize = true;
      }
    newControlPointSize = this->ScreenSizePixel * this->ScreenScaleFactor
      * this->MarkupsDisplayNode->GetGlyphScale() / 100.0 * this->ViewScaleFactorMmPerPixel;
    // Only update the size if there is noticeable difference to avoid slight flickering
    // when the camera is moved
    if (this->ControlPointSize <= 0.0 || fabs(newControlPointSize - this->ControlPointSize) / this->ControlPointSize > 0.05)
      {
      this->ControlPointSize = newControlPointSize;
      updateControlPointSize = true;
      }
    }

  int count = Superclass::RenderOpaqueGeometry(viewport);
  vtkCamera* cam = this->Renderer->GetActiveCamera();
  double cameraPosition[3] = { 0.0, 0.0, 0.0 };
  double viewUp[3] = { 0.0, 1.0, 0.0 };
  if (cam)
    {
    cam->GetPosition(cameraPosition);
    cam->GetViewUp(viewUp);
    if (cam->GetParallelProjection())
      {
      // Glyphs face camera position.
      // In case of parallel projection, glyphs must face the camera plane normal
      // and not towards the camera position.
      // Glyph filter can only set direction by specifying a point all glyphs face towards.
      // To approximate facing the camera plane normal, we report a much farther camera position
      // to the glypher. This way direction vectors form any points to the far point are
      // approximately parallel to the camera plane normal.
      double dop[3] = { 0.0 };
      cam->GetDirectionOfProjection(dop);
      double viewportHeight = cam->GetParallelScale();
      double distance = 100.0 * viewportHeight;
      cameraPosition[0] -= dop[0] * distance;
      cameraPosition[1] -= dop[1] * distance;
      cameraPosition[2] -= dop[2] * distance;
      }
    }
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
      {
      controlPoints->Glypher->SetFollowedCameraPosition(cameraPosition);
      controlPoints->Glypher->SetFollowedCameraViewUp(viewUp);
      if (updateControlPointSize)
        {
        controlPoints->Glypher->SetScaleFactor(this->ControlPointSize);
        controlPoints->SelectVisiblePoints->SetToleranceWorld(this->ControlPointSize * 0.5);
        }
      count += controlPoints->Actor->RenderOpaqueGeometry(viewport);
      }
    if (controlPoints->OccludedActor->GetVisibility())
      {
      count += controlPoints->OccludedActor->RenderOpaqueGeometry(viewport);
      }
    if (controlPoints->LabelsActor->GetVisibility())
      {
      count += controlPoints->LabelsActor->RenderOpaqueGeometry(viewport);
      }
    if (controlPoints->LabelsOccludedActor->GetVisibility())
      {
      count += controlPoints->LabelsOccludedActor->RenderOpaqueGeometry(viewport);
      }
    count += this->TextActor->RenderOpaqueGeometry(viewport);
    }

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = Superclass::RenderTranslucentPolygonalGeometry(viewport);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    controlPoints->Actor->SetPropertyKeys(this->GetPropertyKeys());
    controlPoints->OccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    if (controlPoints->Actor->GetVisibility())
      {
      count += controlPoints->Actor->RenderTranslucentPolygonalGeometry(viewport);
      }
    if (controlPoints->OccludedActor->GetVisibility())
      {
      count += controlPoints->OccludedActor->RenderTranslucentPolygonalGeometry(viewport);
      }
    if (controlPoints->LabelsActor->GetVisibility())
      {
      count += controlPoints->LabelsActor->RenderTranslucentPolygonalGeometry(viewport);
      }
    if (controlPoints->LabelsOccludedActor->GetVisibility())
      {
      count += controlPoints->LabelsOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
      }
    }
  if (this->TextActor->GetVisibility() && !this->TextActorOccluded)
    {
    count += this->TextActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerMarkupsWidgetRepresentation3D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility() && controlPoints->Actor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    if (controlPoints->OccludedActor->GetVisibility() && controlPoints->OccludedActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    if (controlPoints->LabelsActor->GetVisibility() && controlPoints->LabelsActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    if (controlPoints->LabelsOccludedActor->GetVisibility() && controlPoints->LabelsOccludedActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    }
  if (this->TextActor->GetVisibility() && !this->TextActorOccluded && this->TextActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
double *vtkSlicerMarkupsWidgetRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors(
  {
    reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Unselected])->Actor,
    reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Selected])->Actor,
    reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[Active])->Actor
  });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::PrintSelf(ostream& os,
                                                      vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    os << indent << "Pipeline " << i << "\n";
    if (controlPoints->Actor)
      {
       os << indent << "Points Visibility: " << controlPoints->Actor->GetVisibility() << "\n";
      }
    else
      {
      os << indent << "Points: (none)\n";
      }
    if (controlPoints->LabelsActor)
      {
      os << indent << "Labels Visibility: " << controlPoints->LabelsActor->GetVisibility() << "\n";
      }
    else
      {
      os << indent << "Labels Points: (none)\n";
      }
    if (controlPoints->Property)
      {
      os << indent << "Property: " << controlPoints->Property << "\n";
      }
    else
      {
      os << indent << "Property: (none)\n";
      }
    }
  if (this->TextActor)
    {
    os << indent << "Text Visibility: " << this->TextActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Text Visibility: (none)\n";
    }
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D* vtkSlicerMarkupsWidgetRepresentation3D::GetControlPointsPipeline(int controlPointType)
{
  return reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[controlPointType]);
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::SetRenderer(vtkRenderer *ren)
{
  if (ren == this->Renderer)
    {
    return;
    }
  Superclass::SetRenderer(ren);
  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[controlPointType]);
    controlPoints->SelectVisiblePoints->SetRenderer(ren);
    }
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation3D::AccuratePick(int x, int y, double pickPoint[3])
{
  if (!this->AccuratePicker->Pick(x, y, 0, this->Renderer))
    {
    return false;
    }

  vtkPoints* pickPositions = this->AccuratePicker->GetPickedPositions();
  vtkIdType numberOfPickedPositions = pickPositions->GetNumberOfPoints();
  if (numberOfPickedPositions<1)
    {
    return false;
    }

  // There may be multiple picked positions, choose the one closest to the camera
  double cameraPosition[3] = { 0,0,0 };
  this->Renderer->GetActiveCamera()->GetPosition(cameraPosition);
  pickPositions->GetPoint(0, pickPoint);
  double minDist2 = vtkMath::Distance2BetweenPoints(pickPoint, cameraPosition);
  for (vtkIdType i = 1; i < numberOfPickedPositions; i++)
    {
    double currentMinDist2 = vtkMath::Distance2BetweenPoints(pickPositions->GetPoint(i), cameraPosition);
    if (currentMinDist2<minDist2)
      {
      pickPositions->GetPoint(i, pickPoint);
      minDist2 = currentMinDist2;
      }
    }
  return true;
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsWidgetRepresentation3D::GetViewScaleFactorAtPosition(double positionWorld[3])
{
  double viewScaleFactorMmPerPixel = 1.0;
  if (!this->Renderer || !this->Renderer->GetActiveCamera())
    {
    return viewScaleFactorMmPerPixel;
    }

  vtkCamera * cam = this->Renderer->GetActiveCamera();
  if (cam->GetParallelProjection())
    {
    // Viewport: xmin, ymin, xmax, ymax; range: 0.0-1.0; origin is bottom left
    // Determine the available renderer size in pixels
    double minX = 0;
    double minY = 0;
    this->Renderer->NormalizedDisplayToDisplay(minX, minY);
    double maxX = 1;
    double maxY = 1;
    this->Renderer->NormalizedDisplayToDisplay(maxX, maxY);
    int rendererSizeInPixels[2] = { static_cast<int>(maxX - minX), static_cast<int>(maxY - minY) };
    // Parallel scale: height of the viewport in world-coordinate distances.
    // Larger numbers produce smaller images.
    viewScaleFactorMmPerPixel = (cam->GetParallelScale() * 2.0) / double(rendererSizeInPixels[1]);
    }
  else
    {
    double cameraFP[4] = { positionWorld[0], positionWorld[1], positionWorld[2], 1.0 };

    double cameraViewUp[3] = { 0 };
    cam->GetViewUp(cameraViewUp);
    vtkMath::Normalize(cameraViewUp);

    // Get distance in pixels between two points at unit distance above and below the focal point
    this->Renderer->SetWorldPoint(cameraFP[0] + cameraViewUp[0], cameraFP[1] + cameraViewUp[1], cameraFP[2] + cameraViewUp[2], cameraFP[3]);
    this->Renderer->WorldToDisplay();
    double topCenter[3] = { 0 };
    this->Renderer->GetDisplayPoint(topCenter);
    topCenter[2] = 0.0;
    this->Renderer->SetWorldPoint(cameraFP[0] - cameraViewUp[0], cameraFP[1] - cameraViewUp[1], cameraFP[2] - cameraViewUp[2], cameraFP[3]);
    this->Renderer->WorldToDisplay();
    double bottomCenter[3] = { 0 };
    this->Renderer->GetDisplayPoint(bottomCenter);
    bottomCenter[2] = 0.0;
    double distInPixels = sqrt(vtkMath::Distance2BetweenPoints(topCenter, bottomCenter));

    // if render window is not initialized yet then distInPixels == 0.0,
    // in that case just leave the default viewScaleFactorMmPerPixel
    if (distInPixels > 1e-3)
      {
      // 2.0 = 2x length of viewUp vector in mm (because viewUp is unit vector)
      viewScaleFactorMmPerPixel = 2.0 / distInPixels;
      }
    }
  return viewScaleFactorMmPerPixel;
}


//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateViewScaleFactor()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000.0;
  if (!this->Renderer || !this->Renderer->GetActiveCamera())
    {
    return;
    }

  int* screenSize = this->Renderer->GetRenderWindow()->GetScreenSize();
  double screenSizePixel = sqrt(screenSize[0] * screenSize[0] + screenSize[1] * screenSize[1]);
  if (screenSizePixel < 1.0)
    {
    // render window is not fully initialized yet
    return;
    }
  this->ScreenSizePixel = screenSizePixel;

  double cameraFP[3] = { 0.0 };
  this->Renderer->GetActiveCamera()->GetFocalPoint(cameraFP);
  this->ViewScaleFactorMmPerPixel = this->GetViewScaleFactorAtPosition(cameraFP);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateControlPointSize()
{
  if (this->MarkupsDisplayNode->GetUseGlyphScale())
    {
    this->ControlPointSize = this->ScreenSizePixel * this->ScreenScaleFactor
      * this->MarkupsDisplayNode->GetGlyphScale() / 100.0 * this->ViewScaleFactorMmPerPixel;
    }
  else
    {
    this->ControlPointSize = this->MarkupsDisplayNode->GetGlyphSize();
    }
  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[controlPointType]);
    controlPoints->SelectVisiblePoints->SetToleranceWorld(this->ControlPointSize*0.5);
    }
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation3D::GetNthControlPointViewVisibility(int n)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->GetVisibility())
    {
    return false;
    }

  // Check SelectVisiblePoints output to see if the point is occluded or not.
  // SelectVisiblePoints is very sensitive to when it is executed (it has to check the z buffer after
  // opaque geometry is rendered but 2D labels are not yet), therefore we do not
  // update its output but just use the last output generated for the last rendering.
  for (int controlPointType = 0; controlPointType <= Active; ++controlPointType)
    {
    if ((controlPointType == Unselected && markupsNode->GetNthControlPointSelected(n))
      || (controlPointType == Selected && !markupsNode->GetNthControlPointSelected(n)))
      {
      continue;
      }
    ControlPointsPipeline3D* controlPoints = this->GetControlPointsPipeline(controlPointType);
    vtkPolyData* visiblePointsPoly = controlPoints->SelectVisiblePoints->GetOutput();
    if (!visiblePointsPoly || !visiblePointsPoly->GetPointData())
      {
      continue;
      }
    vtkIdTypeArray* visiblePointIndices = vtkIdTypeArray::SafeDownCast(visiblePointsPoly->GetPointData()->GetAbstractArray("controlPointIndices"));
    if (!visiblePointIndices)
      {
      continue;
      }
    if (visiblePointIndices->LookupValue(n) >= 0)
      {
      // visible
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateInteractionPipeline()
{
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(this->ViewNode);
  if (!viewNode)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }
  // Final visibility handled by superclass in vtkSlicerMarkupsWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::UpdateRelativeCoincidentTopologyOffsets(vtkMapper* mapper, vtkMapper* occludedMapper)
{
  Superclass::UpdateRelativeCoincidentTopologyOffsets(mapper);

  if (!occludedMapper)
    {
    return;
    }

  Superclass::UpdateRelativeCoincidentTopologyOffsets(occludedMapper);

  if (!this->MarkupsDisplayNode
    || !this->MarkupsDisplayNode->GetOccludedVisibility()
    || this->MarkupsDisplayNode->GetOccludedOpacity() <= 0.0)
    {
    return;
    }

  occludedMapper->SetRelativeCoincidentTopologyLineOffsetParameters(-1, this->OccludedRelativeOffset);
  occludedMapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(-1, this->OccludedRelativeOffset);
  occludedMapper->SetRelativeCoincidentTopologyPointOffsetParameter(this->OccludedRelativeOffset);
}
