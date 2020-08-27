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

#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

#include "itkNumberToString.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

// CSV table field indexes
static const int FIELD_ID = 0;
static const int FIELD_XYZ = 1; // 3 values
static const int FIELD_WXYZ = 4; // 4 values
static const int FIELD_VISIBILITY = 8;
static const int FIELD_SELECTED = 9;
static const int FIELD_LOCKED = 10;
static const int FIELD_LABEL = 11;
static const int FIELD_DESCRIPTION = 12;
static const int FIELD_ASSOCIATED_NODE_ID = 13;

//------------------------------------------------------------------------------

class CsvCodec
{
public:
  CsvCodec()  = default;
  ~CsvCodec()  = default;

  void ReadFromString(const std::string &row)
    {
    CSVState state = UnquotedField;
    this->Fields.clear();
    std::string currentField;
    for (std::string::const_iterator c = row.begin(); c != row.end(); ++c)
      {
      switch (state)
        {
      case UnquotedField:
        if (*c == this->Separator)
          {
          // end of field
          this->Fields.push_back(currentField);
          currentField.clear();
          }
        else if (*c == '"' && currentField.empty())
          {
          // If quote occurs within the field then the quote does
          // not indicate a quoted field, it simply means a quote character.
          // Therefore, only switch to quoted-field mode if quote
          // is the first character in the field.
          state = QuotedField;
          }
        else
          {
          currentField.push_back(*c);
          }
        break;
      case QuotedField:
        if (*c == '"')
          {
          state = QuotedQuote;
          }
        else
          {
          currentField.push_back(*c);
          }
        break;
      case QuotedQuote:
        if (*c == this->Separator)
          {
          // , after closing quote
          this->Fields.push_back(currentField);
          currentField.clear();
          state = UnquotedField;
          }
        else if (*c == '"')
          {
          // double-quote ("") in a quoted field means a single quote (")
          currentField.push_back('"');
          state = QuotedField;
          }
        else
          {
          // This is an invalid character sequence, such as the last quote
          // and the following space in this example:
          //   ...,"This ""is"" a, quoted" field,...
          // We save the character and revert back to unquoted mode to not lose any data:
          //   [This "is" a, quoted field]
          currentField.push_back(*c);
          state = UnquotedField;
          }
        break;
        }
      }
    this->Fields.push_back(currentField);
    }

  char GetSeparator() { return this->Separator; }
  void SetSeparator(char separator) { this->Separator = separator; }

  inline int GetNumberOfFields() const
  {
    return static_cast<int>(this->Fields.size());
  }

  std::string GetField(int fieldIndex)
    {
    if (fieldIndex < 0 || fieldIndex >= this->GetNumberOfFields())
      {
      return "";
      }
    return this->Fields[fieldIndex];
    }


  bool GetStringField(int fieldIndex, std::string &fieldValue)
    {
    if (fieldIndex < 0 || fieldIndex >= this->GetNumberOfFields())
      {
      return false;
      }
    fieldValue = this->Fields[fieldIndex];
    return true;
    }

  bool GetDoubleField(int fieldIndex, double &fieldValue)
    {
    if (fieldIndex < 0 || fieldIndex >= this->GetNumberOfFields())
      {
      return false;
      }
    vtkVariant v = vtkVariant(this->Fields[fieldIndex]);
    fieldValue = v.ToDouble();
    return v.IsValid();
    }

  bool GetDoubleField(int fieldIndex, double &fieldValue, double defaultValue)
    {
    if (fieldIndex < 0 || fieldIndex >= this->GetNumberOfFields() || this->Fields[fieldIndex].empty())
      {
      fieldValue = defaultValue;
      return true;
      }
    vtkVariant v = vtkVariant(this->Fields[fieldIndex]);
    fieldValue = v.ToDouble();
    if (!v.IsValid())
      {
      fieldValue = defaultValue;
      return false;
      }
    return true;
    }

  bool GetIntField(int fieldIndex, int &fieldValue)
    {
    if (fieldIndex < 0 || fieldIndex >= this->GetNumberOfFields())
      {
      return false;
      }
    vtkVariant v = vtkVariant(this->Fields[fieldIndex]);
    fieldValue = v.ToInt();
    return v.IsValid();
    }

  bool GetIntField(int fieldIndex, int &fieldValue, int defaultValue)
    {
    if (fieldIndex < 0 || fieldIndex >= this->GetNumberOfFields() || this->Fields[fieldIndex].empty())
      {
      fieldValue = defaultValue;
      return true;
      }
    vtkVariant v = vtkVariant(this->Fields[fieldIndex]);
    fieldValue = v.ToInt();
    if (!v.IsValid())
      {
      fieldValue = defaultValue;
      return false;
      }
    return true;
    }

protected:
  enum CSVState
    {
    UnquotedField,
    QuotedField,
    QuotedQuote
    };

