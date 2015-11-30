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
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableNode);

//----------------------------------------------------------------------------
vtkMRMLTableNode::vtkMRMLTableNode()
{
  this->Table = NULL;
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
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
  of << indent << " locked=\"" << (this->GetLocked() ? "true" : "false") << "\"";
  of << indent << " useFirstColumnAsRowHeader=\"" << (this->GetUseFirstColumnAsRowHeader() ? "true" : "false") << "\"";
  of << indent << " useColumnNameAsColumnHeader=\"" << (this->GetUseColumnNameAsColumnHeader() ? "true" : "false") << "\"";
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
  else
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

  if (this->Table && this->Table == vtkTable::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
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
  os << indent << "\nTable Data:";
  if (this->GetTable())
    {
    os << "\n";
    this->GetTable()->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "none\n";
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
          this->Table->InsertNextBlankRow();
          }
        }
      else if (numberOfColumnsToAddToTable<0)
        {
        // Need to add more items to the array to match the table size.
        // To make sure that augmentation of the array is consistent, we create a dummy vtkTable
        // and use its InsertNextBlankRow() method.
        vtkNew<vtkTable> augmentingTable;
        augmentingTable->AddColumn(newColumn);
        int numberOfColumnsToAddToArray = -numberOfColumnsToAddToTable;
        for (int i=0; i<numberOfColumnsToAddToArray; i++)
          {
          augmentingTable->InsertNextBlankRow();
          }
        }
      }
    }
  else
    {
    int numberOfRows = this->Table->GetNumberOfRows();
    newColumn = vtkSmartPointer<vtkStringArray>::New();
    newColumn->SetNumberOfTuples(numberOfRows);
    vtkVariant emptyCell("");
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
  this->Table->RemoveColumn(columnIndex);
  this->Table->Modified();
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
  vtkIdType rowIndex = this->Table->InsertNextBlankRow();
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
