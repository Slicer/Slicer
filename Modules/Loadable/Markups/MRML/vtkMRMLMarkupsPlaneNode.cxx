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
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMeasurementArea.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformNode.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkPlane.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsPlaneNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode::vtkMRMLMarkupsPlaneNode()
{
  this->RequiredNumberOfControlPoints = 1;
  this->MaximumNumberOfControlPoints = 1;

  this->ObjectToBaseMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->BaseToNodeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  // Add observers so that we can update the plane/control points
  this->CurveInputPoly->GetPoints()->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  this->ObjectToBaseMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  this->BaseToNodeMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

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
  vtkMRMLWriteXMLIntMacro(maximumNumberOfControlPoints, MaximumNumberOfControlPoints);
  vtkMRMLWriteXMLIntMacro(requiredNumberOfControlPoints, RequiredNumberOfControlPoints);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(maximumNumberOfControlPoints, MaximumNumberOfControlPoints);
  vtkMRMLReadXMLIntMacro(requiredNumberOfControlPoints, RequiredNumberOfControlPoints);

  // Now handled by storage node
  vtkMRMLReadXMLEnumMacro(sizeMode, SizeMode);
  vtkMRMLReadXMLVectorMacro(size, Size, double, 2);
  vtkMRMLReadXMLFloatMacro(autoSizeScalingFactor, AutoSizeScalingFactor);
  vtkMRMLReadXMLOwnedMatrix4x4Macro(planeTobaseMatrix, ObjectToBaseMatrix); // Backwards compatible with old name
  vtkMRMLReadXMLOwnedMatrix4x4Macro(objectToBaseMatrix, ObjectToBaseMatrix);

  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(PlaneType);
  vtkMRMLCopyIntMacro(MaximumNumberOfControlPoints);
  vtkMRMLCopyIntMacro(RequiredNumberOfControlPoints);
  vtkMRMLCopyEnumMacro(SizeMode);
  vtkMRMLCopyVectorMacro(Size, double, 2);
  vtkMRMLCopyVectorMacro(Normal, double, 3);
  vtkMRMLCopyVectorMacro(Center, double, 3);
  vtkMRMLCopyFloatMacro(AutoSizeScalingFactor);
  vtkMRMLCopyEndMacro();

  Superclass::CopyContent(anode, deepCopy);

  // Copy the plane orientation matrices after the points have been copied
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyOwnedMatrix4x4Macro(ObjectToBaseMatrix);
  vtkMRMLCopyOwnedMatrix4x4Macro(BaseToNodeMatrix);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(SizeMode);
  vtkMRMLPrintVectorMacro(Size, double, 2);
  vtkMRMLPrintFloatMacro(AutoSizeScalingFactor);
  vtkMRMLPrintMatrix4x4Macro(ObjectToBaseMatrix);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (!transform)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);

  bool wasUpdatingControlPointsFromPlane = this->IsUpdatingControlPointsFromPlane;
  this->IsUpdatingControlPointsFromPlane = true;

  bool wasUpdatingPlaneFromControlPoints = this->IsUpdatingPlaneFromControlPoints;
  this->IsUpdatingPlaneFromControlPoints = true;

  vtkNew<vtkMatrix4x4> oldBaseToNodeMatrix;
  oldBaseToNodeMatrix->DeepCopy(this->BaseToNodeMatrix);

  Superclass::ApplyTransform(transform);

  vtkNew<vtkMatrix4x4> newBaseToNodeMatrix;
  this->GenerateOrthogonalMatrix(oldBaseToNodeMatrix, newBaseToNodeMatrix, transform, false);
  this->BaseToNodeMatrix->DeepCopy(newBaseToNodeMatrix);

  double newXAxis_Node[3] = { 1.0, 0.0, 0.0 };
  double newYAxis_Node[3] = { 0.0, 1.0, 0.0 };
  double newZAxis_Node[3] = { 0.0, 0.0, 1.0 };
  double newCenter_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetAxes(newXAxis_Node, newYAxis_Node, newZAxis_Node);
  this->GetCenter(newCenter_Node);

  // Update size by calculating diffference in scaling between transformed/untransformed axes
  vtkAbstractTransform* transformInverse = transform->GetInverse();
  this->Size[0] /= vtkMath::Norm(transformInverse->TransformVectorAtPoint(newCenter_Node, newXAxis_Node));
  this->Size[1] /= vtkMath::Norm(transformInverse->TransformVectorAtPoint(newCenter_Node, newYAxis_Node));

  this->IsUpdatingControlPointsFromPlane = wasUpdatingControlPointsFromPlane;
  this->IsUpdatingPlaneFromControlPoints = wasUpdatingPlaneFromControlPoints;

  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsPlaneNode::GetPlaneTypeAsString(int planeType)
{
  switch (planeType)
    {
    case vtkMRMLMarkupsPlaneNode::PlaneType3Points:
      return "threePoints";
    case vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal:
      return "pointNormal";
    case vtkMRMLMarkupsPlaneNode::PlaneTypePlaneFit:
      return "planeFit";
    default:
    break;
    }
  return "";
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsPlaneNode::GetPlaneTypeFromString(const char* planeType)
{
  if (planeType == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLMarkupsPlaneNode::PlaneType_Last; i++)
    {
    if (strcmp(planeType, vtkMRMLMarkupsPlaneNode::GetPlaneTypeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown plane type
  return -1;
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
  return "";
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsPlaneNode::GetSizeModeFromString(const char* sizeMode)
{
  if (sizeMode == nullptr)
    {
    // invalid size mode
    return -1;
    }
  for (int i = 0; i < SizeMode_Last; ++i)
    {
    if (strcmp(vtkMRMLMarkupsPlaneNode::GetSizeModeAsString(i), sizeMode) == 0)
      {
      return i;
      }
    }
  // unknown size mode
  return -1;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetSizeMode(int sizeMode)
{
  if (this->SizeMode == sizeMode)
    {
    return;
    }
  this->SizeMode = sizeMode;

  MRMLNodeModifyBlocker blocker(this);
  this->UpdateControlPointsFromPlane();
  this->UpdatePlaneFromControlPoints();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetPlaneType(int planeType)
{
  if (this->PlaneType == planeType)
    {
    return;
    }

  this->PlaneType = planeType;

  switch (this->PlaneType)
    {
    case PlaneTypePointNormal:
      this->RequiredNumberOfControlPoints = this->NormalPointRequired ? 2 : 1;
      this->MaximumNumberOfControlPoints = this->NormalPointRequired ? 2 : 1;
      break;
    case PlaneType3Points:
      this->RequiredNumberOfControlPoints = 3;
      this->MaximumNumberOfControlPoints = 3;
      break;
    case PlaneTypePlaneFit:
      this->RequiredNumberOfControlPoints = 0;
      this->MaximumNumberOfControlPoints = -1;
      break;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->UpdateControlPointsFromPlane();
  this->UpdatePlaneFromControlPoints();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetNormalPointRequired(bool normalPointRequired)
{
  if (normalPointRequired == this->NormalPointRequired)
    {
    return;
    }

  this->NormalPointRequired = normalPointRequired;
  this->RequiredNumberOfControlPoints = this->NormalPointRequired ? 2 : 1;
  this->MaximumNumberOfControlPoints = this->NormalPointRequired ? 2 : 1;

  this->Modified();
}


//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetNormal(double normal_Node[3])
{
  if (!normal_Node)
    {
    vtkErrorMacro("GetNormal: Invalid normal argument");
    return;
    }
  this->GetAxes(nullptr, nullptr, normal_Node);
}

//----------------------------------------------------------------------------
double* vtkMRMLMarkupsPlaneNode::GetNormal()
{
  this->GetNormal(this->Normal);
  return this->Normal;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetNormalWorld(double normalWorld[3])
{
  if (!normalWorld)
    {
    vtkErrorMacro("GetNormalWorld: Invalid normal argument");
    return;
    }
  this->GetAxesWorld(nullptr, nullptr, normalWorld);
}

//----------------------------------------------------------------------------
double* vtkMRMLMarkupsPlaneNode::GetNormalWorld()
{
  this->GetNormalWorld(this->NormalWorld);
  return this->NormalWorld;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetNormal(const double normal_Node[3])
{
  if (!normal_Node)
    {
    vtkErrorMacro("SetNormal: Invalid normal argument");
    return;
    }

  double epsilon = 0.0001;

  double newNormal_Node[3] = { normal_Node[0], normal_Node[1], normal_Node[2] };
  double normalLength = vtkMath::Normalize(newNormal_Node);
  if (normalLength < epsilon)
    {
    vtkErrorMacro("SetNormal: Invalid normal");
    return;
    }

  MRMLNodeModifyBlocker blocker(this);

  if (this->GetPlaneType() != PlaneTypePlaneFit)
    {
    // If we are not using plane fit, then we can generate the control points
    // base on the current parameters.
    this->SetIsPlaneValid(true);
    }

  double currentNormal_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetNormal(currentNormal_Node);

  if (vtkMath::Dot(newNormal_Node, currentNormal_Node) >= 1.0 - epsilon)
    {
    // Normal vectors are equivalent, no change required.
    this->UpdateControlPointsFromPlane();
    return;
    }

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

  this->UpdateControlPointsFromPlane();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetNormal(double x_Node, double y_Node, double z_Node)
{
  double normal_Node[3] = { x_Node, y_Node, z_Node };
  this->SetNormal(normal_Node);
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
void vtkMRMLMarkupsPlaneNode::SetNormalWorld(double x_World, double y_World, double z_World)
{
  double normal_World[3] = { x_World, y_World, z_World };
  this->SetNormalWorld(normal_World);
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

  double origin_Object[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  this->GetObjectToNodeMatrix(objectToNodeMatrix);

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->SetMatrix(objectToNodeMatrix);
  objectToNodeTransform->TransformPoint(origin_Object, origin_Node);
}

//----------------------------------------------------------------------------
double* vtkMRMLMarkupsPlaneNode::GetOrigin()
{
  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  this->GetObjectToNodeMatrix(objectToNodeMatrix);

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->SetMatrix(objectToNodeMatrix);
  double originObject[3] = { 0.0, 0.0, 0.0 };
  objectToNodeTransform->TransformPoint(originObject, this->Origin);
  return this->Origin;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetOriginWorld(double origin_World[3])
{
  if (!origin_World)
    {
    vtkErrorMacro("GetOriginWorld: Invalid origin argument");
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
double* vtkMRMLMarkupsPlaneNode::GetOriginWorld()
{
  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);
  double originObject[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(originObject, this->OriginWorld);
  return this->OriginWorld;
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
  if (this->GetPlaneType() != PlaneTypePlaneFit)
    {
    // If we are not using plane fit, then we can generate the control points
    // base on the current parameters.
    this->SetIsPlaneValid(true);
    }

  double previousOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetOrigin(previousOrigin_Node);

  double displacementVector_Node[3] = { 0.0 };
  vtkMath::Subtract(origin_Node, previousOrigin_Node, displacementVector_Node);

  vtkNew<vtkTransform> oldToNewOriginTransform;
  oldToNewOriginTransform->Translate(displacementVector_Node);
  this->ApplyTransform(oldToNewOriginTransform);
  this->UpdateControlPointsFromPlane();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOrigin(double x_Node, double y_Node, double z_Node)
{
  double origin_Node[3] = { x_Node, y_Node, z_Node };
  this->SetOrigin(origin_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOriginWorld(const double origin_World[3])
{
  double origin_Node[3] = { 0.0 };
  this->TransformPointFromWorld(origin_World, origin_Node);
  this->SetOrigin(origin_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetOriginWorld(double x_World, double y_World, double z_World)
{
  double origin_World[3] = { x_World, y_World, z_World };
  this->SetOrigin(origin_World);
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
void vtkMRMLMarkupsPlaneNode::GetBaseToNodeMatrix(vtkMatrix4x4* baseToNodeMatrix)
{
  if (!baseToNodeMatrix)
    {
    vtkErrorMacro(<< "GetObjectToNodeMatrix: Invalid objectToNodeMatrix");
    return;
    }

  baseToNodeMatrix->DeepCopy(this->BaseToNodeMatrix);
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLMarkupsPlaneNode::GetBaseToNodeMatrix()
{
  return this->BaseToNodeMatrix;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetObjectToNodeMatrix(vtkMatrix4x4* objectToNodeMatrix)
{
  if (!objectToNodeMatrix)
    {
    vtkErrorMacro(<< "GetObjectToNodeMatrix: Invalid objectToNodeMatrix");
    return;
    }

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->PostMultiply();
  objectToNodeTransform->Concatenate(this->ObjectToBaseMatrix);
  objectToNodeTransform->Concatenate(this->BaseToNodeMatrix);
  objectToNodeMatrix->DeepCopy(objectToNodeTransform->GetMatrix());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetObjectToWorldMatrix(vtkMatrix4x4* objectToWorldMatrix)
{
  if (!objectToWorldMatrix)
    {
    return;
    }

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->PostMultiply();
  objectToWorldTransform->Concatenate(this->ObjectToBaseMatrix);
  objectToWorldTransform->Concatenate(this->BaseToNodeMatrix);
  if (this->GetParentTransformNode())
    {
    vtkNew<vtkGeneralTransform> transformToWorld;
    this->GetParentTransformNode()->GetTransformToWorld(transformToWorld);
    this->GenerateOrthogonalMatrix(objectToWorldTransform->GetMatrix(), objectToWorldMatrix, transformToWorld);
    }
  else
    {
    objectToWorldMatrix->DeepCopy(objectToWorldTransform->GetMatrix());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetBaseToWorldMatrix(vtkMatrix4x4* baseToWorldMatrix)
{
  if (!baseToWorldMatrix)
    {
    return;
    }

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->PostMultiply();
  objectToWorldTransform->Concatenate(this->BaseToNodeMatrix);
  if (this->GetParentTransformNode())
    {
    vtkNew<vtkGeneralTransform> transformToWorld;
    this->GetParentTransformNode()->GetTransformToWorld(transformToWorld);
    this->GenerateOrthogonalMatrix(objectToWorldTransform->GetMatrix(), baseToWorldMatrix, transformToWorld);
    }
  else
    {
    baseToWorldMatrix->DeepCopy(objectToWorldTransform->GetMatrix());
    }
}


//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetAxes(double xAxis_Node[3], double yAxis_Node[3], double zAxis_Node[3])
{
  if (!xAxis_Node && !yAxis_Node && !zAxis_Node)
    {
    vtkErrorMacro("GetAxes: Invalid input arguments");
    return;
    }

  if (xAxis_Node)
      {
      xAxis_Node[0] = 1.0;
      xAxis_Node[1] = 0.0;
      xAxis_Node[2] = 0.0;
      }

  if (yAxis_Node)
      {
      yAxis_Node[0] = 0.0;
      yAxis_Node[1] = 1.0;
      yAxis_Node[2] = 0.0;
      }

  if (zAxis_Node)
      {
      zAxis_Node[0] = 0.0;
      zAxis_Node[1] = 0.0;
      zAxis_Node[2] = 1.0;
      }

  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  this->GetObjectToNodeMatrix(objectToNodeMatrix);

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->SetMatrix(objectToNodeMatrix);

  if (xAxis_Node)
    {
    double xAxis_Object[3] = { 1.0, 0.0, 0.0 };
    objectToNodeTransform->TransformVector(xAxis_Object, xAxis_Node);
    }

  if (yAxis_Node)
    {
    double yAxis_Object[3] = { 0.0, 1.0, 0.0 };
    objectToNodeTransform->TransformVector(yAxis_Object, yAxis_Node);
    }

  if (zAxis_Node)
    {
    double zAxis_Object[3] = { 0.0, 0.0, 1.0 };
    objectToNodeTransform->TransformVector(zAxis_Object, zAxis_Node);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetAxesWorld(double xAxis_World[3], double yAxis_World[3], double zAxis_World[3])
{
  if (!xAxis_World && !yAxis_World && !zAxis_World)
    {
    vtkErrorMacro("GetAxesWorld: Invalid input arguments");
    return;
    }

  if (xAxis_World)
      {
      xAxis_World[0] = 1.0;
      xAxis_World[1] = 0.0;
      xAxis_World[2] = 0.0;
      }

  if (yAxis_World)
      {
      yAxis_World[0] = 0.0;
      yAxis_World[1] = 1.0;
      yAxis_World[2] = 0.0;
      }

  if (zAxis_World)
      {
      zAxis_World[0] = 0.0;
      zAxis_World[1] = 0.0;
      zAxis_World[2] = 1.0;
      }

  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);

  if (xAxis_World)
    {
    double xAxis_Object[3] = { 1.0, 0.0, 0.0 };
    objectToWorldTransform->TransformVector(xAxis_Object, xAxis_World);
    vtkMath::Normalize(xAxis_World);
    }

  if (yAxis_World)
    {
    double yAxis_Object[3] = { 0.0, 1.0, 0.0 };
    objectToWorldTransform->TransformVector(yAxis_Object, yAxis_World);
    vtkMath::Normalize(yAxis_World);
    }

  if (zAxis_World)
    {
    double zAxis_Object[3] = { 0.0, 0.0, 1.0 };
    objectToWorldTransform->TransformVector(zAxis_Object, zAxis_World);
    vtkMath::Normalize(zAxis_World);
    }
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
  double tempX[3] = { 0.0, 0.0, 0.0 };
  double tempY[3] = { 0.0, 0.0, 0.0 };
  double tempZ[3] = { 0.0, 0.0, 0.0 };
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

  if (fabs(vtkMath::Dot(xAxis_Node, yAxis_Node)) >= epsilon ||
      fabs(vtkMath::Dot(yAxis_Node, zAxis_Node)) >= epsilon ||
      fabs(vtkMath::Dot(zAxis_Node, xAxis_Node)) >= epsilon)
    {
    vtkErrorMacro("SetAxes: Invalid vectors");
    }

  MRMLNodeModifyBlocker blocker(this);

  if (this->GetPlaneType() != PlaneTypePlaneFit)
    {
    // If we are not using plane fit, then we can generate the control points
    // base on the current parameters.
    this->SetIsPlaneValid(true);
    }

  double previousXAxis_Node[3] = { 0.0, 0.0, 0.0 };
  double previousYAxis_Node[3] = { 0.0, 0.0, 0.0 };
  double previousZAxis_Node[3] = { 0.0, 0.0, 0.0 };
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

  double origin_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetOrigin(origin_Node);

  vtkNew<vtkTransform> oldToNewAxesTransform;
  oldToNewAxesTransform->PostMultiply();
  oldToNewAxesTransform->Translate(-origin_Node[0], -origin_Node[1], -origin_Node[2]);
  oldToNewAxesTransform->Concatenate(previousAxisToIdentity);
  oldToNewAxesTransform->Concatenate(identityToNewAxis);
  oldToNewAxesTransform->Translate(origin_Node);

  this->ApplyTransform(oldToNewAxesTransform);
  this->UpdateControlPointsFromPlane();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetAxesWorld(const double xAxis_World[3], const double yAxis_World[3], const double zAxis_World[3])
{
  double xAxis_Node[3] = { xAxis_World[0], xAxis_World[1], xAxis_World[2] };
  double yAxis_Node[3] = { yAxis_World[0], yAxis_World[1], yAxis_World[2] };
  double zAxis_Node[3] = { zAxis_World[0], zAxis_World[1], zAxis_World[2] };

  MRMLNodeModifyBlocker blocker(this);

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

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller == this->CurveInputPoly->GetPoints() || caller == this->GetParentTransformNode())
    {
    if (!this->IsUpdatingControlPointsFromPlane && !this->IsUpdatingPlaneFromControlPoints)
      {
      this->UpdatePlaneFromControlPoints();
      }
    }
  else if (event == vtkCommand::ModifiedEvent && caller == this->BaseToNodeMatrix.GetPointer())
    {
    if (!this->IsUpdatingControlPointsFromPlane && !this->IsUpdatingPlaneFromControlPoints)
      {
      this->UpdateInteractionHandleToWorldMatrix();
      this->UpdateControlPointsFromPlane();
      }
    }
  else if (event == vtkCommand::ModifiedEvent && caller == this->ObjectToBaseMatrix.GetPointer())
    {
    this->Modified();
    }
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetSize(double planeSize[2])
{
  planeSize[0] = this->Size[0];
  planeSize[1] = this->Size[1];
}

//----------------------------------------------------------------------------
double* vtkMRMLMarkupsPlaneNode::GetSize()
{
  return this->Size;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetSize(double x, double y)
{
  MRMLNodeModifyBlocker blocker(this);

  if (this->Size[0] == x && this->Size[1] == y)
    {
    return;
    }

  if (this->Size[0] > 0.0)
    {
    this->PlaneBounds[0] /= this->Size[0];
    this->PlaneBounds[1] /= this->Size[0];
    }
  else
    {
    this->PlaneBounds[0] = -0.5;
    this->PlaneBounds[1] = 0.5;
    }

  if (this->Size[1] > 0.0)
    {
    this->PlaneBounds[2] /= this->Size[1];
    this->PlaneBounds[3] /= this->Size[1];
    }
  else
    {
    this->PlaneBounds[2] = -0.5;
    this->PlaneBounds[3] = 0.5;
    }

  this->Size[0] = x;
  this->Size[1] = y;

  this->PlaneBounds[0] *= this->Size[0];
  this->PlaneBounds[1] *= this->Size[0];
  this->PlaneBounds[2] *= this->Size[1];
  this->PlaneBounds[3] *= this->Size[1];

  this->UpdateAllMeasurements();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetSizeWorld(double size_World[2])
{
  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);
  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);

  double sideVectorX_Object[3] = { this->Size[0], 0.0, 0.0 };
  double sideVectorY_Object[3] = { 0.0, this->Size[1], 0.0 };

  size_World[0] = vtkMath::Norm(objectToWorldTransform->TransformVector(sideVectorX_Object));
  size_World[1] = vtkMath::Norm(objectToWorldTransform->TransformVector(sideVectorY_Object));
}

//----------------------------------------------------------------------------
double* vtkMRMLMarkupsPlaneNode::GetSizeWorld()
{
  this->GetSizeWorld(this->SizeWorld);
  return this->SizeWorld;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetSizeWorld(const double size_World[2])
{
  this->SetSizeWorld(size_World[0], size_World[1]);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetSizeWorld(double sizeX_World, double sizeY_World)
{
  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  this->GetObjectToWorldMatrix(objectToWorldMatrix);
  vtkNew<vtkTransform> worldToObjectTransform;
  worldToObjectTransform->SetMatrix(objectToWorldMatrix);
  worldToObjectTransform->Inverse();

  double sideVectorX_World[3] = { 0.0, 0.0, 0.0 };
  double sideVectorY_World[3] = { 0.0, 0.0, 0.0 };
  this->GetAxesWorld(sideVectorX_World, sideVectorY_World, nullptr);
  vtkMath::MultiplyScalar(sideVectorX_World, sizeX_World);
  vtkMath::MultiplyScalar(sideVectorY_World, sizeY_World);

  double sideVectorX_Object[3] = { 1.0, 0.0, 0.0 };
  worldToObjectTransform->TransformVector(sideVectorX_World, sideVectorX_Object);

  double sideVectorY_Object[3] = { 0.0, 1.0, 0.0 };
  worldToObjectTransform->TransformVector(sideVectorY_World, sideVectorY_Object);

  this->SetSize(vtkMath::Norm(sideVectorX_Object), vtkMath::Norm(sideVectorY_Object));
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::SetPlaneBounds(double x0, double x1, double y0, double y1)
{
  if (this->PlaneBounds[0] == x0 && this->PlaneBounds[1] == x1
    && this->PlaneBounds[2] == y0 && this->PlaneBounds[3] == y1)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);

  this->PlaneBounds[0] = std::min(x0, x1);
  this->PlaneBounds[1] = std::max(x0, x1);
  this->PlaneBounds[2] = std::min(y0, y1);
  this->PlaneBounds[3] = std::max(y0, y1);

  this->Size[0] = std::max(0.0, this->PlaneBounds[1] - this->PlaneBounds[0]);
  this->Size[1] = std::max(0.0, this->PlaneBounds[3] - this->PlaneBounds[2]);

  this->UpdateAllMeasurements();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLMarkupsPlaneNode::GetObjectToBaseMatrix()
{
  return this->ObjectToBaseMatrix;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)
{
  Superclass::OnTransformNodeReferenceChanged(transformNode);
  this->UpdateInteractionHandleToWorldMatrix();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateInteractionHandleToWorldMatrix()
{
  double handleX_World[3] = { 0.0, 0.0, 0.0 };
  double handleY_World[3] = { 0.0, 0.0, 0.0 };
  double handleZ_World[3] = { 0.0, 0.0, 0.0 };
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
  double posPlane4[4] = { 0.0, 0.0, 0.0, 0.0 };
  worldToObjectMatrix->MultiplyPoint(posWorld4, posPlane4);

  double closestPosPlane4[4] = { posPlane4[0], posPlane4[1], posPlane4[2], posPlane4[3] };
  closestPosPlane4[2] = 0.0; // Project to plane

  if (!infinitePlane)
    {
    double planeSize_Object[2] = { 0.0, 0.0 };
    this->GetSize(planeSize_Object);
    closestPosPlane4[0] = std::max(std::min(closestPosPlane4[0], planeSize_Object[0] * 0.5), -1.0 * planeSize_Object[0] * 0.5);
    closestPosPlane4[1] = std::max(std::min(closestPosPlane4[1], planeSize_Object[1] * 0.5), -1.0 * planeSize_Object[1] * 0.5);
    }

  double closestPosWorld4[4] = { 0.0, 0.0, 0.0, 0.0 };
  objectToWorldMatrix->MultiplyPoint(closestPosPlane4, closestPosWorld4);
  for (int i = 0; i < 3; ++i)
    {
    closestPosWorld[i] = closestPosWorld4[i];
    }

  return std::sqrt(vtkMath::Distance2BetweenPoints(closestPosWorld, posWorld));
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsPlaneNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLMarkupsPlaneJsonStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != nullptr &&
    vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(this->GetDisplayNode()) != nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkMRMLMarkupsPlaneNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLMarkupsPlaneDisplayNode* dispNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLMarkupsPlaneDisplayNode"));
  if (!dispNode)
    {
    vtkErrorMacro("vtkMRMLMarkupsPlaneNode::CreateDefaultDisplayNodes failed: scene failed to instantiate a vtkMRMLMarkupsPlaneDisplayNode node");
    return;
    }
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetRASBounds(double bounds[6])
{
  if (!bounds)
    {
    vtkErrorMacro("Invalid bounds argument");
    return;
    }

  vtkNew<vtkPoints> cornerPoints_World;
  this->GetPlaneCornerPointsWorld(cornerPoints_World);

  double planeBounds_World[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  this->CalculatePlaneBounds(cornerPoints_World, planeBounds_World);

  // Get bounds from control points
  Superclass::GetRASBounds(bounds);
  bounds[0] = std::min(bounds[0], planeBounds_World[0]);
  bounds[1] = std::max(bounds[1], planeBounds_World[1]);
  bounds[2] = std::min(bounds[2], planeBounds_World[2]);
  bounds[3] = std::max(bounds[3], planeBounds_World[3]);
  bounds[4] = std::min(bounds[4], planeBounds_World[4]);
  bounds[5] = std::max(bounds[5], planeBounds_World[5]);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetBounds(double bounds[6])
{
  if (!bounds)
    {
    vtkErrorMacro("Invalid bounds argument");
    return;
    }

  vtkNew<vtkPoints> cornerPoints_Node;
  this->GetPlaneCornerPoints(cornerPoints_Node);

  double planeBounds_Node[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  this->CalculatePlaneBounds(cornerPoints_Node, planeBounds_Node);

  // Get bounds from control points
  Superclass::GetBounds(bounds);
  bounds[0] = std::min(bounds[0], planeBounds_Node[0]);
  bounds[1] = std::max(bounds[1], planeBounds_Node[1]);
  bounds[2] = std::min(bounds[2], planeBounds_Node[2]);
  bounds[3] = std::max(bounds[3], planeBounds_Node[3]);
  bounds[4] = std::min(bounds[4], planeBounds_Node[4]);
  bounds[5] = std::max(bounds[5], planeBounds_Node[5]);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneCornerPoints(vtkPoints* points_Node)
{
  double xAxis_Node[3] = { 0.0, 0.0, 0.0 };
  double yAxis_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetAxes(xAxis_Node, yAxis_Node, nullptr);

  double center_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetCenter(center_Node);

  double size_Node[2] = { 0.0, 0.0 };
  this->GetSize(size_Node);

  this->CalculatePlaneCornerPoints(points_Node, xAxis_Node, yAxis_Node, center_Node, size_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GetPlaneCornerPointsWorld(vtkPoints* points_World)
{
  double xAxis_World[3] = { 0.0, 0.0, 0.0 };
  double yAxis_World[3] = { 0.0, 0.0, 0.0 };
  this->GetAxesWorld(xAxis_World, yAxis_World, nullptr);

  double center_World[3] = { 0.0, 0.0, 0.0 };
  this->GetCenterWorld(center_World);

  double size_World[2] = { 0.0, 0.0 };
  this->GetSizeWorld(size_World);

  this->CalculatePlaneCornerPoints(points_World, xAxis_World, yAxis_World, center_World, size_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CalculatePlaneCornerPoints(vtkPoints * points, double xAxis[3], double yAxis[3], double center[3], double size[2])
{
  if (!points)
    {
    vtkErrorMacro("CalculatePlaneCornerPoints: Invalid points");
    return;
    }

  double planeBounds[4] = { 0.0, -1.0, 0.0, -1.0 };
  this->GetPlaneBounds(planeBounds);

  // Scale the bounds so that they match the specified plane size.
  planeBounds[0] *= size[0] / this->Size[0];
  planeBounds[1] *= size[0] / this->Size[0];
  planeBounds[2] *= size[1] / this->Size[1];
  planeBounds[3] *= size[1] / this->Size[1];

  double xAxisNegative[3] = { xAxis[0], xAxis[1], xAxis[2] };
  vtkMath::MultiplyScalar(xAxisNegative, planeBounds[0]);
  double xAxisPositive[3] = { xAxis[0], xAxis[1], xAxis[2] };
  vtkMath::MultiplyScalar(xAxisPositive, planeBounds[1]);

  double yAxisNegative[3] = { yAxis[0], yAxis[1], yAxis[2] };
  vtkMath::MultiplyScalar(yAxisNegative, planeBounds[2]);
  double yAxisPositive[3] = { yAxis[0], yAxis[1], yAxis[2] };
  vtkMath::MultiplyScalar(yAxisPositive, planeBounds[3]);

  points->SetNumberOfPoints(4);

  double point0[3] = { 0.0, 0.0, 0.0 }; // LP
  vtkMath::Add(center, point0, point0);
  vtkMath::Add(xAxisNegative, point0, point0);
  vtkMath::Add(yAxisNegative, point0, point0);
  points->SetPoint(0, point0);

  double point1[3] = { 0.0, 0.0, 0.0 }; // LA
  vtkMath::Add(center, point1, point1);
  vtkMath::Add(xAxisNegative, point1, point1);
  vtkMath::Add(yAxisPositive, point1, point1);
  points->SetPoint(1, point1);

  double point2[3] = { 0.0, 0.0, 0.0 }; // RA
  vtkMath::Add(center, point2, point2);
  vtkMath::Add(xAxisPositive, point2, point2);
  vtkMath::Add(yAxisPositive, point2, point2);
  points->SetPoint(2, point2);

  double point3[3] = { 0.0, 0.0, 0.0 }; // RP
  vtkMath::Add(center, point3, point3);
  vtkMath::Add(xAxisPositive, point3, point3);
  vtkMath::Add(yAxisNegative, point3, point3);
  points->SetPoint(3, point3);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::CalculatePlaneBounds(vtkPoints* cornerPoints, double bounds[6])
{
  if (!cornerPoints || !bounds)
    {
    vtkErrorMacro("CalculatePlaneBounds: Invalid arguments");
    return;
    }

  vtkBoundingBox box;
  for (int i = 0; i < cornerPoints->GetNumberOfPoints(); ++i)
    {
    box.AddPoint(cornerPoints->GetPoint(i));
    }
  box.GetBounds(bounds);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdatePlaneFromControlPoints()
{
  if (this->IsUpdatingControlPointsFromPlane || this->IsUpdatingPlaneFromControlPoints)
    {
    return;
    }

  this->IsUpdatingPlaneFromControlPoints = true;
    {
    // Block events in this scope
    MRMLNodeModifyBlocker blocker(this);

    switch (this->PlaneType)
      {
      case PlaneTypePointNormal:
        this->UpdatePlaneFromPointNormal();
        break;
      case PlaneType3Points:
        this->UpdatePlaneFrom3Points();
        break;
      case PlaneTypePlaneFit:
        this->UpdatePlaneFromPlaneFit();
        break;
      default:
        break;
      }
    }
  this->IsUpdatingPlaneFromControlPoints = false;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdatePlaneFromPointNormal()
{
  double origin_Node[3] = { 0.0, 0.0, 0.0 };
  if (this->GetNumberOfControlPoints() > 0)
    {
    this->GetNthControlPointPosition(0, origin_Node);
    }

  vtkNew<vtkTransform> baseToNodeTransform;
  baseToNodeTransform->PostMultiply();
  baseToNodeTransform->Concatenate(this->BaseToNodeMatrix);

  double oldOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  baseToNodeTransform->TransformPoint(oldOrigin_Node, oldOrigin_Node);

  baseToNodeTransform->Translate(-oldOrigin_Node[0], -oldOrigin_Node[1], -oldOrigin_Node[2]);

  double oldZ_Node[3] = { 0.0, 0.0, 1.0 };
  baseToNodeTransform->TransformVector(oldZ_Node, oldZ_Node);

  double newX_Node[3] = { 1.0, 0.0, 0.0 };
  double newY_Node[3] = { 0.0, 1.0, 0.0 };
  double newZ_Node[3] = { 0.0, 0.0, 1.0 };
  if (this->GetNumberOfControlPoints() > 1)
    {
    double normalPoint_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPosition(1, normalPoint_Node);
    vtkMath::Subtract(normalPoint_Node, origin_Node, newZ_Node);
    vtkMath::Normalize(newZ_Node);

    vtkMath::Perpendiculars(newZ_Node, newX_Node, newY_Node, 0.0);
    }
  else
    {
    baseToNodeTransform->TransformVector(newX_Node, newX_Node);
    baseToNodeTransform->TransformVector(newY_Node, newY_Node);
    baseToNodeTransform->TransformVector(newZ_Node, newZ_Node);
    }

  double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(oldZ_Node, newZ_Node));
  double epsilon = 0.001;
  if (angle > epsilon)
    {
    double rotationVector_Node[3] = { 1.0, 0.0, 0.0 };
    vtkMath::Cross(oldZ_Node, newZ_Node, rotationVector_Node);
    vtkMath::Normalize(rotationVector_Node);
    baseToNodeTransform->RotateWXYZ(angle, rotationVector_Node);
    }
  baseToNodeTransform->Translate(origin_Node);
  this->BaseToNodeMatrix->DeepCopy(baseToNodeTransform->GetMatrix());
  // this->BaseToNodeMatrix modified event is ignored if we get here from a MRML node callback,
  // so we need to call Modified() to ensure that node modification is notified.
  this->Modified();
  if (this->GetNumberOfDefinedControlPoints(true/*include preview*/) >= 1 && this->Size[0] >= 0.0 && this->Size[1] >= 0.0)
    {
    this->SetIsPlaneValid(true);
    }
  else
    {
    this->SetIsPlaneValid(false);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdatePlaneFrom3Points()
{
  if (this->GetNumberOfControlPoints() < 3)
    {
    // Not enough points to define the plane
    this->SetIsPlaneValid(false);
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
  this->BaseToNodeMatrix->DeepCopy(baseToNodeMatrix);
  // this->BaseToNodeMatrix modified event is ignored if we get here from a MRML node callback,
  // so we need to call Modified() to ensure that node modification is notified.
  this->Modified();

  this->SetIsPlaneValid(true);
  this->UpdatePlaneSize();
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneNode::GetClosestFitPlaneFromControlPoints(vtkMatrix4x4* closestFitPlane)
{
  // The orientation of the coordinate system is adjusted so that the z axis aligns with the normal of the
  // best fit plane defined by the control points.

  int numberOfControlPoints = this->GetNumberOfMarkups();
  vtkNew<vtkPoints> controlPoints_Node;
  for (int i = 0; i < numberOfControlPoints; ++i)
    {
    double controlPointPosition_Node[3] = { 0.0 };
    this->GetNthControlPointPosition(i, controlPointPosition_Node);
    controlPoints_Node->InsertNextPoint(controlPointPosition_Node);
    }

  vtkNew<vtkPlane> bestFitPlane_Node;
  if (!vtkAddonMathUtilities::FitPlaneToPoints(controlPoints_Node, bestFitPlane_Node))
    {
    vtkErrorMacro("GetClosestFitPlaneFromControlPoints: Could not fit plane to points");
    return false;
    }

  double newZ_Node[3] = { 0.0, 0.0, 0.0 };
  bestFitPlane_Node->GetNormal(newZ_Node);

  vtkNew<vtkMatrix4x4> oldBaseToNodeMatrix;
  this->GetBaseToNodeMatrix(oldBaseToNodeMatrix);

  vtkNew<vtkTransform> oldBaseToNodeTransform;
  oldBaseToNodeTransform->SetMatrix(oldBaseToNodeMatrix);

  double oldZ_Node[3] = { 0.0, 0.0, 1.0 };
  oldBaseToNodeTransform->TransformVector(oldZ_Node, oldZ_Node);

  if (vtkMath::Dot(oldZ_Node, newZ_Node) < -0.9)
    {
    vtkMath::MultiplyScalar(newZ_Node, -1.0);
    }

  double oldOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  oldBaseToNodeTransform->TransformPoint(oldOrigin_Node, oldOrigin_Node);

  vtkNew<vtkTransform> baseToNodeTransform;
  baseToNodeTransform->PostMultiply();
  baseToNodeTransform->Concatenate(oldBaseToNodeMatrix);
  baseToNodeTransform->Translate(-oldOrigin_Node[0], -oldOrigin_Node[1], -oldOrigin_Node[2]);

  double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(oldZ_Node, newZ_Node));
  double epsilon = 0.001;
  if (angle > epsilon)
    {
    double rotationVector_Node[3] = { 1.0, 0.0, 0.0 };
    vtkMath::Cross(oldZ_Node, newZ_Node, rotationVector_Node);
    vtkMath::Normalize(rotationVector_Node);
    baseToNodeTransform->RotateWXYZ(angle, rotationVector_Node);
    }
  baseToNodeTransform->Translate(bestFitPlane_Node->GetOrigin());
  closestFitPlane->DeepCopy(baseToNodeTransform->GetMatrix());

  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdatePlaneFromPlaneFit()
{
  // The origin of the coordinate system is at the center of mass of the control points
  if (this->GetNumberOfControlPoints() < 3)
    {
    this->BaseToNodeMatrix->Identity();
    if (this->SizeMode == vtkMRMLMarkupsPlaneNode::SizeModeAuto)
      {
      this->SetSize(0.0, 0.0);
      }
    this->SetIsPlaneValid(false);
    }
  else
    {
    // The orientation of the coordinate system is adjusted so that the z axis aligns with the normal of the
    // best fit plane defined by the control points.
    vtkNew<vtkMatrix4x4> bestFitMatrix_Node;
    bool valid = this->GetClosestFitPlaneFromControlPoints(this->BaseToNodeMatrix);
    this->SetIsPlaneValid(valid);
    this->UpdatePlaneSize();
   }

  // this->BaseToNodeMatrix modified event is ignored if we get here from a MRML node callback,
  // so we need to call Modified() to ensure that node modification is notified.
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdatePlaneSize()
{
  if (this->SizeMode != vtkMRMLMarkupsPlaneNode::SizeModeAuto)
    {
    return;
    }

  if (!this->GetIsPlaneValid())
    {
    this->SetSize(0.0, 0.0);
    return;
    }

  vtkNew<vtkMatrix4x4> objectToBaseMatrix;
  this->GetBaseToWorldMatrix(objectToBaseMatrix);

  vtkNew<vtkTransform> worldToBaseTransform;
  worldToBaseTransform->SetMatrix(objectToBaseMatrix);
  worldToBaseTransform->Inverse();

  double xMax_Base = 0.0;
  double yMax_Base = 0.0;

  // Size mode auto means we need to recalculate the diameter of the plane from the control points.
  // Get plane size in world coordinate system units
  for (int i = 0; i < this->GetNumberOfControlPoints(); ++i)
    {
    double point_World[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPositionWorld(i, point_World);

    double point_Base[3] = { 0.0, 0.0, 0.0 };
    worldToBaseTransform->TransformPoint(point_World, point_Base);

    xMax_Base = std::max({ std::abs(point_Base[0]), xMax_Base });
    yMax_Base = std::max({ std::abs(point_Base[1]), yMax_Base });
    }

  double xRadius = xMax_Base * this->AutoSizeScalingFactor;
  double yRadius = yMax_Base * this->AutoSizeScalingFactor;
  this->SetPlaneBounds(-xRadius, xRadius, -yRadius, yRadius);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateControlPointsFromPlane()
{
  if (this->IsUpdatingControlPointsFromPlane || this->IsUpdatingPlaneFromControlPoints || !this->GetIsPlaneValid())
    {
    return;
    }

  this->IsUpdatingControlPointsFromPlane = true;

  {
    // Block events in this scope
    MRMLNodeModifyBlocker blocker(this);

    switch (this->PlaneType)
      {
      case vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal:
        this->UpdateControlPointsFromPointNormal();
        break;
      case vtkMRMLMarkupsPlaneNode::PlaneType3Points:
        this->UpdateControlPointsFrom3Points();
        break;
      case vtkMRMLMarkupsPlaneNode::PlaneTypePlaneFit:
        this->UpdateControlPointsFromPlaneFit();
        break;
      default:
        break;
      }
  }

  this->IsUpdatingControlPointsFromPlane = false;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateControlPointsFromPointNormal()
{
  if (!this->NormalPointRequired && this->GetNumberOfDefinedControlPoints() > 1)
    {
    while (this->GetNumberOfDefinedControlPoints() > 1)
      {
      this->RemoveNthControlPoint(1);
      }
    }

  if (this->GetIsPlaneValid() && this->GetNumberOfControlPoints() == 0)
    {
    this->AddControlPoint(vtkVector3d());
    }

  if (this->GetIsPlaneValid() && this->GetNumberOfControlPoints() > 0)
    {
    vtkNew<vtkMatrix4x4> baseToWorldMatrix;
    this->GetBaseToWorldMatrix(baseToWorldMatrix);

    vtkNew<vtkTransform> baseToWorldTransform;
    baseToWorldTransform->SetMatrix(baseToWorldMatrix);

    double origin_World[3] = { 0,0,0 };
    baseToWorldTransform->TransformPoint(origin_World, origin_World);

    this->SetNthControlPointPositionWorld(0, origin_World, this->GetNthControlPointPositionStatus(0));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateControlPointsFrom3Points()
{
  if (!this->GetIsPlaneValid())
    {
    return;
    }

  if (this->Size[0] <= 0.0 || this->Size[1] <= 0.0)
    {
    return;
    }

  if (this->GetNumberOfDefinedControlPoints() > 3)
    {
    // If we switch from another plane type, we may have more than 3 points
    // Remove them until we only have 3.
    while (this->GetNumberOfDefinedControlPoints() > 3)
      {
      this->RemoveNthControlPoint(3);
      }
    }

  vtkNew<vtkMatrix4x4> baseToWorldMatrix;
  this->GetBaseToWorldMatrix(baseToWorldMatrix);

  vtkNew<vtkTransform> baseToWorldTransform;
  baseToWorldTransform->SetMatrix(baseToWorldMatrix);

  double origin_World[3]{ 0.0, 0.0, 0.0 };
  baseToWorldTransform->TransformPoint(origin_World, origin_World);

  double xAxis_World[3] = { 1.0, 0.0, 0.0 };
  baseToWorldTransform->TransformVector(xAxis_World, xAxis_World);

  double yAxis_World[3] = { 0.0, 1.0, 0.0 };
  baseToWorldTransform->TransformVector(yAxis_World, yAxis_World);

  /// If the plane is valid but doesn't have all 3 points placed, then place the remaining points
  if (this->GetNumberOfControlPoints() < 3)
    {
    this->AddNControlPoints(3 - this->GetNumberOfControlPoints());

    double point1_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::MultiplyScalar(xAxis_World, this->Size[0] * 0.5);
    vtkMath::Add(origin_World, xAxis_World, point1_World);

    double point2_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::MultiplyScalar(yAxis_World, this->Size[1] * 0.5);
    vtkMath::Add(origin_World, yAxis_World, point2_World);

    this->SetNthControlPointPositionWorld(0, origin_World);
    this->SetNthControlPointPositionWorld(1, point1_World);
    this->SetNthControlPointPositionWorld(2, point2_World);
    }

  double point0_World[3] = { 0.0, 0.0, 0.0 };
  double point1_World[3] = { 0.0, 0.0, 0.0 };
  double point2_World[3] = { 0.0, 0.0, 0.0 };
  this->GetNthControlPointPositionWorld(0, point0_World);
  this->GetNthControlPointPositionWorld(1, point1_World);
  this->GetNthControlPointPositionWorld(2, point2_World);

  // Check if existing vectors are unique.
  double vectorPoint0ToPoint1_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(point1_World, point0_World, vectorPoint0ToPoint1_World);
  double distancePoint0ToPoint1_World = vtkMath::Normalize(vectorPoint0ToPoint1_World);

  double vectorPoint0ToPoint2_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(point2_World, point0_World, vectorPoint0ToPoint2_World);
  double distancePoint0ToPoint2_World = vtkMath::Normalize(vectorPoint0ToPoint2_World);

  bool pointChanged = false;
  double epsilon = 1e-5;
  if (distancePoint0ToPoint1_World <= epsilon)
    {
    // Point1 is at same position as point0.
    // Move point1 away in x axis.
    vtkMath::Add(point1_World, xAxis_World, point1_World);
    pointChanged = true;
    }

  if (distancePoint0ToPoint2_World <= epsilon)
    {
    // Point2 is at same position as point0.
    // Move point2 away in y axis.
    vtkMath::Add(point2_World, yAxis_World, point2_World);
    pointChanged = true;
    }

  if (vtkMath::Dot(vectorPoint0ToPoint1_World, vectorPoint0ToPoint2_World) >= 1.0 - epsilon)
    {
    // Point1 and point2 are along the same vector from point0.
    // Find a perpendicular vector and move point2.
    double perpendicularAxis_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Perpendiculars(vectorPoint0ToPoint2_World, perpendicularAxis_World, nullptr, 0.0);
    vtkMath::Add(point0_World, perpendicularAxis_World, point2_World);
    }

  if (pointChanged)
    {
    this->SetNthControlPointPositionWorld(0, point0_World);
    this->SetNthControlPointPositionWorld(1, point1_World);
    this->SetNthControlPointPositionWorld(2, point2_World);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::UpdateControlPointsFromPlaneFit()
{
  // Determine the plane origin and normal using the current control points
  vtkNew<vtkMatrix4x4> bestFitMatrix_Node;
  this->GetClosestFitPlaneFromControlPoints(bestFitMatrix_Node);

  vtkNew<vtkMatrix4x4> oldNodeToBase;
  vtkMatrix4x4::Invert(bestFitMatrix_Node, oldNodeToBase);

  vtkNew<vtkTransform> oldToNewTransform;
  oldToNewTransform->PostMultiply();
  oldToNewTransform->Concatenate(oldNodeToBase);
  oldToNewTransform->Concatenate(this->BaseToNodeMatrix);

  vtkNew<vtkTransformPolyDataFilter> transformPoints;
  transformPoints->SetInputData(this->CurveInputPoly);
  transformPoints->SetTransform(oldToNewTransform);
  transformPoints->Update();
  this->SetControlPointPositionsWorld(transformPoints->GetOutput()->GetPoints());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GenerateOrthogonalMatrix(vtkMatrix4x4* inputMatrix,
  vtkMatrix4x4* outputMatrix, vtkAbstractTransform* transform/*=nullptr*/, bool applyScaling/*=true*/)
{
  double xAxis[3] = { 0.0, 0.0, 0.0 };
  double yAxis[3] = { 0.0, 0.0, 0.0 };
  double zAxis[3] = { 0.0, 0.0, 0.0 };
  double origin[3] = { 0.0,0.0, 0.0 };
  for (int i = 0; i < 3; ++i)
    {
    xAxis[i] = inputMatrix->GetElement(i, 0);
    yAxis[i] = inputMatrix->GetElement(i, 1);
    zAxis[i] = inputMatrix->GetElement(i, 2);
    origin[i] = inputMatrix->GetElement(i, 3);
    }
  vtkMRMLMarkupsPlaneNode::GenerateOrthogonalMatrix(xAxis, yAxis, zAxis, origin, outputMatrix, transform, applyScaling);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneNode::GenerateOrthogonalMatrix(double xAxis[3], double yAxis[3], double zAxis[3], double origin[3],
  vtkMatrix4x4* outputMatrix, vtkAbstractTransform* transform/*=nullptr*/, bool applyScaling/*=true*/)
{
  if (!xAxis || !yAxis || !zAxis || !origin || !transform || !outputMatrix)
    {
    vtkGenericWarningMacro("GenerateOrthogonalMatrix: Invalid arguments");
    return;
    }

  double xAxisTransformed[3] = { xAxis[0],  xAxis[1], xAxis[2] };
  double yAxisTransformed[3] = { yAxis[0],  yAxis[1], yAxis[2] };
  double zAxisTransformed[3] = { zAxis[0],  zAxis[1], zAxis[2] };
  double originTransformed[3] = { origin[0],  origin[1], origin[2] };

  double xAxisScale = vtkMath::Norm(xAxis);
  double yAxisScale = vtkMath::Norm(yAxis);
  double zAxisScale = vtkMath::Norm(zAxis);

  if (transform)
    {
    transform->TransformVectorAtPoint(origin, xAxis, xAxisTransformed);
    transform->TransformVectorAtPoint(origin, yAxis, yAxisTransformed);
    transform->TransformVectorAtPoint(origin, zAxis, zAxisTransformed);
    transform->TransformPoint(origin, originTransformed);
    }

  vtkMath::Cross(xAxisTransformed, yAxisTransformed, zAxisTransformed);
  vtkMath::Normalize(zAxisTransformed);
  vtkMath::Cross(zAxisTransformed, xAxisTransformed, yAxisTransformed);
  vtkMath::Normalize(yAxisTransformed);
  vtkMath::Cross(yAxisTransformed, zAxisTransformed, xAxisTransformed);
  vtkMath::Normalize(xAxisTransformed);

  if (applyScaling)
    {
    if (transform)
      {
      vtkAbstractTransform* inverseTransform = transform->GetInverse();
      xAxisScale /= vtkMath::Norm(inverseTransform->TransformVectorAtPoint(originTransformed, xAxisTransformed));
      yAxisScale /= vtkMath::Norm(inverseTransform->TransformVectorAtPoint(originTransformed, yAxisTransformed));
      zAxisScale /= vtkMath::Norm(inverseTransform->TransformVectorAtPoint(originTransformed, zAxisTransformed));
      }
    vtkMath::MultiplyScalar(xAxisTransformed, xAxisScale);
    vtkMath::MultiplyScalar(yAxisTransformed, yAxisScale);
    vtkMath::MultiplyScalar(zAxisTransformed, zAxisScale);
    }

  for (int i = 0; i < 3; ++i)
    {
    outputMatrix->SetElement(i, 0, xAxisTransformed[i]);
    outputMatrix->SetElement(i, 1, yAxisTransformed[i]);
    outputMatrix->SetElement(i, 2, zAxisTransformed[i]);
    outputMatrix->SetElement(i, 3, originTransformed[i]);
    }
}
