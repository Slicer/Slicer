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

#include "vtkSlicerMarkupsWidgetRepresentation3D.h"
#include "vtkCellPicker.h"
#include "vtkCleanPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkAssemblyPath.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
#include "vtkAssemblyPath.h"
#include "vtkMath.h"
#include "vtkInteractorObserver.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkLine.h"
#include "vtkCoordinate.h"
#include "vtkGlyph3D.h"
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
#include "vtkProperty.h"
#include "vtkTextProperty.h"
#include "vtkActor2D.h"
#include "vtkLabelPlacementMapper.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkStringArray.h"
#include "vtkLabelHierarchy.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkMarkupsGlyphSource2D.h"

vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D::ControlPointsPipeline3D()
{
  this->Glypher = vtkSmartPointer<vtkGlyph3D>::New();
  this->Glypher->SetInputData(this->ControlPointsPolyData);
  this->Glypher->SetVectorModeToUseNormal();
  this->Glypher->OrientOn();
  this->Glypher->ScalingOn();
  this->Glypher->SetScaleModeToDataScalingOff();
  this->Glypher->SetScaleFactor(1.0);

  // By default the Points are rendered as spheres
  this->Glypher->SetSourceConnection(this->GlyphSourceSphere->GetOutputPort());

  this->Property = vtkSmartPointer<vtkProperty>::New();
  this->Property->SetRepresentationToSurface();
  this->Property->SetColor(0.4, 1.0, 1.0);
  this->Property->SetAmbient(0.0);
  this->Property->SetDiffuse(1.0);
  this->Property->SetSpecular(0.0);
  this->Property->SetShading(true);
  this->Property->SetSpecularPower(1.0);
  this->Property->SetPointSize(10.);
  this->Property->SetLineWidth(2.);
  this->Property->SetOpacity(1.);

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Mapper->SetInputConnection(this->Glypher->GetOutputPort());
  // This turns on resolve coincident topology for everything
  // as it is a class static on the mapper
  vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
  this->Mapper->ScalarVisibilityOff();

  this->Actor = vtkSmartPointer<vtkActor>::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetProperty(this->Property);

  // Labels

  this->SelectVisiblePoints = vtkSmartPointer<vtkSelectVisiblePoints>::New();
  this->SelectVisiblePoints->SetInputData(this->LabelControlPointsPolyData);
  //this->SelectVisiblePoints->SetInputData(this->ControlPointsPolyData);

  this->PointSetToLabelHierarchyFilter->SetInputConnection(this->SelectVisiblePoints->GetOutputPort());

  this->LabelsMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  this->LabelsMapper->SetInputConnection(this->PointSetToLabelHierarchyFilter->GetOutputPort());

  this->LabelsMapper->PlaceAllLabelsOn();

  /*
  We could consider showing labels with semi-transparent background:
  this->LabelsMapper->SetShapeToRect();
  this->LabelsMapper->SetStyleToFilled();
  this->LabelsMapper->SetBackgroundOpacity(0.4);
  */

  this->LabelsActor = vtkSmartPointer<vtkActor2D>::New();
  this->LabelsActor->SetMapper(this->LabelsMapper);
  this->LabelsActor->PickableOff();
  this->LabelsActor->DragableOff();
};

vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D::~ControlPointsPipeline3D()
{
}

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

  this->ControlPointSize = 10; // will be set from the markup's GlyphScale
  this->Tolerance = 5.0;

  this->AccuratePicker = vtkSmartPointer<vtkCellPicker>::New();
  this->AccuratePicker->SetTolerance(.005);
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation3D::~vtkSlicerMarkupsWidgetRepresentation3D()
{
}

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
  vtkMRMLMarkupsDisplayNode* display = this->MarkupsDisplayNode;
  if (!display)
    {
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  this->ControlPointSize = this->MarkupsDisplayNode->GetGlyphScale();

  int numPoints = markupsNode->GetNumberOfControlPoints();

  for (int i = 0; i<NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    this->UpdateRelativeCoincidentTopologyOffsets(controlPoints->Mapper);
    controlPoints->Glypher->SetScaleFactor(this->ControlPointSize);
    }


  int activeControlPointIndex = this->MarkupsDisplayNode->GetActiveControlPoint();

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[controlPointType]);

    controlPoints->ControlPoints->SetNumberOfPoints(0);
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);

    controlPoints->LabelControlPoints->SetNumberOfPoints(0);
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);

    controlPoints->Labels->SetNumberOfValues(0);
    controlPoints->LabelsPriority->SetNumberOfValues(0);

    int startIndex = 0;
    int stopIndex = numPoints - 1;
    if (controlPointType == Active)
      {
      if (activeControlPointIndex >= 0 && activeControlPointIndex < numPoints &&
        markupsNode->GetNthControlPointVisibility(activeControlPointIndex))
        {
        startIndex = activeControlPointIndex;
        stopIndex = startIndex;
        controlPoints->Actor->VisibilityOn();
        controlPoints->LabelsActor->SetVisibility(display->GetTextVisibility());
        }
      else
        {
        controlPoints->Actor->VisibilityOff();
        controlPoints->LabelsActor->VisibilityOff();
        continue;
        }
      }
    else
      {
      controlPoints->LabelsActor->SetVisibility(display->GetTextVisibility());
      }

    for (int pointIndex = startIndex; pointIndex <= stopIndex; pointIndex++)
      {

      if (controlPointType != Active
        && (!markupsNode->GetNthControlPointVisibility(pointIndex) || pointIndex == activeControlPointIndex ||
        ((controlPointType == Selected) != (markupsNode->GetNthControlPointSelected(pointIndex) != 0))))
        {
        continue;
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
      }

    controlPoints->ControlPoints->Modified();
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    controlPoints->ControlPointsPolyData->Modified();

    controlPoints->LabelControlPoints->Modified();
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    controlPoints->LabelControlPointsPolyData->Modified();

    }
}


