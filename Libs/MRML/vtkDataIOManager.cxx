#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkDataIOManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkURIHandler.h"

#include <list>
#include <string>
#include <algorithm>
#include <set>

vtkStandardNewMacro ( vtkDataIOManager );
vtkCxxRevisionMacro(vtkDataIOManager, "$Revision: 1.9.12.1 $");


//----------------------------------------------------------------------------
vtkDataIOManager::vtkDataIOManager()
{
  this->DataTransferCollection = vtkCollection::New();
  this->CacheManager = NULL;
  this->EnableAsynchronousIO = 0;

  //--- set up callback
  this->TransferUpdateCommand = vtkCallbackCommand::New();
  this->TransferUpdateCommand->SetClientData ( reinterpret_cast<void *>(this) );
  this->TransferUpdateCommand->SetCallback (vtkDataIOManager::TransferUpdateCallback );
  this->InUpdateCallbackFlag = 0;
}

//----------------------------------------------------------------------------
vtkDataIOManager::~vtkDataIOManager()
{
   
  if ( this->TransferUpdateCommand )
    {
    this->TransferUpdateCommand->Delete();
    this->TransferUpdateCommand = NULL;
    }
  
  if ( this->DataTransferCollection )
    {
    this->DataTransferCollection->RemoveAllItems();
    this->DataTransferCollection->Delete();
    this->DataTransferCollection = NULL;
    }

  if ( this->CacheManager )
    {
    this->CacheManager->Delete();
    this->CacheManager = NULL;
    }
  this->EnableAsynchronousIO = 0;
}

//----------------------------------------------------------------------------
void vtkDataIOManager::TransferUpdateCallback ( vtkObject *caller,
                                       unsigned long eid, void *clientData, void *callData )
{
  vtkDataIOManager *self = reinterpret_cast <vtkDataIOManager *>(clientData);
  if ( self->GetInUpdateCallbackFlag())
    {
    return;
    }
  self->SetInUpdateCallbackFlag(1);
  self->ProcessTransferUpdates( caller, eid, callData );
  self->SetInUpdateCallbackFlag(0);
}



//----------------------------------------------------------------------------
void vtkDataIOManager::ProcessTransferUpdates ( vtkObject *caller, unsigned long event, void *callData )
{
  vtkDebugMacro("ProcessTransferUpdates: invoking transfer update event.");
  this->InvokeEvent ( vtkDataIOManager::TransferUpdateEvent);
}


//----------------------------------------------------------------------------
void vtkDataIOManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "DataTransferCollection: " << this->GetDataTransferCollection() << "\n";
  os << indent << "CacheManager: " << this->GetCacheManager() << "\n";
  os << indent << "EnableAsynchronousIO: " << this->GetEnableAsynchronousIO() << "\n";

}


//----------------------------------------------------------------------------
void vtkDataIOManager::SetTransferStatus(vtkDataTransfer *transfer, int status)
{
  if ( transfer != NULL )
    {
    if ( transfer->GetTransferStatus() != status )
      {
      transfer->SetTransferStatus(status);
      }
    }
}

//----------------------------------------------------------------------------
int vtkDataIOManager::GetNumberOfDataTransfers()
{
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }
  return ( this->DataTransferCollection->GetNumberOfItems() );
}



//----------------------------------------------------------------------------
int vtkDataIOManager::GetTransferStatus( vtkDataTransfer *transfer)
{
  return ( transfer->GetTransferStatus() );
}



//----------------------------------------------------------------------------
vtkDataTransfer *vtkDataIOManager::AddNewDataTransfer ( )
{
  vtkDataTransfer *transfer = vtkDataTransfer::New();
  transfer->SetTransferID ( this->GetUniqueTransferID() );
  this->AddDataTransfer ( transfer );
  return (transfer );
}


//----------------------------------------------------------------------------
vtkDataTransfer *vtkDataIOManager::AddNewDataTransfer ( vtkMRMLNode *node )
{
  if (node == NULL)
    {
    vtkErrorMacro("AddNewDataTransfer: node is null");
    return NULL;
    }
  vtkDataTransfer *transfer = vtkDataTransfer::New();
  transfer->SetTransferID ( this->GetUniqueTransferID() );
  transfer->SetTransferNodeID ( node->GetID() );
  this->AddDataTransfer ( transfer );
  vtkDebugMacro("AddNewDataTransfer: returning new transfer");
  return (transfer );
}


