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

#include "vtkSlicerMarkupsWidgetRepresentation2D.h"
#include "vtkCleanPolyData.h"
#include "vtkMarkupsGlyphSource2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkActor2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
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
#include "vtkBox.h"
#include "vtkIntArray.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkWindow.h"
#include "vtkProperty2D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkLabelPlacementMapper.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkCoordinate.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkLabelHierarchy.h"
#include "vtkStringArray.h"
#include "vtkTextProperty.h"

vtkSlicerMarkupsWidgetRepresentation2D::ControlPointsPipeline2D::ControlPointsPipeline2D()
{
  this->Glypher = vtkSmartPointer<vtkGlyph2D>::New();
  this->Glypher->SetInputData(this->ControlPointsPolyData);
  this->Glypher->SetScaleFactor(1.0);

  // By default the Points are rendered as spheres
  this->Glypher->SetSourceConnection(this->GlyphSourceSphere->GetOutputPort());

  this->Property = vtkSmartPointer<vtkProperty2D>::New();
  this->Property->SetColor(0.4, 1.0, 1.0);
  this->Property->SetPointSize(10.);
  this->Property->SetLineWidth(2.);
  this->Property->SetOpacity(1.);

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->Mapper->SetInputConnection(this->Glypher->GetOutputPort());
  this->Mapper->ScalarVisibilityOff();
  vtkNew<vtkCoordinate> coordinate;
  //coordinate->SetViewport(this->Renderer); TODO: check this
  // World coordinates of the node can not be used to build the actors of the representation
  // Because the world coorinate in the node are the 3D ones.
  coordinate->SetCoordinateSystemToDisplay();
  this->Mapper->SetTransformCoordinate(coordinate);

  this->Actor = vtkSmartPointer<vtkActor2D>::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetProperty(this->Property);

  // Labels
  this->LabelsMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();

  // PointSetToLabelHierarchyFilter perturbs (slightly modifies) label positions around
  // coincident points, but the result is far from optimal (labels are quite far from
  // the control point and not at the same distance).
  // We set maximum depth from the default 5 to 10 to essentially eliminate
  // this coincident label position perturbation (perturbation may still be visible
  // when zooming into the view very much). Probably it would be cleaner to have on option
  // to disable this label position adjustment in PointSetToLabelHierarchyFilter.
  this->PointSetToLabelHierarchyFilter->SetMaximumDepth(10);

  this->LabelsMapper->PlaceAllLabelsOn();

  this->LabelsMapper->SetInputConnection(this->PointSetToLabelHierarchyFilter->GetOutputPort());
  // Here it will be the best to use Display coorinate system
  // in that way we would not need the addtional copy of the polydata in LabelFocalData (in Viewport coordinates)
  // However the LabelsMapper seems not working with the Display coordiantes.
  this->LabelsMapper->GetAnchorTransform()->SetCoordinateSystemToNormalizedViewport();
  this->LabelsActor = vtkSmartPointer<vtkActor2D>::New();
  this->LabelsActor->SetMapper(this->LabelsMapper);
}

