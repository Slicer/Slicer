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

  //--- do some node nullchecking first.
  if ( node == NULL )
    {
    return 0;
    }
  vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast ( node );
  if ( dnode == NULL )
    {
    return 0;
    }

  //--- if handler is good and there's enough cache space, queue the read
  vtkURIHandler *handler = dnode->GetStorageNode()->GetURIHandler();
  if ( handler == NULL)
    {
    return 0;
    }
  const char *source = dnode->GetStorageNode()->GetURI();
  const char *dest = this->GetDataIOManager()->GetCacheManager()->GetFilenameFromURI ( source );

  //--- construct and add a record of the transfer
  //--- which includes the ID of associated node
  vtkDataTransfer *dt = this->GetDataIOManager()->AddNewDataTransfer ( node );
  if ( dt == NULL )
    {
    return 0;
    }
  dt->SetSourceURI ( source );
  dt->SetDestinationURI ( dest );
  dt->SetHandler ( handler );
  dt->SetTransferType ( vtkDataTransfer::RemoteDownload );
  this->GetDataIOManager()->SetTransferStatus ( dt, vtkDataTransfer::Unspecified, true );
  this->ScheduleRead ( dt );
  return 1;

}




//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ScheduleRead (  vtkDataTransfer *transfer )
{
   bool ret;

   if ( this->GetDataIOManager()->GetEnableAsynchronousIO() )
     {

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
   else
     {
     this->ApplyReadAndWait ( transfer );
     }

}



//----------------------------------------------------------------------------
void vtkDataIOManagerLogic:: ApplyReadAndWait ( vtkDataTransfer *transfer )
{
  // Just execute and wait.
  //node->Register(this);
  if ( transfer != NULL )
    {
    vtkDataIOManagerLogic::ApplyTransfer ( transfer );
    }
}




//----------------------------------------------------------------------------
int vtkDataIOManagerLogic::QueueWrite ( vtkMRMLNode *node )
{
  return 0;
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ScheduleWrite( vtkDataTransfer *transfer )
{
}


//----------------------------------------------------------------------------
void vtkDataIOManagerLogic::ApplyWriteAndWait ( vtkDataTransfer *transfer )
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
  if ( dt == NULL )
    {
    return;
    }

  
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID ((dt->GetTransferNodeID() ));
  if ( node == NULL )
    {
    return;
    }

  std::string cmdString = "node->GetStorageNode()->ReadData()";
  char **command;
  if ( dt->GetTransferType() == vtkDataTransfer::RemoteDownload  )
    {
    //--- create a ReadData command
    }
  else if ( dt->GetTransferType() == vtkDataTransfer::RemoteUpload  )
    {
    //--- create a WriteData command
    }

  //--- An example of what we want here is in vtkCommandLineModuleLogic.cxx.
  //--- it appears as though, for SharedObjectModules, each module's
  //--- ProcessInformation contains certain definitions about EntryPoints, etc.
  //--- These seem to be set up in the ModuleFactory, using kwsys DynamicLoader
  //--- to open a library, get relevant things (?) symbol addresses, and creates entry points.
  //--- Then, CommandLineModuleLogic builds a command string, and a
  //--- 'commandLineAsString' using the processinformation,
  //--- and tries to run the module using the a call like that below:
  // if (entryPoint != NULL ) {
  // (*entryPoint)commandLineAsString.size(), command);}
  
  //--- TODO: set up some kind of progress feedback...
  // Set the callback for progress.  This will only be used for the
  // scope of this function.
  //--- TODO: kill self. How will this work?
  //TransferNodePair lnp( dt, node );
  //node->GetModuleDescription().GetProcessInformation()->SetProgressCallback( vtkDataIOManagerLogic::ProgressCallback, &lnp );
  // Check for Cancelled!
  // Check for timeout!


  // Check for Cancelled
  // Check for timeout

/*
  if ( all went well...)
    {
    node->SetStatus(vtkMRMLCommandLineModuleNode::Completed, false);
    this->GetApplicationLogic()->RequestModified( node );
    }
*/
  
  // clean up
  delete [] command;
  //  node->Unregister ( this );
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
