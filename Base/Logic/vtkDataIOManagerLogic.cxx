#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "vtkDataIOManagerLogic.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLDisplayableNode.h"
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
    vtkSetAndObserveMRMLNodeEventsMacro ( this->DataIOManager, iomanager, events );
    events->Delete();
    }
}


//----------------------------------------------------------------------------
vtkDataTransfer* vtkDataIOManagerLogic::AddNewDataTransfer ( vtkMRMLNode *node )
{
  if ( this->GetDataIOManager() == NULL )
    {
    vtkErrorMacro("AddNewDataTransfer: node is null");
    return (NULL);
    }
  else
    {
    return ( this->GetDataIOManager()->AddNewDataTransfer ( node ));
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
  vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast ( node );
  if ( dnode == NULL )
    {
    vtkErrorMacro("QueueRead: unable to cast input mrml node " << node->GetID() << " to a displayable node");
    return 0;
    }

  if ( dnode->GetStorageNode() == NULL )
    {
    vtkErrorMacro("QueueRead: unable to get storage node from the displayable node " << dnode->GetID() << ", returning");
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
  vtkDataTransfer *transfer = this->AddNewDataTransfer ( node );
  if ( transfer == NULL )
    {
    vtkErrorMacro("QueueRead: failed to add new data transfer");
    return 0;
    }
  transfer->SetSourceURI ( source );
  transfer->SetDestinationURI ( dest );
  transfer->SetHandler ( handler );
  transfer->SetTransferType ( vtkDataTransfer::RemoteDownload );
  transfer->SetTransferStatus ( vtkDataTransfer::Unspecified, true );

  /*
  if (this->GetDataIOManager()->GetEnableAsynchronousIO() )
    {
    vtkWarningMacro("QueueRead: disabling asynch mode!!!");
    this->GetDataIOManager()->SetEnableAsynchronousIO(0);
    }
  */
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
      return 0;
      }
    task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                          &vtkDataIOManagerLogic::ApplyTransfer, transfer);
  
    // Schedule the transfer
    bool ret = 0;
    if (ret = this->GetApplicationLogic()->ScheduleTask( task ) )
      {
      transfer->SetTransferStatus( vtkDataTransfer::Scheduled, true);
      }
    task->Delete();
    if ( !ret )
      {
      return 0;
      }
    }
  else
    {
    vtkDebugMacro("QueueRead: Schedule a SYNCHRONOUS data transfer");
    //---
    //--- Execute a SYNCHRONOUS data transfer
    //---
    this->ApplyTransfer ( transfer );
    // now set the node's storage node state to ready
    dnode->GetStorageNode()->SetReadStateReady();
    }
  return 1;
}








//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueWrite ( vtkMRMLNode *node )
{
  return 0;
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
      if ( asynchIO )
        {
        handler->StageFileRead( source, dest);
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
      if ( asynchIO )
        {
        //handler->StageFileWrite( source, dest);
        }
      else
        {
        //handler->StageFileWrite( source, dest);
        }
      }
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
