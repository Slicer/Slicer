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

  double objectToBase[16] = { 0.0 };
  vtkMatrix4x4* objectToBaseMatrix = planeNode->GetObjectToBaseMatrix();
  for (int i = 0; i < 4; ++i)
    {
    objectToBase[4 * i]     = objectToBaseMatrix->GetElement(i, 0);
    objectToBase[4 * i + 1] = objectToBaseMatrix->GetElement(i, 1);
    objectToBase[4 * i + 2] = objectToBaseMatrix->GetElement(i, 2);
    objectToBase[4 * i + 3] = objectToBaseMatrix->GetElement(i, 3);
    }
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    for (int i = 0; i < 8; ++i)
      {
      objectToBase[i] = -objectToBase[i];
      }
    }
  writer.Key("objectToBase");
  this->WriteVector(writer, objectToBase, 16);

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
  this->WriteVector(writer, size);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::vtkInternalPlane::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupsObject)
{
  if (!markupsNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternalPlane::UpdateMarkupsNodeFromJsonDocument failed: invalid markupsNode");
    return false;
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);

  bool success = true;
  success = vtkInternal::UpdateMarkupsNodeFromJsonValue(markupsNode, markupsObject);

  int coordinateSystem = this->External->GetCoordinateSystem();

  double objectToBase[16] = { 0.0 };
  if (markupsObject.HasMember("objectToBase"))
    {
    rapidjson::Value& objectToBaseItem= markupsObject["objectToBase"];
    if (!this->ReadVector(objectToBaseItem, objectToBase, 16))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
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
    }
  vtkNew<vtkMatrix4x4> objectToBaseMatrix;
  for (int i = 0; i < 4; ++i)
    {
    objectToBaseMatrix->SetElement(i, 0, objectToBase[4*i]);
    objectToBaseMatrix->SetElement(i, 1, objectToBase[4*i + 1]);
    objectToBaseMatrix->SetElement(i, 2, objectToBase[4*i + 2]);
    objectToBaseMatrix->SetElement(i, 3, objectToBase[4*i + 3]);
    }
  planeNode->GetObjectToBaseMatrix()->DeepCopy(objectToBaseMatrix);

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
    success &= this->ReadVector(sizeItem, size);
    planeNode->SetSize(size);
    }

  return success;
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
