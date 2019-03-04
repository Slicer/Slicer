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

#include "vtkSlicerAngleRepresentation2D.h"
#include "vtkCleanPolyData.h"
#include "vtkOpenGLPolyDataMapper2D.h"
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
#include "vtkTextProperty.h"
#include "vtkTubeFilter.h"
#include "vtkStringArray.h"
#include "vtkPickingManager.h"
#include "vtkVectorText.h"
#include "vtkOpenGLTextActor.h"
#include "vtkArcSource.h"
#include "cmath"
#include "vtkMRMLMarkupsDisplayNode.h"

vtkStandardNewMacro(vtkSlicerAngleRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation2D::vtkSlicerAngleRepresentation2D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();
  this->Arc = vtkSmartPointer<vtkArcSource>::New();
  this->Arc->SetResolution(30);

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputData(this->Line);
  this->TubeFilter->SetNumberOfSides(20);
  this->TubeFilter->SetRadius(1);

  this->ArcTubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->ArcTubeFilter->SetInputConnection(this->Arc->GetOutputPort());
  this->ArcTubeFilter->SetNumberOfSides(20);
  this->ArcTubeFilter->SetRadius(1);

  this->LineMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper2D>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  this->ArcMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper2D>::New();
  this->ArcMapper->SetInputConnection(this->ArcTubeFilter->GetOutputPort());

  this->LineActor = vtkSmartPointer<vtkActor2D>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->ArcActor = vtkSmartPointer<vtkActor2D>::New();
  this->ArcActor->SetMapper(this->ArcMapper);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->TextActor = vtkSmartPointer<vtkOpenGLTextActor>::New();
  this->TextActor->SetInput("0");
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(Unselected)->TextProperty);

  this->LabelFormat = "%-#6.3g";
}

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation2D::~vtkSlicerAngleRepresentation2D()
{
}

//----------------------------------------------------------------------
bool vtkSlicerAngleRepresentation2D::GetTransformationReferencePoint(double referencePointWorld[3])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() < 2)
    {
    return false;
    }
  markupsNode->GetNthControlPointPositionWorld(1, referencePointWorld);
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::BuildArc()
{
  // Build Arc
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() != 3)
    {
    return;
    }

  double p1[2], p2[2], c[2], vector2[3], vector1[3];
  double l1 = 0.0, l2 = 0.0;
  this->GetNthNodeDisplayPosition(0, p1);
  this->GetNthNodeDisplayPosition(1, c);
  this->GetNthNodeDisplayPosition(2, p2);

  // Compute the angle (only if necessary since we don't want
  // fluctuations in angle value as the camera moves, etc.)
  if (fabs(p1[0]-c[0]) < 0.001 || fabs(p2[0]-c[0]) < 0.001)
    {
    return;
    }

  vector1[0] = p1[0] - c[0];
  vector1[1] = p1[1] - c[1];
  vector1[2] = 0.;
  vector2[0] = p2[0] - c[0];
  vector2[1] = p2[1] - c[1];
  vector2[2] = 0.;
  l1 = vtkMath::Normalize(vector1);
  l2 = vtkMath::Normalize(vector2);
  double angle = acos(vtkMath::Dot(vector1, vector2));

  // Place the label and place the arc
  const double length = l1 < l2 ? l1 : l2;
  const double anglePlacementRatio = 0.5;
  const double l = length * anglePlacementRatio;
  double arcp1[3] = {l * vector1[0] + c[0],
                     l * vector1[1] + c[1],
                     0.};
  double arcp2[3] = {l * vector2[0] + c[0],
                     l * vector2[1] + c[1],
                     0.};
  double arcc[3] = {c[0], c[1], 0.};

  this->Arc->SetPoint1(arcp1);
  this->Arc->SetPoint2(arcp2);
  this->Arc->SetCenter(arcc);
  this->Arc->Update();

  char buf[80];
  buf[sizeof(buf) - 1] = 0;
  snprintf(buf, sizeof(buf), this->LabelFormat.c_str(), vtkMath::DegreesFromRadians(angle));
  this->TextActor->SetInput(buf);

  double textPosDisplay[2], vector3[3];
  vector3[0] = vector1[0] + vector2[0];
  vector3[1] = vector1[1] + vector2[1];
  vector3[2] = vector1[2] + vector2[2];
  vtkMath::Normalize(vector3);
  textPosDisplay[0] = c[0] + vector3[0] * length * 0.6;
  textPosDisplay[1] = c[1] + vector3[1] * length * 0.6;

  int X = static_cast<int>(textPosDisplay[0]);
  int Y = static_cast<int>(textPosDisplay[1]);
  this->TextActor->SetDisplayPosition(X,Y);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=NULL*/)
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

  // Update lines geometry

  this->BuildLine(this->Line, true);
  this->BuildArc();

  // Update lines display properties

  this->TextActor->SetVisibility(this->MarkupsDisplayNode->GetTextVisibility());

  this->TubeFilter->SetRadius(this->ViewScaleFactor * this->ControlPointSize * 0.125);
  this->ArcTubeFilter->SetRadius(this->ViewScaleFactor * this->ControlPointSize * 0.125);

  bool allNodeVisibile = this->GetAllControlPointsVisible();
  this->LineActor->SetVisibility(allNodeVisibile);
  this->ArcActor->SetVisibility(allNodeVisibile && markupsNode->GetNumberOfControlPoints() == 3);
  this->TextActor->SetVisibility(allNodeVisibile && markupsNode->GetNumberOfControlPoints() == 3);

  int controlPointType = Unselected;
  if (this->MarkupsDisplayNode->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = Active;
    }
  else if (!markupsNode->GetNthControlPointSelected(0) ||
           (markupsNode->GetNumberOfControlPoints() > 1 && !markupsNode->GetNthControlPointSelected(1)) ||
           (markupsNode->GetNumberOfControlPoints() > 2 && !markupsNode->GetNthControlPointSelected(2)))
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
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::CanInteract(
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

  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  this->PixelTolerance = this->ControlPointSize * (1.0 + this->Tolerance) * this->ViewScaleFactor;
  double pixelTolerance2 = this->PixelTolerance * this->PixelTolerance;

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();

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
    if (distance2 < pixelTolerance2 && distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
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
  this->TextActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->LineActor->ReleaseGraphicsResources(win);
  this->ArcActor->ReleaseGraphicsResources(win);
  this->TextActor->ReleaseGraphicsResources(win);
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
  if (this->TextActor->GetVisibility())
    {
    count +=  this->TextActor->RenderOverlay(viewport);
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
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderOpaqueGeometry(viewport);
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
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerAngleRepresentation2D::HasTranslucentPolygonalGeometry()
{
  int result=0;
  if (this->LineActor->GetVisibility())
    {
    result |= this->LineActor->HasTranslucentPolygonalGeometry();
    }
  if (this->ArcActor->GetVisibility())
    {
    result |= this->ArcActor->HasTranslucentPolygonalGeometry();
    }
  if (this->TextActor->GetVisibility())
    {
    result |= this->TextActor->HasTranslucentPolygonalGeometry();
    }
  result |= this->Superclass::HasTranslucentPolygonalGeometry();

  return result;
}

//----------------------------------------------------------------------
double *vtkSlicerAngleRepresentation2D::GetBounds()
{
  return NULL;
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

  if (this->TextActor)
    {
    os << indent << "Text Visibility: " << this->TextActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Text Visibility: (none)\n";
    }

  os << indent << "Label Format: ";
  os << this->LabelFormat << "\n";
}
