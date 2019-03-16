#ifndef __vtkDataIOManager_h
#define __vtkDataIOManager_h

// MRML includes
#include "vtkMRML.h"
class vtkCacheManager;
class vtkDataFileFormatHelper;
class vtkDataTransfer;
class vtkMRMLNode;

// VTK includes
#include <vtkObject.h>
class vtkCallbackCommand;
class vtkCollection;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

class VTK_MRML_EXPORT vtkDataIOManager : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkDataIOManager *New();
  vtkTypeMacro(vtkDataIOManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkGetObjectMacro ( DataTransferCollection, vtkCollection );
  void SetDataTransferCollection(vtkCollection* dataTransfer );
  vtkGetObjectMacro ( CacheManager, vtkCacheManager );
  virtual void SetCacheManager(vtkCacheManager* cacheManager);
  vtkGetMacro ( EnableAsynchronousIO, int );
  vtkGetMacro ( InUpdateCallbackFlag, int );
  vtkSetMacro ( InUpdateCallbackFlag, int );

  ///
  /// Get/Set the DataFileFormatHelper object
  vtkDataFileFormatHelper* GetFileFormatHelper();
  virtual void SetFileFormatHelper(vtkDataFileFormatHelper* helper);

  void SetEnableAsynchronousIO ( int );

  ///
  /// Creates and adds a new data transfer object to the collection
  vtkDataTransfer *AddNewDataTransfer ( );
  vtkDataTransfer *AddNewDataTransfer ( vtkMRMLNode *node);
  void AddNewDataTransfer ( vtkDataTransfer *transfer, vtkMRMLNode *node);
  void AllTransfersClearedFromCache();

  ///
  /// Adds a new data transfer object to the collection
  void AddDataTransfer ( vtkDataTransfer *transfer );
  ///
  /// Removes a data transfer object from the collection
  void RemoveDataTransfer ( vtkDataTransfer *transfer );
  void RemoveDataTransfer ( int transferID );
  int GetNumberOfDataTransfers();
  ///
  /// Returns an individual data transfer by id
  vtkDataTransfer *GetDataTransfer ( int transferID );
  ///
  /// Clears all data transfers from the collection; called after
  /// the cache is cleared.
  void ClearDataTransfers ( );
  ///
  /// Gets a unique id to assign to a new data transfer.
  int GetUniqueTransferID ( );

  ///
  /// Invokes a RemoteReadEvent on the node
  /// so that logic can take care of scheduling and applying it
  void QueueRead ( vtkMRMLNode *node );

  ///
  /// Invokes a RemoteWriteEvent on the node
  /// so that logic can take care of scheduling and applying it
  void QueueWrite ( vtkMRMLNode *node );

  ///
  /// Set the status of a data transfer (Idle, Scheduled, Cancelled Running,
  /// Completed).  The "modify" parameter indicates whether the object
  /// can be modified by the call.
  void SetTransferStatus(vtkDataTransfer *transfer, int status);
  int GetTransferStatus( vtkDataTransfer *transfer);

  const char* GetTransferStatusString( vtkDataTransfer *transfer );

  virtual void ProcessTransferUpdates ( vtkObject *caller, unsigned long event, void *callData );

  enum
    {
      RemoteReadEvent = 19001,
      RemoteWriteEvent,
      LocalReadEvent,
      LocalWriteEvent,
      NewTransferEvent,
      TransferUpdateEvent,
      SettingsUpdateEvent,
      DisplayManagerWindowEvent,
      RefreshDisplayEvent
    };

  /// function that gets called when a data transfer has been updated.
  static void TransferUpdateCallback ( vtkObject *__caller,
                                       unsigned long eid, void *__clientData, void *callData );

  /// Holder for update callback
  vtkCallbackCommand *TransferUpdateCommand;
  int InUpdateCallbackFlag;

 private:
  vtkCollection *DataTransferCollection;
  vtkCacheManager *CacheManager;
  int EnableAsynchronousIO;

  vtkDataFileFormatHelper* FileFormatHelper;

 protected:
  vtkDataIOManager();
  ~vtkDataIOManager() override;
  vtkDataIOManager(const vtkDataIOManager&);
  void operator=(const vtkDataIOManager&);

};

#endif



