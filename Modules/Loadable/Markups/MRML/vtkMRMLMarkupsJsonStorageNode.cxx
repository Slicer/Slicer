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
#include "vtkMRMLMarkupsJsonStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLStaticMeasurement.h"
#include "vtkMRMLUnitNode.h"

#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/RegularExpression.hxx>
#include <vtksys/SystemTools.hxx>

#include "itkNumberToString.h"

// Relax JSON standard and allow reading/writing of nan and inf
// values. Such values should not normally occur, but if they do then
// it is easier to troubleshoot problems if numerical values are the
// same in memory and files.
// kWriteNanAndInfFlag = 2,        //!< Allow writing of Infinity, -Infinity and NaN.
#define RAPIDJSON_WRITE_DEFAULT_FLAGS 2
// kParseNanAndInfFlag = 256,      //!< Allow parsing NaN, Inf, Infinity, -Inf and -Infinity as doubles.
#define RAPIDJSON_PARSE_DEFAULT_FLAGS 256

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"


namespace
{
  // Schema ID used to be an URL where the schema was available, but since "master" branch was renamed
  // to "main" the URL does not resolve to a valid file anymore. This is not a problem, because Schema ID does
  // not have to correspond to an URL where the schema is available according to the JSON standard, it is just
  // recommended for compatibility with software that assumes this. The ID will likely to be changed to have
  // "main" in the name in the future, but for compatibility with Slicer < 5.1 the current value is preserved for now.
  // After sufficient time has passed and we are no longer concerned about forward compatibility with
  // Slicer < 5.1, the branch name may be changed to "main".
  const std::string MARKUPS_SCHEMA =
    "https://raw.githubusercontent.com/slicer/slicer/master/Modules/Loadable/Markups/Resources/Schema/markups-schema-v1.0.3.json#";
  // regex should be lower case
  const std::string ACCEPTED_MARKUPS_SCHEMA_REGEX = ".*markups-schema-v1\\.[0-9]+\\.[0-9]+\\.json#*$";
}

