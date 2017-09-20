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
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>

// Reserved property names
static const char SCHEMA_COLUMN_NAME[] = "columnName";
static const char SCHEMA_COLUMN_TYPE[] = "type";
static const char SCHEMA_COLUMN_DEFAULT_VALUE[] = "defaultValue";
static const char SCHEMA_COLUMN_LONG_NAME[] = "longName";
static const char SCHEMA_COLUMN_DESCRIPTION[] = "description";
static const char SCHEMA_COLUMN_UNIT_LABEL[] = "unitLabel";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableNode);

//----------------------------------------------------------------------------
vtkMRMLTableNode::vtkMRMLTableNode()
{
  this->Table = NULL;
  this->Schema = NULL;
  this->Locked = false;
  this->UseColumnNameAsColumnHeader = false;
  this->UseFirstColumnAsRowHeader = false;
  this->HideFromEditorsOff();

  this->SetAndObserveTable(vtkSmartPointer<vtkTable>::New());
}

//----------------------------------------------------------------------------
vtkMRMLTableNode::~vtkMRMLTableNode()
{
  if (this->Table)
    {
    this->Table->Delete();
    this->Table = NULL;
    }
  if (this->Schema)
    {
    this->Schema->Delete();
    this->Schema = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);
  of << " locked=\"" << (this->GetLocked() ? "true" : "false") << "\"";
  of << " useFirstColumnAsRowHeader=\"" << (this->GetUseFirstColumnAsRowHeader() ? "true" : "false") << "\"";
  of << " useColumnNameAsColumnHeader=\"" << (this->GetUseColumnNameAsColumnHeader() ? "true" : "false") << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "locked"))
      {
      this->SetLocked(strcmp(attValue,"true")?0:1);
      }
    else if (!strcmp(attName, "useColumnNameAsColumnHeader"))
      {
      this->SetUseColumnNameAsColumnHeader(strcmp(attValue,"true")?0:1);
      }
    else if (!strcmp(attName, "useFirstColumnAsRowHeader"))
      {
      this->SetUseFirstColumnAsRowHeader(strcmp(attValue,"true")?0:1);
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
//
void vtkMRMLTableNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLTableNode *node = vtkMRMLTableNode::SafeDownCast(anode);
  if (!node)
    {
    vtkErrorMacro("vtkMRMLTableNode::Copy failed: invalid or incompatible source node");
    return;
    }
  int disabledModify = this->StartModify();
  Superclass::Copy(anode);
  // Schema
  if (this->GetSchema()!=NULL && node->GetSchema()==NULL)
    {
    this->SetAndObserveSchema(NULL);
    }
  else if (this->GetSchema() == NULL && node->GetSchema() != NULL)
    {
    vtkNew<vtkTable> newTable;
    newTable->DeepCopy(node->GetSchema());
    this->SetAndObserveSchema(newTable.GetPointer());
    }
  else if (this->GetSchema() != NULL && node->GetSchema() != NULL)
    {
    this->GetSchema()->DeepCopy(node->GetSchema());
    this->Schema->Modified();
    }
  // Table
  if (this->GetTable()!=NULL && node->GetTable()==NULL)
    {
    this->SetAndObserveTable(NULL);
    }
  else if (this->GetTable()==NULL && node->GetTable()!=NULL)
    {
    vtkNew<vtkTable> newTable;
    newTable->DeepCopy(node->GetTable());
    this->SetAndObserveTable(newTable.GetPointer());
    }
  else if(this->GetTable() != NULL && node->GetTable() != NULL)
    {
    this->GetTable()->DeepCopy(node->GetTable());
    this->Table->Modified();
    }
  this->SetLocked(node->GetLocked());
  this->SetUseColumnNameAsColumnHeader(node->GetUseColumnNameAsColumnHeader());
  this->SetUseFirstColumnAsRowHeader(node->GetUseFirstColumnAsRowHeader());
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  vtkTable* callerTable = vtkTable::SafeDownCast(caller);
  if (event == vtkCommand::ModifiedEvent &&  callerTable != NULL
    && (this->Table == callerTable || this->Schema == callerTable))
    {
    // this indicates that the table model (that is stored in a separate file) is modified
    // and therefore the object will be marked as changed for file saving
    this->StorableModifiedTime.Modified();
    // this indicates that data stored in the node is changed (either the table or other
    // data members are changed)
    this->Modified();
    return;
    }

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "\nLocked: " << this->GetLocked();
  os << indent << "\nUseColumnNameAsColumnHeader: " << this->GetUseColumnNameAsColumnHeader();
  os << indent << "\nUseFirstColumnAsRowHeader: " << this->GetUseFirstColumnAsRowHeader();
  os << indent << "\nColumns:";
  vtkTable* table = this->GetTable();
  if (table)
    {
    // Column (columnIndex): (columnName) [(columnType):(numberOfComponents)]
    os << (table->GetNumberOfColumns()>0 ? "\n" : " (none)");
    for (int columnIndex = 0; columnIndex < table->GetNumberOfColumns(); ++columnIndex)
      {
      os << indent << "  Column "<< columnIndex<<": ";
      vtkAbstractArray* column = table->GetColumn(columnIndex);
      if (column == NULL)
        {
        os << "(invalid)\n";
        continue;
        }
      os << (column->GetName() ? column->GetName() : "(undefined)");
      os << " [" << column->GetDataTypeAsString();
      if (column->GetNumberOfComponents() != 1)
        {
        os << ":" << column->GetNumberOfComponents();
        }
      os << "]\n";
      }
    }
  else
    {
    os << " (no table)\n";
    }
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTableNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLTableStorageNode::New());
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetAndObserveTable(vtkTable* table)
{
  if (table==this->Table)
    {
    return;
    }
  vtkSetAndObserveMRMLObjectMacro(this->Table, table);
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetAndObserveSchema(vtkTable* schema)
{
  if (schema == this->Schema)
    {
    return;
    }
  vtkSetAndObserveMRMLObjectMacro(this->Schema, schema);
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkAbstractArray* vtkMRMLTableNode::AddColumn(vtkAbstractArray* column)
{
  // Create table (if not available already)
  if (!this->Table)
    {
    this->SetAndObserveTable(vtkSmartPointer<vtkTable>::New());
    if (!this->Table)
      {
      vtkErrorMacro("vtkMRMLTableNode::AddColumn failed: failed to add VTK table");
      return 0;
      }
    }

  // Multiple modifications may be done in the table. Let's ignore modifications
  // until we are all done.
  int tableWasModified = this->StartModify();

  // Create new column (if not provided)
  vtkSmartPointer<vtkAbstractArray> newColumn;
  if (column)
    {
    newColumn = column;
    if (this->Table->GetNumberOfColumns()>0)
      {
      // There are columns in the table already, so we need to make sure the number of rows is matching
      int numberOfColumnsToAddToTable = newColumn->GetNumberOfTuples()-this->Table->GetNumberOfRows();
      if (numberOfColumnsToAddToTable>0)
        {
        // Table is shorter than the array, add empty rows to the table.
        for (int i=0; i<numberOfColumnsToAddToTable; i++)
          {
          this->InsertNextBlankRowWithDefaultValues(this->Table);
          }
        }
      else if (numberOfColumnsToAddToTable<0)
        {
        // Need to add more items to the array to match the table size.
        // To make sure that augmentation of the array is consistent, we create a dummy vtkTable
        // and use InsertNextBlankRowWithDefaultValues() method.
        vtkNew<vtkTable> augmentingTable;
        augmentingTable->AddColumn(newColumn);
        int numberOfColumnsToAddToArray = -numberOfColumnsToAddToTable;
        for (int i=0; i<numberOfColumnsToAddToArray; i++)
          {
          this->InsertNextBlankRowWithDefaultValues(augmentingTable.GetPointer());
          }
        }
      }
    }
  else
    {
    int numberOfRows = this->Table->GetNumberOfRows();
    newColumn = vtkSmartPointer<vtkFloatArray>::New();
    newColumn->SetNumberOfTuples(numberOfRows);
    vtkVariant emptyCell(0.);
    for (int i=0; i<numberOfRows; i++)
      {
      newColumn->SetVariantValue(i, emptyCell);
      }
    }

  // Generate a new unique column name (if not provided)
  if (!newColumn->GetName())
    {
    std::string newColumnName;
    int i=1;
    do
      {
      std::stringstream ss;
      ss << "Column " << i;
      newColumnName = ss.str();
      i++;
      }
    while (this->Table->GetColumnByName(newColumnName.c_str())!=0);
    newColumn->SetName(newColumnName.c_str());
    }

  this->Table->AddColumn(newColumn);
  this->Table->Modified();
  this->EndModify(tableWasModified);
  return newColumn;
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetColumnIndex(const char* columnName)
{
  if (!this->Table)
    {
    return -1;
    }
  return this->GetColumnIndex(this->Table->GetColumnByName(columnName));
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetColumnIndex(vtkAbstractArray* column)
{
  if (column == 0)
    {
    return -1;
    }
  if (!this->Table)
    {
    return -1;
    }

  for (int columnIndex = 0; columnIndex < this->Table->GetNumberOfColumns(); columnIndex++)
    {
    if (this->Table->GetColumn(columnIndex) == column)
      {
      return columnIndex;
      }
    }

  // not found
  return -1;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnName(int columnIndex)
{
  if (this->Table == NULL)
    {
    return "";
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == NULL)
    {
    return "";
    }
  return columnName;
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::CopyAllColumnProperties(const std::string& sourceColumnName, const std::string& targetColumnName)
{
  if (this->Schema == NULL)
    {
    vtkWarningMacro("vtkMRMLTableNode::CopyAllColumnProperties failed: schema is empty");
    return;
    }
  vtkIdType rowIndexTarget = this->GetPropertyRowIndex(targetColumnName);
  if (rowIndexTarget < 0)
    {
    vtkStringArray* nameColumn = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(SCHEMA_COLUMN_NAME));
    if (nameColumn == NULL)
      {
      vtkErrorMacro("vtkMRMLTableNode::CopyAllColumnProperties failed: schema is invalid");
      return;
      }
    rowIndexTarget = this->Schema->InsertNextBlankRow();
    nameColumn->SetValue(rowIndexTarget, targetColumnName);
    }
  int numberOfSchemaColumns = this->Schema->GetNumberOfColumns();
  vtkIdType rowIndexSource = this->GetPropertyRowIndex(sourceColumnName);
  for (int schemaColumnIndex = 0; schemaColumnIndex < numberOfSchemaColumns; schemaColumnIndex++)
    {
    vtkStringArray* column = vtkStringArray::SafeDownCast(this->Schema->GetColumn(schemaColumnIndex));
    if (column == NULL || column->GetName() == NULL // invalid column
      || !std::string(column->GetName()).compare(SCHEMA_COLUMN_NAME)) // columnName column
      {
      continue;
      }
    if (rowIndexSource >= 0)
      {
      column->SetValue(rowIndexTarget, column->GetValue(rowIndexSource));
      }
    else
      {
      column->SetValue(rowIndexTarget, "");
      }
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::RenameColumn(int columnIndex, const char* newNamePtr)
{
  std::string newName = (newNamePtr ? newNamePtr : "");
  if (newName.empty())
    {
    vtkErrorMacro("vtkMRMLTableNode::RenameColumn failed: invalid new name");
    return false;
    }
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::RenameColumn failed: invalid table");
    return false;
    }
  if (columnIndex<0 || columnIndex >= this->Table->GetNumberOfColumns())
    {
    vtkErrorMacro("vtkMRMLTableNode::RenameColumn failed: invalid column index: " << columnIndex);
    return false;
    }

  vtkAbstractArray* column = this->Table->GetColumn(columnIndex);
  std::string oldName = (column->GetName() ? column->GetName() : "");
  if (oldName == newName)
    {
    // no change
    return true;
    }

  bool wasModified = this->StartModify();

  // If a column already exists by the new name (for example
  // this happens when a column is copied to a new column)
  // then keep the column properties, otherwise overwrite it.
  if (this->Schema != NULL)
    {
    if (this->GetColumnIndex(newName.c_str()) < 0)
      {
      this->CopyAllColumnProperties(oldName, newName);
      }
    }

  // Rename column in table
  column->SetName(newName.c_str());

  // If no more columns exists by the old name
  // then we remove the old column properties.
  if (this->Schema != NULL)
    {
    if (this->GetColumnIndex(oldName.c_str()) < 0)
      {
      this->RemoveAllColumnProperties(oldName);
      }
    }

  this->Table->Modified();
  this->EndModify(wasModified);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::RemoveColumn(int columnIndex)
{
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveColumn failed: invalid table");
    return false;
    }
  if (columnIndex<0 || columnIndex>=this->Table->GetNumberOfColumns())
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveColumn failed: invalid column index: "<<columnIndex);
    return false;
    }
  std::string columnName = this->GetColumnName(columnIndex);

  bool wasModified = this->StartModify();
  this->Table->RemoveColumn(columnIndex);
  if (!this->Table->GetColumnByName(columnName.c_str()))
    {
    // there are no more columns by this name, so remove properties, too
    this->RemoveAllColumnProperties(columnName);
    }
  this->Table->Modified();
  this->EndModify(wasModified);

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::RemoveAllColumns()
{
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveAllColumns failed: invalid table");
    return false;
    }
  bool wasModified = this->StartModify();
  this->SetAndObserveSchema(NULL);
  this->Table->Initialize();
  this->EndModify(wasModified);
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::AddEmptyRow()
{
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::AddEmptyRow failed: invalid table");
    return -1;
    }
  int tableWasModified = this->StartModify();
  if (this->Table->GetNumberOfColumns()==0)
    {
    vtkDebugMacro("vtkMRMLTableNode::AddEmptyRow called for an empty table. Add an empty column first.");
    this->AddColumn();
    }
  vtkIdType rowIndex = this->InsertNextBlankRowWithDefaultValues(this->Table);
  this->Table->Modified();
  this->EndModify(tableWasModified);
  return rowIndex;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::RemoveRow(int rowIndex)
{
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveRow failed: invalid table");
    return false;
    }
  if (this->Table->GetNumberOfColumns()==0)
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveRow failed: no columns are defined");
    return false;
    }
    if (rowIndex<0 || rowIndex>=this->Table->GetNumberOfRows())
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveRow failed: invalid row index: "<<rowIndex);
    return false;
    }
  this->Table->RemoveRow(rowIndex);
  this->Table->Modified();
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetCellText(int rowIndex, int columnIndex)
{
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::GetCellText failed: invalid table");
    return "";
    }
  if (columnIndex<0 || columnIndex>=this->Table->GetNumberOfColumns())
    {
    vtkErrorMacro("vtkMRMLTableNode::GetCellText failed: invalid column index "<<columnIndex);
    return "";
    }
  if (rowIndex<0 || rowIndex>=this->Table->GetNumberOfRows())
    {
    vtkErrorMacro("vtkMRMLTableNode::GetCellText failed: invalid row index: "<<rowIndex);
    return "";
    }
  return this->Table->GetValue(rowIndex, columnIndex).ToString();
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::SetCellText(int rowIndex, int columnIndex, const char* text)
{
  if (!this->Table)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetCellText failed: invalid table");
    return false;
    }
  if (columnIndex<0 || columnIndex>=this->Table->GetNumberOfColumns())
    {
    vtkErrorMacro("vtkMRMLTableNode::SetCellText failed: invalid column index "<<columnIndex);
    return false;
    }
  if (rowIndex<0 || rowIndex>=this->Table->GetNumberOfRows())
    {
    vtkErrorMacro("vtkMRMLTableNode::SetCellText failed: invalid row index: "<<rowIndex);
    return false;
    }
  this->Table->SetValue(rowIndex, columnIndex, vtkVariant(text));
  this->Table->Modified();
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetNumberOfRows()
{
  if (!this->Table)
    {
    return 0;
    }
  return this->Table->GetNumberOfRows();
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetNumberOfColumns()
{
  if (!this->Table)
    {
    return 0;
    }
  return this->Table->GetNumberOfColumns();
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLTableNode::GetPropertyRowIndex(const std::string& columnName)
{
  if (this->Schema == NULL)
    {
    return -1;
    }
  vtkStringArray* columnNameArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(SCHEMA_COLUMN_NAME));
  if (columnNameArray == NULL)
    {
    return -1;
    }
  return columnNameArray->LookupValue(columnName);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::GetAllColumnPropertyNames(vtkStringArray* propertyNames)
{
  if (propertyNames == NULL)
    {
    vtkErrorMacro("vtkMRMLTableNode::GetAllColumnPropertyNames failed: invalid propertyNames");
    return;
    }

  propertyNames->Initialize();
  if (this->Schema == NULL)
    {
    return;
    }
  int numberOfSchemaColumns = this->Schema->GetNumberOfColumns();
  for (int schemaColumnIndex = 0; schemaColumnIndex < numberOfSchemaColumns; schemaColumnIndex++)
    {
    vtkAbstractArray* column = this->Schema->GetColumn(schemaColumnIndex);
    if (column == NULL || column->GetName() == NULL)
      {
      // invalid column
      continue;
      }
    std::string columnName = column->GetName();
    if (!columnName.compare(SCHEMA_COLUMN_NAME) || !columnName.compare(SCHEMA_COLUMN_TYPE))
      {
      // reserved for internal use
      continue;
      }
    propertyNames->InsertNextValue(columnName.c_str());
    }
}


//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnProperty(int columnIndex, const std::string& propertyName)
{
  if (this->Table == NULL)
    {
    return "";
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == NULL)
    {
    return "";
    }
  return this->GetColumnProperty(columnName, propertyName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnProperty(const std::string& columnName, const std::string& propertyName)
{
  if (propertyName == SCHEMA_COLUMN_NAME || propertyName == SCHEMA_COLUMN_TYPE)
    {
    vtkErrorMacro("vtkMRMLTableNode::GetColumnProperty failed: reserved propertyName: " << propertyName);
    return "";
    }
  return this->GetColumnPropertyInternal(columnName, propertyName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnPropertyInternal(const std::string& columnName, const std::string& propertyName)
{
  if (this->Schema == NULL)
    {
    return "";
    }

  vtkStringArray* propertyArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(propertyName.c_str()));
  if (propertyArray == NULL)
    {
    return "";
    }

  vtkIdType rowIndex = this->GetPropertyRowIndex(columnName);
  if (rowIndex < 0)
    {
    return "";
    }

  return propertyArray->GetValue(rowIndex);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnProperty(int columnIndex, const std::string& propertyName, const std::string& propertyValue)
{
  if (this->Table == NULL)
    {
    return;
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == NULL)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed: invalid column index");
    return;
    }
  this->SetColumnProperty(columnName, propertyName, propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnProperty(const std::string& columnName, const std::string& propertyName, const std::string& propertyValue)
{
  if (propertyName == SCHEMA_COLUMN_NAME || propertyName == SCHEMA_COLUMN_TYPE)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed: reserved propertyName: " << propertyName);
    return;
    }
  if (propertyName.empty())
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed: property name is invalid");
    return;
    }
  // Make sure there is a schema
  if (this->Schema == NULL)
    {
    vtkNew<vtkTable> newSchema;
    this->SetAndObserveSchema(newSchema.GetPointer());
    }
  // Make sure there is a column array
  vtkStringArray* columnNameArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(SCHEMA_COLUMN_NAME));
  if (columnNameArray == NULL)
    {
    vtkNew<vtkStringArray> newColumnNameArray;
    newColumnNameArray->SetName(SCHEMA_COLUMN_NAME);
    // Number of rows in new column match number of rows in table
    int numberOfRows = this->Schema->GetNumberOfRows();
    newColumnNameArray->SetNumberOfValues(numberOfRows);
    for (int i = 0; i<numberOfRows; i++)
      {
      newColumnNameArray->SetVariantValue(i, "");
      }
    this->Schema->AddColumn(newColumnNameArray.GetPointer());
    columnNameArray = newColumnNameArray.GetPointer();
    }
  // Make sure there is an array for the selected property
  vtkStringArray* propertyArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(propertyName.c_str()));
  if (propertyArray == NULL)
    {
    vtkNew<vtkStringArray> newPropertyArray;
    newPropertyArray->SetName(propertyName.c_str());
    // Number of rows in new column match number of rows in table
    int numberOfRows = this->Schema->GetNumberOfRows();
    newPropertyArray->SetNumberOfValues(numberOfRows);
    for (int i = 0; i<numberOfRows; i++)
      {
        newPropertyArray->SetVariantValue(i, "");
      }
    this->Schema->AddColumn(newPropertyArray.GetPointer());
    propertyArray = newPropertyArray.GetPointer();
    }
  // Make sure there is an entry for the selected column
  vtkIdType rowIndex = this->GetPropertyRowIndex(columnName.c_str());
  if (rowIndex < 0)
    {
    rowIndex = this->Schema->InsertNextBlankRow();
    columnNameArray->SetValue(rowIndex, columnName.c_str());
    }

  // Set value
  if (propertyArray->GetValue(rowIndex) != propertyValue)
    {
    propertyArray->SetValue(rowIndex, propertyValue.c_str());
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::RemoveColumnProperty(const std::string& propertyName)
{
  if (propertyName == SCHEMA_COLUMN_NAME || propertyName == SCHEMA_COLUMN_TYPE)
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveColumnProperty failed: reserved propertyName: " << propertyName);
    return;
    }
  if (this->Schema == NULL)
    {
    return;
    }
  this->Schema->RemoveColumnByName(propertyName.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::RemoveAllColumnProperties(int columnIndex)
{
  if (this->Table == NULL)
    {
    return;
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == NULL)
    {
    return;
    }
  this->RemoveAllColumnProperties(columnName);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::RemoveAllColumnProperties(const std::string& columnName)
{
  if (this->Schema == NULL)
    {
    return;
    }
  int rowIndex = this->GetPropertyRowIndex(columnName);
  if (rowIndex < 0)
    {
    return;
    }
  this->Schema->RemoveRow(rowIndex);
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetColumnValueTypeFromSchema(const std::string& columnName)
{
  std::string valueTypeStr = this->GetColumnPropertyInternal(columnName, SCHEMA_COLUMN_TYPE);
  if (valueTypeStr.empty())
    {
    return VTK_VOID;
    }
  int valueType = this->GetValueTypeFromString(valueTypeStr);
  if (valueType == VTK_VOID)
    {
    vtkErrorMacro("Unknown column value type: " << valueTypeStr << " using string instead. Supported types: string, double, float, int, unsigned int, bit,"
      ", short, unsigned short, long, unsigned long, char, signed char, unsigned char, long long, unsigned long long"
      ", __int64, unsigned __int64, idtype");
    }
  return valueType;
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnLongName(const std::string& columnName, const std::string& longName)
{
  this->SetColumnProperty(columnName, SCHEMA_COLUMN_LONG_NAME, longName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnLongName(const std::string& columnName)
{
  return this->GetColumnProperty(columnName, SCHEMA_COLUMN_LONG_NAME);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnDescription(const std::string& columnName, const std::string& description)
{
  this->SetColumnProperty(columnName, SCHEMA_COLUMN_DESCRIPTION, description);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnDescription(const std::string& columnName)
{
  return this->GetColumnProperty(columnName, SCHEMA_COLUMN_DESCRIPTION);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnDefaultValue(const std::string& columnName, const std::string& description)
{
  this->SetColumnProperty(columnName, SCHEMA_COLUMN_DEFAULT_VALUE, description);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnDefaultValue(const std::string& columnName)
{
  return this->GetColumnProperty(columnName, SCHEMA_COLUMN_DEFAULT_VALUE);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnUnitLabel(const std::string& columnName, const std::string& unitLabel)
{
  this->SetColumnProperty(columnName, SCHEMA_COLUMN_UNIT_LABEL, unitLabel);
}

//----------------------------------------------------------------------------
std::string  vtkMRMLTableNode::GetColumnUnitLabel(const std::string& columnName)
{
  return this->GetColumnProperty(columnName, SCHEMA_COLUMN_UNIT_LABEL);
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetValueTypeFromString(std::string valueTypeStr)
{
  valueTypeStr = vtksys::SystemTools::LowerCase(valueTypeStr);
  const int MAX_TYPE_ID = 50; // currently there are less than 30 types, it is not likely there will be ever more than this
  for (int typeId = 0; typeId < MAX_TYPE_ID; typeId++)
    {
    if (valueTypeStr == vtkImageScalarTypeNameMacro(typeId))
      {
      return typeId;
      }
    }
  // not found
  return VTK_VOID;
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLTableNode::InsertNextBlankRowWithDefaultValues(vtkTable* table)
{
  if (table == NULL)
    {
    vtkErrorMacro("vtkMRMLTableNode::InsertNextBlankRowWithDefaultValues failed: invalid table");
    return -1;
    }

  vtkIdType rowIndex = table->InsertNextBlankRow();

  for (int columnIndex = 0; columnIndex < table->GetNumberOfColumns(); ++columnIndex)
    {
    vtkAbstractArray* column = table->GetColumn(columnIndex);
    if (column == NULL)
      {
      continue;
      }
    vtkVariant defaultValue(this->GetColumnProperty(columnIndex, "defaultValue"));
    column->SetVariantValue(rowIndex, defaultValue);
    }

  return rowIndex;
}