//----------------------------------------------------------------------------
void vtkDataIOManager::AddNewDataTransfer ( vtkDataTransfer *transfer, vtkMRMLNode *node )
{
  if (node != NULL && transfer != NULL )
    {
    this->AddDataTransfer ( transfer );
    this->InvokeEvent (vtkDataIOManager::NewTransferEvent, transfer );
    vtkDebugMacro("AddNewDataTransfer: added data transfer to dataIOManager's collection, invoked the new transfer event");
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManager::AllTransfersClearedFromCache()
{
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }

  vtkDataTransfer *dt;
  int n = this->DataTransferCollection->GetNumberOfItems();
  for ( int i=0; i < n; i++ )
    {
    dt = vtkDataTransfer::SafeDownCast (this->DataTransferCollection->GetItemAsObject ( i ) );
    if ( dt != NULL )
      {
      dt->SetTransferCached(0);
      }
    }

}


//----------------------------------------------------------------------------
void vtkDataIOManager::AddDataTransfer ( vtkDataTransfer *transfer )
{

  if (transfer == NULL)
    {
    vtkErrorMacro("AddDataTransfer: can't add a null transfer");
    return;
    }
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }
  vtkDebugMacro("AddDataTransfer: adding item");
  this->DataTransferCollection->AddItem ( transfer );
  transfer->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->TransferUpdateCommand );
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDataIOManager::RemoveDataTransfer ( vtkDataTransfer *transfer)
{
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }
  if ( transfer != NULL )
    {
    // remove observer before deleting
    transfer->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->TransferUpdateCommand );
    this->DataTransferCollection->RemoveItem ( transfer );
    this->Modified();
    }
}



