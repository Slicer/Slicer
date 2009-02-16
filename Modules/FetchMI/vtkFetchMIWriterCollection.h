#ifndef __vtkFetchMIWriterCollection_h
#define __vtkFetchMIWriterCollection_h

#include "vtkMRML.h"
#include "vtkObject.h"
#include "vtkCollection.h"
#include "vtkFetchMIWriter.h"

class VTK_FETCHMI_EXPORT vtkFetchMIWriterCollection : public vtkCollection
{
public:
  static vtkFetchMIWriterCollection *New();
  vtkTypeRevisionMacro(vtkFetchMIWriterCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return a writer that corresponds to the specified name.
  vtkFetchMIWriter * FindWriterByName ( const char *name );

  // Description:
  // Add a new writer to the collection with the specified name. The writer
  // can be retrieved by this name using the FindWriterByName (name) method.
  void AddWriterByName ( vtkFetchMIWriter *w, const char *name );

  // Description:
  // Delete a writer with the specified name from the collection.
  void DeleteWriterByName ( const char *name );

  // Description:
  // Add a Writer to the Collection. It is recommended to use the AddWriterByName(name)
  // method, so the Writer can be retrieved and deleted by name.
  void AddItem ( vtkFetchMIWriter *w ) {
  this->vtkCollection::AddItem ( (vtkObject *)w); };

  // Description:
  // Get the next Writer in the list. Return NULL
  // when at the end of the list.
  vtkFetchMIWriter *GetNextWriter() {
  return static_cast<vtkFetchMIWriter *>(this->GetNextItemAsObject() ); };

  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextWriter() variant
  // where possible.
  vtkFetchMIWriter *GetNextItem() { return this->GetNextWriter(); };

   //BTX
  // Description: 
  // Reentrant safe way to get an object in a collection. 
  // Just pass the same cookie back and forth. 
  vtkFetchMIWriter *GetNextWriter(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkFetchMIWriter *>(this->GetNextItemAsObject(cookie));};
  //ETX

 protected:
  vtkFetchMIWriterCollection();
  ~vtkFetchMIWriterCollection();

  // hide the standard AddItem from the user and compiler.
  void AddItem (vtkObject *o ) { this->vtkCollection::AddItem(o); } ;
  
  vtkFetchMIWriterCollection(const vtkFetchMIWriterCollection&);
  void operator=(const vtkFetchMIWriterCollection&); private:
};

#endif
