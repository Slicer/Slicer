#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "vtkDataIOManagerLogic.h"
#include "vtkMRMLStorageNode.h"
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
int vtkDataIOManagerLogic::QueueRead ( vtkMRMLNode *node )
{

  //--- do some checking first.
  vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast ( node );
  if ( dnode == NULL )
    {
    return 0;
    }

  vtkCacheManager *cm = this->DataIOManager->GetCacheManager();
  if ( cm != NULL )
    {
    //--- check to see if RemoteCacheLimit is exceeded
    //--- check to see if FreeBufferSize is exceeded.
    }

  vtkURIHandler *handler = dnode->GetStorageNode()->GetURIHandler();
  const char *source = dnode->GetStorageNode()->GetURI();
  const char *dest = this->DataIOManager->GetCacheManager()->GetFilenameFromURI ( source );
    
  //--- if handler is good and there's enough cache space, queue the read
  if ( (handler != NULL) && ( cm->GetCurrentCacheSize() < cm->GetRemoteCacheLimit() ))
    {
    //--- construct and add a record of the transfer
    vtkDataTransfer *dt = this->DataIOManager->AddNewDataTransfer ( node );
    //---TODO: Figure out what's source and what's destination
    dt->SetSourceURI ( source );
    dt->SetDestinationURI ( dest );
    dt->SetHandler ( handler );
    dt->SetTransferType ( vtkDataTransfer::RemoteDownload );
    this->DataIOManager->SetTransferStatus ( dt, vtkDataTransfer::Unspecified, true );

    //--- if force redownload is enabled, remove the old file from cache.
    if (cm->GetEnableForceRedownload () )
      {
      this->DataIOManager->GetCacheManager()->RemoveFromCache ( dest );
      }
    this->ScheduleRead ( node, dt->GetTransferID() );
    return 1;
    }
  else
    {
    return 0;
    }
}



//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ScheduleRead ( vtkMRMLNode *node, int transferID )
{
   bool ret;

   vtkDataTransfer *transfer = this->DataIOManager->GetDataTransfer ( transferID );

   vtkSlicerTask *task = vtkSlicerTask::New();
   // Pass the current data transfer, which has a pointer to the associated
   // mrml node, as client data to the task.
   task->SetTaskFunction(this, (vtkSlicerTask::TaskFunctionPointer)
                        &vtkDataIOManagerLogic::ApplyTransfer, transfer);
  
  // Client data on the task is just a regular pointer, up the
  // reference count on the node, we'll decrease the reference count
  // once the task actually runs
  //  node->Register(this);
  
  // Schedule the transfer
   ret = this->GetApplicationLogic()->ScheduleTask( task );
  
  if (!ret)
    {
    vtkWarningMacro( << "Could not schedule transfer" );
    }
  else
    {
      this->DataIOManager->SetTransferStatus(transfer, vtkDataTransfer::Scheduled, true);
    }
  task->Delete();

}



//----------------------------------------------------------------------------
void vtkDataIOManagerLogic:: ApplyReadAndWait ( vtkMRMLNode *node, int transferID )
{
  // Just execute and wait.
  //node->Register(this);
  vtkDataIOManagerLogic::ApplyTransfer ( node );
}




//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueWrite ( vtkMRMLNode *node )
{
  return 0;
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ScheduleWrite( vtkMRMLNode *node, int transferID )
{
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ApplyWriteAndWait ( vtkMRMLNode *node, int transferID )
{
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

  //--- do we need to reinterpret each specific subclass of displayable node or will this work?
  vtkDataTransfer *dt = reinterpret_cast < vtkDataTransfer*> (clientdata);


/*
  //--- TODO: set up some kind of progress feedback...
  // Set the callback for progress.  This will only be used for the
  // scope of this function.
  //--- TODO: kill self. How will this work?
  TransferNodePair lnp( dt, node );
  node->GetModuleDescription().GetProcessInformation()
    ->SetProgressCallback( vtkDataIOManagerLogic::ProgressCallback, &lnp );

  // Check for Cancelled!
  // Check for timeout!

  //--- encode this as a command: node->GetStorageNode()->ReadData();

  //--- set up thread
  itksysProcess *process = itksysProcess_New();
  itksysProcess_SetCommand ( process, command );
  itksysProcess_SetOption ( process,   itksysProcess_Option_Detach, 0);
  itksysProcess_SetOption ( process,   itksysProcess_Option_HideWindow, 1 );
  itksysProcess_SetTimeout (process, 600.0); //  10 minutes...

  // execute the command
  itksysProcess_Execute ( process );  

  // Check for Cancelled
  // Check for timeout
  
  itksysProcess_WaitForExit(process, 0);
  itksysProcess_Delete(process);
  
  if ( all went well...)
    {
    node->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
    this->GetApplicationLogic()->RequestModified( node );
    }
    
  // clean up
  delete [] command;
  //  node->Unregister ( this );
*/

}



//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ProgressCallback ( void *who )
{

    TransferNodePair *lnp = reinterpret_cast<TransferNodePair*>(who);
    //--- get a pointer back to DataIOManagerLogic (?) dunno.
    //--- use its RequestModified method to call update on the DataIOManager (?)
    // lnp->first->RequestModified(lnp->second);

}
