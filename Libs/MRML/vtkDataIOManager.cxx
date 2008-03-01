#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkDataIOManager.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLScene.h"

vtkStandardNewMacro ( vtkDataIOManager );
vtkCxxRevisionMacro(vtkDataIOManager, "$Revision: 1.9.12.1 $");

//----------------------------------------------------------------------------
vtkDataIOManager::vtkDataIOManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->DataTransferCollection = vtkCollection::New();
  this->CacheManager = NULL;
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

  if (this->CallbackCommand)
    {
    this->CallbackCommand->Delete();
    }
  this->CacheManager = NULL;

}


//----------------------------------------------------------------------------
int vtkDataIOManager::QueueWrite ( vtkMRMLDisplayableNode *node )
{
    if ( !node )
    {
    return (0);
    }
  if ( !node->GetStorageNode() )
    {
    return (0);
    }


//  const char *uri = node->GetURI ( );
  const char *uri = "http://www.media.mit.edu/~wjp/dingle.tst";
  const char *filename;
  
  //--- manage cache if there is a cache manager
  if ( this->GetCacheManager() != NULL )
    {
    filename = this->GetCacheManager()->GetFilenameFromURI(uri);
    if ( this->GetCacheManager()->GetEnableForceRedownload() )
      {
      this->GetCacheManager()->RemoveFromCache (filename);
      }
    }

  //--- spawn new thread of control and handle the read.
/*
  vtkURIHandler *handler = node->GetStorageNode()->GetURIHandler();
  if ( handler )
  {
    node->GetStorageNode()->AddObserver();
    //--- create new thread and call this:
    node->GetStorageNode()->WriteData ( node );
    }
    else
    {
    return (0);
    }
*/

  //--- create and configure a data transfer object for tracking
  vtkDataTransfer *dt = vtkDataTransfer::New ( );
  dt->SetTransferStatus ( vtkDataTransfer::Initialized );
  if ( !strcmp ( uri, filename ))
    {
    dt->SetTransferType ( vtkDataTransfer::LocalSave );
    }
  else
    {
    dt->SetTransferType ( vtkDataTransfer::RemoteUpload );
    }
  dt->SetTransferID ( this->GetUniqueTransferID() );
  dt->Modified();
  this->AddDataTransfer ( dt  );
  return (1);

}



//----------------------------------------------------------------------------
int vtkDataIOManager::QueueRead ( vtkMRMLDisplayableNode *node )
{
  
  if ( !node )
    {
    return (0);
    }
  if ( !node->GetStorageNode() )
    {
    return (0);
    }


//  const char *uri = node->GetURI ( );
  const char *uri = "http://www.media.mit.edu/~wjp/dingle.tst";
  const char *filename;
  
  //--- manage cache if there is a cache manager
  if ( this->GetCacheManager() != NULL )
    {
    filename = this->GetCacheManager()->GetFilenameFromURI(uri);
    if ( this->GetCacheManager()->GetEnableForceRedownload() )
      {
      this->GetCacheManager()->RemoveFromCache (filename);
      }
    }

  //--- spawn new thread of control and handle the read.
/*
  vtkURIHandler *handler = node->GetStorageNode()->GetURIHandler();
  if ( handler )
  {
    node->GetStorageNode()->AddObserver();
    //--- create new thread and call this:
    node->GetStorageNode()->ReadData ( node );
    }
    else
    {
    return (0);
    }
*/

  //--- create and configure a data transfer object for tracking
  vtkDataTransfer *dt = vtkDataTransfer::New ( );
  dt->SetTransferStatus ( vtkDataTransfer::Initialized );
  if ( !strcmp ( uri, filename ))
    {
    dt->SetTransferType ( vtkDataTransfer::LocalLoad );
    }
  else
    {
    dt->SetTransferType ( vtkDataTransfer::RemoteDownload );
    }
  dt->SetTransferID ( this->GetUniqueTransferID() );
  dt->Modified();
  this->AddDataTransfer ( dt  );
  return (1);
}









//----------------------------------------------------------------------------
void vtkDataIOManager::Configure ( )
{
  this->Asynchronous = false;
}



//----------------------------------------------------------------------------
void vtkDataIOManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "DataTransferCollection: " << this->GetDataTransferCollection() << "\n";

}


//----------------------------------------------------------------------------
void vtkDataIOManager::AddDataTransfer ( vtkDataTransfer *transfer )
{

  if ( this->DataTransferCollection == NULL )
    {
    this->DataTransferCollection = vtkCollection::New();
    }
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
    for ( int i=0; i < n; i++ )
      {
      dt = vtkDataTransfer::SafeDownCast(this->DataTransferCollection->GetItemAsObject ( i ) );
      if  ( id == dt->GetTransferID() )
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
    }
  return ( id );
}



