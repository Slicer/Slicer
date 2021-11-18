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
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// VTK includes
#include <vtkActor2D.h>
#include <vtkArcSource.h>
#include <vtkAppendPolyData.h>
#include <vtkCellLocator.h>
#include <vtkClipPolyData.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkDoubleArray.h>
#include <vtkFeatureEdges.h>
#include <vtkGlyph2D.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlane.h>
#include <vtkPlaneCutter.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSampleImplicitFunctionFilter.h>
#include <vtkSlicerPlaneRepresentation2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLProceduralColorNode.h"

vtkStandardNewMacro(vtkSlicerPlaneRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerPlaneRepresentation2D::vtkSlicerPlaneRepresentation2D()
{
  this->PlaneCutter->SetInputConnection(this->PlaneFilter->GetOutputPort());
  this->PlaneCutter->SetPlane(this->SlicePlane);

  this->PlaneCompositeFilter->SetInputConnection(this->PlaneCutter->GetOutputPort());

  this->PlaneClipperSlicePlane->SetInputConnection(this->PlaneFilter->GetOutputPort());
  this->PlaneClipperSlicePlane->SetClipFunction(this->SlicePlane);
  this->PlaneClipperSlicePlane->GenerateClippedOutputOn();

  this->PlaneClipperStartFadeNear->SetInputConnection(this->PlaneClipperSlicePlane->GetOutputPort(0));
  this->PlaneClipperStartFadeNear->SetClipFunction(this->SlicePlane);
  this->PlaneClipperStartFadeNear->GenerateClippedOutputOn();

  this->PlaneClipperEndFadeNear->SetInputConnection(this->PlaneClipperStartFadeNear->GetOutputPort(0));
  this->PlaneClipperEndFadeNear->SetClipFunction(this->SlicePlane);
  this->PlaneClipperEndFadeNear->GenerateClippedOutputOn();

  this->PlaneClipperStartFadeFar->SetInputConnection(this->PlaneClipperSlicePlane->GetOutputPort(1));
  this->PlaneClipperStartFadeFar->SetClipFunction(this->SlicePlane);
  this->PlaneClipperStartFadeFar->GenerateClippedOutputOn();

  this->PlaneClipperEndFadeFar->SetInputConnection(this->PlaneClipperStartFadeFar->GetOutputPort(1));
  this->PlaneClipperEndFadeFar->SetClipFunction(this->SlicePlane);
  this->PlaneClipperEndFadeFar->GenerateClippedOutputOn();

  this->PlaneAppend->AddInputConnection(this->PlaneClipperStartFadeNear->GetOutputPort(1));
  this->PlaneAppend->AddInputConnection(this->PlaneClipperEndFadeNear->GetOutputPort(0));
  this->PlaneAppend->AddInputConnection(this->PlaneClipperEndFadeNear->GetOutputPort(1));
  this->PlaneAppend->AddInputConnection(this->PlaneClipperStartFadeFar->GetOutputPort(0));
  this->PlaneAppend->AddInputConnection(this->PlaneClipperEndFadeFar->GetOutputPort(0));
  this->PlaneAppend->AddInputConnection(this->PlaneClipperEndFadeFar->GetOutputPort(1));
  this->PlaneAppend->AddInputConnection(this->PlaneCompositeFilter->GetOutputPort());

  this->PlaneSliceDistance->SetImplicitFunction(this->SlicePlane);
  this->PlaneSliceDistance->SetInputConnection(this->PlaneAppend->GetOutputPort());

  this->PlaneWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->PlaneWorldToSliceTransformer->SetInputConnection(this->PlaneSliceDistance->GetOutputPort());

  this->PlaneFillMapper->SetInputConnection(this->PlaneWorldToSliceTransformer->GetOutputPort());
  this->PlaneFillMapper->SetLookupTable(this->PlaneFillColorMap);
  this->PlaneFillMapper->SetScalarVisibility(true);

  this->PlaneFillActor->SetMapper(this->PlaneFillMapper);
  this->PlaneFillActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->PlaneOutlineFilter->SetInputConnection(this->PlaneFilter->GetOutputPort());

  this->PlanePickingAppend->AddInputConnection(this->PlaneOutlineFilter->GetOutputPort());
  this->PlanePickingAppend->AddInputConnection(this->PlaneCompositeFilter->GetOutputPort());

  this->PlaneOutlineWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->PlaneOutlineWorldToSliceTransformer->SetInputConnection(this->PlaneOutlineFilter->GetOutputPort());

  this->PlaneOutlineMapper->SetInputConnection(this->PlaneOutlineWorldToSliceTransformer->GetOutputPort());
  this->PlaneOutlineMapper->SetScalarVisibility(true);

  this->PlaneOutlineActor->SetMapper(this->PlaneOutlineMapper);
  this->PlaneOutlineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->ArrowFilter->SetGlyphTypeToThickArrow();
  this->ArrowFilter->FilledOn();

  this->ArrowGlypher->SetSourceConnection(this->ArrowFilter->GetOutputPort());

  this->ArrowMapper->SetInputConnection(this->ArrowGlypher->GetOutputPort());
  this->ArrowMapper->ScalarVisibilityOff();

  this->ArrowActor->SetMapper(this->ArrowMapper);
  this->ArrowActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->LabelFormat = "%s: %-#6.3g";
}

//----------------------------------------------------------------------
vtkSlicerPlaneRepresentation2D::~vtkSlicerPlaneRepresentation2D() = default;

//----------------------------------------------------------------------
bool vtkSlicerPlaneRepresentation2D::GetTransformationReferencePoint(double referencePointWorld[3])
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
void vtkSlicerPlaneRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  this->BuildPlane();

  bool visible = true;
  if (!markupsNode->GetIsPlaneValid())
    {
    visible = false;
    }

  // Update plane display properties
  if (visible && !this->MarkupsDisplayNode->GetSliceProjection())
    {
    this->PlaneSliceDistance->Update();
    if (!this->IsRepresentationIntersectingSlice(vtkPolyData::SafeDownCast(this->PlaneSliceDistance->GetOutput()),
                                                 this->PlaneSliceDistance->GetScalarArrayName()))
      {
      visible = false;
      }
    }

  this->PlaneFillActor->SetVisibility(visible && this->MarkupsDisplayNode->GetFillVisibility());
  this->PlaneOutlineActor->SetVisibility(visible && this->MarkupsDisplayNode->GetOutlineVisibility());

  vtkMRMLMarkupsPlaneDisplayNode* planeDisplayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(this->MarkupsDisplayNode);
  if (planeDisplayNode)
    {
    this->ArrowActor->SetVisibility(visible && planeDisplayNode->GetNormalVisibility());
    this->ArrowActor->GetProperty()->SetOpacity(planeDisplayNode->GetOpacity() * planeDisplayNode->GetNormalOpacity());
    }
  else
    {
    this->ArrowActor->SetVisibility(visible);
    }

  // Properties label display
  if (visible && this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && this->AnyPointVisibilityOnSlice
    && markupsNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
    {
    double textPos[3] = { 0.0,  0.0, 0.0 };
    this->GetNthControlPointDisplayPosition(0, textPos);
    this->TextActor->SetDisplayPosition(static_cast<int>(textPos[0]), static_cast<int>(textPos[1]));
    this->TextActor->SetVisibility(true);
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }

  if (!visible)
    {
    return;
    }

  int controlPointType = Unselected;
  if (this->MarkupsDisplayNode->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentPlane)
    {
    controlPointType = Active;
    }
  else if ((markupsNode->GetNumberOfControlPoints() > 0 && !markupsNode->GetNthControlPointSelected(0)) ||
           (markupsNode->GetNumberOfControlPoints() > 1 && !markupsNode->GetNthControlPointSelected(1)) ||
           (markupsNode->GetNumberOfControlPoints() > 2 && !markupsNode->GetNthControlPointSelected(2)))
    {
    controlPointType = Unselected;
    }
  else
    {
    controlPointType = Selected;
    }
  vtkNew<vtkProperty2D> borderProperty;
  borderProperty->DeepCopy(this->GetControlPointsPipeline(Unselected)->Property);

  this->PlaneFillActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->PlaneOutlineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ArrowActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
    {
    // Update the line color mapping from the colorNode stored in the markups display node
    vtkColorTransferFunction* colormap = this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction();
    this->PlaneFillMapper->SetLookupTable(colormap);
    this->PlaneOutlineMapper->SetLookupTable(colormap);
    }
  else
    {
    // if there is no line color node, build the color mapping from few variables
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdatePlaneFillColorMap(this->PlaneFillColorMap, this->PlaneFillActor->GetProperty()->GetColor());
    this->UpdatePlaneOutlineColorMap(this->PlaneOutlineColorMap, this->PlaneFillActor->GetProperty()->GetColor());

    this->PlaneFillMapper->SetLookupTable(this->PlaneFillColorMap);
    this->PlaneOutlineMapper->SetLookupTable(this->PlaneOutlineColorMap);
    }
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::UpdatePlaneFillColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3])
{
  Superclass::UpdateDistanceColorMap(colormap, color);
  double opacity = this->MarkupsDisplayNode->GetFillVisibility()
    ? this->MarkupsDisplayNode->GetOpacity() * this->MarkupsDisplayNode->GetFillOpacity()
    : 0.0;
  double limit = this->MarkupsDisplayNode->GetLineColorFadingEnd();
  double tolerance = this->MarkupsDisplayNode->GetLineColorFadingStart();
  vtkPiecewiseFunction* opacityFunction = colormap->GetScalarOpacityFunction();
  opacityFunction->RemoveAllPoints();
  opacityFunction->AddPoint(-limit, opacity * 0.2);
  opacityFunction->AddPoint(-tolerance, opacity);
  opacityFunction->AddPoint(tolerance, opacity);
  opacityFunction->AddPoint(limit, opacity * 0.2);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::UpdatePlaneOutlineColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3])
{
  Superclass::UpdateDistanceColorMap(colormap, color);
  double opacity = this->MarkupsDisplayNode->GetOutlineVisibility()
    ? this->MarkupsDisplayNode->GetOpacity() * this->MarkupsDisplayNode->GetOutlineOpacity()
    : 0.0;
  vtkPiecewiseFunction* opacityFunction = colormap->GetScalarOpacityFunction();
  opacityFunction->RemoveAllPoints();
  opacityFunction->AddPoint(0.0, opacity);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if ( !markupsNode || markupsNode->GetLocked() || !markupsNode->GetIsPlaneValid()
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

  this->CanInteractWithPlane(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::CanInteractWithPlane(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode
    || !this->Visibility
    || !this->PlaneOutlineActor->GetVisibility())
    {
    return;
    }

  this->PlaneFilter->Update();
  if (this->PlaneFilter->GetOutput() && this->PlaneFilter->GetOutput()->GetNumberOfPoints() == 0)
    {
    return;
    }

  this->PlanePickingAppend->Update();

  double closestPointWorld[3] = { 0.0, 0.0, 0.0 };

  if (interactionEventData->IsWorldPositionValid())
    {
    double planeOrigin[3] = { 0.0, 0.0, 0.0 };
    planeNode->GetOrigin(planeOrigin);

    double planeNormal[3] = { 0.0, 0.0, 0.0 };
    planeNode->GetNormal(planeNormal);

    vtkNew<vtkPlane> plane;
    plane->SetOrigin(planeOrigin);
    plane->SetNormal(planeNormal);

    const double* worldPosition0 = interactionEventData->GetWorldPosition();
    double worldPosition1[3] = { worldPosition0[0], worldPosition0[1], worldPosition0[2] };
    vtkMath::Add(worldPosition1, this->SlicePlane->GetNormal(), worldPosition1);

    double worldPosition[3] = { 0.0, 0.0, 0.0 };
    double t;
    plane->IntersectWithLine(worldPosition0, worldPosition1, t, worldPosition);

    double dist2World = VTK_DOUBLE_MAX;

    vtkPolyData* planeSliceIntersection = this->PlanePickingAppend->GetOutput();
    for (int lineIndex = 0; lineIndex < planeSliceIntersection->GetNumberOfCells(); ++lineIndex)
      {
      vtkLine* line = vtkLine::SafeDownCast(planeSliceIntersection->GetCell(lineIndex));
      if (!line)
        {
        continue;
        }

      double edgePoint0World[3] = { 0.0, 0.0, 0.0 };
      double edgePoint1World[3] = { 0.0, 0.0, 0.0 };

      line->GetPoints()->GetPoint(0, edgePoint0World);
      line->GetPoints()->GetPoint(1, edgePoint1World);

      double t;
      double currentClosestPointWorld[3] = { 0.0, 0.0, 0.0 };
      double currentDist2World = vtkLine::DistanceToLine(worldPosition, edgePoint0World, edgePoint1World, t, currentClosestPointWorld);
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
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };
  double dist2Display = vtkMath::Distance2BetweenPoints(displayPosition3, closestPointDisplay);
  if (dist2Display < pixelTolerance * pixelTolerance && dist2Display < closestDistance2)
    {
    closestDistance2 = dist2Display;
    foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentPlane;
    foundComponentIndex = 0;
    }
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->PlaneFillActor->GetActors(pc);
  this->PlaneOutlineActor->GetActors(pc);
  this->ArrowActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->PlaneFillActor->ReleaseGraphicsResources(win);
  this->PlaneOutlineActor->ReleaseGraphicsResources(win);
  this->ArrowActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerPlaneRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = Superclass::RenderOverlay(viewport);
  if (this->PlaneFillActor->GetVisibility())
    {
    count +=  this->PlaneFillActor->RenderOverlay(viewport);
    }
  if (this->PlaneOutlineActor->GetVisibility())
    {
    count +=  this->PlaneOutlineActor->RenderOverlay(viewport);
    }
  if (this->ArrowActor->GetVisibility())
    {
    count +=  this->ArrowActor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerPlaneRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count = Superclass::RenderOpaqueGeometry(viewport);
  if (this->PlaneFillActor->GetVisibility())
    {
    count += this->PlaneFillActor->RenderOpaqueGeometry(viewport);
    }
  if (this->PlaneOutlineActor->GetVisibility())
    {
    count += this->PlaneOutlineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ArrowActor->GetVisibility())
    {
    count += this->ArrowActor->RenderOpaqueGeometry(viewport);
    }
  count += this->Superclass::RenderOpaqueGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerPlaneRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->PlaneFillActor->GetVisibility())
    {
    count += this->PlaneFillActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->PlaneOutlineActor->GetVisibility())
    {
    count += this->PlaneOutlineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ArrowActor->GetVisibility())
    {
    count += this->ArrowActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);

  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerPlaneRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->PlaneFillActor->GetVisibility() && this->PlaneFillActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->PlaneOutlineActor->GetVisibility() && this->PlaneOutlineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ArrowActor->GetVisibility() && this->ArrowActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerPlaneRepresentation2D::GetBounds()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->PlaneFillActor)
    {
    os << indent << "Plane Actor Visibility: " << this->PlaneFillActor->GetVisibility() << "\n";
    }
  else if (this->PlaneOutlineActor)
    {
    os << indent << "Plane Border Actor Visibility: " << this->PlaneOutlineActor->GetVisibility() << "\n";
    }
  else if (this->ArrowActor)
    {
    os << indent << "Arrow Actor Visibility: " << this->ArrowActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Line Actor: (none)\n";
    }

  os << indent << "Label Format: ";
  os << this->LabelFormat << "\n";
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::BuildPlane()
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode || !planeNode->GetIsPlaneValid())
    {
    this->PlaneFillMapper->SetInputData(vtkNew<vtkPolyData>());
    this->ArrowMapper->SetInputData(vtkNew<vtkPolyData>());
    return;
    }

  double xAxis_World[3] = { 0.0, 0.0, 0.0 };
  double yAxis_World[3] = { 0.0, 0.0, 0.0 };
  double zAxis_World[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetAxesWorld(xAxis_World, yAxis_World, zAxis_World);

  double epsilon = 1e-5;
  if (vtkMath::Norm(xAxis_World) <= epsilon ||
      vtkMath::Norm(yAxis_World) <= epsilon ||
      vtkMath::Norm(zAxis_World) <= epsilon)
    {
    this->PlaneFillMapper->SetInputData(vtkNew<vtkPolyData>());
    this->ArrowMapper->SetInputData(vtkNew<vtkPolyData>());
    return;
    }

  this->PlaneFillMapper->SetInputConnection(this->PlaneWorldToSliceTransformer->GetOutputPort());
  this->ArrowMapper->SetInputConnection(this->ArrowGlypher->GetOutputPort());

  double origin_World[3] = { 0.0 };
  planeNode->GetOriginWorld(origin_World);

  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  planeNode->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);

  // Update the plane
  double bounds_Object[4] = { 0.0, -1.0, 0.0, -1.0 };
  planeNode->GetPlaneBounds(bounds_Object);

  double planePoint0_Object[3] = { bounds_Object[0], bounds_Object[2], 0.0 };
  double planePoint0_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint0_Object, planePoint0_World);

  double planePoint1_Object[3] = { bounds_Object[0], bounds_Object[3], 0.0 };
  double planePoint1_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint1_Object, planePoint1_World);

  double planePoint2_Object[3] = { bounds_Object[1], bounds_Object[2], 0.0 };
  double planePoint2_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint2_Object, planePoint2_World);

  this->PlaneFilter->SetOrigin(planePoint0_World);
  this->PlaneFilter->SetPoint1(planePoint1_World);
  this->PlaneFilter->SetPoint2(planePoint2_World);

  double* arrowVectorSlice = this->WorldToSliceTransform->TransformDoubleVector(zAxis_World);

  // Update the normal vector
  if (vtkMath::Dot(this->SlicePlane->GetNormal(), zAxis_World) > 1.0 - epsilon)
    {
    this->ArrowFilter->SetGlyphTypeToCircle();
    this->ArrowFilter->SetRotationAngle(0.0);
    }
  else if (vtkMath::Dot(this->SlicePlane->GetNormal(), zAxis_World) < -1.0 + epsilon)
    {
    this->ArrowFilter->SetGlyphTypeToCross();
    this->ArrowFilter->SetRotationAngle(0.0);
    }
  else
    {
    arrowVectorSlice[2] = 0.0;

    double xVector_World[3] = { 1.0, 0.0, 0.0 };
    double yVector_World[3] = { 0.0, 1.0, 0.0 };
    double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(arrowVectorSlice, xVector_World));
    if (vtkMath::Dot(arrowVectorSlice, yVector_World) < 0.0)
      {
      angle = -angle;
      }

    this->ArrowFilter->SetGlyphTypeToThickArrow();
    this->ArrowFilter->SetRotationAngle(angle);
    }

  double origin_Slice[2] = { 0.0 };
  this->GetWorldToSliceCoordinates(origin_World, origin_Slice);
  vtkMath::Normalize(arrowVectorSlice);
  vtkMath::MultiplyScalar(arrowVectorSlice, this->ControlPointSize);
  vtkMath::Add(origin_Slice, arrowVectorSlice, origin_Slice);

  vtkNew<vtkPoints> arrowPoints_World;
  arrowPoints_World->InsertNextPoint(origin_Slice);

  vtkNew<vtkPolyData> arrowPolyData_World;
  arrowPolyData_World->SetPoints(arrowPoints_World);

  this->ArrowGlypher->SetInputData(arrowPolyData_World);
  this->ArrowGlypher->SetScaleFactor(this->ControlPointSize*2);

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }

  vtkNew<vtkPlane> planeStartFadeNear;
  planeStartFadeNear->SetOrigin(this->SlicePlane->GetOrigin());
  planeStartFadeNear->SetNormal(this->SlicePlane->GetNormal());
  planeStartFadeNear->Push(displayNode->GetLineColorFadingStart());
  this->PlaneClipperStartFadeNear->SetClipFunction(planeStartFadeNear);

  vtkNew<vtkPlane> planeEndFadeNear;
  planeEndFadeNear->SetOrigin(this->SlicePlane->GetOrigin());
  planeEndFadeNear->SetNormal(this->SlicePlane->GetNormal());
  planeEndFadeNear->Push(displayNode->GetLineColorFadingEnd());
  this->PlaneClipperEndFadeNear->SetClipFunction(planeEndFadeNear);

  vtkNew<vtkPlane> planeStartFadeFar;
  planeStartFadeFar->SetOrigin(this->SlicePlane->GetOrigin());
  planeStartFadeFar->SetNormal(this->SlicePlane->GetNormal());
  planeStartFadeFar->Push(-1.0 * displayNode->GetLineColorFadingStart());
  this->PlaneClipperStartFadeFar->SetClipFunction(planeStartFadeFar);

  vtkNew<vtkPlane> planeEndFadeFar;
  planeEndFadeFar->SetOrigin(this->SlicePlane->GetOrigin());
  planeEndFadeFar->SetNormal(this->SlicePlane->GetNormal());
  planeEndFadeFar->Push(-1.0 * displayNode->GetLineColorFadingEnd());
  this->PlaneClipperEndFadeFar->SetClipFunction(planeEndFadeFar);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipelinePlane2D(this);
  this->InteractionPipeline->InitializePipeline();
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::UpdateInteractionPipeline()
{
  Superclass::UpdateInteractionPipeline();

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode || !planeNode->GetIsPlaneValid())
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  if (!this->PlaneFillActor->GetVisibility() && !this->PlaneOutlineActor->GetVisibility())
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  this->InteractionPipeline->Actor->SetVisibility(this->MarkupsDisplayNode->GetVisibility()
    && this->MarkupsDisplayNode->GetVisibility2D()
    && this->MarkupsDisplayNode->GetHandlesInteractive());

  vtkNew<vtkTransform> handleToWorldTransform;
  handleToWorldTransform->SetMatrix(planeNode->GetInteractionHandleToWorldMatrix());
  this->InteractionPipeline->HandleToWorldTransform->DeepCopy(handleToWorldTransform);

  MarkupsInteractionPipelinePlane2D* interactionPipeline = dynamic_cast<MarkupsInteractionPipelinePlane2D*>(this->InteractionPipeline);
  interactionPipeline->UpdateScaleHandles();
  interactionPipeline->WorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);
}