//----------------------------------------------------------------------------
void vtkDataIOManager::RemoveDataTransfer ( int transferID )
{
  vtkDataTransfer *dt;
  if ( this->DataTransferCollection == NULL )
    {    // remove observer before deleting
    this->DataTransferCollection = vtkCollection::New();
    }

  int n = this->DataTransferCollection->GetNumberOfItems();
  for ( int i=0; i < n; i++ )
    {
    dt = vtkDataTransfer::SafeDownCast (this->DataTransferCollection->GetItemAsObject ( i ) );
    if ( dt != NULL )
      {
      if ( dt->GetTransferID() == transferID )
        {
        // remove observer before deleting
        dt->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->TransferUpdateCallback );
        this->DataTransferCollection->RemoveItem ( i );
        this->Modified();
        break;
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkDataTransfer * vtkDataIOManager::GetDataTransfer ( int transferID )
{
  
  vtkDataTransfer *dt;

  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }

  int n = this->DataTransferCollection->GetNumberOfItems();
  for ( int i=0; i < n; i++ )
    {
    dt = vtkDataTransfer::SafeDownCast ( this->DataTransferCollection->GetItemAsObject (i) );
    if ( transferID == dt->GetTransferID() )
      {
      return ( dt );
      }
    }
  return ( NULL );
}



//----------------------------------------------------------------------------
void vtkDataIOManager::ClearDataTransfers( )
{

  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }
  this->DataTransferCollection->RemoveAllItems();
  this->Modified();
}


//----------------------------------------------------------------------------
void vtkDataIOManager::SetEnableAsynchronousIO ( int val )
{
  if ( val == 1 || val == 0 )
    {
    this->EnableAsynchronousIO = val;
    this->InvokeEvent ( vtkDataIOManager::SettingsUpdateEvent );
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManager::QueueRead ( vtkMRMLNode *node )
{
  if (node == NULL)
    {
    vtkErrorMacro("QueueRead: null input node!");
    return;
    }
  vtkMRMLStorableNode *dnode = vtkMRMLStorableNode::SafeDownCast ( node );
  if (dnode == NULL)
    {
    vtkErrorMacro("QueueRead: unable to cast input mrml node to a storable node");
    return;
    }
  // look for a pending storage node
  int storageNodeIndex = -1;
  if (dnode->GetStorageNode() == NULL)
    {
    vtkErrorMacro("QueueRead: unable to get storage node from the storable node, returning");
    return;
    }
  for (int i = 0; i < dnode->GetNumberOfStorageNodes(); i++)
    {
    if (dnode->GetNthStorageNode(i)->GetReadState() == vtkMRMLStorageNode::Pending)
      {
      vtkDebugMacro("QueueRead: found a pending storage node at index " << i << ", setting it to scheduled");
      dnode->GetNthStorageNode(i)->SetReadStateScheduled();
      storageNodeIndex = i;
      break;
      }
    }
  if (storageNodeIndex == -1)
    {
    vtkErrorMacro("QueueRead: unable to find a pending storage node!");
    return;
    }
  //vtkURIHandler *handler = dnode->GetNthStorageNode(storageNodeIndex)->GetURIHandler();
  vtkDebugMacro("QueueRead: got the uri handler from the storage node");
  const char *source = dnode->GetNthStorageNode(storageNodeIndex)->GetURI();
  const char *dest; 

  if (source == NULL)
    {
    vtkDebugMacro("QueueRead: storage node's URI is null, returning.");
    return;
    }
  vtkCacheManager *cm = this->GetCacheManager();
  if ( cm != NULL )
    {
    dest = cm->GetFilenameFromURI(source);
    if (dest == NULL)
      {
      vtkDebugMacro("QueueRead: unable to get file name from source URI " << source);
      return;
      }
    else
      {
      vtkDebugMacro("QueueRead: got destination: " << dest);
      }
    //--- check to see if RemoteCacheLimit is exceeded
    //--- check to see if FreeBufferSize is exceeded.
   
    //--- if force redownload is enabled, remove the old file from cache.
    if (cm->GetEnableForceRedownload () )
      {
      vtkDebugMacro("QueueRead: Calling remove from cache");
      this->GetCacheManager()->DeleteFromCache ( dest );
      }
    //---
    //--- WJPtest
    //--- Test for space to download the file. If no space,
    //--- check for InsufficientFreeBufferNotificationFlag.
    //--- if set, do nothing.
    //--- if not yet set, send event, then set it.
    //--- This flag is here to help us avoid bugging the user if
    //--- they are downloading a large scene.
    float bufsize = (cm->GetRemoteCacheLimit() * 1000000.0) -  (cm->GetRemoteCacheFreeBufferSize() * 1000000.0);
    if ( (cm->GetCurrentCacheSize()*1000000.0) >= bufsize )
      {
      //--- No space left in cache. Don't trigger logic to download;
      //--- let GUI post a pop-up dialog to inform user.
      cm->InvokeEvent ( vtkCacheManager::InsufficientFreeBufferEvent );
      cm->SetInsufficientFreeBufferNotificationFlag(1);
      }
    else
      {
      //--- reset the cachemanager's flag -- looks like enough cache space
      //--- exists to do the download.
      cm->SetInsufficientFreeBufferNotificationFlag(0);
      //--- trigger logic to download, if there's cache space.
      //--- and signal this remote read event to Logic and GUI.
      vtkDebugMacro("QueueRead: invoking a remote read event on the data io manager");
      this->InvokeEvent ( vtkDataIOManager::RemoteReadEvent, node);
      }
    }
  else
    {
    vtkErrorMacro("QueueRead: cache manager is null.");
    }

}

//----------------------------------------------------------------------------
void vtkDataIOManager::QueueWrite ( vtkMRMLNode *node )
{
  if (node == NULL)
    {
    vtkErrorMacro("QueueWrite: null input node!");
    return;
    }
  vtkMRMLStorableNode *dnode = vtkMRMLStorableNode::SafeDownCast ( node );
  if (dnode == NULL)
    {
    vtkErrorMacro("QueueWrite: unable to cast input mrml node to a storable node");
    return;
    }
  if (dnode->GetStorageNode() == NULL)
    {
    vtkErrorMacro("QueueWrite: unable to get storage node from the storable node, returning");
    return;
    }
  int storageNodeIndex = -1;
  for (int i = 0; i < dnode->GetNumberOfStorageNodes(); i++)
    {
    if (dnode->GetNthStorageNode(i)->GetWriteState() == vtkMRMLStorageNode::Pending)
      {
      vtkDebugMacro("QueueWrite: found a pending storage node at index " << i << ", changing it to scheduled");
      dnode->GetNthStorageNode(i)->SetWriteStateScheduled();
      storageNodeIndex = i;
      continue;
      }
    }
  if (storageNodeIndex == -1)
    {
    vtkErrorMacro("QueueWrite: unable to find a pending storage node!");
    return;
    }
  //vtkURIHandler *handler = dnode->GetNthStorageNode(storageNodeIndex)->GetURIHandler();
  vtkDebugMacro("QueueWrite: got the uri handler from the storage node");
  const char *source = dnode->GetNthStorageNode(storageNodeIndex)->GetFileName();
  const char *dest = dnode->GetNthStorageNode(storageNodeIndex)->GetURI();

  if (source == NULL)
    {
    vtkDebugMacro("QueueWrite: storage node's file is null, returning.");
    return;
    }
  if (dest == NULL)
    {
    vtkDebugMacro("QueueWrite: unable to get destination URI, source = " << source);
    return;
    }
  else
    {
    vtkDebugMacro("QueueWrite: got destination: " << dest);
    }
 
  vtkDebugMacro("QueueWrite: invoking a remote write event on the data io manager");
  //--- signal this remote write event to Logic and GUI.
  this->InvokeEvent ( vtkDataIOManager::RemoteWriteEvent, node);
}


//----------------------------------------------------------------------------
int vtkDataIOManager::GetUniqueTransferID ( )
{
  
  //--- keep looping until we find an id that is unique
  int id = 1;
  int exists = 0;
  vtkDataTransfer *dt;
    
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }

  // loop until found or return.
  while ( !exists )
    {
    
    // loop thru the existing data transfers
    int n = this->DataTransferCollection->GetNumberOfItems();
    vtkDebugMacro("GetUniqueTransferID: in loop, id = " << id << ", n = " << n);
    for ( int i=0; i < n; i++ )
      {
      dt = vtkDataTransfer::SafeDownCast(this->DataTransferCollection->GetItemAsObject ( i ) );
      if  ( dt != NULL && id == dt->GetTransferID() )
        {
        exists = 1;
        break;
        }
      }
    // finished looping -- did we find the id?
    if ( exists )
      {
      // if so, try a new id
      id++;
      exists = 0;
      }
    else
      {
      vtkDebugMacro("GetUniqueTransferID: in loop, returning id = " << id);
      return (id);
      }
    }
  vtkDebugMacro("GetUniqueTransferID: returning id = " << id);
  return ( id );
}



