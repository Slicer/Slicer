#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkDataIOManager.h"
#include "vtkCallbackCommand.h"

vtkStandardNewMacro ( vtkDataIOManager );
vtkCxxRevisionMacro(vtkDataIOManager, "$Revision: 1.9.12.1 $");

//----------------------------------------------------------------------------
vtkDataIOManager::vtkDataIOManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->DataTransferCollection = vtkCollection::New();
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



