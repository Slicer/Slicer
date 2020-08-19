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

// MRML includes
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>
#include <vtkTriangle.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsPlaneNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode::vtkMRMLMarkupsPlaneNode()
{
  this->MaximumNumberOfControlPoints = 3;
  this->RequiredNumberOfControlPoints = 3;
  this->SizeMode = SizeModeAuto;
  this->AutoSizeScalingFactor = 1.0;
  for (int i = 0; i < 6; ++i)
    {
    this->PlaneBounds[i] = 0.0;
    }
  this->PlaneToPlaneOffsetMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode::~vtkMRMLMarkupsPlaneNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLWriteXMLMatrix4x4Macro(planeToPlaneOffsetMatrix, PlaneToPlaneOffsetMatrix);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLReadXMLOwnedMatrix4x4Macro(planeToPlaneOffsetMatrix, PlaneToPlaneOffsetMatrix);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(SizeMode);
  vtkMRMLCopyOwnedMatrix4x4Macro(PlaneToPlaneOffsetMatrix);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(SizeMode);
  vtkMRMLPrintMatrix4x4Macro(PlaneToPlaneOffsetMatrix);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsPlaneNode::GetSizeModeAsString(int sizeMode)
{
  switch (sizeMode)
    {
  case SizeModeAuto:
    return "auto";
  case SizeModeAbsolute:
    return "absolute";
  default:
    break;
    }
  return "unknown";
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsPlaneNode::GetSizeModeFromString(const char* sizeMode)
{
  for (int i = 0; i < SizeMode_Last; ++i)
    {
    if (strcmp(this->GetSizeModeAsString(i), sizeMode) == 0)
      {
      return i;
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetNormal(double normal[3])
{
  if (!normal)
    {
    vtkErrorMacro("GetNormal: Invalid normal argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 3)
    {
    normal[0] = 0;
    normal[1] = 0;
    normal[2] = 1;
    vtkWarningMacro("GetNormal: Not enough points to define plane");
    return;
    }

  double x[3] = { 0 };
  double y[3] = { 0 };
  this->GetAxes(x, y, normal);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetNormalWorld(double normalWorld[3])
{
  if (!normalWorld)
    {
    vtkErrorMacro("GetNormalWorld: Invalid normal argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 3)
    {
    normalWorld[0] = 0;
    normalWorld[1] = 0;
    normalWorld[2] = 1;
    vtkWarningMacro("GetNormalWorld: Not enough points to define plane");
    return;
    }

  double x[3] = { 0 };
  double y[3] = { 0 };
  this->GetAxesWorld(x, y, normalWorld);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetNormal(const double normal[3])
{
  if (!normal)
    {
    vtkErrorMacro("SetNormal: Invalid normal argument");
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->CreatePlane();

  double newNormal[3] = { normal[0], normal[1], normal[2] };
  vtkMath::Normalize(newNormal);

  double currentNormal[3] = { 0 };
  this->GetNormal(currentNormal);

  double epsilon = 0.0001;
  if (vtkMath::Dot(newNormal, currentNormal) >= 1.0 - epsilon)
    {
    // Normal vectors are equivalent, no change required.
    return;
    }

  vtkNew<vtkTransform> planeToLocalTransform;
  //planeToLocalTransform->Concatenate(this->LocalToPlaneTransform); //TODO
  planeToLocalTransform->Inverse();

  double currentNormalLocal[3] = { 0 };
  planeToLocalTransform->TransformVector(currentNormal, currentNormalLocal);

  double newNormalLocal[3] = { 0 };
  planeToLocalTransform->TransformVector(newNormal, newNormalLocal);

  double angleRadians = vtkMath::AngleBetweenVectors(currentNormalLocal, newNormalLocal);
  double rotationAxis[3] = { 0 };
  vtkMath::Cross(currentNormalLocal, newNormalLocal, rotationAxis);
  if (vtkMath::Norm(rotationAxis) < epsilon)
    {
    // New + old normals are facing opposite directions.
    // Find a perpendicular axis to flip around.
    vtkMath::Perpendiculars(currentNormalLocal, rotationAxis, nullptr, 0);
    }

  vtkNew<vtkTransform> transform;
  double point0[3] = { 0 };
  this->GetNthControlPointPosition(0, point0);
  transform->Translate(point0);
  transform->RotateWXYZ(vtkMath::DegreesFromRadians(angleRadians), rotationAxis);
  for (int i = 0; i < 3; ++i)
    {
    point0[i] = -1 * point0[i];
    }
  transform->Translate(point0);

  double point1[3] = { 0 };
  this->GetNthControlPointPosition(1, point1);
  transform->TransformPoint(point1, point1);
  this->SetNthControlPointPosition(1, point1[0], point1[1], point1[2]);

  double point2[3] = { 0 };
  this->GetNthControlPointPosition(2, point2);
  transform->TransformPoint(point2, point2);
  this->SetNthControlPointPosition(2, point2[0], point2[1], point2[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetNormalWorld(const double inNormal[3])
{
  double normal[3] = { inNormal[0], inNormal[1], inNormal[2] };

  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();
  if (transformNode)
    {
    // Get transform
    vtkNew<vtkGeneralTransform> transformToWorld;
    transformNode->GetTransformFromWorld(transformToWorld.GetPointer());

    // Convert coordinates
    double origin[3] = { 0 };
    this->GetOriginWorld(origin);
    transformToWorld->TransformVectorAtPoint(origin, normal, normal);
    }
  this->SetNormal(normal);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetOrigin(double origin_Local[3])
{
  if (!origin_Local)
    {
    vtkErrorMacro("GetOrigin: Invalid origin argument");
    return;
    }

  origin_Local[0] = 0.0;
  origin_Local[1] = 0.0;
  origin_Local[2] = 0.0;

  if (this->GetNumberOfControlPoints() < 1)
    {
    vtkWarningMacro("GetOrigin: Not enough points to define plane origin");
    return;
    }

  double origin_Plane[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkMatrix4x4> planeToLocalMatrix;
  this->GetPlaneToLocalMatrix(planeToLocalMatrix);

  vtkNew<vtkTransform> planeToLocalTransform;
  planeToLocalTransform->SetMatrix(planeToLocalMatrix);
  planeToLocalTransform->TransformPoint(origin_Plane, origin_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetOriginWorld(double origin_World[3])
{
  if (!origin_World)
    {
    vtkErrorMacro("GetOriginWorld: Invalid origin argument");
    return;
    }

  origin_World[0] = 0.0;
  origin_World[1] = 0.0;
  origin_World[2] = 0.0;

  if (this->GetNumberOfControlPoints() < 1)
    {
    vtkWarningMacro("GetOriginWorld: Not enough points to define plane origin");
    return;
    }

  double origin_Plane[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkMatrix4x4> planeToWorldMatrix;
  this->GetPlaneToWorldMatrix(planeToWorldMatrix);

  vtkNew<vtkTransform> planeToWorldTransform;
  planeToWorldTransform->SetMatrix(planeToWorldMatrix);
  planeToWorldTransform->TransformPoint(origin_Plane, origin_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOrigin(const double origin_Local[3])
{
  if (!origin_Local)
    {
    vtkWarningMacro("SetOrigin: Invalid origin argument");
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  if (this->GetNumberOfControlPoints() < 1)
    {
    this->AddNControlPoints(1);
    }

  double previousOrigin_Local[3] = { 0.0, 0.0, 0.0 };
  this->GetOrigin(previousOrigin_Local);

  double displacementVector_Local[3] = { 0.0 };
  vtkMath::Subtract(origin_Local, previousOrigin_Local, displacementVector_Local);

  for (int i = 0; i < this->GetNumberOfControlPoints(); ++i)
    {
    double position_Local[3] = { 0.0 };
    this->GetNthControlPointPosition(i, position_Local);
    vtkMath::Add(position_Local, displacementVector_Local, position_Local);
    this->SetNthControlPointPosition(i, position_Local[0], position_Local[1], position_Local[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOriginWorld(const double origin_World[3])
{
  double origin_Local[3] = { 0.0 };
  this->TransformPointFromWorld(origin_World, origin_Local);
  this->SetOrigin(origin_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CalculateAxesFromPoints(const double point0[3], const double point1[3], const double point2[3],
  double x[3], double y[3], double z[3])
{
  vtkMath::Subtract(point1, point0, x);
  vtkMath::Normalize(x);

  double tempVector[3] = { 0.0 };
  vtkMath::Subtract(point2, point0, tempVector);
  vtkMath::Cross(x, tempVector, z);
  vtkMath::Normalize(z);

  vtkMath::Cross(z, x, y);
  vtkMath::Normalize(y);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneToLocalMatrix(vtkMatrix4x4* planeToLocalMatrix)
{
  if (!planeToLocalMatrix)
    {
    return;
    }

  if (this->GetNumberOfControlPoints() < 1)
    {
    return;
    }

  double point0_Local[3] = { 0.0 };
  this->GetNthControlPointPosition(0, point0_Local);

  vtkNew<vtkMatrix4x4> planeOffsetToLocalMatrix;
  for (int i = 0; i < 3; ++i)
    {
    planeOffsetToLocalMatrix->SetElement(i, 3, point0_Local[i]);
    }

  if (this->GetNumberOfControlPoints() >= 3)
    {
    double point1_Local[3] = { 0.0 };
    double point2_Local[3] = { 0.0 };

    this->GetNthControlPointPosition(1, point1_Local);
    this->GetNthControlPointPosition(2, point2_Local);

    double xAxis_Local[3] = { 0.0 };
    double yAxis_Local[3] = { 0.0 };
    double zAxis_Local[3] = { 0.0 };
    this->CalculateAxesFromPoints(point0_Local, point1_Local, point2_Local, xAxis_Local, yAxis_Local, zAxis_Local);
    for (int i = 0; i < 3; ++i)
      {
      planeOffsetToLocalMatrix->SetElement(i, 0, xAxis_Local[i]);
      planeOffsetToLocalMatrix->SetElement(i, 1, yAxis_Local[i]);
      planeOffsetToLocalMatrix->SetElement(i, 2, zAxis_Local[i]);
      }
    }

  vtkNew<vtkTransform> planeToLocalTransform;
  planeToLocalTransform->PostMultiply();
  planeToLocalTransform->Concatenate(this->PlaneToPlaneOffsetMatrix);
  planeToLocalTransform->Concatenate(planeOffsetToLocalMatrix);
  planeToLocalMatrix->DeepCopy(planeToLocalTransform->GetMatrix());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneToWorldMatrix(vtkMatrix4x4* planeToWorldMatrix)
{
  if (!planeToWorldMatrix)
    {
    return;
    }

  if (this->GetNumberOfControlPoints() < 1)
    {
    return;
    }

  double point0_World[3] = { 0.0 };
  this->GetNthControlPointPositionWorld(0, point0_World);

  vtkNew<vtkMatrix4x4> planeOffsetToWorldMatrix;
  for (int i = 0; i < 3; ++i)
    {
    planeOffsetToWorldMatrix->SetElement(i, 3, point0_World[i]);
    }

  if (this->GetNumberOfControlPoints() >= 3)
    {
    double point1_World[3] = { 0.0 };
    double point2_World[3] = { 0.0 };

    this->GetNthControlPointPositionWorld(1, point1_World);
    this->GetNthControlPointPositionWorld(2, point2_World);

    double xAxis_World[3] = { 0.0 };
    double yAxis_World[3] = { 0.0 };
    double zAxis_World[3] = { 0.0 };
    this->CalculateAxesFromPoints(point0_World, point1_World, point2_World, xAxis_World, yAxis_World, zAxis_World);
    for (int i = 0; i < 3; ++i)
      {
      planeOffsetToWorldMatrix->SetElement(i, 0, xAxis_World[i]);
      planeOffsetToWorldMatrix->SetElement(i, 1, yAxis_World[i]);
      planeOffsetToWorldMatrix->SetElement(i, 2, zAxis_World[i]);
      }
    }

  vtkNew<vtkTransform> planeToLocalTransform;
  planeToLocalTransform->PostMultiply();
  planeToLocalTransform->Concatenate(this->PlaneToPlaneOffsetMatrix);
  planeToLocalTransform->Concatenate(planeOffsetToWorldMatrix);
  planeToWorldMatrix->DeepCopy(planeToLocalTransform->GetMatrix());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetAxes(double xAxis_Local[3], double yAxis_Local[3], double zAxis_Local[3])
{
  if (!xAxis_Local || !yAxis_Local || !zAxis_Local)
    {
    vtkErrorMacro("GetAxes: Invalid input argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 3)
    {
    vtkWarningMacro("GetAxes: Not enough points to define plane axis");
    return;
    }

  for (int i = 0; i < 3; ++i)
    {
    xAxis_Local[i] = 0.0;
    yAxis_Local[i] = 0.0;
    zAxis_Local[i] = 0.0;
    }
  xAxis_Local[0] = 1.0;
  yAxis_Local[1] = 1.0;
  zAxis_Local[2] = 1.0;

  double xAxis_Plane[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Plane[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Plane[3] = { 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> planeToLocalMatrix;
  this->GetPlaneToLocalMatrix(planeToLocalMatrix);

  vtkNew<vtkTransform> planeToLocalTransform;
  planeToLocalTransform->SetMatrix(planeToLocalMatrix);
  planeToLocalTransform->TransformVector(xAxis_Plane, xAxis_Local);
  planeToLocalTransform->TransformVector(yAxis_Plane, yAxis_Local);
  planeToLocalTransform->TransformVector(zAxis_Plane, zAxis_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetAxesWorld(double xAxis_World[3], double yAxis_World[3], double zAxis_World[3])
{
  if (!xAxis_World || !yAxis_World || !zAxis_World)
    {
    vtkErrorMacro("GetAxesWorld: Invalid input argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 3)
    {
    vtkWarningMacro("GetAxes: Not enough points to define plane axis");
    return;
    }

  for (int i = 0; i < 3; ++i)
    {
    xAxis_World[i] = 0.0;
    yAxis_World[i] = 0.0;
    zAxis_World[i] = 0.0;
    }
  xAxis_World[0] = 1.0;
  yAxis_World[1] = 1.0;
  zAxis_World[2] = 1.0;

  double xAxis_Plane[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Plane[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Plane[3] = { 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> planeToWorldMatrix;
  this->GetPlaneToWorldMatrix(planeToWorldMatrix);

  vtkNew<vtkTransform> planeToWorldTransform;
  planeToWorldTransform->SetMatrix(planeToWorldMatrix);
  planeToWorldTransform->TransformVector(xAxis_Plane, xAxis_World);
  planeToWorldTransform->TransformVector(yAxis_Plane, yAxis_World);
  planeToWorldTransform->TransformVector(zAxis_Plane, zAxis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetAxes(const double xAxis_Local[3], const double yAxis_Local[3], const double zAxis_Local[3])
{
  if (!xAxis_Local || !yAxis_Local || !zAxis_Local)
    {
    vtkErrorMacro("SetAxes: Invalid input axes");
    return;
    }

  double epsilon = 1e-5;
  double tempX[3] = { 0.0 };
  double tempY[3] = { 0.0 };
  double tempZ[3] = { 0.0 };
  vtkMath::Cross(yAxis_Local, zAxis_Local, tempX);
  vtkMath::Cross(zAxis_Local, xAxis_Local, tempY);
  vtkMath::Cross(xAxis_Local, yAxis_Local, tempZ);
  if (vtkMath::Dot(tempX, xAxis_Local) <= 1.0 - epsilon ||
      vtkMath::Dot(tempY, yAxis_Local) <= 1.0 - epsilon ||
      vtkMath::Dot(tempZ, zAxis_Local) <= 1.0 - epsilon)
    {
    vtkErrorMacro("SetAxes: Invalid direction vectors!");
    return;
    }

  if (vtkMath::Dot(xAxis_Local, yAxis_Local) >= epsilon ||
      vtkMath::Dot(yAxis_Local, zAxis_Local) >= epsilon ||
      vtkMath::Dot(zAxis_Local, xAxis_Local) >= epsilon)
    {
    vtkErrorMacro("SetAxes: Invalid vectors");
    }

  MRMLNodeModifyBlocker blocker(this);
  this->CreatePlane();

  double previousXAxis_Local[3] = { 0.0 };
  double previousYAxis_Local[3] = { 0.0 };
  double previousZAxis_Local[3] = { 0.0 };
  this->GetAxes(previousXAxis_Local, previousYAxis_Local, previousZAxis_Local);

  vtkNew<vtkMatrix4x4> previousAxisToIdentity;
  for (int i = 0; i < 3; ++i)
    {
    previousAxisToIdentity->SetElement(i, 0, previousXAxis_Local[i]);
    previousAxisToIdentity->SetElement(i, 1, previousYAxis_Local[i]);
    previousAxisToIdentity->SetElement(i, 2, previousZAxis_Local[i]);
    }
  previousAxisToIdentity->Invert();

  vtkNew<vtkMatrix4x4> identityToNewAxis;
  for (int i = 0; i < 3; ++i)
    {
    identityToNewAxis->SetElement(i, 0, xAxis_Local[i]);
    identityToNewAxis->SetElement(i, 1, yAxis_Local[i]);
    identityToNewAxis->SetElement(i, 2, zAxis_Local[i]);
    }

  double point0_Local[3] = { 0 };
  this->GetNthControlPointPosition(0, point0_Local);

  vtkNew<vtkTransform> transform;
  transform->PostMultiply();
  for (int i = 0; i < 3; ++i)
    {
    point0_Local[i] = -1 * point0_Local[i];
    }
  transform->Translate(point0_Local);
  transform->Concatenate(previousAxisToIdentity);
  transform->Concatenate(identityToNewAxis);
  for (int i = 0; i < 3; ++i)
    {
    point0_Local[i] = -1 * point0_Local[i];
    }
  transform->Translate(point0_Local);

  for (int i = 0; i < 3; ++i)
    {
    double point_Local[4] = { 0, 0, 0, 1 };
    this->GetNthControlPointPosition(i, point_Local);
    transform->MultiplyPoint(point_Local, point_Local);
    this->SetNthControlPointPosition(i, point_Local[0], point_Local[1], point_Local[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetAxesWorld(const double xAxis_World[3], const double yAxis_World[3], const double zAxis_World[3])
{
  double xAxis_Local[3] = { xAxis_World[0], xAxis_World[1], xAxis_World[2] };
  double yAxis_Local[3] = { yAxis_World[0], yAxis_World[1], yAxis_World[2] };
  double zAxis_Local[3] = { zAxis_World[0], zAxis_World[1], zAxis_World[2] };

  MRMLNodeModifyBlocker blocker(this);
  this->CreatePlane();

  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();
  if (transformNode)
    {
    // Get transform
    vtkNew<vtkGeneralTransform> worldToLocalTransform;
    transformNode->GetTransformFromWorld(worldToLocalTransform.GetPointer());

    // Convert coordinates
    double origin_World[3] = { 0 };
    this->GetOriginWorld(origin_World);
    worldToLocalTransform->TransformVectorAtPoint(origin_World, xAxis_World, xAxis_Local);
    worldToLocalTransform->TransformVectorAtPoint(origin_World, yAxis_World, yAxis_Local);
    worldToLocalTransform->TransformVectorAtPoint(origin_World, zAxis_World, zAxis_Local);
    }
  this->SetAxes(xAxis_Local, yAxis_Local, zAxis_Local);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneBounds(double PlaneBounds[6])
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    for (int i = 0; i < 6; ++i)
      {
      PlaneBounds[i] = 0.0;
      }
    return;
    }

  // Size mode auto means we need to recalculate the diameter of the plane from the control points.
  if (this->SizeMode == vtkMRMLMarkupsPlaneNode::SizeModeAuto)
    {
    double point0_Local[3] = { 0.0 };
    double point1_Local[3] = { 0.0 };
    double point2_Local[3] = { 0.0 };
    this->GetNthControlPointPosition(0, point0_Local);
    this->GetNthControlPointPosition(1, point1_Local);
    this->GetNthControlPointPosition(2, point2_Local);

    double point0_Plane[3] = { 0.0 };
    double point1_Plane[3] = { 0.0 };
    double point2_Plane[3] = { 0.0 };
    vtkNew<vtkTransform> localToPlaneTransform;
    localToPlaneTransform->SetMatrix(this->PlaneToPlaneOffsetMatrix); // TODO
    localToPlaneTransform->TransformPoint(point0_Local, point0_Plane);
    localToPlaneTransform->TransformPoint(point1_Local, point1_Plane);
    localToPlaneTransform->TransformPoint(point2_Local, point2_Plane);

    double xAxis_Local[3] = { 0.0 };
    double yAxis_Local[3] = { 0.0 };
    double zAxis_Local[3] = { 0.0 };
    this->GetAxes(xAxis_Local, yAxis_Local, zAxis_Local);

    // Update the plane
    double vectorPoint0ToPoint1_Plane[3] = { 0 };
    vtkMath::Subtract(point1_Plane, point0_Plane, vectorPoint0ToPoint1_Plane);

    double vectorPoint0ToPoint2_Plane[3] = { 0 };
    vtkMath::Subtract(point2_Plane, point0_Plane, vectorPoint0ToPoint2_Plane);

    double point1X = std::abs(vtkMath::Dot(vectorPoint0ToPoint1_Plane, xAxis_Local));
    double point2X = std::abs(vtkMath::Dot(vectorPoint0ToPoint2_Plane, xAxis_Local));
    double xMax = std::max({ 0.0, point1X, point2X });

    double point1Y = std::abs(vtkMath::Dot(vectorPoint0ToPoint1_Plane, yAxis_Local));
    double point2Y = std::abs(vtkMath::Dot(vectorPoint0ToPoint2_Plane, yAxis_Local));
    double yMax = std::max({ 0.0, point1Y, point2Y });

    this->PlaneBounds[0] = xMax * this->AutoSizeScalingFactor * -1.0;
    this->PlaneBounds[1] = xMax * this->AutoSizeScalingFactor;
    this->PlaneBounds[2] = yMax * this->AutoSizeScalingFactor * -1.0;
    this->PlaneBounds[3] = yMax * this->AutoSizeScalingFactor;
    this->PlaneBounds[4] = 0.0;
    this->PlaneBounds[5] = 0.0;
    }

  for (int i = 0; i < 6; ++i)
    {
    PlaneBounds[i] = this->PlaneBounds[i];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CreatePlane()
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    this->AddNControlPoints(3 - this->GetNumberOfControlPoints());
    }

  double point0_Local[3] = { 0.0 };
  double point1_Local[3] = { 0.0 };
  double point2_Local[3] = { 0.0 };
  this->GetNthControlPointPosition(0, point0_Local);
  this->GetNthControlPointPosition(1, point1_Local);
  this->GetNthControlPointPosition(2, point2_Local);

  // Check if existing vectors are unique.
  double vectorPoint0ToPoint1_Local[3] = { 0.0 };
  double vectorPoint0ToPoint2_Local[3] = { 0.0 };
  vtkMath::Subtract(point1_Local, point0_Local, vectorPoint0ToPoint1_Local);
  vtkMath::Subtract(point2_Local, point0_Local, vectorPoint0ToPoint2_Local);

  bool pointChanged = false;
  double epsilon = 1e-5;
  if (vtkMath::Norm(vectorPoint0ToPoint1_Local) <= epsilon)
    {
    // Point1 is at same position as point0.
    // Move point1 away in x axis.
    double xVector[3] = { 1,0,0 };
    vtkMath::Add(point1_Local, xVector, point1_Local);
    pointChanged = true;
    }

  if (vtkMath::Norm(vectorPoint0ToPoint2_Local) <= epsilon)
    {
    // Point2 is at same position as point0.
    // Move point2 away in y axis.
    double yVector[3] = { 0,1,0 };
    vtkMath::Add(point2_Local, yVector, point2_Local);
    pointChanged = true;
    }

  vtkMath::Subtract(point1_Local, point0_Local, vectorPoint0ToPoint1_Local);
  vtkMath::Subtract(point2_Local, point0_Local, vectorPoint0ToPoint2_Local);
  if (vtkMath::Dot(vectorPoint0ToPoint1_Local, vectorPoint0ToPoint2_Local) >= 1.0 - epsilon)
    {
    // Point1 and point2 are along the same vector from point0.
    // Find a perpendicular vector and move point2.
    double perpendicular_Local[3] = { 0.0 };
    vtkMath::Perpendiculars(vectorPoint0ToPoint2_Local, perpendicular_Local, nullptr, 0.0);
    vtkMath::Add(point0_Local, perpendicular_Local, point2_Local);
    }

  if (pointChanged)
    {
    this->SetNthControlPointPosition(1, point1_Local[0], point1_Local[1], point1_Local[2]);
    this->SetNthControlPointPosition(2, point2_Local[0], point2_Local[1], point2_Local[2]);
    }
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLMarkupsPlaneNode::GetPlaneToPlaneOffsetMatrix()
{
  return this->PlaneToPlaneOffsetMatrix;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateInteractionHandleToWorldMatrix()
{
  double handleX_World[3] = { 0.0 };
  double handleY_World[3] = { 0.0 };
  double handleZ_World[3] = { 0.0 };
  if (this->GetNumberOfControlPoints() < 3)
    {
    return;
    }

  this->GetAxesWorld(handleX_World, handleY_World, handleZ_World);

  double origin_Local[3] = { 0 };
  this->GetOrigin(origin_Local);

  vtkNew<vtkMatrix4x4> handleToWorldMatrix;
  for (int i = 0; i < 3; ++i)
    {
    handleToWorldMatrix->SetElement(i, 0, handleX_World[i]);
    handleToWorldMatrix->SetElement(i, 1, handleY_World[i]);
    handleToWorldMatrix->SetElement(i, 2, handleZ_World[i]);
    handleToWorldMatrix->SetElement(i, 3, origin_Local[i]);
    }
  this->InteractionHandleToWorldMatrix->DeepCopy(handleToWorldMatrix);
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsPlaneNode::GetClosestPointOnPlaneWorld(const double posWorld[3], double closestPosWorld[3], bool infinitePlane/*=true*/)
{
  if (!posWorld)
    {
    vtkErrorMacro("GetClosestPointOnPlaneWorld: Invalid posWorld");
    return 0.0;
    }
  if (!closestPosWorld)
    {
    vtkErrorMacro("GetClosestPointOnPlaneWorld: Invalid closestPosWorld");
    return 0.0;
    }

  vtkNew<vtkMatrix4x4> planeToWorldMatrix;
  this->GetPlaneToWorldMatrix(planeToWorldMatrix);

  vtkNew<vtkMatrix4x4> worldToPlaneMatrix;
  worldToPlaneMatrix->DeepCopy(planeToWorldMatrix);
  worldToPlaneMatrix->Invert();

  double posWorld4[4] = { posWorld[0], posWorld[1], posWorld[2], 1.0 };
  double closestPosPlane4[4] = { 0.0, 0.0, 0.0, 0.0 };
  worldToPlaneMatrix->MultiplyPoint(posWorld4, closestPosPlane4);

  double distanceToPlane = closestPosPlane4[2];
  closestPosPlane4[2] = 0.0; // Project to plane

  if (!infinitePlane)
    {
    double planeBounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    this->GetPlaneBounds(planeBounds);
    for (int i = 0; i < 3; ++i)
      {
      closestPosPlane4[i] = std::max(closestPosPlane4[i], planeBounds[2 * i]);
      closestPosPlane4[i] = std::min(closestPosPlane4[i], planeBounds[2 * i + 1]);
      }
    }

  double closestPosWorld4[4] = { 0.0, 0.0, 0.0, 0.0 };
  planeToWorldMatrix->MultiplyPoint(closestPosPlane4, closestPosWorld4);
  for (int i = 0; i < 3; ++i)
    {
    closestPosWorld[i] = closestPosWorld4[i];
    }
  return distanceToPlane;
}
