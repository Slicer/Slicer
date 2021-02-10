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

#include "vtkMRMLMarkupsROINode.h"

// MRML includes
#include <vtkMRMLMarkupsROIJsonStorageNode.h>
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

const int NUMBER_OF_BOX_CONTROL_POINTS = 2; // 2 points used for initial ROI definition, then removed
const int NUMBER_OF_BOUNDING_BOX_CONTROL_POINTS = 1e6; // Any number of points

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsROINode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsROINode::vtkMRMLMarkupsROINode()
{
  this->PropertiesLabelText = "";

  this->ROIType = vtkMRMLMarkupsROINode::ROITypeBox;
  this->RequiredNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;

  this->Size[0] = 0.0;
  this->Size[1] = 0.0;
  this->Size[2] = 0.0;

  this->IsUpdatingControlPointsFromROI = false;
  this->IsUpdatingROIFromControlPoints = false;

  this->CurveInputPoly->GetPoints()->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  this->ROIToLocalMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->ROIToLocalMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  this->InteractionHandleToWorldMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  this->InsideOut = false;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsROINode::~vtkMRMLMarkupsROINode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(ROIType);
  vtkMRMLCopyVectorMacro(Size, double, 3);
  vtkMRMLCopyOwnedMatrix4x4Macro(ROIToLocalMatrix);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(ROIType);
  vtkMRMLPrintVectorMacro(Size, double, 3);
  vtkMRMLPrintMatrix4x4Macro(ROIToLocalMatrix);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsROINode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLMarkupsROIJsonStorageNode"));
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsROINode::GetROITypeAsString(int roiType)
{
  switch (roiType)
    {
    case vtkMRMLMarkupsROINode::ROITypeBox:
      return "Box";
    case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
      return "BoundingBox";
    default:
      break;
    }
  return "";
}

//-----------------------------------------------------------
int vtkMRMLMarkupsROINode::GetROITypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLMarkupsROINode::ROIType_Last; i++)
    {
    if (strcmp(name, vtkMRMLMarkupsROINode::GetROITypeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetXAxisWorld(double axis_World[3])
{
  this->GetAxisWorld(0, axis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetYAxisWorld(double axis_World[3])
{
  this->GetAxisWorld(1, axis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetZAxisWorld(double axis_World[3])
{
  this->GetAxisWorld(2, axis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetAxisWorld(int axisIndex, double axis_World[3])
{
  if (axisIndex < 0 || axisIndex >= 3)
    {
    vtkErrorMacro("Invalid axisIndex. Must be 0, 1, or 2.");
    return;
    }

  double axis_Local[3] = { 0.0, 0.0, 0.0 };
  this->GetAxisLocal(axisIndex, axis_Local);

  vtkNew<vtkGeneralTransform> localToWorldTransform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, localToWorldTransform);
  double center_Local[3] = { 0.0, 0.0, 0.0 };
  this->GetCenter(center_Local);
  localToWorldTransform->TransformVectorAtPoint(center_Local, axis_Local, axis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetXAxisLocal(double axis_Local[3])
{
  this->GetAxisLocal(0, axis_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetYAxisLocal(double axis_Local[3])
{
  this->GetAxisLocal(1, axis_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetZAxisLocal(double axis_Local[3])
{
  this->GetAxisLocal(2, axis_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetAxisLocal(int axisIndex, double axis_Local[3])
{
  if (axisIndex < 0 || axisIndex >= 3)
    {
    vtkErrorMacro("Invalid axisIndex. Must be 0, 1, or 2.");
    return;
    }

  double axis4_Local[4] = { 0.0, 0.0, 0.0, 0.0 };
  axis4_Local[axisIndex] = 1.0;
  this->ROIToLocalMatrix->MultiplyPoint(axis4_Local, axis4_Local);

  axis_Local[0] = axis4_Local[0];
  axis_Local[1] = axis4_Local[1];
  axis_Local[2] = axis4_Local[2];
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)
{
  Superclass::OnTransformNodeReferenceChanged(transformNode);
  this->UpdateInteractionHandleToWorldMatrix();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller == this->CurveInputPoly->GetPoints() || caller == this->GetParentTransformNode())
    {
    // TODO: Update handles from parent transform
    this->UpdateROIFromControlPoints();
    }
  else if (caller == this->ROIToLocalMatrix.GetPointer() && event == vtkCommand::ModifiedEvent)
    {
    this->UpdateInteractionHandleToWorldMatrix();
    this->Modified();
    }
  else if (caller = this->InteractionHandleToWorldMatrix)
    {
    vtkNew<vtkMatrix4x4> worldToLocalTransform;
    if (this->GetParentTransformNode())
      {
      this->GetParentTransformNode()->GetMatrixTransformFromWorld(worldToLocalTransform);
      }

    vtkNew<vtkTransform> roiToLocalTransform;
    roiToLocalTransform->Concatenate(worldToLocalTransform);
    roiToLocalTransform->Concatenate(this->InteractionHandleToWorldMatrix);
    this->ROIToLocalMatrix->DeepCopy(roiToLocalTransform->GetMatrix());
    this->Modified();
    }
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetROIType(int roiType)
{
  if (this->ROIType == roiType)
    {
    return;
    }

  this->ROIType = roiType;
  switch (roiType)
    {
    case vtkMRMLMarkupsROINode::ROITypeBox:
      this->RequiredNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;
      break;
    case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
      this->RequiredNumberOfControlPoints = NUMBER_OF_BOUNDING_BOX_CONTROL_POINTS;
      break;
    default:
      break;
    }
  this->UpdateROIFromControlPoints();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetCenter(double center_Local[3])
{
  if (!center_Local)
    {
    vtkErrorMacro("GetCenter: Invalid origin argument");
    return;
    }

  double center_Local4[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->ROIToLocalMatrix->MultiplyPoint(center_Local4, center_Local4);

  center_Local[0] = center_Local4[0];
  center_Local[1] = center_Local4[1];
  center_Local[2] = center_Local4[2];
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetCenterWorld(double center_World[3])
{
  if (!center_World)
    {
    vtkErrorMacro("GetCenterWorld: Invalid origin argument");
    return;
    }

  double center_Local[3] = { 0.0, 0.0, 0.0 };
  this->GetCenter(center_Local);
  this->TransformPointToWorld(center_Local, center_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetCenterWorld(const double center_World[3])
{
  if (!center_World)
    {
    vtkErrorMacro("SetCenterWorld: Invalid origin argument");
    return;
    }

  double center_Local[3] = { 0.0, 0.0, 0.0 };
  this->TransformPointFromWorld(center_World, center_Local);
  this->SetCenter(center_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetCenter(const double center_Local[3])
{
  if (!center_Local)
    {
    vtkErrorMacro("SetCenter: Invalid origin argument");
    return;
    }

  if (this->ROIToLocalMatrix->GetElement(0, 3) == center_Local[0] &&
    this->ROIToLocalMatrix->GetElement(1, 3) == center_Local[1] &&
    this->ROIToLocalMatrix->GetElement(2, 3) == center_Local[2])
    {
    return;
    }

  vtkNew<vtkMatrix4x4> newROIToLocalMatrix;
  newROIToLocalMatrix->DeepCopy(this->ROIToLocalMatrix);
  for (int i = 0; i < 3; ++i)
    {
    newROIToLocalMatrix->SetElement(i, 3, center_Local[i]);
    }

  MRMLNodeModifyBlocker blocker(this);
  this->ROIToLocalMatrix->DeepCopy(newROIToLocalMatrix);
  this->UpdateControlPointsFromROI();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetSize(const double center[3])
{
  this->SetSize(center[0], center[1], center[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetSize(double x, double y, double z)
{
  if (this->Size[0] == x && this->Size[1] == y && this->Size[2] == z)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->Size[0] = x;
  this->Size[1] = y;
  this->Size[2] = z;
  this->UpdateControlPointsFromROI();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetBoundsROI(double bounds_ROI[6])
{
  double center[3] = { 0.0, 0.0, 0.0 };
  this->GetSize(center);
  for (int i = 0; i < 3; ++i)
    {
    bounds_ROI[2 * i] = -center[i] * 0.5;
    bounds_ROI[2*i+1] =  center[i] * 0.5;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateROIFromControlPoints()
{
  if (this->IsUpdatingControlPointsFromROI || this->IsUpdatingROIFromControlPoints)
    {
    return;
    }

  this->IsUpdatingROIFromControlPoints = true;

  {
    // Block events in this scope
    MRMLNodeModifyBlocker blocker(this);

    switch (this->ROIType)
    {
    case vtkMRMLMarkupsROINode::ROITypeBox:
      this->UpdateBoxROIFromControlPoints();
      break;
    case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
      this->UpdateBoundingBoxROIFromControlPoints();
      break;
    default:
      break;
    }
  }

  this->IsUpdatingROIFromControlPoints = false;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateBoxROIFromControlPoints()
{
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  if (numberOfControlPoints == 0)
    {
    return;
    }

  this->UpdateBoundingBoxROIFromControlPoints();

  // Find the shortest non-zero roi axis to pad the ROI if we need to give it some thickness.
  double newSize[3] = { 0.0, 0.0, 0.0 };
  this->GetSize(newSize);
  double minimumSize = VTK_DOUBLE_MAX;
  for (int i = 0; i < 3; ++i)
    {
    if (newSize[i] <= 0.0)
      {
      continue;
      }
    minimumSize = std::min(minimumSize, newSize[i]);
    }
  if (minimumSize == VTK_DOUBLE_MAX)
    {
    minimumSize = 0.0;
    }

  // If the requied control points exist, then we should give the ROI some thickness along the 3rd axis.
  if (this->GetNumberOfControlPoints() == NUMBER_OF_BOX_CONTROL_POINTS)
    {
    if (newSize[0] == 0.0)
      {
      newSize[0] = minimumSize;
      }
    if (newSize[1] == 0.0)
      {
      newSize[1] = minimumSize;
      }
    if (newSize[2] == 0.0)
      {
      newSize[2] = minimumSize;
      }
    }
  this->SetSize(newSize);

  // If all of the control points have been defined, then the ROI has been defined. The control points can be removed.
  if (this->GetNumberOfDefinedControlPoints() == NUMBER_OF_BOX_CONTROL_POINTS)
    {
    this->RequiredNumberOfControlPoints = 0; // Leave place mode
    this->RemoveAllControlPoints();
    }
  else
    {
    this->RequiredNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateBoundingBoxROIFromControlPoints()
{
  MRMLNodeModifyBlocker blocker(this);

  double bounds_ROI[6] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, };
  if (this->GetNumberOfControlPoints() == 0)
    {
    for (int i = 0; i < 6; ++i)
      {
      bounds_ROI[i] = 0.0;
      }
    }

  // Calculate the bounding box defined by the control points in ROI coordinates.
  vtkNew<vtkTransform> localToROITransform;
  localToROITransform->SetMatrix(this->ROIToLocalMatrix);
  localToROITransform->Inverse();
  for (int pointIndex = 0; pointIndex < this->GetNumberOfControlPoints(); ++pointIndex)
    {
    double point_Local[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPosition(pointIndex, point_Local);

    double point_ROI[3] = { 0.0, 0.0, 0.0 };
    localToROITransform->TransformPoint(point_Local, point_ROI);
    for (int i = 0; i < 3; ++i)
      {
      bounds_ROI[2 * i] = std::min(bounds_ROI[2 * i], point_ROI[i]);
      bounds_ROI[2 * i + 1] = std::max(bounds_ROI[2 * i + 1], point_ROI[i]);
      }
    }

  double center_ROI[4] = { 0.0, 0.0, 0.0, 1.0 };
  double newSize[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; ++i)
    {
    newSize[i] = bounds_ROI[2 * i + 1] - bounds_ROI[2 * i];
    center_ROI[i] = (bounds_ROI[2 * i + 1] + bounds_ROI[2 * i]) / 2.0;
    }

  this->SetSize(newSize);

  double center_Local[4] = { 0.0, 0.0, 0.0, 0.0 };
  this->ROIToLocalMatrix->MultiplyPoint(center_ROI, center_Local);
  this->SetCenter(center_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateControlPointsFromROI()
{
  if (this->IsUpdatingControlPointsFromROI || this->IsUpdatingROIFromControlPoints)
    {
    return;
    }

  this->IsUpdatingControlPointsFromROI = true;

  {
    // Block events in this scope
    MRMLNodeModifyBlocker blocker(this);

    switch (this->ROIType)
      {
      case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
        this->UpdateControlPointsFromBoundingBoxROI();
        break;
      default:
        break;
      }
  }

  this->IsUpdatingControlPointsFromROI = false;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateControlPointsFromBoundingBoxROI()
{
  vtkNew<vtkTransform> localToROITransform;
  localToROITransform->SetMatrix(this->ROIToLocalMatrix);
  localToROITransform->Inverse();

  double bounds_ROI[6] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };
  for (int pointIndex = 0; pointIndex < this->GetNumberOfControlPoints(); ++pointIndex)
    {
    double point_Local[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPosition(pointIndex, point_Local);

    double point_ROI[3] = { 0.0, 0.0, 0.0 };
    localToROITransform->TransformPoint(point_Local, point_ROI);
    for (int i = 0; i < 3; ++i)
      {
      bounds_ROI[2 * i] = std::min(bounds_ROI[2 * i], point_ROI[i]);
      bounds_ROI[2 * i + 1] = std::max(bounds_ROI[2 * i + 1], point_ROI[i]);
      }
    }
  if (this->GetNumberOfControlPoints() == 0)
    {
    for (int i = 0; i < 6; ++i)
      {
      bounds_ROI[i] = 0.0;
      }
    }

  double scale_ROI[3] = { 1.0, 1.0, 1.0 };
  double translation_ROI[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; ++i)
    {
    double oldSize = bounds_ROI[2 * i + 1] - bounds_ROI[2 * i];
    scale_ROI[i] = this->Size[i] / oldSize;
    translation_ROI[i] = -(bounds_ROI[2 * i + 1] + bounds_ROI[2 * i]) / 2.0;
    }

  // Based on the difference between the side lengths and the bounding box defined by the control points,
  // we apply a transform to scale the control point positions to match the expected ROI dimensions.
  vtkNew<vtkTransform> localToScaledLocalTransform;
  localToScaledLocalTransform->PostMultiply();
  localToScaledLocalTransform->Concatenate(localToROITransform);
  localToScaledLocalTransform->Translate(translation_ROI);
  localToScaledLocalTransform->Scale(scale_ROI);
  localToScaledLocalTransform->Concatenate(this->ROIToLocalMatrix);

  vtkNew<vtkTransformPolyDataFilter> localToScaledLocalTransformFilter;
  localToScaledLocalTransformFilter->SetInputData(this->CurveInputPoly);
  localToScaledLocalTransformFilter->SetTransform(localToScaledLocalTransform);

  vtkNew<vtkGeneralTransform> localToWorldTransform;
  if (this->GetParentTransformNode())
    {
    this->GetParentTransformNode()->GetTransformToWorld(localToWorldTransform);
    }

  vtkNew<vtkTransformPolyDataFilter> localToWorldTransformFilter;
  localToWorldTransformFilter->SetTransform(localToWorldTransform);
  localToWorldTransformFilter->SetInputConnection(localToScaledLocalTransformFilter->GetOutputPort());
  localToWorldTransformFilter->Update();

  this->SetControlPointPositionsWorld(localToWorldTransformFilter->GetOutput()->GetPoints());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateInteractionHandleToWorldMatrix()
{
  vtkNew<vtkMatrix4x4> localToWorldMatrix;
  if (this->GetParentTransformNode())
    {
    this->GetParentTransformNode()->GetMatrixTransformBetweenNodes(this->GetParentTransformNode(), nullptr, localToWorldMatrix);
    }

  // The InteractionHandleToWorld matrix is equivalent to the ROIToWorld matrix.
  vtkNew<vtkTransform> handleToWorldMatrix;
  handleToWorldMatrix->Concatenate(localToWorldMatrix);
  handleToWorldMatrix->Concatenate(this->ROIToLocalMatrix);
  this->InteractionHandleToWorldMatrix->DeepCopy(handleToWorldMatrix->GetMatrix());
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsROINode::GetXYZ(double center[3])
{
  this->GetCenterWorld(center);
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsROINode::SetXYZ(double center[3])
{
  this->SetCenterWorld(center);
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsROINode::SetXYZ(double x, double y, double z)
{
  double tempXYZ[3] = { x, y, z };
  return this->SetXYZ(tempXYZ);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetRadiusXYZ(double radiusXYZ[3])
{
  this->SetSize(radiusXYZ[0] * 2.0, radiusXYZ[1] * 2.0, radiusXYZ[2] * 2.0);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetRadiusXYZ(double x, double y, double z)
{
  double tempXYZ[3] = { x, y, z };
  this->SetRadiusXYZ(tempXYZ);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetRadiusXYZ(double radiusXYZ[3])
{
  double tempSize[3] = { 0.0, 0.0, 0.0 };
  this->GetSize(tempSize);
  radiusXYZ[0] = 0.5 * tempSize[0];
  radiusXYZ[1] = 0.5 * tempSize[1];
  radiusXYZ[2] = 0.5 * tempSize[2];
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetTransformedPlanes(vtkPlanes* planes)
{
  if (!planes)
    {
    vtkErrorMacro("GetTransformedPlanes: Invalid planes");
    return;
    }

  vtkNew<vtkTransform> roiToWorldTransform;
  roiToWorldTransform->SetMatrix(this->ROIToLocalMatrix);

  double center_World[3] = { 0.0, 0.0, 0.0 };
  roiToWorldTransform->TransformPoint(center_World, center_World);

  vtkNew<vtkDoubleArray> normals;
  normals->SetNumberOfComponents(3);

  vtkNew<vtkPoints> points;


  double lNormal_World[3] = { -1.0, 0.0, 0.0 };
  roiToWorldTransform->TransformVector(lNormal_World, lNormal_World);
  vtkMath::MultiplyScalar(lNormal_World, 0.5 * this->Size[0]);
  double lOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(center_World, lNormal_World, lOrigin_World);
  points->InsertNextPoint(lOrigin_World[0], lOrigin_World[1], lOrigin_World[2]);
  normals->InsertNextTuple3(lNormal_World[0], lNormal_World[1], lNormal_World[2]);

  double rNormal_World[3] = { 1.0, 0.0, 0.0 };
  roiToWorldTransform->TransformVector(rNormal_World, rNormal_World);
  vtkMath::MultiplyScalar(rNormal_World, 0.5 * this->Size[0]);
  double rOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(center_World, rNormal_World, rOrigin_World);
  points->InsertNextPoint(rOrigin_World[0], rOrigin_World[1], rOrigin_World[2]);
  normals->InsertNextTuple3(rNormal_World[0], rNormal_World[1], rNormal_World[2]);

  double pNormal_World[3] = { 0.0, -1.0, 0.0 };
  roiToWorldTransform->TransformVector(pNormal_World, pNormal_World);
  vtkMath::MultiplyScalar(pNormal_World, 0.5 * this->Size[1]);
  double pOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(center_World, pNormal_World, pOrigin_World);
  points->InsertNextPoint(pOrigin_World[0], pOrigin_World[1], pOrigin_World[2]);
  normals->InsertNextTuple3(pNormal_World[0], pNormal_World[1], pNormal_World[2]);

  double aNormal_World[3] = { 0.0, 1.0, 0.0 };
  roiToWorldTransform->TransformVector(aNormal_World, aNormal_World);
  vtkMath::MultiplyScalar(aNormal_World, 0.5 * this->Size[1]);
  double aOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(center_World, aNormal_World, aOrigin_World);
  points->InsertNextPoint(aOrigin_World[0], aOrigin_World[1], aOrigin_World[2]);
  normals->InsertNextTuple3(aNormal_World[0], aNormal_World[1], aNormal_World[2]);

  double iNormal_World[3] = { 0.0, 0.0, -1.0 };
  roiToWorldTransform->TransformVector(iNormal_World, iNormal_World);
  vtkMath::MultiplyScalar(iNormal_World, 0.5 * this->Size[2]);
  double iOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(center_World, iNormal_World, iOrigin_World);
  points->InsertNextPoint(iOrigin_World[0], iOrigin_World[1], iOrigin_World[2]);
  normals->InsertNextTuple3(iNormal_World[0], iNormal_World[1], iNormal_World[2]);

  double sNormal_World[3] = { 0.0, 0.0, 1.0 };
  roiToWorldTransform->TransformVector(sNormal_World, sNormal_World);
  vtkMath::MultiplyScalar(sNormal_World, 0.5 * this->Size[2]);
  double sOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(center_World, sNormal_World, sOrigin_World);
  points->InsertNextPoint(sOrigin_World[0], sOrigin_World[1], sOrigin_World[2]);
  normals->InsertNextTuple3(sNormal_World[0], sNormal_World[1], sNormal_World[2]);

  if (this->InsideOut)
    {
    for (int i = 0; i < normals->GetNumberOfTuples(); ++i)
      {
      double* normal = normals->GetTuple3(i);
      normals->SetTuple3(i, -normal[0], -normal[1], -normal[2]);
      }
    }
  planes->SetNormals(normals);
  planes->SetPoints(points);

  vtkNew<vtkGeneralTransform> localToWorldTransform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, localToWorldTransform);
  planes->SetTransform(localToWorldTransform);
}