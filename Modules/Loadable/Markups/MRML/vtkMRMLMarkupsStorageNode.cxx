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
#include "vtkMRMLMarkupsNode.h"
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
  this->CoordinateSystem = vtkMRMLMarkupsStorageNode::RAS;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::~vtkMRMLMarkupsStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "coordinateSystem"))
      {
      this->SetCoordinateSystem(atoi(attValue));
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << "CoordinateSystem = " << this->GetCoordinateSystemAsString().c_str() << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkIndent indent(nIndent);

  of << indent << " coordinateSystem=\"" << this->CoordinateSystem << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLMarkupsStorageNode *node = (vtkMRMLMarkupsStorageNode *) anode;
  if (!node)
    {
    return;
    }

  this->SetCoordinateSystem(node->GetCoordinateSystem());
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
int vtkMRMLMarkupsStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
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
const char* vtkMRMLMarkupsStorageNode::GetDefaultWriteFileExtension()
{
  return "mcsv";
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString()
{
  std::string coordString;
  if (this->CoordinateSystem == vtkMRMLMarkupsStorageNode::RAS)
    {
    coordString = std::string("RAS");
    }
  else if (this->CoordinateSystem == vtkMRMLMarkupsStorageNode::LPS)
    {
    coordString = std::string("LPS");
    }
  else if (this->CoordinateSystem == vtkMRMLMarkupsStorageNode::IJK)
    {
    coordString = std::string("IJK");
    }
  return coordString;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseRASOn()
{
  this->SetCoordinateSystem(vtkMRMLMarkupsStorageNode::RAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseRAS()
{
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsStorageNode::RAS)
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
  this->SetCoordinateSystem(vtkMRMLMarkupsStorageNode::LPS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseLPS()
{
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsStorageNode::LPS)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseIJKOn()
{
  this->SetCoordinateSystem(vtkMRMLMarkupsStorageNode::IJK);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseIJK()
{
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsStorageNode::IJK)
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
