#ifndef __vtkUserTagTable_h
#define __vtkUserTagTable_h

#include "vtkObject.h"
#include "vtkTable.h"
#include "vtkMRML.h"


class VTK_MRML_EXPORT vtkUserTagTable : public vtkTable
{
public:
  /// The Usual vtk class functions
  static vtkUserTagTable *New();
  vtkTypeMacro(vtkUserTagTable, vtkTable);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Get the vtkTable that contains user-defined keyword/value pairs.
  vtkGetObjectMacro ( TagTable, vtkTable );

  ///
  /// Add a tag column to the TagTable; the column's name will be "keyword"
  /// and the column's 0th entry will be "value". Only unique keywords are added.
  virtual int AddKeywordValuePair ( const char *keyword, void *value);

  ///
  /// Add a tag column to the TagTable; the column's name will be "keyword"
  /// and the column's 0th entry will be "value".
  virtual int AddKeywordValuePair ( const char *keyword, const char *value);

  ///
  /// Get the keyword at a particular column in the TagTable.
  virtual const char *GetUserTagKeyword ( int index);

  ///
  /// Get the value at a particular column in the TagTable
  virtual const char *GetUserTagValue ( int index);

  ///
  /// Get the value for a keyword in the TagTable
  virtual const char *GetUserTagValue ( const char* keyword);

  ///
  /// Checks TagTable to see if a keyword already exists.
  /// Returns 1 if keyword is found, or 0 if it's not found.
  virtual int CheckTableForKeyword ( const char *keyword);

  ///
  /// Deletes a column corresponding to "index" in the TagTable.
  virtual int DeleteKeywordValuePair ( int index );

  ///
  /// Deletes a column with the name matching "keyword" in the TagTable
  virtual int DeleteKeywordValuePair ( const char *keyword );

  ///
  /// Returns the index of the TagTable column whose name matches "keyword"
  virtual int GetKeywordColumn ( const char* keyword  );

  ///
  /// Convenience method that returns the number of columns in TagTable.
  virtual int GetNumberOfColumns ( );

 private:
  vtkTable *TagTable;

 protected:
  vtkUserTagTable();
  virtual ~vtkUserTagTable();
  vtkUserTagTable(const vtkUserTagTable&);
  void operator=(const vtkUserTagTable&);

};

#endif
