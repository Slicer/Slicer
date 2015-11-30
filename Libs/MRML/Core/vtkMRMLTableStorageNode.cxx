/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// MRML includes
#include "vtkMRMLTableStorageNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkDelimitedTextReader.h>
#include <vtkDelimitedTextWriter.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::vtkMRMLTableStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::~vtkMRMLTableStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLTableNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTableStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string(""))
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }
  vtkMRMLTableNode *tableNode = vtkMRMLTableNode::SafeDownCast(refNode);
  if (tableNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a table node");
    return 0;
    }

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName) == false)
    {
    vtkErrorMacro("ReadData: table file '" << fullName << "' not found.");
    return 0;
    }

  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(fullName.c_str());
  reader->SetHaveHeaders(true);
  // Make sure string delimiter characters are removed (sombody may have written a tsv with string delimiters)
  reader->SetUseStringDelimiter(true);
  // File contents is preserved better if we don't try to detect numeric columns
  reader->DetectNumericColumnsOff();

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  vtkDebugMacro("ReadData: extension = " << extension);
  if ( extension == std::string(".tsv") || extension == std::string(".txt"))
    {
    reader->SetFieldDelimiterCharacters("\t");

    }
  else if ( extension == std::string(".csv") )
    {
    reader->SetFieldDelimiterCharacters(",");
    }
  else
    {
    vtkErrorMacro("ReadData: failed to read table file: " << fullName << " - file extension not supported: " << extension );
    return 0;
    }
  try
    {
    reader->Update();
    tableNode->SetAndObserveTable(reader->GetOutput());
    }
  catch (...)
    {
    vtkErrorMacro("ReadData: failed to read table file: " << fullName);
    return 0;
    }

  vtkDebugMacro("ReadData: successfully read table from file: " << fullName);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTableStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  if (this->GetFileName() == NULL)
    {
    vtkErrorMacro("WriteData: file name is not set");
    return 0;
    }
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
      vtkErrorMacro("WriteData: file name not specified");
    return 0;
    }

  vtkMRMLTableNode *tableNode = vtkMRMLTableNode::SafeDownCast(refNode);
  if (tableNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a valid table node");
    return 0;
    }

  vtkNew<vtkDelimitedTextWriter> writer;
  writer->SetFileName(fullName.c_str());
  writer->SetInputData( tableNode->GetTable() );

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension == ".tsv" || extension == ".txt")
    {
    writer->SetFieldDelimiter("\t");
    writer->SetUseStringDelimiter(false); // otherwise it would write each value in double-quotes
    }
  else if (extension == ".csv")
    {
    writer->SetFieldDelimiter(",");
    writer->SetUseStringDelimiter(true); // it causes writing each value in double-quotes but allows using commas in the string
    }
  else
    {
    vtkErrorMacro("WriteData: failed to write file: " << fullName << " - file extension not supported: " << extension );
    return 0;
    }
  try
    {
    writer->Write();
    }
  catch (...)
    {
    vtkErrorMacro("WriteData: failed to write file: " << fullName );
    return 0;
    }

  vtkDebugMacro("WriteData: successfully wrote table to file: " << fullName);
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Tab-separated values (.tsv)");
  this->SupportedReadFileTypes->InsertNextValue("Comma-separated values (.csv)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Tab-separated values (.tsv)");
  this->SupportedWriteFileTypes->InsertNextValue("Comma-separated values (.csv)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLTableStorageNode::GetDefaultWriteFileExtension()
{
  return "tsv";
}
