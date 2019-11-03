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
#include "vtkSlicerAngleRepresentation3D.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"
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

  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  this->ArcMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ArcMapper->SetInputConnection(this->ArcTubeFilter->GetOutputPort());

  this->LineActor = vtkSmartPointer<vtkActor>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->ArcActor = vtkSmartPointer<vtkActor>::New();
  this->ArcActor->SetMapper(this->ArcMapper);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->TextActor = vtkSmartPointer<vtkTextActor>::New();
  this->TextActor->SetInput("0");
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(Unselected)->TextProperty);

  this->LabelFormat = "%-#6.3g";
}

//----------------------------------------------------------------------
vtkSlicerAngleRepresentation3D::~vtkSlicerAngleRepresentation3D()
= default;

//----------------------------------------------------------------------
bool vtkSlicerAngleRepresentation3D::GetTransformationReferencePoint(double referencePointWorld[3])
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
void vtkSlicerAngleRepresentation3D::BuildArc()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  // Build Arc
  if (markupsNode->GetNumberOfControlPoints() != 3)
    {
    return;
    }

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
  double angle = acos(vtkMath::Dot(vector1, vector2));

  // Place the label and place the arc
  const double length = l1 < l2 ? l1 : l2;
  const double anglePlacementRatio = 0.5;
  const double angleTextPlacementRatio = 0.7;
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
  this->Arc->Update();

  char buf[80] = {0};
  snprintf(buf, sizeof(buf)-1, this->LabelFormat.c_str(), vtkMath::DegreesFromRadians(angle));
  this->TextActor->SetInput(buf);

  double vector3[3] = { vector1[0] + vector2[0],
                        vector1[1] + vector2[1],
                        vector1[2] + vector2[2] };
  vtkMath::Normalize(vector3);
  double textPos[3] = { lText * vector3[0] + c[0],
                        lText * vector3[1] + c[1],
                        lText * vector3[2] + c[2]};

  this->Renderer->SetWorldPoint(textPos);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(textPos);

  this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]),
                                      static_cast<int>(textPos[1]));
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
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

  this->BuildLine(this->Line, false);
  this->BuildArc();

  // Update lines display properties

  this->TextActor->SetVisibility(this->MarkupsDisplayNode->GetPropertiesLabelVisibility());

  this->UpdateRelativeCoincidentTopologyOffsets(this->LineMapper);
  this->UpdateRelativeCoincidentTopologyOffsets(this->ArcMapper);

  this->TubeFilter->SetRadius(this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() * 0.5);
  this->ArcTubeFilter->SetRadius(this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() * 0.5);

  bool lineVisibility = this->GetAllControlPointsVisible();

  this->LineActor->SetVisibility(lineVisibility);
  this->ArcActor->SetVisibility(lineVisibility && markupsNode->GetNumberOfControlPoints() == 3);
  this->TextActor->SetVisibility(lineVisibility && markupsNode->GetNumberOfControlPoints() == 3);

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = this->GetAllControlPointsSelected() ? Selected : Unselected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ArcActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->Superclass::GetActors(pc);
  this->LineActor->GetActors(pc);
  this->ArcActor->GetActors(pc);
  this->TextActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerAngleRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
  this->ArcActor->ReleaseGraphicsResources(win);
  this->TextActor->ReleaseGraphicsResources(win);
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
  if (this->TextActor->GetVisibility())
    {
    count +=  this->TextActor->RenderOverlay(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerAngleRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOpaqueGeometry(viewport);
  if (this->LineActor->GetVisibility())
    {
    this->TubeFilter->SetRadius(this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() * 0.5);
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ArcActor->GetVisibility())
    {
    this->ArcTubeFilter->SetRadius(this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() * 0.5);
    count += this->ArcActor->RenderOpaqueGeometry(viewport);
    }
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderOpaqueGeometry(viewport);
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
  if (this->TextActor->GetVisibility() && this->TextActor->HasTranslucentPolygonalGeometry())
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
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
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