  char Separator{','};
  std::vector<std::string> Fields;
};


namespace
{
  const std::string WHITESPACE = " \n\r\t\f\v";

  std::string ltrim(const std::string& s)
    {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
    }

  std::string rtrim(const std::string& s)
    {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

  std::string trim(const std::string& s)
    {
    return rtrim(ltrim(s));
    }
}

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialStorageNode::vtkMRMLMarkupsFiducialStorageNode()
{
  this->DefaultWriteFileExtension = "fcsv";
  this->FieldDelimiterCharacters = ",";
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialStorageNode::~vtkMRMLMarkupsFiducialStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsFiducialNode") ||
         refNode->IsA("vtkMRMLMarkupsLineNode") ||
         refNode->IsA("vtkMRMLMarkupsAngleNode") ||
         refNode->IsA("vtkMRMLMarkupsCurveNode") ||
         refNode->IsA("vtkMRMLMarkupsPlaneNode");
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialStorageNode::SetPointFromString(vtkMRMLMarkupsNode *markupsNode, int pointIndex, const char* line)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed: invalid markupsnode");
    return false;
    }

  if (pointIndex<0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed: invalid pointIndex");
    return false;
    }

  if (this->GetCoordinateSystem() != vtkMRMLStorageNode::CoordinateSystemRAS
    && this->GetCoordinateSystem() != vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed: invalid coordinate system");
    return false;
    }

  char separator = ',';
  if (!this->FieldDelimiterCharacters.empty())
    {
    separator = this->FieldDelimiterCharacters[0];
    }

  CsvCodec parser;
  parser.SetSeparator(separator);
  parser.ReadFromString(line);

  // ID (if missing, use default)
  std::string id;
  parser.GetStringField(FIELD_ID, id);

