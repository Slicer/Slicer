#ifndef __vtkFetchMIClientCallerCollection_h
#define __vtkFetchMIClientCallerCollection_h

#include "vtkMRML.h"
#include "vtkObject.h"
#include "vtkCollection.h"
#include "vtkFetchMIClientCaller.h"

class VTK_FETCHMI_EXPORT vtkFetchMIClientCallerCollection : public vtkCollection
{
public:
  static vtkFetchMIClientCallerCollection *New();
  vtkTypeRevisionMacro(vtkFetchMIClientCallerCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return a parser that corresponds to the specified name.
  vtkFetchMIClientCaller * FindClientCallerByName ( const char *name );

  // Description:
  // Add a new parser to the collection with the specified name. The parser
  // can be retrieved by this name using the FindClientCallerByName (name) method.
  void AddClientCallerByName ( vtkFetchMIClientCaller *p, const char *name );

  // Description:
  // Delete a parser with the specified name from the collection.
  void DeleteClientCallerByName ( const char *name );

  // Description:
  // Add a ClientCaller to the Collection. It is recommended to use the AddClientCallerByName(name)
  // method, so the ClientCaller can be retrieved and deleted by name.
  void AddItem ( vtkFetchMIClientCaller *p ) {
  this->vtkCollection::AddItem ( (vtkObject *)p); };

  // Description:
  // Get the next ClientCaller in the list. Return NULL
  // when at the end of the list.
  vtkFetchMIClientCaller *GetNextClientCaller() {
  return static_cast<vtkFetchMIClientCaller *>(this->GetNextItemAsObject() ); };

  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextClientCaller() variant
  // where possible.
  vtkFetchMIClientCaller *GetNextItem() { return this->GetNextClientCaller(); };

   //BTX
  // Description: 
  // Reentrant safe way to get an object in a collection. 
  // Just pass the same cookie back and forth. 
  vtkFetchMIClientCaller *GetNextClientCaller(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkFetchMIClientCaller *>(this->GetNextItemAsObject(cookie));};
  //ETX

 protected:
  vtkFetchMIClientCallerCollection();
  ~vtkFetchMIClientCallerCollection();

  // hide the standard AddItem from the user and compiler.
  void AddItem (vtkObject *o ) { this->vtkCollection::AddItem(o); } ;
  
  vtkFetchMIClientCallerCollection(const vtkFetchMIClientCallerCollection&);
  void operator=(const vtkFetchMIClientCallerCollection&); private:
};

#endif
