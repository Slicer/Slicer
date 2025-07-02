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

#include "vtkDataFileFormatHelper.h"
#include "vtkMRMLI18N.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTextNode.h"
#include "vtkMRMLTextStorageNode.h"

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
void vtkMRMLTextStorageNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  if (!this->SupportedReadFileExtensions.empty())
  {
    vtkMRMLWriteXMLStdStringVectorMacro(readExtensions, SupportedReadFileExtensions, std::vector);
  }
  if (!this->SupportedWriteFileExtensions.empty())
  {
    vtkMRMLWriteXMLStdStringVectorMacro(writeExtensions, SupportedWriteFileExtensions, std::vector);
  }
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStdStringVectorMacro(readExtensions, SupportedReadFileExtensions, std::vector);
  vtkMRMLReadXMLStdStringVectorMacro(writeExtensions, SupportedWriteFileExtensions, std::vector);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLTextStorageNode* node = vtkMRMLTextStorageNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyStdStringVectorMacro(SupportedReadFileExtensions);
  vtkMRMLCopyStdStringVectorMacro(SupportedWriteFileExtensions);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStdStringVectorMacro(SupportedReadFileExtensions, std::vector);
  vtkMRMLPrintStdStringVectorMacro(SupportedWriteFileExtensions, std::vector);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
int vtkMRMLTextStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
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
bool vtkMRMLTextStorageNode::CanWriteFromReferenceNode(vtkMRMLNode* refNode)
{
  vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(refNode);
  if (textNode == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                        std::string("Only text nodes can written in this format."));
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTextStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(refNode);
  if (textNode == nullptr)
  {
    vtkErrorMacro(<< "vtkMRMLTextStorageNode::WriteDataInternal: Do not recognize node type "
                  << refNode->GetClassName());
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
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                       "vtkMRMLTextStorageNode::WriteDataInternal",
                                       "Text file '" << fullName.c_str()
                                                     << "' could not be overwritten while trying to write ("
                                                     << (this->ID ? this->ID : "(unknown)") << ").");
      return 0;
    }
  }

  std::ofstream file;
  file.open(fullName);
  if (!file.is_open())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLTextStorageNode::WriteDataInternal",
                                     "Text file '" << fullName.c_str()
                                                   << "' could not be opened for writing while trying to write ("
                                                   << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }
  file << textNode->GetText();
  this->StageWriteData(refNode);
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLTextStorageNode", "Text file") + " (.txt)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLTextStorageNode", "XML document") + " (.xml)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLTextStorageNode", "JSON document") + " (.json)");
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLTextStorageNode", "Text file") + " (.txt)");
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLTextStorageNode", "XML document") + " (.xml)");
  //: File format name
  this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLTextStorageNode", "JSON document") + " (.json)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLTextStorageNode::GetDefaultWriteFileExtension()
{
  // use the first writause the first write file type as file extension
  vtkStringArray* fileTypes = this->GetSupportedWriteFileTypes();
  if (fileTypes && fileTypes->GetNumberOfValues() > 0)
  {
    std::string fileExtension =
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(fileTypes->GetValue(0).c_str());
    if (!fileExtension.empty())
    {
      // Remove leading "."
      if (fileExtension[0] == '.')
      {
        fileExtension.erase(0, 1);
      }
      this->SetDefaultWriteFileExtension(fileExtension.c_str());
      return this->DefaultWriteFileExtension.c_str();
    }
  }
  return "txt";
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::SetSupportedReadFileExtensions(const std::vector<std::string> fileExtensions)
{
  this->SupportedReadFileExtensions = fileExtensions;
  this->SupportedReadFileTypes->Reset();
  this->SupportedReadFileTypes->SetNumberOfTuples(0);
  for (const std::string& fileExtension : fileExtensions)
  {
    // We do not let developers to specify custom file format description, because then a translated format description
    // would need to be stored in the scene file, which would be difficult to get translated to the current application
    // language.
    this->SupportedReadFileTypes->InsertNextValue(
      vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLTextStorageNode", "Text file (.%1)"), fileExtension.c_str()));
  }
}

//----------------------------------------------------------------------------
void vtkMRMLTextStorageNode::SetSupportedWriteFileExtensions(const std::vector<std::string> fileExtensions)
{
  this->SupportedWriteFileExtensions = fileExtensions;
  this->SupportedWriteFileTypes->Reset();
  this->SupportedWriteFileTypes->SetNumberOfTuples(0);
  for (const std::string& fileExtension : fileExtensions)
  {
    // We do not let developers to specify custom file format description, because then a translated format description
    // would need to be stored in the scene file, which would be difficult to get translated to the current application
    // language.
    this->SupportedWriteFileTypes->InsertNextValue(
      vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLTextStorageNode", "Text file (.%1)"), fileExtension.c_str()));
  }
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLTextStorageNode::GetSupportedReadFileExtensions()
{
  return this->SupportedReadFileExtensions;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLTextStorageNode::GetSupportedWriteFileExtensions()
{
  return this->SupportedWriteFileExtensions;
}
