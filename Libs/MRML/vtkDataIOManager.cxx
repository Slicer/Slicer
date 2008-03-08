#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkDataIOManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLDisplayableNode.h"
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
}

//----------------------------------------------------------------------------
vtkDataIOManager::~vtkDataIOManager()
{
   
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
void vtkDataIOManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "DataTransferCollection: " << this->GetDataTransferCollection() << "\n";
  os << indent << "CacheManager: " << this->GetCacheManager() << "\n";
  os << indent << "EnableAsynchronousIO: " << this->GetEnableAsynchronousIO() << "\n";

}


//----------------------------------------------------------------------------
void vtkDataIOManager::SetTransferStatus(vtkDataTransfer *transfer, int status, bool modify)
{
  if ( transfer != NULL )
    {
    if ( transfer->GetTransferStatus() != status )
      {
      transfer->SetTransferStatus(status, modify);
      if ( modify )
        {
        //--- for whoever is observing.
        transfer->Modified();
        this->Modified();
        }
      }
    }
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
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDataIOManager::RemoveDataTransfer ( vtkDataTransfer *transfer)
{
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }
  this->DataTransferCollection->RemoveItem ( transfer );
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkDataIOManager::RemoveDataTransfer ( int transferID )
{
  vtkDataTransfer *dt;
  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }

  int n = this->DataTransferCollection->GetNumberOfItems();
  for ( int i=0; i < n; i++ )
    {
    dt = vtkDataTransfer::SafeDownCast (this->DataTransferCollection->GetItemAsObject ( i ) );
    if ( dt->GetTransferID() == transferID )
      {
      this->DataTransferCollection->RemoveItem ( i );
      this->Modified();
      break;
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
void vtkDataIOManager::QueueRead ( vtkMRMLNode *node )
{
  if (node == NULL)
    {
    vtkErrorMacro("QueueRead: null input node!");
    return;
    }
  vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast ( node );
  if (dnode == NULL)
    {
    vtkErrorMacro("QueueRead: unable to cast input mrml node to a displayable node");
    return;
    }
  if (dnode->GetStorageNode() == NULL)
    {
    vtkErrorMacro("QueueRead: unable to get storage node from the displayable node, returning");
    return;
    }
  vtkURIHandler *handler = dnode->GetStorageNode()->GetURIHandler();
  vtkDebugMacro("QueueRead: got the uri handler from the storage node");
  const char *source = dnode->GetStorageNode()->GetURI();
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
    
    //--- trigger logic to download, if there's cache space.
    if ( cm->GetCurrentCacheSize() < cm->GetRemoteCacheLimit() )
      {
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
  this->InvokeEvent ( vtkDataIOManager::RemoteWriteEvent, node );
}


//----------------------------------------------------------------------------
int vtkDataIOManager::GetUniqueTransferID ( )
{
  
  //--- keep looping until we find an id that is unique
  int id = 1;
  int i = 0;
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



