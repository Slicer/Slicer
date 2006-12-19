/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
//#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkSlicerTask.h"


#include "itksys/SystemTools.hxx"

#include <queue>

vtkCxxRevisionMacro(vtkSlicerApplicationLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerApplicationLogic);

class ProcessingTaskQueue : public std::queue<vtkSmartPointer<vtkSlicerTask> > {};
class ModifiedQueue : public std::queue<vtkSmartPointer<vtkObject> > {};
class ReadDataRequest
{
public:
  ReadDataRequest(const std::string& node, const std::string& filename,
                  bool displayData, bool deleteFile)
    {
      m_Node = node;
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  ReadDataRequest(const char *node, const char *filename, bool displayData,
                  bool deleteFile)
    {
      m_Node = node;
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  ReadDataRequest()
    : m_Node(""), m_Filename(""), m_DisplayData( false ), m_DeleteFile( false )
    {
    }

  const std::string& GetNode() const { return m_Node; }
  const std::string& GetFilename() const { return m_Filename; }
  bool GetDisplayData() const { return m_DisplayData; }
  bool GetDeleteFile() const { return m_DeleteFile; }
  
protected:
  std::string m_Node;
  std::string m_Filename;
  bool m_DisplayData;
  bool m_DeleteFile;
};
class ReadDataQueue : public std::queue<ReadDataRequest> {} ;



//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::vtkSlicerApplicationLogic()
{
    this->Views = NULL;
    this->Slices = NULL;
    this->Modules = NULL;
    this->ActiveSlice = NULL;
    this->SelectionNode = NULL;

    this->ProcessingThreader = itk::MultiThreader::New();
    this->ProcessingThreadId = -1;
    this->ProcessingThreadActive = false;
    this->ProcessingThreadActiveLock = itk::MutexLock::New();
    this->ProcessingTaskQueueLock = itk::MutexLock::New();

    this->ModifiedQueueActive = false;
    this->ModifiedQueueActiveLock = itk::MutexLock::New();
    this->ModifiedQueueLock = itk::MutexLock::New();

    this->ReadDataQueueActive = false;
    this->ReadDataQueueActiveLock = itk::MutexLock::New();
    this->ReadDataQueueLock = itk::MutexLock::New();

    this->InternalTaskQueue = new ProcessingTaskQueue;
    this->InternalModifiedQueue = new ModifiedQueue;

    this->InternalReadDataQueue = new ReadDataQueue;
    
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::~vtkSlicerApplicationLogic()
{
  if (this->Views)
    {
        this->Views->Delete();
        this->Views = NULL;
    }
  if (this->Slices)
    {
        this->Slices->Delete();
        this->Slices = NULL;
    }
  if (this->Modules)
    {
        this->Modules->Delete();
        this->Modules = NULL;
    }
  this->SetSelectionNode ( NULL );
  this->SetActiveSlice ( NULL );

  // Note that TerminateThread does not kill a thread, it only waits
  // for the thread to finish.  We need to signal the thread that we
  // want to terminate
  if (this->ProcessingThreadId != -1 && this->ProcessingThreader)
    {
    // Signal the processingThread that we are terminating. 
    this->ProcessingThreadActiveLock->Lock();
    this->ProcessingThreadActive = false;
    this->ProcessingThreadActiveLock->Unlock();
    
    // Wait for the thread to finish and clean up the state of the threader
    this->ProcessingThreader->TerminateThread( this->ProcessingThreadId );
    
    this->ProcessingThreadId = -1;
    }
  
  delete this->InternalTaskQueue;
  this->InternalTaskQueue = 0;
  
  delete this->InternalModifiedQueue;
  this->InternalModifiedQueue = 0;
  
  delete this->InternalReadDataQueue;
  this->InternalReadDataQueue = 0;

  // TODO - unregister/delete ivars
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ClearCollections ( ) {
    if ( this->Views) {
        this->Views->RemoveAllItems ( );
    }
    if ( this->Slices ) {
        this->Slices->RemoveAllItems ( );
    }
    if ( this->Modules ) {
        this->Modules->RemoveAllItems ( );
    }
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/ )
{


  //
  // Look for a selection node in the scene
  // - we always use the first one in the scene
  // - if it doesn't match the one we had, we switch
  // - if there isn't one, we create one
  // - we add it to the scene if needed
  //
  vtkMRMLSelectionNode *node;
  node = vtkMRMLSelectionNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

  if ( node == NULL )
    {
    node = vtkMRMLSelectionNode::New();
    this->SetSelectionNode (node);
    node->Delete();
    }

  if ( this->SelectionNode != node )
    {
      this->SetSelectionNode (node);
    }

  if (this->MRMLScene->GetNodeByID(this->SelectionNode->GetID()) == NULL)
    {
    this->SetMRMLScene(this->GetMRMLScene());
    this->MRMLScene->AddNode(this->SelectionNode);
    this->SetAndObserveMRMLScene(this->GetMRMLScene());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PropagateVolumeSelection()
{
  if ( !this->SelectionNode || !this->MRMLScene )
    {
    return;
    }

  int i, nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  char *ID = this->SelectionNode->GetActiveVolumeID();
  char *labelID = this->SelectionNode->GetActiveLabelVolumeID();

  vtkMRMLSliceCompositeNode *cnode;
  for (i = 0; i < nnodes; i++)
    {
    cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
            this->MRMLScene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    cnode->SetBackgroundVolumeID( ID );
    cnode->SetLabelVolumeID( labelID );
    }
}

//----------------------------------------------------------------------------
// Create a new Slice with it's associated class instances
vtkSlicerSliceLogic *vtkSlicerApplicationLogic::CreateSlice ()
{
    // Create the logic instances
    vtkSlicerSliceLogic *sliceLogic = vtkSlicerSliceLogic::New();
    vtkSlicerSliceLayerLogic *bg = vtkSlicerSliceLayerLogic::New();
    vtkSlicerSliceLayerLogic *fg = vtkSlicerSliceLayerLogic::New();

    // Create the mrml nodes to store state
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::New();
    this->MRMLScene->AddNode(sliceNode);

    // Configure the logic
    sliceLogic->SetBackgroundLayer(bg);
    sliceLogic->SetForegroundLayer(fg);
    sliceLogic->SetSliceNode(sliceNode);

    // Update internal state
    this->Slices->AddItem(sliceLogic);
    this->SetActiveSlice(sliceLogic);

    // Since they were New(), they should be Deleted(). If it crashes
    // then something is not ref-counted properly and should be fixed
    // (otherwise you are just leaking)

    sliceLogic->Delete();
    bg->Delete();
    fg->Delete();
    sliceNode->Delete();

    return (sliceLogic);
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerApplicationLogic:             " << this->GetClassName() << "\n"; 
} 

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::CreateProcessingThread()
{
  if (this->ProcessingThreadId == -1)
    {
    this->ProcessingThreadActiveLock->Lock();
    this->ProcessingThreadActive = true;
    this->ProcessingThreadActiveLock->Unlock();
    
    this->ProcessingThreadId
      = this->ProcessingThreader
      ->SpawnThread(vtkSlicerApplicationLogic::ProcessingThreaderCallback,
                    this);

    // Setup the communication channel back to the main thread
    this->ModifiedQueueActiveLock->Lock();
    this->ModifiedQueueActive = true;
    this->ModifiedQueueActiveLock->Unlock();
    this->ReadDataQueueActiveLock->Lock();
    this->ReadDataQueueActive = true;
    this->ReadDataQueueActiveLock->Unlock();

    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessModified");
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessReadData");
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::TerminateProcessingThread()
{
  if (this->ProcessingThreadId != -1)
    {
    this->ModifiedQueueActiveLock->Lock();
    this->ModifiedQueueActive = false;
    this->ModifiedQueueActiveLock->Unlock();

    this->ReadDataQueueActiveLock->Lock();
    this->ReadDataQueueActive = false;
    this->ReadDataQueueActiveLock->Unlock();
    
    this->ProcessingThreadActiveLock->Lock();
    this->ProcessingThreadActive = false;
    this->ProcessingThreadActiveLock->Unlock();

    this->ProcessingThreader->TerminateThread( this->ProcessingThreadId );

    this->ProcessingThreadId = -1;
    }
}


ITK_THREAD_RETURN_TYPE
vtkSlicerApplicationLogic
::ProcessingThreaderCallback( void *arg )
{
  
#ifdef ITK_USE_WIN32_THREADS
  // Adjust the priority of this thread
  SetThreadPriority(GetCurrentThread(),
                    THREAD_PRIORITY_BELOW_NORMAL);
#endif

#ifdef ITK_USE_PTHREADS
  // Adjust the priority of all PROCESS level threads.  Not a perfect solution.
  nice(20);
#endif
    
  // pull out the reference to the appLogic
  vtkSlicerApplicationLogic *appLogic
    = (vtkSlicerApplicationLogic*)
    (((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);

  // Tell the app to start processing any tasks slated for the
  // processing thread
  appLogic->ProcessTasks();

  return ITK_THREAD_RETURN_VALUE;
}

void vtkSlicerApplicationLogic::ProcessTasks()
{
  bool active = true;
  vtkSmartPointer<vtkSlicerTask> task = 0;
  
  while (active)
    {
    // Check to see if we should be shutting down
    this->ProcessingThreadActiveLock->Lock();
    active = this->ProcessingThreadActive;
    this->ProcessingThreadActiveLock->Unlock();

    if (active)
      {
      // pull a task off the queue
      this->ProcessingTaskQueueLock->Lock();
      if ((*this->InternalTaskQueue).size() > 0)
        {
        // std::cout << "Number of queued tasks: " << (*this->InternalTaskQueue).size() << std::endl;
        task = (*this->InternalTaskQueue).front();
        (*this->InternalTaskQueue).pop();
        }
      this->ProcessingTaskQueueLock->Unlock();
      
      // process the task (should this be in a separate thread?)
      if (task)
        {
        task->Execute();
        task = 0;
        }
      }

    // busy wait
    itksys::SystemTools::Delay(100);
    }
}

bool vtkSlicerApplicationLogic::ScheduleTask( vtkSlicerTask *task )
{
  bool active;

  // std::cout << "Scheduling a task ";

  // only schedule a task if the processing task is up
  this->ProcessingThreadActiveLock->Lock();
  active = this->ProcessingThreadActive;
  this->ProcessingThreadActiveLock->Unlock();

  if (active)
    {
    this->ProcessingTaskQueueLock->Lock();
    (*this->InternalTaskQueue).push( task );
    //std::cout << (*this->InternalTaskQueue).size() << std::endl;
    this->ProcessingTaskQueueLock->Unlock();
    
    return true;
    }

  // could not schedule the task
  return false;
}


bool vtkSlicerApplicationLogic::RequestModified( vtkObject *obj )
{
  bool active;

  //std::cout << "Requesting a modified on " << obj;

  // only request a Modified if the Modified queue is up
  this->ModifiedQueueActiveLock->Lock();
  active = this->ModifiedQueueActive;
  this->ModifiedQueueActiveLock->Unlock();

  if (active)
    {
    this->ModifiedQueueLock->Lock();
    (*this->InternalModifiedQueue).push( obj );
//     std::cout << " [" << (*this->InternalModifiedQueue).size()
//               << "] " << std::endl;
    this->ModifiedQueueLock->Unlock();
    
    return true;
    }

  // could not request the Modified
  return false;
}

bool vtkSlicerApplicationLogic::RequestReadData( const char *refNode, const char *filename, bool displayData, bool deleteFile )
{
  bool active;

  //std::cout << "Requesting " << filename << " be read into node " <<
  // refNode << std::endl;

  // only request to read a file if the ReadData queue is up
  this->ReadDataQueueActiveLock->Lock();
  active = this->ReadDataQueueActive;
  this->ReadDataQueueActiveLock->Unlock();

  if (active)
    {
    this->ReadDataQueueLock->Lock();
    (*this->InternalReadDataQueue).push( ReadDataRequest(refNode, filename,
                                                         displayData,
                                                         deleteFile) );
//     std::cout << " [" << (*this->InternalReadDataQueue).size()
//               << "] " << std::endl;
    this->ReadDataQueueLock->Unlock();
    
    return true;
    }

  // could not request the record be added to the queue
  return false;
}


void vtkSlicerApplicationLogic::ProcessModified()
{
  bool active = true;
  vtkSmartPointer<vtkObject> obj = 0;
  
  // Check to see if we should be shutting down
  this->ModifiedQueueActiveLock->Lock();
  active = this->ModifiedQueueActive;
  this->ModifiedQueueActiveLock->Unlock();
  
  if (active)
    {
    // pull an object off the queue to modify
    this->ModifiedQueueLock->Lock();
    if ((*this->InternalModifiedQueue).size() > 0)
      {
      obj = (*this->InternalModifiedQueue).front();
      (*this->InternalModifiedQueue).pop();

      // pop off any extra copies of the same object to save some updates
      while (!(*this->InternalModifiedQueue).empty() 
                    && (obj == (*this->InternalModifiedQueue).front()))
        {
        (*this->InternalModifiedQueue).pop();
        }
      }
    this->ModifiedQueueLock->Unlock();
    
    // Modify the object
    if (obj)
      {
      obj->Modified();
      obj = 0;
      }
    }
  
  // schedule the next timer
  vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                       100, this, "ProcessModified");
}

void vtkSlicerApplicationLogic::ProcessReadData()
{
  bool active = true;
  ReadDataRequest req;

  // Check to see if we should be shutting down
  this->ReadDataQueueActiveLock->Lock();
  active = this->ReadDataQueueActive;
  this->ReadDataQueueActiveLock->Unlock();
  
  if (active)
    {
    // pull an object off the queue to modify
    this->ReadDataQueueLock->Lock();
    if ((*this->InternalReadDataQueue).size() > 0)
      {
      req = (*this->InternalReadDataQueue).front();
      (*this->InternalReadDataQueue).pop();

      }
    this->ReadDataQueueLock->Unlock();

    // What type of node is the data really?
    vtkMRMLNode *nd = 0;
    vtkMRMLNode *disp = 0;
    vtkMRMLStorageNode *in = 0;
    vtkMRMLScalarVolumeNode *svnd = 0;
    vtkMRMLVectorVolumeNode *vvnd = 0;
    vtkMRMLDiffusionTensorVolumeNode *dtvnd = 0;
    vtkMRMLDiffusionWeightedVolumeNode *dwvnd = 0;
    vtkMRMLModelNode *mnd = 0;

    nd = this->MRMLScene->GetNodeByID( req.GetNode().c_str() );

    svnd  = vtkMRMLScalarVolumeNode::SafeDownCast(nd);
    vvnd  = vtkMRMLVectorVolumeNode::SafeDownCast(nd);
    dtvnd = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(nd);
    dwvnd = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(nd);
    mnd   = vtkMRMLModelNode::SafeDownCast(nd);

    // Read the data into the referenced node
    if (itksys::SystemTools::FileExists( req.GetFilename().c_str() ))
      {
      if (svnd || vvnd)
        {
        // Load a scalar or vector volume node
        in = vtkMRMLVolumeArchetypeStorageNode::New();
        disp = vtkMRMLVolumeDisplayNode::New();
        vtkMRMLVolumeDisplayNode *displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(disp);
        displayNode->SetDefaultColorMap();
        }
      else if (dtvnd || dwvnd)
        {
        // Load a diffusion tensor or a diffusion weighted node
//        in = vtkMRMLNRRDStorageNode::New();
        if (dtvnd)
          {
          disp = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
          }
        else
          {
          disp = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
          }
        }
      else if (mnd)
        {
        // Load a model node
        in = vtkMRMLModelStorageNode::New();
        disp = vtkMRMLModelDisplayNode::New();
        }

      // Have the storage node read the data into the current node
      if (in)
        {
        try
          {
          in->SetFileName( req.GetFilename().c_str() );
          in->ReadData( nd );
          }
        catch (itk::ExceptionObject& exc)
          {
          std::stringstream information;
          information << "Exception while reading " << req.GetFilename()
                      << ", " << exc;
          vtkErrorMacro( << information.str().c_str() );
          }
        catch (...)
          {
          std::stringstream information;
          information << "Unknown exception while reading "
                      << req.GetFilename();
          vtkErrorMacro( << information.str().c_str() );
          }
        in->Delete();
        }

      // Delete the file if requested
      if (req.GetDeleteFile())
        {
        bool removed;
        removed = itksys::SystemTools::RemoveFile( req.GetFilename().c_str() );
        if (!removed)
          {
          std::stringstream information;
          information << "Unable to delete temporary file "
                      << req.GetFilename() << std::endl;
          vtkWarningMacro( << information.str().c_str() );
          }
        }
      }

    // Display the data if requested
    //
    if (req.GetDisplayData())
      {
      // Set up the display node.  What if the node already had a
      // display node?
      if (disp)
        {
        disp->SetScene( this->MRMLScene );
        this->MRMLScene->AddNode( disp );

        if (svnd) svnd->SetAndObserveDisplayNodeID( disp->GetID() );
        else if (vvnd) vvnd->SetAndObserveDisplayNodeID( disp->GetID() );
        else if (dtvnd) dtvnd->SetAndObserveDisplayNodeID( disp->GetID() );
        else if (dwvnd) dwvnd->SetAndObserveDisplayNodeID( disp->GetID() );
        else if (mnd) mnd->SetAndObserveDisplayNodeID( disp->GetID() );
        
        disp->Delete();
        }

      // If scalar volume, set the volume as the active volume and
      // propagate selection.
      //
      // Models are always displayed when loaded above.
      // 
      // Tensors? Vectors?
      if (svnd)
        {
        if (svnd->GetLabelMap())
          {
          this->GetSelectionNode()
            ->SetActiveLabelVolumeID( req.GetNode().c_str() );
          }
        else
          {
          this->GetSelectionNode()
            ->SetActiveVolumeID( req.GetNode().c_str() );
          }
        this->PropagateVolumeSelection();
        }
      }
    
    }
  
  // schedule the next timer
  vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                       100, this, "ProcessReadData");
}

