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
#include "vtkMRMLTableSQLiteStorageNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkBitArray.h>
#include <vtkNew.h>
#include <vtkSQLQuery.h>
#include <vtkRowQueryToTable.h>
#include <vtkSQLDatabase.h>
#include <vtkSQLiteDatabase.h>
#include <vtkSQLiteQuery.h>
#include <vtkSmartPointer.h>

#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableSQLiteStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTableSQLiteStorageNode::vtkMRMLTableSQLiteStorageNode()
{
  this->TableName = nullptr;
  this->Password = nullptr;
  this->DefaultWriteFileExtension = "sqlite3";
}

//----------------------------------------------------------------------------
vtkMRMLTableSQLiteStorageNode::~vtkMRMLTableSQLiteStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLTableSQLiteStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLTableSQLiteStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLTableNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTableSQLiteStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
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

  std::string dbname = std::string("sqlite://") + fullName;
  vtkSmartPointer<vtkSQLiteDatabase> database = vtkSmartPointer<vtkSQLiteDatabase>::Take(
                   vtkSQLiteDatabase::SafeDownCast( vtkSQLiteDatabase::CreateFromURL(dbname.c_str())));

  if (!database.GetPointer() || !database->Open(this->GetPassword(), vtkSQLiteDatabase::USE_EXISTING))
    {
    vtkErrorMacro("ReadData: database file '" << fullName << "cannot be opened");
    return 0;
    }

  vtkSmartPointer<vtkSQLiteQuery> query = vtkSmartPointer<vtkSQLiteQuery>::Take(
                   vtkSQLiteQuery::SafeDownCast( database->GetQueryInstance()));
  std::string queryString("select * from ");
  queryString += std::string(this->TableName);
  query->SetQuery(queryString.c_str());
  query->Execute();

  vtkSmartPointer<vtkRowQueryToTable> queryToTable = vtkSmartPointer<vtkRowQueryToTable>::New();
  queryToTable->SetQuery(query);
  queryToTable->Update();

  vtkSmartPointer<vtkTable> table = queryToTable->GetOutput();

  tableNode->SetAndObserveTable(table);

  vtkDebugMacro("ReadData: successfully read table from file: " << fullName);

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTableSQLiteStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
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
  if(!this->TableName || std::string(this->TableName).empty())
    {
    vtkErrorMacro(<<"No table name specified!");
    return 0;
    }

  std::string dbname = std::string("sqlite://") + fullName;

  //vtkSmartPointer<vtkSQLiteDatabase> database = vtkSmartPointer<vtkSQLiteDatabase>::Take(vtkSQLiteDatabase::SafeDownCast( vtkSQLiteDatabase::CreateFromURL(dbname.c_str())));
  vtkSQLiteDatabase *database = vtkSQLiteDatabase::SafeDownCast( vtkSQLiteDatabase::CreateFromURL(dbname.c_str()));

  if (!database || !database->Open(this->GetPassword(), vtkSQLiteDatabase::USE_EXISTING_OR_CREATE))
    {
    vtkErrorMacro("ReadData: database file '" << fullName << "cannot be opened");
    return 0;
    }

  vtkTable *table = tableNode->GetTable();
  if (!table)
    {
    vtkErrorMacro("ReadData: no table to write for the node '" << std::string(tableNode->GetName()));
    return 0;
    }

  if (!this->TableName || std::string(this->TableName).empty())
    {
    vtkErrorMacro("ReadData: no table name for the node '" << std::string(tableNode->GetName()));
    return 0;
    }

  // first try to drop the table
  this->DropTable(this->TableName, database);

  //converting this table to SQLite will require two queries: one to create
  //the table, and another to populate its rows with data.
  std::string createTableQuery = "CREATE TABLE IF NOT EXISTS ";
  createTableQuery += this->TableName;
  createTableQuery += "(";

  std::string insertPreamble = "INSERT into ";
  insertPreamble += this->TableName;
  insertPreamble += "(";

  //get the columns from the vtkTable to finish the query
  vtkIdType numColumns = table->GetNumberOfColumns();
  for(vtkIdType i = 0; i < numColumns; i++)
    {
    //get this column's name
    std::string columnName = table->GetColumn(i)->GetName();
    createTableQuery += columnName;
    insertPreamble += "'" + columnName + "'";

    //figure out what type of data is stored in this column
    std::string columnType = table->GetColumn(i)->GetClassName();

    if( (columnType.find("String") != std::string::npos) ||
        (columnType.find("Data") != std::string::npos) ||
        (columnType.find("Variant") != std::string::npos) )
      {
      createTableQuery += " TEXT";
      }
    else if( (columnType.find("Double") != std::string::npos) ||
             (columnType.find("Float") != std::string::npos) )
      {
      createTableQuery += " REAL";
      }
    else
      {
      createTableQuery += " INTEGER";
      }
    if(i == numColumns - 1)
      {
      createTableQuery += ");";
      insertPreamble += ") VALUES (";
      }
    else
      {
      createTableQuery += ", ";
      insertPreamble += ", ";
      }
    }

  //perform the create table query
  vtkSQLiteQuery *query =
    static_cast<vtkSQLiteQuery*>(database->GetQueryInstance());

  query->SetQuery(createTableQuery.c_str());
  cout << "creating the table" << endl;
  if(!query->Execute())
    {
    vtkErrorMacro(<<"Error performing 'create table' query");
    }

  //iterate over the rows of the vtkTable to complete the insert query
  vtkIdType numRows = table->GetNumberOfRows();
  for(vtkIdType i = 0; i < numRows; i++)
    {
    std::string insertQuery = insertPreamble;
    for (vtkIdType j = 0; j < numColumns; j++)
      {
      insertQuery += "'" + table->GetValue(i, j).ToString() + "'";
      if(j < numColumns - 1)
        {
        insertQuery += ", ";
        }
      }
    insertQuery += ");";
    //perform the insert query for this row
    query->SetQuery(insertQuery.c_str());
    if(!query->Execute())
      {
      vtkErrorMacro(<<"Error performing 'insert' query");
      }
    }

  //cleanup and return
  query->Delete();
  database->Close();
  database->Delete();

  vtkDebugMacro("WriteData: successfully wrote table to database: " << fullName);
  return 1;
}

