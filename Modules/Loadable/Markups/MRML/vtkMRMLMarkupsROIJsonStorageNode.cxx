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
#include "vtkMRMLMarkupsROIJsonStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"

#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

#include <vtkMRMLMarkupsROIJsonStorageNode_Private.h>

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLMarkupsROIJsonStorageNode::vtkInternalROI::vtkInternalROI(vtkMRMLMarkupsROIJsonStorageNode* external)
  : vtkMRMLMarkupsJsonStorageNode::vtkInternal(external)
{
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsROIJsonStorageNode::vtkInternalROI::~vtkInternalROI() = default;

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROIJsonStorageNode::vtkInternalROI::WriteMarkup(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsNode* markupsNode)
{
  bool success = true;
  success = success && this->WriteBasicProperties(writer, markupsNode);
  success = success && this->WriteROIProperties(writer, vtkMRMLMarkupsROINode::SafeDownCast(markupsNode));
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
bool vtkMRMLMarkupsROIJsonStorageNode::vtkInternalROI::WriteROIProperties(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsROINode* roiNode)
{
  if (!roiNode)
    {
    return false;
    }

  writer.Key("roiType");
  writer.String(roiNode->GetROITypeAsString(roiNode->GetROIType()));

  int coordinateSystem = this->External->GetCoordinateSystem();
  double center_Node[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetCenter(center_Node);
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    center_Node[0] = -center_Node[0];
    center_Node[1] = -center_Node[1];
    }
  writer.Key("center");
  this->WriteVector(writer, center_Node);

  double orientationMatrix[9] = { 0.0 };
  vtkMatrix4x4* objectToNodeMatrix = roiNode->GetObjectToNodeMatrix();
  for (int i = 0; i < 3; ++i)
    {
    orientationMatrix[3 * i]     = objectToNodeMatrix->GetElement(i, 0);
    orientationMatrix[3 * i + 1] = objectToNodeMatrix->GetElement(i, 1);
    orientationMatrix[3 * i + 2] = objectToNodeMatrix->GetElement(i, 2);
    }
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    for (int i = 0; i < 6; ++i)
      {
      orientationMatrix[i] = -orientationMatrix[i];
      }
    }
  writer.Key("orientation");
  this->WriteVector(writer, orientationMatrix, 9);

  double size[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSize(size);
  writer.Key("size");
  this->WriteVector(writer, size);

  writer.Key("insideOut");
  writer.Bool(roiNode->GetInsideOut());

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROIJsonStorageNode::vtkInternalROI::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupsObject)
{
  if (!markupsNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternalROI::UpdateMarkupsNodeFromJsonDocument failed: invalid markupsNode");
    return false;
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(markupsNode);

  bool success = true;

  if (markupsObject.HasMember("roiType"))
    {
    rapidjson::Value& roiTypeItem = markupsObject["roiType"];
    std::string roiType = roiTypeItem.GetString();
    roiNode->SetROIType(roiNode->GetROITypeFromString(roiType.c_str()));
    }

  int coordinateSystem = this->External->GetCoordinateSystem();
  double center_Node[3] = { 0.0, 0.0, 0.0 };
  if (markupsObject.HasMember("center"))
    {
    rapidjson::Value& centerItem = markupsObject["center"];
    if (!this->ReadVector(centerItem, center_Node))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "File reading failed: center position must be a 3-element numeric array.");
      return false;
      }
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      center_Node[0] = -center_Node[0];
      center_Node[1] = -center_Node[1];
      }
    }

  if (markupsObject.HasMember("size"))
    {
    rapidjson::Value& sizeItem = markupsObject["size"];
    double size[3] = { 0.0, 0.0, 0.0 };
    success &= this->ReadVector(sizeItem, size);
    roiNode->SetSize(size);
    }

  double orientationMatrix[9] = { 0.0 };
  if (markupsObject.HasMember("orientation"))
    {
    rapidjson::Value& orientationItem = markupsObject["orientation"];
    if (!this->ReadVector(orientationItem, orientationMatrix, 9))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "File reading failed: orientation 9-element numeric array.");
      return false;
      }
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      for (int i = 0; i < 6; i++)
        {
        orientationMatrix[i] = -orientationMatrix[i];
        }
      }
    }
  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  for (int i = 0; i < 3; ++i)
    {
    objectToNodeMatrix->SetElement(i, 0, orientationMatrix[3*i]);
    objectToNodeMatrix->SetElement(i, 1, orientationMatrix[3*i + 1]);
    objectToNodeMatrix->SetElement(i, 2, orientationMatrix[3*i + 2]);
    objectToNodeMatrix->SetElement(i, 3, center_Node[i]);
    }
  roiNode->GetObjectToNodeMatrix()->DeepCopy(objectToNodeMatrix);

  if (markupsObject.HasMember("insideOut"))
    {
    rapidjson::Value& insideOutItem = markupsObject["insideOut"];
    bool insideOut = insideOutItem.GetBool();
    roiNode->SetInsideOut(insideOut);
    }

  return vtkInternal::UpdateMarkupsNodeFromJsonValue(markupsNode, markupsObject);
}


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsROIJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsROIJsonStorageNode::vtkMRMLMarkupsROIJsonStorageNode()
{
  this->Internal = new vtkInternalROI(this);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsROIJsonStorageNode::~vtkMRMLMarkupsROIJsonStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROIJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsROINode");
}
