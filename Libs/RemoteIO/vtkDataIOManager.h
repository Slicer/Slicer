#ifndef __vtkDataIOManager_h
#define __vtkDataIOManager_h

#include <vtkRemoteIOConfigure.h>
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkRemoteIO.h"
#include "vtkUnsignedLongArray.h"
#include "vtkIntArray.h"
#include "vtkDataTransfer.h"
#include "vtkCollection.h"

#include <list>
#include <string>

class vtkCallbackCommand;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif


class VTK_RemoteIO_EXPORT vtkDataIOManager : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkDataIOManager *New();
  vtkTypeRevisionMacro(vtkDataIOManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkGetObjectMacro ( DataTransferCollection, vtkCollection );
  vtkSetObjectMacro ( DataTransferCollection, vtkCollection );

  // Description:
  // Adds a new data transfer object to the collection
  void AddDataTransfer ( vtkDataTransfer *transfer );
  // Description:
  // Removes a data transfer object from the collection
  void RemoveDataTransfer ( vtkDataTransfer *transfer );
  void RemoveDataTransfer ( int transferID );
  // Description:
  // Returns an individual data transfer by id
  vtkDataTransfer *GetDataTransfer ( int transferID );
  // Description:
  // Clears all data transfers from the collection; called after
  // the cache is cleared.
  void ClearDataTransfers ( );
  // Description:
  // Gets a unique id to assign to a new data transfer.
  int GetUniqueTransferID ( );

  // Description:
  // Get i/o settings.
  vtkGetMacro ( Asynchronous, bool);
  vtkSetMacro ( Asynchronous, bool);
  virtual void Configure ();
  
 private:
  vtkCollection *DataTransferCollection;


 protected:
  vtkDataIOManager();
  virtual ~vtkDataIOManager();
  vtkDataIOManager(const vtkDataIOManager&);
  void operator=(const vtkDataIOManager&);

  // Description:
  // Holder for callback
  vtkCallbackCommand *CallbackCommand;

  bool Asynchronous;
};

#endif

