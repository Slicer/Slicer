/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

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

#include <vtkCodedEntry.h>
#include "vtkMRMLMarkupsPlaneJsonStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"

#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

#include <vtkMRMLMarkupsPlaneJsonStorageNode_Private.h>

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::vtkInternalPlane(vtkMRMLMarkupsPlaneJsonStorageNode* external)
  : vtkMRMLMarkupsJsonStorageNode::vtkInternal(external)
{
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::~vtkInternalPlane() = default;

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::WriteMarkup(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsNode* markupsNode)
{
  bool success = true;
  success = success && this->WriteBasicProperties(writer, markupsNode);
  success = success && this->WritePlaneProperties(writer, vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode));
  success = success && this->WriteControlPoints(writer, markupsNode);
  success = success && this->WriteMeasurements(writer, markupsNode);
  if (success)
    {
    vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
    if (displayNode)
      {
      success = success && this->WriteDisplayProperties(writer, displayNode);
      }
    }
  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::WritePlaneProperties(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsPlaneNode* planeNode)
{
  if (!planeNode)
    {
    return false;
    }

  writer.Key("planeType");
  writer.String(planeNode->GetPlaneTypeAsString(planeNode->GetPlaneType()));

  writer.Key("sizeMode");
  writer.String(planeNode->GetSizeModeAsString(planeNode->GetSizeMode()));

  writer.Key("autoScalingFactor");
  writer.Double(planeNode->GetAutoSizeScalingFactor());

  int coordinateSystem = this->External->GetCoordinateSystem();
  double center_Node[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetOrigin(center_Node);
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    center_Node[0] = -center_Node[0];
    center_Node[1] = -center_Node[1];
    }
  writer.Key("center");
  this->WriteVector(writer, center_Node);

  double normal_Node[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetNormal(normal_Node);
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    normal_Node[0] = -normal_Node[0];
    normal_Node[1] = -normal_Node[1];
    }
  writer.Key("normal");
  this->WriteVector(writer, normal_Node);

  double* objectToBase = planeNode->GetObjectToBaseMatrix()->GetData();
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    double objectToBaseLPS[16] = { 0.0 };
    for (int i = 0; i < 16; ++i)
      {
      objectToBaseLPS[i] = objectToBase[i];
      }
    for (int i = 0; i < 8; ++i)
      {
      objectToBaseLPS[i] *= -1.0;
      }
    writer.Key("objectToBase");
    this->WriteVector(writer, objectToBaseLPS, 16);
    }
  else
    {
    writer.Key("objectToBase");
    this->WriteVector(writer, objectToBase, 16);
    }

  double* baseToNode = planeNode->GetBaseToNodeMatrix()->GetData();
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    double baseToNodeLPS[16] = { 0.0 };
    for (int i = 0; i < 16; ++i)
      {
      baseToNodeLPS[i] = baseToNode[i];
      }
    for (int i = 0; i < 8; ++i)
      {
      baseToNodeLPS[i] *= -1.0;
      }
    writer.Key("baseToNode");
    this->WriteVector(writer, baseToNodeLPS, 16);
    }
  else
    {
    writer.Key("baseToNode");
    this->WriteVector(writer, baseToNode, 16);
    }

  double orientation[9] = { 0.0 };

  double xAxis_Node[3] = { 0.0, 0.0, 0.0 };
  double yAxis_Node[3] = { 0.0, 0.0, 0.0 };
  double zAxis_Node[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetAxes(xAxis_Node, yAxis_Node, zAxis_Node);
  double origin_Node[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetOrigin(origin_Node);
  for (int i = 0; i < 3; ++i)
    {
    orientation[3 * i] = xAxis_Node[i];
    orientation[3 * i + 1] = yAxis_Node[i];
    orientation[3 * i + 2] = zAxis_Node[i];
    }
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    for (int i = 0; i < 6; ++i)
      {
      orientation[i] = -orientation[i];
      }
    }
  writer.Key("orientation");
  this->WriteVector(writer, orientation, 9);

  double size[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetSize(size);
  writer.Key("size");
  this->WriteVector(writer, size, 3);

  double planeBounds[4] = { 0.0, 0.0, 0.0, 0.0 };
  planeNode->GetPlaneBounds(planeBounds);
  writer.Key("planeBounds");
  this->WriteVector(writer, planeBounds, 4);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupsObject)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);
  if (!planeNode)
    {
    vtkErrorWithObjectMacro(this->External,
      "vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::UpdateMarkupsNodeFromJsonDocument failed: invalid markupsNode");
    return false;
    }

  MRMLNodeModifyBlocker blocker(planeNode);

  if (markupsObject.HasMember("planeType"))
    {
    rapidjson::Value& planeTypeItem = markupsObject["planeType"];
    std::string planeType = planeTypeItem.GetString();
    planeNode->SetPlaneType(planeNode->GetPlaneTypeFromString(planeType.c_str()));
    }
  else
    {
    /// Plane was created when the only type of plane was defined with 3 points.
    planeNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);
    }

  if (!vtkInternal::UpdateMarkupsNodeFromJsonValue(markupsNode, markupsObject))
    {
    return false;
    }

  int coordinateSystem = this->External->GetCoordinateSystem();

  if (markupsObject.HasMember("objectToBase"))
    {
    double objectToBase[16] = { 0.0 };
    rapidjson::Value& objectToBaseItem= markupsObject["objectToBase"];
    if (!this->ReadVector(objectToBaseItem, objectToBase, 16))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "File reading failed: objectToBase 16-element numeric array.");
      return false;
      }
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      for (int i = 0; i < 8; i++)
        {
        objectToBase[i] = -objectToBase[i];
        }
      }
    planeNode->GetObjectToBaseMatrix()->DeepCopy(objectToBase);
    }

  if (markupsObject.HasMember("baseToNode"))
    {
    double baseToNode[16] = { 0.0 };
    rapidjson::Value& baseToNodeItem = markupsObject["baseToNode"];
    if (!this->ReadVector(baseToNodeItem, baseToNode, 16))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "File reading failed: objectToBase 16-element numeric array.");
      return false;
      }
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      for (int i = 0; i < 8; i++)
        {
        baseToNode[i] = -baseToNode[i];
        }
      }
    planeNode->GetBaseToNodeMatrix()->DeepCopy(baseToNode);
    }


  if (markupsObject.HasMember("sizeMode"))
    {
    rapidjson::Value& sizeModeItem = markupsObject["sizeMode"];
    std::string sizeMode = sizeModeItem.GetString();
    planeNode->SetSizeMode(planeNode->GetSizeModeFromString(sizeMode.c_str()));
    }

  if (markupsObject.HasMember("autoScalingFactor"))
    {
    rapidjson::Value& autoScalingFactorItem = markupsObject["autoScalingFactor"];
    double autoScalingFactor = autoScalingFactorItem.GetDouble();
    planeNode->SetAutoSizeScalingFactor(autoScalingFactor);
    }

  if (markupsObject.HasMember("size"))
    {
    rapidjson::Value& sizeItem = markupsObject["size"];
    double size[3] = { 0.0, 0.0, 0.0 };
    if (!this->ReadVector(sizeItem, size, 3))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "File reading failed: size 3-element numeric array.");
      return false;
      }
    planeNode->SetSize(size);
    }

  if (markupsObject.HasMember("planeBounds"))
    {
    rapidjson::Value& planeBoundsItem = markupsObject["planeBounds"];
    double planeBounds[4] = { 0.0, 0.0, 0.0, 0.0 };
    if (!this->ReadVector(planeBoundsItem, planeBounds, 4))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "File reading failed: planeBounds 4-element numeric array.");
      return false;
      }
    planeNode->SetPlaneBounds(planeBounds);
    }

  return true;
}


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsPlaneJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneJsonStorageNode::vtkMRMLMarkupsPlaneJsonStorageNode()
{
  this->Internal = new vtkInternalPlane(this);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneJsonStorageNode::~vtkMRMLMarkupsPlaneJsonStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsPlaneNode");
}
