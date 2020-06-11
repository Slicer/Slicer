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

#include "vtkMRMLMarkupsJsonStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
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
  const std::string MARKUPS_SCHEMA =
    "https://raw.githubusercontent.com/slicer/slicer/master/Modules/Loadable/Markups/Resources/Schema/markups-schema-v1.0.0.json#";
}

//---------------------------------------------------------------------------
class vtkMRMLMarkupsJsonStorageNode::vtkInternal
{
public:
  vtkInternal(vtkMRMLMarkupsJsonStorageNode* external);
  ~vtkInternal();

  // Reader
  rapidjson::Document* CreateJsonDocumentFromFile(const char* filePath);
  std::string GetMarkupsClassNameFromMarkupsType(std::string markupsType);
  std::string GetMarkupsClassNameFromJsonValue(rapidjson::Value& markupObject);
  bool UpdateMarkupsNodeFromJsonValue (vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupObject);
  bool UpdateMarkupsDisplayNodeFromJsonValue(vtkMRMLMarkupsDisplayNode* displayNode, rapidjson::Value& markupObject);
  bool ReadVector(rapidjson::Value& item, double* v, int numberOfComponents=3);
  bool ReadControlPoints(rapidjson::Value& item, int coordinateSystem, vtkMRMLMarkupsNode* markupsNode);


