#ifndef __vtkFetchMIParserCollection_h
#define __vtkFetchMIParserCollection_h

#include "vtkMRML.h"
#include "vtkObject.h"
#include "vtkCollection.h"
#include "vtkFetchMIParser.h"

class VTK_FETCHMI_EXPORT vtkFetchMIParserCollection : public vtkCollection
{
public:
  static vtkFetchMIParserCollection *New();
  vtkTypeRevisionMacro(vtkFetchMIParserCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return a parser that corresponds to the specified name.
  vtkFetchMIParser * FindParserByName ( const char *name );

  // Description:
  // Add a new parser to the collection with the specified name. The parser
  // can be retrieved by this name using the FindParserByName (name) method.
  void AddParserByName ( vtkFetchMIParser *p, const char *name );

  // Description:
  // Delete a parser with the specified name from the collection.
  void DeleteParserByName ( const char *name );

  // Description:
  // Add a Parser to the Collection. It is recommended to use the AddParserByName(name)
  // method, so the Parser can be retrieved and deleted by name.
  void AddItem ( vtkFetchMIParser *p ) {
  this->vtkCollection::AddItem ( (vtkObject *)p); };

  // Description:
  // Get the next Parser in the list. Return NULL
  // when at the end of the list.
  vtkFetchMIParser *GetNextParser() {
  return static_cast<vtkFetchMIParser *>(this->GetNextItemAsObject() ); };

  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextParser() variant
  // where possible.
  vtkFetchMIParser *GetNextItem() { return this->GetNextParser(); };

   //BTX
  // Description: 
  // Reentrant safe way to get an object in a collection. 
  // Just pass the same cookie back and forth. 
  vtkFetchMIParser *GetNextParser(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkFetchMIParser *>(this->GetNextItemAsObject(cookie));};
  //ETX

 protected:
  vtkFetchMIParserCollection();
  ~vtkFetchMIParserCollection();

  // hide the standard AddItem from the user and compiler.
  void AddItem (vtkObject *o ) { this->vtkCollection::AddItem(o); } ;
  
  vtkFetchMIParserCollection(const vtkFetchMIParserCollection&);
  void operator=(const vtkFetchMIParserCollection&); private:
};

#endif
