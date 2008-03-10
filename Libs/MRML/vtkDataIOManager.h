#ifndef __vtkDataIOManager_h
#define __vtkDataIOManager_h


#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkUnsignedLongArray.h"
#include "vtkIntArray.h"
#include "vtkCallbackCommand.h"

#include "vtkDataTransfer.h"
#include "vtkCacheManager.h"
#include "vtkCollection.h"
#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif



class VTK_MRML_EXPORT vtkDataIOManager : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkDataIOManager *New();
  vtkTypeRevisionMacro(vtkDataIOManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkGetObjectMacro ( DataTransferCollection, vtkCollection );
  vtkSetObjectMacro ( DataTransferCollection, vtkCollection );
  vtkGetObjectMacro ( CacheManager, vtkCacheManager );
  vtkSetObjectMacro ( CacheManager, vtkCacheManager );
  vtkGetMacro ( EnableAsynchronousIO, int );
  vtkSetMacro ( EnableAsynchronousIO, int );
  vtkGetMacro ( InUpdateCallbackFlag, int );
  vtkSetMacro ( InUpdateCallbackFlag, int );
  
  // Description:
  // Creates and adds a new data transfer object to the collection
  vtkDataTransfer *AddNewDataTransfer ( );
  vtkDataTransfer *AddNewDataTransfer ( vtkMRMLNode *node);
  void AddNewDataTransfer ( vtkDataTransfer *transfer, vtkMRMLNode *node);
  void AllTransfersClearedFromCache();

  // Description:
  // Adds a new data transfer object to the collection
  void AddDataTransfer ( vtkDataTransfer *transfer );
  // Description:
  // Removes a data transfer object from the collection
  void RemoveDataTransfer ( vtkDataTransfer *transfer );
  void RemoveDataTransfer ( int transferID );
  int GetNumberOfDataTransfers();
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
  // Invokes a RemoteReadEvent on the node 
  // so that logic can take care of scheduling and applying it
  void QueueRead ( vtkMRMLNode *node );
  
  // Description:
  // Invokes a RemoteWriteEvent on the node 
  // so that logic can take care of scheduling and applying it
  void QueueWrite ( vtkMRMLNode *node );

  // Description:
  // Set the status of a data transfer (Idle, Scheduled, Cancelled Running,
  // Completed).  The "modify" parameter indicates whether the object
  // can be modified by the call.
  void SetTransferStatus(vtkDataTransfer *transfer, int status);
  int GetTransferStatus( vtkDataTransfer *transfer);

  const char* GetTransferStatusString( vtkDataTransfer *transfer )
    {
    return (transfer->GetTransferStatusString () );
    };

  virtual void ProcessTransferUpdates ( vtkObject *caller, unsigned long event, void *callData );
  
  //BTX
  enum
    {
      RemoteReadEvent = 19001,
      RemoteWriteEvent,
      LocalReadEvent,
      LocalWriteEvent,
      NewTransferEvent,
      TransferUpdateEvent,
    };

  // function that gets called when a data transfer has been updated.
  static void TransferUpdateCallback ( vtkObject *__caller,
                                       unsigned long eid, void *__clientData, void *callData );
  //ETX

  // Holder for update callback
  vtkCallbackCommand *TransferUpdateCommand;
  int InUpdateCallbackFlag;
  
 private:
  vtkCollection *DataTransferCollection;
  vtkCacheManager *CacheManager;
  int EnableAsynchronousIO;

 protected:
  vtkDataIOManager();
  virtual ~vtkDataIOManager();
  vtkDataIOManager(const vtkDataIOManager&);
  void operator=(const vtkDataIOManager&);

};

#endif

