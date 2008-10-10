#ifndef __vtkTagTable_h
#define __vtkTagTable_h

#include "vtkObject.h"
#include "vtkTable.h"
#include "vtkMRML.h"

#include <string>
#include <map>

class VTK_MRML_EXPORT vtkTagTable : public vtkObject
{
  public:
  // The Usual vtk class functions
  static vtkTagTable *New();
  vtkTypeRevisionMacro(vtkTagTable, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the vtkTable that contains user-defined attribute/value pairs.
  vtkGetStringMacro ( Name );
  vtkSetStringMacro ( Name );
  
  // Description:
  // Method that sets up default and required tags for a service.
  // Each derived class should fill out this method.
  virtual void Initialize() { };

  // Description:
  // Add a tag column to the TagTable
  virtual void AddOrUpdateTag ( const char *attribute, const char *value );
  // Description:
  // Add a tag column to the TagTable
  virtual void AddOrUpdateTag ( const char *attribute, const char *value, int selected );
  // Description:
  // Add a tag column to the TagTable
  virtual void AddUniqueTag ( const char *attribute, const char *value );
  // Description:
  // Replace a tag column in the TagTable
  virtual int UpdateTag ( const char *attribute, const char *value );

  // Description:
  // Get the attribute at a particular index in the TagTable.
  virtual const char *GetTagAttribute ( int index);

  // Description:
  // Get the value of a tag in the TagTable
  virtual const char *GetTagValue ( int index );

  // Description:
  // Get the value of a tag in the TagTable
  virtual const char *GetTagValue ( const char* attribute);
  
  // Description:
  // Checks TagTable to see if a attribute already exists.
  // Returns 1 if attribute is found, or 0 if it's not found.
  virtual int CheckTableForTag ( const char *attribute, const char *value );

  virtual void SelectTag ( const char *attribute );
  virtual void DeselectTag ( const char *attribute );
  virtual int IsTagSelected ( const char *attribute );
  
  
  // Description:
  // Deletes a column with the name matching "attribute" in the TagTable
  virtual void DeleteTag ( const char *attribute );

  // Description:
  // Clears all tags out of table.
  virtual void ClearTagTable ( );
  
  // Description:
  // Convenience method that returns the number of tags in TagTable.
  virtual int GetNumberOfTags ( );
  
  //BTX
  // describes attribute value pairs
  std::map<std::string, std::string> TagTable;
  // describes whether each attribute is selected or not.
  std::map<std::string, int > TagSelectionTable;
  //ETX

 private:
  char *Name;

 protected:
  vtkTagTable();
  virtual ~vtkTagTable();
  vtkTagTable(const vtkTagTable&);
  void operator=(const vtkTagTable&);

};

#endif






