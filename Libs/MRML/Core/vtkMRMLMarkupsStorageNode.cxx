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

#include "vtkMRMLMarkupsStorageNode.h"

#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include <sstream>


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::vtkMRMLMarkupsStorageNode()
{
  this->CoordinateSystem = vtkMRMLMarkupsStorageNode::LPS;
  this->DefaultWriteFileExtension = "mcsv";
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::~vtkMRMLMarkupsStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(CoordinateSystem);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(CoordinateSystem);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsStorageNode::ReadDataInternal(vtkMRMLNode *vtkNotUsed(refNode))
{
  vtkWarningMacro("Subclasses must implement ReadDataInternal!");
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsStorageNode::WriteDataInternal(vtkMRMLNode * vtkNotUsed(refNode))
{
  vtkWarningMacro("Subclasses must implement WriteDataInternal!");
  return 0;

}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Markups CSV (.mcsv)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Markups CSV (.mcsv)");
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString()
{
  return vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(this->CoordinateSystem);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseRASOn()
{
  this->SetCoordinateSystem(vtkMRMLStorageNode::CoordinateSystemRAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseRAS()
{
  if (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemRAS)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseLPSOn()
{
  this->SetCoordinateSystem(vtkMRMLStorageNode::CoordinateSystemLPS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseLPS()
{
  if (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemLPS)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::ConvertStringToStorageFormat(std::string input)
{
  std::string output = input;

  // are there any commas that will require the string to have double
  // quotes put around it? also for now putting quotes around the
  // whole string if there are internal quotes
  bool surroundingQuotesNeeded = false;
  size_t commaPos = output.find(",");
  size_t quotePos = output.find("\"");

  if (commaPos != std::string::npos ||
      quotePos != std::string::npos)
    {
    surroundingQuotesNeeded = true;
    }

  // escape any extant double quotes by adding an extra double quote
  while (quotePos != std::string::npos)
    {
    output.replace(quotePos, 1, std::string("\"\""));
    quotePos = output.find("\"",quotePos+2);
    }
  if (surroundingQuotesNeeded)
    {
    // put quotes around the whole thing
    output = std::string("\"") + output + std::string("\"");
    }
  return output;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::ConvertStringFromStorageFormat(std::string input)
{
  std::string output = input;

  if (output.size() == 0)
    {
    return output;
    }
  // remove any leading and trailing quotes
  if (output.find_first_of("\"") == size_t(0))
    {
    output.erase(0, 1);
    }
  size_t last = output.size() - 1;
  if (output.find_last_of("\"") == last)
    {
    output.erase(last, 1);
    }

  // change any doubled quotes to single quotes
  size_t quotesPos = output.find("\"\"");
  while (quotesPos != std::string::npos)
    {
    output.replace(quotesPos, 2, "\"");
    quotesPos = output.find("\"\"");
    }
  return output;
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::GetFirstQuotedString(std::string inputString, size_t *endCommaPos)
{
  std::string outputString;
  size_t pos;
  if (inputString[0] != '"')
    {
    vtkWarningMacro("GetFirstQuotedString: input string does not start with a double quote: " << inputString.c_str());
    return outputString;
    }

  pos = inputString.find("\",");
  // is this a double quote then a comma?
  if (pos > 0)
    {
    while (inputString[pos-1] == '"')
      {
      // if it's a triple quote then a comma, that's valid
      if (pos > 1 &&
          inputString[pos-2] == '"')
        {
        //std::cout << "\t\tGetFirstQuotedString: found a triple quote and comma at pos " << pos << std::endl;
        break;
        }
      //std::cout << "\t\tGetFirstQuotedString: found a double quote and a comma at pos " << pos << std::endl;
      // find the next candidate
      pos = inputString.find("\",", pos+1);
      }
    }
  // include the end quote in the substring
  pos++;
  //std::cout << "\tGetFirstQuotedString: returning end comma position " << pos << std::endl;
  *endCommaPos = pos;
  outputString = inputString.substr(0, *endCommaPos);
  return outputString;
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString(int id)
{
  return vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(id);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsStorageNode::GetCoordinateSystemFromString(const char* name)
{
  // For backward-compatibility with old scenes (magic number was used instead of string)
  if (strcmp(name, "0") == 0)
    {
    return vtkMRMLStorageNode::CoordinateSystemRAS;
    }
  else if (strcmp(name, "1") == 0)
    {
    return vtkMRMLStorageNode::CoordinateSystemLPS;
    }

  // Current method, store coordinate system as string
  return vtkMRMLStorageNode::GetCoordinateSystemTypeFromString(name);
}
