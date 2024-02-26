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
#include "vtkMRMLMarkupsJsonElement.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsROIJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsROIJsonStorageNode::vtkMRMLMarkupsROIJsonStorageNode() {}

//----------------------------------------------------------------------------
vtkMRMLMarkupsROIJsonStorageNode::~vtkMRMLMarkupsROIJsonStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROIJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLMarkupsROINode");
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROIJsonStorageNode::WriteBasicProperties(vtkMRMLMarkupsJsonWriter* writer,
                                                            vtkMRMLMarkupsNode* markupsNode)
{
  if (!vtkMRMLMarkupsJsonStorageNode::WriteBasicProperties(writer, markupsNode))
  {
    return false;
  }

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(markupsNode);
  if (!roiNode)
  {
    vtkErrorWithObjectMacro(this, "vtkMRMLMarkupsROINode::WriteBasicProperties failed: invalid markupsNode");
    return false;
  }

  writer->WriteStringProperty("roiType", roiNode->GetROITypeAsString(roiNode->GetROIType()));

  int coordinateSystem = this->GetCoordinateSystem();

  double center_Node[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetCenter(center_Node);
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
  {
    center_Node[0] = -center_Node[0];
    center_Node[1] = -center_Node[1];
  }
  writer->WriteVectorProperty("center", center_Node);

  double orientationMatrix[9] = { 0.0 };
  vtkMatrix4x4* objectToNodeMatrix = roiNode->GetObjectToNodeMatrix();
  for (int i = 0; i < 3; ++i)
  {
    orientationMatrix[3 * i] = objectToNodeMatrix->GetElement(i, 0);
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
  writer->WriteVectorProperty("orientation", orientationMatrix, 9);

  writer->WriteVectorProperty("size", roiNode->GetSize());
  writer->WriteBoolProperty("insideOut", roiNode->GetInsideOut());

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsROIJsonStorageNode::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode,
                                                                      vtkMRMLMarkupsJsonElement* markupsObject)
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(markupsNode);
  if (!roiNode)
  {
    vtkErrorWithObjectMacro(
      this, "vtkMRMLMarkupsROIJsonStorageNode::UpdateMarkupsNodeFromJsonValue failed: invalid markupsNode");
    return false;
  }

  MRMLNodeModifyBlocker blocker(markupsNode);

  if (markupsObject->HasMember("roiType"))
  {
    roiNode->SetROIType(roiNode->GetROITypeFromString(markupsObject->GetStringProperty("roiType").c_str()));
  }

  int coordinateSystem = this->GetCoordinateSystem();

  double center_Node[3] = { 0.0, 0.0, 0.0 };
  if (markupsObject->HasMember("center"))
  {
    if (!markupsObject->GetVectorProperty("center", center_Node))
    {
      vtkErrorToMessageCollectionWithObjectMacro(
        this,
        this->GetUserMessages(),
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

  if (markupsObject->HasMember("size"))
  {
    double size[3] = { 0.0, 0.0, 0.0 };
    markupsObject->GetVectorProperty("size", size);
    roiNode->SetSize(size);
  }

  double orientationMatrix[9] = { 0.0 };
  if (markupsObject->HasMember("orientation"))
  {
    if (!markupsObject->GetVectorProperty("orientation", orientationMatrix, 9))
    {
      vtkErrorToMessageCollectionWithObjectMacro(
        this,
        this->GetUserMessages(),
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
    objectToNodeMatrix->SetElement(i, 0, orientationMatrix[3 * i]);
    objectToNodeMatrix->SetElement(i, 1, orientationMatrix[3 * i + 1]);
    objectToNodeMatrix->SetElement(i, 2, orientationMatrix[3 * i + 2]);
    objectToNodeMatrix->SetElement(i, 3, center_Node[i]);
  }
  roiNode->GetObjectToNodeMatrix()->DeepCopy(objectToNodeMatrix);

  if (markupsObject->HasMember("insideOut"))
  {
    roiNode->SetInsideOut(markupsObject->GetBoolProperty("insideOut"));
  }

  return Superclass::UpdateMarkupsNodeFromJsonValue(markupsNode, markupsObject);
}
