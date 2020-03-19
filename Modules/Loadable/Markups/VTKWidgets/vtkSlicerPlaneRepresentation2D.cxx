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
#include "vtkMRMLProceduralColorNode.h"

#include "vtkMRMLMarkupsPlaneNode.h"

#include "vtkMarkupsGlyphSource2D.h"

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

  this->PlaneMapper->SetInputConnection(this->PlaneWorldToSliceTransformer->GetOutputPort());
  this->PlaneMapper->SetLookupTable(this->ColorMap);
  this->PlaneMapper->SetScalarVisibility(true);

  this->PlaneActor->SetMapper(this->PlaneMapper);
  this->PlaneActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->PlaneBorderWorldToSliceTransformer->SetTransform(this->WorldToSliceTransform);
  this->PlaneBorderWorldToSliceTransformer->SetInputConnection(this->PlaneFilter->GetOutputPort());

  this->PlaneBorderFilter->SetInputConnection(this->PlaneBorderWorldToSliceTransformer->GetOutputPort());
  this->PlaneBorderMapper->SetInputConnection(this->PlaneBorderFilter->GetOutputPort());
  this->PlaneBorderActor->SetMapper(this->PlaneBorderMapper);

  this->PlaneBorderActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->ArrowFilter->SetGlyphTypeToThickArrow();
  this->ArrowFilter->FilledOn();

  this->ArrowGlypher->SetSourceConnection(this->ArrowFilter->GetOutputPort());
  //this->ArrowGlypher->OrientOn();

  this->ArrowMapper->SetInputConnection(this->ArrowGlypher->GetOutputPort());
  this->ArrowMapper->SetScalarVisibility(true);

  this->ArrowActor->SetMapper(this->ArrowMapper);
  this->ArrowActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->LabelFormat = "%s: %-#6.3g";
}

