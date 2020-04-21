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

const char* COMPONENT_SEPERATOR = "_";

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::vtkMRMLTableStorageNode()
{
  this->DefaultWriteFileExtension = "tsv";
  this->AutoFindSchema = true;
}

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::~vtkMRMLTableStorageNode() = default;

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
std::vector<vtkMRMLTableStorageNode::ColumnInfo> vtkMRMLTableStorageNode::GetColumnInfo(vtkMRMLTableNode* tableNode, vtkTable* rawTable)
{
  vtkTable* schema = tableNode->GetSchema();
  std::vector<vtkMRMLTableStorageNode::ColumnInfo> columnDetails;

  vtkStringArray* schemaColumnNameArray = nullptr;
  vtkStringArray* schemaComponentNamesArray = nullptr;
  if (schema != nullptr)
    {
    schemaColumnNameArray = vtkStringArray::SafeDownCast(schema->GetColumnByName("columnName"));
    schemaComponentNamesArray = vtkStringArray::SafeDownCast(schema->GetColumnByName("componentNames"));
    }

  // Populate the output table column details.
  // If the schema exists, read the contents and determine column data type/component names/component arrays
  if (schema != nullptr &&
      schemaColumnNameArray != nullptr &&
      schemaComponentNamesArray != nullptr)
    {
    for (int schemaRowIndex = 0; schemaRowIndex < schema->GetNumberOfRows(); ++schemaRowIndex)
      {
      vtkMRMLTableStorageNode::ColumnInfo columnInfo;
      columnInfo.ColumnName = schemaColumnNameArray->GetValue(schemaRowIndex);
      columnInfo.ScalarType = tableNode->GetColumnValueTypeFromSchema(columnInfo.ColumnName);
      columnInfo.NullValueString = tableNode->GetColumnProperty(columnInfo.ColumnName, "nullValue");

      std::vector<vtkAbstractArray*> componentArrays;
      std::string componentNamesStr = schemaComponentNamesArray->GetValue(schemaRowIndex);
      if (componentNamesStr.empty())
        {
        vtkAbstractArray* rawColumn = rawTable->GetColumnByName(columnInfo.ColumnName.c_str());
        if (rawColumn == nullptr)
          {
          /// We still add the invalid column to the info so that it can be replace with default values later
          vtkWarningMacro("vtkMRMLTableStorageNode::GetColumnInfo: invalid column " << columnInfo.ColumnName);
          }
        componentArrays.push_back(rawColumn);
        }
      else
        {
        std::stringstream ss(componentNamesStr);
        std::string componentName;
        while (std::getline(ss, componentName, '|'))
          {
          std::string componentColumnName = columnInfo.ColumnName + COMPONENT_SEPERATOR + componentName;
          vtkAbstractArray* rawColumn = rawTable->GetColumnByName(componentColumnName.c_str());
          if (rawColumn == nullptr)
            {
            /// We still add the invalid column to the info so that it can be replace with default values later
            vtkWarningMacro("vtkMRMLTableStorageNode::GetColumnInfo: invalid column - " << columnInfo.ColumnName
              << " component - " << componentName);
            }
          componentArrays.push_back(rawColumn);
          columnInfo.ComponentNames.push_back(componentName);
          }
        }
      columnInfo.RawComponentArrays = componentArrays;
      columnDetails.push_back(columnInfo);
      }
    }
  else
    {
    for (int col = 0; col < rawTable->GetNumberOfColumns(); ++col)
      {
      vtkMRMLTableStorageNode::ColumnInfo columnInfo;
      vtkStringArray* column = vtkStringArray::SafeDownCast(rawTable->GetColumn(col));
      if (column == nullptr)
        {
        vtkWarningMacro("vtkMRMLTableStorageNode::GetColumnInfo: invalid column - " << col);
        continue;
        }
      if (!column->GetName())
        {
        vtkWarningMacro("vtkMRMLTableStorageNode::GetColumnDetails: empty column name, skipping column");
        continue;
        }
      columnInfo.ColumnName = column->GetName();
      columnInfo.ScalarType = tableNode->GetColumnValueTypeFromSchema(columnInfo.ColumnName);
      columnInfo.RawComponentArrays.push_back(column);
      columnInfo.NullValueString = tableNode->GetColumnProperty(columnInfo.ColumnName, "nullValue");
      columnDetails.push_back(columnInfo);
      }
    }
  return columnDetails;
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::FillDataFromStringArray(vtkStringArray* stringComponentArray, vtkDataArray* typedComponentArray, std::string nullValueString)
{
  if (!stringComponentArray || !typedComponentArray)
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::FillTypedComponentArray: Invalid input");
    return;
    }

  if (stringComponentArray->GetNumberOfValues() != typedComponentArray->GetNumberOfValues())
    {
    vtkErrorMacro("vtkMRMLTableStorageNode::FillTypedComponentArray: Number of tuples between string ("
      << stringComponentArray->GetNumberOfValues() << ") and typed (" << stringComponentArray->GetNumberOfValues() << ") arrays");
    return;
    }

  int numberOfTuples = stringComponentArray->GetNumberOfTuples();

  // Initialize with null value
  if (typedComponentArray->IsNumeric())
    {
    // numeric arrays can be initialized in one batch
    double nullValue = 0.0;
    if (!nullValueString.empty())
      {
      nullValue = vtkVariant(nullValueString).ToDouble();
      }
    typedComponentArray->FillComponent(0, nullValue);
    }
  else
    {
    vtkVariant nullValue(nullValueString);
    for (vtkIdType row = 0; row < numberOfTuples; ++row)
      {
      typedComponentArray->SetVariantValue(row, nullValue);
      }
    }

  // Set values
  vtkIdType scalarTypeId = typedComponentArray->GetDataType();
  if (scalarTypeId == VTK_CHAR || scalarTypeId == VTK_SIGNED_CHAR || scalarTypeId == VTK_UNSIGNED_CHAR)
    {
    bool valid = false;
    for (vtkIdType row = 0; row < numberOfTuples; ++row)
      {
      if (stringComponentArray->GetValue(row).empty())
        {
        // empty cell, leave the null value
        continue;
        }
      int value = stringComponentArray->GetVariantValue(row).ToInt(&valid);
      if (!valid)
        {
        continue;
        }
      typedComponentArray->SetVariantValue(row, vtkVariant(value));
      }
    }
  else
    {
    for (vtkIdType row = 0; row < numberOfTuples; ++row)
      {
      if (stringComponentArray->GetValue(row).empty())
        {
        // empty cell, leave the null value
        continue;
        }
      typedComponentArray->SetVariantValue(row, stringComponentArray->GetVariantValue(row));
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::AddColumnToTable(vtkTable* table, vtkMRMLTableStorageNode::ColumnInfo columnInfo)
{
  std::string columnName = columnInfo.ColumnName;
  int valueTypeId = columnInfo.ScalarType;
  std::vector<vtkAbstractArray*> rawComponentArrays = columnInfo.RawComponentArrays;
  std::string nullValueString = columnInfo.NullValueString;

  if (valueTypeId == VTK_VOID)
    {
    // schema is not defined or no valid column type is defined for column
    valueTypeId = VTK_STRING;
    }
  if (valueTypeId == VTK_STRING)
    {
    if (rawComponentArrays.size() > 0)
      {
      vtkAbstractArray* columnArray = rawComponentArrays[0];
      if (columnArray)
        {
        columnArray->SetName(columnName.c_str());
        table->AddColumn(columnArray);
        }
      }
    }
  else
    {
    // Output column. Can be multi-component
    vtkSmartPointer<vtkDataArray> typedColumn = vtkSmartPointer<vtkDataArray>::Take(vtkDataArray::CreateDataArray(valueTypeId));
    typedColumn->SetName(columnName.c_str());
    typedColumn->SetNumberOfComponents(rawComponentArrays.size());
    vtkIdType numberOfTuples = 0;
    for (vtkAbstractArray* rawComponentArray : rawComponentArrays)
      {
      if (rawComponentArray == nullptr)
        {
        continue;
        }
      numberOfTuples = std::max(numberOfTuples, rawComponentArray->GetNumberOfTuples());
      }
    typedColumn->SetNumberOfTuples(numberOfTuples);

    vtkIdType componentIndex = 0;
    for (vtkAbstractArray* componentArray : rawComponentArrays)
      {
      vtkSmartPointer<vtkStringArray> rawComponentArray = vtkStringArray::SafeDownCast(componentArray);
      if (rawComponentArray == nullptr)
        {
        vtkWarningMacro("vtkMRMLTableStorageNode::ReadTable: Failed to read component for column " << columnName);
        // Add an empty default array for components that are not found
        rawComponentArray = vtkSmartPointer<vtkStringArray>::New();
        rawComponentArray->SetNumberOfComponents(1);
        rawComponentArray->SetNumberOfTuples(numberOfTuples);
        }

      // Single-component array for a potentially multi-component column
      vtkSmartPointer<vtkDataArray> typedComponentArray = vtkSmartPointer<vtkDataArray>::Take(vtkDataArray::CreateDataArray(valueTypeId));
      typedComponentArray->SetName(rawComponentArray->GetName());
      typedComponentArray->SetNumberOfComponents(1);
      typedComponentArray->SetNumberOfTuples(numberOfTuples);

      /// Fill the component array with the correct values of the correct type
      this->FillDataFromStringArray(rawComponentArray, typedComponentArray, nullValueString);

      if (rawComponentArrays.size() > 1)
        {
        // Multi-component column. Copy the contents of the single component column into the output.
        typedColumn->CopyComponent(componentIndex, typedComponentArray, 0);
        }
      else
        {
        // Single-component column. Add the column directly to the output.
        typedColumn = typedComponentArray;
        }

      if (componentIndex < static_cast<vtkIdType>(columnInfo.ComponentNames.size()))
        {
        std::string componentName = columnInfo.ComponentNames[componentIndex];
        typedColumn->SetComponentName(componentIndex, componentName.c_str());
        }
      ++componentIndex;
      }
    table->AddColumn(typedColumn);
  }
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
    return false;
    }

  /// Get the info for the columns defined in the schema (Column name, component arrays, component names, scalar type)
  /// If the schema does not exist, then the raw table is used to generate the table info.
  std::vector<vtkMRMLTableStorageNode::ColumnInfo> columnDetails = this->GetColumnInfo(tableNode, rawTable);

  vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
  for (vtkMRMLTableStorageNode::ColumnInfo columnInfo : columnDetails)
    {
    this->AddColumnToTable(table, columnInfo);
    }

  tableNode->SetAndObserveTable(table);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::WriteTable(std::string filename, vtkMRMLTableNode* tableNode)
{
  vtkTable* originalTable = tableNode->GetTable();

  vtkNew<vtkTable> newTable;
  for (int i = 0; i < originalTable->GetNumberOfColumns(); ++i)
    {
    vtkAbstractArray* oldColumn = originalTable->GetColumn(i);
    vtkDataArray* oldDataArray = vtkDataArray::SafeDownCast(oldColumn);
    int numberOfComponents = oldColumn->GetNumberOfComponents();

    // Component names are only valid for vtkDataArray
    // If we cannot cast to a vtkDataArray, then add to table as is.
    // Otherwise, separate to individual components
    if (!oldDataArray)
      {
      newTable->AddColumn(oldColumn);
      }
    else
      {
      std::string columnName;
      if (oldColumn->GetName())
        {
        columnName = oldColumn->GetName();
        }

      std::vector<std::string> componentNames = tableNode->GetComponentNames(columnName);
      for (int componentIndex = 0; componentIndex < numberOfComponents; ++componentIndex)
        {
        std::string newColumnName = columnName;
        if (static_cast<int>(componentNames.size()) > componentIndex)
          {
          std::stringstream newColumnNameSS;
          newColumnNameSS << columnName << COMPONENT_SEPERATOR << componentNames[componentIndex];
          newColumnName = newColumnNameSS.str();
          }
        vtkSmartPointer<vtkDataArray> newColumn = vtkSmartPointer<vtkDataArray>::Take(oldDataArray->NewInstance());
        newColumn->SetNumberOfComponents(1);
        newColumn->SetNumberOfTuples(oldColumn->GetNumberOfTuples());
        newColumn->SetName(newColumnName.c_str());
        newColumn->CopyComponent(0, oldDataArray, componentIndex);
        newTable->AddColumn(newColumn);
        }
      }
    }

  vtkNew<vtkDelimitedTextWriter> writer;
  writer->SetFileName(filename.c_str());
  writer->SetInputData(newTable);

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

  // Create a copy, as it is not nice if writing to file has a side effect of modifying some
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
    schemaTable->AddColumn(newArray);
    columnNameArray = newArray;
    }

  vtkStringArray* columnTypeArray = vtkStringArray::SafeDownCast(schemaTable->GetColumnByName("type"));
  if (columnTypeArray == nullptr)
    {
    vtkNew<vtkStringArray> newArray;
    newArray->SetName("type");
    newArray->SetNumberOfValues(schemaTable->GetNumberOfRows());
    schemaTable->AddColumn(newArray);
    columnTypeArray = newArray;
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

  vtkStringArray* componentNamesArray = vtkStringArray::SafeDownCast(schemaTable->GetColumnByName("componentNames"));
  if (componentNamesArray == nullptr)
    {
    vtkNew<vtkStringArray> newArray;
    newArray->SetName("componentNames");
    newArray->SetNumberOfValues(schemaTable->GetNumberOfRows());
    schemaTable->AddColumn(newArray);
    componentNamesArray = newArray;
    }

  if (componentNamesArray != nullptr)
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

      std::vector<std::string> componentNames = vtkMRMLTableNode::GetComponentNamesFromArray(column);
      std::string componentNamesStr = vtkMRMLTableNode::GetComponentNamesAsString(componentNames);
      componentNamesArray->SetValue(schemaRowIndex, componentNamesStr.c_str());
      }
    }

  vtkNew<vtkDelimitedTextWriter> writer;
  vtkNew<vtkErrorSink> errorWarningObserver;
  errorWarningObserver->SetObservedObject(writer);

  writer->SetFileName(filename.c_str());
  writer->SetInputData(schemaTable);

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
