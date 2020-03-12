
// Logic includes
#include "vtkDataIOManagerLogic.h"

// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkPermissionPrompter.h"
#include "vtkMRMLScene.h"
#include <vtkURIHandler.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// ITKsys includes

// STD includes
#include <cassert>

#ifdef linux
#include "unistd.h"
#endif

#ifdef _WIN32
#else
#include <sys/types.h>
#include <unistd.h>
#endif

//----------------------------------------------------------------------------
#ifndef vtkSetAndObserveDataIOManagerEventsMacro
#define vtkSetAndObserveDataIOManagerEventsMacro(node,value,events) {         \
  vtkObject *_oldNode = (node);                                               \
  this->GetDataIOObserverManager()->SetAndObserveObjectEvents(                \
     vtkObjectPointer(&(node)), (value), (events));                           \
  vtkObject *_newNode = (node);                                               \
  if (_oldNode != _newNode)                                                   \
    {                                                                         \
    this->Modified();                                                         \
    }                                                                         \
};
#endif

vtkStandardNewMacro ( vtkDataIOManagerLogic );

typedef std::pair< vtkDataTransfer *, vtkMRMLNode * > TransferNodePair;

//----------------------------------------------------------------------------
vtkDataIOManagerLogic::vtkDataIOManagerLogic()
{
  this->DataIOManager = nullptr;

  this->DataIOObserverManager = vtkObserverManager::New();
  this->DataIOObserverManager->GetCallbackCommand()->SetClientData(this);
  this->DataIOObserverManager->GetCallbackCommand()->SetCallback(
    vtkDataIOManagerLogic::DataIOManagerCallback);
}

//----------------------------------------------------------------------------
vtkDataIOManagerLogic::~vtkDataIOManagerLogic()
{
  if ( this->DataIOManager )
    {
    this->SetAndObserveDataIOManager ( nullptr );
    if (this->DataIOManager)
      {
      this->DataIOManager->Delete();
      this->DataIOManager = nullptr;
      }
    }
  if (this->DataIOObserverManager)
    {
    this->DataIOObserverManager->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "DataIOManager: " << this->GetDataIOManager() << "\n";
}

//----------------------------------------------------------------------------
vtkObserverManager* vtkDataIOManagerLogic::GetDataIOObserverManager()
{
  return this->DataIOObserverManager;
}

//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::DataIOManagerCallback(
  vtkObject* caller, unsigned long eid, void* clientData, void* callData)
{
  vtkDataIOManagerLogic *self = reinterpret_cast<vtkDataIOManagerLogic *>(clientData);
  assert(vtkDataIOManager::SafeDownCast(caller));
  self->ProcessDataIOManagerEvents(caller, eid, callData);
}

//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ProcessDataIOManagerEvents(
#ifndef NDEBUG
  vtkObject *caller,
#else
  vtkObject *vtkNotUsed(caller),
