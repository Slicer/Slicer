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
#include "vtkMRMLMeasurementArea.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCollection.h>
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
  this->ObjectToBaseMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  // Setup measurements calculated for this markup type
  vtkNew<vtkMRMLMeasurementArea> areaMeasurement;
  areaMeasurement->SetEnabled(false);
  areaMeasurement->SetName("area");
  areaMeasurement->SetInputMRMLNode(this);
  this->Measurements->AddItem(areaMeasurement);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode::~vtkMRMLMarkupsPlaneNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLWriteXMLMatrix4x4Macro(objectToBaseMatrix, ObjectToBaseMatrix);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLReadXMLOwnedMatrix4x4Macro(planeToPlaneOffsetMatrix, ObjectToBaseMatrix); // Backwards compatible with old name
  vtkMRMLReadXMLOwnedMatrix4x4Macro(objectToBaseMatrix, ObjectToBaseMatrix);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(SizeMode);
  vtkMRMLCopyOwnedMatrix4x4Macro(ObjectToBaseMatrix);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(SizeMode);
  vtkMRMLPrintMatrix4x4Macro(ObjectToBaseMatrix);
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

  double newNormal_Node[3] = { normal[0], normal[1], normal[2] };
  vtkMath::Normalize(newNormal_Node);

  double currentNormal_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetNormal(currentNormal_Node);

  double epsilon = 0.0001;
  if (vtkMath::Dot(newNormal_Node, currentNormal_Node) >= 1.0 - epsilon)
    {
    // Normal vectors are equivalent, no change required.
    return;
    }

  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  this->GetObjectToNodeMatrix(objectToNodeMatrix);

  double angleRadians = vtkMath::AngleBetweenVectors(currentNormal_Node, newNormal_Node);
  double rotationAxis_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Cross(currentNormal_Node, newNormal_Node, rotationAxis_Node);
  if (vtkMath::Norm(rotationAxis_Node) < epsilon)
    {
    // New + old normals are facing opposite directions.
    // Find a perpendicular axis to flip around.
    vtkMath::Perpendiculars(currentNormal_Node, rotationAxis_Node, nullptr, 0);
    }

  vtkNew<vtkTransform> oldToNewNormalTransform;
  double origin_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetOrigin(origin_Node);
  oldToNewNormalTransform->Translate(origin_Node);
  oldToNewNormalTransform->RotateWXYZ(vtkMath::DegreesFromRadians(angleRadians), rotationAxis_Node);
  vtkMath::MultiplyScalar(origin_Node, -1.0);
  oldToNewNormalTransform->Translate(origin_Node);

  this->ApplyTransform(oldToNewNormalTransform);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetNormalWorld(const double normal_World[3])
{
  double normal_Node[3] = { normal_World[0], normal_World[1], normal_World[2] };

  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();
  if (transformNode)
    {
    // Get transform
    vtkNew<vtkGeneralTransform> transformToWorld;
    transformNode->GetTransformFromWorld(transformToWorld);

    // Convert coordinates
    double origin_World[3] = { 0 };
    this->GetOriginWorld(origin_World);
    transformToWorld->TransformVectorAtPoint(origin_World, normal_World, normal_Node);
    }
  this->SetNormal(normal_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetOrigin(double origin_Node[3])
{
  if (!origin_Node)
    {
    vtkErrorMacro("GetOrigin: Invalid origin argument");
    return;
    }

  origin_Node[0] = 0.0;
  origin_Node[1] = 0.0;
  origin_Node[2] = 0.0;

  if (this->GetNumberOfControlPoints() < 1)
    {
    vtkWarningMacro("GetOrigin: Not enough points to define plane origin");
    return;
    }

  double origin_Object[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  this->GetObjectToNodeMatrix(objectToNodeMatrix);

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->SetMatrix(objectToNodeMatrix);
  objectToNodeTransform->TransformPoint(origin_Object, origin_Node);
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

  double origin_Object[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);
  objectToWorldTransform->TransformPoint(origin_Object, origin_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOrigin(const double origin_Node[3])
{
  if (!origin_Node)
    {
    vtkWarningMacro("SetOrigin: Invalid origin argument");
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  if (this->GetNumberOfControlPoints() < 1)
    {
    this->AddNControlPoints(1);
    }

  double previousOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetOrigin(previousOrigin_Node);

  double displacementVector_Node[3] = { 0.0 };
  vtkMath::Subtract(origin_Node, previousOrigin_Node, displacementVector_Node);

  vtkNew<vtkTransform> oldToNewOriginTransform;
  oldToNewOriginTransform->Translate(displacementVector_Node);
  this->ApplyTransform(oldToNewOriginTransform);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOriginWorld(const double origin_World[3])
{
  double origin_Node[3] = { 0.0 };
  this->TransformPointFromWorld(origin_World, origin_Node);
  this->SetOrigin(origin_Node);
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
void vtkMRMLMarkupsPlaneNode::GetObjectToNodeMatrix(vtkMatrix4x4* objectToNodeMatrix)
{
  if (!objectToNodeMatrix)
    {
    return;
    }

  if (this->GetNumberOfControlPoints() < 1)
    {
    return;
    }

  double point0_Node[3] = { 0.0 };
  this->GetNthControlPointPosition(0, point0_Node);

  vtkNew<vtkMatrix4x4> baseToNodeMatrix;
  for (int i = 0; i < 3; ++i)
    {
    baseToNodeMatrix->SetElement(i, 3, point0_Node[i]);
    }

  if (this->GetNumberOfControlPoints() >= 3)
    {
    double point1_Node[3] = { 0.0 };
    double point2_Node[3] = { 0.0 };

    this->GetNthControlPointPosition(1, point1_Node);
    this->GetNthControlPointPosition(2, point2_Node);

    double xAxis_Node[3] = { 0.0 };
    double yAxis_Node[3] = { 0.0 };
    double zAxis_Node[3] = { 0.0 };
    this->CalculateAxesFromPoints(point0_Node, point1_Node, point2_Node, xAxis_Node, yAxis_Node, zAxis_Node);
    for (int i = 0; i < 3; ++i)
      {
      baseToNodeMatrix->SetElement(i, 0, xAxis_Node[i]);
      baseToNodeMatrix->SetElement(i, 1, yAxis_Node[i]);
      baseToNodeMatrix->SetElement(i, 2, zAxis_Node[i]);
      }
    }

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->PostMultiply();
  objectToNodeTransform->Concatenate(this->ObjectToBaseMatrix);
  objectToNodeTransform->Concatenate(baseToNodeMatrix);
  objectToNodeMatrix->DeepCopy(objectToNodeTransform->GetMatrix());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetObjectToWorldMatrix(vtkMatrix4x4* objectToWorldMatrix)
{
  if (!objectToWorldMatrix)
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

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->PostMultiply();
  objectToNodeTransform->Concatenate(this->ObjectToBaseMatrix);
  objectToNodeTransform->Concatenate(planeOffsetToWorldMatrix);
  objectToWorldMatrix->DeepCopy(objectToNodeTransform->GetMatrix());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetAxes(double xAxis_Node[3], double yAxis_Node[3], double zAxis_Node[3])
{
  if (!xAxis_Node || !yAxis_Node || !zAxis_Node)
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
    xAxis_Node[i] = 0.0;
    yAxis_Node[i] = 0.0;
    zAxis_Node[i] = 0.0;
    }
  xAxis_Node[0] = 1.0;
  yAxis_Node[1] = 1.0;
  zAxis_Node[2] = 1.0;

  double xAxis_Object[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Object[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Object[3] = { 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  this->GetObjectToNodeMatrix(objectToNodeMatrix);

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->SetMatrix(objectToNodeMatrix);
  objectToNodeTransform->TransformVector(xAxis_Object, xAxis_Node);
  objectToNodeTransform->TransformVector(yAxis_Object, yAxis_Node);
  objectToNodeTransform->TransformVector(zAxis_Object, zAxis_Node);
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

  double xAxis_Object[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Object[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Object[3] = { 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);
  objectToWorldTransform->TransformVector(xAxis_Object, xAxis_World);
  objectToWorldTransform->TransformVector(yAxis_Object, yAxis_World);
  objectToWorldTransform->TransformVector(zAxis_Object, zAxis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetAxes(const double xAxis_Node[3], const double yAxis_Node[3], const double zAxis_Node[3])
{
  if (!xAxis_Node || !yAxis_Node || !zAxis_Node)
    {
    vtkErrorMacro("SetAxes: Invalid input axes");
    return;
    }

  double epsilon = 1e-5;
  double tempX[3] = { 0.0 };
  double tempY[3] = { 0.0 };
  double tempZ[3] = { 0.0 };
  vtkMath::Cross(yAxis_Node, zAxis_Node, tempX);
  vtkMath::Cross(zAxis_Node, xAxis_Node, tempY);
  vtkMath::Cross(xAxis_Node, yAxis_Node, tempZ);
  if (vtkMath::Dot(tempX, xAxis_Node) <= 1.0 - epsilon ||
      vtkMath::Dot(tempY, yAxis_Node) <= 1.0 - epsilon ||
      vtkMath::Dot(tempZ, zAxis_Node) <= 1.0 - epsilon)
    {
    vtkErrorMacro("SetAxes: Invalid direction vectors!");
    return;
    }

  if (vtkMath::Dot(xAxis_Node, yAxis_Node) >= epsilon ||
      vtkMath::Dot(yAxis_Node, zAxis_Node) >= epsilon ||
      vtkMath::Dot(zAxis_Node, xAxis_Node) >= epsilon)
    {
    vtkErrorMacro("SetAxes: Invalid vectors");
    }

  MRMLNodeModifyBlocker blocker(this);
  this->CreatePlane();

  double previousXAxis_Node[3] = { 0.0 };
  double previousYAxis_Node[3] = { 0.0 };
  double previousZAxis_Node[3] = { 0.0 };
  this->GetAxes(previousXAxis_Node, previousYAxis_Node, previousZAxis_Node);

  vtkNew<vtkMatrix4x4> previousAxisToIdentity;
  for (int i = 0; i < 3; ++i)
    {
    previousAxisToIdentity->SetElement(i, 0, previousXAxis_Node[i]);
    previousAxisToIdentity->SetElement(i, 1, previousYAxis_Node[i]);
    previousAxisToIdentity->SetElement(i, 2, previousZAxis_Node[i]);
    }
  previousAxisToIdentity->Invert();

  vtkNew<vtkMatrix4x4> identityToNewAxis;
  for (int i = 0; i < 3; ++i)
    {
    identityToNewAxis->SetElement(i, 0, xAxis_Node[i]);
    identityToNewAxis->SetElement(i, 1, yAxis_Node[i]);
    identityToNewAxis->SetElement(i, 2, zAxis_Node[i]);
    }

  double origin_Node[3] = { 0 };
  this->GetOrigin(origin_Node);

  vtkNew<vtkTransform> oldToNewAxesTransform;
  oldToNewAxesTransform->PostMultiply();
  vtkMath::MultiplyScalar(origin_Node, -1);
  oldToNewAxesTransform->Translate(origin_Node);
  oldToNewAxesTransform->Concatenate(previousAxisToIdentity);
  oldToNewAxesTransform->Concatenate(identityToNewAxis);
  vtkMath::MultiplyScalar(origin_Node, -1);
  oldToNewAxesTransform->Translate(origin_Node);

  this->ApplyTransform(oldToNewAxesTransform);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetAxesWorld(const double xAxis_World[3], const double yAxis_World[3], const double zAxis_World[3])
{
  double xAxis_Node[3] = { xAxis_World[0], xAxis_World[1], xAxis_World[2] };
  double yAxis_Node[3] = { yAxis_World[0], yAxis_World[1], yAxis_World[2] };
  double zAxis_Node[3] = { zAxis_World[0], zAxis_World[1], zAxis_World[2] };

  MRMLNodeModifyBlocker blocker(this);
  this->CreatePlane();

  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();
  if (transformNode)
    {
    // Get transform
    vtkNew<vtkGeneralTransform> worldToNodeTransform;
    transformNode->GetTransformFromWorld(worldToNodeTransform.GetPointer());

    // Convert coordinates
    double origin_World[3] = { 0 };
    this->GetOriginWorld(origin_World);
    worldToNodeTransform->TransformVectorAtPoint(origin_World, xAxis_World, xAxis_Node);
    worldToNodeTransform->TransformVectorAtPoint(origin_World, yAxis_World, yAxis_Node);
    worldToNodeTransform->TransformVectorAtPoint(origin_World, zAxis_World, zAxis_Node);
    }
  this->SetAxes(xAxis_Node, yAxis_Node, zAxis_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneBounds(double planeBounds_Object[6])
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    for (int i = 0; i < 6; ++i)
      {
      planeBounds_Object[i] = 0.0;
      }
    return;
    }

  // Size mode auto means we need to recalculate the diameter of the plane from the control points.
  if (this->SizeMode == vtkMRMLMarkupsPlaneNode::SizeModeAuto)
    {
    double point0_Node[3] = { 0.0, 0.0, 0.0 };
    double point1_Node[3] = { 0.0, 0.0, 0.0 };
    double point2_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPosition(0, point0_Node);
    this->GetNthControlPointPosition(1, point1_Node);
    this->GetNthControlPointPosition(2, point2_Node);

    vtkNew<vtkMatrix4x4> objectToNodeMatrix;
    this->GetObjectToNodeMatrix(objectToNodeMatrix);

    vtkNew<vtkTransform> nodeToObjectTransform;
    nodeToObjectTransform->SetMatrix(objectToNodeMatrix);
    nodeToObjectTransform->Inverse();

    double point0_Object[3] = { 0.0, 0.0, 0.0 };
    double point1_Object[3] = { 0.0, 0.0, 0.0 };
    double point2_Object[3] = { 0.0, 0.0, 0.0 };
    nodeToObjectTransform->TransformPoint(point0_Node, point0_Object);
    nodeToObjectTransform->TransformPoint(point1_Node, point1_Object);
    nodeToObjectTransform->TransformPoint(point2_Node, point2_Object);

    double xMax = std::max({ std::abs(point0_Object[0]), std::abs(point1_Object[0]), std::abs(point2_Object[0]) });
    double yMax = std::max({ std::abs(point0_Object[1]), std::abs(point1_Object[1]), std::abs(point2_Object[1]) });

    this->PlaneBounds[0] = xMax * this->AutoSizeScalingFactor * -1.0;
    this->PlaneBounds[1] = xMax * this->AutoSizeScalingFactor;
    this->PlaneBounds[2] = yMax * this->AutoSizeScalingFactor * -1.0;
    this->PlaneBounds[3] = yMax * this->AutoSizeScalingFactor;
    this->PlaneBounds[4] = 0.0;
    this->PlaneBounds[5] = 0.0;
    }

  for (int i = 0; i < 6; ++i)
    {
    planeBounds_Object[i] = this->PlaneBounds[i];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CreatePlane()
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    this->AddNControlPoints(3 - this->GetNumberOfControlPoints());
    }

  double point0_Node[3] = { 0.0 };
  double point1_Node[3] = { 0.0 };
  double point2_Node[3] = { 0.0 };
  this->GetNthControlPointPosition(0, point0_Node);
  this->GetNthControlPointPosition(1, point1_Node);
  this->GetNthControlPointPosition(2, point2_Node);

  // Check if existing vectors are unique.
  double vectorPoint0ToPoint1_Node[3] = { 0.0 };
  double vectorPoint0ToPoint2_Node[3] = { 0.0 };
  vtkMath::Subtract(point1_Node, point0_Node, vectorPoint0ToPoint1_Node);
  vtkMath::Subtract(point2_Node, point0_Node, vectorPoint0ToPoint2_Node);

  bool pointChanged = false;
  double epsilon = 1e-5;
  if (vtkMath::Norm(vectorPoint0ToPoint1_Node) <= epsilon)
    {
    // Point1 is at same position as point0.
    // Move point1 away in x axis.
    double xVector[3] = { 1,0,0 };
    vtkMath::Add(point1_Node, xVector, point1_Node);
    pointChanged = true;
    }

  if (vtkMath::Norm(vectorPoint0ToPoint2_Node) <= epsilon)
    {
    // Point2 is at same position as point0.
    // Move point2 away in y axis.
    double yVector[3] = { 0,1,0 };
    vtkMath::Add(point2_Node, yVector, point2_Node);
    pointChanged = true;
    }

  vtkMath::Subtract(point1_Node, point0_Node, vectorPoint0ToPoint1_Node);
  vtkMath::Subtract(point2_Node, point0_Node, vectorPoint0ToPoint2_Node);
  if (vtkMath::Dot(vectorPoint0ToPoint1_Node, vectorPoint0ToPoint2_Node) >= 1.0 - epsilon)
    {
    // Point1 and point2 are along the same vector from point0.
    // Find a perpendicular vector and move point2.
    double perpendicular_Node[3] = { 0.0 };
    vtkMath::Perpendiculars(vectorPoint0ToPoint2_Node, perpendicular_Node, nullptr, 0.0);
    vtkMath::Add(point0_Node, perpendicular_Node, point2_Node);
    }

  if (pointChanged)
    {
    this->SetNthControlPointPosition(1, point1_Node[0], point1_Node[1], point1_Node[2]);
    this->SetNthControlPointPosition(2, point2_Node[0], point2_Node[1], point2_Node[2]);
    }
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLMarkupsPlaneNode::GetObjectToBaseMatrix()
{
  return this->ObjectToBaseMatrix;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateInteractionHandleToWorldMatrix()
{
  double handleX_World[3] = { 0.0, 0.0, 0.0 };
  double handleY_World[3] = { 0.0, 0.0, 0.0 };
  double handleZ_World[3] = { 0.0, 0.0, 0.0 };
  if (this->GetNumberOfControlPoints() < 3)
    {
    return;
    }

  this->GetAxesWorld(handleX_World, handleY_World, handleZ_World);

  double origin_World[3] = { 0.0, 0.0, 0.0 };
  this->GetOriginWorld(origin_World);

  vtkNew<vtkMatrix4x4> handleToWorldMatrix;
  for (int i = 0; i < 3; ++i)
    {
    handleToWorldMatrix->SetElement(i, 0, handleX_World[i]);
    handleToWorldMatrix->SetElement(i, 1, handleY_World[i]);
    handleToWorldMatrix->SetElement(i, 2, handleZ_World[i]);
    handleToWorldMatrix->SetElement(i, 3, origin_World[i]);
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

  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkMatrix4x4> worldToObjectMatrix;
  worldToObjectMatrix->DeepCopy(objectToWorldMatrix);
  worldToObjectMatrix->Invert();

  double posWorld4[4] = { posWorld[0], posWorld[1], posWorld[2], 1.0 };
  double closestPosPlane4[4] = { 0.0, 0.0, 0.0, 0.0 };
  worldToObjectMatrix->MultiplyPoint(posWorld4, closestPosPlane4);

  double distanceToObject = closestPosPlane4[2];
  closestPosPlane4[2] = 0.0; // Project to plane

  if (!infinitePlane)
    {
    double planeBounds_Object[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    this->GetPlaneBounds(planeBounds_Object);
    for (int i = 0; i < 3; ++i)
      {
      closestPosPlane4[i] = std::max(closestPosPlane4[i], planeBounds_Object[2 * i]);
      closestPosPlane4[i] = std::min(closestPosPlane4[i], planeBounds_Object[2 * i + 1]);
      }
    }

  double closestPosWorld4[4] = { 0.0, 0.0, 0.0, 0.0 };
  objectToWorldMatrix->MultiplyPoint(closestPosPlane4, closestPosWorld4);
  for (int i = 0; i < 3; ++i)
    {
    closestPosWorld[i] = closestPosWorld4[i];
    }
  return distanceToObject;
}
