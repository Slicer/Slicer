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
#include <vtkErrorSink.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkBitArray.h>
#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::vtkMRMLTableStorageNode()
{
  this->DefaultWriteFileExtension = "tsv";
  this->AutoFindSchema = true;
}

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::~vtkMRMLTableStorageNode()
= default;

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

  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }
  vtkMRMLTableNode *tableNode = vtkMRMLTableNode::SafeDownCast(refNode);
  if (tableNode == nullptr)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID() << " to a table node");
    return 0;
    }

  // Check that the file exists
  if (vtksys::SystemTools::FileExists(fullName) == false)
    {
    vtkErrorMacro("ReadData: table file '" << fullName << "' not found.");
    return 0;
    }

  if (this->GetSchemaFileName().empty() && this->AutoFindSchema)
    {
    this->SetSchemaFileName(this->FindSchemaFileName(fullName.c_str()).c_str());
    }
  if (!this->GetSchemaFileName().empty())
    {
    if (!this->ReadSchema(this->GetSchemaFileName(), tableNode))
      {
      vtkErrorMacro("ReadData: failed to read table schema from '" << this->GetSchemaFileName() << "'");
      return 0;
      }
    }

  if (!this->ReadTable(fullName, tableNode))
    {
    vtkErrorMacro("ReadData: failed to read table from '" << fullName << "'");
    return 0;
    }

  vtkDebugMacro("ReadData: successfully read table from file: " << fullName);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTableStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  if (this->GetFileName() == nullptr)
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
  if (tableNode == nullptr)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a valid table node");
    return 0;
    }

  if (!this->WriteTable(fullName, tableNode))
    {
    vtkErrorMacro("WriteData: failed to write table node " << refNode->GetID() << " to file " << fullName);
    return 0;
    }
  vtkDebugMacro("WriteData: successfully wrote table to file: " << fullName);

  // Only write a schema file if some table properties are specified
  bool needToWriteSchema = (!this->GetSchemaFileName().empty()) || (tableNode->GetSchema() != nullptr);
  if (!needToWriteSchema && tableNode->GetTable() != nullptr)
    {
    // Make sure we create a schema file if there is any non-string column type
    vtkTable* table = tableNode->GetTable();
    for (int col = 0; col < table->GetNumberOfColumns(); ++col)
      {
      vtkAbstractArray* column = table->GetColumn(col);
      if (column == nullptr)
        {
        // invalid column
        continue;
        }
      if (column->GetDataType() != VTK_STRING)
        {
        needToWriteSchema = true;
        break;
        }
      }
    }

  if (needToWriteSchema)
    {
    std::string schemaFileName = this->GenerateSchemaFileName(fullName.c_str());
    this->SetSchemaFileName(schemaFileName.c_str());
    if (!this->WriteSchema(schemaFileName, tableNode))
      {
      vtkErrorMacro("WriteData: failed to write table node " << refNode->GetID() << " schema  to file " << schemaFileName);
      return 0;
      }
    vtkDebugMacro("WriteData: successfully wrote schema to file: " << schemaFileName);
    }

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
void vtkMRMLTableStorageNode::SetSchemaFileName(const char* schemaFileName)
{
  this->ResetFileNameList();
  this->AddFileName(schemaFileName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableStorageNode::GetSchemaFileName()
{
  const char* schemaFileNamePtr = this->GetNthFileName(0);
  return (schemaFileNamePtr ? schemaFileNamePtr : "");
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableStorageNode::FindSchemaFileName(const char* filePath)
{
  std::string expectedSchemaFileName = this->GenerateSchemaFileName(filePath);
  if (!vtksys::SystemTools::FileExists(expectedSchemaFileName))
    {
    // schema file not found
    return "";
    }
  return expectedSchemaFileName;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableStorageNode::GenerateSchemaFileName(const char* filePath)
{
  std::string filePathStd = (filePath ? filePath : "");
  if (filePathStd.empty())
    {
    // If filePath is not specified then use current filename
    filePathStd = (this->GetFileName() ? this->GetFileName() : "");
    }
  if (filePathStd.empty())
    {
    return "";
    }
  std::string fileName = vtksys::SystemTools::GetFilenameName(filePathStd);
  std::string extension = this->GetSupportedFileExtension(fileName.c_str());

  if (fileName.length() < extension.length() ||
    fileName.compare(fileName.length() - extension.length(), extension.length(), extension) != 0)
    {
    // extension not matched to the end of filename
    return "";
    }

  // Insert .schema before file extension (something.csv => something.schema.csv)
  filePathStd.insert(filePathStd.length() - extension.length(), + ".schema");

  return filePathStd;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableStorageNode::GetFieldDelimiterCharacters(std::string filename)
{
  std::string lowercaseFileExt = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(filename);
  std::string fieldDelimiterCharacters;
  if (lowercaseFileExt == std::string(".tsv") || lowercaseFileExt == std::string(".txt"))
    {
    fieldDelimiterCharacters = "\t";
    }
  else if (lowercaseFileExt == std::string(".csv"))
    {
    fieldDelimiterCharacters = ",";
    }
  else
    {
    vtkErrorMacro("Cannot determine field delimiter character from file extension: " << lowercaseFileExt);
    }
  return fieldDelimiterCharacters;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::ReadSchema(std::string filename, vtkMRMLTableNode* tableNode)
{
  if (filename.empty())
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::ReadSchema failed: filename not specified");
    return false;
    }

  if (vtksys::SystemTools::FileExists(filename) == false)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::ReadSchema failed: schema file '" << filename << "' not found.");
    return false;
    }

  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(filename.c_str());
  reader->SetHaveHeaders(true);
  reader->SetFieldDelimiterCharacters(this->GetFieldDelimiterCharacters(filename).c_str());
  // Make sure string delimiter characters are removed (somebody may have written a tsv with string delimiters)
  reader->SetUseStringDelimiter(true);
  // File contents is preserved better if we don't try to detect numeric columns
  reader->DetectNumericColumnsOff();

  // Read table
  vtkTable* schemaTable = nullptr;
  try
    {
    reader->Update();
    schemaTable = reader->GetOutput();
    }
  catch (...)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::ReadSchema failed from file: " << filename);
    return false;
    }

  vtkStringArray* columnNameArray = vtkStringArray::SafeDownCast(schemaTable->GetColumnByName("columnName"));
  if (columnNameArray == nullptr)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::ReadSchema failed from file: " << filename <<". Required 'columnName' column is not found in schema.");
    return false;
    }

  tableNode->SetAndObserveSchema(schemaTable);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::ReadTable(std::string filename, vtkMRMLTableNode* tableNode)
{
  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(filename.c_str());
  reader->SetHaveHeaders(true);
  reader->SetFieldDelimiterCharacters(this->GetFieldDelimiterCharacters(filename).c_str());
  // Make sure string delimiter characters are removed (somebody may have written a tsv with string delimiters)
  reader->SetUseStringDelimiter(true);
  // File contents is preserved better if we don't try to detect numeric columns
  reader->DetectNumericColumnsOff();

  // Read table
  vtkTable* rawTable = nullptr;
  try
    {
    reader->Update();
    rawTable = reader->GetOutput();
    }
  catch (...)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::ReadTable: failed to read table file: " << filename);
    return 0;
    }

  vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
  for (int col = 0; col < rawTable->GetNumberOfColumns(); ++col)
    {
    vtkStringArray* column = vtkStringArray::SafeDownCast(rawTable->GetColumn(col));
    if (column == nullptr)
      {
      // invalid column
      continue;
      }
    if (!column->GetName())
      {
      vtkWarningMacro("vtkMRMLTableStorageNode::ReadTable: empty column name in file: " << filename << ", skipping column");
      continue;
      }

    std::string columnName = column->GetName();
    int valueTypeId = tableNode->GetColumnValueTypeFromSchema(columnName);
    if (valueTypeId == VTK_VOID)
      {
      // schema is not defined or no valid column type is defined for column
      valueTypeId = VTK_STRING;
      }
    if (valueTypeId == VTK_STRING)
      {
      column->SetName(columnName.c_str());
      table->AddColumn(column);
      }
    else
      {
      vtkSmartPointer<vtkDataArray> typedColumn = vtkSmartPointer<vtkDataArray>::Take(vtkDataArray::CreateDataArray(valueTypeId));
      typedColumn->SetName(columnName.c_str());
      vtkIdType numberOfTuples = column->GetNumberOfTuples();
      typedColumn->SetNumberOfTuples(numberOfTuples);

      // Initialize with null value
      std::string nullValueStr = tableNode->GetColumnProperty(columnName, "nullValue");
      if (typedColumn->IsNumeric())
        {
        // numeric arrays can be initialized in one batch
        double nullValue = 0.0;
        if (!nullValueStr.empty())
          {
          nullValue = vtkVariant(nullValueStr).ToDouble();
          }
        typedColumn->FillComponent(0, nullValue);
        }
      else
        {
        vtkVariant nullValue(nullValueStr);
        for (vtkIdType row = 0; row < numberOfTuples; ++row)
          {
          typedColumn->SetVariantValue(row, nullValue);
          }
        }

      // Set values
      if (valueTypeId == VTK_CHAR || valueTypeId == VTK_SIGNED_CHAR || valueTypeId == VTK_UNSIGNED_CHAR)
        {
        bool valid = false;
        for (vtkIdType row = 0; row < numberOfTuples; ++row)
          {
          if (column->GetValue(row).empty())
            {
            // empty cell, leave the null value
            continue;
            }
          int value = column->GetVariantValue(row).ToInt(&valid);
          if (!valid)
            {
            continue;
            }
          typedColumn->SetVariantValue(row, vtkVariant(value));
          }
        }
      else
        {
        for (vtkIdType row = 0; row < numberOfTuples; ++row)
          {
          if (column->GetValue(row).empty())
            {
            // empty cell, leave the null value
            continue;
            }
          typedColumn->SetVariantValue(row, column->GetVariantValue(row));
          }
        }

      table->AddColumn(typedColumn);
      }
    }
  tableNode->SetAndObserveTable(table);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::WriteTable(std::string filename, vtkMRMLTableNode* tableNode)
{
  vtkNew<vtkDelimitedTextWriter> writer;
  writer->SetFileName(filename.c_str());
  writer->SetInputData(tableNode->GetTable());

  std::string delimiter = this->GetFieldDelimiterCharacters(filename);
  writer->SetFieldDelimiter(delimiter.c_str());

  // SetUseStringDelimiter(true) causes writing each value in double-quotes, which is not very nice,
  // but if the delimiter character is the comma then we have to use this mode, as commas occur in
  // string values quite often.
  writer->SetUseStringDelimiter(delimiter==",");

  vtkNew<vtkErrorSink> errorWarningObserver;
  errorWarningObserver->SetObservedObject(writer);
  try
    {
    writer->Write();
    }
  catch (...)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::WriteTable: failed to write file: " << filename);
    return false;
    }
  errorWarningObserver->DisplayMessages();
  if (errorWarningObserver->HasErrors())
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::WriteTable: failed to write file: " << filename);
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::WriteSchema(std::string filename, vtkMRMLTableNode* tableNode)
{
  vtkNew<vtkTable> schemaTable;

  // Create a copy, as it is nice if writing to file has a side effect of modifying some
  // data in the node
  if (tableNode->GetSchema())
    {
    schemaTable->DeepCopy(tableNode->GetSchema());
    }

  vtkStringArray* columnNameArray = vtkStringArray::SafeDownCast(schemaTable->GetColumnByName("columnName"));
  if (columnNameArray == nullptr)
    {
    vtkNew<vtkStringArray> newArray;
    newArray->SetName("columnName");
    newArray->SetNumberOfValues(schemaTable->GetNumberOfRows());
    schemaTable->AddColumn(newArray.GetPointer());
    columnNameArray = newArray.GetPointer();
    }

  vtkStringArray* columnTypeArray = vtkStringArray::SafeDownCast(schemaTable->GetColumnByName("type"));
  if (columnTypeArray == nullptr)
    {
    vtkNew<vtkStringArray> newArray;
    newArray->SetName("type");
    newArray->SetNumberOfValues(schemaTable->GetNumberOfRows());
    schemaTable->AddColumn(newArray.GetPointer());
    columnTypeArray = newArray.GetPointer();
    }

  // Add column type to schema
  vtkTable* table = tableNode->GetTable();
  if (table != nullptr)
    {
    for (int col = 0; col < table->GetNumberOfColumns(); ++col)
      {
      vtkAbstractArray* column = table->GetColumn(col);
      if (column == nullptr)
        {
        // invalid column
        continue;
        }
      if (!column->GetName())
        {
        vtkWarningMacro("vtkMRMLTableStorageNode::WriteSchema: empty column name in file: " << filename << ", skipping column");
        continue;
        }

      vtkIdType schemaRowIndex = columnNameArray->LookupValue(column->GetName());
      if (schemaRowIndex < 0)
        {
        schemaRowIndex = schemaTable->InsertNextBlankRow();
        columnNameArray->SetValue(schemaRowIndex, column->GetName());
        }
      columnTypeArray->SetValue(schemaRowIndex, vtkMRMLTableNode::GetValueTypeAsString(column->GetDataType()));
      }
    }

  vtkNew<vtkDelimitedTextWriter> writer;
  vtkNew<vtkErrorSink> errorWarningObserver;
  errorWarningObserver->SetObservedObject(writer);

  writer->SetFileName(filename.c_str());
  writer->SetInputData(schemaTable.GetPointer());

  std::string delimiter = this->GetFieldDelimiterCharacters(filename);
  writer->SetFieldDelimiter(delimiter.c_str());

  // SetUseStringDelimiter(true) causes writing each value in double-quotes, which is not very nice,
  // but if the delimiter character is the comma then we have to use this mode, as commas occur in
  // string values quite often.
  writer->SetUseStringDelimiter(delimiter == ",");

  try
    {
    writer->Write();
    }
  catch (...)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::WriteSchema: failed to write file: " << filename);
    return false;
    }

  errorWarningObserver->DisplayMessages();
  if (errorWarningObserver->HasErrors())
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::WriteTable: failed to write file: " << filename);
    return false;
    }

  return true;
}