#include <vtkMRMLMarkupsJsonStorageNode_Private.h>

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode::vtkInternal::vtkInternal(vtkMRMLMarkupsJsonStorageNode* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode::vtkInternal::~vtkInternal()
{
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonStorageNode::vtkInternal::GetCoordinateUnitsFromSceneAsString(vtkMRMLMarkupsNode* markupsNode)
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
std::unique_ptr<rapidjson::Document> vtkMRMLMarkupsJsonStorageNode::vtkInternal::CreateJsonDocumentFromFile(const char* filePath)
{
  // Read document from file
  FILE* fp = fopen(filePath, "r");
  if (!fp)
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "Error opening the file '" << filePath << "'");
    return nullptr;
    }
  std::unique_ptr<rapidjson::Document> jsonRoot = std::unique_ptr<rapidjson::Document>(new rapidjson::Document);
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (jsonRoot->ParseStream(fs).HasParseError())
  {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "Error parsing the file'" << filePath << "'");
    fclose(fp);
    return nullptr;
  }
  fclose(fp);

  // Verify schema
  if (!jsonRoot->HasMember("@schema"))
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "File reading failed. File '" << filePath << "' does not contain schema information");
    return nullptr;
    }
  rapidjson::Value& schema = (*jsonRoot)["@schema"];

  std::string schemaString = schema.GetString();
  // make schema string lower case to match the regex and make comparison case insensitive
  std::transform(schemaString.begin(), schemaString.end(), schemaString.begin(), ::tolower);

  vtksys::RegularExpression filterProgressRegExp(ACCEPTED_MARKUPS_SCHEMA_REGEX);
  if (!filterProgressRegExp.find(schemaString))
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "File reading failed. File '" << filePath << "' is expected to contain @schema: "
      << MARKUPS_SCHEMA << " (different minor and patch version numbers are accepted).");
    return nullptr;
    }

  return jsonRoot;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadVector(rapidjson::Value& item, double* v, int numberOfComponents/*=3*/)
{
  if (!item.IsArray())
    {
    return false;
    }
  if (static_cast<int>(item.Size()) != numberOfComponents)
    {
    return false;
    }
  bool success = true;
  for (int i = 0; i < numberOfComponents; i++)
    {
    if (!item[i].IsDouble())
      {
      success = false;
      continue;
      }
    v[i] = item[i].GetDouble();
    }
  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints(rapidjson::Value& controlPointsArray, int coordinateSystem, vtkMRMLMarkupsNode* markupsNode)
{
  if (!markupsNode)
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints",
      "File reading failed: invalid markups node");
    return false;
    }
  if (!controlPointsArray.IsArray())
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints",
      "File reading failed: invalid controlPoints item (it is expected to be an array).");
    return false;
    }
  bool wasUpdatingPoints = markupsNode->IsUpdatingPoints;
  markupsNode->IsUpdatingPoints = true;
  for (rapidjson::SizeType controlPointIndex = 0; controlPointIndex < controlPointsArray.Size(); ++controlPointIndex)
    {
    rapidjson::Value& controlPointItem = controlPointsArray[controlPointIndex];
    vtkMRMLMarkupsNode::ControlPoint* cp = new vtkMRMLMarkupsNode::ControlPoint;
    if (controlPointItem.HasMember("id"))
      {
      cp->ID = controlPointItem["id"].GetString();
      }
    if (controlPointItem.HasMember("label"))
      {
      cp->Label = controlPointItem["label"].GetString();
      }
    if (controlPointItem.HasMember("description"))
      {
      cp->Description = controlPointItem["description"].GetString();
      }
    if (controlPointItem.HasMember("associatedNodeID"))
      {
      cp->AssociatedNodeID = controlPointItem["associatedNodeID"].GetString();
      }

    if (controlPointItem.HasMember("positionStatus"))
      {
      int positionStatus = vtkMRMLMarkupsNode::GetPositionStatusFromString(controlPointItem["positionStatus"].GetString());
      if (positionStatus < 0)
        {
        vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints",
          "File reading failed: invalid positionStatus '" << controlPointItem["positionStatus"].GetString()
          << "' for control point " << controlPointIndex + 1 << ".");
        return false;
        }
      cp->PositionStatus = positionStatus;
      }
    if (controlPointItem.HasMember("position"))
      {
      rapidjson::Value& positionItem = controlPointItem["position"];
      if (!this->ReadVector(positionItem, cp->Position))
        {
        // If positionStatus is not defined there is a position value
        // then it indicates that a valid position should be present,
        // therefore it is an error that the position vector is invalid.
        if (!controlPointItem.HasMember("positionStatus")
          || cp->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
          {
          vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
            "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints",
            "File reading failed: position must be a 3-element numeric array"
            << " for control point " << controlPointIndex + 1 << ".");
          return false;
          }
        }
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
        {
        cp->Position[0] = -cp->Position[0];
        cp->Position[1] = -cp->Position[1];
        }
      if (!controlPointItem.HasMember("positionStatus"))
        {
        cp->PositionStatus = vtkMRMLMarkupsNode::PositionDefined;
        }
      }
    else
      {
      if (controlPointItem.HasMember("positionStatus")
        && cp->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
        {
        vtkWarningToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints",
          "File content is inconsistent: positionStatus is set to defined but no position values are provided"
          << " for control point " << controlPointIndex + 1 << ".");
        }
      cp->PositionStatus = vtkMRMLMarkupsNode::PositionUndefined;
      }

    if (controlPointItem.HasMember("orientation"))
      {
      rapidjson::Value& orientationItem = controlPointItem["orientation"];
      if (!this->ReadVector(orientationItem, cp->OrientationMatrix, 9))
        {
        vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints",
          "File reading failed: position must be a 3-element numeric array"
          << " for control point " << controlPointIndex + 1 << ".");
        return false;
        }
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
        {
        for (int i = 0; i < 6; ++i)
          {
          cp->OrientationMatrix[i] *= -1.0;
          }
        }
      }

    if (controlPointItem.HasMember("selected"))
      {
      cp->Selected = controlPointItem["selected"].GetBool();
      }
    if (controlPointItem.HasMember("locked"))
      {
      cp->Locked = controlPointItem["locked"].GetBool();
      }
    if (controlPointItem.HasMember("visibility"))
      {
      cp->Visibility = controlPointItem["visibility"].GetBool();
      }

    markupsNode->AddControlPoint(cp, false);
    }

  markupsNode->IsUpdatingPoints = wasUpdatingPoints;
  markupsNode->UpdateAllMeasurements();

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements(rapidjson::Value& measurementsArray, vtkMRMLMarkupsNode* markupsNode)
{
  if (!markupsNode)
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
      "Markups measurement reading failed: invalid markups node.");
    return false;
    }
  if (!measurementsArray.IsArray())
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
       "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
      "Markups measurement reading failed: invalid measurements item (expected it to be an array).");
    return false;
    }

  for (rapidjson::SizeType measurementIndex = 0; measurementIndex < measurementsArray.Size(); ++measurementIndex)
    {
    rapidjson::Value& measurementItem = measurementsArray[measurementIndex];

    if (!measurementItem.HasMember("name"))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
        "Skipped measurement with missing 'name' attribute.");
      continue;
      }

    const char* measurementName = measurementItem["name"].GetString();
    // Lookup measurements and see if an existing one needs to be updated or a new one added
    vtkSmartPointer<vtkMRMLMeasurement> measurement = markupsNode->GetMeasurement(measurementName);
    if (measurement.GetPointer() == nullptr)
      {
      measurement = vtkSmartPointer<vtkMRMLStaticMeasurement>::New();
      measurement->SetName(measurementName);
      markupsNode->AddMeasurement(measurement);
      }

    if (measurementItem.HasMember("enabled"))
      {
      measurement->SetEnabled(measurementItem["enabled"].GetBool());
      }
    if (measurementItem.HasMember("units"))
      {
      rapidjson::Value& unitsItem = measurementItem["units"];
      std::string unitsValue;
      std::string unitsScheme = "UCUM";
      std::string unitsMeaning;
      vtkSmartPointer<vtkCodedEntry> unitsCode;
      if (unitsItem.IsArray() && unitsItem.Size()>0)
        {
        unitsCode = measurement->GetUnitsCode();
        if (!unitsCode)
          {
          unitsCode = vtkSmartPointer<vtkCodedEntry>::New();
          }
        unitsValue = unitsItem[0].GetString();
        if (unitsItem.Size() > 1)
          {
          unitsScheme = unitsItem[1].GetString();
          }
        if (unitsItem.Size() > 2)
          {
          unitsMeaning = unitsItem[2].GetString();
          }
        unitsCode->SetValueSchemeMeaning(unitsValue, unitsScheme, unitsMeaning);
        }
      else if (unitsItem.IsString())
        {
        unitsValue = unitsItem.GetString();
        }
      if (measurementItem.HasMember("value"))
        {
        measurement->SetDisplayValue(measurementItem["value"].GetDouble(), unitsValue.c_str());
        }
      else
        {
        measurement->ClearValue(vtkMRMLMeasurement::InsufficientInput);
        measurement->SetUnits(unitsValue);
        }
      measurement->SetUnitsCode(unitsCode);
      }
    if (measurementItem.HasMember("description"))
      {
      measurement->SetDescription(measurementItem["description"].GetString());
      }
    if (measurementItem.HasMember("printFormat"))
      {
      measurement->SetPrintFormat(measurementItem["printFormat"].GetString());
      }

    if (measurementItem.HasMember("quantityCode"))
      {
      rapidjson::Value& codedItem = measurementItem["quantityCode"];
      if (codedItem.IsArray() && codedItem.Size()>1)
        {
        if (!measurement->GetQuantityCode())
          {
          vtkNew<vtkCodedEntry> quantityCode;
          measurement->SetQuantityCode(quantityCode);
          }
        std::string codedMeaning;
        if (codedItem.Size() > 2)
          {
          codedMeaning = codedItem[2].GetString();
          }
        measurement->GetQuantityCode()->SetValueSchemeMeaning(codedItem[0].GetString(), codedItem[1].GetString(), codedMeaning);
        }
      else
        {
        measurement->SetQuantityCode(nullptr);
        }
      }
    else
      {
      measurement->SetQuantityCode(nullptr);
      }

    if (measurementItem.HasMember("derivationCode"))
      {
      rapidjson::Value& codedItem = measurementItem["derivationCode"];
      if (codedItem.IsArray() && codedItem.Size()>1)
        {
        if (!measurement->GetDerivationCode())
          {
          vtkNew<vtkCodedEntry> quantityCode;
          measurement->SetDerivationCode(quantityCode);
          }
        std::string codedMeaning;
        if (codedItem.Size() > 2)
          {
          codedMeaning = codedItem[2].GetString();
          }
        measurement->GetDerivationCode()->SetValueSchemeMeaning(codedItem[0].GetString(), codedItem[1].GetString(), codedMeaning);
        }
      else
        {
        measurement->SetDerivationCode(nullptr);
        }
      }
    else
      {
      measurement->SetDerivationCode(nullptr);
      }

    if (measurementItem.HasMember("methodCode"))
      {
      rapidjson::Value& codedItem = measurementItem["methodCode"];
      if (codedItem.IsArray() && codedItem.Size()>1)
        {
        if (!measurement->GetMethodCode())
          {
          vtkNew<vtkCodedEntry> quantityCode;
          measurement->SetMethodCode(quantityCode);
          }
        std::string codedMeaning;
        if (codedItem.Size() > 2)
          {
          codedMeaning = codedItem[2].GetString();
          }
        measurement->GetMethodCode()->SetValueSchemeMeaning(codedItem[0].GetString(), codedItem[1].GetString(), codedMeaning);
        }
      else
        {
        measurement->SetMethodCode(nullptr);
        }
      }
    else
      {
      measurement->SetMethodCode(nullptr);
      }

    if (measurementItem.HasMember("controlPointValues"))
      {
      rapidjson::Value& controlPointValuesItem = measurementItem["controlPointValues"];
      if (!controlPointValuesItem.IsArray())
        {
        vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
          "Measurement reading failed: controlPointValues must be an array in measurement " << measurementName);
        continue;
        }
      int numberOfTuples = controlPointValuesItem.GetArray().Size();
      if (numberOfTuples < 1)
        {
        // no values stored in the array
        continue;
        }
      rapidjson::Value& firstControlPointValue = controlPointValuesItem.GetArray()[0];
      vtkNew<vtkDoubleArray> controlPointValues;
      controlPointValues->SetName(measurementName);
      if (firstControlPointValue.IsDouble())
        {
        controlPointValues->SetNumberOfValues(numberOfTuples);
        double* values = controlPointValues->GetPointer(0);
        bool success = this->ReadVector(controlPointValuesItem, values, numberOfTuples);
        if (!success)
          {
          vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
            "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
            "Measurement reading failed: error while reading controlPointValues array from measurement " << measurementName);
          continue;
          }
        }
      else if (firstControlPointValue.IsArray())
        {
        int numberOfComponents = firstControlPointValue.GetArray().Size();
        controlPointValues->SetNumberOfComponents(numberOfComponents);
        controlPointValues->SetNumberOfTuples(numberOfTuples);
        double* values = controlPointValues->GetPointer(0);
        for (auto& value : controlPointValuesItem.GetArray())
          {
          (void)value; // unused
          bool success = this->ReadVector(controlPointValuesItem, values, numberOfComponents);
          if (!success)
            {
            vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
              "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
              "Measurement reading failed: error while reading controlPointValues array (all items are expected to contain the same number of components)"
              << " in measurement " << measurementName);
            continue;
            }
          values += numberOfComponents;
          }
        }
      else
        {
        vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
          "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadMeasurements",
          "Measurement reading failed: invalid controlPointValues content (must contain array of doubles or arrays) in measurement " << measurementName);
        continue;
        }
      measurement->SetControlPointValues(controlPointValues);
      }
    } // For each measurement

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupObject)
{
  if (!markupsNode)
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
      "Markups reading failed: invalid markupsNode");
    return false;
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  // Need to disable control point lock (the actual value will be set in the end of the method)
  markupsNode->SetFixedNumberOfControlPoints(false);

  // clear out the list
  markupsNode->RemoveAllControlPoints();

  /// Added for backwards compatibility with storage nodes created before vtkMRMLMarkupsPlaneStorageNode or additional plane types were implemented.
  /// This check must be done here to preserve compatibility with scenes saved before vtkMRMLMarkupsPlaneJsonStorageNode was added.
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);
  if (planeNode && !markupObject.HasMember("planeType"))
    {
    // If planeType is not defined, then the json file was written before the additional plane types were implemented.
    // Previously the only plane type used was PlaneType3Points.
    planeNode->SetPlaneType(vtkMRMLMarkupsPlaneNode::PlaneType3Points);
    }

  std::string coordinateSystemStr = "LPS";
  if (markupObject.HasMember("coordinateSystem"))
    {
    coordinateSystemStr = markupObject["coordinateSystem"].GetString();
    }
  if (!coordinateSystemStr.empty())
    {
    int coordinateSystemFlag = vtkMRMLMarkupsStorageNode::GetCoordinateSystemFromString(coordinateSystemStr.c_str());
    vtkDebugWithObjectMacro(this->External, "CoordinateSystem = " << coordinateSystemFlag);
    this->External->SetCoordinateSystem(coordinateSystemFlag);
    }
  int coordinateSystem = this->External->GetCoordinateSystem();

  std::string coordinateUnits;
  if (markupObject.HasMember("coordinateUnits"))
    {
    rapidjson::Value& unitsItem = markupObject["coordinateUnits"];
    vtkSmartPointer<vtkCodedEntry> unitsCode;
    if (unitsItem.IsArray() && unitsItem.Size()>0)
      {
      coordinateUnits = unitsItem[0].GetString();
      if (unitsItem.Size() > 1)
        {
        std::string unitsScheme = unitsItem[1].GetString();
        if (!unitsScheme.empty())
          {
          if (unitsScheme != "UCUM")
            {
            vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
              "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
              "Markups reading failed: only UCUM coding scheme is supported for coordinateUnits, got " << unitsScheme << " instead");
            return false;
            }
          }
        }
      }
    else if (unitsItem.IsString())
      {
      coordinateUnits = unitsItem.GetString();
      }
    }
  if (!coordinateUnits.empty())
    {
    std::string coordinateUnitsInScene = this->GetCoordinateUnitsFromSceneAsString(markupsNode);
    if (!coordinateUnitsInScene.empty() && coordinateUnits != coordinateUnitsInScene)
      {
      // We should scale the coordinate values if there is a mismatch but for now we just refuse to load the markups
      // if units do not match.
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "Markups reading failed: length unit in the scene (" << coordinateUnitsInScene
          << ") does not match coordinate system unit in the markups file (" << coordinateUnits << ").");
      return false;
      }
    }

  if (markupObject.HasMember("locked"))
    {
    markupsNode->SetLocked(markupObject["locked"].GetBool());
    }

  if (markupObject.HasMember("labelFormat"))
    {
    markupsNode->SetControlPointLabelFormat(markupObject["labelFormat"].GetString());
    }

  if (markupObject.HasMember("lastUsedControlPointNumber"))
    {
    markupsNode->SetLastUsedControlPointNumber(markupObject["lastUsedControlPointNumber"].GetInt());
    }

  if (markupObject.HasMember("controlPoints"))
    {
    if (!this->ReadControlPoints(markupObject["controlPoints"], coordinateSystem, markupsNode))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "Markups reading failed: invalid controlPoints item.");
      return  false;
      }
    }

  if (markupObject.HasMember("measurements"))
    {
    if (!this->ReadMeasurements(markupObject["measurements"], markupsNode))
      {
      vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
        "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue",
        "Markups reading failed: invalid measurements item.");
      return  false;
      }
    }

  // SetFixedNumberOfControlPoints() must be called after control points are already set
  if (markupObject.HasMember("fixedNumberOfControlPoints"))
    {
    markupsNode->SetFixedNumberOfControlPoints(markupObject["fixedNumberOfControlPoints"].GetBool());
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsDisplayNodeFromJsonValue(
  vtkMRMLMarkupsDisplayNode* displayNode, rapidjson::Value& markupObject)
{
  if (!displayNode)
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsDisplayNodeFromJsonValue",
      "Markups reading failed: invalid invalid display node.");
    return false;
    }

  rapidjson::Value& displayItem = markupObject["display"];
  if (!displayItem.IsObject())
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsDisplayNodeFromJsonValue",
      "Markups reading failed: display item is not found.");
    return false;
    }

  MRMLNodeModifyBlocker blocker(displayNode);

  if (displayItem.HasMember("visibility"))
    {
    displayNode->SetVisibility(displayItem["visibility"].GetBool());
    }
  if (displayItem.HasMember("opacity"))
    {
    displayNode->SetOpacity(displayItem["opacity"].GetDouble());
    }
  if (displayItem.HasMember("color"))
    {
    double color[3] = { 0.5, 0.5, 0.5 };
    this->ReadVector(displayItem["visibility"], color);
    displayNode->SetColor(color);
    }
  if (displayItem.HasMember("selectedColor"))
    {
    double color[3] = { 0.5, 0.5, 0.5 };
    if (this->ReadVector(displayItem["selectedColor"], color))
      {
      displayNode->SetSelectedColor(color);
      }
    }
  if (displayItem.HasMember("activeColor"))
    {
    double color[3] = { 0.5, 0.5, 0.5 };
    if (this->ReadVector(displayItem["activeColor"], color))
      {
      displayNode->SetActiveColor(color);
      }
    }
  if (displayItem.HasMember("propertiesLabelVisibility"))
    {
    displayNode->SetPropertiesLabelVisibility(displayItem["propertiesLabelVisibility"].GetBool());
    }
  if (displayItem.HasMember("pointLabelsVisibility"))
    {
    displayNode->SetPointLabelsVisibility(displayItem["pointLabelsVisibility"].GetBool());
    }

  if (displayItem.HasMember("textScale"))
    {
    displayNode->SetTextScale(displayItem["textScale"].GetDouble());
    }
  if (displayItem.HasMember("glyphType"))
    {
    int glyphType = vtkMRMLMarkupsDisplayNode::GetGlyphTypeFromString(displayItem["glyphType"].GetString());
    if (glyphType >= 0)
      {
      displayNode->SetGlyphType(glyphType);
      }
    }
  if (displayItem.HasMember("glyphScale"))
    {
    displayNode->SetGlyphScale(displayItem["glyphScale"].GetDouble());
    }
  if (displayItem.HasMember("glyphSize"))
    {
    displayNode->SetGlyphSize(displayItem["glyphSize"].GetDouble());
    }
  if (displayItem.HasMember("useGlyphScale"))
    {
    displayNode->SetUseGlyphScale(displayItem["useGlyphScale"].GetBool());
    }
  if (displayItem.HasMember("sliceProjection"))
    {
    displayNode->SetSliceProjection(displayItem["sliceProjection"].GetBool());
    }
  if (displayItem.HasMember("sliceProjectionUseFiducialColor"))
    {
    displayNode->SetSliceProjectionUseFiducialColor(displayItem["sliceProjectionUseFiducialColor"].GetBool());
    }
  if (displayItem.HasMember("sliceProjectionOutlinedBehindSlicePlane"))
    {
    displayNode->SetSliceProjectionOutlinedBehindSlicePlane(displayItem["sliceProjectionOutlinedBehindSlicePlane"].GetBool());
    }
  if (displayItem.HasMember("sliceProjectionColor"))
    {
    double color[3] = { 0.5, 0.5, 0.5 };
    if (this->ReadVector(displayItem["sliceProjectionColor"], color))
      {
      displayNode->SetSliceProjectionColor(color);
      }
    }
  if (displayItem.HasMember("sliceProjectionOpacity"))
    {
    displayNode->SetSliceProjectionOpacity(displayItem["sliceProjectionOpacity"].GetDouble());
    }
  if (displayItem.HasMember("lineThickness"))
    {
    displayNode->SetLineThickness(displayItem["lineThickness"].GetDouble());
    }
  if (displayItem.HasMember("lineColorFadingStart"))
    {
    displayNode->SetLineColorFadingStart(displayItem["lineColorFadingStart"].GetDouble());
    }
  if (displayItem.HasMember("lineColorFadingEnd"))
    {
    displayNode->SetLineColorFadingEnd(displayItem["lineColorFadingEnd"].GetDouble());
    }
  if (displayItem.HasMember("lineColorFadingSaturation"))
    {
    displayNode->SetLineColorFadingSaturation(displayItem["lineColorFadingSaturation"].GetDouble());
    }
  if (displayItem.HasMember("lineColorFadingHueOffset"))
    {
    displayNode->SetLineColorFadingHueOffset(displayItem["lineColorFadingHueOffset"].GetDouble());
    }
  if (displayItem.HasMember("handlesInteractive"))
    {
    displayNode->SetHandlesInteractive(displayItem["handlesInteractive"].GetBool());
    }
  if (displayItem.HasMember("translationHandleVisibility"))
    {
    displayNode->SetTranslationHandleVisibility(displayItem["translationHandleVisibility"].GetBool());
    }
  if (displayItem.HasMember("rotationHandleVisibility"))
    {
    displayNode->SetRotationHandleVisibility(displayItem["rotationHandleVisibility"].GetBool());
    }
  if (displayItem.HasMember("scaleHandleVisibility"))
    {
    displayNode->SetScaleHandleVisibility(displayItem["scaleHandleVisibility"].GetBool());
    }
  if (displayItem.HasMember("interactionHandleScale"))
    {
    displayNode->SetInteractionHandleScale(displayItem["interactionHandleScale"].GetDouble());
    }
  if (displayItem.HasMember("snapMode"))
    {
    int snapMode = vtkMRMLMarkupsDisplayNode::GetSnapModeFromString(displayItem["snapMode"].GetString());
    if (snapMode >= 0)
      {
      displayNode->SetSnapMode(snapMode);
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteMarkup(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsNode* markupsNode)
{
  bool success = true;
  success = success && this->WriteBasicProperties(writer, markupsNode);
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
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteBasicProperties(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsNode* markupsNode)
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
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteBasicProperties",
      "Writing failed: invalid class name '" << markupsType << "'");
    return false;
    }
  writer.Key("type"); writer.String(markupsType.c_str());

  writer.Key("coordinateSystem");
  writer.String(this->External->GetCoordinateSystemAsString(this->External->GetCoordinateSystem()));

  writer.Key("coordinateUnits");
  writer.String(this->GetCoordinateUnitsFromSceneAsString(markupsNode).c_str());

  writer.Key("locked");
  writer.Bool(markupsNode->GetLocked());

  writer.Key("fixedNumberOfControlPoints");
  writer.Bool(markupsNode->GetFixedNumberOfControlPoints());

  writer.Key("labelFormat");
  writer.String(markupsNode->GetControlPointLabelFormat().c_str());

  writer.Key("lastUsedControlPointNumber");
  writer.Int(markupsNode->GetLastUsedControlPointNumber());

  /*
  if (markupsNode->GetName())
    {
    writer.Key("name"); writer.String(markupsNode->GetName());
    }
    */

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteControlPoints(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsNode* markupsNode)
{
  int coordinateSystem = this->External->GetCoordinateSystem();
  if (coordinateSystem != vtkMRMLStorageNode::CoordinateSystemRAS
    && coordinateSystem != vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    vtkErrorToMessageCollectionWithObjectMacro(this->External, this->External->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteControlPoints",
      "Writing failed: Invalid coordinate system '" << coordinateSystem << "'");
    return false;
    }

  writer.Key("controlPoints");
  writer.StartArray();

  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  for (int controlPointIndex = 0; controlPointIndex < numberOfControlPoints; controlPointIndex++)
    {
    vtkMRMLMarkupsNode::ControlPoint* cp = markupsNode->GetNthControlPoint(controlPointIndex);

    writer.StartObject();

    writer.Key("id"); writer.String(cp->ID.c_str());
    writer.Key("label"); writer.String(cp->Label.c_str());
    writer.Key("description"); writer.String(cp->Description.c_str());
    writer.Key("associatedNodeID"); writer.String(cp->AssociatedNodeID.c_str());

    if(cp->PositionStatus == vtkMRMLMarkupsNode::PositionDefined)
      {
      double xyz[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNthControlPointPosition(controlPointIndex, xyz);
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
        {
        xyz[0] = -xyz[0];
        xyz[1] = -xyz[1];
        }
      writer.Key("position"); this->WriteVector(writer, xyz);
      }
    else
      {
      writer.Key("position"); writer.String("");
      }
    double* orientationMatrix = markupsNode->GetNthControlPointOrientationMatrix(controlPointIndex);
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      double orientationMatrixLPS[9] = {
        -orientationMatrix[0], -orientationMatrix[1], -orientationMatrix[2],
        -orientationMatrix[3], -orientationMatrix[4], -orientationMatrix[5],
         orientationMatrix[6],  orientationMatrix[7],  orientationMatrix[8]
        };
      writer.Key("orientation"); this->WriteVector(writer, orientationMatrixLPS, 9);
      }
    else
      {
      writer.Key("orientation"); this->WriteVector(writer, orientationMatrix, 9);
      }

    writer.Key("selected"); writer.Bool(cp->Selected);
    writer.Key("locked"); writer.Bool(cp->Locked);
    writer.Key("visibility"); writer.Bool(cp->Visibility);
    writer.Key("positionStatus"); writer.String(vtkMRMLMarkupsNode::GetPositionStatusAsString(cp->PositionStatus));

    writer.EndObject();
    }

  writer.EndArray();

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteMeasurements(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsNode* markupsNode)
{
  writer.Key("measurements");
  writer.StartArray();

  int numberOfMeasurements = markupsNode->GetNumberOfMeasurements();

  for (int measurementIndex = 0; measurementIndex < numberOfMeasurements; measurementIndex++)
    {
    vtkMRMLMeasurement* measurement = markupsNode->GetNthMeasurement(measurementIndex);

    writer.StartObject();

    writer.Key("name"); writer.String(measurement->GetName().c_str());
    writer.Key("enabled"); writer.Bool(measurement->GetEnabled());
    if (measurement->GetValueDefined())
      {
      writer.Key("value"); writer.Double(measurement->GetDisplayValue());
      }
    if (measurement->GetUnitsCode())
      {
      writer.Key("units"); writer.String(measurement->GetUnitsCode()->GetAsString().c_str());
      }
    else if (!measurement->GetUnits().empty())
      {
      writer.Key("units"); writer.String(measurement->GetUnits().c_str());
      }
    if (!measurement->GetDescription().empty())
      {
      writer.Key("description"); writer.String(measurement->GetDescription().c_str());
      }
    if (!measurement->GetPrintFormat().empty())
      {
      writer.Key("printFormat"); writer.String(measurement->GetPrintFormat().c_str());
      }

    if (measurement->GetQuantityCode())
      {
      writer.Key("quantityCode"); writer.String(measurement->GetQuantityCode()->GetAsString().c_str());
      }
    if (measurement->GetDerivationCode())
      {
      writer.Key("derivationCode"); writer.String(measurement->GetDerivationCode()->GetAsString().c_str());
      }
    if (measurement->GetMethodCode())
      {
      writer.Key("methodCode"); writer.String(measurement->GetMethodCode()->GetAsString().c_str());
      }

    if (measurement->GetControlPointValues())
      {
      writer.Key("controlPointValues");
      vtkDoubleArray* controlPointValues = measurement->GetControlPointValues();
      int numberOfComponents = controlPointValues->GetNumberOfComponents();
      int numberOfTuples = controlPointValues->GetNumberOfTuples();
      if (numberOfComponents == 1)
        {
        // write single-component array as single array
        double* values = controlPointValues->GetPointer(0);
        // WriteVector() method would write all values in a single line, so we do not use it here
        writer.StartArray();
        for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
          {
          writer.Double(values[tupleIndex]);
          }
        writer.EndArray();
        }
      else
        {
        // write multi-component array as an array of arrays
        writer.StartArray();
        for (int tupleIndex=0; tupleIndex<numberOfTuples; ++tupleIndex)
          {
          double* tuple = controlPointValues->GetTuple(tupleIndex);
          this->WriteVector(writer, tuple, numberOfComponents);
          }
        writer.EndArray();
        }
      }

    writer.EndObject();
    }

  writer.EndArray();

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteDisplayProperties(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteDisplayProperties failed: invalid display node");
    return false;
    }
  writer.Key("display");
  writer.StartObject();

  writer.Key("visibility"); writer.Bool(markupsDisplayNode->GetVisibility());
  writer.Key("opacity"); writer.Double(markupsDisplayNode->GetOpacity());

  writer.Key("color"); this->WriteVector(writer, markupsDisplayNode->GetColor());
  writer.Key("selectedColor"); this->WriteVector(writer, markupsDisplayNode->GetSelectedColor());
  writer.Key("activeColor"); this->WriteVector(writer, markupsDisplayNode->GetActiveColor());

  writer.Key("propertiesLabelVisibility"); writer.Bool(markupsDisplayNode->GetPropertiesLabelVisibility());
  writer.Key("pointLabelsVisibility"); writer.Bool(markupsDisplayNode->GetPointLabelsVisibility());
  writer.Key("textScale"); writer.Double(markupsDisplayNode->GetTextScale());
  writer.Key("glyphType"); writer.String(markupsDisplayNode->GetGlyphTypeAsString(markupsDisplayNode->GetGlyphType()));
  writer.Key("glyphScale"); writer.Double(markupsDisplayNode->GetGlyphScale());
  writer.Key("glyphSize"); writer.Double(markupsDisplayNode->GetGlyphSize());
  writer.Key("useGlyphScale"); writer.Bool(markupsDisplayNode->GetUseGlyphScale());

  writer.Key("sliceProjection"); writer.Bool(markupsDisplayNode->GetSliceProjection());
  writer.Key("sliceProjectionUseFiducialColor"); writer.Bool(markupsDisplayNode->GetSliceProjectionUseFiducialColor());
  writer.Key("sliceProjectionOutlinedBehindSlicePlane"); writer.Bool(markupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());
  writer.Key("sliceProjectionColor"); this->WriteVector(writer, markupsDisplayNode->GetSliceProjectionColor());
  writer.Key("sliceProjectionOpacity"); writer.Double(markupsDisplayNode->GetSliceProjectionOpacity());

  writer.Key("lineThickness"); writer.Double(markupsDisplayNode->GetLineThickness());
  writer.Key("lineColorFadingStart"); writer.Double(markupsDisplayNode->GetLineColorFadingStart());
  writer.Key("lineColorFadingEnd"); writer.Double(markupsDisplayNode->GetLineColorFadingEnd());
  writer.Key("lineColorFadingSaturation"); writer.Double(markupsDisplayNode->GetLineColorFadingSaturation());
  writer.Key("lineColorFadingHueOffset"); writer.Double(markupsDisplayNode->GetLineColorFadingHueOffset());

  writer.Key("handlesInteractive"); writer.Bool(markupsDisplayNode->GetHandlesInteractive());
  writer.Key("translationHandleVisibility"); writer.Bool(markupsDisplayNode->GetTranslationHandleVisibility());
  writer.Key("rotationHandleVisibility"); writer.Bool(markupsDisplayNode->GetRotationHandleVisibility());
  writer.Key("scaleHandleVisibility"); writer.Bool(markupsDisplayNode->GetScaleHandleVisibility());
  writer.Key("interactionHandleScale"); writer.Double(markupsDisplayNode->GetInteractionHandleScale());

  writer.Key("snapMode"); writer.String(markupsDisplayNode->GetSnapModeAsString(markupsDisplayNode->GetSnapMode()));

  writer.EndObject();
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteVector(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, double* v, int numberOfComponents/*=3*/)
{
  writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
  writer.StartArray();
  for (int i = 0; i < numberOfComponents; i++)
    {
    writer.Double(v[i]);
    }
  writer.EndArray();
  writer.SetFormatOptions(rapidjson::kFormatDefault);
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonStorageNode::vtkInternal::GetMarkupsClassNameFromMarkupsType(std::string markupsType)
{
  std::string className = std::string("vtkMRMLMarkups") + markupsType + std::string("Node");
  return className;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsJsonStorageNode::vtkInternal::GetMarkupsClassNameFromJsonValue(rapidjson::Value& markupObject)
{
  std::string markupsType = markupObject["type"].GetString();
  std::string className = this->GetMarkupsClassNameFromMarkupsType(markupsType);
  return className;
}

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsJsonStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode::vtkMRMLMarkupsJsonStorageNode()
{
  this->Internal = new vtkInternal(this);
  this->DefaultWriteFileExtension = "mrk.json";
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsJsonStorageNode::~vtkMRMLMarkupsJsonStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsNode");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::GetMarkupsTypesInFile(const char* filePath, std::vector<std::string>& outputMarkupsTypes)
{
  std::unique_ptr<rapidjson::Document> jsonRoot = this->Internal->CreateJsonDocumentFromFile(filePath);
  if (!jsonRoot)
    {
    // error is already logged
    return;
    }
  rapidjson::Value& markups = (*jsonRoot)["markups"];
  if (markups.IsArray())
    {
    int numberOfMarkups = markups.GetArray().Size();
    for (int markupIndex = 0; markupIndex < numberOfMarkups; ++markupIndex)
      {
      rapidjson::Value& markup = markups.GetArray()[markupIndex];
      std::string markupsType = markup["type"].GetString();
      outputMarkupsTypes.push_back(markupsType);
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile(const char* filePath, const char* nodeName/*=nullptr*/, int markupIndex/*=0*/)
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
  std::unique_ptr<rapidjson::Document> jsonRoot = this->Internal->CreateJsonDocumentFromFile(filePath);
  if (!jsonRoot)
    {
    // error is already logged
    return nullptr;
    }

  rapidjson::Value& markups = (*jsonRoot)["markups"];
  if (!markups.IsArray())
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
      "Adding markups node from file failed: no valid valid 'markups' array is found"
      << " in file '" << filePath << "'.");
    return nullptr;
    }
  if (markupIndex >= static_cast<int>(markups.GetArray().Size()))
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
      "Adding markups node from file failed: requested markup index" << markupIndex << " not found"
      << " (number of available markups: " << markups.GetArray().Size()
      << " in file '" << filePath << "'.");
    return nullptr;
    }

  rapidjson::Value& markup = markups.GetArray()[markupIndex];
  std::string className = this->Internal->GetMarkupsClassNameFromJsonValue(markup);
  if (!markup.HasMember("type"))
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
      "Adding markups node from file failed: required 'type' value is not found"
      << " in file '" << filePath << "'.");
    return nullptr;
    }
  std::string markupsType = markup["type"].GetString();

  std::string newNodeName;
  if (nodeName && strlen(nodeName)>0)
    {
    newNodeName = nodeName;
    }
  else
    {
    newNodeName = scene->GetUniqueNameByString(this->GetFileNameWithoutExtension(filePath).c_str());
    }
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(scene->AddNewNodeByClass(className.c_str(), newNodeName));
  if (!markupsNode)
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
      "vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile",
      "Adding markups node from file failed: cannot instantiate class '" << className
      << " in file '" << filePath << "'.");
    return nullptr;
    }

  bool success = true;
  success = success && this->Internal->UpdateMarkupsNodeFromJsonValue(markupsNode, markup);

  vtkMRMLMarkupsDisplayNode* displayNode = nullptr;
  if (success && markupsNode)
    {
    markupsNode->CreateDefaultDisplayNodes();
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
    if (displayNode && markup.HasMember("display"))
      {
      success = success && this->Internal->UpdateMarkupsDisplayNodeFromJsonValue(displayNode, markup);
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
int vtkMRMLMarkupsJsonStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "Reading markups node file failed: null reference node.");
    return 0;
    }

  const char* filePath = this->GetFileName();
  if (!filePath)
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "Reading markups node file failed: invalid filename.");
    return 0;
    }
  std::unique_ptr<rapidjson::Document> jsonRoot = this->Internal->CreateJsonDocumentFromFile(filePath);
  if (!jsonRoot)
    {
    // error is already logged
    return 0;
    }


  rapidjson::Value& markups = (*jsonRoot)["markups"];
  if (!markups.IsArray() || markups.GetArray().Size() < 1)
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: cannot read " << refNode->GetClassName()
      << " markup from file " << filePath << " (does not contain valid 'markups' array)");
    return 0;
    }

  rapidjson::Value& markup = markups.GetArray()[0];
  std::string className = this->Internal->GetMarkupsClassNameFromJsonValue(markup);
  if (className != refNode->GetClassName())
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal",
      "Reading markups node file failed: cannot read " << refNode->GetClassName()
      << " markups class from file " << filePath << " (it contains " << className << ").");
    return 0;
    }

  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(refNode);
  bool success = this->Internal->UpdateMarkupsNodeFromJsonValue(markupsNode, markup);

  this->Modified();
  return success ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
      "Writing markups node file failed: file name not specified.");
    return 0;
    }
  vtkDebugMacro("WriteDataInternal: have file name " << fullName.c_str());

  // cast the input node
  vtkMRMLMarkupsNode *markupsNode = nullptr;
  if (refNode->IsA("vtkMRMLMarkupsNode"))
    {
    markupsNode = dynamic_cast <vtkMRMLMarkupsNode *> (refNode);
    }

  if (markupsNode == nullptr)
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
      "Writing markups node file failed: unable to cast input node " << refNode->GetID() << " to a known markups node.");
    return 0;
    }

  // open the file for writing
  FILE* fp = fopen(fullName.c_str(), "wb");
  if (!fp)
    {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLMarkupsJsonStorageNode::WriteDataInternal",
      "Writing markups node file failed: unable to open file '" << fullName.c_str() << "' for writing.");
    return 0;
    }

  // Prepare JSON writer and output stream.
  char writeBuffer[65536];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);

  writer.StartObject();
  writer.Key("@schema"); writer.String(MARKUPS_SCHEMA.c_str());

  writer.Key("markups");
  writer.StartArray();

  writer.StartObject();
  bool success = this->Internal->WriteMarkup(writer, markupsNode);
  writer.EndObject();

  writer.EndArray();

  writer.EndObject();
  fclose(fp);
  return (success ? 1 : 0);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Markups JSON (.mrk.json)");
  this->SupportedReadFileTypes->InsertNextValue("Markups JSON (.json)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Markups JSON (.mrk.json)");
  this->SupportedWriteFileTypes->InsertNextValue("Markups JSON (.json)");
}
