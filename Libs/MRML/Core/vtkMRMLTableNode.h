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

#ifndef __vtkMRMLTableNode_h
#define __vtkMRMLTableNode_h

#include <string>
#include <vector>

#include "vtkMRMLStorableNode.h"

// MRML Includes
class vtkMRMLStorageNode;

// VTK Includes
class vtkTable;

/// \brief MRML node to represent a table object
///
/// This class can store a spreadsheet. Data is stored as a vtkTable object.
/// A few convenience methods are provided for manipulating rows, columns, and cells.
/// For performance-critical use cases it is recommended to manipulate the vtkTable
/// object directly. If the vtkTable object content is modified then Modified() method
/// has to be called on the vtkTable object when modifications are completed.
class VTK_MRML_EXPORT vtkMRMLTableNode : public vtkMRMLStorableNode
{
public:
  /// Data types supported by the table. Used in qMRMLTableModel for visualization.

public:
  static vtkMRMLTableNode *New();
  vtkTypeMacro(vtkMRMLTableNode,vtkMRMLStorableNode);

  void PrintSelf(ostream& os, vtkIndent indent);

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() { return "Table"; };

  ///
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------
  virtual void SetAndObserveTable(vtkTable* table);
  vtkGetObjectMacro(Table, vtkTable);

  ///
  /// Set schema table
  ///
  /// Each row of the schema table contains description of a data table column. Columns of the schema table:
  /// - columnName: name of the data table column that properties are defined for (required)
  /// - type: data type of the column. Supported types: string, double, float, int, unsigned int, bit,
  ///   short, unsigned short, long, unsigned long, char, signed char, unsigned char, long long, unsigned long long
  ///   __int64, unsigned __int64, idtype. Default: string.
  /// - defaultValue: value to be used when a value is not specified
  /// - longName: full human-readable name of the column
  /// - description: human-readable detailed description of the column
  /// - unitLabel: simple unit label
  /// - unitCodeMeaning: standard unit definition. Example: Standardized Uptake Value body weight.
  /// - unitCodeValue: standard unit definition. Example: {SUVbw}g/ml.
  /// - unitCodingSchemeDesignator: standard unit definition. Example: UCUM.
  virtual void SetAndObserveSchema(vtkTable* schema);
  vtkGetObjectMacro(Schema, vtkTable);

  ///
  /// Table contents cannot be edited through the user interface
  vtkGetMacro(Locked, bool);
  vtkSetMacro(Locked, bool);

  /// First column should be treated as row label
  vtkGetMacro(UseFirstColumnAsRowHeader, bool);
  vtkSetMacro(UseFirstColumnAsRowHeader, bool);

  ///
  /// Column name should be treated as column label
  vtkGetMacro(UseColumnNameAsColumnHeader, bool);
  vtkSetMacro(UseColumnNameAsColumnHeader, bool);

  ///
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  ///
  /// Add an array to the table as a new column.
  /// If no column is provided then an empty column is added.
  /// If a column is provided then the number of rows in the table and tuples in
  /// the provided array will be matched by either adding empty rows to the table
  /// or empty elements to the array.
  /// If a column is provided that does not have a name then a name will be generated
  /// automatically that is unique among all table column names.
  vtkAbstractArray* AddColumn(vtkAbstractArray* column = 0);

  ///
  /// Rename an array in the table (including associated properties).
  /// If a column by that name already exists then column properties of the existing column
  /// will be overwritten with the renamed column's properties.
  /// Returns with true on success.
  bool RenameColumn(int columnIndex, const char* newName);

  ///
  /// Remove array from the table (including associated properties).
  /// Returns with true on success.
  bool RemoveColumn(int columnIndex);

  ///
  /// Remove all columns from the table (including associated properties).
  /// Returns with true on success.
  bool RemoveAllColumns();
  
  ///
  /// Add an empty row at the end of the table
  /// Returns the index of the inserted row or -1 on failure.
  int AddEmptyRow();

  ///
  /// Remove row from the table
  /// Returns with true on success.
  bool RemoveRow(int rowIndex);

  ///
  /// Convenience method for getting a single value in the table.
  /// Returns empty string if failed to get value. Returns the value on success.
  std::string GetCellText(int rowIndex, int columnIndex);

  ///
  /// Convenience method for setting a single value in the table.
  /// Returns true if the setting was successful.
  /// This updates the node immediately, therefore it is not efficient when a large number
  /// of values has to be modified. For bulk operations get the vtkTable object using
  /// GetTable() method and manipulate that directly.
  bool SetCellText(int rowIndex, int columnIndex, const char* text);

