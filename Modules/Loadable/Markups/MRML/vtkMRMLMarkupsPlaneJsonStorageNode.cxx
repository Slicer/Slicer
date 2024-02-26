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
#include "vtkMRMLMarkupsJsonElement.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsPlaneJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneJsonStorageNode::vtkMRMLMarkupsPlaneJsonStorageNode() {}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneJsonStorageNode::~vtkMRMLMarkupsPlaneJsonStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLMarkupsPlaneNode");
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::WriteBasicProperties(vtkMRMLMarkupsJsonWriter* writer,
                                                              vtkMRMLMarkupsNode* markupsNode)
{
  if (!vtkMRMLMarkupsJsonStorageNode::WriteBasicProperties(writer, markupsNode))
  {
    return false;
  }

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);
  if (!planeNode)
  {
    vtkErrorWithObjectMacro(this,
                            "vtkMRMLMarkupsPlaneJsonStorageNode::WriteBasicProperties failed: invalid markupsNode");
    return false;
  }

  writer->WriteStringProperty("planeType", planeNode->GetPlaneTypeAsString(planeNode->GetPlaneType()));

  writer->WriteStringProperty("sizeMode", planeNode->GetSizeModeAsString(planeNode->GetSizeMode()));

  writer->WriteDoubleProperty("autoScalingFactor", planeNode->GetAutoSizeScalingFactor());

  int coordinateSystem = this->GetCoordinateSystem();
  double center_Node[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetOrigin(center_Node);
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
  {
    center_Node[0] = -center_Node[0];
    center_Node[1] = -center_Node[1];
  }
  writer->WriteVectorProperty("center", center_Node);

  double normal_Node[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetNormal(normal_Node);
  if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
  {
    normal_Node[0] = -normal_Node[0];
    normal_Node[1] = -normal_Node[1];
  }
  writer->WriteVectorProperty("normal", normal_Node);

  double* objectToBase = planeNode->GetObjectToBaseMatrix()->GetData();
  writer->WriteMatrix4x4Property(
    "objectToBase", objectToBase, coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS);

  double* baseToNode = planeNode->GetBaseToNodeMatrix()->GetData();
  writer->WriteMatrix4x4Property("baseToNode", baseToNode, coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS);

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
  writer->WriteVectorProperty("orientation", orientation, 9);

  double size[3] = { 0.0, 0.0, 0.0 };
  planeNode->GetSize(size);
  writer->WriteVectorProperty("size", size);

  double planeBounds[4] = { 0.0, 0.0, 0.0, 0.0 };
  planeNode->GetPlaneBounds(planeBounds);
  writer->WriteVectorProperty("planeBounds", planeBounds, 4);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsPlaneJsonStorageNode::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode,
                                                                        vtkMRMLMarkupsJsonElement* markupObject)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);
  if (!planeNode)
  {
    vtkErrorWithObjectMacro(
      this, "vtkMRMLMarkupsPlaneJsonStorageNode::UpdateMarkupsNodeFromJsonDocument failed: invalid markupsNode");
    return false;
  }

  MRMLNodeModifyBlocker blocker(planeNode);

  if (markupObject->HasMember("planeType"))
  {
    planeNode->SetPlaneType(planeNode->GetPlaneTypeFromString(markupObject->GetStringProperty("planeType").c_str()));
  }
  else
  {
    /// Plane was created when the only type of plane was defined with 3 points.
    planeNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);
  }

  if (!vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue(markupsNode, markupObject))
  {
    return false;
  }

  int coordinateSystem = this->GetCoordinateSystem();

  if (markupObject->HasMember("objectToBase"))
  {
    double objectToBase[16] = { 0.0 };
    if (!markupObject->GetVectorProperty("objectToBase", objectToBase, 16))
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsPlaneJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
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

  if (markupObject->HasMember("baseToNode"))
  {
    double baseToNode[16] = { 0.0 };
    if (!markupObject->GetMatrix4x4Property(
          "baseToNode", baseToNode, coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS))
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsPlaneJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                                 "File reading failed: baseToNode 16-element numeric array.");
      return false;
    }
    planeNode->GetBaseToNodeMatrix()->DeepCopy(baseToNode);
  }

  if (markupObject->HasMember("sizeMode"))
  {
    planeNode->SetSizeMode(planeNode->GetSizeModeFromString(markupObject->GetStringProperty("sizeMode").c_str()));
  }

  if (markupObject->HasMember("autoScalingFactor"))
  {
    planeNode->SetAutoSizeScalingFactor(markupObject->GetDoubleProperty("autoScalingFactor"));
  }

  if (markupObject->HasMember("size"))
  {
    double size[3] = { 0.0, 0.0, 0.0 };
    if (!markupObject->GetVectorProperty("size", size))
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsPlaneJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                                 "File reading failed: size 3-element numeric array.");
      return false;
    }
    planeNode->SetSize(size);
  }

  if (markupObject->HasMember("planeBounds"))
  {
    double planeBounds[4] = { 0.0, 0.0, 0.0, 0.0 };
    if (!markupObject->GetVectorProperty("planeBounds", planeBounds, 4))
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsPlaneJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                                 "File reading failed: planeBounds 4-element numeric array.");
      return false;
    }
    planeNode->SetPlaneBounds(planeBounds);
  }

  return true;
}
