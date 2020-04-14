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

#ifndef __vtkMRMLTableStorageNode_h
#define __vtkMRMLTableStorageNode_h

#include "vtkMRMLStorageNode.h"

class vtkMRMLTableNode;
class vtkTable;

/// \brief MRML node for handling Table node storage
///
/// vtkMRMLTableStorageNode allows reading/writing of table node from
/// to comma or tab-separated files.
///
/// If the file extension is .tsv or .txt then it is assumed to be tab-separated.
/// Values in tab-separated files may not contain tabs but may contain any other
/// characters (including commas and quotation marks).
///
/// If the file extension is .csv then it is assumed to be comma-separated.
/// Values in comma-separated files may not contain quotation marks but may contain
/// any other characters (including commas and tabs).
///
class VTK_MRML_EXPORT vtkMRMLTableStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLTableStorageNode *New();
  vtkTypeMacro(vtkMRMLTableStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "TableStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

  /// Get/Set schema file name, which contain description of data type of each column
  virtual void SetSchemaFileName(const char* schemaFileName);
  virtual std::string GetSchemaFileName();

  /// Finds schema file corresponding to a table file.
  std::string FindSchemaFileName(const char* fileName);

  /// If enabled and schema filename is not specified then when the data is read,
  /// an attempt will be made to find and load a schema file.
  vtkSetMacro(AutoFindSchema, bool);
  vtkGetMacro(AutoFindSchema, bool);
  vtkBooleanMacro(AutoFindSchema, bool);

protected:
  vtkMRMLTableStorageNode();
  ~vtkMRMLTableStorageNode() override;
  vtkMRMLTableStorageNode(const vtkMRMLTableStorageNode&);
  void operator=(const vtkMRMLTableStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node. Returns 0 on failure.
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node. Returns 0 on failure.
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  std::string GenerateSchemaFileName(const char* fileName);

  virtual std::string GetFieldDelimiterCharacters(std::string filename);

  // Struct for managing column information
  struct StructColumnInfo
  {
    std::string ColumnName;
    std::vector<vtkAbstractArray*> RawComponentArrays;
    int ScalarType = VTK_STRING;
    std::vector<std::string> ComponentNames;
    std::string NullValueString;
  };
  using ColumnInfo = struct StructColumnInfo;

  /// Determines information about the columns in the table, including column name,
  /// The raw components that should be includeded in the table, the scalar type,
  /// and the names of the components.
  std::vector<ColumnInfo> GetColumnInfo(vtkMRMLTableNode* tableNode, vtkTable* rawTable);

  /// Casts the data in the string array to the correct type and stores it in the data array
  void FillDataFromStringArray(vtkStringArray* stringComponentArray, vtkDataArray* dataArray, std::string nullValueString="");

  /// Adds the column specified by the given columnInfo to the table.
  /// Handles both single and multi-component columns
  void AddColumnToTable(vtkTable* table, ColumnInfo columnInfo);

  bool ReadSchema(std::string filename, vtkMRMLTableNode* tableNode);
  bool ReadTable(std::string filename, vtkMRMLTableNode* tableNode);

  bool WriteTable(std::string filename, vtkMRMLTableNode* tableNode);
  bool WriteSchema(std::string filename, vtkMRMLTableNode* tableNode);

  bool AutoFindSchema;
};

#endif
