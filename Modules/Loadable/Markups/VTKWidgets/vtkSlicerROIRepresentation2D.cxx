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
#include <vtkContourTriangulator.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// Markups VTK widgets includes
#include <vtkSlicerROIRepresentation2D.h>

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"

vtkStandardNewMacro(vtkSlicerROIRepresentation2D);

//----------------------------------------------------------------------
vtkSlicerROIRepresentation2D::vtkSlicerROIRepresentation2D()
{
  this->ROISource = nullptr;

  this->ROIToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->ROIToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
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
void vtkSlicerROIRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

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

  this->ROIToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());

  int controlPointType = Selected;

  double opacity = this->MarkupsDisplayNode->GetOpacity();

  double fillOpacity = this->MarkupsDisplayNode->GetFillVisibility()
    ? opacity * this->MarkupsDisplayNode->GetFillOpacity() : 0.0;
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIProperty->SetOpacity(fillOpacity);

  double outlineOpacity = this->MarkupsDisplayNode->GetOutlineVisibility()
    ? opacity * this->MarkupsDisplayNode->GetOutlineOpacity() : 0.0;
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIOutlineProperty->SetOpacity(outlineOpacity);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::SetROISource(vtkPolyDataAlgorithm* roiSource)
{
  this->ROISource = roiSource;
  if (this->ROISource)
    {
    this->ROIToWorldTransformFilter->SetInputConnection(roiSource->GetOutputPort());
    }
  else
    {
    this->ROIToWorldTransformFilter->RemoveAllInputConnections(0);
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

  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSize(sideLengths);
  cubeSource->SetXLength(sideLengths[0]);
  cubeSource->SetYLength(sideLengths[1]);
  cubeSource->SetZLength(sideLengths[2]);

  this->ROIOutlineActor->SetVisibility(vtkMath::Norm(sideLengths) >= 1e-6);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->ROIActor->GetActors(pc);
  this->ROIOutlineActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->ROIActor->ReleaseGraphicsResources(win);
  this->ROIOutlineActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerROIRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
    {
    count +=  this->ROIActor->RenderOverlay(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    count +=  this->ROIOutlineActor->RenderOverlay(viewport);
    }
  count += Superclass::RenderOverlay(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerROIRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
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
int vtkSlicerROIRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
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
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipelineROI2D(this);
  this->InteractionPipeline->InitializePipeline();
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::UpdateInteractionPipeline()
{
  Superclass::UpdateInteractionPipeline();
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode || !this->MarkupsDisplayNode || !this->ROISource)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  // To avoid "No input data" error messages from vtkTransformPolyDataFilter, we need to check that the input is not empty.
  this->ROIOutlineCutter->Update();
  vtkPolyData* roiPolyData = vtkPolyData::SafeDownCast(this->ROISource->GetOutput());
  vtkPolyData* outlinePolyData = this->ROIOutlineCutter->GetOutput();
  if ((roiPolyData && roiPolyData->GetNumberOfPoints() == 0) || (outlinePolyData && outlinePolyData->GetNumberOfPoints() == 0))
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  this->InteractionPipeline->Actor->SetVisibility(this->MarkupsDisplayNode->GetVisibility()
    && this->MarkupsDisplayNode->GetVisibility3D()
    && this->MarkupsDisplayNode->GetHandlesInteractive());

  vtkNew<vtkTransform> handleToWorldTransform;
  handleToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());
  this->InteractionPipeline->HandleToWorldTransform->DeepCopy(handleToWorldTransform);

  MarkupsInteractionPipelineROI2D* interactionPipeline = static_cast<MarkupsInteractionPipelineROI2D*>(this->InteractionPipeline);
  interactionPipeline->UpdateScaleHandles();
  interactionPipeline->WorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);
}

//-----------------------------------------------------------------------------
vtkSlicerROIRepresentation2D::MarkupsInteractionPipelineROI2D::MarkupsInteractionPipelineROI2D(vtkSlicerMarkupsWidgetRepresentation* representation)
  : MarkupsInteractionPipelineROI(representation)
{
  this->WorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformFilter->SetTransform(vtkNew<vtkTransform>());
  this->WorldToSliceTransformFilter->SetInputConnection(this->HandleToWorldTransformFilter->GetOutputPort());
  this->Mapper->SetInputConnection(this->WorldToSliceTransformFilter->GetOutputPort());
  this->Mapper->SetTransformCoordinate(nullptr);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::MarkupsInteractionPipelineROI2D::GetViewPlaneNormal(double viewPlaneNormal[3])
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

//---------------------------------------------------------------------------
int IntersectWithFinitePlane(double n[3], double o[3],  double pOrigin[3], double px[3], double py[3],
  double x0[3], double x1[3])
{
  // Copied with fixes from vtkPlane in VTK9.
  // Can be replaced when we switch fully to VTK9.

  // Since we are dealing with convex shapes, if there is an intersection a
  // single line is produced as output. So all this is necessary is to
  // intersect the four bounding lines of the finite line and find the two
  // intersection points.
  int numInts = 0;
  double t, * x = x0;
  double xr0[3], xr1[3];

  // First line
  xr0[0] = pOrigin[0];
  xr0[1] = pOrigin[1];
  xr0[2] = pOrigin[2];
  xr1[0] = px[0];
  xr1[1] = px[1];
  xr1[2] = px[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }

  // Second line
  xr1[0] = py[0];
  xr1[1] = py[1];
  xr1[2] = py[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // Third line
  xr0[0] = -pOrigin[0] + px[0] + py[0];
  xr0[1] = -pOrigin[1] + px[1] + py[1];
  xr0[2] = -pOrigin[2] + px[2] + py[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // Fourth and last line
  xr1[0] = px[0];
  xr1[1] = px[1];
  xr1[2] = px[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // No intersection has occurred, or a single degenerate point
  return 0;
}


//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation2D::MarkupsInteractionPipelineROI2D::UpdateScaleHandles()
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(
    vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation)->GetMarkupsNode());
  if (!roiNode)
  {
    return;
  }

  double viewPlaneOrigin4[4] = { 0.0, 0.0, 0.0, 1.0 };
  double viewPlaneNormal4[4] = { 0.0, 0.0, 1.0, 0.0 };
  if (this->Representation)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(this->Representation->GetViewNode());
    if (sliceNode)
      {
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneNormal4, viewPlaneNormal4);
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneOrigin4, viewPlaneOrigin4);
      }
    }

  vtkMatrix4x4* roiToWorldMatrix = roiNode->GetInteractionHandleToWorldMatrix();
  vtkNew<vtkTransform> worldToROITransform;
  worldToROITransform->Concatenate(roiToWorldMatrix);
  worldToROITransform->Inverse();

  double viewPlaneOrigin_ROI[3] = { 0.0, 0.0, 0.0 };
  double viewPlaneNormal_ROI[3] = { 0.0, 0.0, 0.0 };
  worldToROITransform->TransformPoint(viewPlaneOrigin4, viewPlaneOrigin_ROI);
  worldToROITransform->TransformVector(viewPlaneNormal4, viewPlaneNormal_ROI);

  double sideLengths[3] = { 0.0,  0.0, 0.0 };
  roiNode->GetSize(sideLengths);
  vtkMath::MultiplyScalar(sideLengths, 0.5);

  vtkNew<vtkPoints> roiPoints;
  roiPoints->SetNumberOfPoints(6);

  vtkIdTypeArray* visibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  visibilityArray->SetNumberOfValues(roiPoints->GetNumberOfPoints());
  visibilityArray->Fill(1);

  vtkNew<vtkPlane> plane;
  plane->SetNormal(viewPlaneNormal_ROI);
  plane->SetOrigin(viewPlaneOrigin_ROI);

  // Left face handle
  double lFacePoint_ROI[3] =  { -sideLengths[0], -sideLengths[1], -sideLengths[2] };
  double lFacePointX_ROI[3] = { -sideLengths[0],  sideLengths[1], -sideLengths[2] };
  double lFacePointY_ROI[3] = { -sideLengths[0], -sideLengths[1],  sideLengths[2] };
  double lFaceIntersection0_ROI[3] = { 0.0, 0.0, 0.0 };
  double lFaceIntersection1_ROI[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_ROI, viewPlaneOrigin_ROI,
    lFacePoint_ROI, lFacePointX_ROI, lFacePointY_ROI, lFaceIntersection0_ROI, lFaceIntersection1_ROI))
    {
    vtkMath::Add(lFaceIntersection0_ROI, lFaceIntersection1_ROI, lFacePoint_ROI);
    vtkMath::MultiplyScalar(lFacePoint_ROI, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visibile.
    visibilityArray->SetValue(vtkMRMLMarkupsROINode::HandleLFace, false);
    }
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleLFace, lFacePoint_ROI);

  // Right face handle
  double rFacePoint_ROI[3] =  { sideLengths[0], -sideLengths[1], -sideLengths[2] };
  double rFacePointX_ROI[3] = { sideLengths[0],  sideLengths[1], -sideLengths[2] };
  double rFacePointY_ROI[3] = { sideLengths[0], -sideLengths[1],  sideLengths[2] };
  double rFaceIntersection0_ROI[3] = { 0.0, 0.0, 0.0 };
  double rFaceIntersection1_ROI[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_ROI, viewPlaneOrigin_ROI,
    rFacePoint_ROI, rFacePointX_ROI, rFacePointY_ROI, rFaceIntersection0_ROI, rFaceIntersection1_ROI))
    {
    vtkMath::Add(rFaceIntersection0_ROI, rFaceIntersection1_ROI, rFacePoint_ROI);
    vtkMath::MultiplyScalar(rFacePoint_ROI, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visibile.
    visibilityArray->SetValue(vtkMRMLMarkupsROINode::HandleRFace, false);
    }
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleRFace, rFacePoint_ROI);

  // Posterior face handle
  double pFacePoint_ROI[3] =  { -sideLengths[0], -sideLengths[1], -sideLengths[2] };
  double pFacePointX_ROI[3] = {  sideLengths[0], -sideLengths[1], -sideLengths[2] };
  double pFacePointY_ROI[3] = { -sideLengths[0], -sideLengths[1],  sideLengths[2] };
  double pFaceIntersection0_ROI[3] = { 0.0, 0.0, 0.0 };
  double pFaceIntersection1_ROI[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_ROI, viewPlaneOrigin_ROI,
    pFacePoint_ROI, pFacePointX_ROI, pFacePointY_ROI, pFaceIntersection0_ROI, pFaceIntersection1_ROI))
    {
    vtkMath::Add(pFaceIntersection0_ROI, pFaceIntersection1_ROI, pFacePoint_ROI);
    vtkMath::MultiplyScalar(pFacePoint_ROI, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visibile.
    visibilityArray->SetValue(vtkMRMLMarkupsROINode::HandlePFace, false);
    }
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandlePFace, pFacePoint_ROI);

  // Anterior face handle
  double aFacePoint_ROI[3] =  { -sideLengths[0],  sideLengths[1], -sideLengths[2] };
  double aFacePointX_ROI[3] = {  sideLengths[0],  sideLengths[1], -sideLengths[2] };
  double aFacePointY_ROI[3] = { -sideLengths[0],  sideLengths[1],  sideLengths[2] };
  double aFaceIntersection0_ROI[3] = { 0.0, 0.0, 0.0 };
  double aFaceIntersection1_ROI[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_ROI, viewPlaneOrigin_ROI,
    aFacePoint_ROI, aFacePointX_ROI, aFacePointY_ROI, aFaceIntersection0_ROI, aFaceIntersection1_ROI))
    {
    vtkMath::Add(aFaceIntersection0_ROI, aFaceIntersection1_ROI, aFacePoint_ROI);
    vtkMath::MultiplyScalar(aFacePoint_ROI, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visibile.
    visibilityArray->SetValue(vtkMRMLMarkupsROINode::HandleAFace, false);
    }
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleAFace, aFacePoint_ROI);

  // Inferior face handle
  double iFacePoint_ROI[3] =  { -sideLengths[0], -sideLengths[1], -sideLengths[2] };
  double iFacePointX_ROI[3] = {  sideLengths[0], -sideLengths[1], -sideLengths[2] };
  double iFacePointY_ROI[3] = { -sideLengths[0],  sideLengths[1], -sideLengths[2] };
  double iFaceIntersection0_ROI[3] = { 0.0, 0.0, 0.0 };
  double iFaceIntersection1_ROI[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_ROI, viewPlaneOrigin_ROI,
    iFacePoint_ROI, iFacePointX_ROI, iFacePointY_ROI, iFaceIntersection0_ROI, iFaceIntersection1_ROI))
    {
    vtkMath::Add(iFaceIntersection0_ROI, iFaceIntersection1_ROI, iFacePoint_ROI);
    vtkMath::MultiplyScalar(iFacePoint_ROI, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visibile.
    visibilityArray->SetValue(vtkMRMLMarkupsROINode::HandleIFace, false);
    }
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleIFace, iFacePoint_ROI);

  // Superior face handle
  double sFacePoint_ROI[3] =  { -sideLengths[0], -sideLengths[1], sideLengths[2] };
  double sFacePointX_ROI[3] = {  sideLengths[0], -sideLengths[1], sideLengths[2] };
  double sFacePointY_ROI[3] = { -sideLengths[0],  sideLengths[1], sideLengths[2] };
  double sFaceIntersection0_ROI[3] = { 0.0, 0.0, 0.0 };
  double sFaceIntersection1_ROI[3] = { 0.0, 0.0, 0.0 };
  if (IntersectWithFinitePlane(viewPlaneNormal_ROI, viewPlaneOrigin_ROI,
    sFacePoint_ROI, sFacePointX_ROI, sFacePointY_ROI, sFaceIntersection0_ROI, sFaceIntersection1_ROI))
    {
    vtkMath::Add(sFaceIntersection0_ROI, sFaceIntersection1_ROI, sFacePoint_ROI);
    vtkMath::MultiplyScalar(sFacePoint_ROI, 0.5);
    }
  else
    {
    // Face does not intersect with the slice. Handle should not be visibile.
    visibilityArray->SetValue(vtkMRMLMarkupsROINode::HandleSFace, false);
    }
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleSFace, sFacePoint_ROI);

  vtkNew<vtkTransform> worldToHandleTransform;
  worldToHandleTransform->DeepCopy(this->HandleToWorldTransform);
  worldToHandleTransform->Inverse();

  vtkNew<vtkTransform> roiToHandleTransform;
  roiToHandleTransform->Concatenate(roiToWorldMatrix);
  roiToHandleTransform->Concatenate(worldToHandleTransform);

  vtkNew<vtkPolyData> scaleHandlePoints;
  scaleHandlePoints->SetPoints(roiPoints);

  vtkNew<vtkTransformPolyDataFilter> transformHandlesWorldToHandleFilter;
  transformHandlesWorldToHandleFilter->SetInputData(scaleHandlePoints);
  transformHandlesWorldToHandleFilter->SetTransform(roiToHandleTransform);
  transformHandlesWorldToHandleFilter->Update();

  this->ScaleHandlePoints->SetPoints(transformHandlesWorldToHandleFilter->GetOutput()->GetPoints());
}