//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::CanInteract(
  const int displayPosition[2], const double vtkNotUsed(worldPosition)[3],
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1)
    {
    return;
    }

  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  double pixelTolerance2 = this->PixelTolerance * this->PixelTolerance;

  closestDistance2 = VTK_DOUBLE_MAX; // in display coordinate system
  foundComponentIndex = -1;
  if (markupsNode->GetNumberOfControlPoints() > 2 && this->ClosedLoop && markupsNode)
    {
    // Check if center is selected
    double centerPosWorld[3], centerPosDisplay[3];
    markupsNode->GetCenterPosition(centerPosWorld);
    this->Renderer->SetWorldPoint(centerPosWorld);
    this->Renderer->WorldToDisplay();
    this->Renderer->GetDisplayPoint(centerPosDisplay);
    double dist2 = vtkMath::Distance2BetweenPoints(centerPosDisplay, displayPosition3);
    if (dist2 < pixelTolerance2)
      {
      closestDistance2 = dist2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentCenterPoint;
      foundComponentIndex = 0;
      }
    }

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();
  for (int i = 0; i < numberOfPoints; i++)
    {
    if (!markupsNode->GetNthControlPointVisibility(i))
      {
      continue;
      }
    double centerPosWorld[3], centerPosDisplay[3];
    markupsNode->GetNthControlPointPositionWorld(i, centerPosWorld);
    this->Renderer->SetWorldPoint(centerPosWorld);
    this->Renderer->WorldToDisplay();
    this->Renderer->GetDisplayPoint(centerPosDisplay);
    double dist2 = vtkMath::Distance2BetweenPoints(centerPosDisplay, displayPosition3);
    if (dist2 < pixelTolerance2 && dist2 < closestDistance2)
      {
      closestDistance2 = dist2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentControlPoint;
      foundComponentIndex = i;
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
void vtkSlicerMarkupsWidgetRepresentation3D::CanInteractWithLine(
  const int vtkNotUsed(displayPosition)[2], const double worldPosition[3],
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1)
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
  // Update from slice node
  if (!caller || caller == this->ViewNode.GetPointer())
    {
    this->UpdateViewScaleFactor();
    }

  this->ControlPointSize = this->MarkupsDisplayNode->GetGlyphScale();
  this->PixelTolerance = this->ControlPointSize * (1.0 + this->Tolerance) * this->ViewScaleFactor;

  Superclass::UpdateFromMRML(caller, event, callData);

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!this->ViewNode || !markupsNode || !this->MarkupsDisplayNode
    || !this->MarkupsDisplayNode->GetVisibility()
    || !this->MarkupsDisplayNode->IsDisplayableInView(this->ViewNode->GetID())
    )
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    double* color = this->GetWidgetColor(controlPointType);

    ControlPointsPipeline3D* controlPoints = this->GetControlPointsPipeline(controlPointType);
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
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    controlPoints->Actor->GetActors(pc);
    controlPoints->LabelsActor->GetActors(pc);
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    controlPoints->Actor->ReleaseGraphicsResources(win);
    controlPoints->LabelsActor->ReleaseGraphicsResources(win);
    }
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
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
int vtkSlicerMarkupsWidgetRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
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
int vtkSlicerMarkupsWidgetRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline3D* controlPoints = reinterpret_cast<ControlPointsPipeline3D*>(this->ControlPoints[i]);
    // The internal actor needs to share property keys. This allows depth peeling
    // etc to work.
    controlPoints->Actor->SetPropertyKeys(this->GetPropertyKeys());
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
    if (controlPoints->LabelsActor->GetVisibility() && controlPoints->LabelsActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
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