vtkSlicerMarkupsWidgetRepresentation2D::ControlPointsPipeline2D::~ControlPointsPipeline2D()
{
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::vtkSlicerMarkupsWidgetRepresentation2D()
{
  for (int i = 0; i<NumberOfControlPointTypes; i++)
  {
    this->ControlPoints[i] = new ControlPointsPipeline2D;
  }

  this->ControlPoints[Selected]->TextProperty->SetColor(1.0, 0.5, 0.5);
  reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[Selected])->Property->SetColor(1.0, 0.5, 0.5);

  this->ControlPoints[Active]->TextProperty->SetColor(0.4, 1.0, 0.); // bright green
  reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[Active])->Property->SetColor(0.4, 1.0, 0.);


  this->PointsVisibilityOnSlice = vtkSmartPointer<vtkIntArray>::New();
  this->PointsVisibilityOnSlice->SetName("pointsVisibilityOnSlice");
  this->PointsVisibilityOnSlice->Allocate(100);
  this->PointsVisibilityOnSlice->SetNumberOfValues(1);
  this->PointsVisibilityOnSlice->SetValue(0,0);

  // by default, multiply the display node scale by this when setting scale on elements in 2d windows
  // 2d glyphs and text need to be scaled by 1/60 to show up properly in the 2d slice windows
  this->ScaleFactor2D = 0.01667;

}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::~vtkSlicerMarkupsWidgetRepresentation2D()
{
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetSliceToWorldCoordinates(const double slicePos[2],
                                                                   double worldPos[3])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (this->Renderer == nullptr || sliceNode == nullptr)
    {
    return;
    }

  double xyzw[4] =
  {
    slicePos[0] - this->Renderer->GetOrigin()[0],
    slicePos[1] - this->Renderer->GetOrigin()[1],
    0.0,
    1.0
  };
  double rasw[4] = { 0.0, 0.0, 0.0, 1.0 };

  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(xyzw, rasw);

  worldPos[0] = rasw[0]/rasw[3];
  worldPos[1] = rasw[1]/rasw[3];
  worldPos[2] = rasw[2]/rasw[3];
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetWorldToSliceCoordinates(const double worldPos[3], double slicePos[2])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (this->Renderer == nullptr ||
      sliceNode == nullptr)
    {
    return;
    }

  double sliceCoordinates[4], worldCoordinates[4];
  worldCoordinates[0] = worldPos[0];
  worldCoordinates[1] = worldPos[1];
  worldCoordinates[2] = worldPos[2];
  worldCoordinates[3] = 1.0;

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  sliceNode->GetXYToRAS()->Invert(sliceNode->GetXYToRAS(), rasToxyMatrix.GetPointer());

  rasToxyMatrix->MultiplyPoint(worldCoordinates, sliceCoordinates);

  slicePos[0] = sliceCoordinates[0];
  slicePos[1] = sliceCoordinates[1];
}


//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateAllPointsAndLabelsFromMRML(double labelsOffset)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!this->ViewNode || !markupsNode || !this->MarkupsDisplayNode || !this->Renderer)
  {
    return;
  }
  int activeControlPointIndex = this->MarkupsDisplayNode->GetActiveControlPoint();

  int numPoints = markupsNode->GetNumberOfControlPoints();

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[controlPointType]);

    controlPoints->ControlPoints->Reset();
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->Reset();

    controlPoints->LabelControlPoints->Reset();
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->Reset();
    controlPoints->Labels->Reset();
    controlPoints->LabelsPriority->Reset();

    int startIndex = 0;
    int stopIndex = numPoints - 1;
    if (controlPointType == Active)
    {
      if (activeControlPointIndex >= 0 && activeControlPointIndex < numPoints
        && markupsNode->GetNthControlPointVisibility(activeControlPointIndex)
        && this->PointsVisibilityOnSlice->GetValue(activeControlPointIndex))
      {
        startIndex = activeControlPointIndex;
        stopIndex = startIndex;
      }
      else
      {
        controlPoints->Actor->VisibilityOff();
        controlPoints->LabelsActor->VisibilityOff();
        continue;
      }
    }

    for (int pointIndex = startIndex; pointIndex <= stopIndex; pointIndex++)
      {
      if (!markupsNode->GetNthControlPointVisibility(pointIndex) || !this->PointsVisibilityOnSlice->GetValue(pointIndex))
      {
        continue;
      }
      if (controlPointType != Active)
      {
        bool thisNodeSelected = (markupsNode->GetNthControlPointSelected(pointIndex) != 0);
        if((controlPointType == Selected) != thisNodeSelected)
        {
          continue;
        }
        if (pointIndex == activeControlPointIndex)
        {
          continue;
        }
      }

      double slicePos[3] = { 0.0 };
      this->GetNthNodeDisplayPosition(pointIndex, slicePos);

      controlPoints->ControlPoints->InsertNextPoint(slicePos);
      slicePos[0] += labelsOffset;
      slicePos[1] += labelsOffset;
      this->Renderer->SetDisplayPoint(slicePos);
      this->Renderer->DisplayToView();
      double viewPos[3] = { 0.0 };
      this->Renderer->GetViewPoint(viewPos);
      this->Renderer->ViewToNormalizedViewport(viewPos[0], viewPos[1], viewPos[2]);
      controlPoints->LabelControlPoints->InsertNextPoint(viewPos);

      double pointNormalWorld[3] = { 0.0, 0.0, 1.0 };
      markupsNode->GetNthControlPointNormalWorld(pointIndex, pointNormalWorld);
      // probably we should transform this orientation to display coordinate system
      controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(pointNormalWorld);
      controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(pointNormalWorld);

      controlPoints->Labels->InsertNextValue(markupsNode->GetNthControlPointLabel(pointIndex));
      controlPoints->LabelsPriority->InsertNextValue(std::to_string(pointIndex));
      }

    controlPoints->ControlPoints->Modified();
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    controlPoints->ControlPointsPolyData->Modified();

    controlPoints->LabelControlPoints->Modified();
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    controlPoints->LabelControlPointsPolyData->Modified();

    if (controlPointType == Active)
      {
      controlPoints->Actor->VisibilityOn();
      controlPoints->LabelsActor->VisibilityOn();
      }

    }

}

