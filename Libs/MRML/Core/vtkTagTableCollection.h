#ifndef __vtkTagTableCollection_h
#define __vtkTagTableCollection_h

// MRML includes
#include "vtkMRML.h"
class vtkTagTable;

// VTK includes
#include <vtkCollection.h>

class VTK_MRML_EXPORT vtkTagTableCollection : public vtkCollection
{
public:
  static vtkTagTableCollection *New();
  vtkTypeMacro(vtkTagTableCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkTagTable * FindTagTableByName ( const char *name );
  void AddTableByName ( vtkTagTable *t, const char *name );
  void DeleteTableByName ( const char *name );
  void ClearAllTagTables ( );
  void SetRestoreSelectionStateForAllTables ( int val);

  ///
  /// Add a Tag Table to the list.
  void AddItem ( vtkTagTable *t ) {
  this->vtkCollection::AddItem ( (vtkObject *)t); };

  ///
  /// Get the next Table in the list. Return nullptr
  /// when at the end of the list.
  vtkTagTable *GetNextTable();

  ///
  /// Access routine provided for compatibility with previous
  /// versions of VTK.  Please use the GetNextTable() variant
  /// where possible.
  vtkTagTable *GetNextItem() { return this->GetNextTable(); };

  ///
  /// Reentrant safe way to get an object in a collection.
  /// Just pass the same cookie back and forth.
  vtkTagTable *GetNextTable(vtkCollectionSimpleIterator &cookie);

 protected:
  vtkTagTableCollection();
  ~vtkTagTableCollection() override;

  /// hide the standard AddItem from the user and compiler.
  void AddItem (vtkObject *o ) { this->vtkCollection::AddItem(o); } ;

  vtkTagTableCollection(const vtkTagTableCollection&);
  void operator=(const vtkTagTableCollection&); private:
};

#endif
