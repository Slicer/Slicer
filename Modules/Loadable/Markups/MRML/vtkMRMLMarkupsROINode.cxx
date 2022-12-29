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
#include <vtkMRMLMarkupsROIDisplayNode.h>
#include <vtkMRMLMarkupsROIJsonStorageNode.h>
#include "vtkMRMLMeasurementVolume.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include "vtkAddonMathUtilities.h"
#include <vtkBoundingBox.h>
#include <vtkBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImplicitSum.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

const int NUMBER_OF_BOX_CONTROL_POINTS = 2; // 2 points used for initial ROI definition, then removed
const int NUMBER_OF_BOUNDING_BOX_CONTROL_POINTS = -1; // Any number of points

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsROINode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsROINode::vtkMRMLMarkupsROINode()
{
  this->PropertiesLabelText = "";

  this->RequiredNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;
  this->MaximumNumberOfControlPoints = -1;
  this->IsUpdatingControlPointsFromROI = false;
  this->IsUpdatingROIFromControlPoints = false;

  this->CurveInputPoly->GetPoints()->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  this->ObjectToNodeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->ObjectToNodeMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  this->ObjectToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  this->InteractionHandleToWorldMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);

  // Setup measurements calculated for this markup type
  vtkNew<vtkMRMLMeasurementVolume> volumeMeasurement;
  volumeMeasurement->SetEnabled(false);
  volumeMeasurement->SetName("volume");
  volumeMeasurement->SetInputMRMLNode(this);
  this->Measurements->AddItem(volumeMeasurement);

  this->ImplicitFunction = vtkSmartPointer<vtkImplicitSum>::New();
  this->ImplicitFunction->SetTransform(vtkNew<vtkTransform>());
  this->ImplicitFunctionWorld = vtkSmartPointer<vtkImplicitSum>::New();
  this->ImplicitFunctionWorld->SetTransform(vtkNew<vtkTransform>());
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
  vtkMRMLCopyOwnedMatrix4x4Macro(ObjectToNodeMatrix);
  vtkMRMLCopyBooleanMacro(InsideOut);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(ROIType);
  vtkMRMLPrintVectorMacro(Size, double, 3);
  vtkMRMLPrintMatrix4x4Macro(ObjectToNodeMatrix);
  vtkMRMLPrintBooleanMacro(InsideOut);
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
void vtkMRMLMarkupsROINode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != nullptr &&
    vtkMRMLMarkupsROIDisplayNode::SafeDownCast(this->GetDisplayNode()) != nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkMRMLMarkupsROINode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLMarkupsROIDisplayNode* dispNode = vtkMRMLMarkupsROIDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLMarkupsROIDisplayNode"));
  if (!dispNode)
    {
    vtkErrorMacro("vtkMRMLMarkupsROINode::CreateDefaultDisplayNodes failed: scene failed to instantiate a vtkMRMLMarkupsROIDisplayNode node");
    return;
    }
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetAndObserveObjectToNodeMatrix(vtkMatrix4x4* objectToNodeMatrix)
{
  if (this->ObjectToNodeMatrix == objectToNodeMatrix)
    {
    return;
    }

  if (this->ObjectToNodeMatrix)
    {
    this->ObjectToNodeMatrix->RemoveObserver(this->MRMLCallbackCommand);
    }

  this->ObjectToNodeMatrix = objectToNodeMatrix;
  if (!this->ObjectToNodeMatrix)
    {
    this->ObjectToNodeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    }

  this->ObjectToNodeMatrix->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  this->ObjectToNodeMatrix->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (!transform)
    {
    vtkErrorMacro("ApplyTransform: Invalid transform");
    return;
    }

  MRMLNodeModifyBlocker blocker(this);

  bool wasUpdatingControlPointsFromROI = this->IsUpdatingControlPointsFromROI;
  this->IsUpdatingControlPointsFromROI = true;

  bool wasUpdatingROIFromControlPoints = this->IsUpdatingROIFromControlPoints;
  this->IsUpdatingROIFromControlPoints = true;

  vtkNew<vtkMatrix4x4> oldObjectToNodeMatrix;
  oldObjectToNodeMatrix->DeepCopy(this->ObjectToNodeMatrix);

  Superclass::ApplyTransform(transform);

  vtkNew<vtkMatrix4x4> newObjectToNodeMatrix;
  this->GenerateOrthogonalMatrix(oldObjectToNodeMatrix, newObjectToNodeMatrix, transform, false);
  this->ObjectToNodeMatrix->DeepCopy(newObjectToNodeMatrix);

  double xAxis_Node_New[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Node_New[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Node_New[3] = { 0.0, 0.0, 1.0 };
  double center_Node_New[3] = { 0.0, 0.0, 0.0 };
  this->GetXAxis(xAxis_Node_New);
  this->GetYAxis(yAxis_Node_New);
  this->GetZAxis(zAxis_Node_New);
  this->GetCenter(center_Node_New);

  // Update size by calculating diffference in scaling between transformed/untransformed axes
  vtkAbstractTransform* transformInverse = transform->GetInverse();
  this->Size[0] /= vtkMath::Norm(transformInverse->TransformVectorAtPoint(center_Node_New, xAxis_Node_New));
  this->Size[1] /= vtkMath::Norm(transformInverse->TransformVectorAtPoint(center_Node_New, yAxis_Node_New));
  this->Size[2] /= vtkMath::Norm(transformInverse->TransformVectorAtPoint(center_Node_New, zAxis_Node_New));

  this->UpdateObjectToWorldMatrix();

  this->IsUpdatingControlPointsFromROI = wasUpdatingControlPointsFromROI;
  this->IsUpdatingROIFromControlPoints = wasUpdatingROIFromControlPoints;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetRASBounds(double bounds[6])
{
  if (!bounds)
    {
    vtkErrorMacro("Invalid bounds argument");
    return;
    }

  if (this->ROIType == ROITypeBox || this->ROIType == ROITypeBoundingBox)
    {
    double xAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetXAxisWorld(xAxis_World);
    double yAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetYAxisWorld(yAxis_World);
    double zAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetZAxisWorld(zAxis_World);
    double center_World[3] = { 0.0, 0.0, 0.0 };
    this->GetCenterWorld(center_World);
    double size_World[3] = { 0.0, 0.0, 0.0 };
    this->GetSizeWorld(size_World);
    this->GenerateBoxBounds(bounds, xAxis_World, yAxis_World, zAxis_World, center_World, size_World);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetBounds(double bounds[6])
{
  if (!bounds)
    {
    vtkErrorMacro("Invalid bounds argument");
    return;
    }

  if (this->ROIType == ROITypeBox || this->ROIType == ROITypeBoundingBox)
    {
    double xAxis_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetXAxis(xAxis_Node);
    double yAxis_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetYAxis(yAxis_Node);
    double zAxis_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetZAxis(zAxis_Node);
    double center_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetCenter(center_Node);
    this->GenerateBoxBounds(bounds, xAxis_Node, yAxis_Node, zAxis_Node, center_Node, this->Size);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GenerateBoxBounds(double bounds[6], double xAxis[3], double yAxis[3], double zAxis[3], double center[3], double size[3])
{
  if (!bounds || !xAxis || !yAxis || !zAxis || !center || !size)
    {
    vtkErrorMacro("Invalid arguments");
    return;
    }

  double xFaceVector[3] = { xAxis[0], xAxis[1], xAxis[2] };
  vtkMath::MultiplyScalar(xFaceVector, 0.5 * size[0]);

  double yFaceVector[3] = { yAxis[0], yAxis[1], yAxis[2] };
  vtkMath::MultiplyScalar(yFaceVector, 0.5 * size[1]);

  double zFaceVector[3] = { zAxis[0], zAxis[1], zAxis[2] };
  vtkMath::MultiplyScalar(zFaceVector, 0.5 * size[2]);

  vtkBoundingBox box;
  for (int k = 0; k < 2; ++k)
    {
    for (int j = 0; j < 2; ++j)
      {
      for (int i = 0; i < 2; ++i)
        {
        double cornerPoint[3] = { center[0], center[1], center[2] };
        if (i == 0)
          {
          vtkMath::Subtract(cornerPoint, xFaceVector, cornerPoint);
          }
        else
          {
          vtkMath::Add(cornerPoint, xFaceVector, cornerPoint);
          }

        if (j == 0)
          {
          vtkMath::Subtract(cornerPoint, yFaceVector, cornerPoint);
          }
        else
          {
          vtkMath::Add(cornerPoint, yFaceVector, cornerPoint);
          }

        if (k == 0)
          {
          vtkMath::Subtract(cornerPoint, zFaceVector, cornerPoint);
          }
        else
          {
          vtkMath::Add(cornerPoint, zFaceVector, cornerPoint);
          }

        box.AddPoint(cornerPoint);
        }
      }
    }
  box.GetBounds(bounds);
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

  double axis4_World[4] = { 0.0, 0.0, 0.0, 0.0 };
  axis4_World[axisIndex] = 1.0;
  this->ObjectToWorldMatrix->MultiplyPoint(axis4_World, axis4_World);

  axis_World[0] = axis4_World[0];
  axis_World[1] = axis4_World[1];
  axis_World[2] = axis4_World[2];
  vtkMath::Normalize(axis_World);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetXAxis(double axis_Node[3])
{
  this->GetAxis(0, axis_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetYAxis(double axis_Node[3])
{
  this->GetAxis(1, axis_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetZAxis(double axis_Node[3])
{
  this->GetAxis(2, axis_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetAxis(int axisIndex, double axis_Node[3])
{
  if (axisIndex < 0 || axisIndex >= 3)
    {
    vtkErrorMacro("Invalid axisIndex. Must be 0, 1, or 2.");
    return;
    }

  double axis4_Node[4] = { 0.0, 0.0, 0.0, 0.0 };
  axis4_Node[axisIndex] = 1.0;
  this->ObjectToNodeMatrix->MultiplyPoint(axis4_Node, axis4_Node);

  axis_Node[0] = axis4_Node[0];
  axis_Node[1] = axis4_Node[1];
  axis_Node[2] = axis4_Node[2];
  vtkMath::Normalize(axis_Node);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)
{
  Superclass::OnTransformNodeReferenceChanged(transformNode);
  this->UpdateObjectToWorldMatrix();
  this->UpdateInteractionHandleToWorldMatrix();
  this->UpdateAllMeasurements();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller == this->CurveInputPoly->GetPoints() || caller == this->GetParentTransformNode())
    {
    this->UpdateROIFromControlPoints();
    this->UpdateObjectToWorldMatrix();
    }
  else if (caller == this->ObjectToNodeMatrix.GetPointer() && event == vtkCommand::ModifiedEvent)
    {
    this->UpdateObjectToWorldMatrix();
    this->UpdateInteractionHandleToWorldMatrix();
    this->UpdateControlPointsFromROI();
    }
  else if (caller == this->InteractionHandleToWorldMatrix.GetPointer() && !this->IsUpdatingInteractionHandleToWorldMatrix)
    {
    // InteractionHandleToWorldMatrix was modified externally,
    // align the ObjectToNode matrix to the interaction handle directions.

    double center_World[4] = { 0.0, 0.0, 0.0, 1.0 };
    this->InteractionHandleToWorldMatrix->MultiplyPoint(center_World, center_World);

    vtkNew<vtkGeneralTransform> worldToNode;
    vtkMRMLTransformNode::GetTransformBetweenNodes(nullptr, this->GetParentTransformNode(), worldToNode);

    double xAxis_World[3] = {
      this->InteractionHandleToWorldMatrix->GetElement(0, 0),
      this->InteractionHandleToWorldMatrix->GetElement(1, 0),
      this->InteractionHandleToWorldMatrix->GetElement(2, 0) };
    double xAxis_Node[3] = { 1.0, 0.0, 0.0 };
    worldToNode->TransformVectorAtPoint(center_World, xAxis_World, xAxis_Node);
    vtkMath::Normalize(xAxis_Node);

    double yAxis_World[3] = {
      this->InteractionHandleToWorldMatrix->GetElement(0, 1),
      this->InteractionHandleToWorldMatrix->GetElement(1, 1),
      this->InteractionHandleToWorldMatrix->GetElement(2, 1) };
    double yAxis_Node[3] = { 0.0, 1.0, 0.0 };
    worldToNode->TransformVectorAtPoint(center_World, yAxis_World, yAxis_Node);
    vtkMath::Normalize(yAxis_Node);

    double zAxis_World[3] = {
      this->InteractionHandleToWorldMatrix->GetElement(0, 2),
      this->InteractionHandleToWorldMatrix->GetElement(1, 2),
      this->InteractionHandleToWorldMatrix->GetElement(2, 2) };
    double zAxis_Node[3] = { 0.0, 0.0, 1.0 };
    worldToNode->TransformVectorAtPoint(center_World, zAxis_World, zAxis_Node);
    vtkMath::Normalize(zAxis_Node);

    double center_Node[3] = { 0.0, 0.0, 0.0 };
    worldToNode->TransformPoint(center_World, center_Node);

    vtkNew<vtkMatrix4x4> newObjectToNodeMatrix;
    for (int i = 0; i < 3; ++i)
      {
      newObjectToNodeMatrix->SetElement(i, 0, xAxis_Node[i]);
      newObjectToNodeMatrix->SetElement(i, 1, yAxis_Node[i]);
      newObjectToNodeMatrix->SetElement(i, 2, zAxis_Node[i]);
      newObjectToNodeMatrix->SetElement(i, 3, center_Node[i]);
      }

    this->ObjectToNodeMatrix->DeepCopy(newObjectToNodeMatrix);
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
      this->MaximumNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;
      break;
    case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
      this->RequiredNumberOfControlPoints = NUMBER_OF_BOUNDING_BOX_CONTROL_POINTS;
      this->MaximumNumberOfControlPoints = NUMBER_OF_BOUNDING_BOX_CONTROL_POINTS;
      break;
    default:
      break;
    }
  this->UpdateROIFromControlPoints();
  this->Modified();
}

;
vtkVector3d GetCenterWorld();

//----------------------------------------------------------------------------
vtkVector3d vtkMRMLMarkupsROINode::GetCenter()
{
  vtkVector3d center(
    this->ObjectToNodeMatrix->GetElement(0, 3),
    this->ObjectToNodeMatrix->GetElement(1, 3),
    this->ObjectToNodeMatrix->GetElement(2, 3));
  return center;
}

//----------------------------------------------------------------------------
vtkVector3d vtkMRMLMarkupsROINode::GetCenterWorld()
{
  vtkVector3d centerWorld(
    this->ObjectToWorldMatrix->GetElement(0, 3),
    this->ObjectToWorldMatrix->GetElement(1, 3),
    this->ObjectToWorldMatrix->GetElement(2, 3));
  return centerWorld;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetCenter(double center_Node[3])
{
  if (!center_Node)
    {
    vtkErrorMacro("GetCenter: Invalid origin argument");
    return;
    }
  vtkVector3d center = this->GetCenter();
  center_Node[0] = center.GetX();
  center_Node[1] = center.GetY();
  center_Node[2] = center.GetZ();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetCenterWorld(double center_World[3])
{
  if (!center_World)
    {
    vtkErrorMacro("GetCenterWorld: Invalid origin argument");
    return;
    }
  vtkVector3d center = this->GetCenterWorld();
  center_World[0] = center.GetX();
  center_World[1] = center.GetY();
  center_World[2] = center.GetZ();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetCenterWorld(const double center_World[3])
{
  if (!center_World)
    {
    vtkErrorMacro("SetCenterWorld: Invalid origin argument");
    return;
    }

  double center_Node[3] = { 0.0, 0.0, 0.0 };
  this->TransformPointFromWorld(center_World, center_Node);
  this->SetCenter(center_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetCenterWorld(double x, double y, double z)
{
  double xyz[3] = { x, y, z };
  this->SetCenterWorld(xyz);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetCenter(const double center_Node[3])
{
  if (!center_Node)
    {
    vtkErrorMacro("SetCenter: Invalid origin argument");
    return;
    }

  if (this->ObjectToNodeMatrix->GetElement(0, 3) == center_Node[0] &&
    this->ObjectToNodeMatrix->GetElement(1, 3) == center_Node[1] &&
    this->ObjectToNodeMatrix->GetElement(2, 3) == center_Node[2])
    {
    return;
    }

  vtkNew<vtkMatrix4x4> newObjectToNodeMatrix;
  newObjectToNodeMatrix->DeepCopy(this->ObjectToNodeMatrix);
  for (int i = 0; i < 3; ++i)
    {
    newObjectToNodeMatrix->SetElement(i, 3, center_Node[i]);
    }

  MRMLNodeModifyBlocker blocker(this);
  this->ObjectToNodeMatrix->DeepCopy(newObjectToNodeMatrix);
  this->UpdateControlPointsFromROI();
  this->UpdateImplicitFunction();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetCenter(double x, double y, double z)
{
  double xyz[3] = { x, y, z };
  this->SetCenter(xyz);
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
  this->UpdateImplicitFunction();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetSizeWorld(double size_World[3])
{
  if (!size_World)
    {
    vtkErrorMacro("GetSizeWorld: Invalid arguments");
    return;
    }

  double xSize_Object[3] = { this->Size[0], 0.0, 0.0 };
  double ySize_Object[3] = { 0.0, this->Size[1], 0.0 };
  double zSize_Object[3] = { 0.0, 0.0, this->Size[2] };
  double center_Object[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(this->ObjectToWorldMatrix);
  size_World[0] = vtkMath::Norm(objectToWorldTransform->TransformVectorAtPoint(center_Object, xSize_Object));
  size_World[1] = vtkMath::Norm(objectToWorldTransform->TransformVectorAtPoint(center_Object, ySize_Object));
  size_World[2] = vtkMath::Norm(objectToWorldTransform->TransformVectorAtPoint(center_Object, zSize_Object));
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetSizeWorld(const double size_World[3])
{
  this->SetSizeWorld(size_World[0], size_World[1], size_World[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetSizeWorld(double x_World, double y_World, double z_World)
{
  vtkNew<vtkGeneralTransform> worldToNodeTransform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(nullptr, this->GetParentTransformNode(), worldToNodeTransform);

  double xAxis_World[3] = { 1.0, 0.0, 0.0 };
  double yAxis_World[3] = { 0.0, 1.0, 0.0 };
  double zAxis_World[3] = { 0.0, 0.0, 1.0 };
  double center_World[3] = { 0.0, 0.0, 0.0 };

  this->GetXAxisWorld(xAxis_World);
  this->GetYAxisWorld(yAxis_World);
  this->GetZAxisWorld(zAxis_World);
  this->GetCenterWorld(center_World);

  double xAxis_Node[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Node[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Node[3] = { 0.0, 1.0, 1.0 };

  worldToNodeTransform->TransformVectorAtPoint(center_World, xAxis_World, xAxis_Node);
  worldToNodeTransform->TransformVectorAtPoint(center_World, yAxis_World, yAxis_Node);
  worldToNodeTransform->TransformVectorAtPoint(center_World, zAxis_World, zAxis_Node);

  double xAxis_Scale = vtkMath::Norm(xAxis_Node);
  double yAxis_Scale = vtkMath::Norm(yAxis_Node);
  double zAxis_Scale = vtkMath::Norm(zAxis_Node);
  if (xAxis_Scale == 0.0 || yAxis_Scale == 0.0 || zAxis_Scale == 0.0)
    {
    return;
    }

  double size_Node[3] = { 0.0, 0.0, 0.0 };
  size_Node[0] = x_World * xAxis_Scale;
  size_Node[1] = y_World * yAxis_Scale;
  size_Node[2] = z_World * zAxis_Scale;
  this->SetSize(size_Node);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::SetInsideOut(bool insideOut)
{
  if (this->InsideOut == insideOut)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->InsideOut = insideOut;
  this->UpdateImplicitFunction();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateImplicitFunction()
{
  vtkImplicitSum* sumFunction = vtkImplicitSum::SafeDownCast(this->ImplicitFunction);
  vtkImplicitSum* sumFunctionWorld = vtkImplicitSum::SafeDownCast(this->ImplicitFunctionWorld);
  if (!sumFunction || !sumFunctionWorld)
    {
    vtkErrorMacro("vtkMRMLMarkupsROINode::UpdateImplicitFunction: Invalid implicit function");
    return;
    }

  sumFunction->RemoveAllFunctions();
  sumFunctionWorld->RemoveAllFunctions();

   if (this->ROIType == ROITypeBox || this->ROIType == ROITypeBoundingBox)
    {
    vtkNew<vtkBox> boxFunction;
    boxFunction->SetBounds(
      -this->Size[0] / 2.0, this->Size[0] / 2.0,
      -this->Size[1] / 2.0, this->Size[1] / 2.0,
      -this->Size[2] / 2.0, this->Size[2] / 2.0);

    // By setting the function weight to -1.0, the sign of the box function is flipped,
    // giving an "inside out" box.
    double functionWeight = this->InsideOut ? -1.0 : 1.0;
    sumFunction->AddFunction(boxFunction, functionWeight);
    sumFunctionWorld->AddFunction(boxFunction, functionWeight);

    vtkNew<vtkTransform> nodeToObject;
    nodeToObject->SetMatrix(this->ObjectToNodeMatrix);
    nodeToObject->Inverse();
    this->ImplicitFunction->SetTransform(nodeToObject);

    vtkNew<vtkTransform> worldToObject;
    worldToObject->SetMatrix(this->ObjectToWorldMatrix);
    worldToObject->Inverse();
    this->ImplicitFunctionWorld->SetTransform(worldToObject);
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

    this->UpdateImplicitFunction();
  }

  this->IsUpdatingROIFromControlPoints = false;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateBoxROIFromControlPoints()
{
  int numberOfControlPoints = this->GetNumberOfControlPoints();
  if (numberOfControlPoints == 0)
    {
    this->SetSize(0.0,  0.0, 0.0);
    return;
    }

  double oldSize[3] = { 0.0, 0.0, 0.0 };
  this->GetSize(oldSize);

  this->UpdateBoundingBoxROIFromControlPoints();
  if (this->GetNumberOfControlPoints() >= NUMBER_OF_BOX_CONTROL_POINTS)
    {
    double newSize[3] = { 0.0, 0.0, 0.0 };
    this->GetSize(newSize);

    // Find the shortest non-zero roi axis to pad the ROI if we need to give it some thickness.
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

    // If the required control points exist, then we should give the ROI some thickness along the 3rd axis.
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
    this->SetSize(newSize);
    }
  else if (this->GetNumberOfDefinedControlPoints() == this->RequiredNumberOfControlPoints)
    {
    this->SetSize(oldSize);
    }

  // If all of the control points have been defined, then the ROI has been defined. The control points can be removed.
  if (this->GetNumberOfDefinedControlPoints() >= NUMBER_OF_BOX_CONTROL_POINTS)
    {
    this->UpdateControlPointsFromBoxROI();
    }
  else if (this->GetNumberOfDefinedControlPoints() == 0)
    {
    this->RequiredNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;
    this->MaximumNumberOfControlPoints = NUMBER_OF_BOX_CONTROL_POINTS;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateBoundingBoxROIFromControlPoints()
{
  MRMLNodeModifyBlocker blocker(this);

  double bounds_Object[6] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, };
  if (this->GetNumberOfControlPoints() == 0)
    {
    for (int i = 0; i < 6; ++i)
      {
      bounds_Object[i] = 0.0;
      }
    }

  // Calculate the bounding box defined by the control points in ROI coordinates.
  vtkNew<vtkTransform> nodeToObjectTransform;
  nodeToObjectTransform->SetMatrix(this->ObjectToNodeMatrix);
  nodeToObjectTransform->Inverse();
  for (int pointIndex = 0; pointIndex < this->GetNumberOfControlPoints(); ++pointIndex)
    {
    double point_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPosition(pointIndex, point_Node);

    double point_Object[3] = { 0.0, 0.0, 0.0 };
    nodeToObjectTransform->TransformPoint(point_Node, point_Object);
    for (int i = 0; i < 3; ++i)
      {
      bounds_Object[2 * i] = std::min(bounds_Object[2 * i], point_Object[i]);
      bounds_Object[2 * i + 1] = std::max(bounds_Object[2 * i + 1], point_Object[i]);
      }
    }

  double center_Object[4] = { 0.0, 0.0, 0.0, 1.0 };
  double newSize[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; ++i)
    {
    newSize[i] = bounds_Object[2 * i + 1] - bounds_Object[2 * i];
    center_Object[i] = (bounds_Object[2 * i + 1] + bounds_Object[2 * i]) / 2.0;
    }

  this->SetSize(newSize);

  double center_Node[4] = { 0.0, 0.0, 0.0, 0.0 };
  this->ObjectToNodeMatrix->MultiplyPoint(center_Object, center_Node);
  this->SetCenter(center_Node);
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
      case vtkMRMLMarkupsROINode::ROITypeBox:
        this->UpdateControlPointsFromBoxROI();
        break;
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
  vtkNew<vtkTransform> nodeToObjectTransform;
  nodeToObjectTransform->SetMatrix(this->ObjectToNodeMatrix);
  nodeToObjectTransform->Inverse();

  double bounds_Object[6] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };
  for (int pointIndex = 0; pointIndex < this->GetNumberOfControlPoints(); ++pointIndex)
    {
    double point_Node[3] = { 0.0, 0.0, 0.0 };
    this->GetNthControlPointPosition(pointIndex, point_Node);

    double point_Object[3] = { 0.0, 0.0, 0.0 };
    nodeToObjectTransform->TransformPoint(point_Node, point_Object);
    for (int i = 0; i < 3; ++i)
      {
      bounds_Object[2 * i] = std::min(bounds_Object[2 * i], point_Object[i]);
      bounds_Object[2 * i + 1] = std::max(bounds_Object[2 * i + 1], point_Object[i]);
      }
    }
  if (this->GetNumberOfControlPoints() == 0)
    {
    for (int i = 0; i < 6; ++i)
      {
      bounds_Object[i] = 0.0;
      }
    }

  double scale_Object[3] = { 1.0, 1.0, 1.0 };
  double translation_Object[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; ++i)
    {
    double oldSize = bounds_Object[2 * i + 1] - bounds_Object[2 * i];
    scale_Object[i] = this->Size[i] / oldSize;
    translation_Object[i] = -(bounds_Object[2 * i + 1] + bounds_Object[2 * i]) / 2.0;
    }

  // Based on the difference between the side lengths and the bounding box defined by the control points,
  // we apply a transform to scale the control point positions to match the expected ROI dimensions.
  vtkNew<vtkTransform> nodeToScaledNodeTransform;
  nodeToScaledNodeTransform->PostMultiply();
  nodeToScaledNodeTransform->Concatenate(nodeToObjectTransform);
  nodeToScaledNodeTransform->Translate(translation_Object);
  nodeToScaledNodeTransform->Scale(scale_Object);
  nodeToScaledNodeTransform->Concatenate(this->ObjectToNodeMatrix);

  vtkNew<vtkTransformPolyDataFilter> nodeToScaledNodeTransformFilter;
  nodeToScaledNodeTransformFilter->SetInputData(this->CurveInputPoly);
  nodeToScaledNodeTransformFilter->SetTransform(nodeToScaledNodeTransform);

  vtkNew<vtkGeneralTransform> nodeToWorldTransform;
  if (this->GetParentTransformNode())
    {
    this->GetParentTransformNode()->GetTransformToWorld(nodeToWorldTransform);
    }

  vtkNew<vtkTransformPolyDataFilter> nodeToWorldTransformFilter;
  nodeToWorldTransformFilter->SetTransform(nodeToWorldTransform);
  nodeToWorldTransformFilter->SetInputConnection(nodeToScaledNodeTransformFilter->GetOutputPort());
  nodeToWorldTransformFilter->Update();

  this->SetControlPointPositionsWorld(nodeToWorldTransformFilter->GetOutput()->GetPoints());
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateControlPointsFromBoxROI()
{
  if (vtkMath::Norm(this->Size) == 0.0)
    {
    return;
    }

  double center_World[3] = { 0.0, 0.0, 0.0 };
  this->GetCenterWorld(center_World);
  vtkNew<vtkPoints> points_World;
  points_World->InsertNextPoint(center_World);
  this->SetControlPointPositionsWorld(points_World);
  this->MaximumNumberOfControlPoints = 1;
  this->RequiredNumberOfControlPoints = 1;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateInteractionHandleToWorldMatrix()
{
  bool wasUpdatingInteractionHandleToWorldMatrix = this->IsUpdatingInteractionHandleToWorldMatrix;
  this->IsUpdatingInteractionHandleToWorldMatrix = true;

  vtkNew<vtkMatrix4x4> newInteractionHandleToWorld;
  newInteractionHandleToWorld->DeepCopy(this->ObjectToWorldMatrix);
  for (int j = 0; j < 3; ++j)
    {
    double axis[4] = { 0.0, 0.0, 0.0, 0.0 };
    axis[j] = 1.0;
    double size = vtkMath::Norm(newInteractionHandleToWorld->MultiplyDoublePoint(axis));
    for (int i = 0; i < 3; ++i)
      {
      double element = newInteractionHandleToWorld->GetElement(i, j);
      newInteractionHandleToWorld->SetElement(i, j, element / size);
      }
    }
  this->InteractionHandleToWorldMatrix->DeepCopy(newInteractionHandleToWorld);

  this->IsUpdatingInteractionHandleToWorldMatrix = wasUpdatingInteractionHandleToWorldMatrix;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::UpdateObjectToWorldMatrix()
{
  vtkNew<vtkGeneralTransform> nodeToWorldTansform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(this->GetParentTransformNode(), nullptr, nodeToWorldTansform);

  vtkNew<vtkMatrix4x4> newObjectToWorldMatrix;
  this->GenerateOrthogonalMatrix(this->ObjectToNodeMatrix, newObjectToWorldMatrix, nodeToWorldTansform);
  this->ObjectToWorldMatrix->DeepCopy(newObjectToWorldMatrix);

  this->UpdateImplicitFunction();

  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsROINode::GetXYZ(double center[3])
{
  this->GetCenter(center);
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsROINode::SetXYZ(double center[3])
{
  this->SetCenter(center);
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
void vtkMRMLMarkupsROINode::GetPlanes(vtkPlanes* planes, bool insideOut/*=false*/)
{
  if (!planes)
    {
    vtkErrorMacro("GetPlanes: Invalid planes");
    return;
    }

  double center_Node[3] = { 0.0, 0.0, 0.0 };
  this->GetCenter(center_Node);

  vtkNew<vtkDoubleArray> normals;
  normals->SetNumberOfComponents(3);

  vtkNew<vtkPoints> points;

  double lNormal_Node[3] = { -1.0, 0.0, 0.0 };
  this->GetAxis(0, lNormal_Node);
  vtkMath::MultiplyScalar(lNormal_Node, -1.0);
  normals->InsertNextTuple3(lNormal_Node[0], lNormal_Node[1], lNormal_Node[2]);

  double lOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(lNormal_Node, 0.5 * this->Size[0]);
  vtkMath::Add(center_Node, lNormal_Node, lOrigin_Node);
  points->InsertNextPoint(lOrigin_Node[0], lOrigin_Node[1], lOrigin_Node[2]);

  double rNormal_Node[3] = { 1.0, 0.0, 0.0 };
  this->GetAxis(0, rNormal_Node);
  normals->InsertNextTuple3(rNormal_Node[0], rNormal_Node[1], rNormal_Node[2]);

  double rOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(rNormal_Node, 0.5 * this->Size[0]);
  vtkMath::Add(center_Node, rNormal_Node, rOrigin_Node);
  points->InsertNextPoint(rOrigin_Node[0], rOrigin_Node[1], rOrigin_Node[2]);

  double pNormal_Node[3] = { 0.0, -1.0, 0.0 };
  this->GetAxis(1, pNormal_Node);
  vtkMath::MultiplyScalar(pNormal_Node, -1.0);
  normals->InsertNextTuple3(pNormal_Node[0], pNormal_Node[1], pNormal_Node[2]);

  double pOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(pNormal_Node, 0.5 * this->Size[1]);
  vtkMath::Add(center_Node, pNormal_Node, pOrigin_Node);
  points->InsertNextPoint(pOrigin_Node[0], pOrigin_Node[1], pOrigin_Node[2]);

  double aNormal_Node[3] = { 0.0, 1.0, 0.0 };
  this->GetAxis(1, aNormal_Node);
  normals->InsertNextTuple3(aNormal_Node[0], aNormal_Node[1], aNormal_Node[2]);

  double aOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(aNormal_Node, 0.5 * this->Size[1]);
  vtkMath::Add(center_Node, aNormal_Node, aOrigin_Node);
  points->InsertNextPoint(aOrigin_Node[0], aOrigin_Node[1], aOrigin_Node[2]);

  double iNormal_Node[3] = { 0.0, 0.0, -1.0 };
  this->GetAxis(2, iNormal_Node);
  vtkMath::MultiplyScalar(iNormal_Node, -1.0);
  normals->InsertNextTuple3(iNormal_Node[0], iNormal_Node[1], iNormal_Node[2]);

  double iOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(iNormal_Node, 0.5 * this->Size[2]);
  vtkMath::Add(center_Node, iNormal_Node, iOrigin_Node);
  points->InsertNextPoint(iOrigin_Node[0], iOrigin_Node[1], iOrigin_Node[2]);

  double sNormal_Node[3] = { 0.0, 0.0, 1.0 };
  this->GetAxis(2, sNormal_Node);
  normals->InsertNextTuple3(sNormal_Node[0], sNormal_Node[1], sNormal_Node[2]);

  double sOrigin_Node[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(sNormal_Node, 0.5 * this->Size[2]);
  vtkMath::Add(center_Node, sNormal_Node, sOrigin_Node);
  points->InsertNextPoint(sOrigin_Node[0], sOrigin_Node[1], sOrigin_Node[2]);

  if (insideOut)
    {
    for (int i = 0; i < normals->GetNumberOfTuples(); ++i)
      {
      double* normal = normals->GetTuple3(i);
      normals->SetTuple3(i, -normal[0], -normal[1], -normal[2]);
      }
    }
  planes->SetNormals(normals);
  planes->SetPoints(points);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetPlanesWorld(vtkPlanes* planes, bool insideOut/*=false*/)
{
  if (!planes)
    {
    vtkErrorMacro("GetPlanesWorld: Invalid planes");
    return;
    }

  double center_World[3] = { 0.0, 0.0, 0.0 };
  this->GetCenterWorld(center_World);

  vtkNew<vtkDoubleArray> normals;
  normals->SetNumberOfComponents(3);

  vtkNew<vtkPoints> points;

  double lNormal_World[3] = { -1.0, 0.0, 0.0 };
  this->GetAxisWorld(0, lNormal_World);
  vtkMath::MultiplyScalar(lNormal_World, -1.0);
  normals->InsertNextTuple3(lNormal_World[0], lNormal_World[1], lNormal_World[2]);

  double lOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(lNormal_World, 0.5 * this->Size[0]);
  vtkMath::Add(center_World, lNormal_World, lOrigin_World);
  points->InsertNextPoint(lOrigin_World[0], lOrigin_World[1], lOrigin_World[2]);

  double rNormal_World[3] = { 1.0, 0.0, 0.0 };
  this->GetAxisWorld(0, rNormal_World);
  normals->InsertNextTuple3(rNormal_World[0], rNormal_World[1], rNormal_World[2]);

  double rOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(rNormal_World, 0.5 * this->Size[0]);
  vtkMath::Add(center_World, rNormal_World, rOrigin_World);
  points->InsertNextPoint(rOrigin_World[0], rOrigin_World[1], rOrigin_World[2]);

  double pNormal_World[3] = { 0.0, -1.0, 0.0 };
  this->GetAxisWorld(1, pNormal_World);
  vtkMath::MultiplyScalar(pNormal_World, -1.0);
  normals->InsertNextTuple3(pNormal_World[0], pNormal_World[1], pNormal_World[2]);

  double pOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(pNormal_World, 0.5 * this->Size[1]);
  vtkMath::Add(center_World, pNormal_World, pOrigin_World);
  points->InsertNextPoint(pOrigin_World[0], pOrigin_World[1], pOrigin_World[2]);

  double aNormal_World[3] = { 0.0, 1.0, 0.0 };
  this->GetAxisWorld(1, aNormal_World);
  normals->InsertNextTuple3(aNormal_World[0], aNormal_World[1], aNormal_World[2]);

  double aOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(aNormal_World, 0.5 * this->Size[1]);
  vtkMath::Add(center_World, aNormal_World, aOrigin_World);
  points->InsertNextPoint(aOrigin_World[0], aOrigin_World[1], aOrigin_World[2]);

  double iNormal_World[3] = { 0.0, 0.0, -1.0 };
  this->GetAxisWorld(2, iNormal_World);
  vtkMath::MultiplyScalar(iNormal_World, -1.0);
  normals->InsertNextTuple3(iNormal_World[0], iNormal_World[1], iNormal_World[2]);

  double iOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(iNormal_World, 0.5 * this->Size[2]);
  vtkMath::Add(center_World, iNormal_World, iOrigin_World);
  points->InsertNextPoint(iOrigin_World[0], iOrigin_World[1], iOrigin_World[2]);

  double sNormal_World[3] = { 0.0, 0.0, 1.0 };
  this->GetAxisWorld(2, sNormal_World);
  normals->InsertNextTuple3(sNormal_World[0], sNormal_World[1], sNormal_World[2]);

  double sOrigin_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::MultiplyScalar(sNormal_World, 0.5 * this->Size[2]);
  vtkMath::Add(center_World, sNormal_World, sOrigin_World);
  points->InsertNextPoint(sOrigin_World[0], sOrigin_World[1], sOrigin_World[2]);

  if (insideOut)
    {
    for (int i = 0; i < normals->GetNumberOfTuples(); ++i)
      {
      double* normal = normals->GetTuple3(i);
      normals->SetTuple3(i, -normal[0], -normal[1], -normal[2]);
      }
    }
  planes->SetNormals(normals);
  planes->SetPoints(points);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetTransformedPlanes(vtkPlanes* planes, bool insideOut/*=false*/)
{
  if (!planes)
    {
    vtkErrorMacro("GetTransformedPlanes: Invalid planes");
    return;
    }

  this->GetPlanesWorld(planes, insideOut);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROINode::IsPointInROI(double point_Node[3])
{
  return this->ImplicitFunction->FunctionValue(point_Node) <= 0;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROINode::IsPointInROIWorld(double point_World[3])
{
  return this->ImplicitFunctionWorld->FunctionValue(point_World) <= 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GenerateOrthogonalMatrix(vtkMatrix4x4* inputMatrix,
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
  vtkMRMLMarkupsROINode::GenerateOrthogonalMatrix(xAxis, yAxis, zAxis, origin, outputMatrix, transform, applyScaling);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GenerateOrthogonalMatrix(double xAxis[3], double yAxis[3], double zAxis[3], double origin[3],
  vtkMatrix4x4* outputMatrix, vtkAbstractTransform* transform/*=nullptr*/, bool applyScaling/*=true*/)
{
  if (!xAxis || !yAxis || !zAxis || !origin || !transform || !outputMatrix)
    {
    vtkErrorWithObjectMacro(nullptr, "GenerateOrthogonalMatrix: Invalid arguments");
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

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::WriteCLI(std::vector<std::string>& commandLine, std::string prefix,
         int coordinateSystem, int vtkNotUsed(multipleFlag))
{
  // check if the coordinate system flag is set to LPS, otherwise assume RAS
  bool useLPS = (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS);

  if (!prefix.empty())
    {
    commandLine.push_back(prefix);
    }

  std::stringstream ss;

  // Note: CLI interface uses node coordinate system (not World).
  // This means that parent transforms are ignored.

  // ROI center
  double centerPosition[3] = { 0.0, 0.0, 0.0 };
  this->GetCenter(centerPosition);
  if (useLPS)
    {
    centerPosition[0] = -centerPosition[0];
    centerPosition[1] = -centerPosition[1];
    }
  ss << centerPosition[0] << "," << centerPosition[1] << "," << centerPosition[2];

  // ROI radius
  double roiDiameter[3] = { 0.0, 0.0, 0.0 };
  this->GetSize(roiDiameter);
  ss << "," << roiDiameter[0] / 2.0 << "," << roiDiameter[1] / 2.0 << "," << roiDiameter[2] / 2.0;

  // ROI orientation (for backward compatibility, only write it out if rotated)
  if (this->GetObjectToNodeMatrixRotated())
    {
    // Get axis directions in the requested coordinate system
    double axes[3][3];
    for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
      {
      this->GetAxis(axisIndex, axes[axisIndex]);
      if (useLPS)
        {
        axes[axisIndex][0] = -axes[axisIndex][0];
        axes[axisIndex][1] = -axes[axisIndex][1];
        }
      }
    // To conform with the usual matrix element ordering, the order is:
    // x0, y0, z0, x1, y1, z1, x2, y2, z2.
    for (int row = 0; row < 3; ++row)
      {
      ss << "," << axes[0][row] << "," << axes[1][row] << "," << axes[2][row];
      }
  }

  commandLine.push_back(ss.str());
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsROINode::GetObjectToNodeMatrixRotated()
{
  vtkMatrix4x4* objectToNode = this->GetObjectToNodeMatrix();
  const double tolerance = 1e-3;
  for (int row = 0; row < 3; row++)
    {
    for (int col = 0; col < 3; col++)
      {
      double expectedValue = (row == col ? 1.0 : 0.0);
      if (fabs(objectToNode->GetElement(row, col)-expectedValue) > tolerance)
        {
        // rotated
        return true;
        }
      }
    }
  // not rotated
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsROINode::GetObjectBounds(double bounds[6])
{
  double diameter[3] = { 0.0, 0.0, 0.0 };
  this->GetSize(diameter);
  bounds[0] = -diameter[0] / 2.0;
  bounds[1] = diameter[0] / 2.0;
  bounds[2] = -diameter[1] / 2.0;
  bounds[3] = diameter[1] / 2.0;
  bounds[4] = -diameter[2] / 2.0;
  bounds[5] = diameter[2] / 2.0;
}