#endif
  unsigned long event, void *callData)
{
  assert(vtkDataIOManager::SafeDownCast( caller ));
  vtkMRMLNode *node = reinterpret_cast <vtkMRMLNode *> (callData);
  // ignore node events that aren't volumes or slice nodes
  if ( (node != nullptr) && (event == vtkDataIOManager::RemoteReadEvent ) )
    {
    vtkDebugMacro("ProcessMRMLEvents: calling queue read on the node " << node->GetID());
    this->QueueRead ( node );
    node->InvokeEvent ( vtkDataIOManager::RefreshDisplayEvent );
    }
  else if ( (node != nullptr) && (event == vtkDataIOManager::RemoteWriteEvent ) )
    {
    vtkDebugMacro("ProcessMRMLEvents: calling queue write on the node " << node->GetID());
    this->QueueWrite ( node );
    node->InvokeEvent ( vtkDataIOManager::RefreshDisplayEvent );
    }
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::SetAndObserveDataIOManager ( vtkDataIOManager *iomanager )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue ( vtkDataIOManager::RemoteReadEvent );
  events->InsertNextValue ( vtkDataIOManager::RemoteWriteEvent );
  events->InsertNextValue ( vtkDataIOManager::LocalReadEvent );
  events->InsertNextValue ( vtkDataIOManager::LocalWriteEvent );
  vtkSetAndObserveDataIOManagerEventsMacro( this->DataIOManager, iomanager, events.GetPointer() );
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::AddNewDataTransfer ( vtkDataTransfer *dt, vtkMRMLNode *node )
{
  if ( this->GetDataIOManager() != nullptr )
    {
    this->GetDataIOManager()->AddNewDataTransfer ( dt, node );
    }
}

//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::DeleteDataTransferFromCache ( vtkDataTransfer *dt )
{
  if ( dt != nullptr )
    {
    vtkDataIOManager *dm = this->GetDataIOManager();
    if ( dm )
      {
      vtkCacheManager *cm = dm->GetCacheManager();
      if ( cm != nullptr )
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
    if ( cm != nullptr )
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
  if ( dt != nullptr )
    {
    dt->SetCancelRequested ( 1 );
    dt->SetTransferStatus ( vtkDataTransfer::CancelPending );
    }
}





//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueRead ( vtkMRMLNode *node )
{
//  this->DebugOn();
  //--- do some node nullchecking first.
  if ( node == nullptr )
    {
    vtkErrorMacro("QueueRead: null input node!");
    return 0;
    }
  vtkMRMLStorableNode *dnode = vtkMRMLStorableNode::SafeDownCast ( node );
  if ( dnode == nullptr )
    {
    vtkErrorMacro("QueueRead: unable to cast input mrml node " << node->GetID() << " to a storable node");
    return 0;
    }

  if ( dnode->GetStorageNode() == nullptr )
    {
    vtkErrorMacro("QueueRead: unable to get storage node from the storable node " << dnode->GetID() << ", returning");
    return 0;
    }
  //--- find the storage node that's in the scheduled state
  int numStorageNodes = dnode->GetNumberOfStorageNodes();
  int storageNodeIndex = -1;
  int numScheduledNodes = 0;
  for (int n = 0; n < numStorageNodes; n++)
    {
    if (dnode->GetNthStorageNode(n)->GetReadState() == vtkMRMLStorageNode::Scheduled)
      {
      vtkDebugMacro("QueueRead: found storage node in scheduled state, n = " << n);
      if (storageNodeIndex == -1)
        {
        storageNodeIndex = n;
        }
      numScheduledNodes++;
      }
    }
  vtkDebugMacro("QueueRead: there are " << numScheduledNodes << " storage nodes scheduled to be read on " << node->GetID());
  if (storageNodeIndex == -1)
    {
    vtkDebugMacro("QueueRead: no storage nodes found in scheduled state, checked " << numStorageNodes);
    return 0;
    }
  else
    {
    // set it to working, the storage node will wait to read until it's ready
    // (ApplyTransfer will set it to ready when it's done)
    vtkDebugMacro("QueueRead: setting " << storageNodeIndex << " storage node read state to working, uri = " <<  dnode->GetNthStorageNode(storageNodeIndex)->GetURI());
    dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateTransferring();
    }

  //--- if handler is good and there's enough cache space, queue the read
  vtkURIHandler *handler = dnode->GetNthStorageNode(storageNodeIndex)->GetURIHandler();
  if ( handler == nullptr)
    {
    vtkErrorMacro("QueueRead: null URI handler!");
    return 0;
    }
  else
    {
    vtkDebugMacro("QueueRead: got the handler");
    }
  if ( this->DataIOManager == nullptr )
    {
    vtkErrorMacro("QueueRead: DataIOManager is null");
    return 0;
    }

  vtkCacheManager *cm = this->GetDataIOManager()->GetCacheManager();
  if ( cm == nullptr )
    {
    vtkErrorMacro("QueueRead: CacheManager is null");
    return 0;
    }


  const char *source = dnode->GetNthStorageNode(storageNodeIndex)->GetURI();
  const char *dest = cm->GetFilenameFromURI ( source );
  vtkDebugMacro("QueueRead: got the source " << source << " and dest " << dest);


  //--- set the destination filename in the node.
  dnode->GetNthStorageNode(storageNodeIndex)->SetFileName ( dest );

  // -- now loop over any uri list and set the filenames
  dnode->GetNthStorageNode(storageNodeIndex)->ResetFileNameList();
  bool allCachedFilesExist = true;
  for (int uriNum = 0; uriNum < dnode->GetNthStorageNode(storageNodeIndex)->GetNumberOfURIs(); uriNum++)
    {
    const char *sourceN =  dnode->GetNthStorageNode(storageNodeIndex)->GetNthURI(uriNum);
    if (sourceN)
      {
      const char *destN = cm->GetFilenameFromURI(sourceN);
      if (destN)
        {
        dnode->GetNthStorageNode(storageNodeIndex)->AddFileName (destN);
        vtkDebugMacro("QueueRead: set " << uriNum << " filename to " << destN << ", source uri = " << sourceN);
        // check if it exists
        if (!cm->CachedFileExists(destN))
          {
          allCachedFilesExist = false;
          }
        }
      }
    }
  //---
  //--- WJPtest:
  //--- Again, test for space to download the file.
  //--- This test has been done in MRML (DataIOManager), but with asynchIO,
  //--- Cache may have become full since the remote read was queued.
  //---
  float bufsize = (cm->GetRemoteCacheLimit() * 1000000.0) -  (cm->GetRemoteCacheFreeBufferSize() * 1000000.0);
  if ( (cm->GetCurrentCacheSize()*1000000.0) >= bufsize )
    {
    //--- No space left in cache.
    if ( cm->CachedFileExists (dest) )
      {
      //--- Load the cached version as a last resort.
      dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateTransferDone();
      }
    else
      {
      //--- Mark the node's read state as cancelled.
      dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateCancelled();
      }
    //--- Invoke an event that will trigger GUI to post
    //--- a message box telling if one hasn't been posted already
    //--- user that there's insufficient space in the cache,
    //--- to download new data, but that Slicer is loading
    //--- a cached version IF one is available.
    if ( cm->GetInsufficientFreeBufferNotificationFlag() == 0 )
      {
      cm->InvokeEvent ( vtkCacheManager::InsufficientFreeBufferEvent );
      cm->SetInsufficientFreeBufferNotificationFlag(1);
      }
    return 1;
    }
  //---
  //---END WJPtest

  ///---
  //--- if the filename already exists in cache and
  //--- user has selected not to redownload cached files
  //--- just return.
  if ( (cm->CachedFileExists ( dest ) ) &&
       allCachedFilesExist &&
       ( !(cm->GetEnableForceRedownload())) )
    {
    dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateTransferDone();
    vtkDebugMacro("QueueRead: the destination file is there and we're not forceing redownload");
    return 1;
    }

  //--- Otherwise, just do the data transfer whether
  //--- the file already exists in cache or not
  //--- (download or redownload)
  //---
  //--- TODO: build out the logic to handle creating
  //--- new versions of the dataset in cache.

  //--- if permissions are required, invoke the permissions prompter.
  int retval = -1;
  if ( handler->GetPermissionPrompter() != nullptr )
    {
    while (retval < 0 )
      {
      //--- keep prompting until user provides all information, or user cancels.
      retval = handler->GetPermissionPrompter()->Prompt(nullptr);
      }
    }
  if ( retval == 0)
    {
    //--- no permission fields were completed.
    //--- Transfer should be cancelled -- how do we do this?
    dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateCancelled();
    vtkDebugMacro("QueueRead: cancelling data transfer.");
    return 0;
    }

  //--- construct and add a record of the transfer
  //--- which includes the ID of associated node
  vtkNew<vtkDataTransfer> transfer0;
  transfer0->SetTransferID ( this->GetDataIOManager()->GetUniqueTransferID() );
  transfer0->SetTransferNodeID ( node->GetID() );
  transfer0->SetSourceURI ( source );
  transfer0->SetDestinationURI ( dest );
  transfer0->SetHandler ( handler );
  transfer0->SetTransferType ( vtkDataTransfer::RemoteDownload );
  transfer0->SetTransferStatus ( vtkDataTransfer::Idle );
  transfer0->SetCancelRequested ( 0 );
  //--- Add the data transfer to the collection, and
  //--- the resulting mrml call will trigger an event
  //--- that causes GUI to refresh.
  this->AddNewDataTransfer ( transfer0.GetPointer(), node );
  this->GetDataIOManager()->InvokeEvent ( vtkDataIOManager::RefreshDisplayEvent );

  vtkDebugMacro("QueueRead: asynchronous enabled = " << this->GetDataIOManager()->GetEnableAsynchronousIO());

  if ( this->GetDataIOManager()->GetEnableAsynchronousIO() )
    {
    vtkDebugMacro("QueueRead: Schedule an ASYNCHRONOUS data transfer");
    //---
    //--- Schedule an ASYNCHRONOUS data transfer
    //---
    vtkNew<vtkSlicerTask> task;
    task->SetTypeToNetworking();
    transfer0->SetTransferStatus ( vtkDataTransfer::Pending );
    task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                          &vtkDataIOManagerLogic::ApplyTransfer, transfer0.GetPointer());

    // Schedule the transfer
    if ( ! this->GetApplicationLogic()->ScheduleTask( task.GetPointer() ) )
      {
      transfer0->SetTransferStatus( vtkDataTransfer::CompletedWithErrors);
      return 0;
      }
    }
  else
    {
    vtkDebugMacro("QueueRead: Schedule a SYNCHRONOUS data transfer");
    //---
    //--- Execute a SYNCHRONOUS data transfer
    //---
    transfer0->SetTransferStatus( vtkDataTransfer::Running);
    this->ApplyTransfer ( transfer0.GetPointer() );
    transfer0->SetTransferStatus( vtkDataTransfer::Completed);
    // now set the node's storage node state to ready
    vtkDebugMacro("QueueRead: setting storage node state to transferdone: " << dnode->GetNthStorageNode(storageNodeIndex)->GetURI());
    if (dnode->GetNthStorageNode(storageNodeIndex)->GetNumberOfURIs() == 0)
      {
      // done
      dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateTransferDone();
      }
    }
//  this->DebugOff();

  // loop over any other files in the storage node
  for (int n = 0; n < dnode->GetNthStorageNode(storageNodeIndex)->GetNumberOfURIs(); n++)
    {
    const char *sourceN =  dnode->GetNthStorageNode(storageNodeIndex)->GetNthURI(n);
    const char *destN = dnode->GetNthStorageNode(storageNodeIndex)->GetNthFileName(n);

    vtkNew<vtkDataTransfer> transfer1;
    transfer1->SetTransferID ( this->GetDataIOManager()->GetUniqueTransferID() );
    transfer1->SetTransferNodeID ( node->GetID() );
    transfer1->SetSourceURI ( sourceN );
    transfer1->SetDestinationURI ( destN );
    // use one handler for all files in the storage node
    transfer1->SetHandler ( handler );
    transfer1->SetTransferType ( vtkDataTransfer::RemoteDownload );
    transfer1->SetTransferStatus ( vtkDataTransfer::Idle );
    transfer1->SetCancelRequested ( 0 );
    this->AddNewDataTransfer ( transfer1.GetPointer(), node );
    this->GetDataIOManager()->InvokeEvent ( vtkDataIOManager::RefreshDisplayEvent );

    if ( this->GetDataIOManager()->GetEnableAsynchronousIO() )
      {
      vtkDebugMacro("QueueRead: Schedule an ASYNCHRONOUS data transfer, n = " << n);
      vtkNew<vtkSlicerTask> task;
      task->SetTypeToNetworking();
      transfer1->SetTransferStatus ( vtkDataTransfer::Pending );
      task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                            &vtkDataIOManagerLogic::ApplyTransfer, transfer1.GetPointer());

      // Schedule the transfer
      if ( ! this->GetApplicationLogic()->ScheduleTask( task.GetPointer() ) )
        {
        transfer1->SetTransferStatus( vtkDataTransfer::CompletedWithErrors);
        return 0;
        }
      }
    else
      {
      vtkDebugMacro("QueueRead: Schedule a SYNCHRONOUS data transfer, n = " << n);
      transfer1->SetTransferStatus( vtkDataTransfer::Running);
      this->ApplyTransfer ( transfer1.GetPointer() );
      transfer1->SetTransferStatus( vtkDataTransfer::Completed);
      // now set the node's storage node state to ready

      }
    }
  if ( dnode->GetNthStorageNode(storageNodeIndex)->GetNumberOfURIs() > 0 &&
       !this->GetDataIOManager()->GetEnableAsynchronousIO())
    {
    vtkDebugMacro("QueueRead: setting storage node state to transferdone after synchronous transfer of all files: " << dnode->GetNthStorageNode(storageNodeIndex)->GetURI());
    dnode->GetNthStorageNode(storageNodeIndex)->SetReadStateTransferDone();
    }

  return 1;
}




//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueWrite ( vtkMRMLNode *node )
{
    //--- do some node nullchecking first.
  if ( node == nullptr )
    {
    vtkErrorMacro("QueueWrite: null input node!");
    return 0;
    }
  vtkMRMLStorableNode *dnode = vtkMRMLStorableNode::SafeDownCast ( node );
  if ( dnode == nullptr )
    {
    vtkErrorMacro("QueueWrite: unable to cast input mrml node " << node->GetID() << " to a storable node");
    return 0;
    }

  if ( dnode->GetStorageNode() == nullptr )
    {
    vtkErrorMacro("QueueWrite: unable to get storage node from the storable node " << dnode->GetID() << ", returning");
    return 0;
    }
  //--- find the storage node in scheduled state
  int storageNodeIndex = -1;
  for (int i = 0; i < dnode->GetNumberOfStorageNodes(); i++)
    {
    if (dnode->GetNthStorageNode(i)->GetWriteState() == vtkMRMLStorageNode::Scheduled)
      {
      storageNodeIndex = i;
      vtkDebugMacro("QueueWrite: found a scheduled storage node at index " << i);
      break;
      }
    }
  if (storageNodeIndex == -1)
    {
    vtkErrorMacro("QueueWrite: Could not find a scheduled storage node.");
    return 0;
    }

  //--- if handler is good and there's enough cache space, queue the read
  vtkURIHandler *handler = dnode->GetNthStorageNode(storageNodeIndex)->GetURIHandler();
  if ( handler == nullptr)
    {
    vtkErrorMacro("QueueWrite: null URI handler!");
    return 0;
    }
  else
    {
    vtkDebugMacro("QueueWrite: got the handler");
    }
  if ( this->DataIOManager == nullptr )
    {
    vtkErrorMacro("QueueWrite: DataIOManager is null");
    return 0;
    }

  vtkCacheManager *cm = this->GetDataIOManager()->GetCacheManager();
  if ( cm == nullptr )
    {
    vtkErrorMacro("QueueWrite: CacheManager is null");
    return 0;
    }

  // Get the number of filenames and URIs associated with this storage node.
  // (assuming they should be equal)
  int numFiles = dnode->GetNthStorageNode(storageNodeIndex)->GetNumberOfFileNames();
  int numURIs = dnode->GetNthStorageNode(storageNodeIndex)->GetNumberOfURIs();
  //--- Note: the storageNode's FileName is repeated in its FileList, as
  //--- FileListMember0 *if* there are multiple files for the dataset.
  //--- However, the storageNode's URI is *not* repeated in its URIList
  //--- as URIListMember0. Would be nice if these things were the same.
  //--- but for now, respect the implementation: the URIList will be one
  //--- entity shorter than the FileNameList.

  std::string src;
  std::string dst;
  if ( numFiles == 0 )
    {
    //--- numFiles will be 0 if there's just one file in the storage node.
    //--- (no additional files in the list.)
    //--- adjust numFiles so we move thru the loop below at least once.
    src = dnode->GetNthStorageNode(storageNodeIndex)->GetFileName();
    dst = dnode->GetNthStorageNode(storageNodeIndex)->GetURI();
    numFiles =1;
    }

  if ( numFiles != (numURIs+1) )
    {
    vtkErrorMacro("QueueWrite: Storage node has different number of FileNames and URIs.");
    return 0;
    }

  std::vector<std::string> pathComponents;

  // schedule the write of each
  for ( int n=0; n < numFiles; n++ )
    {
    if ( n == 0 )
      {
      //--- here we need to join the cache path to the filename.

      src = dnode->GetNthStorageNode(storageNodeIndex)->GetFileName();
      dst = dnode->GetNthStorageNode(storageNodeIndex)->GetURI();
      }
    else
      {
      src.clear();
      dst.clear();
      pathComponents.clear();
      vtksys::SystemTools::SplitPath( cm->GetRemoteCacheDirectory(), pathComponents);
      pathComponents.emplace_back(dnode->GetNthStorageNode(storageNodeIndex)->GetNthFileName(n));
      src = vtksys::SystemTools::JoinPath(pathComponents);
      dst = dnode->GetNthStorageNode(storageNodeIndex)->GetNthURI(n-1);
      }
    //--- Construct and add a record of the transfer
    //--- which includes the ID of associated node
    vtkNew<vtkDataTransfer> transfer;
    transfer->SetTransferID ( this->GetDataIOManager()->GetUniqueTransferID() );
    transfer->SetTransferNodeID ( node->GetID() );
    transfer->SetSourceURI ( src.c_str() );
    transfer->SetDestinationURI ( dst.c_str() );
    transfer->SetHandler ( handler );
    transfer->SetTransferType ( vtkDataTransfer::RemoteUpload );
    transfer->SetTransferStatus ( vtkDataTransfer::Idle );
    transfer->SetCancelRequested ( 0 );
    this->AddNewDataTransfer ( transfer.GetPointer(), node );
    this->GetDataIOManager()->InvokeEvent ( vtkDataIOManager::RefreshDisplayEvent );

    vtkDebugMacro("QueueWrite: asynchronous enabled = " << this->GetDataIOManager()->GetEnableAsynchronousIO());

    if ( this->GetDataIOManager()->GetEnableAsynchronousIO() )
      {
      vtkDebugMacro("QueueWrite: Schedule an ASYNCHRONOUS data transfer");
      //---
      //--- Schedule an ASYNCHRONOUS data transfer
      //---
      vtkNew<vtkSlicerTask> task;
      task->SetTypeToNetworking();
      transfer->SetTransferStatus ( vtkDataTransfer::Pending );
      task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                            &vtkDataIOManagerLogic::ApplyTransfer, transfer.GetPointer());

      // Schedule the transfer
      if ( ! this->GetApplicationLogic()->ScheduleTask( task.GetPointer() ) )
        {
        transfer->SetTransferStatus( vtkDataTransfer::CompletedWithErrors);
        return 0;
        }
      }
    else
      {
      vtkDebugMacro("QueueWrite: Schedule a SYNCHRONOUS data transfer");
      //---
      //--- Execute a SYNCHRONOUS data transfer
      //---
      transfer->SetTransferStatus( vtkDataTransfer::Running);
      this->ApplyTransfer ( transfer.GetPointer() );
      this->GetDataIOManager()->InvokeEvent ( vtkDataIOManager::RefreshDisplayEvent );
      transfer->SetTransferStatus( vtkDataTransfer::Completed);
      // now set the node's storage node state to ready
      dnode->GetNthStorageNode(storageNodeIndex)->SetWriteStateTransferDone();
      }
    }
  return 1;
}