  // Position
  double xyz[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; i++)
    {
    if (!parser.GetDoubleField(FIELD_XYZ + i, xyz[i]))
      {
      vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed:"
        << " numeric values expected for xyz, got instead: "<<parser.GetField(FIELD_XYZ + i));
      return false;
      }
    }

  // Orientation
  double wxyz[4] = { 1.0, 0.0, 0.0, 0.0 };
  for (int i = 0; i < 4; i++)
    {
    if (!parser.GetDoubleField(FIELD_WXYZ + i, wxyz[i], wxyz[i]))
      {
      vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed:"
        " numeric values expected for wxyz, got instead: " << parser.GetField(FIELD_WXYZ + i));
      return false;
      }
    }

  // Flag attributes
  int visibility = 1;
  if (!parser.GetIntField(FIELD_VISIBILITY, visibility, visibility))
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed:"
      " numeric values expected for visibility field, got instead: " << parser.GetField(FIELD_VISIBILITY));
    return false;
    }
  int selected = 1;
  if (!parser.GetIntField(FIELD_SELECTED, selected, selected))
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed:"
      " numeric values expected for selected field, got instead: " << parser.GetField(FIELD_SELECTED));
    return false;
    }
  int locked = 0;
  if (!parser.GetIntField(FIELD_LOCKED, locked, locked))
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed:"
      " numeric values expected for locked field, got instead: " << parser.GetField(FIELD_LOCKED));
    return false;
    }

  std::string label;
  parser.GetStringField(FIELD_LABEL, label);
  std::string description;
  parser.GetStringField(FIELD_DESCRIPTION, description);
  std::string associatedNodeID;
  parser.GetStringField(FIELD_ASSOCIATED_NODE_ID, associatedNodeID);

  if (pointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    vtkVector3d point(0, 0, 0);
    markupsNode->AddControlPoint(point);
    }

  if (id.empty())
    {
    if (this->GetScene())
      {
      id = markupsNode->GenerateUniqueControlPointID();
      }
    }

  markupsNode->SetNthControlPointID(pointIndex, id);

  if (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemRAS)
    {
    markupsNode->SetNthControlPointPosition(pointIndex, xyz[0], xyz[1], xyz[2]);
    }
  else if (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    markupsNode->SetNthControlPointPosition(pointIndex, -xyz[0], -xyz[1], xyz[2]);
    }

  markupsNode->SetNthControlPointOrientation(pointIndex, wxyz[0], wxyz[1], wxyz[2], wxyz[3]);

  markupsNode->SetNthControlPointVisibility(pointIndex, visibility);
  markupsNode->SetNthControlPointSelected(pointIndex, selected);
  markupsNode->SetNthControlPointLocked(pointIndex, locked);
  markupsNode->SetNthControlPointLabel(pointIndex, label);
  markupsNode->SetNthControlPointDescription(pointIndex, description);
  markupsNode->SetNthControlPointAssociatedNodeID(pointIndex, associatedNodeID);

  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsFiducialStorageNode::GetPointAsString(vtkMRMLMarkupsNode *markupsNode, int pointIndex)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::GetMarkupAsString failed: invalid markupsnode");
    return "";
    }

  char separator = ',';
  if (!this->FieldDelimiterCharacters.empty())
    {
    separator = this->FieldDelimiterCharacters[0];
    }

  std::string id = markupsNode->GetNthControlPointID(pointIndex);
  vtkDebugMacro("WriteDataInternal: wrote id " << id.c_str());

  double xyz[3] = { 0.0, 0.0, 0.0 };
  markupsNode->GetNthControlPointPosition(pointIndex, xyz);
  if (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    xyz[0] = -xyz[0];
    xyz[1] = -xyz[1];
    }
  else if (this->GetCoordinateSystem() != vtkMRMLStorageNode::CoordinateSystemRAS)
    {
    vtkErrorMacro("WriteData: invalid coordinate system index " << this->GetCoordinateSystem());
    return "";
    }

  double orientation[4] = { 1.0, 0.0, 0.0, 0.0 };
  markupsNode->GetNthControlPointOrientation(pointIndex, orientation);
  bool vis = markupsNode->GetNthControlPointVisibility(pointIndex);
  bool sel = markupsNode->GetNthControlPointSelected(pointIndex);
  bool lock = markupsNode->GetNthControlPointLocked(pointIndex);

  std::string label = this->ConvertStringToStorageFormat(markupsNode->GetNthControlPointLabel(pointIndex));
  std::string desc = this->ConvertStringToStorageFormat(markupsNode->GetNthControlPointDescription(pointIndex));

  std::string associatedNodeID = markupsNode->GetNthControlPointAssociatedNodeID(pointIndex);

  // use double-conversion library (via ITK) for better
  // float64 string representation.
  itk::NumberToString<double> DoubleConvert;

  std::stringstream of;
  of.setf(std::ios::fixed, std::ios::floatfield);
  of << id.c_str();
  of << separator << DoubleConvert(xyz[0]) << separator << DoubleConvert(xyz[1]) << separator << DoubleConvert(xyz[2]);
  of << separator << DoubleConvert(orientation[0]) << separator << DoubleConvert(orientation[1]);
  of << separator << DoubleConvert(orientation[2]) << separator << DoubleConvert(orientation[3]);
  of << separator << vis << separator << sel << separator << lock;
  of << separator << label;
  of << separator << desc;
  of << separator << associatedNodeID;

  return of.str();
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkErrorMacro("ReadDataInternal: null reference node!");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(refNode);
  if (!markupsNode)
    {
    return 0;
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  // check if it's an annotation csv file
  bool parseAsAnnotationFiducial = false;
  std::string ext = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (ext.compare(".acsv") == 0)
    {
    parseAsAnnotationFiducial = true;
    }

  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    if (markupsNode->GetNumberOfControlPoints() > 0)
      {
      // clear out the list
      markupsNode->RemoveAllControlPoints();
      }

    char line[MARKUPS_BUFFER_SIZE];

    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    int thisMarkupNumber = 0;

    // check for the version
    std::string version;
    // only print out the warning once
    bool printedVersionWarning = false;

    while (fstr.good())
      {
      fstr.getline(line, MARKUPS_BUFFER_SIZE);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
        std::string lineString = std::string(line);
        lineString.erase(0, 1); // delete leading #

        int separatorIndex = lineString.find('=');
        std::string name = trim(lineString.substr(0, separatorIndex));
        std::string value = trim(lineString.substr(separatorIndex + 1));

        if (name == "Markups fiducial file version")
          {
          version = value;
          vtkDebugMacro("Version = " << version);
          }
        else if (name == "CoordinateSystem")
          {
          int coordinateSystemFlag = vtkMRMLMarkupsStorageNode::GetCoordinateSystemFromString(value.c_str());
          vtkDebugMacro("CoordinateSystem = " << coordinateSystemFlag);
          this->SetCoordinateSystem(coordinateSystemFlag);
          }
        else if (name == "columns")
          {
          // the markups header, for now the header is fixed, so the listed values are ignored
          }
        }
      else
        {
        // is it empty?
        if (line[0] == '\0')
          {
          vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
          }
        else
          {
          if (version.size() == 0)
            {
            std::string label = std::string("");
            double x = 0.0, y = 0.0, z = 0.0;
            int sel = 1, vis = 1;
            vtkVector3d point(0, 0, 0);
            markupsNode->AddControlPoint(point);

            std::stringstream ss(line);
            std::string component;

            if (parseAsAnnotationFiducial)
              {
              // annotation fiducial line format = point|x|y|z|sel|vis
              // label
              getline(ss, component, '|');
              if (component.size())
                {
                vtkDebugMacro("Got point string = " << component.c_str());
                // use the file name for the point label
                label = this->GetFileNameWithoutExtension(this->GetFileName());
                markupsNode->SetNthControlPointLabel(thisMarkupNumber,label);
                }

              // x,y,z
              getline(ss, component, '|');
              x = atof(component.c_str());
              getline(ss, component, '|');
              y = atof(component.c_str());
              getline(ss, component, '|');
              z = atof(component.c_str());
              markupsNode->SetNthControlPointPosition(thisMarkupNumber,x,y,z);

              // selected
              getline(ss, component, '|');
              sel = atoi(component.c_str());
              markupsNode->SetNthControlPointSelected(thisMarkupNumber,sel);

              // visibility
              getline(ss, component, '|');
              vtkDebugMacro("component = " << component.c_str());
              vis = atoi(component.c_str());
              markupsNode->SetNthControlPointVisibility(thisMarkupNumber,vis);
              }
            else
              {
              if (!printedVersionWarning)
                {
                vtkWarningMacro("Have an unversioned file, assuming Slicer 3 format .fcsv");
                printedVersionWarning = true;
                }
              // point line format = label,x,y,z,sel,vis

              char separator = ',';
              if (!this->FieldDelimiterCharacters.empty())
                {
                separator = this->FieldDelimiterCharacters[0];
                }

              // label
              getline(ss, component, separator);
              if (component.size())
                {
                vtkDebugMacro("Got label = " << component.c_str());
                label = component;
                markupsNode->SetNthControlPointLabel(thisMarkupNumber,label);
                }

              // x,y,z
              getline(ss, component, separator);
              x = atof(component.c_str());
              getline(ss, component, separator);
              y = atof(component.c_str());
              getline(ss, component, separator);
              z = atof(component.c_str());
              markupsNode->SetNthControlPointPosition(thisMarkupNumber,x,y,z);

              // selected
              getline(ss, component, separator);
              sel = atoi(component.c_str());
              markupsNode->SetNthControlPointSelected(thisMarkupNumber,sel);

              // visibility
              getline(ss, component, separator);
              vtkDebugMacro("component = " << component.c_str());
              vis = atoi(component.c_str());
              markupsNode->SetNthControlPointVisibility(thisMarkupNumber,vis);
              }
            thisMarkupNumber++;
            }
          else
            {
            vtkDebugMacro("\n\n\n\nVersion = " << version << ", got a line: \n\"" << line << "\"");
            this->SetPointFromString(markupsNode, markupsNode->GetNumberOfControlPoints(), line);

            thisMarkupNumber++;
            } // point line
          }
        }
      }
    fstr.close();
    }
  else
    {
    vtkErrorMacro("ERROR opening markups file " << this->FileName << endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialStorageNode: File name not specified");
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

  this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent,
    "fcsv file format only stores control point coordinates and a limited set of display properties.");

  // open the file for writing
  fstream of;

  of.open(fullName.c_str(), fstream::out);

  if (!of.is_open())
    {
    vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
    return 0;
    }

  // put down a header
  of << "# Markups fiducial file version = " << Slicer_VERSION << endl;
  of << "# CoordinateSystem = " << vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString(this->GetCoordinateSystem()) << endl;

  // label the columns
  // id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID
  // orientation is a quaternion, angle and axis
  // associatedNodeID and description and label can be empty strings
  // id,x,y,z,ow,ox,oy,oz,vis,sel,lock,,,
  // label can have spaces, everything up to next comma is used, no quotes
  // necessary, same with the description
  char separator = ',';
  if (!this->FieldDelimiterCharacters.empty())
  {
    separator = this->FieldDelimiterCharacters[0];
  }
  of << "# columns = id" << separator << "x" << separator << "y" << separator << "z" << separator << "ow" << separator
    << "ox" << separator << "oy" << separator << "oz" << separator << "vis" << separator << "sel" << separator
    << "lock" << separator << "label" << separator << "desc" << separator << "associatedNodeID" << endl;
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    of << this->GetPointAsString(markupsNode, i);
    of << endl;
    }

  of.close();

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Markups Fiducial CSV (.fcsv)");
  this->SupportedReadFileTypes->InsertNextValue("Annotation Fiducial CSV (.acsv)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Markups Fiducial CSV (.fcsv)");
}
