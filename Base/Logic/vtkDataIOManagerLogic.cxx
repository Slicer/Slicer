#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "vtkDataIOManagerLogic.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLScene.h"
#include "vtkCommand.h"

#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "itksys/RegularExpression.hxx"

#include <algorithm>
#include <set>

#ifdef linux 
#include "unistd.h"
#endif

#ifdef _WIN32
#else
#include <sys/types.h>
#include <unistd.h>
#endif


vtkStandardNewMacro ( vtkDataIOManagerLogic );
vtkCxxRevisionMacro(vtkDataIOManagerLogic, "$Revision: 1.9.12.1 $");

typedef std::pair< vtkDataTransfer *, vtkMRMLNode * > TransferNodePair;

//----------------------------------------------------------------------------
vtkDataIOManagerLogic::vtkDataIOManagerLogic()
{
  this->DataIOManager = NULL;
}


//----------------------------------------------------------------------------
vtkDataIOManagerLogic::~vtkDataIOManagerLogic()
{
  if ( this->DataIOManager )
    {
    this->SetAndObserveDataIOManager ( NULL );
    this->DataIOManager->Delete();
    this->DataIOManager = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "DataIOManager: " << this->GetDataIOManager() << "\n";
}

//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{

  vtkDataIOManager *m = vtkDataIOManager::SafeDownCast ( caller );
  if ( m != NULL )
    {
    vtkMRMLNode *node = reinterpret_cast <vtkMRMLNode *> (callData);
    // ignore node events that aren't volumes or slice nodes
    if ( (node != NULL) && (event == vtkDataIOManager::RemoteReadEvent ) )
      {
      vtkDebugMacro("ProcessMRMLEvents: calling queue read on the node " << node->GetID());
      this->QueueRead ( node );
      }  
    else if ( (node != NULL) && (event == vtkDataIOManager::RemoteWriteEvent ) )
      {
      vtkDebugMacro("ProcessMRMLEvents: calling queue write on teh node " << node->GetID());
      this->QueueWrite ( node );
      }
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::SetAndObserveDataIOManager ( vtkDataIOManager *iomanager )
{
  //--- remove all observers and delete if we need to reset the iomanager
  if ( this->DataIOManager!= NULL )
    {
    vtkSetAndObserveMRMLNodeMacro ( this->DataIOManager, NULL );
    }
  //--- if we're resetting to a new value
  if ( iomanager != NULL )
    {
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue ( vtkDataIOManager::RemoteReadEvent );
    events->InsertNextValue ( vtkDataIOManager::RemoteWriteEvent );
    events->InsertNextValue ( vtkDataIOManager::LocalReadEvent );
    events->InsertNextValue ( vtkDataIOManager::LocalWriteEvent );
    vtkSetAndObserveMRMLNodeEventsMacro ( this->DataIOManager, iomanager, events );
    events->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::AddNewDataTransfer ( vtkDataTransfer *dt, vtkMRMLNode *node )
{
  if ( this->GetDataIOManager() != NULL )
    {
    this->GetDataIOManager()->AddNewDataTransfer ( dt, node );
    }
}

//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::DeleteDataTransferFromCache ( vtkDataTransfer *dt )
{
  if ( dt != NULL )
    {
    vtkDataIOManager *dm = this->GetDataIOManager();
    if ( dm )
      {
      vtkCacheManager *cm = dm->GetCacheManager();
      if ( cm != NULL )
        {
        if ( cm->CachedFileExists( dt->GetDestinationURI() ) )
          {
          cm->DeleteFromCache ( dt->GetDestinationURI() );        
          dt->SetTransferCached (0);
          cm->InvokeEvent ( vtkCacheManager::CacheDeleteEvent );
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ClearCache ()
{
  vtkDataIOManager *dm = this->GetDataIOManager();
  if ( dm )
    {
    vtkCacheManager *cm = dm->GetCacheManager();
    if ( cm != NULL )
      {
      cm->ClearCache();
      dm->AllTransfersClearedFromCache();
      cm->InvokeEvent ( vtkCacheManager::CacheClearEvent );
      }
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::CancelDataTransfer ( vtkDataTransfer *dt )
{
  if ( dt != NULL )
    {
    dt->SetCancelRequested ( 1 );
    dt->SetTransferStatus ( vtkDataTransfer::CancelPending );
    vtkDataIOManager *dm = this->GetDataIOManager();
    }
}





//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueRead ( vtkMRMLNode *node )
{

  //--- do some node nullchecking first.
  if ( node == NULL )
    {
    vtkErrorMacro("QueueRead: null input node!");
    return 0;
    }
  vtkMRMLStorableNode *dnode = vtkMRMLStorableNode::SafeDownCast ( node );
  if ( dnode == NULL )
    {
    vtkErrorMacro("QueueRead: unable to cast input mrml node " << node->GetID() << " to a storable node");
    return 0;
    }

  if ( dnode->GetStorageNode() == NULL )
    {
    vtkErrorMacro("QueueRead: unable to get storage node from the storable node " << dnode->GetID() << ", returning");
    return 0;
    }

  //--- if handler is good and there's enough cache space, queue the read
  vtkURIHandler *handler = dnode->GetStorageNode()->GetURIHandler();
  if ( handler == NULL)
    {
    vtkErrorMacro("QueueRead: null URI handler!");
    return 0;
    }
  else
    {
    vtkDebugMacro("QueueRead: got the handler");
    }
  if ( this->DataIOManager == NULL )
    {
    vtkErrorMacro("QueueRead: DataIOManager is null");
    return 0;
    }

  vtkCacheManager *cm = this->GetDataIOManager()->GetCacheManager();
  if ( cm == NULL )
    {
    vtkErrorMacro("QueueRead: CacheManager is null");
    return 0;
    }

  const char *source = dnode->GetStorageNode()->GetURI();
  const char *dest = cm->GetFilenameFromURI ( source );
  vtkDebugMacro("QueueRead: got the source " << source << " and dest " << dest);

  //--- set the destination filename in the node.
  dnode->GetStorageNode()->SetFileName ( dest );
  
  //--- if the filename already exists in cache and
  //--- user has selected not to redownload cached files
  //--- just return.
  if ( (cm->CachedFileExists ( dest )) && ( !(cm->GetEnableForceRedownload())) )
    {
    dnode->GetStorageNode()->SetReadStateReady();
    vtkDebugMacro("QueueRead: the destination file is there and we're not forceing redownload");
    return 1;
    }

  //--- Otherwise, just do the data transfer whether
  //--- the file already exists in cache or not
  //--- (download or redownload)
  //---
  //--- TODO: build out the logic to handle creating
  //--- new versions of the dataset in cache.
  
  //--- construct and add a record of the transfer
  //--- which includes the ID of associated node
  vtkDataTransfer *transfer = vtkDataTransfer::New();
  if ( transfer == NULL )
    {
    vtkErrorMacro("QueueRead: failed to add new data transfer");
    return 0;
    }
  transfer->SetTransferID ( this->GetDataIOManager()->GetUniqueTransferID() );
  transfer->SetTransferNodeID ( node->GetID() );
  transfer->SetSourceURI ( source );
  transfer->SetDestinationURI ( dest );
  transfer->SetHandler ( handler );
  transfer->SetTransferType ( vtkDataTransfer::RemoteDownload );
  transfer->SetTransferStatus ( vtkDataTransfer::Idle );
  transfer->SetCancelRequested ( 0 );
  this->AddNewDataTransfer ( transfer, node );
  
  vtkDebugMacro("QueueRead: asynchronous enabled = " << this->GetDataIOManager()->GetEnableAsynchronousIO());
  
  if ( this->GetDataIOManager()->GetEnableAsynchronousIO() )
    {
    vtkDebugMacro("QueueRead: Schedule an ASYNCHRONOUS data transfer");
    //---
    //--- Schedule an ASYNCHRONOUS data transfer
    //---
    vtkSlicerTask *task = vtkSlicerTask::New();
    // Pass the current data transfer, which has a pointer 
    // to the associated mrml node, as client data to the task.
    if ( !task )
      {
      transfer->Delete();
      return 0;
      }
    transfer->SetTransferStatus ( vtkDataTransfer::Pending );
    task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                          &vtkDataIOManagerLogic::ApplyTransfer, transfer);
  
    // Schedule the transfer
    bool ret = 0;
    ret = this->GetApplicationLogic()->ScheduleTask( task );
    if ( !ret  )
      {
      transfer->SetTransferStatus( vtkDataTransfer::CompletedWithErrors);
      }
    task->Delete();
    if ( !ret )
      {
      transfer->Delete();
      return 0;
      }
    }
  else
    {
    vtkDebugMacro("QueueRead: Schedule a SYNCHRONOUS data transfer");
    //---
    //--- Execute a SYNCHRONOUS data transfer
    //---
    transfer->SetTransferStatus( vtkDataTransfer::Running);
    this->ApplyTransfer ( transfer );
    transfer->SetTransferStatus( vtkDataTransfer::Completed);
    // now set the node's storage node state to ready
    dnode->GetStorageNode()->SetReadStateReady();
    }
  transfer->Delete();
  return 1;
}








//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueWrite ( vtkMRMLNode *node )
{
    //--- do some node nullchecking first.
  if ( node == NULL )
    {
    vtkErrorMacro("QueueWrite: null input node!");
    return 0;
    }
  vtkMRMLStorableNode *dnode = vtkMRMLStorableNode::SafeDownCast ( node );
  if ( dnode == NULL )
    {
    vtkErrorMacro("QueueWrite: unable to cast input mrml node " << node->GetID() << " to a storable node");
    return 0;
    }

  if ( dnode->GetStorageNode() == NULL )
    {
    vtkErrorMacro("QueueWrite: unable to get storage node from the storable node " << dnode->GetID() << ", returning");
    return 0;
    }

  //--- if handler is good and there's enough cache space, queue the read
  vtkURIHandler *handler = dnode->GetStorageNode()->GetURIHandler();
  if ( handler == NULL)
    {
    vtkErrorMacro("QueueWrite: null URI handler!");
    return 0;
    }
  else
    {
    vtkDebugMacro("QueueWrite: got the handler");
    }
  if ( this->DataIOManager == NULL )
    {
    vtkErrorMacro("QueueWrite: DataIOManager is null");
    return 0;
    }

  vtkCacheManager *cm = this->GetDataIOManager()->GetCacheManager();
  if ( cm == NULL )
    {
    vtkErrorMacro("QueueWrite: CacheManager is null");
    return 0;
    }

  // this may just have to be GetFileName()
  const char *source = dnode->GetStorageNode()->GetFileName();
  const char *dest = dnode->GetStorageNode()->GetURI();
  vtkDebugMacro("QueueWrite: got the source " << source << " and dest " << dest);
  
  // don't need to check the cache manager

  //--- Otherwise, just do the data transfer 
  //---
  //--- construct and add a record of the transfer
  //--- which includes the ID of associated node
  vtkDataTransfer *transfer = vtkDataTransfer::New();
  if ( transfer == NULL )
    {
    vtkErrorMacro("QueueWrite: failed to add new data transfer");
    return 0;
    }
  transfer->SetTransferID ( this->GetDataIOManager()->GetUniqueTransferID() );
  transfer->SetTransferNodeID ( node->GetID() );
  transfer->SetSourceURI ( source );
  transfer->SetDestinationURI ( dest );
  transfer->SetHandler ( handler );
  transfer->SetTransferType ( vtkDataTransfer::RemoteUpload );
  transfer->SetTransferStatus ( vtkDataTransfer::Idle );
  transfer->SetCancelRequested ( 0 );
  this->AddNewDataTransfer ( transfer, node );
  
  vtkDebugMacro("QueueWrite: asynchronous enabled = " << this->GetDataIOManager()->GetEnableAsynchronousIO());
  
  if ( this->GetDataIOManager()->GetEnableAsynchronousIO() )
    {
    vtkErrorMacro("QueueWrite: NOT IMPLEMENTED to schedule an ASYNCHRONOUS data transfer");
    }
  else
    {
    vtkDebugMacro("QueueWrite: Schedule a SYNCHRONOUS data transfer");
    //---
    //--- Execute a SYNCHRONOUS data transfer
    //---
    transfer->SetTransferStatus( vtkDataTransfer::Running);
    this->ApplyTransfer ( transfer );
    transfer->SetTransferStatus( vtkDataTransfer::Completed);
    // now set the node's storage node state to ready
    dnode->GetStorageNode()->SetWriteStateReady();
    }
  transfer->Delete();
  return 1;
}




//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ApplyTransfer( void *clientdata )
{

  //--- node is on the input
  if ( clientdata == NULL )
    {
    vtkErrorMacro ( "No transfer target was found");
    return;
    }

  //--- get the DataTransfer from the clientdata
  vtkDataTransfer *dt = reinterpret_cast < vtkDataTransfer*> (clientdata);
  if ( dt == NULL )
    {
    vtkErrorMacro("ApplyTransfer: data transfer is null");
    return;
    }

  //assume synchronous io if no data manager exists.
  int asynchIO = 0;
  vtkDataIOManager *iom = this->GetDataIOManager();
  if (iom != NULL)
    {
    asynchIO = iom->GetEnableAsynchronousIO();
    }

  
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID ((dt->GetTransferNodeID() ));
  if ( node == NULL )
    {
    vtkErrorMacro("ApplyTransfer: can't get mrml node from transfer node id " << dt->GetTransferNodeID());
    return;
    }

  const char *source = dt->GetSourceURI();
  const char *dest = dt->GetDestinationURI();
  if ( dt->GetTransferType() == vtkDataTransfer::RemoteDownload  )
    {
    //---
    //--- Download data
    //---
     vtkURIHandler *handler = dt->GetHandler();
     if ( handler != NULL && source != NULL && dest != NULL )
      {
      if ( asynchIO && dt->GetTransferStatus() == vtkDataTransfer::Pending)
        {
        dt->SetTransferStatusNoModify ( vtkDataTransfer::Running );
        this->GetApplicationLogic()->RequestModified( dt );
        handler->StageFileRead( source, dest);
        dt->SetTransferStatusNoModify ( vtkDataTransfer::Completed );
        this->GetApplicationLogic()->RequestModified( dt );

        vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast( node );
        if ( !storableNode )
          {
          vtkErrorMacro( "could not get storable node for scheduled data transfer" );
          return;
          }
        vtkMRMLStorageNode *storageNode = storableNode->GetStorageNode();
        if ( !storageNode )
          {
          vtkErrorMacro( "no storage node for scheduled data transfer" );
          return;
          }
        storageNode->SetDisableModifiedEvent( 1 );
        storageNode->SetReadStateReady();
        storageNode->SetDisableModifiedEvent( 0 );
        this->GetApplicationLogic()->RequestReadData( node->GetID(), dest, 0, 0 );
        }
      else
        {
        vtkDebugMacro("ApplyTransfer: stage file read on the handler..., source = " << source << ", dest = " << dest);
        handler->StageFileRead( source, dest);
        }
      }
    }
  else if ( dt->GetTransferType() == vtkDataTransfer::RemoteUpload  )
    {
    //---
    //--- Upload data
    //---
    vtkURIHandler *handler = dt->GetHandler();
    if ( handler != NULL && source != NULL && dest != NULL )
      {
      if ( asynchIO && dt->GetTransferStatus() == vtkDataTransfer::Pending)
        {
        dt->SetTransferStatusNoModify ( vtkDataTransfer::Running );
        this->GetApplicationLogic()->RequestModified( dt );
        handler->StageFileWrite( source, dest);
        dt->SetTransferStatusNoModify ( vtkDataTransfer::Completed );
        this->GetApplicationLogic()->RequestModified( dt );

        vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast( node );
        if ( !storableNode )
          {
          vtkErrorMacro( "ApplyTransfer: Upload: could not get storable node for scheduled data transfer" );
          return;
          }
        vtkMRMLStorageNode *storageNode = storableNode->GetStorageNode();
        if ( !storageNode )
          {
          vtkErrorMacro( "ApplyTransfer: Upload: no storage node for scheduled data transfer" );
          return;
          }
        storageNode->SetDisableModifiedEvent( 1 );
        storageNode->SetWriteStateReady();
        storageNode->SetDisableModifiedEvent( 0 );
        this->GetApplicationLogic()->RequestWriteData( node->GetID(), dest, 0, 0 );
        }
      else
        {
        vtkDebugMacro("ApplyTransfer: Upload: stage file write on the handler, source = " << source << ", dest = " << dest);
        handler->StageFileWrite( source, dest);
        }
      }
    else
      {
      vtkErrorMacro("ApplyTransfer: Upload: either no handler, or source or dest are null.");
      }
    }
  else
    {
    vtkErrorMacro("ApplyTransfer: unknonw transfer type " <<  dt->GetTransferType() );
    }
}



//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ProgressCallback ( void *who )
{

  //---TODO: figure out how to make this guy work and wire him into the rest of the mechanism
    TransferNodePair *lnp = reinterpret_cast<TransferNodePair*>(who);
    //--- get a pointer back to DataIOManagerLogic (?) dunno.
    //--- use its RequestModified method to call update on the DataIOManager (?)
    // lnp->first->RequestModified(lnp->second);

}
