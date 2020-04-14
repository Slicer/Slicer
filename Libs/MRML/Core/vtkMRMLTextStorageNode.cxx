/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// STD includes
#include <algorithm>
#include <sstream>

#include <vtkMRMLTextNode.h>
#include "vtkMRMLTextStorageNode.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTextStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTextStorageNode::vtkMRMLTextStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLTextStorageNode::~vtkMRMLTextStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLTextStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLTextNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTextStorageNode::ReadDataInternal(vtkMRMLNode * refNode)
{
  if (!this->CanReadInReferenceNode(refNode))
    {
    return 0;
    }

  vtkMRMLTextNode* textNode = dynamic_cast<vtkMRMLTextNode*>(refNode);
  if (!textNode)
    {
    vtkErrorMacro("ReadDataInternal: not a text node.");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
    {
    vtkErrorMacro("ReadDataInternal: text file '" << fullName.c_str() << "' not found.");
    return 0;
    }

  // compute file extension
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension.empty())
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return 0;
    }

  vtkDebugMacro("ReadDataInternal: extension = " << extension.c_str());

  std::ifstream inputFile;
  inputFile.open(fullName);
  if (inputFile.fail())
    {
    vtkErrorMacro("vtkMRMLTextStorageNode::ReadDataInternal: Could not read file");
    return false;
    }

  std::stringstream ss;
  ss << inputFile.rdbuf();
  std::string inputString = ss.str();
  textNode->SetText(inputString.c_str());

  // success
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLTextStorageNode::CanWriteFromReferenceNode(vtkMRMLNode * refNode)
{
  vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(refNode);
  if (textNode == nullptr)
    {
    vtkErrorMacro("vtkMRMLTextStorageNode::CanWriteFromReferenceNode: input is not a text node");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTextStorageNode::WriteDataInternal(vtkMRMLNode * refNode)
{
  vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(refNode);
  if (textNode == nullptr)
    {
    vtkErrorMacro(<< "vtkMRMLTextStorageNode::WriteDataInternal: Do not recognize node type " << refNode->GetClassName());
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }

  // check if the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()))
    {
    if (!vtksys::SystemTools::RemoveFile(fullName.c_str()))
      {
      vtkErrorMacro("WriteData: Could not overwrite existing file");
      }
    }

  ofstream file;
  file.open(fullName);
  file << textNode->GetText();
  this->StageWriteData(refNode);
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Text file (.txt)");
  this->SupportedReadFileTypes->InsertNextValue("XML document (.xml)");
  this->SupportedReadFileTypes->InsertNextValue("JSON document (.json)");
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Text file (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("XML document (.xml)");
  this->SupportedWriteFileTypes->InsertNextValue("JSON document (.json)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLTextStorageNode::GetDefaultWriteFileExtension()
{
  return "txt";
}
