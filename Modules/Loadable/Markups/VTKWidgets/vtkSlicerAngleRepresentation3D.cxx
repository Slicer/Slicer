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
#include "vtkGlyph3D.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkSlicerAngleRepresentation3D.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsAngleNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"

vtkStandardNewMacro(vtkSlicerAngleRepresentation3D);

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation3D::vtkSlicerAngleRepresentation3D()
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

  // Mappers
  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  this->ArcMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ArcMapper->SetInputConnection(this->ArcTubeFilter->GetOutputPort());

  this->LineOccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineOccludedMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  this->ArcOccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ArcOccludedMapper->SetInputConnection(this->ArcTubeFilter->GetOutputPort());

  // Actors
  this->LineActor = vtkSmartPointer<vtkActor>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->ArcActor = vtkSmartPointer<vtkActor>::New();
  this->ArcActor->SetMapper(this->ArcMapper);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->LineOccludedActor = vtkSmartPointer<vtkActor>::New();
  this->LineOccludedActor->SetMapper(this->LineOccludedMapper);
  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(Unselected)->OccludedProperty);

  this->ArcOccludedActor = vtkSmartPointer<vtkActor>::New();
  this->ArcOccludedActor->SetMapper(this->ArcOccludedMapper);
  this->ArcOccludedActor->SetProperty(this->GetControlPointsPipeline(Unselected)->OccludedProperty);

  this->HideTextActorIfAllPointsOccluded = true;
}

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation3D::~vtkSlicerAngleRepresentation3D() = default;

//----------------------------------------------------------------------
bool vtkSlicerAngleRepresentation3D::GetTransformationReferencePoint(double referencePointWorld[3])
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
void vtkSlicerAngleRepresentation3D::BuildArc()
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
  const double length = l1 < l2 ? l1 : l2;
  double anglePlacementRatio = 0.5;
  double angleTextPlacementRatio = 0.7;
  if (markupsNode->GetAngleMeasurementMode() == vtkMRMLMarkupsAngleNode::OrientedPositive)
    {
    // Reduce arc size if angle>180deg (it just takes too much space).
    // arcLengthAdjustmentFactor is a sigmoid function that is 1.0 for angle<180 and 0.5 for angle>180,
    // with a smooth transition.
    double arcLengthAdjustmentFactor = 0.5 + 0.5 / (1+exp((angle-180.0)*0.1));
    anglePlacementRatio *= arcLengthAdjustmentFactor;
    angleTextPlacementRatio *= arcLengthAdjustmentFactor;
    }
  const double lArc = length * anglePlacementRatio;
  const double lText = length * angleTextPlacementRatio;
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

  double vector3[3] = { vector1[0] + vector2[0],
                        vector1[1] + vector2[1],
                        vector1[2] + vector2[2] };
  vtkMath::Normalize(vector3);

  this->TextActor->SetVisibility(this->MarkupsDisplayNode->GetPropertiesLabelVisibility());
  this->TextActorPositionWorld[0] = lText * (longArc ? -1.0 : 1.0) * vector3[0] + c[0];
  this->TextActorPositionWorld[1] = lText * (longArc ? -1.0 : 1.0) * vector3[1] + c[1];
  this->TextActorPositionWorld[2] = lText * (longArc ? -1.0 : 1.0) * vector3[2] + c[2];
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
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

  // Update lines geometry

  this->BuildLine(this->Line, false);
  this->BuildArc();

  // Update lines display properties

  this->UpdateRelativeCoincidentTopologyOffsets(this->LineMapper, this->LineOccludedMapper);
  this->UpdateRelativeCoincidentTopologyOffsets(this->ArcMapper, this->ArcOccludedMapper);

  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  this->TubeFilter->SetRadius(diameter * 0.5);
  this->ArcTubeFilter->SetRadius(diameter * 0.5);

  bool lineVisibility = this->GetAllControlPointsVisible();

  this->LineActor->SetVisibility(lineVisibility);
  this->ArcActor->SetVisibility(lineVisibility && markupsNode->GetNumberOfDefinedControlPoints(true) == 3);
  this->LineOccludedActor->SetVisibility(this->MarkupsDisplayNode && this->LineActor->GetVisibility() && this->MarkupsDisplayNode->GetOccludedVisibility());
  this->ArcOccludedActor->SetVisibility(this->MarkupsDisplayNode && this->ArcActor->GetVisibility() && this->MarkupsDisplayNode->GetOccludedVisibility());

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = this->GetAllControlPointsSelected() ? Selected : Unselected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);
  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);
  this->ArcOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->Superclass::GetActors(pc);
  this->LineActor->GetActors(pc);
  this->ArcActor->GetActors(pc);
  this->ArcOccludedActor->GetActors(pc);
  this->LineOccludedActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
  this->ArcActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerAngleRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOverlay(viewport);
  if (this->LineActor->GetVisibility())
    {
    count +=  this->LineActor->RenderOverlay(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    count +=  this->ArcActor->RenderOverlay(viewport);
    }
  if (this->ArcOccludedActor->GetVisibility())
    {
    count +=  this->ArcOccludedActor->RenderOverlay(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    count +=  this->LineOccludedActor->RenderOverlay(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerAngleRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOpaqueGeometry(viewport);
  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  if (this->LineActor->GetVisibility())
    {
    this->TubeFilter->SetRadius(diameter * 0.5);
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    this->ArcTubeFilter->SetRadius(diameter * 0.5);
    count += this->ArcActor->RenderOpaqueGeometry(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    count += this->LineOccludedActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ArcOccludedActor->GetVisibility())
    {
    count += this->ArcOccludedActor->RenderOpaqueGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerAngleRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->LineActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->LineActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->ArcActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ArcActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->LineOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->LineOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ArcOccludedActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->ArcOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ArcOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerAngleRepresentation3D::HasTranslucentPolygonalGeometry()
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
  if (this->LineOccludedActor->GetVisibility() && this->LineOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ArcOccludedActor->GetVisibility() && this->ArcOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerAngleRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors({ this->LineActor, this->ArcActor });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfDefinedControlPoints(true) < 1
    || !interactionEventData )
    {
    return;
    }
  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    return;
    }

  this->CanInteractWithLine(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}


//-----------------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
    {
    os << indent << "Line Visibility: " << this->LineActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Line Visibility: (none)\n";
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
void vtkSlicerAngleRepresentation3D::UpdateInteractionPipeline()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfDefinedControlPoints(true) < 3)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }
  // Final visibility handled by superclass in vtkSlicerMarkupsWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}