int vtkMRMLTableSQLiteStorageNode::DropTable(char *tableName, vtkSQLiteDatabase* database)
{
  if(!tableName || std::string(tableName).empty())
    {
    std::cerr <<"No table name specified!";
    return 0;
    }

  if (!database)
    {
    std::cerr << "No database specified!";
    return 0;
    }

  vtkStringArray *tables = database->GetTables();
  vtkSmartPointer<vtkSQLiteQuery> query = vtkSmartPointer<vtkSQLiteQuery>::Take(
                       vtkSQLiteQuery::SafeDownCast( database->GetQueryInstance()));

  for (int i=0; i<tables->GetNumberOfValues(); i++)
    {
    if (!tables->GetValue(i).compare(tableName))
      {
      std::string dropTableQuery = "DROP TABLE ";
      dropTableQuery += tableName;
      query->SetQuery(dropTableQuery.c_str());
      query->Execute();
      break;
      }
    }

  //database->Close();
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLTableSQLiteStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("SQLight database (.db)");
  this->SupportedReadFileTypes->InsertNextValue("SQLight database (.db3)");
  this->SupportedReadFileTypes->InsertNextValue("SQLight database (.sqlite)");
  this->SupportedReadFileTypes->InsertNextValue("SQLight database (.sqlite3)");
}

//----------------------------------------------------------------------------
void vtkMRMLTableSQLiteStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("SQLight database (.db)");
  this->SupportedWriteFileTypes->InsertNextValue("SQLight database (.db3)");
  this->SupportedWriteFileTypes->InsertNextValue("SQLight database (.sqlite)");
  this->SupportedWriteFileTypes->InsertNextValue("SQLight database (.sqlite3)");
}
