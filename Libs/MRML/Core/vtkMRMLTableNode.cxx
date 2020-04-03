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
#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"

// VTK includes
#include <vtkBitArray.h>
#include <vtkCharArray.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSignedCharArray.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkUnsignedCharArray.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <deque>
#include <sstream>
#include <string>

// Reserved property names
static const char SCHEMA_COLUMN_NAME[] = "columnName";
static const char SCHEMA_COLUMN_TYPE[] = "type";
static const char SCHEMA_COLUMN_NULL_VALUE[] = "nullValue";
static const char SCHEMA_COLUMN_LONG_NAME[] = "longName";
static const char SCHEMA_COLUMN_DESCRIPTION[] = "description";
static const char SCHEMA_COLUMN_UNIT_LABEL[] = "unitLabel";
static const char SCHEMA_COMPONENT_NAMES[] = "componentNames";

static const char COMPONENT_COUNT_PROPERTY[] = "componentCount";

static const char SCHEMA_DEFAULT_COLUMN_NAME[] = "<default>";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableNode);

//----------------------------------------------------------------------------
vtkMRMLTableNode::vtkMRMLTableNode()
{
  this->Table = nullptr;
  this->Schema = nullptr;
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
    this->Table = nullptr;
    }
  if (this->Schema)
    {
    this->Schema->Delete();
    this->Schema = nullptr;
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
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "locked"))
      {
      this->SetLocked(strcmp(attValue,"true")?false:true);
      }
    else if (!strcmp(attName, "useColumnNameAsColumnHeader"))
      {
      this->SetUseColumnNameAsColumnHeader(strcmp(attValue,"true")?false:true);
      }
    else if (!strcmp(attName, "useFirstColumnAsRowHeader"))
      {
      this->SetUseFirstColumnAsRowHeader(strcmp(attValue,"true")?false:true);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLTableNode* node = vtkMRMLTableNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  // TODO: implement shallow-copy for faster copying of large tables
  // Schema
  if (this->GetSchema()!=nullptr && node->GetSchema()==nullptr)
    {
    this->SetAndObserveSchema(nullptr);
    }
  else if (this->GetSchema() == nullptr && node->GetSchema() != nullptr)
    {
    vtkNew<vtkTable> newTable;
    newTable->DeepCopy(node->GetSchema());
    this->SetAndObserveSchema(newTable.GetPointer());
    }
  else if (this->GetSchema() != nullptr && node->GetSchema() != nullptr)
    {
    this->GetSchema()->DeepCopy(node->GetSchema());
    this->Schema->Modified();
    }
  // Table
  if (this->GetTable()!=nullptr && node->GetTable()==nullptr)
    {
    this->SetAndObserveTable(nullptr);
    }
  else if (this->GetTable()==nullptr && node->GetTable()!=nullptr)
    {
    vtkNew<vtkTable> newTable;
    newTable->DeepCopy(node->GetTable());
    this->SetAndObserveTable(newTable.GetPointer());
    }
  else if(this->GetTable() != nullptr && node->GetTable() != nullptr)
    {
    this->GetTable()->DeepCopy(node->GetTable());
    this->Table->Modified();
    }
  this->SetLocked(node->GetLocked());
  this->SetUseColumnNameAsColumnHeader(node->GetUseColumnNameAsColumnHeader());
  this->SetUseFirstColumnAsRowHeader(node->GetUseFirstColumnAsRowHeader());
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  vtkTable* callerTable = vtkTable::SafeDownCast(caller);
  if (event == vtkCommand::ModifiedEvent &&  callerTable != nullptr
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
      if (column == nullptr)
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
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLTableStorageNode"));
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
      return nullptr;
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
          this->InsertNextBlankRowWithNullValues(this->Table);
          }
        }
      else if (numberOfColumnsToAddToTable<0)
        {
        // Need to add more items to the array to match the table size.
        // To make sure that augmentation of the array is consistent, we create a dummy vtkTable
        // and use InsertNextBlankRowWithNullValues() method.
        vtkNew<vtkTable> augmentingTable;
        augmentingTable->AddColumn(newColumn);
        int numberOfColumnsToAddToArray = -numberOfColumnsToAddToTable;
        for (int i=0; i<numberOfColumnsToAddToArray; i++)
          {
          this->InsertNextBlankRowWithNullValues(augmentingTable.GetPointer());
          }
        }
      }
    }
  else
    {
    int numberOfRows = this->Table->GetNumberOfRows();
    int valueTypeId = this->GetColumnValueTypeFromSchema(SCHEMA_DEFAULT_COLUMN_NAME);
    if (valueTypeId == VTK_VOID)
      {
      // schema is not defined or no valid column type is defined for column
      valueTypeId = VTK_STRING;
      }
    newColumn = vtkSmartPointer<vtkAbstractArray>::Take(vtkAbstractArray::CreateArray(valueTypeId));
    newColumn->SetNumberOfTuples(numberOfRows);

    vtkVariant emptyCell(this->GetColumnProperty(SCHEMA_DEFAULT_COLUMN_NAME, SCHEMA_COLUMN_NULL_VALUE));
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
    while (this->Table->GetColumnByName(newColumnName.c_str())!=nullptr);
    newColumn->SetName(newColumnName.c_str());
    }

  // Copy null value and other column properties
  if (this->Schema && this->GetPropertyRowIndex(SCHEMA_DEFAULT_COLUMN_NAME)>=0)
    {
    this->CopyAllColumnProperties(SCHEMA_DEFAULT_COLUMN_NAME, newColumn->GetName());
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
int vtkMRMLTableNode::GetColumnIndex(const std::string &columnName)
{
  return this->GetColumnIndex(columnName.c_str());
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetColumnIndex(vtkAbstractArray* column)
{
  if (column == nullptr)
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
  if (this->Table == nullptr)
    {
    return "";
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == nullptr)
    {
    return "";
    }
  return columnName;
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::CopyAllColumnProperties(const std::string& sourceColumnName, const std::string& targetColumnName)
{
  if (this->Schema == nullptr)
    {
    vtkWarningMacro("vtkMRMLTableNode::CopyAllColumnProperties failed: schema is empty");
    return;
    }
  vtkIdType rowIndexTarget = this->GetPropertyRowIndex(targetColumnName);
  if (rowIndexTarget < 0)
    {
    vtkStringArray* nameColumn = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(SCHEMA_COLUMN_NAME));
    if (nameColumn == nullptr)
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
    if (column == nullptr || column->GetName() == nullptr // invalid column
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
  if (this->Schema != nullptr)
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
  if (this->Schema != nullptr)
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
  this->SetAndObserveSchema(nullptr);
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
  vtkIdType rowIndex = this->InsertNextBlankRowWithNullValues(this->Table);
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
  if (this->Schema == nullptr)
    {
    return -1;
    }
  vtkStringArray* columnNameArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(SCHEMA_COLUMN_NAME));
  if (columnNameArray == nullptr)
    {
    return -1;
    }
  return columnNameArray->LookupValue(columnName);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::GetAllColumnPropertyNames(vtkStringArray* propertyNames)
{
  if (propertyNames == nullptr)
    {
    vtkErrorMacro("vtkMRMLTableNode::GetAllColumnPropertyNames failed: invalid propertyNames");
    return;
    }

  propertyNames->Initialize();
  if (this->Schema == nullptr)
    {
    return;
    }
  int numberOfSchemaColumns = this->Schema->GetNumberOfColumns();
  for (int schemaColumnIndex = 0; schemaColumnIndex < numberOfSchemaColumns; schemaColumnIndex++)
    {
    vtkAbstractArray* column = this->Schema->GetColumn(schemaColumnIndex);
    if (column == nullptr || column->GetName() == nullptr)
      {
      // invalid column
      continue;
      }
    std::string columnName = column->GetName();
    if (!columnName.compare(SCHEMA_COLUMN_NAME))
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
  if (this->Table == nullptr)
    {
    return "";
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == nullptr)
    {
    return "";
    }
  return this->GetColumnProperty(columnName, propertyName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnProperty(const std::string& columnName, const std::string& propertyName)
{
  if (propertyName == SCHEMA_COLUMN_NAME)
    {
    vtkErrorMacro("vtkMRMLTableNode::GetColumnProperty failed: reserved propertyName: " << propertyName);
    return "";
    }
  if (propertyName == SCHEMA_COLUMN_TYPE)
    {
    return vtkMRMLTableNode::GetValueTypeAsString(this->GetColumnType(columnName));
    }
  if (propertyName == SCHEMA_COMPONENT_NAMES)
    {
    return vtkMRMLTableNode::GetComponentNamesAsString(this->GetComponentNames(columnName));
    }
  if (propertyName == COMPONENT_COUNT_PROPERTY)
    {
    int numberOfComponents = 0;
    if (this->Table && this->Table->GetColumnByName(columnName.c_str()))
      {
      numberOfComponents = this->Table->GetColumnByName(columnName.c_str())->GetNumberOfComponents();
      }
    return vtkVariant(numberOfComponents).ToString();
    }

  return this->GetColumnPropertyInternal(columnName, propertyName);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnPropertyInternal(const std::string& columnName, const std::string& propertyName)
{
  if (this->Schema == nullptr)
    {
    return "";
    }

  vtkStringArray* propertyArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(propertyName.c_str()));
  if (propertyArray == nullptr)
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
  if (this->Table == nullptr)
    {
    return;
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == nullptr)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed: invalid column index");
    return;
    }
  this->SetColumnProperty(columnName, propertyName, propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnProperty(const std::string& columnName, const std::string& propertyName, const std::string& propertyValue)
{
  if (propertyName == SCHEMA_COLUMN_NAME)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed: reserved propertyName: " << propertyName);
    return;
    }
  if (propertyName == SCHEMA_COLUMN_TYPE)
    {
    this->SetColumnType(columnName, vtkMRMLTableNode::GetValueTypeFromString(propertyValue));
    return;
    }
  if (propertyName == SCHEMA_COMPONENT_NAMES)
    {
    this->SetComponentNames(columnName, this->GetComponentNamesFromString(propertyValue));
    }
  if (propertyName == COMPONENT_COUNT_PROPERTY)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed : property is read-only " << COMPONENT_COUNT_PROPERTY);
    return;
    }

  if (propertyName.empty())
    {
    vtkErrorMacro("vtkMRMLTableNode::SetColumnProperty failed: property name is invalid");
    return;
    }
  this->SetColumnPropertyInternal(columnName, propertyName, propertyValue);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::SetColumnPropertyInternal(const std::string& columnName, const std::string& propertyName, const std::string& propertyValue)
{
  // Make sure there is a schema
  if (this->Schema == nullptr)
    {
    vtkNew<vtkTable> newSchema;
    this->SetAndObserveSchema(newSchema.GetPointer());
    }
  // Make sure there is a column array
  vtkStringArray* columnNameArray = vtkStringArray::SafeDownCast(this->Schema->GetColumnByName(SCHEMA_COLUMN_NAME));
  if (columnNameArray == nullptr)
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
  if (propertyArray == nullptr)
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
  if (propertyName == SCHEMA_COLUMN_NAME || propertyName == SCHEMA_COLUMN_TYPE
    || propertyName == SCHEMA_COMPONENT_NAMES || propertyName == COMPONENT_COUNT_PROPERTY)
    {
    vtkErrorMacro("vtkMRMLTableNode::RemoveColumnProperty failed: reserved propertyName: " << propertyName);
    return;
    }
  if (this->Schema == nullptr)
    {
    return;
    }
  this->Schema->RemoveColumnByName(propertyName.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::RemoveAllColumnProperties(int columnIndex)
{
  if (this->Table == nullptr)
    {
    return;
    }
  const char* columnName = this->Table->GetColumnName(columnIndex);
  if (columnName == nullptr)
    {
    return;
    }
  this->RemoveAllColumnProperties(columnName);
}

//----------------------------------------------------------------------------
void vtkMRMLTableNode::RemoveAllColumnProperties(const std::string& columnName)
{
  if (this->Schema == nullptr)
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
void vtkMRMLTableNode::SetColumnNullValue(const std::string& columnName, const std::string& description)
{
  this->SetColumnProperty(columnName, SCHEMA_COLUMN_NULL_VALUE, description);
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetColumnNullValue(const std::string& columnName)
{
  return this->GetColumnProperty(columnName, SCHEMA_COLUMN_NULL_VALUE);
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
    if (valueTypeStr == vtkMRMLTableNode::GetValueTypeAsString(typeId))
      {
      return typeId;
      }
    }
  // not found
  return VTK_VOID;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetValueTypeAsString(int valueType)
{
  return vtkImageScalarTypeNameMacro(valueType);
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLTableNode::InsertNextBlankRowWithNullValues(vtkTable* table)
{
  if (table == nullptr)
    {
    vtkErrorMacro("vtkMRMLTableNode::InsertNextBlankRowWithNullValues failed: invalid table");
    return -1;
    }

  vtkIdType rowIndex = table->InsertNextBlankRow();

  for (int columnIndex = 0; columnIndex < table->GetNumberOfColumns(); ++columnIndex)
    {
    vtkAbstractArray* column = table->GetColumn(columnIndex);
    if (column == nullptr)
      {
      continue;
      }
    vtkVariant nullValue(this->GetColumnProperty(columnIndex, "nullValue"));
    column->SetVariantValue(rowIndex, nullValue);
    }

  return rowIndex;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTableNode::GetDefaultColumnName()
{
  return SCHEMA_DEFAULT_COLUMN_NAME;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::SetDefaultColumnType(const std::string& type, const std::string& nullValue /* ="" */)
{
  int valueType = this->GetValueTypeFromString(type);
  if (valueType == VTK_VOID)
    {
    vtkErrorMacro("vtkMRMLTableNode::SetDefaultColumnType failed: Unknown column value type: " << type
      << ". Supported types: string, double, float, int, unsigned int, bit,"
      ", short, unsigned short, long, unsigned long, char, signed char, unsigned char, long long, unsigned long long"
      ", __int64, unsigned __int64, idtype");
    return false;
    }
  this->SetColumnProperty(SCHEMA_DEFAULT_COLUMN_NAME, SCHEMA_COLUMN_TYPE, type);
  this->SetColumnProperty(SCHEMA_DEFAULT_COLUMN_NAME, SCHEMA_COLUMN_NULL_VALUE, nullValue);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::SetColumnType(const std::string& columnName, int valueType)
{
  int columnIndex = this->GetColumnIndex(columnName.c_str());
  if (columnIndex < 0)
    {
    // there is no such column, it can be only the default column type
    if (columnName != SCHEMA_DEFAULT_COLUMN_NAME)
      {
      vtkErrorMacro("vtkMRMLTableNode::SetColumnType failed: column by this name does not exist: " << columnName);
      return false;
      }
    this->SetColumnPropertyInternal(SCHEMA_DEFAULT_COLUMN_NAME, SCHEMA_COLUMN_TYPE, vtkMRMLTableNode::GetValueTypeAsString(valueType));
    return true;
    }

  if (this->GetColumnType(columnName) == valueType)
    {
    // nothing to do, type not changed
    return true;
    }

  vtkTable* table = this->GetTable();

  vtkSmartPointer<vtkAbstractArray> oldColumn = table->GetColumn(columnIndex);
  vtkSmartPointer<vtkAbstractArray> newColumn = vtkSmartPointer<vtkAbstractArray>::Take(vtkAbstractArray::CreateArray(valueType));
  newColumn->SetName(oldColumn->GetName());
  vtkIdType numberOfTuples = oldColumn->GetNumberOfTuples();
  newColumn->SetNumberOfTuples(numberOfTuples);
  vtkVariant nullValue(this->GetColumnProperty(columnIndex, "nullValue"));

  vtkDataArray* newDataArray = vtkDataArray::SafeDownCast(newColumn);
  if (newDataArray)
    {
    // Initialize it with 0, just in case conversion from null value fails
    // (for example if null value is "abc" and new type is double, then
    // some new column values could remain uninitialized).
    newDataArray->FillComponent(0, 0);
    }

  vtkBitArray* oldColumnBitArray = vtkBitArray::SafeDownCast(oldColumn);
  vtkCharArray* oldColumnCharArray = vtkCharArray::SafeDownCast(oldColumn);
  vtkSignedCharArray* oldColumnSignedCharArray = vtkSignedCharArray::SafeDownCast(oldColumn);
  vtkUnsignedCharArray* oldColumnUnsignedCharArray = vtkUnsignedCharArray::SafeDownCast(oldColumn);
  vtkCharArray* newColumnCharArray = vtkCharArray::SafeDownCast(newColumn);
  vtkSignedCharArray* newColumnSignedCharArray = vtkSignedCharArray::SafeDownCast(newColumn);
  vtkUnsignedCharArray* newColumnUnsignedCharArray = vtkUnsignedCharArray::SafeDownCast(newColumn);
  if (oldColumnBitArray)
    {
    // vtkVariant cannot convert values from VTK_BIT type, therefore we need to handle it
    // as a special case here.
    for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
      {
      int value = oldColumnBitArray->GetValue(tupleIndex);
      newColumn->SetVariantValue(tupleIndex, vtkVariant(value));
      }
    }
  else if (oldColumnCharArray || oldColumnSignedCharArray || oldColumnUnsignedCharArray)
    {
    // char may be converted as a single-character string
    for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
      {
      int value = oldColumn->GetVariantValue(tupleIndex).ToInt();
      newColumn->SetVariantValue(tupleIndex, vtkVariant(value));
      }
    }
  else if (newColumnCharArray || newColumnSignedCharArray || newColumnUnsignedCharArray)
    {
    // char may be converted as a single-character string
    for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
      {
      int value = oldColumn->GetVariantValue(tupleIndex).ToInt();
      newColumn->SetVariantValue(tupleIndex, vtkVariant(value));
      }
    }
  else
    {
    for (int tupleIndex = 0; tupleIndex < numberOfTuples; ++tupleIndex)
      {
      // Initialize it with null value, just in case conversion from previous value fails
      newColumn->SetVariantValue(tupleIndex, nullValue);
      // Copy converted value
      std::string valueAsString = oldColumn->GetVariantValue(tupleIndex).ToString();
      vtkVariant value(valueAsString);
      newColumn->SetVariantValue(tupleIndex, value);
      }
    }

  // Replace column: vtkTable API has no way of inserting a column, so we temporarily remove all columns
  // after the selected column and put back in order.
  int wasModified = this->StartModify();
  std::deque< vtkSmartPointer< vtkAbstractArray > > removedColumns;
  while (table->GetNumberOfColumns() > columnIndex)
    {
    removedColumns.emplace_back(table->GetColumn(table->GetNumberOfColumns()-1));
    table->RemoveColumn(table->GetNumberOfColumns() - 1);
    }
  removedColumns.pop_back(); // discard the last column, that is the old column
  table->AddColumn(newColumn);
  while (!removedColumns.empty())
    {
    table->AddColumn(removedColumns.back());
    removedColumns.pop_back();
    }
  this->Modified();
  this->EndModify(wasModified);

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTableNode::GetColumnType(const std::string& columnName)
{
  if (columnName == SCHEMA_DEFAULT_COLUMN_NAME)
    {
    int valueTypeId = this->GetColumnValueTypeFromSchema(SCHEMA_DEFAULT_COLUMN_NAME);
    if (valueTypeId == VTK_VOID)
      {
      // schema is not defined or no valid column type is defined for column
      valueTypeId = VTK_STRING;
      }
    return valueTypeId;
    }

  int columnIndex = this->GetColumnIndex(columnName.c_str());
  if (columnIndex < 0)
    {
    return VTK_VOID;
    }
  int valueTypeId = this->Table->GetColumn(columnIndex)->GetDataType();
  return valueTypeId;
}

//----------------------------------------------------------------------------
const std::vector<std::string> vtkMRMLTableNode::GetComponentNamesFromString(const std::string& componentNamesString)
{
  std::stringstream ss(componentNamesString);
  std::string componentName;
  std::vector<std::string> componentNames;
  while (std::getline(ss, componentName, '|'))
    {
    componentNames.push_back(componentName);
    }
  return componentNames;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTableNode::GetComponentNamesAsString(const std::vector<std::string>& componentNames)
{
  std::stringstream ss;
  for (size_t componentCount = 0; componentCount != componentNames.size(); ++componentCount)
    {
    if (componentCount != 0)
      {
      ss << "|";
      }
    ss << componentNames[componentCount];
    }
  return ss.str();
}

//----------------------------------------------------------------------------
bool vtkMRMLTableNode::SetComponentNames(const std::string& columnName, const std::vector<std::string>& componentNames)
{
  vtkAbstractArray* column = nullptr;
  if (this->Table && columnName != SCHEMA_DEFAULT_COLUMN_NAME)
    {
    column = this->Table->GetColumnByName(columnName.c_str());
    }

  if (column == nullptr)
    {
    return false;
    }

  if (column->GetNumberOfComponents() != static_cast<int>(componentNames.size()))
    {
    return false;
    }

  int componentIndex = 0;
  for (std::string componentName : componentNames)
    {
    column->SetComponentName(componentIndex, componentName.c_str());
    ++componentIndex;
    }

  return true;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLTableNode::GetComponentNames(const std::string& columnName)
{
  vtkAbstractArray* column = nullptr;
  if (this->Table && columnName != SCHEMA_DEFAULT_COLUMN_NAME)
    {
    column = this->Table->GetColumnByName(columnName.c_str());
    }
  return vtkMRMLTableNode::GetComponentNamesFromArray(column);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLTableNode::GetComponentNamesFromSchema(const std::string& columnName)
{
  return this->GetComponentNamesFromString(this->GetColumnPropertyInternal(columnName, SCHEMA_COLUMN_TYPE));
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLTableNode::GetComponentNamesFromArray(vtkAbstractArray* array)
{
  std::vector<std::string> componentNames;
  if (!array)
    {
    return componentNames;
    }

  if (!vtkDataArray::SafeDownCast(array))
    {
    // We don't handle component names for non vtkDataArray types
    return componentNames;
    }

  if (array->GetNumberOfComponents() <= 1 && !array->HasAComponentName())
    {
    // For single-component array we don't create default component names
    return componentNames;
    }

  // Get component names (and generate missing names automatically)
  for (int componentIndex = 0; componentIndex < array->GetNumberOfComponents(); ++componentIndex)
    {
    std::string componentName;
    if (array->GetComponentName(componentIndex))
      {
      componentName = array->GetComponentName(componentIndex);
      }
    else
      {
      // Default to colum integer name 0, 1, 2, etc.
      std::stringstream ss;
      ss << componentIndex;
      componentName = ss.str();
      }

    // Generate a unique component name
    int i=0;
    std::string tempComponentName = componentName;
    while (std::find(componentNames.begin(), componentNames.end(), componentName) != componentNames.end())
      {
      std::stringstream ss;
      ss << tempComponentName << "_" << i;
      componentName = ss.str();
      i++;
      }

    componentNames.push_back(componentName);
    }
  return componentNames;
}