//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ApplyTransfer( void *clientdata )
{

  //--- node is on the input
  if ( clientdata == nullptr )
    {
    vtkErrorMacro ( "No transfer target was found");
    return;
    }

  //--- get the DataTransfer from the clientdata
  vtkDataTransfer *dt = reinterpret_cast < vtkDataTransfer*> (clientdata);
  if ( dt == nullptr )
    {
    vtkErrorMacro("ApplyTransfer: data transfer is null");
    return;
    }

  //assume synchronous io if no data manager exists.
  int asynchIO = 0;
  vtkDataIOManager *iom = this->GetDataIOManager();
  if (iom != nullptr)
    {
    asynchIO = iom->GetEnableAsynchronousIO();
    }


  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID ((dt->GetTransferNodeID() ));
  if ( node == nullptr )
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
     if ( handler != nullptr && source != nullptr && dest != nullptr )
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
          vtkErrorMacro( "ApplyTransfer: could not get storable node for scheduled data transfer" );
          return;
          }
        // find the storage node that's been scheduled  and we're working on it
        int storageNodeIndex = -1;
        for (int i = 0; i < storableNode->GetNumberOfStorageNodes(); i++)
          {
          if (storableNode->GetNthStorageNode(i)->GetReadState() == vtkMRMLStorageNode::Transferring &&
              strcmp(storableNode->GetNthStorageNode(i)->GetURI(),source) == 0)
            {
            vtkDebugMacro("ApplyTransfer: found a working storage node who's uri matches source " << source << " at " << i);
            storageNodeIndex = i;
            break;
            }
          }
        if (storageNodeIndex == -1)
          {
          vtkErrorMacro("ApplyTransfer: unable to find a storage node in scheduled state.");
          }
        vtkMRMLStorageNode *storageNode = storableNode->GetNthStorageNode(storageNodeIndex);
        if ( !storageNode )
          {
          vtkErrorMacro( "ApplyTransfer: no storage node for scheduled data transfer" );
          return;
          }
        storageNode->SetDisableModifiedEvent( 1 );
        // let the storage node know that the remote transfer is done
        vtkDebugMacro("ApplyTransfer: setting storage node read state to transfer done for uri " << storageNode->GetURI());
        storageNode->SetReadStateTransferDone();
        storageNode->SetDisableModifiedEvent( 0 );
        this->GetApplicationLogic()->RequestReadFile( node->GetID(), dest, 0, 0 );
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
    if ( handler != nullptr && source != nullptr && dest != nullptr )
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
        // find the storage node that's scheduled
        int storageNodeIndex = -1;
        for (int i = 0; i < storableNode->GetNumberOfStorageNodes(); i++)
          {
          if (storableNode->GetNthStorageNode(i)->GetWriteState() == vtkMRMLStorageNode::Scheduled)
            {
            storageNodeIndex = i;
            }
          }
        if (storageNodeIndex == -1)
          {
          vtkErrorMacro("ApplyTransfer: upload: unable to find a storage node in scheduled state.");
          }
        vtkMRMLStorageNode *storageNode = storableNode->GetNthStorageNode(storageNodeIndex);
        if ( !storageNode )
          {
          vtkErrorMacro( "ApplyTransfer: Upload: no storage node for scheduled data transfer" );
          return;
          }
        storageNode->SetDisableModifiedEvent( 1 );
        storageNode->SetWriteStateTransferDone();
        storageNode->SetDisableModifiedEvent( 0 );
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
    vtkErrorMacro("ApplyTransfer: unknown transfer type " <<  dt->GetTransferType() );
    }
}



//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ProgressCallback ( void * vtkNotUsed(who) )
{

  //---TODO: figure out how to make this guy work and wire him into the rest of the mechanism
  //    TransferNodePair *lnp = reinterpret_cast<TransferNodePair*>(who);
    //--- get a pointer back to DataIOManagerLogic (?) dunno.
    //--- use its RequestModified method to call update on the DataIOManager (?)
    // lnp->first->RequestModified(lnp->second);

}