//----------------------------------------------------------------------
vtkMRMLSliceNode *vtkSlicerMarkupsWidgetRepresentation2D::GetSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->ViewNode);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::GetNthNodeDisplayPosition(int n, double slicePos[2])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || n<0 || n>=markupsNode->GetNumberOfControlPoints() || !sliceNode)
    {
    return 0;
    }
  double worldPos[3];
  markupsNode->GetNthControlPointPositionWorld(n, worldPos);
  this->GetWorldToSliceCoordinates(worldPos, slicePos);
  return 1;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::SetNthPointSliceVisibility(int n, bool visibility)
{
  this->PointsVisibilityOnSlice->InsertValue(n, visibility);
  this->Modified();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::SetCenterSliceVisibility(bool visibility)
{
  this->CenterVisibilityOnSlice = visibility;
  this->Modified();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData/*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  // Update from slice node
  if (!caller || caller == this->ViewNode.GetPointer())
  {
    this->UpdateViewScaleFactor();
  }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!this->ViewNode || !markupsNode || !this->MarkupsDisplayNode
    || !this->MarkupsDisplayNode->GetVisibility()
    || !this->MarkupsDisplayNode->IsDisplayableInView(this->ViewNode->GetID())
    )
    {
    this->VisibilityOff();
    return;
    }

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
  {
    double* color = this->GetWidgetColor(controlPointType);

    ControlPointsPipeline2D* controlPoints = this->GetControlPointsPipeline(controlPointType);
    controlPoints->Property->SetColor(color);
    controlPoints->Property->SetOpacity(this->MarkupsDisplayNode->GetOpacity());

    controlPoints->TextProperty->SetColor(color);
    controlPoints->TextProperty->SetOpacity(this->MarkupsDisplayNode->GetOpacity());
    controlPoints->TextProperty->SetFontSize(static_cast<int>(5. * this->MarkupsDisplayNode->GetTextScale()));

    if (this->MarkupsDisplayNode->GlyphTypeIs3D())
    {
      this->GetControlPointsPipeline(controlPointType)->Glypher->SetSourceConnection(
        this->GetControlPointsPipeline(controlPointType)->GlyphSourceSphere->GetOutputPort());
    }
    else
    {
      vtkMarkupsGlyphSource2D* glyphSource = this->GetControlPointsPipeline(controlPointType)->GlyphSource2D;
      glyphSource->SetGlyphType(this->MarkupsDisplayNode->GetGlyphType());
      this->GetControlPointsPipeline(controlPointType)->Glypher->SetSourceConnection(glyphSource->GetOutputPort());
    }
  }

  this->ControlPointSize = this->MarkupsDisplayNode->GetGlyphScale() * this->ScaleFactor2D;

  // Points widgets have only one Markup/Representation
  for (int PointIndex = 0; PointIndex < markupsNode->GetNumberOfControlPoints(); PointIndex++)
  {
    bool visibility = this->IsPointDisplayableOnSlice(markupsNode, PointIndex);
    this->SetNthPointSliceVisibility(PointIndex, visibility);
  }
  if (markupsNode->GetCurveClosed())
  {
    bool visibility = this->IsCenterDisplayableOnSlice(markupsNode);
    this->SetCenterSliceVisibility(visibility);
  }


  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[controlPointType]);
    controlPoints->LabelsActor->SetVisibility(this->MarkupsDisplayNode->GetTextVisibility());
    controlPoints->Glypher->SetScaleFactor(this->ViewScaleFactor * this->ControlPointSize);
  }

  this->UpdateAllPointsAndLabelsFromMRML(this->ViewScaleFactor * this->ControlPointSize);

  this->VisibilityOn();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::CanInteract(
  const int displayPosition[2], const double vtkNotUsed(position)[3],
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
    || !this->MarkupsDisplayNode->IsDisplayableInView(this->ViewNode->GetID()))
  {
    return;
  }

  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  this->PixelTolerance = this->ControlPointSize * (1.0 + this->Tolerance) * this->ViewScaleFactor;
  double pixelTolerance2 = this->PixelTolerance * this->PixelTolerance;

  closestDistance2 = VTK_DOUBLE_MAX; // in display coordinate system
  foundComponentIndex = -1;
  if (markupsNode->GetNumberOfControlPoints() > 2 && this->ClosedLoop && markupsNode && this->CenterVisibilityOnSlice)
    {
    // Check if center is selected
    double centerPosWorld[3], centerPosDisplay[3];
    markupsNode->GetCenterPositionWorld(centerPosWorld);
    this->GetWorldToSliceCoordinates(centerPosWorld, centerPosDisplay);

    double dist2 = vtkMath::Distance2BetweenPoints(centerPosDisplay, displayPosition3);
    if ( dist2 < pixelTolerance2)
      {
      closestDistance2 = dist2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentCenterPoint;
      foundComponentIndex = 0;
      }
    }

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();

  double pointDisplayPos[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  sliceNode->GetXYToRAS()->Invert(sliceNode->GetXYToRAS(), rasToxyMatrix.GetPointer());
  for (int i = 0; i < numberOfPoints; i++)
    {
    if (!this->PointsVisibilityOnSlice->GetValue(i))
      {
      continue;
      }
    markupsNode->GetNthControlPointPositionWorld(i, pointWorldPos);
    rasToxyMatrix->MultiplyPoint(pointWorldPos, pointDisplayPos);
    double dist2 = vtkMath::Distance2BetweenPoints(pointDisplayPos, displayPosition3);
    if (dist2 < pixelTolerance2 && dist2 < closestDistance2)
      {
      closestDistance2 = dist2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentControlPoint;
      foundComponentIndex = i;
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::CanInteractWithLine(
  const int displayPosition[2], const double vtkNotUsed(worldPosition)[3],
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1)
  {
    return;
  }

  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  this->PixelTolerance = this->ControlPointSize * (1.0 + this->Tolerance) * this->ViewScaleFactor;
  double pixelTolerance2 = this->PixelTolerance * this->PixelTolerance;

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();

  double pointDisplayPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointDisplayPos2[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos2[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  sliceNode->GetXYToRAS()->Invert(sliceNode->GetXYToRAS(), rasToxyMatrix.GetPointer());
  for (int i = 0; i < numberOfPoints - 1; i++)
  {
    if (!this->PointsVisibilityOnSlice->GetValue(i))
    {
      continue;
    }
    if (!this->PointsVisibilityOnSlice->GetValue(i + 1))
    {
      i++; // skip one more, as the next iteration would use (i+1)-th point
      continue;
    }
    markupsNode->GetNthControlPointPositionWorld(i, pointWorldPos1);
    rasToxyMatrix->MultiplyPoint(pointWorldPos1, pointDisplayPos1);
    markupsNode->GetNthControlPointPositionWorld(i + 1, pointWorldPos2);
    rasToxyMatrix->MultiplyPoint(pointWorldPos2, pointDisplayPos2);

    double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
    double distance2 = vtkLine::DistanceToLine(displayPosition3, pointDisplayPos1, pointDisplayPos2, relativePositionAlongLine);
    if (distance2 < pixelTolerance2 && distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
    {
      closestDistance2 = distance2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
      foundComponentIndex = i;
    }
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetActors(vtkPropCollection *pc)
{
  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    controlPoints->Actor->GetActors(pc);
    controlPoints->LabelsActor->GetActors(pc);
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    controlPoints->Actor->ReleaseGraphicsResources(win);
    controlPoints->LabelsActor->ReleaseGraphicsResources(win);
  }
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
    {
      count += controlPoints->Actor->RenderOverlay(viewport);
    }
    if (controlPoints->LabelsActor->GetVisibility())
    {
      count += controlPoints->LabelsActor->RenderOverlay(viewport);
    }
  }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count = 0;
  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
    {
      count += controlPoints->Actor->RenderOpaqueGeometry(viewport);
    }
    if (controlPoints->LabelsActor->GetVisibility())
    {
      count += controlPoints->LabelsActor->RenderOpaqueGeometry(viewport);
    }
  }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = 0;
  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
    {
      count += controlPoints->Actor->RenderTranslucentPolygonalGeometry(viewport);
    }
    if (controlPoints->LabelsActor->GetVisibility())
    {
      count += controlPoints->LabelsActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerMarkupsWidgetRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility() && controlPoints->Actor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    if (controlPoints->LabelsActor->GetVisibility() && controlPoints->LabelsActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
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

  os << indent << "ScaleFactor2D = " << this->ScaleFactor2D << std::endl;
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::ControlPointsPipeline2D* vtkSlicerMarkupsWidgetRepresentation2D::GetControlPointsPipeline(int controlPointType)
{
  return reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[controlPointType]);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsPointDisplayableOnSlice(vtkMRMLMarkupsNode *markupsNode, int pointIndex)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }

  // if there's no node, it's not visible
  if (!markupsNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.")
    return false;
    }

  bool showPoint = true;

  // allow annotations to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
  {
    return false;
  }

  double transformedWorldCoordinates[4];
  markupsNode->GetNthControlPointPositionWorld(pointIndex, transformedWorldCoordinates);

  // now get the displayCoordinates for the transformed worldCoordinates
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates, displayCoordinates);

  // check if the point is close enough to the slice to be shown
  if (showPoint)
  {
    // the third coordinate of the displayCoordinates is the distance to the slice
    double distanceToSlice = displayCoordinates[2];
    double maxDistance = 0.5 + (sliceNode->GetDimensions()[2] - 1);
    /*vtkDebugMacro("Slice node " << sliceNode->GetName()
      << ": distance to slice = " << distanceToSlice
      << ", maxDistance = " << maxDistance
      << "\n\tslice node dimensions[2] = "
      << sliceNode->GetDimensions()[2]);*/
    if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
    {
      // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
      // hence, we do not want to show this widget
      showPoint = false;
    }
  }

  return showPoint;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsCenterDisplayableOnSlice(vtkMRMLMarkupsNode *markupsNode)
{
  // if no slice node, it doesn't constrain the visibility, so return that
  // it's visible
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
  {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the sliceNode.")
    return false;
  }

  // if there's no node, it's not visible
  if (!markupsNode)
  {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.")
    return false;
  }

  bool showPoint = true;

  // allow annotations to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (displayNode && !displayNode->IsDisplayableInView(sliceNode->GetID()))
  {
    return 0;
  }

  // down cast the node as a controlpoints node to get the coordinates
  double transformedWorldCoordinates[4];
  markupsNode->GetCenterPositionWorld(transformedWorldCoordinates);

  // now get the displayCoordinates for the transformed worldCoordinates
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates, displayCoordinates);

  // check if the point is close enough to the slice to be shown
  if (showPoint)
  {
    // the third coordinate of the displayCoordinates is the distance to the slice
    double distanceToSlice = displayCoordinates[2];
    double maxDistance = 0.5 + (sliceNode->GetDimensions()[2] - 1);
    /*vtkDebugMacro("Slice node " << sliceNode->GetName()
      << ": distance to slice = " << distanceToSlice
      << ", maxDistance = " << maxDistance
      << "\n\tslice node dimensions[2] = "
      << sliceNode->GetDimensions()[2]);*/
    if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
    {
      // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
      // hence, we do not want to show this widget
      showPoint = false;
    }
  }

  return showPoint;
}