//----------------------------------------------------------------------
vtkSlicerPlaneRepresentation2D::~vtkSlicerPlaneRepresentation2D()
= default;

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
  if (!markupsNode || !this->MarkupsDisplayNode
    || !this->MarkupsDisplayNode->GetVisibility()
    || !this->MarkupsDisplayNode->IsDisplayableInView(this->ViewNode->GetID())
    )
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  this->BuildPlane();

  bool visible = true;
  if (markupsNode->GetNumberOfControlPoints() < 3)
    {
    visible = false;
    }

  // Update plane display properties

  if (visible && !this->MarkupsDisplayNode->GetSliceProjection())
    {
    this->PlaneSliceDistance->Update();
    vtkPolyData* plane = vtkPolyData::SafeDownCast(this->PlaneSliceDistance->GetOutput());
    if (!plane)
      {
      visible = false;
      }
    else
      {
      double sliceNormalVectorXY[4] = { 0, 0, 1, 0 };
      double sliceNormalVectorWorld[4] = { 0, 0, 1, 0 };
      vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
      xyToRAS->MultiplyPoint(sliceNormalVectorXY, sliceNormalVectorWorld);
      double sliceThicknessMm = vtkMath::Norm(sliceNormalVectorWorld);
      double* scalarRange = plane->GetScalarRange();
      if (scalarRange[0] > 0.5 * sliceThicknessMm || scalarRange[1] < -0.5 * sliceThicknessMm)
        {
        visible = false;
        }
      }
    }

  this->PlaneActor->SetVisibility(visible);
  this->PlaneBorderActor->SetVisibility(visible);
  this->ArrowActor->SetVisibility(visible);

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

  this->PlaneActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->PlaneBorderActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ArrowActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  if (this->MarkupsDisplayNode->GetLineColorNode() && this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction())
    {
    // Update the line color mapping from the colorNode stored in the markups display node
    vtkColorTransferFunction* colormap = this->MarkupsDisplayNode->GetLineColorNode()->GetColorTransferFunction();
    this->PlaneMapper->SetLookupTable(colormap);
    this->PlaneBorderMapper->SetLookupTable(colormap);
    this->ArrowMapper->SetLookupTable(colormap);
    }
  else
    {
    // if there is no line color node, build the color mapping from few variables
    // (color, opacity, distance fading, saturation and hue offset) stored in the display node
    this->UpdateDistanceColorMap(this->ColorMap, this->PlaneActor->GetProperty()->GetColor());
    this->PlaneMapper->SetLookupTable(this->ColorMap);
    this->PlaneBorderMapper->SetLookupTable(this->ColorMap);
    this->ArrowMapper->SetLookupTable(this->ColorMap);
    }
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::UpdateDistanceColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3])
{
  Superclass::UpdateDistanceColorMap(colormap, color);
  double opacity = this->MarkupsDisplayNode->GetOpacity();
  double limit = this->MarkupsDisplayNode->GetLineColorFadingEnd();
  double tolerance = this->MarkupsDisplayNode->GetLineColorFadingStart();
  vtkPiecewiseFunction* opacityFunction = colormap->GetScalarOpacityFunction();
  opacityFunction->RemoveAllPoints();
  opacityFunction->AddPoint(-limit, opacity * 0.2);
  opacityFunction->AddPoint(-tolerance, opacity*0.5);
  opacityFunction->AddPoint(tolerance, opacity*0.5);
  opacityFunction->AddPoint(limit, opacity*0.2);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::CanInteract(
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
  // Create the tree
  vtkSmartPointer<vtkCellLocator> cellLocator =
    vtkSmartPointer<vtkCellLocator>::New();
  this->PlaneFilter->Update();
  if (this->PlaneFilter->GetOutput() && this->PlaneFilter->GetOutput()->GetNumberOfPoints() == 0)
    {
    return;
    }

  cellLocator->SetDataSet(this->PlaneFilter->GetOutput());
  cellLocator->BuildLocator();

  const double* worldPosition = interactionEventData->GetWorldPosition();
  double closestPoint[3];//the coordinates of the closest point will be returned here
  double distance2; //the squared distance to the closest point will be returned here
  vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
  int subId; //this is rarely used (in triangle strips only, I believe)
  cellLocator->FindClosestPoint(worldPosition, closestPoint, cellId, subId, distance2);

  double toleranceWorld = this->ControlPointSize / 2;
  if (distance2 < toleranceWorld)
    {
    closestDistance2 = distance2;
    foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentPlane;
    foundComponentIndex = 0;
    }
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->PlaneActor->GetActors(pc);
  this->PlaneBorderActor->GetActors(pc);
  this->ArrowActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->PlaneActor->ReleaseGraphicsResources(win);
  this->PlaneBorderActor->ReleaseGraphicsResources(win);
  this->ArrowActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerPlaneRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = Superclass::RenderOverlay(viewport);
  if (this->PlaneActor->GetVisibility())
    {
    count +=  this->PlaneActor->RenderOverlay(viewport);
    }
  if (this->PlaneBorderActor->GetVisibility())
    {
    count +=  this->PlaneBorderActor->RenderOverlay(viewport);
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
  if (this->PlaneActor->GetVisibility())
    {
    count += this->PlaneActor->RenderOpaqueGeometry(viewport);
    }
  if (this->PlaneBorderActor->GetVisibility())
    {
    count += this->PlaneBorderActor->RenderOpaqueGeometry(viewport);
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
  if (this->PlaneActor->GetVisibility())
    {
    count += this->PlaneActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->PlaneBorderActor->GetVisibility())
    {
    count += this->PlaneBorderActor->RenderTranslucentPolygonalGeometry(viewport);
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
  if (this->PlaneActor->GetVisibility() && this->PlaneActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->PlaneBorderActor->GetVisibility() && this->PlaneBorderActor->HasTranslucentPolygonalGeometry())
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

  if (this->PlaneActor)
    {
    os << indent << "Plane Actor Visibility: " << this->PlaneActor->GetVisibility() << "\n";
    }
  else if (this->PlaneBorderActor)
    {
    os << indent << "Plane Border Actor Visibility: " << this->PlaneBorderActor->GetVisibility() << "\n";
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
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() != 3)
  {
    this->PlaneMapper->SetInputData(vtkNew<vtkPolyData>());
    this->ArrowMapper->SetInputData(vtkNew<vtkPolyData>());
    return;
  }

  double x[3], y[3], z[3] = { 0 };
  markupsNode->GetPlaneAxesWorld(x, y, z);

  if (vtkMath::Norm(x) <= 0.0001 || vtkMath::Norm(y) <= 0.0001 || vtkMath::Norm(z) <= 0.0001)
  {
    this->PlaneMapper->SetInputData(vtkNew<vtkPolyData>());
    this->ArrowMapper->SetInputData(vtkNew<vtkPolyData>());
    return;
  }

  this->PlaneMapper->SetInputConnection(this->PlaneWorldToSliceTransformer->GetOutputPort());
  this->ArrowMapper->SetInputConnection(this->ArrowGlypher->GetOutputPort());

  double origin[3] = { 0.0 };
  markupsNode->GetOriginWorld(origin);

  // Update the plane
  double size[2] = { 0 };
  markupsNode->GetSize(size);
  vtkMath::MultiplyScalar(x, size[0] / 2);
  vtkMath::MultiplyScalar(y, size[1] / 2);

  double planePoint1[3] = { 0 };
  vtkMath::Subtract(origin, x, planePoint1);
  vtkMath::Subtract(planePoint1, y, planePoint1);

  double planePoint2[3] = { 0 };
  vtkMath::Subtract(origin, x, planePoint2);
  vtkMath::Add(planePoint2, y, planePoint2);

  double planePoint3[3] = { 0 };
  vtkMath::Add(origin, x, planePoint3);
  vtkMath::Subtract(planePoint3, y, planePoint3);

  this->PlaneFilter->SetOrigin(planePoint1);
  this->PlaneFilter->SetPoint1(planePoint2);
  this->PlaneFilter->SetPoint2(planePoint3);

  double* arrowVectorSlice = this->WorldToSliceTransform->TransformDoubleVector(z);

  // Update the normal vector
  double epsilon = 0.001;
  if (vtkMath::Dot(this->SlicePlane->GetNormal(), z) > 1 - epsilon)
    {
    this->ArrowFilter->SetGlyphTypeToCircle();
    this->ArrowFilter->SetRotationAngle(0);
    }
  else if (vtkMath::Dot(this->SlicePlane->GetNormal(), z) < -1 + epsilon)
    {
    this->ArrowFilter->SetGlyphTypeToCross();
    this->ArrowFilter->SetRotationAngle(0);
    }
  else
    {
    arrowVectorSlice[2] = 0.0;

    double xVector[3] = { 1,0,0 };
    double yVector[3] = { 0, 1, 0 };
    double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(arrowVectorSlice, xVector));
    if (vtkMath::Dot(arrowVectorSlice, yVector) < 0)
      {
      angle = -angle;
      }

    this->ArrowFilter->SetGlyphTypeToThickArrow();
    this->ArrowFilter->SetRotationAngle(angle);
    }

  double slicePos[2] = { 0 };
  this->GetWorldToSliceCoordinates(origin, slicePos);
  vtkMath::Normalize(arrowVectorSlice);
  vtkMath::MultiplyScalar(arrowVectorSlice, this->ControlPointSize);
  vtkMath::Add(slicePos, arrowVectorSlice, slicePos);

  vtkNew<vtkPoints> arrowPoints;
  arrowPoints->InsertNextPoint(slicePos);

  vtkNew<vtkPolyData> arrowPolyData;
  arrowPolyData->SetPoints(arrowPoints);

  this->ArrowGlypher->SetInputData(arrowPolyData);
  this->ArrowGlypher->SetScaleFactor(this->ControlPointSize*2);

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
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
  planeStartFadeFar->Push(-1*displayNode->GetLineColorFadingStart());
  this->PlaneClipperStartFadeFar->SetClipFunction(planeStartFadeFar);

  vtkNew<vtkPlane> planeEndFadeFar;
  planeEndFadeFar->SetOrigin(this->SlicePlane->GetOrigin());
  planeEndFadeFar->SetNormal(this->SlicePlane->GetNormal());
  planeEndFadeFar->Push(-1*displayNode->GetLineColorFadingEnd());
  this->PlaneClipperEndFadeFar->SetClipFunction(planeEndFadeFar);
}
