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

#ifndef __vtkMRMLTableSQLightStorageNode_h
#define __vtkMRMLTableSQLightStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for handling Table node storage
///
/// vtkMRMLTableSQLiteStorageNode allows reading/writing of table node from
/// SQLight database.
///
///

class vtkSQLiteDatabase;

class VTK_MRML_EXPORT vtkMRMLTableSQLiteStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLTableSQLiteStorageNode *New();
  vtkTypeMacro(vtkMRMLTableSQLiteStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "TableSQLightStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

  /// SQLight Database password
  vtkSetStringMacro(Password);
  vtkGetStringMacro(Password);

  /// SQLight Database table name
  vtkSetStringMacro(TableName);
  vtkGetStringMacro(TableName);

  /// Drop a specified table from the database
  static int DropTable(char *tableName, vtkSQLiteDatabase* database);

protected:
  vtkMRMLTableSQLiteStorageNode();
  ~vtkMRMLTableSQLiteStorageNode() override;
  vtkMRMLTableSQLiteStorageNode(const vtkMRMLTableSQLiteStorageNode&);
  void operator=(const vtkMRMLTableSQLiteStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node. Returns 0 on failure.
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node. Returns 0 on failure.
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  char *TableName;
  char *Password;
};

#endif
