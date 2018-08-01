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

#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"

#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

#include <sstream>

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
  CsvCodec() : Separator(',') {}
  ~CsvCodec() {}

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

  char Separator;
  std::vector<std::string> Fields;
};

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialStorageNode::vtkMRMLMarkupsFiducialStorageNode()
{
  this->DefaultWriteFileExtension = "fcsv";
  this->FieldDelimiterCharacters = ",";
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialStorageNode::~vtkMRMLMarkupsFiducialStorageNode()
{
}

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
  return refNode->IsA("vtkMRMLMarkupsFiducialNode");
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString(vtkMRMLMarkupsNode *markupsNode, int markupIndex, const char* line)
{
  if (!markupsNode)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed: invalid markupsnode");
    return false;
    }

  if (markupIndex<0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsFiducialStorageNode::SetMarkupFromString failed: invalid markupIndex");
    return false;
    }

  if (this->GetCoordinateSystem() != vtkMRMLMarkupsFiducialStorageNode::RAS
    && this->GetCoordinateSystem() != vtkMRMLMarkupsFiducialStorageNode::LPS)
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

  // Set values in markup

  int wasModified = markupsNode->StartModify();
  if (markupIndex >= markupsNode->GetNumberOfMarkups())
    {
    markupIndex = markupsNode->AddMarkupWithNPoints(1);
    }

  if (id.empty())
    {
    if (this->GetScene())
      {
      id = this->GetScene()->GenerateUniqueName(this->GetID());
      }
    }
  markupsNode->SetNthMarkupID(markupIndex, id);

  if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::RAS)
    {
    markupsNode->SetMarkupPoint(markupIndex, 0, xyz[0], xyz[1], xyz[2]);
    }
  else if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::LPS)
    {
    markupsNode->SetMarkupPointLPS(markupIndex, 0, xyz[0], xyz[1], xyz[2]);
    }

  markupsNode->SetNthMarkupOrientation(markupIndex, wxyz[0], wxyz[1], wxyz[2], wxyz[3]);

  markupsNode->SetNthMarkupVisibility(markupIndex, visibility);
  markupsNode->SetNthMarkupSelected(markupIndex, selected);
  markupsNode->SetNthMarkupLocked(markupIndex, locked);
  markupsNode->SetNthMarkupLabel(markupIndex, label);
  markupsNode->SetNthMarkupDescription(markupIndex, description);
  markupsNode->SetNthMarkupAssociatedNodeID(markupIndex, associatedNodeID);
  markupsNode->EndModify(wasModified);

  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsFiducialStorageNode::GetMarkupAsString(vtkMRMLMarkupsNode *markupsNode, int markupIndex)
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

  std::string id = markupsNode->GetNthMarkupID(markupIndex);
  vtkDebugMacro("WriteDataInternal: wrote id " << id.c_str());

  int p = 0;
  double xyz[3] = { 0.0, 0.0, 0.0 };
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::RAS)
    {
    markupsNode->GetMarkupPoint(markupIndex,p,xyz);
    }
  else if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::LPS)
    {
    markupsNode->GetMarkupPointLPS(markupIndex,p,xyz);
    }
  else
    {
    vtkErrorMacro("WriteData: invalid coordinate system index " << this->GetCoordinateSystem());
    return "";
    }

  double orientation[4] = { 1.0, 0.0, 0.0, 0.0 };
  markupsNode->GetNthMarkupOrientation(markupIndex, orientation);
  bool vis = markupsNode->GetNthMarkupVisibility(markupIndex);
  bool sel = markupsNode->GetNthMarkupSelected(markupIndex);
  bool lock = markupsNode->GetNthMarkupLocked(markupIndex);

  std::string label = this->ConvertStringToStorageFormat(markupsNode->GetNthMarkupLabel(markupIndex));
  std::string desc = this->ConvertStringToStorageFormat(markupsNode->GetNthMarkupDescription(markupIndex));

  std::string associatedNodeID = markupsNode->GetNthMarkupAssociatedNodeID(markupIndex);

  std::stringstream of;
  of.precision(3);
  of.setf(std::ios::fixed, std::ios::floatfield);
  of << id.c_str();
  of << separator << xyz[0] << separator << xyz[1] << separator << xyz[2];
  of << separator << orientation[0] << separator << orientation[1] << separator << orientation[2] << separator << orientation[3];
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
  vtkMRMLMarkupsNode *markupsNode =
    vtkMRMLMarkupsNode::SafeDownCast(refNode);
  if (!markupsNode)
    {
    return 0;
    }

  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  vtkMRMLDisplayNode * mrmlNode = markupsNode->GetDisplayNode();
  if (mrmlNode)
    {
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(mrmlNode);
    }
  if (!displayNode)
    {
    vtkWarningMacro("ReadDataInternal: no display node!");
    if (this->GetScene())
      {
      vtkWarningMacro("ReadDataInternal: adding a new display node.");
      displayNode = vtkMRMLMarkupsDisplayNode::New();
      this->GetScene()->AddNode(displayNode);
      markupsNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      displayNode->Delete();
      }
    }

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
    if (markupsNode->GetNumberOfMarkups() > 0)
      {
      // clear out the list
      markupsNode->RemoveAllMarkups();
      }

    char line[MARKUPS_BUFFER_SIZE];

    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    int thisMarkupNumber = 0;

    // check for the version
    std::string version;
    // only print out the warning once
    bool printedVersionWarning = false;

    // coordinate system
    int coordinateSystemFlag = 0;

    while (fstr.good())
      {
      fstr.getline(line, MARKUPS_BUFFER_SIZE);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");

        // if there's a space after the hash, check for the version
        if (line[1] == ' ')
          {
          vtkDebugMacro("Have a possible option in line " << line);
          std::string lineString = std::string(line);
          if (lineString.find("# Markups fiducial file version = ") != std::string::npos)
            {
            version = lineString.substr(34,std::string::npos);
            vtkDebugMacro("Version = " << version);
            }
          else if (lineString.find("# CoordinateSystem = ") != std::string::npos)
            {
            std::string str = lineString.substr(21,std::string::npos);
            coordinateSystemFlag = atoi(str.c_str());
            vtkDebugMacro("CoordinateSystem = " << coordinateSystemFlag);
            this->SetCoordinateSystem(coordinateSystemFlag);
            }
          else if (lineString.find("# columns = ") != std::string::npos)
            {
            // the markups header, fixed
            }
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

            markupsNode->AddMarkupWithNPoints(1);

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
                std::string filenameName = vtksys::SystemTools::GetFilenameName(this->GetFileName());
                label = vtksys::SystemTools::GetFilenameWithoutExtension(filenameName);
                markupsNode->SetNthMarkupLabel(thisMarkupNumber,label);
                }

              // x,y,z
              getline(ss, component, '|');
              x = atof(component.c_str());
              getline(ss, component, '|');
              y = atof(component.c_str());
              getline(ss, component, '|');
              z = atof(component.c_str());
              markupsNode->SetMarkupPoint(thisMarkupNumber,0,x,y,z);

              // selected
              getline(ss, component, '|');
              sel = atoi(component.c_str());
              markupsNode->SetNthMarkupSelected(thisMarkupNumber,sel);

              // visibility
              getline(ss, component, '|');
              vtkDebugMacro("component = " << component.c_str());
              vis = atoi(component.c_str());
              markupsNode->SetNthMarkupVisibility(thisMarkupNumber,vis);
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
                markupsNode->SetNthMarkupLabel(thisMarkupNumber,label);
                }

              // x,y,z
              getline(ss, component, separator);
              x = atof(component.c_str());
              getline(ss, component, separator);
              y = atof(component.c_str());
              getline(ss, component, separator);
              z = atof(component.c_str());
              markupsNode->SetMarkupPoint(thisMarkupNumber,0,x,y,z);

              // selected
              getline(ss, component, separator);
              sel = atoi(component.c_str());
              markupsNode->SetNthMarkupSelected(thisMarkupNumber,sel);

              // visibility
              getline(ss, component, separator);
              vtkDebugMacro("component = " << component.c_str());
              vis = atoi(component.c_str());
              markupsNode->SetNthMarkupVisibility(thisMarkupNumber,vis);
              }
            thisMarkupNumber++;
            }
          else
            {
            // Slicer 4 markups fiducial file
            vtkDebugMacro("\n\n\n\nVersion = " << version << ", got a line: \n\"" << line << "\"");
            this->SetMarkupFromString(markupsNode, markupsNode->GetNumberOfMarkups(), line);
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
  vtkMRMLMarkupsNode *markupsNode = NULL;
  if ( refNode->IsA("vtkMRMLMarkupsNode") )
    {
    markupsNode = dynamic_cast <vtkMRMLMarkupsNode *> (refNode);
    }

  if (markupsNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known markups node");
    return 0;
    }

  // open the file for writing
  fstream of;

  of.open(fullName.c_str(), fstream::out);

  if (!of.is_open())
    {
    vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
    return 0;
    }
  int numberOfMarkups = markupsNode->GetNumberOfMarkups();

  // put down a header
  of << "# Markups fiducial file version = " << Slicer_VERSION << endl;
  of << "# CoordinateSystem = " << this->GetCoordinateSystem() << endl;

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
  for (int i = 0; i < numberOfMarkups; i++)
    {
    of << this->GetMarkupAsString(markupsNode, i);
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
