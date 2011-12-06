#ifndef __vtkFetchMIServerCollection_h
#define __vtkFetchMIServerCollection_h

// FetchMI includes
#include "vtkFetchMIServer.h"

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include "vtkObject.h"
#include "vtkCollection.h"

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIServerCollection : public vtkCollection
{
public:
  static vtkFetchMIServerCollection *New();
  vtkTypeRevisionMacro(vtkFetchMIServerCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return a server that corresponds to the specified name.
  vtkFetchMIServer * FindServerByName ( const char *name );

  // Description:
  // Add a new server to the collection with the specified name. The server
  // can be retrieved by this name using the FindServerByName (name) method.
  void AddServerByName ( vtkFetchMIServer *s, const char *name );

  // Description:
  // Delete a server with the specified name from the collection.
  void DeleteServerByName ( const char *name );

  // Description:
  // Add a Server to the Collection. It is recommended to use the AddServerByName(name)
  // method, so the Server can be retrieved and deleted by name.
  void AddItem ( vtkFetchMIServer *s ) {
  this->vtkCollection::AddItem ( (vtkObject *)s); };

  // Description:
  // Get the next Server in the list. Return NULL
  // when at the end of the list.
  vtkFetchMIServer *GetNextServer() {
  return static_cast<vtkFetchMIServer *>(this->GetNextItemAsObject() ); };

  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextServer() variant
  // where possible.
  vtkFetchMIServer *GetNextItem() { return this->GetNextServer(); };

  // Description:
  // This method returns 1 if the service type is known, 0 if not.
  // NOTE TO DEVELOPERS: known web service types are enumeraged
  // in this class. Currently supported service types are:
  // vtkFetchMIServerCollection::XND, and
  // vtkFetchMIServerCollection::HID is being developed.
  // As new services are supported, please add them to the enumeration
  // and the check in this method.
  int IsKnownServiceType ( const char *stype );

  // Description: 
  // Reentrant safe way to get an object in a collection; just pass the
  // same cookie back and forth. 
  vtkFetchMIServer *GetNextServer(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkFetchMIServer *>(this->GetNextItemAsObject(cookie));};

 protected:
  vtkFetchMIServerCollection();
  ~vtkFetchMIServerCollection();

  // hide the standard AddItem from the user and compiler.
  void AddItem (vtkObject *o ) { this->vtkCollection::AddItem(o); } ;
  
  vtkFetchMIServerCollection(const vtkFetchMIServerCollection&);
  void operator=(const vtkFetchMIServerCollection&); private:
};

#endif