  // Writer
  bool WriteBasicProperties(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsNode* markupsNode);
  bool WriteControlPoints(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsNode* markupsNode);
  bool WriteDisplayProperties(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsDisplayNode* markupsDisplayNode);
  void WriteVector(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, double* v, int numberOfComponents = 3);

private:
  vtkMRMLMarkupsJsonStorageNode* External;
};

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
rapidjson::Document* vtkMRMLMarkupsJsonStorageNode::vtkInternal::CreateJsonDocumentFromFile(const char* filePath)
{
  // Read document from file
  FILE* fp = fopen(filePath, "r");
  if (!fp)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal failed: error opening the file '" << filePath);
    return nullptr;
    }
  rapidjson::Document* jsonRoot = new rapidjson::Document;
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (jsonRoot->ParseStream(fs).HasParseError())
  {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal failed: error parsing the file  '" << filePath);
    delete jsonRoot;
    fclose(fp);
    return nullptr;
  }
  fclose(fp);

  // Verify schema
  if (!(*jsonRoot).HasMember("@schema"))
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal failed: file "
      << filePath << " does not contain schema information");
    delete jsonRoot;
    return nullptr;
    }
  rapidjson::Value& schema = (*jsonRoot)["@schema"];
  if (schema.GetString() != MARKUPS_SCHEMA)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::ReadDataInternal failed: file "
      << filePath << " is expected to contain @schema: " + MARKUPS_SCHEMA);
    delete jsonRoot;
    return nullptr;
    }

  return jsonRoot;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadVector(rapidjson::Value& item, double v[3], int numberOfComponents/*=3*/)
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
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::ReadControlPoints failed: invalid markupsNode");
    return false;
    }
  if (!controlPointsArray.IsArray())
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonDocument failed:"
      << " invalid controlPoints item");
    return false;
    }

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
        vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonDocument failed: invalid positionStatus");
        return false;
        }
      cp->PositionStatus = positionStatus;
      }
    if (controlPointItem.HasMember("position"))
      {
      rapidjson::Value& positionItem = controlPointItem["position"];
      if (!this->ReadVector(positionItem, cp->Position))
        {
        vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonDocument failed:"
          << " position must be a 3-element numeric array");
        return false;
        }
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
        {
        cp->Position[0] = -cp->Position[0];
        cp->Position[1] = -cp->Position[1];
        }
      }
    else
      {
      cp->PositionStatus = vtkMRMLMarkupsNode::PositionUndefined;
      }

    if (controlPointItem.HasMember("orientation"))
      {
      rapidjson::Value& orientationItem = controlPointItem["orientation"];
      if (!this->ReadVector(orientationItem, cp->OrientationMatrix, 9))
        {
        vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonDocument failed: "
          << " position must be a 3-element numeric array");
        return false;
        }
      if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
        {
        for (int i = 0; i < 6; i++)
          {
          cp->OrientationMatrix[i] = -cp->OrientationMatrix[i];
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
  return true;
}
//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupObject)
{
  if (!markupsNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonDocument failed: invalid markupsNode");
    return false;
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  // clear out the list
  markupsNode->RemoveAllControlPoints();

  std::string coordinateSystemStr = markupObject["coordinateSystem"].GetString();
  if (!coordinateSystemStr.empty())
    {
    int coordinateSystemFlag = vtkMRMLMarkupsStorageNode::GetCoordinateSystemFromString(coordinateSystemStr.c_str());
    vtkDebugWithObjectMacro(this->External, "CoordinateSystem = " << coordinateSystemFlag);
    this->External->SetCoordinateSystem(coordinateSystemFlag);
    }
  int coordinateSystem = this->External->GetCoordinateSystem();

  if (markupObject.HasMember("locked"))
    {
    markupsNode->SetLocked(markupObject["locked"].GetBool());
    }
  if (markupObject.HasMember("labelFormat"))
    {
    markupsNode->SetMarkupLabelFormat(markupObject["labelFormat"].GetString());
    }

  if (markupObject.HasMember("controlPoints"))
    {
    if (!this->ReadControlPoints(markupObject["controlPoints"], coordinateSystem, markupsNode))
      {
      vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsNodeFromJsonDocument failed:"
        << " invalid controlPoints item");
      return  false;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsDisplayNodeFromJsonValue(
  vtkMRMLMarkupsDisplayNode* displayNode, rapidjson::Value& markupObject)
{
  if (!displayNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsDisplayNodeFromJsonDocument failed:"
      << " invalid displayNode");
    return false;
    }

  rapidjson::Value& displayItem = markupObject["display"];
  if (!displayItem.IsObject())
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::vtkInternal::UpdateMarkupsDisplayNodeFromJsonDocument failed:"
      << " invalid display item is not found");
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
    vtkErrorWithObjectMacro(this->External, "WriteData failed: invalid class name");
    return false;
    }
  writer.Key("type"); writer.String(markupsType.c_str());

  writer.Key("coordinateSystem");
  writer.String(this->External->GetCoordinateSystemAsString(this->External->GetCoordinateSystem()));

  writer.Key("locked");
  writer.Bool(markupsNode->GetLocked());

  writer.Key("labelFormat");
  writer.String(markupsNode->GetMarkupLabelFormat().c_str());

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
    vtkErrorWithObjectMacro(this->External, "vtkMRMLMarkupsJsonStorageNode::WriteControlPoints failed:"
      << " Invalid coordinate system: " << coordinateSystem);
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

    double xyz[3] = { 0.0, 0.0, 0.0 };
    markupsNode->GetNthControlPointPosition(controlPointIndex, xyz);
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      xyz[0] = -xyz[0];
      xyz[1] = -xyz[1];
      }
    writer.Key("position"); this->WriteVector(writer, xyz);

    double* orientationMatrix = markupsNode->GetNthControlPointOrientationMatrix(controlPointIndex);
    if (coordinateSystem == vtkMRMLStorageNode::CoordinateSystemLPS)
      {
      double orientationMatrixLPS[9] =
        {
        -orientationMatrix[0], -orientationMatrix[1], -orientationMatrix[2],
        -orientationMatrix[3], -orientationMatrix[4], -orientationMatrix[5],
        orientationMatrix[6], orientationMatrix[7], orientationMatrix[8]
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
  writer.Key("snapMode"); writer.String(markupsDisplayNode->GetSnapModeAsString(markupsDisplayNode->GetSnapMode()));

  writer.EndObject();
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsJsonStorageNode::vtkInternal::WriteVector(
  rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, double v[3], int numberOfComponents/*=3*/)
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
vtkMRMLMarkupsNode* vtkMRMLMarkupsJsonStorageNode::AddNewMarkupsNodeFromFile(const char* filePath, const char* nodeName/*=nullptr*/, int markupIndex/*=0*/)
{
  vtkMRMLScene* scene = this->GetScene();
  if (!scene)
    {
    vtkErrorMacro("vtkMRMLMarkupsJsonStorageNode::AddMarkupsNodeFromFile failed: invalid scene");
    return nullptr;
    }
  if (!filePath)
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: invalid filename");
    return nullptr;
    }
  rapidjson::Document* jsonRoot = this->Internal->CreateJsonDocumentFromFile(filePath);
  if (!jsonRoot)
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: error opening the file '" << filePath);
    return nullptr;
    }

  rapidjson::Value& markups = (*jsonRoot)["markups"];
  if (!markups.IsArray())
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::AddNewMarkupsNodeFromFile failed: cannot read markup from file " << filePath
      << " (does not contain valid 'markups' array)");
    delete jsonRoot;
    return nullptr;
    }
  if (markupIndex >= static_cast<int>(markups.GetArray().Size()))
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::AddNewMarkupsNodeFromFile failed: cannot read markup from file " << filePath
      << " requested markup index " << markupIndex << " is not found (number of available markups: " << markups.GetArray().Size());
    delete jsonRoot;
    return nullptr;
    }

  rapidjson::Value& markup = markups.GetArray()[markupIndex];
  std::string className = this->Internal->GetMarkupsClassNameFromJsonValue(markup);
  if (!markup.HasMember("type"))
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::AddNewMarkupsNodeFromFile failed: cannot find required 'type' value");
    delete jsonRoot;
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
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: cannot instantiate class '" << className);
    delete jsonRoot;
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

  delete jsonRoot;

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

  markupsNode->SetAndObserveStorageNodeID(this->GetID());
  return markupsNode;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkErrorMacro("ReadDataInternal: null reference node!");
    return 0;
    }

  const char* filePath = this->GetFileName();
  if (!filePath)
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: invalid filename");
    return 0;
    }
  rapidjson::Document* jsonRoot = this->Internal->CreateJsonDocumentFromFile(filePath);
  if (!jsonRoot)
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: error opening the file '" << filePath);
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
    vtkErrorMacro("vtkMRMLMarkupsStorageNode::ReadDataInternal failed: cannot read " << refNode->GetClassName()
      << " markups class from file " << filePath << " (it contains " << className << ")");
    return 0;
    }

  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(refNode);
  bool success = this->Internal->UpdateMarkupsNodeFromJsonValue(markupsNode, markup);

  this->Modified();
  delete jsonRoot;
  return success ? 1 : 0;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsJsonStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLMarkupsJsonStorageNode: File name not specified");
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
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known markups node");
    return 0;
    }

  // open the file for writing
  FILE* fp = fopen(fullName.c_str(), "wb");
  if (!fp)
    {
    vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
    return 0;
    }

  // Prepare JSON writer and output stream.
  char writeBuffer[65536];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);

  bool success = true;
  writer.StartObject();
  writer.Key("@schema"); writer.String(MARKUPS_SCHEMA.c_str());

  writer.Key("markups");
  writer.StartArray();

  writer.StartObject();
  success = success && this->Internal->WriteBasicProperties(writer, markupsNode);
  success = success && this->Internal->WriteControlPoints(writer, markupsNode);
  if (success)
    {
    vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
    if (displayNode)
      {
      success = success && this->Internal->WriteDisplayProperties(writer, displayNode);
      }
    }
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
