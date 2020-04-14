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
#include "vtkMRMLMarkupsFiducialStorageNode.h"
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
  this->LocalToPlaneTransform = vtkSmartPointer<vtkMatrix4x4>::New();
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode::~vtkMRMLMarkupsPlaneNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLWriteXMLMatrix4x4Macro(localToPlaneTransform, LocalToPlaneTransform);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLReadXMLOwnedMatrix4x4Macro(localToPlaneTransform, LocalToPlaneTransform);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::Copy(vtkMRMLNode *anode)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::Copy(anode);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(SizeMode);
  vtkMRMLCopyOwnedMatrix4x4Macro(LocalToPlaneTransform);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(SizeMode);
  vtkMRMLPrintMatrix4x4Macro(LocalToPlaneTransform);
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
  this->GetPlaneAxes(x, y, normal);
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
  this->GetPlaneAxesWorld(x, y, normalWorld);
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
  planeToLocalTransform->Concatenate(this->LocalToPlaneTransform);
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
void vtkMRMLMarkupsPlaneNode::GetOrigin(double outOrigin[3])
{
  if (!outOrigin)
    {
    vtkErrorMacro("GetOrigin: Invalid origin argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 1)
    {
    outOrigin[0] = 0;
    outOrigin[1] = 0;
    outOrigin[2] = 0;
    vtkWarningMacro("GetOrigin: Not enough points to define plane origin");
    return;
    }

  outOrigin[0] = 0;
  outOrigin[1] = 0;
  outOrigin[2] = 0;

  vtkNew<vtkTransform> localToPlaneTransform;
  localToPlaneTransform->SetMatrix(this->LocalToPlaneTransform);
  localToPlaneTransform->TransformPoint(outOrigin, outOrigin);

  double origin[3] = { 0 };
  this->GetNthControlPointPosition(0, origin);
  vtkMath::Add(origin, outOrigin, outOrigin);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetOriginWorld(double originWorld[3])
{
  if (!originWorld)
    {
    vtkErrorMacro("GetOriginWorld: Invalid origin argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 1)
    {
    originWorld[0] = 0;
    originWorld[1] = 0;
    originWorld[2] = 0;
    vtkWarningMacro("GetOriginWorld: Not enough points to define plane origin");
    return;
    }

  this->GetOrigin(originWorld);
  this->TransformPointToWorld(originWorld, originWorld);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOrigin(const double origin[3])
{
  if (!origin)
    {
    vtkWarningMacro("SetOrigin: Invalid origin argument");
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  if (this->GetNumberOfControlPoints() < 1)
    {
    this->AddNControlPoints(1);
    }

  double newControlPointPos[3] = { 0, 0, 0 };
  vtkNew<vtkTransform> planeToLocalTransform;
  planeToLocalTransform->SetMatrix(this->LocalToPlaneTransform);
  planeToLocalTransform->Inverse();
  planeToLocalTransform->TransformPoint(origin, newControlPointPos);

  double previousControlPointPos[3] = { 0.0 };
  this->GetNthControlPointPosition(0, previousControlPointPos);

  this->SetNthControlPointPosition(0,
    newControlPointPos[0], newControlPointPos[1], newControlPointPos[2]);

  if (this->GetNumberOfControlPoints() < 3)
    {
    return;
    }

  double displacementVector[3] = { 0.0 };
  vtkMath::Subtract(newControlPointPos, previousControlPointPos, displacementVector);

  for (int i = 1; i < this->GetNumberOfControlPoints(); ++i)
    {
    double currentControlPointPos[3] = { 0.0 };
    this->GetNthControlPointPosition(i, currentControlPointPos);
    vtkMath::Add(currentControlPointPos, displacementVector, currentControlPointPos);
    this->SetNthControlPointPosition(i,
      currentControlPointPos[0], currentControlPointPos[1], currentControlPointPos[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOriginWorld(const double originWorld[3])
{
  double originLocal[3] = { 0 };
  this->TransformPointFromWorld(originWorld, originLocal);
  this->SetOrigin(originLocal);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CalculateAxesFromPoints(const double point0[3], const double point1[3], const double point2[3],
  double x[3], double y[3], double z[3])
{
  vtkMath::Subtract(point1, point0, x);
  vtkMath::Normalize(x);

  double tempVector[3] = { 0 };
  vtkMath::Subtract(point2, point0, tempVector);
  vtkMath::Cross(x, tempVector, z);
  vtkMath::Normalize(z);

  vtkMath::Cross(z, x, y);
  vtkMath::Normalize(y);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneAxes(double x[3], double y[3], double z[3])
{
  if (!x || !y || !z)
    {
    vtkErrorMacro("GetPlaneAxes: Invalid input argument");
    return;
    }

  if (this->GetNumberOfControlPoints() < 3)
    {
    vtkWarningMacro("GetPlaneAxes: Not enough points to define plane axis");
    return;
    }

  double point0[3] = { 0 };
  this->GetNthControlPointPosition(0, point0);
  double point1[3] = { 0 };
  this->GetNthControlPointPosition(1, point1);
  double point2[3] = { 0 };
  this->GetNthControlPointPosition(2, point2);

  vtkNew<vtkTransform> localToPlaneTransform;
  localToPlaneTransform->SetMatrix(this->LocalToPlaneTransform);
  localToPlaneTransform->TransformPoint(point0, point0);
  localToPlaneTransform->TransformPoint(point1, point1);
  localToPlaneTransform->TransformPoint(point2, point2);

  this->CalculateAxesFromPoints(point0, point1, point2, x, y, z);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneAxesWorld(double x[3], double y[3], double z[3])
{
  if (!x || !y || !z)
    {
    vtkErrorMacro("GetPlaneAxesWorld: Invalid input argument");
    return;
    }

  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();

  double point0[3] = { 0 };
  this->GetNthControlPointPosition(0, point0);
  double point1[3] = { 0 };
  this->GetNthControlPointPosition(1, point1);
  double point2[3] = { 0 };
  this->GetNthControlPointPosition(2, point2);

  vtkNew<vtkTransform> localToPlaneTransform;
  localToPlaneTransform->SetMatrix(this->LocalToPlaneTransform);
  localToPlaneTransform->TransformPoint(point0, point0);
  localToPlaneTransform->TransformPoint(point1, point1);
  localToPlaneTransform->TransformPoint(point2, point2);

  if (transformNode)
    {
    // Get transform
    vtkNew<vtkGeneralTransform> transformToWorld;
    transformNode->GetTransformToWorld(transformToWorld.GetPointer());
    transformToWorld->TransformPoint(point0, point0);
    transformToWorld->TransformPoint(point1, point1);
    transformToWorld->TransformPoint(point2, point2);
    }

  this->CalculateAxesFromPoints(point0, point1, point2, x, y, z);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetPlaneAxes(const double inX[3], const double inY[3], const double inZ[3])
{
  if (!inX || !inY || !inZ)
    {
    vtkErrorMacro("SetPlaneAxes: Invalid input axes");
    return;
    }

  double x[3], y[3], z[3] = { 0 };
  vtkNew<vtkTransform> planeToLocalTransform;
  planeToLocalTransform->SetMatrix(this->LocalToPlaneTransform);
  planeToLocalTransform->Inverse();
  planeToLocalTransform->TransformPoint(inX, x);
  planeToLocalTransform->TransformPoint(inY, y);
  planeToLocalTransform->TransformPoint(inZ, z);

  double epsilon = 0.0001;

  double tempX[3], tempY[3], tempZ[3] = { 0 };
  vtkMath::Cross(y, z, tempX);
  vtkMath::Cross(z, x, tempY);
  vtkMath::Cross(x, y, tempZ);
  if (vtkMath::Dot(tempX, x) <= 1 - epsilon ||
      vtkMath::Dot(tempY, y) <= 1 - epsilon ||
      vtkMath::Dot(tempZ, z) <= 1 - epsilon)
    {
    vtkErrorMacro("SetPlaneAxes: Invalid direction vectors!");
    return;
    }

  if (vtkMath::Dot(x, y) >= epsilon || vtkMath::Dot(y, z) >= epsilon || vtkMath::Dot(z, x) >= epsilon)
    {
    vtkErrorMacro("SetPlaneAxes: Invalid vectors");
    }

  MRMLNodeModifyBlocker blocker(this);
  this->CreatePlane();

  double oldX[3], oldY[3], oldZ[3] = { 0 };
  this->GetPlaneAxes(oldX, oldY, oldZ);

  vtkNew<vtkMatrix4x4> oldVectorsToIdentity;
  for (int i = 0; i < 3; ++i)
    {
    oldVectorsToIdentity->SetElement(i, 0, oldX[i]);
    oldVectorsToIdentity->SetElement(i, 1, oldY[i]);
    oldVectorsToIdentity->SetElement(i, 2, oldZ[i]);
    }
  oldVectorsToIdentity->Invert();

  vtkNew<vtkMatrix4x4> identityToNewVectors;
  for (int i = 0; i < 3; ++i)
    {
    identityToNewVectors->SetElement(i, 0, x[i]);
    identityToNewVectors->SetElement(i, 1, y[i]);
    identityToNewVectors->SetElement(i, 2, z[i]);
    }

  double point0[3] = { 0 };
  this->GetNthControlPointPosition(0, point0);

  vtkNew<vtkTransform> transform;
  transform->Translate(point0);
  transform->Concatenate(oldVectorsToIdentity);
  transform->Concatenate(identityToNewVectors);
  for (int i = 0; i < 3; ++i)
    {
    point0[i] = -1 * point0[i];
    }
  transform->Translate(point0);

  for (int i = 0; i < 3; ++i)
    {
    double controlPoint[4] = { 0, 0, 0, 1 };
    this->GetNthControlPointPosition(i, controlPoint);
    transform->MultiplyPoint(controlPoint, controlPoint);
    this->SetNthControlPointPosition(i, controlPoint[0], controlPoint[1], controlPoint[2]);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetPlaneAxesWorld(const double inX[3], const double inY[3], const double inZ[3])
{
  double x[3] = { inX[0], inX[1], inX[2] };
  double y[3] = { inY[0], inY[1], inY[2] };
  double z[3] = { inZ[0], inZ[1], inZ[2] };

  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();
  if (transformNode)
    {
    // Get transform
    vtkNew<vtkGeneralTransform> transformToWorld;
    transformNode->GetTransformFromWorld(transformToWorld.GetPointer());

    // Convert coordinates
    double origin[3] = { 0 };
    this->GetOriginWorld(origin);
    transformToWorld->TransformVectorAtPoint(origin, x, x);
    transformToWorld->TransformVectorAtPoint(origin, y, y);
    transformToWorld->TransformVectorAtPoint(origin, z, z);
    }
  this->SetPlaneAxes(x, y, z);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetSize(double size[3])
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    size[0] = 0.0;
    size[1] = 0.0;
    size[2] = 0.0;
    return;
    }

  // Size mode auto means we need to recalculate the diameter of the plane from the control points.
  if (this->SizeMode == vtkMRMLMarkupsPlaneNode::SizeModeAuto)
    {
    double point0[3] = { 0.0 };
    double point1[3] = { 0.0 };
    double point2[3] = { 0.0 };
    this->GetNthControlPointPosition(0, point0);
    this->GetNthControlPointPosition(1, point1);
    this->GetNthControlPointPosition(2, point2);

    vtkNew<vtkTransform> localToPlaneTransform;
    localToPlaneTransform->SetMatrix(this->LocalToPlaneTransform);
    localToPlaneTransform->TransformPoint(point0, point0);
    localToPlaneTransform->TransformPoint(point1, point1);
    localToPlaneTransform->TransformPoint(point2, point2);

    double x[3], y[3], z[3] = { 0 };
    this->GetPlaneAxes(x, y, z);

    // Update the plane
    double vector1[3] = { 0 };
    vtkMath::Subtract(point1, point0, vector1);

    double vector2[3] = { 0 };
    vtkMath::Subtract(point2, point0, vector2);

    double point1X = std::abs(vtkMath::Dot(vector1, x));
    double point2X = std::abs(vtkMath::Dot(vector2, x));
    double xMax = std::max({ 0.0, point1X, point2X });

    double point1Y = std::abs(vtkMath::Dot(vector1, y));
    double point2Y = std::abs(vtkMath::Dot(vector2, y));
    double yMax = std::max({ 0.0, point1Y, point2Y });

    this->Size[0] = 2 * xMax * this->AutoSizeScalingFactor;
    this->Size[1] = 2 * yMax * this->AutoSizeScalingFactor;
    this->Size[2] = 0.0;
    }

  for (int i = 0; i < 3; ++i)
    {
    size[i] = this->Size[i];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CreatePlane()
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    this->AddNControlPoints(3 - this->GetNumberOfControlPoints());
    }

  double point0[3], point1[3], point2[3] = { 0 };
  this->GetNthControlPointPosition(0, point0);
  this->GetNthControlPointPosition(1, point1);
  this->GetNthControlPointPosition(2, point2);

  // Check if existing vectors are unique.
  double vector1[3], vector2[3] = { 0 };
  vtkMath::Subtract(point1, point0, vector1);
  vtkMath::Subtract(point2, point0, vector2);

  bool pointChanged = false;
  double epsilon = 0.0001;
  if (vtkMath::Norm(vector1) <= epsilon)
    {
    // Point1 is at same position as point0.
    // Move point1 away in x axis.
    double xVector[3] = { 1,0,0 };
    vtkMath::Add(point1, xVector, point1);
    pointChanged = true;
    }

  if (vtkMath::Norm(vector2) <= epsilon)
    {
    // Point2 is at same position as point0.
    // Move point2 away in y axis.
    double yVector[3] = { 0,1,0 };
    vtkMath::Add(point2, yVector, point2);
    pointChanged = true;
    }

  vtkMath::Subtract(point1, point0, vector1);
  vtkMath::Subtract(point2, point0, vector2);
  if (vtkMath::Dot(vector1, vector2) >= 1 - epsilon)
    {
    // Point1 and point2 are along the same vector from point0.
    // Find a perpendicular vector and move point2.
    double vector[3] = { 0,0,0 };
    vtkMath::Perpendiculars(vector2, vector, nullptr, 0.0);
    vtkMath::Add(point0, vector, point2);
    }

  if (pointChanged)
    {
    this->SetNthControlPointPosition(1, point1[0], point1[1], point1[2]);
    this->SetNthControlPointPosition(2, point2[0], point2[1], point2[2]);
    }
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLMarkupsPlaneNode::GetLocalToPlaneTransform()
{
  return this->LocalToPlaneTransform;
}