  ///
  /// Get column index of the first column by the specified name.
  /// Returns -1 if no such column is found.
  int GetColumnIndex(const char* columnName);

  ///
  /// Get column index from column pointer.
  /// Returns -1 if column is not found.
  int GetColumnIndex(vtkAbstractArray* column);

  ///
  /// Get column name from column index.
  /// Returns empty string if column is not found.
  std::string GetColumnName(int columnIndex);

  ///
  /// Convenience method for getting number of rows in the table.
  int GetNumberOfRows();

  ///
  /// Convenience method for getting number of columns in the table.
  int GetNumberOfColumns();

  ///
  /// Set default value for blank rows and missing non-string values.
  void SetColumnDefaultValue(const std::string& columnName, const std::string& defaultValue);
  std::string GetColumnDefaultValue(const std::string& columnName);

  ///
  /// Set a full human-readable name of a column.
  /// When there is no space constraings, the full name of the column may` displayed
  /// instead/in addition to columnName to identify a column.
  void SetColumnLongName(const std::string& columnName, const std::string& description);
  std::string GetColumnLongName(const std::string& columnName);

  ///
  /// Set human-readable description of a column.
  void SetColumnDescription(const std::string& columnName, const std::string& description);
  std::string GetColumnDescription(const std::string& columnName);

  ///
  /// Set measurement unit for the data stored in the selected column.
  void SetColumnUnitLabel(const std::string& columnName, const std::string& unitLabel);
  std::string GetColumnUnitLabel(const std::string& columnName);

  ///
  /// Get a column property.
  /// Property names reserved for internal use: columnName, type.
  /// \sa SetAndObserveSchema, GetColumnValueTypeFromSchema
  std::string GetColumnProperty(const std::string& columnName, const std::string& propertyName);
  std::string GetColumnProperty(int columnIndex, const std::string& propertyName);

  ///
  /// Get list of all column property names.
  /// \sa SetAndObserveSchema
  void GetAllColumnPropertyNames(vtkStringArray* propertyNames);

  ///
  /// Set a column property value.
  /// Property names reserved for internal use: columnName, type.
  /// \sa SetAndObserveSchema, GetColumnValueTypeFromSchema
  void SetColumnProperty(const std::string& columnName, const std::string& propertyName, const std::string& propertyValue);
  void SetColumnProperty(int columnIndex, const std::string& propertyName, const std::string& propertyValue);

  ///
  /// Set a column property value.
  /// Property names reserved for internal use: columnName, type.
  /// \sa SetAndObserveSchema()
  void RemoveColumnProperty(const std::string& propertyName);

  ///
  /// Copy all properties from one column to another.
  void CopyAllColumnProperties(const std::string& sourceColumnName, const std::string& targetColumnName);

  ///
  /// Remove all properties defined for the specified column.
  /// Te remove all properties for all columns, use SetAndObserveScheme(NULL).
  void RemoveAllColumnProperties(const std::string& columnName);
  void RemoveAllColumnProperties(int columnIndex);

  /// Get column type stored in the schema as VTK type ID. It should only be used during reading/writing of the node,
  /// because once the table column is created, the actual column type is the type of the associated VTK data array.
  int GetColumnValueTypeFromSchema(const std::string& columnName);

  /// Insert next blank row into the table, using default values defined in the schema.
  /// \return row index of the inserted row
  vtkIdType InsertNextBlankRowWithDefaultValues(vtkTable* table);

  //----------------------------------------------------------------
  /// Constructor and destructor
  //----------------------------------------------------------------
 protected:
  vtkMRMLTableNode();
  ~vtkMRMLTableNode();
  vtkMRMLTableNode(const vtkMRMLTableNode&);
  void operator=(const vtkMRMLTableNode&);


 protected:

  /// Get column property, even for reserved properties
  std::string GetColumnPropertyInternal(const std::string& columnName, const std::string& propertyName);

  vtkIdType GetPropertyRowIndex(const std::string& columnName);

  /// Get value type id from string (inverse of vtkImageScalarTypeNameMacro)
  int GetValueTypeFromString(std::string valueTypeStr);

  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------

  vtkTable* Table;
  bool Locked;
  bool UseColumnNameAsColumnHeader;
  bool UseFirstColumnAsRowHeader;

  vtkTable* Schema;

};

#endif
