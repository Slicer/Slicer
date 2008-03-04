#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkDataIOManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

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
      transfer->SetTransferStatus(status);
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
  vtkDataTransfer *transfer = vtkDataTransfer::New();
  transfer->SetTransferID ( this->GetUniqueTransferID() );
  transfer->SetTransferNodeID ( node->GetID() );
  this->AddDataTransfer ( transfer );
  return (transfer );
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
void vtkDataIOManager::QueueRead ( vtkMRMLNode *node )
{
  this->InvokeEvent ( vtkDataIOManager::RemoteReadEvent, node);
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