//----------------------------------------------------------------------
vtkSlicerPlaneRepresentation2D::MarkupsInteractionPipelinePlane2D::MarkupsInteractionPipelinePlane2D(vtkSlicerMarkupsWidgetRepresentation* representation)
  : vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane(representation)
{
  this->WorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformFilter->SetTransform(vtkNew<vtkTransform>());
  this->WorldToSliceTransformFilter->SetInputConnection(this->HandleToWorldTransformFilter->GetOutputPort());
  this->Mapper->SetInputConnection(this->WorldToSliceTransformFilter->GetOutputPort());
  this->Mapper->SetTransformCoordinate(nullptr);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::MarkupsInteractionPipelinePlane2D::GetViewPlaneNormal(double viewPlaneNormal[3])
{
  if (!viewPlaneNormal)
    {
    return;
    }

  double viewPlaneNormal4[4] = { 0.0, 0.0, 1.0, 0.0 };
  if (this->Representation)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(this->Representation->GetViewNode());
    if (sliceNode)
      {
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneNormal4, viewPlaneNormal4);
      }
    }
  viewPlaneNormal[0] = viewPlaneNormal4[0];
  viewPlaneNormal[1] = viewPlaneNormal4[1];
  viewPlaneNormal[2] = viewPlaneNormal4[2];
}
