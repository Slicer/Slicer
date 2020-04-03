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

  void PrintSelf(ostream& os, vtkIndent indent) override;

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLTableNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "Table"; }

  ///
  /// Method to propagate events generated in mrml
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData) override;

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------
  virtual void SetAndObserveTable(vtkTable* table);
  vtkGetObjectMacro(Table, vtkTable);

  ///
  /// Set schema table
  ///
  /// Each row of the schema table contains description of a data table column. Columns of the schema table:
  /// - columnName: name of the data table column that properties are defined for  (required)
  ///   Column name \<default\> is reserved for defining default properties for new columns.
  /// - type: data type of the column. Supported types: string, double, float, int, unsigned int, bit,
  ///   short, unsigned short, long, unsigned long, char, signed char, unsigned char, long long, unsigned long long
  ///   __int64, unsigned __int64, idtype. Default: string.
  /// - nullValue: value to be used when a value is not specified (new table row is added, blank string is entered, etc)
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
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  ///
  /// Add an array to the table as a new column.
  /// If no column is provided then an empty column is added.
  /// If a column is provided then the number of rows in the table and tuples in
  /// the provided array will be matched by either adding empty rows to the table
  /// or empty elements to the array.
  /// If a column is provided that does not have a name then a name will be generated
  /// automatically that is unique among all table column names.
  vtkAbstractArray* AddColumn(vtkAbstractArray* column = nullptr);

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
  int GetColumnIndex(const std::string &columnName);

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
  /// Set null value for blank rows and missing values.
  void SetColumnNullValue(const std::string& columnName, const std::string& nullValue);
  std::string GetColumnNullValue(const std::string& columnName);

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
  /// Property name "columnName" is reserved for internal use.
  /// \sa SetAndObserveSchema, GetColumnValueTypeFromSchema
  std::string GetColumnProperty(const std::string& columnName, const std::string& propertyName);
  std::string GetColumnProperty(int columnIndex, const std::string& propertyName);

  ///
  /// Get list of all column property names.
  /// \sa SetAndObserveSchema
  void GetAllColumnPropertyNames(vtkStringArray* propertyNames);

  ///
  /// Set a column property value.
  /// Property name "columnName" is reserved for internal use.
  /// Property name "type" converts existing values in the column.
  /// \sa SetAndObserveSchema
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
  /// To remove all properties for all columns, use SetAndObserveScheme(nullptr).
  void RemoveAllColumnProperties(const std::string& columnName);
  void RemoveAllColumnProperties(int columnIndex);

  /// Get column type stored in the schema as VTK type ID. It should only be used during reading/writing of the node,
  /// because once the table column is created, the actual column type is the type of the associated VTK data array.
  int GetColumnValueTypeFromSchema(const std::string& columnName);

  /// Insert next blank row into the table, using null values defined in the schema.
  /// \return row index of the inserted row
  vtkIdType InsertNextBlankRowWithNullValues(vtkTable* table);

  /// Get data type of a column.
  /// \return column type ID
  int GetColumnType(const std::string& columnName);

  /// Change data type of a column.
  /// If values are stored in the column then values will be converted, therefore
  /// data loss may occur.
  /// \param type: new column value type ID
  /// \return True on success.
  bool SetColumnType(const std::string& columnName, int typeId);

  /// Set default type and null value for new columns.
  /// This is a convenience method to set column type with a simple call.
  /// To get or set other default properties, call SetColumnProperty()
  /// or GetColumnProperty() using the column name returned by GetDefaultColumnName().
  /// \param type: type of values stored in new columns by default (string, double, ...)
  /// \param nullValue: this value is used when a new row is added to the column
  /// \return True on success.
  /// \sa SetAndObserveSchema
  bool SetDefaultColumnType(const std::string& type, const std::string& nullValue="");

  /// Name of the column that stores default properties that are used when a new column is created.
  static const char* GetDefaultColumnName();

  /// Get value type id from string (inverse of vtkImageScalarTypeNameMacro)
  static int GetValueTypeFromString(std::string valueTypeStr);

  /// Get value type id from string (uses vtkImageScalarTypeNameMacro)
  static std::string GetValueTypeAsString(int valueType);

  /// Set the component names for the specified column
  bool SetComponentNames(const std::string& columnName, const std::vector<std::string>& componentNames);

  /// Get the vector of component names from the
  std::vector<std::string> GetComponentNames(const std::string& columnName);

  /// Get component names stored in the schema. It should only be used during reading/writing of the node,
  /// because once the table column is created, the actual column names are stored in the data array.
  std::vector<std::string> GetComponentNamesFromSchema(const std::string& columnName);

  /// Get component names from a vtkAbstractArray as a vector of string
  static std::vector<std::string> GetComponentNamesFromArray(vtkAbstractArray* array);

  /// Convert the vector of component names from a string of component names separated with the '|' character
  static const std::vector<std::string> GetComponentNamesFromString(const std::string& componentNameString);

  /// Convert the component names as a string of component names separated with the '|' character
  static std::string GetComponentNamesAsString(const std::vector<std::string>& componentNames);

  //----------------------------------------------------------------
  /// Constructor and destructor
  //----------------------------------------------------------------
 protected:
  vtkMRMLTableNode();
  ~vtkMRMLTableNode() override;
  vtkMRMLTableNode(const vtkMRMLTableNode&);
  void operator=(const vtkMRMLTableNode&);

 protected:

  /// Get column property, even for reserved properties
  std::string GetColumnPropertyInternal(const std::string& columnName, const std::string& propertyName);

  /// Set column property, even for reserved properties
  void SetColumnPropertyInternal(const std::string& columnName, const std::string& propertyName, const std::string& propertyValue);

  vtkIdType GetPropertyRowIndex(const std::string& columnName);

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
