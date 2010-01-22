#ifndef __vtkTagTableCollection_h
#define __vtkTagTableCollection_h

#include "vtkMRML.h"
#include "vtkObject.h"
#include "vtkCollection.h"
#include "vtkTagTable.h"

class VTK_MRML_EXPORT vtkTagTableCollection : public vtkCollection
{
public:
  static vtkTagTableCollection *New();
  vtkTypeRevisionMacro(vtkTagTableCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  /// Get the next Table in the list. Return NULL
  /// when at the end of the list.
  vtkTagTable *GetNextTable() {
  return static_cast<vtkTagTable *>(this->GetNextItemAsObject() ); };

  /// 
  /// Access routine provided for compatibility with previous
  /// versions of VTK.  Please use the GetNextTable() variant
  /// where possible.
  vtkTagTable *GetNextItem() { return this->GetNextTable(); };

   //BTX
  ///  
  /// Reentrant safe way to get an object in a collection. 
  /// Just pass the same cookie back and forth. 
  vtkTagTable *GetNextTable(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkTagTable *>(this->GetNextItemAsObject(cookie));};
  //ETX

 protected:
  vtkTagTableCollection();
  ~vtkTagTableCollection();

  /// hide the standard AddItem from the user and compiler.
  void AddItem (vtkObject *o ) { this->vtkCollection::AddItem(o); } ;
  
  vtkTagTableCollection(const vtkTagTableCollection&);
  void operator=(const vtkTagTableCollection&); private:
};

#endif