//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkSlicerMarkupsWidgetRepresentation2D::GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
  {
    vtkErrorMacro("GetWorldToDisplayCoordinates: no slice node!");
    return;
  }

  // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

  vtkMatrix4x4 * xyToRasMatrix = sliceNode->GetXYToRAS();
  vtkNew<vtkMatrix4x4> rasToXyMatrix;

  // we need to invert this matrix
  xyToRasMatrix->Invert(xyToRasMatrix, rasToXyMatrix.GetPointer());

  double worldCoordinates[4];
  worldCoordinates[0] = r;
  worldCoordinates[1] = a;
  worldCoordinates[2] = s;
  worldCoordinates[3] = 1;

  rasToXyMatrix->MultiplyPoint(worldCoordinates, displayCoordinates);
  xyToRasMatrix = nullptr;
}

//---------------------------------------------------------------------------
// Convert world to display coordinates
void vtkSlicerMarkupsWidgetRepresentation2D::GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates)
{
  if (worldCoordinates == nullptr)
  {
    return;
  }

  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::GetAllControlPointsVisible()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }

  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (!this->PointsVisibilityOnSlice->GetValue(controlPointIndex) ||
      !markupsNode->GetNthControlPointVisibility(controlPointIndex))
      {
      return false;
      }
    }
  return true;
}
