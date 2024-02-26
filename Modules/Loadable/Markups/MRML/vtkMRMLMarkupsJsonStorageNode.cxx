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

==============================================================================*/

#include <vtkCodedEntry.h>
#include "vtkMRMLMarkupsJsonElement.h"
#include "vtkMRMLMarkupsJsonStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLStaticMeasurement.h"
#include "vtkMRMLUnitNode.h"

#include "vtkMRMLI18N.h"
#include "vtkMRMLScene.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/RegularExpression.hxx>
#include <vtksys/SystemTools.hxx>

#include "itkNumberToString.h"

namespace
{
// Schema ID used to be an URL where the schema was available, but since "master" branch was renamed
// to "main" the URL does not resolve to a valid file anymore. This is not a problem, because Schema ID does
// not have to correspond to an URL where the schema is available according to the JSON standard, it is just
// recommended for compatibility with software that assumes this. The ID will likely to be changed to have
// "main" in the name in the future, but for compatibility with Slicer < 5.1 the current value is preserved for now.
// After sufficient time has passed and we are no longer concerned about forward compatibility with
// Slicer < 5.1, the branch name may be changed to "main".
const std::string MARKUPS_SCHEMA = "https://raw.githubusercontent.com/slicer/slicer/master/Modules/Loadable/Markups/"
                                   "Resources/Schema/markups-schema-v1.0.3.json#";
// regex should be lower case
const std::string ACCEPTED_MARKUPS_SCHEMA_REGEX = ".*markups-schema-v1\\.[0-9]+\\.[0-9]+\\.json#*$";
} // namespace

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode::vtkMRMLMarkupsJsonStorageNode()
{
  this->DefaultWriteFileExtension = "mrk.json";
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode::~vtkMRMLMarkupsJsonStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::Copy(vtkMRMLNode* anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLMarkupsNode");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::GetMarkupsTypesInFile(const char* filePath,
                                                          std::vector<std::string>& outputMarkupsTypes)
{
  vtkNew<vtkMRMLMarkupsJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> jsonElement =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement.GetPointer())
  {
    // error already logged
    return;
  }
  jsonElement->GetArrayItemsStringProperty("markups", "type", outputMarkupsTypes);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile(const char* filePath,
                                                                             const char* nodeName /*=nullptr*/,
                                                                             int markupIndex /*=0*/)
{
  vtkMRMLScene* scene = this->GetScene();
  if (!scene)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                     "Adding markups node from file failed: invalid scene.");
    return nullptr;
  }
  if (!filePath)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                     "Adding markups node from file failed: invalid filename.");
    return nullptr;
  }
  vtkNew<vtkMRMLMarkupsJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> jsonElement =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement.GetPointer())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                               jsonReader->GetUserMessages()->GetAllMessagesAsString());
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLMarkupsJsonElement> markups =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(jsonElement->GetArrayProperty("markups"));
  if (!markups.GetPointer())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                     "Adding markups node from file failed: no valid valid 'markups' array is found"
                                       << " in file '" << filePath << "'.");
    return nullptr;
  }
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> markup =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(markups->GetArrayItem(markupIndex));
  if (!markup.GetPointer())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                     "Failed to get markup " << markupIndex << " in file '" << filePath << "'.");
    return nullptr;
  }

  std::string markupsType = markup->GetStringProperty("type");
  if (markupsType.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                     "Adding markups node from file failed: required 'type' value is not found"
                                       << " in file '" << filePath << "'.");
    return nullptr;
  }
  std::string className = this->GetMarkupsClassNameFromMarkupsType(markupsType);

  std::string newNodeName;
  if (nodeName && strlen(nodeName) > 0)
  {
    newNodeName = nodeName;
  }
  else
  {
    newNodeName = scene->GetUniqueNameByString(this->GetFileNameWithoutExtension(filePath).c_str());
  }
  vtkMRMLMarkupsNode* markupsNode =
    vtkMRMLMarkupsNode::SafeDownCast(scene->AddNewNodeByClass(className.c_str(), newNodeName));
  if (!markupsNode)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                     "Adding markups node from file failed: cannot instantiate class '"
                                       << className << " in file '" << filePath << "'.");
    return nullptr;
  }

  bool success = true;
  success = success && this->UpdateMarkupsNodeFromJsonValue(markupsNode, markup);

  vtkMRMLMarkupsDisplayNode* displayNode = nullptr;
  if (success && markupsNode)
  {
    markupsNode->CreateDefaultDisplayNodes();
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
    vtkSmartPointer<vtkMRMLMarkupsJsonElement> markupDisplay =
      vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(markup->GetObjectProperty("display"));
    if (displayNode && markupDisplay.GetPointer())
    {
      success = success && this->UpdateMarkupsDisplayNodeFromJsonValue(displayNode, markupDisplay);
    }
  }

  if (!success)
  {
    if (displayNode)
    {
      scene->RemoveNode(displayNode);
    }
    if (markupsNode)
    {
      scene->RemoveNode(markupsNode);
    }
    return nullptr;
  }

  this->StoredTime->Modified();
  markupsNode->SetAndObserveStorageNodeID(this->GetID());
  return markupsNode;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  if (!refNode)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
                                     "Reading markups node file failed: null reference node.");
    return 0;
  }

  const char* filePath = this->GetFileName();
  if (!filePath)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
                                     "Reading markups node file failed: invalid filename.");
    return 0;
  }
  vtkNew<vtkMRMLMarkupsJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> jsonElement =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement.GetPointer())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
                                               jsonReader->GetUserMessages()->GetAllMessagesAsString());
    return 0;
  }
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> markups =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(jsonElement->GetArrayProperty("markups"));
  if (!markups.GetPointer() || markups->GetArraySize() < 1)
  {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: cannot read "
                  << refNode->GetClassName() << " markup from file " << filePath
                  << " (does not contain valid 'markups' array)");
    return 0;
  }
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> markup =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(markups->GetArrayItem(0));
  if (!markup.GetPointer())
  {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: cannot read "
                  << refNode->GetClassName() << " markup from file " << filePath
                  << " (does not contain valid 'markups' array)");
    return 0;
  }

  std::string markupsType = markup->GetStringProperty("type");
  if (markupsType.empty())
  {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: cannot read "
                  << refNode->GetClassName() << " markup from file " << filePath
                  << " (markup item does not contain 'type' property)");
    return 0;
  }

  std::string className = this->GetMarkupsClassNameFromMarkupsType(markupsType);
  if (className != refNode->GetClassName())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
                                     "Reading markups node file failed: cannot read "
                                       << refNode->GetClassName() << " markups class from file " << filePath
                                       << " (it contains " << className << ").");
    return 0;
  }

  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(refNode);
  bool success = this->UpdateMarkupsNodeFromJsonValue(markupsNode, markup);

  this->Modified();
  return success ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: file name not specified.");
    return 0;
  }
  vtkDebugMacro("WriteDataInternal: have file name " << fullName.c_str());

  // cast the input node
  vtkMRMLMarkupsNode* markupsNode = nullptr;
  if (refNode->IsA("vtkMRMLMarkupsNode"))
  {
    markupsNode = dynamic_cast<vtkMRMLMarkupsNode*>(refNode);
  }

  if (markupsNode == nullptr)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: unable to cast input node "
                                       << refNode->GetID() << " to a known markups node.");
    return 0;
  }

  vtkNew<vtkMRMLMarkupsJsonWriter> writer;
  if (!writer->WriteToFileBegin(fullName.c_str(), MARKUPS_SCHEMA.c_str()))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: unable to open file '" << fullName
                                                                                               << "' for writing.");
    return 0;
  }

  writer->WriteArrayPropertyStart("markups");
  if (!this->WriteMarkup(writer, markupsNode))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed: unable to write markups node '"
                                       << (markupsNode->GetName() ? markupsNode->GetName() : "") << "'.");
    return 0;
  }
  writer->WriteArrayPropertyEnd();

  if (!writer->WriteToFileEnd())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
                                     "Writing markups node file failed for '" << fullName << "'");
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLMarkupsJsonStorageNode", "Markups JSON")
                                                + " (.mrk.json)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLMarkupsJsonStorageNode", "Markups JSON")
                                                + " (.json)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLMarkupsJsonStorageNode", "Markups JSON")
                                                 + " (.mrk.json)");
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLMarkupsJsonStorageNode", "Markups JSON")
                                                 + " (.json)");
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonStorageNode::GetMarkupsClassNameFromMarkupsType(std::string markupsType)
{
  std::string className = std::string("vtkMRMLMarkups") + markupsType + std::string("Node");
  return className;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode,
                                                                   vtkMRMLMarkupsJsonElement* markupObject)
{
  if (!markupsNode)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                               "Markups reading failed: invalid markupsNode");
    return false;
  }

  MRMLNodeModifyBlocker blocker(markupsNode);

  // Need to disable control point lock (the actual value will be set in the end of the method)
  markupsNode->SetFixedNumberOfControlPoints(false);

  // clear out the list
  markupsNode->RemoveAllControlPoints();

  if (markupObject->HasMember("name"))
  {
    markupsNode->SetName(markupObject->GetStringProperty("name").c_str());
  }

  /// Added for backwards compatibility with storage nodes created before vtkMRMLMarkupsPlaneStorageNode or additional
  /// plane types were implemented. This check must be done here to preserve compatibility with scenes saved before
  /// vtkMRMLMarkupsPlaneJsonStorageNode was added.
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);

  if (planeNode && !markupObject->HasMember("planeType"))
  {
    // If planeType is not defined, then the json file was written before the additional plane types were implemented.
    // Previously the only plane type used was PlaneType3Points.
    planeNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);
  }

  std::string coordinateSystemStr = "LPS";
  if (markupObject->HasMember("coordinateSystem"))
  {
    coordinateSystemStr = markupObject->GetStringProperty("coordinateSystem");
  }
  if (!coordinateSystemStr.empty())
  {
    int coordinateSystemFlag = vtkMRMLMarkupsStorageNode::GetCoordinateSystemFromString(coordinateSystemStr.c_str());
    vtkDebugWithObjectMacro(this, "CoordinateSystem = " << coordinateSystemFlag);
    this->SetCoordinateSystem(coordinateSystemFlag);
  }
  int coordinateSystem = this->GetCoordinateSystem();

  std::string coordinateUnits;
  if (markupObject->HasMember("coordinateUnits"))
  {
    vtkSmartPointer<vtkCodedEntry> unitsCode =
      vtkSmartPointer<vtkCodedEntry>::Take(markupObject->GetCodedEntryProperty("coordinateUnits"));

    std::string unitsValue;
    if (unitsCode.GetPointer())
    {
      // Displayed unit is the code value of units
      if (std::string(unitsCode->GetCodingSchemeDesignator() ? unitsCode->GetCodingSchemeDesignator() : "") != "UCUM")
      {
        vtkErrorToMessageCollectionWithObjectMacro(
          this,
          this->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
          "Markups reading failed: only UCUM coding scheme is supported for coordinateUnits, got "
            << unitsCode->GetCodingSchemeDesignator() << " instead");
        return false;
      }
      unitsValue = unitsCode->GetCodeValue();
    }
    else
    {
      // If units was not specified using coded entry it may have been specified by displayed unit
      markupObject->GetStringProperty("coordinateUnits", unitsValue);
    }
  }
  if (!coordinateUnits.empty())
  {
    std::string coordinateUnitsInScene = this->GetCoordinateUnitsFromSceneAsString(markupsNode);
    if (!coordinateUnitsInScene.empty() && coordinateUnits != coordinateUnitsInScene)
    {
      // We should scale the coordinate values if there is a mismatch but for now we just refuse to load the markups
      // if units do not match.
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                                 "Markups reading failed: length unit in the scene ("
                                                   << coordinateUnitsInScene
                                                   << ") does not match coordinate system unit in the markups file ("
                                                   << coordinateUnits << ").");
      return false;
    }
  }

  if (markupObject->HasMember("locked"))
  {
    markupsNode->SetLocked(markupObject->GetBoolProperty("locked"));
  }
  if (markupObject->HasMember("labelFormat"))
  {
    markupsNode->SetControlPointLabelFormat(markupObject->GetStringProperty("labelFormat"));
  }
  if (markupObject->HasMember("lastUsedControlPointNumber"))
  {
    markupsNode->SetLastUsedControlPointNumber(markupObject->GetIntProperty("lastUsedControlPointNumber"));
  }

  vtkSmartPointer<vtkMRMLMarkupsJsonElement> controlPointItem =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(markupObject->GetArrayProperty("controlPoints"));
  if (controlPointItem.GetPointer())
  {
    if (!this->ReadControlPoints(controlPointItem, coordinateSystem, markupsNode))
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                                 "Markups reading failed: invalid controlPoints item.");
      return false;
    }
  }

  vtkSmartPointer<vtkMRMLMarkupsJsonElement> measurementsItem =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(markupObject->GetArrayProperty("measurements"));
  if (measurementsItem.GetPointer())
  {
    if (!this->ReadMeasurements(measurementsItem, markupsNode))
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsNodeFromJsonValue",
                                                 "Markups reading failed: invalid measurements item.");
      return false;
    }
  }

  // SetFixedNumberOfControlPoints() must be called after control points are already set
  if (markupObject->HasMember("fixedNumberOfControlPoints"))
  {
    markupsNode->SetFixedNumberOfControlPoints(markupObject->GetBoolProperty("fixedNumberOfControlPoints"));
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsDisplayNodeFromJsonValue(vtkMRMLMarkupsDisplayNode* displayNode,
                                                                          vtkMRMLMarkupsJsonElement* displayItem)
{
  if (!displayNode)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::UpdateMarkupsDisplayNodeFromJsonValue",
                                               "Markups reading failed: invalid invalid display node.");
    return false;
  }

  MRMLNodeModifyBlocker blocker(displayNode);

  if (displayItem->HasMember("visibility"))
  {
    displayNode->SetVisibility(displayItem->GetBoolProperty("visibility"));
  }
  if (displayItem->HasMember("opacity"))
  {
    displayNode->SetOpacity(displayItem->GetDoubleProperty("opacity"));
  }
  double color[3] = { 0.5, 0.5, 0.5 };
  if (displayItem->GetVectorProperty("color", color))
  {
    displayNode->SetColor(color);
  }
  if (displayItem->GetVectorProperty("selectedColor", color))
  {
    displayNode->SetSelectedColor(color);
  }
  if (displayItem->GetVectorProperty("activeColor", color))
  {
    displayNode->SetActiveColor(color);
  }
  if (displayItem->HasMember("propertiesLabelVisibility"))
  {
    displayNode->SetPropertiesLabelVisibility(displayItem->GetBoolProperty("propertiesLabelVisibility"));
  }
  if (displayItem->HasMember("pointLabelsVisibility"))
  {
    displayNode->SetPointLabelsVisibility(displayItem->GetBoolProperty("pointLabelsVisibility"));
  }

  if (displayItem->HasMember("textScale"))
  {
    displayNode->SetTextScale(displayItem->GetDoubleProperty("textScale"));
  }
  if (displayItem->HasMember("glyphType"))
  {
    int glyphType =
      vtkMRMLMarkupsDisplayNode::GetGlyphTypeFromString(displayItem->GetStringProperty("glyphType").c_str());
    if (glyphType >= 0)
    {
      displayNode->SetGlyphType(glyphType);
    }
  }
  if (displayItem->HasMember("glyphScale"))
  {
    displayNode->SetGlyphScale(displayItem->GetDoubleProperty("glyphScale"));
  }
  if (displayItem->HasMember("glyphSize"))
  {
    displayNode->SetGlyphSize(displayItem->GetDoubleProperty("glyphSize"));
  }
  if (displayItem->HasMember("useGlyphScale"))
  {
    displayNode->SetUseGlyphScale(displayItem->GetBoolProperty("useGlyphScale"));
  }
  if (displayItem->HasMember("sliceProjection"))
  {
    displayNode->SetSliceProjection(displayItem->GetBoolProperty("sliceProjection"));
  }
  if (displayItem->HasMember("sliceProjectionUseFiducialColor"))
  {
    displayNode->SetSliceProjectionUseFiducialColor(displayItem->GetBoolProperty("sliceProjectionUseFiducialColor"));
  }
  if (displayItem->HasMember("sliceProjectionOutlinedBehindSlicePlane"))
  {
    displayNode->SetSliceProjectionOutlinedBehindSlicePlane(
      displayItem->GetBoolProperty("sliceProjectionOutlinedBehindSlicePlane"));
  }
  if (displayItem->GetVectorProperty("sliceProjectionColor", color))
  {
    displayNode->SetSliceProjectionColor(color);
  }
  if (displayItem->HasMember("sliceProjectionOpacity"))
  {
    displayNode->SetSliceProjectionOpacity(displayItem->GetDoubleProperty("sliceProjectionOpacity"));
  }
  if (displayItem->HasMember("lineThickness"))
  {
    displayNode->SetLineThickness(displayItem->GetDoubleProperty("lineThickness"));
  }
  if (displayItem->HasMember("lineColorFadingStart"))
  {
    displayNode->SetLineColorFadingStart(displayItem->GetDoubleProperty("lineColorFadingStart"));
  }
  if (displayItem->HasMember("lineColorFadingEnd"))
  {
    displayNode->SetLineColorFadingEnd(displayItem->GetDoubleProperty("lineColorFadingEnd"));
  }
  if (displayItem->HasMember("lineColorFadingSaturation"))
  {
    displayNode->SetLineColorFadingSaturation(displayItem->GetDoubleProperty("lineColorFadingSaturation"));
  }
  if (displayItem->HasMember("lineColorFadingHueOffset"))
  {
    displayNode->SetLineColorFadingHueOffset(displayItem->GetDoubleProperty("lineColorFadingHueOffset"));
  }
  if (displayItem->HasMember("handlesInteractive"))
  {
    displayNode->SetHandlesInteractive(displayItem->GetBoolProperty("handlesInteractive"));
  }
  if (displayItem->HasMember("translationHandleVisibility"))
  {
    displayNode->SetTranslationHandleVisibility(displayItem->GetBoolProperty("translationHandleVisibility"));
  }
  if (displayItem->HasMember("rotationHandleVisibility"))
  {
    displayNode->SetRotationHandleVisibility(displayItem->GetBoolProperty("rotationHandleVisibility"));
  }
  if (displayItem->HasMember("scaleHandleVisibility"))
  {
    displayNode->SetScaleHandleVisibility(displayItem->GetBoolProperty("scaleHandleVisibility"));
  }
  if (displayItem->HasMember("interactionHandleScale"))
  {
    displayNode->SetInteractionHandleScale(displayItem->GetDoubleProperty("interactionHandleScale"));
  }
  if (displayItem->HasMember("snapMode"))
  {
    int snapMode = vtkMRMLMarkupsDisplayNode::GetSnapModeFromString(displayItem->GetStringProperty("snapMode").c_str());
    if (snapMode >= 0)
    {
      displayNode->SetSnapMode(snapMode);
    }
  }

  return true;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonStorageNode::GetCoordinateUnitsFromSceneAsString(vtkMRMLMarkupsNode* markupsNode)
{
  vtkMRMLUnitNode* unitNode = nullptr;
  if (markupsNode)
  {
    unitNode = markupsNode->GetUnitNode("length");
  }
  std::string unit = "mm";
  if (unitNode)
  {
    if (unitNode->GetSuffix())
    {
      unit = unitNode->GetSuffix();
    }
  }
  return unit;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonElement* vtkMRMLMarkupsJsonStorageNode::ReadMarkupsFile(const char* filePath)
{
  vtkNew<vtkMRMLMarkupsJsonReader> jsonReader;
  vtkSmartPointer<vtkMRMLMarkupsJsonElement> jsonElement =
    vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(jsonReader->ReadFromFile(filePath));
  if (!jsonElement.GetPointer())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                               jsonReader->GetUserMessages()->GetAllMessagesAsString());
    return nullptr;
  }

  // Verify schema
  std::string schemaString = jsonElement->GetSchema();
  if (schemaString.empty())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                               "File reading failed. File '" + std::string(filePath)
                                                 + "' does not contain schema information");
    return nullptr;
  }

  // make schema string lower case to match the regex and make comparison case insensitive
  std::transform(schemaString.begin(), schemaString.end(), schemaString.begin(), ::tolower);

  vtksys::RegularExpression filterProgressRegExp(ACCEPTED_MARKUPS_SCHEMA_REGEX);
  if (!filterProgressRegExp.find(schemaString))
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
                                               "File reading failed. File '" + std::string(filePath)
                                                 + "' is expected to contain @schema: " + MARKUPS_SCHEMA
                                                 + " (different minor and patch version numbers are accepted).");
    return nullptr;
  }

  jsonElement->Register(this);
  return jsonElement;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::ReadControlPoints(vtkMRMLMarkupsJsonElement* controlPointsArray,
                                                      int coordinateSystem,
                                                      vtkMRMLMarkupsNode* markupsNode)
{
  if (!markupsNode)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::ReadControlPoints",
                                               "File reading failed: invalid markups node");
    return false;
  }
  if (!controlPointsArray->IsArray())
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this,
      this->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::ReadControlPoints",
      "File reading failed: invalid controlPoints item (it is expected to be an array).");
    return false;
  }
  bool wasUpdatingPoints = markupsNode->IsUpdatingPoints;
  markupsNode->IsUpdatingPoints = true;
  int numberOfControlPoints = controlPointsArray->GetArraySize();
  for (int controlPointIndex = 0; controlPointIndex < numberOfControlPoints; ++controlPointIndex)
  {
    vtkSmartPointer<vtkMRMLMarkupsJsonElement> controlPointItem =
      vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(controlPointsArray->GetArrayItem(controlPointIndex));
    vtkMRMLMarkupsNode::ControlPoint* cp = new vtkMRMLMarkupsNode::ControlPoint;
    controlPointItem->GetStringProperty("id", cp->ID);
    controlPointItem->GetStringProperty("label", cp->Label);
    controlPointItem->GetStringProperty("description", cp->Description);
    controlPointItem->GetStringProperty("associatedNodeID", cp->AssociatedNodeID);

    std::string positionStatusStr;
    bool hasPositionStatus = controlPointItem->GetStringProperty("positionStatus", positionStatusStr);
    if (hasPositionStatus)
    {
      int positionStatus = vtkMRMLMarkupsNode::GetPositionStatusFromString(positionStatusStr.c_str());
      if (positionStatus < 0)
      {
        vtkErrorToMessageCollectionWithObjectMacro(this,
                                                   this->GetUserMessages(),
                                                   "vtkMRMLMarkupsJsonStorageNode::ReadControlPoints",
                                                   "File reading failed: invalid positionStatus '"
                                                     << positionStatusStr << "' for control point "
                                                     << controlPointIndex + 1 << ".");
        return false;
      }
      cp->PositionStatus = positionStatus;
    }
    else
    {
      // If positionStatus is not missing it means that the position is defined.
      cp->PositionStatus = vtkMRMLMarkupsNode::PositionDefined;
    }

    bool hasPosition = controlPointItem->GetVectorProperty("position", cp->Position);
    if (controlPointItem->HasErrors())
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsJsonStorageNode::ReadControlPoints",
                                                 "File reading failed: position must be a 3-element numeric array"
                                                   << " for control point " << controlPointIndex + 1 << ".");
      return false;
    }
    if (hasPosition)
    {
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
        cp->Position[0] = -cp->Position[0];
        cp->Position[1] = -cp->Position[1];
      }
    }
    else
    {
      if (cp->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
      {
        vtkWarningToMessageCollectionWithObjectMacro(
          this,
          this->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::ReadControlPoints",
          "File content is inconsistent: control point position is expected but not found"
            << " for control point " << controlPointIndex + 1 << ". Setting position status to undefined.");
        cp->PositionStatus = vtkMRMLMarkupsNode::PositionUndefined;
      }
    }

    bool hasOrientation = controlPointItem->GetVectorProperty("orientation", cp->OrientationMatrix, 9);
    if (controlPointItem->HasErrors())
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsJsonStorageNode::ReadControlPoints",
                                                 "File reading failed: orientation must be a 9-element numeric array"
                                                   << " for control point " << controlPointIndex + 1 << ".");
      return false;
    }
    if (hasOrientation)
    {
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
        for (int i = 0; i < 6; ++i)
        {
          cp->OrientationMatrix[i] *= -1.0;
        }
      }
    }

    if (controlPointItem->HasMember("selected"))
    {
      cp->Selected = controlPointItem->GetBoolProperty("selected");
    }
    if (controlPointItem->HasMember("locked"))
    {
      cp->Locked = controlPointItem->GetBoolProperty("locked");
    }
    if (controlPointItem->HasMember("visibility"))
    {
      cp->Visibility = controlPointItem->GetBoolProperty("visibility");
    }
    markupsNode->AddControlPoint(cp, false);
  }

  markupsNode->IsUpdatingPoints = wasUpdatingPoints;
  markupsNode->UpdateAllMeasurements();

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::ReadMeasurements(vtkMRMLMarkupsJsonElement* measurementsArray,
                                                     vtkMRMLMarkupsNode* markupsNode)
{
  if (!markupsNode)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::ReadMeasurements",
                                               "Markups measurement reading failed: invalid markups node.");
    return false;
  }
  if (!measurementsArray->IsArray())
  {
    vtkErrorToMessageCollectionWithObjectMacro(
      this,
      this->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::ReadMeasurements",
      "Markups measurement reading failed: invalid measurements item (expected it to be an array).");
    return false;
  }

  int numberOfMeasurements = measurementsArray->GetArraySize();
  for (int measurementIndex = 0; measurementIndex < numberOfMeasurements; ++measurementIndex)
  {
    vtkSmartPointer<vtkMRMLMarkupsJsonElement> measurementItem =
      vtkSmartPointer<vtkMRMLMarkupsJsonElement>::Take(measurementsArray->GetArrayItem(measurementIndex));

    std::string measurementName;
    bool hasMeasurementName = measurementItem->GetStringProperty("name", measurementName);
    if (!hasMeasurementName)
    {
      vtkErrorToMessageCollectionWithObjectMacro(this,
                                                 this->GetUserMessages(),
                                                 "vtkMRMLMarkupsJsonStorageNode::ReadMeasurements",
                                                 "Skipped measurement with missing 'name' attribute.");
      continue;
    }
    // Lookup measurements and see if an existing one needs to be updated or a new one added
    vtkSmartPointer<vtkMRMLMeasurement> measurement = markupsNode->GetMeasurement(measurementName.c_str());
    if (measurement.GetPointer() == nullptr)
    {
      measurement = vtkSmartPointer<vtkMRMLStaticMeasurement>::New();
      measurement->SetName(measurementName);
      markupsNode->AddMeasurement(measurement);
    }

    if (measurementItem->HasMember("enabled"))
    {
      measurement->SetEnabled(measurementItem->GetBoolProperty("enabled"));
    }

    vtkSmartPointer<vtkCodedEntry> unitsCode =
      vtkSmartPointer<vtkCodedEntry>::Take(measurementItem->GetCodedEntryProperty("units"));
    measurement->SetUnitsCode(unitsCode);

    std::string unitsValue;
    if (unitsCode.GetPointer())
    {
      // Displayed unit is the code value of units
      unitsValue = unitsCode->GetCodeValue();
    }
    else
    {
      // If units was not specified using coded entry it may have been specified by displayed unit
      measurementItem->GetStringProperty("units", unitsValue);
    }

    double value = 0.0;
    if (measurementItem->GetDoubleProperty("value", value))
    {
      measurement->SetDisplayValue(value, unitsValue.c_str());
    }
    else
    {
      measurement->ClearValue(vtkMRMLMeasurement::InsufficientInput);
      measurement->SetUnits(unitsValue);
    }

    std::string description;
    if (measurementItem->GetStringProperty("description", description))
    {
      measurement->SetDescription(description);
    }

    std::string printFormat;
    if (measurementItem->GetStringProperty("printFormat", printFormat))
    {
      measurement->SetPrintFormat(printFormat);
    }

    vtkSmartPointer<vtkCodedEntry> quantityCode =
      vtkSmartPointer<vtkCodedEntry>::Take(measurementItem->GetCodedEntryProperty("quantityCode"));
    measurement->SetQuantityCode(quantityCode);

    vtkSmartPointer<vtkCodedEntry> derivationCode =
      vtkSmartPointer<vtkCodedEntry>::Take(measurementItem->GetCodedEntryProperty("derivationCode"));
    measurement->SetDerivationCode(derivationCode);

    vtkSmartPointer<vtkCodedEntry> methodCode =
      vtkSmartPointer<vtkCodedEntry>::Take(measurementItem->GetCodedEntryProperty("methodCode"));
    measurement->SetMethodCode(methodCode);

    vtkSmartPointer<vtkDoubleArray> controlPointValues =
      vtkSmartPointer<vtkDoubleArray>::Take(measurementItem->GetDoubleArrayProperty("controlPointValues"));
    if (controlPointValues.GetPointer())
    {
      controlPointValues->SetName(measurementName.c_str());
    }
    measurement->SetControlPointValues(controlPointValues);
  } // For each measurement
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::WriteMarkup(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsNode* markupsNode)
{
  writer->WriteObjectStart();
  bool success = true;
  success = this->WriteBasicProperties(writer, markupsNode) && success;
  success = this->WriteControlPoints(writer, markupsNode) && success;
  success = this->WriteMeasurements(writer, markupsNode) && success;
  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
  if (displayNode)
  {
    success = this->WriteDisplayProperties(writer, displayNode) && success;
  }
  writer->WriteObjectEnd();
  return success;
}
//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::WriteBasicProperties(vtkMRMLMarkupsJsonWriter* writer,
                                                         vtkMRMLMarkupsNode* markupsNode)
{
  // Write markupsType (created from class name, stripping vtkMRMLMarkups and Node)
  std::string markupsType = markupsNode->GetClassName();
  if (vtksys::SystemTools::StringStartsWith(markupsType, "vtkMRMLMarkups"))
  {
    markupsType.erase(0, strlen("vtkMRMLMarkups"));
  }
  if (vtksys::SystemTools::StringEndsWith(markupsType, "Node"))
  {
    markupsType.erase(markupsType.size() - strlen("Node"), strlen("Node"));
  }
  if (markupsType.empty())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::WriteBasicProperties",
                                               "Writing failed: invalid class name '" << markupsType << "'");
    return false;
  }

  writer->WriteStringProperty("type", markupsType);
  writer->WriteStringProperty("coordinateSystem", this->GetCoordinateSystemAsString(this->GetCoordinateSystem()));
  writer->WriteStringProperty("coordinateUnits", this->GetCoordinateUnitsFromSceneAsString(markupsNode));
  writer->WriteBoolProperty("locked", markupsNode->GetLocked());
  writer->WriteBoolProperty("fixedNumberOfControlPoints", markupsNode->GetFixedNumberOfControlPoints());
  writer->WriteStringProperty("labelFormat", markupsNode->GetControlPointLabelFormat());
  writer->WriteIntProperty("lastUsedControlPointNumber", markupsNode->GetLastUsedControlPointNumber());

  // if (markupsNode->GetName())
  //   {
  //   writer->WriteStringProperty("name", markupsNode->GetName());
  //   }
  //

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::WriteControlPoints(vtkMRMLMarkupsJsonWriter* writer,
                                                       vtkMRMLMarkupsNode* markupsNode)
{
  int coordinateSystem = this->GetCoordinateSystem();
  if (coordinateSystem != vtkMRMLStorageNode::CoordinateSystemRAS
      && coordinateSystem != vtkMRMLStorageNode::CoordinateSystemLPS)
  {
    vtkErrorToMessageCollectionWithObjectMacro(this,
                                               this->GetUserMessages(),
                                               "vtkMRMLMarkupsJsonStorageNode::WriteControlPoints",
                                               "Writing failed: Invalid coordinate system '" << coordinateSystem
                                                                                             << "'");
    return false;
  }

  writer->WriteArrayPropertyStart("controlPoints");

  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  for (int controlPointIndex = 0; controlPointIndex < numberOfControlPoints; controlPointIndex++)
  {
    vtkMRMLMarkupsNode::ControlPoint* cp = markupsNode->GetNthControlPoint(controlPointIndex);

    writer->WriteObjectStart();

    writer->WriteStringProperty("id", cp->ID.c_str());
    writer->WriteStringProperty("label", cp->Label.c_str());
    writer->WriteStringProperty("description", cp->Description.c_str());
    writer->WriteStringProperty("associatedNodeID", cp->AssociatedNodeID.c_str());

    if (cp->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
    {
      double xyz[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNthControlPointPosition(controlPointIndex, xyz);
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
        xyz[0] = -xyz[0];
        xyz[1] = -xyz[1];
      }
      writer->WriteVectorProperty("position", xyz);
    }
    else
    {
      writer->WriteStringProperty("position", "");
    }
    double* orientationMatrix = markupsNode->GetNthControlPointOrientationMatrix(controlPointIndex);
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
      double orientationMatrixLPS[9] = { -orientationMatrix[0], -orientationMatrix[1], -orientationMatrix[2],
                                         -orientationMatrix[3], -orientationMatrix[4], -orientationMatrix[5],
                                         orientationMatrix[6],  orientationMatrix[7],  orientationMatrix[8] };
      writer->WriteVectorProperty("orientation", orientationMatrixLPS, 9);
    }
    else
    {
      writer->WriteVectorProperty("orientation", orientationMatrix, 9);
    }

    writer->WriteBoolProperty("selected", cp->Selected);
    writer->WriteBoolProperty("locked", cp->Locked);
    writer->WriteBoolProperty("visibility", cp->Visibility);
    writer->WriteStringProperty("positionStatus", vtkMRMLMarkupsNode::GetPositionStatusAsString(cp->PositionStatus));

    writer->WriteObjectEnd();
  }

  writer->WriteArrayPropertyEnd();
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::WriteMeasurements(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsNode* markupsNode)
{
  writer->WriteArrayPropertyStart("measurements");

  int numberOfMeasurements = markupsNode->GetNumberOfMeasurements();

  for (int measurementIndex = 0; measurementIndex < numberOfMeasurements; measurementIndex++)
  {
    vtkMRMLMeasurement* measurement = markupsNode->GetNthMeasurement(measurementIndex);

    writer->WriteObjectStart();

    writer->WriteStringProperty("name", measurement->GetName());
    writer->WriteBoolProperty("enabled", measurement->GetEnabled());
    if (measurement->GetValueDefined())
    {
      writer->WriteDoubleProperty("value", measurement->GetDisplayValue());
    }
    if (measurement->GetUnitsCode())
    {
      writer->WriteCodedEntryProperty("units", measurement->GetUnitsCode());
    }
    else
    {
      writer->WriteStringPropertyIfNotEmpty("units", measurement->GetUnits());
    }
    writer->WriteStringPropertyIfNotEmpty("description", measurement->GetDescription());
    writer->WriteStringPropertyIfNotEmpty("printFormat", measurement->GetPrintFormat());

    if (measurement->GetQuantityCode())
    {
      writer->WriteCodedEntryProperty("quantityCode", measurement->GetQuantityCode());
    }
    if (measurement->GetDerivationCode())
    {
      writer->WriteCodedEntryProperty("derivationCode", measurement->GetDerivationCode());
    }
    if (measurement->GetMethodCode())
    {
      writer->WriteCodedEntryProperty("methodCode", measurement->GetMethodCode());
    }

    if (measurement->GetControlPointValues())
    {
      writer->WriteDoubleArrayProperty("controlPointValues", measurement->GetControlPointValues());
    }

    writer->WriteObjectEnd();
  }

  writer->WriteArrayPropertyEnd();
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::WriteDisplayProperties(vtkMRMLMarkupsJsonWriter* writer,
                                                           vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
  {
    vtkErrorWithObjectMacro(this, "vtkMRMLMarkupsJsonStorageNode::WriteDisplayProperties failed: invalid display node");
    return false;
  }
  writer->WriteObjectPropertyStart("display");

  writer->WriteBoolProperty("visibility", markupsDisplayNode->GetVisibility());
  writer->WriteDoubleProperty("opacity", markupsDisplayNode->GetOpacity());

  writer->WriteVectorProperty("color", markupsDisplayNode->GetColor());
  writer->WriteVectorProperty("selectedColor", markupsDisplayNode->GetSelectedColor());
  writer->WriteVectorProperty("activeColor", markupsDisplayNode->GetActiveColor());

  writer->WriteBoolProperty("propertiesLabelVisibility", markupsDisplayNode->GetPropertiesLabelVisibility());
  writer->WriteBoolProperty("pointLabelsVisibility", markupsDisplayNode->GetPointLabelsVisibility());
  writer->WriteDoubleProperty("textScale", markupsDisplayNode->GetTextScale());
  writer->WriteStringProperty("glyphType",
                              markupsDisplayNode->GetGlyphTypeAsString(markupsDisplayNode->GetGlyphType()));
  writer->WriteDoubleProperty("glyphScale", markupsDisplayNode->GetGlyphScale());
  writer->WriteDoubleProperty("glyphSize", markupsDisplayNode->GetGlyphSize());
  writer->WriteBoolProperty("useGlyphScale", markupsDisplayNode->GetUseGlyphScale());

  writer->WriteBoolProperty("sliceProjection", markupsDisplayNode->GetSliceProjection());
  writer->WriteBoolProperty("sliceProjectionUseFiducialColor",
                            markupsDisplayNode->GetSliceProjectionUseFiducialColor());
  writer->WriteBoolProperty("sliceProjectionOutlinedBehindSlicePlane",
                            markupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());
  writer->WriteVectorProperty("sliceProjectionColor", markupsDisplayNode->GetSliceProjectionColor());
  writer->WriteDoubleProperty("sliceProjectionOpacity", markupsDisplayNode->GetSliceProjectionOpacity());

  writer->WriteDoubleProperty("lineThickness", markupsDisplayNode->GetLineThickness());
  writer->WriteDoubleProperty("lineColorFadingStart", markupsDisplayNode->GetLineColorFadingStart());
  writer->WriteDoubleProperty("lineColorFadingEnd", markupsDisplayNode->GetLineColorFadingEnd());
  writer->WriteDoubleProperty("lineColorFadingSaturation", markupsDisplayNode->GetLineColorFadingSaturation());
  writer->WriteDoubleProperty("lineColorFadingHueOffset", markupsDisplayNode->GetLineColorFadingHueOffset());

  writer->WriteBoolProperty("handlesInteractive", markupsDisplayNode->GetHandlesInteractive());
  writer->WriteBoolProperty("translationHandleVisibility", markupsDisplayNode->GetTranslationHandleVisibility());
  writer->WriteBoolProperty("rotationHandleVisibility", markupsDisplayNode->GetRotationHandleVisibility());
  writer->WriteBoolProperty("scaleHandleVisibility", markupsDisplayNode->GetScaleHandleVisibility());
  writer->WriteDoubleProperty("interactionHandleScale", markupsDisplayNode->GetInteractionHandleScale());

  writer->WriteStringProperty("snapMode", markupsDisplayNode->GetSnapModeAsString(markupsDisplayNode->GetSnapMode()));

  writer->WriteObjectPropertyEnd();
  return true;
}
