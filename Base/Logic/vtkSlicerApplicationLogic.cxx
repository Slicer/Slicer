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
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkSlicerColorLogic.h"
#include "vtkSlicerVolumesLogic.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWApplication.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLNonlinearTransformNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkSlicerTask.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"

#ifdef linux 
#include "unistd.h"
#endif

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
                  int displayData, int deleteFile)
    {
      m_TargetNodes.clear();
      m_SourceNodes.clear();
      m_IsScene = false;
      
      m_TargetNodes.push_back(node);
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  ReadDataRequest(const char *node, const char *filename, int displayData,
                  int deleteFile)
    {
      m_TargetNodes.clear();
      m_SourceNodes.clear();
      m_IsScene = false;
      
      m_TargetNodes.push_back(node);
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  ReadDataRequest(const std::vector<std::string>& targetNodes,
                  const std::vector<std::string>& sourceNodes,
                  const std::string& filename,
                  int displayData, int deleteFile)
    {
      m_IsScene = true;

      m_TargetNodes = targetNodes;
      m_SourceNodes = sourceNodes;
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  ReadDataRequest()
    : m_Filename(""), m_DisplayData( false ), m_DeleteFile( false ),
      m_IsScene( false )
    {
    }

  const std::string& GetNode() const
    {
      static const std::string empty;
      if (m_TargetNodes.size() > 0)
        {
        return m_TargetNodes[0];
        }
      
      return empty;
    }
  
  const std::vector<std::string>& GetSourceNodes() const {return m_SourceNodes;}
  const std::vector<std::string>& GetTargetNodes() const {return m_TargetNodes;}
  const std::string& GetFilename() const { return m_Filename; }
  int GetDisplayData() const { return m_DisplayData; }
  int GetDeleteFile() const { return m_DeleteFile; }
  int GetIsScene() const { return m_IsScene; };
  
protected:
  std::vector<std::string> m_TargetNodes;
  std::vector<std::string> m_SourceNodes;
  std::string m_Filename;
  int m_DisplayData;
  int m_DeleteFile;
  bool m_IsScene;
  
};
class ReadDataQueue : public std::queue<ReadDataRequest> {} ;

class WriteDataRequest
{
public:
  WriteDataRequest(const std::string& node, const std::string& filename,
                  int displayData, int deleteFile)
    {
      m_TargetNodes.clear();
      m_SourceNodes.clear();
      m_IsScene = false;
      
      m_TargetNodes.push_back(node);
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  WriteDataRequest(const char *node, const char *filename, int displayData,
                  int deleteFile)
    {
      m_TargetNodes.clear();
      m_SourceNodes.clear();
      m_IsScene = false;
      
      m_TargetNodes.push_back(node);
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  WriteDataRequest(const std::vector<std::string>& targetNodes,
                  const std::vector<std::string>& sourceNodes,
                  const std::string& filename,
                  int displayData, int deleteFile)
    {
      m_IsScene = true;

      m_TargetNodes = targetNodes;
      m_SourceNodes = sourceNodes;
      m_Filename = filename;
      m_DisplayData = displayData;
      m_DeleteFile = deleteFile;
    }

  WriteDataRequest()
    : m_Filename(""), m_DisplayData( false ), m_DeleteFile( false ),
      m_IsScene( false )
    {
    }

  const std::string& GetNode() const
    {
      static const std::string empty;
      if (m_TargetNodes.size() > 0)
        {
        return m_TargetNodes[0];
        }
      
      return empty;
    }
  
  const std::vector<std::string>& GetSourceNodes() const {return m_SourceNodes;}
  const std::vector<std::string>& GetTargetNodes() const {return m_TargetNodes;}
  const std::string& GetFilename() const { return m_Filename; }
  int GetDisplayData() const { return m_DisplayData; }
  int GetDeleteFile() const { return m_DeleteFile; }
  int GetIsScene() const { return m_IsScene; };
  
protected:
  std::vector<std::string> m_TargetNodes;
  std::vector<std::string> m_SourceNodes;
  std::string m_Filename;
  int m_DisplayData;
  int m_DeleteFile;
  bool m_IsScene;
  
};
class WriteDataQueue : public std::queue<WriteDataRequest> {} ;



//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::vtkSlicerApplicationLogic()
{
    this->Views = vtkCollection::New();
    this->Slices = vtkCollection::New();
    this->Modules = vtkCollection::New();
    this->ActiveSlice = NULL;
    this->SelectionNode = NULL;
    this->InteractionNode = NULL;

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

    this->WriteDataQueueActive = false;
    this->WriteDataQueueActiveLock = itk::MutexLock::New();
    this->WriteDataQueueLock = itk::MutexLock::New();
    
    this->InternalTaskQueue = new ProcessingTaskQueue;
    this->InternalModifiedQueue = new ModifiedQueue;

    this->InternalReadDataQueue = new ReadDataQueue;
    this->InternalWriteDataQueue = new WriteDataQueue;
    
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
  this->SetInteractionNode ( NULL );
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

  delete this->InternalWriteDataQueue;
  this->InternalWriteDataQueue = 0;

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

  // selection node
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
    this->SetSelectionNode ( vtkMRMLSelectionNode::SafeDownCast(this->MRMLScene->AddNode(this->SelectionNode)) );
    this->SetAndObserveMRMLScene(this->GetMRMLScene());
    }


  vtkMRMLInteractionNode *inode;
  inode = vtkMRMLInteractionNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));                                                

  // interaction node
  if ( inode == NULL )
    {
    inode = vtkMRMLInteractionNode::New();
    this->SetInteractionNode ( inode );
    inode->Delete ( );
    }
  if ( this->InteractionNode != inode )
    {
    this->SetInteractionNode (inode );
    }
  if (this->MRMLScene->GetNodeByID(this->InteractionNode->GetID()) == NULL)
    {
    this->SetMRMLScene(this->GetMRMLScene());
    this->SetInteractionNode ( vtkMRMLInteractionNode::SafeDownCast(this->MRMLScene->AddNode(this->InteractionNode)) );
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

  int nitems = this->GetSlices()->GetNumberOfItems();
  for (i = 0; i < nitems; i++)
    {
    vtkSlicerSliceLogic *sliceLogic = vtkSlicerSliceLogic::SafeDownCast(this->GetSlices()->GetItemAsObject(i));
    vtkMRMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
    unsigned int *dims = sliceNode->GetDimensions();
    sliceLogic->FitSliceToAll(dims[0], dims[1]);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PropagateFiducialListSelection()
{
  if ( !this->SelectionNode || !this->MRMLScene )
    {
    return;
    }
  //char *ID = this->SelectionNode->GetActiveFiducialListID();

  // set the Fiducials GUI to show the active list? it's watching the node for
  // now
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

    // Start four network threads (TODO: make the number of threads a setting)
    this->NetworkingThreadIDs.push_back ( this->ProcessingThreader
          ->SpawnThread(vtkSlicerApplicationLogic::NetworkingThreaderCallback,
                    this) );
    /*
     * TODO: it looks like curl is not thread safe by default
     * - maybe there's a setting that cmcurl can have
     *   similar to the --enable-threading of the standard curl build
     *
    this->NetworkingThreadIDs.push_back ( this->ProcessingThreader
          ->SpawnThread(vtkSlicerApplicationLogic::NetworkingThreaderCallback,
                    this) );
    this->NetworkingThreadIDs.push_back ( this->ProcessingThreader
          ->SpawnThread(vtkSlicerApplicationLogic::NetworkingThreaderCallback,
                    this) );
    this->NetworkingThreadIDs.push_back ( this->ProcessingThreader
          ->SpawnThread(vtkSlicerApplicationLogic::NetworkingThreaderCallback,
                    this) );
    */

    // Setup the communication channel back to the main thread
    this->ModifiedQueueActiveLock->Lock();
    this->ModifiedQueueActive = true;
    this->ModifiedQueueActiveLock->Unlock();
    this->ReadDataQueueActiveLock->Lock();
    this->ReadDataQueueActive = true;
    this->ReadDataQueueActiveLock->Unlock();
    this->WriteDataQueueActiveLock->Lock();
    this->WriteDataQueueActive = true;
    this->WriteDataQueueActiveLock->Unlock();

    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessModified");
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessReadData");
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessWriteData");
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

    this->WriteDataQueueActiveLock->Lock();
    this->WriteDataQueueActive = false;
    this->WriteDataQueueActiveLock->Unlock();
    
    this->ProcessingThreadActiveLock->Lock();
    this->ProcessingThreadActive = false;
    this->ProcessingThreadActiveLock->Unlock();

    this->ProcessingThreader->TerminateThread( this->ProcessingThreadId );
    this->ProcessingThreadId = -1;

    std::vector<int>::const_iterator idIterator;
    idIterator = this->NetworkingThreadIDs.begin();
    while (idIterator != this->NetworkingThreadIDs.end())
      {
      this->ProcessingThreader->TerminateThread( *idIterator );
      ++idIterator;
      }
    this->NetworkingThreadIDs.clear();

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
  appLogic->ProcessProcessingTasks();

  return ITK_THREAD_RETURN_VALUE;
}

void vtkSlicerApplicationLogic::ProcessProcessingTasks()
{
  int active = true;
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
        
        // only handle processing tasks in this thread
        task = (*this->InternalTaskQueue).front();
        if ( task->GetType() == vtkSlicerTask::Processing )
          {
          (*this->InternalTaskQueue).pop();
          }
        else 
          {
          task = NULL;
          }
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

ITK_THREAD_RETURN_TYPE
vtkSlicerApplicationLogic
::NetworkingThreaderCallback( void *arg )
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
  appLogic->ProcessNetworkingTasks();

  return ITK_THREAD_RETURN_VALUE;
}

void vtkSlicerApplicationLogic::ProcessNetworkingTasks()
{
  int active = true;
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
        if ( task->GetType() == vtkSlicerTask::Networking )
          {
          (*this->InternalTaskQueue).pop();
          }
        else 
          {
          task = NULL;
          }
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

int vtkSlicerApplicationLogic::ScheduleTask( vtkSlicerTask *task )
{
  int active;

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


int vtkSlicerApplicationLogic::RequestModified( vtkObject *obj )
{
  int active;

  //std::cout << "Requesting a modified on " << obj;

  // only request a Modified if the Modified queue is up
  this->ModifiedQueueActiveLock->Lock();
  active = this->ModifiedQueueActive;
  this->ModifiedQueueActiveLock->Unlock();

  if (active)
    {
    obj->Register(this); // hold the object and release it when processed
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

int vtkSlicerApplicationLogic::RequestReadData( const char *refNode, const char *filename, int displayData, int deleteFile )
{
  int active;

  //std::cout << "RequestReadData: Requesting " << filename << " be read into node " << refNode << ", display data = " << (displayData?"true":"false") <<  std::endl;

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

int vtkSlicerApplicationLogic::RequestWriteData( const char *refNode, const char *filename, int displayData, int deleteFile )
{
  int active;

//  std::cout << "Requesting " << filename << " be read from node " << refNode << ", display data = " << (displayData?"true":"false") <<  std::endl;

  // only request to write a file if the WriteData queue is up
  this->WriteDataQueueActiveLock->Lock();
  active = this->WriteDataQueueActive;
  this->WriteDataQueueActiveLock->Unlock();

  if (active)
    {
    this->WriteDataQueueLock->Lock();
    (*this->InternalWriteDataQueue).push( WriteDataRequest(refNode, filename,
                                                         displayData,
                                                         deleteFile) );
//     std::cout << " [" << (*this->InternalWriteDataQueue).size()
//               << "] " << std::endl;
    this->WriteDataQueueLock->Unlock();
    
    return true;
    }

  // could not request the record be added to the queue
  return false;
}


int
vtkSlicerApplicationLogic
::RequestReadScene(const std::string& filename,
                   std::vector<std::string> &targetIDs,
                   std::vector<std::string> &sourceIDs,
                   int displayData, int deleteFile)
{
  int active;

//  std::cout << "Requesting scene " << filename << " be read " <<  std::endl;

  // only request to read a file if the ReadData queue is up
  this->ReadDataQueueActiveLock->Lock();
  active = this->ReadDataQueueActive;
  this->ReadDataQueueActiveLock->Unlock();

  if (active)
    {
    this->ReadDataQueueLock->Lock();
    (*this->InternalReadDataQueue).push( ReadDataRequest(targetIDs,
                                                         sourceIDs,
                                                         filename,
                                                         displayData,
                                                         deleteFile) );
    this->ReadDataQueueLock->Unlock();
    
    return true;
    }

  // could not request the record be added to the queue
  return false;
}


void vtkSlicerApplicationLogic::ProcessModified()
{
  int active = true;
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
        obj->Delete(); // decrement ref count
        }
      }
    this->ModifiedQueueLock->Unlock();
    
    // if this is a string array, try to evaluate the entries in the interp
    //  - this allows threads to indirectly access the interpreter
    vtkStringArray *stringArray = vtkStringArray::SafeDownCast( obj );
    if ( stringArray != NULL )
      {
      Tcl_Interp *interp = vtkKWApplication::GetMainInterp();
      int numValues = stringArray->GetNumberOfValues();
      for (int i = 0; i < numValues; i++)
        {
        const char *script = stringArray->GetValue( i ).c_str();
        int returnCode;
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION <= 2
        returnCode = Tcl_GlobalEval(interp, script);
#else
        returnCode = Tcl_EvalEx(interp, script, -1, TCL_EVAL_GLOBAL);
#endif  
        if ( returnCode != TCL_OK )
          {
          vtkErrorMacro ("Error evaluating message from script.\n" << 
            script << "\n" <<Tcl_GetStringResult (interp) );
          }
        }
      }

    // Modify the object
    //  - decrement reference count that was increased when it was added to the queue
    if (obj)
      {
      obj->Modified();
      obj->Delete();
      obj = 0;
      }
  }
  
  // schedule the next timer
  if ((*this->InternalModifiedQueue).size() > 0)
    {
    // schedule the next timer sooner in case there is stuff in the queue
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         5, this, "ProcessModified");
    }
  else
    {
    // schedule the next timer for a while later
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessModified");
    }
}

void vtkSlicerApplicationLogic::ProcessReadData()
{
  int active = true;
  ReadDataRequest req;
  
  // Check to see if we should be shutting down
  this->ReadDataQueueActiveLock->Lock();
  active = this->ReadDataQueueActive;
  this->ReadDataQueueActiveLock->Unlock();
  
  if (active)
    {
    // pull an object off the queue 
    this->ReadDataQueueLock->Lock();
    if ((*this->InternalReadDataQueue).size() > 0)
      {
      req = (*this->InternalReadDataQueue).front();
      (*this->InternalReadDataQueue).pop();

      }
    this->ReadDataQueueLock->Unlock();

    if (!req.GetNode().empty())
      {
      if (req.GetIsScene())
        {
        this->ProcessReadSceneData(req);
        }
      else
        {
        this->ProcessReadNodeData(req);
        }
      }
    }
  
  // schedule the next timer
  if ((*this->InternalReadDataQueue).size() > 0)
    {
    // schedule the next timer sooner in case there is stuff in the queue
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         5, this, "ProcessReadData");
    }
  else
    {
    // schedule the next timer for a while later
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessReadData");
    }
}

void vtkSlicerApplicationLogic::ProcessWriteData()
{
  int active = true;
  WriteDataRequest req;
  
  // Check to see if we should be shutting down
  this->WriteDataQueueActiveLock->Lock();
  active = this->WriteDataQueueActive;
  this->WriteDataQueueActiveLock->Unlock();
  
  if (active)
    {
    // pull an object off the queue 
    this->WriteDataQueueLock->Lock();
    if ((*this->InternalWriteDataQueue).size() > 0)
      {
      req = (*this->InternalWriteDataQueue).front();
      (*this->InternalWriteDataQueue).pop();

      }
    this->WriteDataQueueLock->Unlock();

    if (!req.GetNode().empty())
      {
      if (req.GetIsScene())
        {
        this->ProcessWriteSceneData(req);
        }
      else
        {
        this->ProcessWriteNodeData(req);
        }
      }
    }
  
  // schedule the next timer
  if ((*this->InternalWriteDataQueue).size() > 0)
    {
    // schedule the next timer sooner in case there is stuff in the queue
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         5, this, "ProcessWriteData");
    }
  else
    {
    // schedule the next timer for a while later
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
                                         100, this, "ProcessWriteData");
    }
}


void vtkSlicerApplicationLogic::ProcessReadNodeData(ReadDataRequest& req)
{
  // What type of node is the data really? Or is it a scene
  vtkMRMLNode *nd = 0;
  vtkMRMLDisplayNode *disp = 0;
  vtkMRMLStorageNode *storageNode = 0;
  vtkMRMLScalarVolumeNode *svnd = 0;
  vtkMRMLVectorVolumeNode *vvnd = 0;
  vtkMRMLDiffusionTensorVolumeNode *dtvnd = 0;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *dwdpn = NULL;
  vtkMRMLDiffusionWeightedVolumeNode *dwvnd = 0;
  vtkMRMLModelNode *mnd = 0;
  vtkMRMLLinearTransformNode *ltnd = 0;
  vtkMRMLNonlinearTransformNode *nltnd = 0;
  vtkMRMLFiberBundleNode *fbnd = 0;
  vtkMRMLColorTableNode *cnd = 0;
  
  nd = this->MRMLScene->GetNodeByID( req.GetNode().c_str() );

  vtkDebugMacro("ProcessReadNodeData: read data request node id = " << nd->GetID());
  
  svnd  = vtkMRMLScalarVolumeNode::SafeDownCast(nd);
  vvnd  = vtkMRMLVectorVolumeNode::SafeDownCast(nd);
  dtvnd = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(nd);
  dwvnd = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(nd);
  mnd   = vtkMRMLModelNode::SafeDownCast(nd);
  ltnd  = vtkMRMLLinearTransformNode::SafeDownCast(nd);
  nltnd  = vtkMRMLNonlinearTransformNode::SafeDownCast(nd);
  fbnd  = vtkMRMLFiberBundleNode::SafeDownCast(nd);
  cnd = vtkMRMLColorTableNode::SafeDownCast(nd);
  
  bool useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(req.GetFilename().c_str());
  bool storageNodeExists = false;
  int numStorageNodes = 0;

  vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(nd);
  if (storableNode)
    {
    numStorageNodes = storableNode->GetNumberOfStorageNodes();
    for (int n = 0; n < numStorageNodes; n++)
      {
      vtkMRMLStorageNode *testStorageNode = storableNode->GetNthStorageNode(n);
      if (testStorageNode)
        {
        if (useURI && testStorageNode->GetURI() != NULL)
          {
          if (req.GetFilename().compare(testStorageNode->GetURI()) == 0)
            {
            // found a storage node for the remote file
            vtkDebugMacro("ProcessReadNodeData: found a storage node with the right URI: " << testStorageNode->GetURI());
            storageNode = testStorageNode;
            storageNodeExists = true;
            break;
            }
          }
        else if (testStorageNode->GetFileName() != NULL &&
                 req.GetFilename().compare(testStorageNode->GetFileName()) == 0)
          {
          // found the right storage node for a local file
          vtkDebugMacro("ProcessReadNodeData: found a storage node with the right filename: " << testStorageNode->GetFileName());
          storageNode = testStorageNode;
          storageNodeExists = true;
          break;
          }
        }
      }
    }

  // if there wasn't already a matching storage node on the node, make one
  if (!storageNodeExists)
    {
    // Read the data into the referenced node
    if (itksys::SystemTools::FileExists( req.GetFilename().c_str() ))
      {
      if (svnd || vvnd)
        {
        // Load a scalar or vector volume node
        //
        // Need to maintain the original coordinate frame established by 
        // the images sent to the execution model 
        vtkMRMLVolumeArchetypeStorageNode *vin 
          = vtkMRMLVolumeArchetypeStorageNode::New();
        vin->SetCenterImage(0);
        storageNode = vin;
        }
      else if (dtvnd || dwvnd)
        {
        // Load a diffusion tensor or a diffusion weighted node
        //
        // Need to maintain the original coordinate frame established by 
        // the images sent to the execution model 
        vtkMRMLNRRDStorageNode *nin = vtkMRMLNRRDStorageNode::New();
        nin->SetCenterImage(0);
        storageNode = nin;
        }
      else if (fbnd)
        {
        // Load a fiber bundle node
        storageNode = vtkMRMLFiberBundleStorageNode::New();
        }
      else if (cnd)
        {
        // load in a color node
        storageNode = vtkMRMLColorTableStorageNode::New();
        }
      else if (mnd)
        {
        // Load a model node
        // first determine if the model node has a storage node that already points to this file
        /*
        int numStorageNodes = mnd->GetNumberOfStorageNodes();
        storageNode = NULL;
        for (int n = 0; n < numStorageNodes; n++)
          {
          vtkMRMLStorageNode *testStorageNode = mnd->GetNthStorageNode(n);
          if (testStorageNode)
            {
            if (useURI)
              {
              if (req.GetFilename().compare(testStorageNode->GetURI()) == 0)
                {
                // found a storage node for the remote file
                vtkDebugMacro("ProcessReadNodeData: found a storage node with the right URI: " << testStorageNode->GetURI());
                storageNode = testStorageNode;
                break;
                }
              }
              else if (req.GetFilename().compare(testStorageNode->GetFileName()) == 0)
              {
              // found the right storage node for a local file
               vtkDebugMacro("ProcessReadNodeData: found a storage node with the right filename: " << testStorageNode->GetFileName());
              storageNode = testStorageNode;
              break;
              }
            }
          }
         */
        // first determine if it's free surfer file name
        vtkWarningMacro("ProcessReadNodeData: making a new model storage node");
        vtkMRMLFreeSurferModelStorageNode *fssn = vtkMRMLFreeSurferModelStorageNode::New();
        vtkMRMLModelStorageNode *msn = vtkMRMLModelStorageNode::New();
        vtkMRMLFreeSurferModelOverlayStorageNode *fson = vtkMRMLFreeSurferModelOverlayStorageNode::New();
        if (fssn->SupportedFileType(req.GetFilename().c_str()))
          {
          storageNode = fssn;
          msn->Delete();
          fson->Delete();
          }
        else if (fson->SupportedFileType(req.GetFilename().c_str()))
          {
          storageNode = fson;
          msn->Delete();
          fssn->Delete();
          }
        else if (msn->SupportedFileType(req.GetFilename().c_str()))
          {
          storageNode = msn;
          fssn->Delete();
          fson->Delete();
          }
        }
    else if (ltnd || nltnd)
      {
      // Load a linear transform node

      // transforms can be communicated either using storage nodes or
      // in scenes.  we handle the former here.  the latter is handled
      // by ProcessReadSceneData()

      storageNode = vtkMRMLTransformStorageNode::New();
      }
      // file is there on disk
      }
     // done making a new storage node
    }
  
    // Have the storage node read the data into the current node
    if (storageNode)
      {
      try
        {
        vtkMRMLStorableNode *storableNode = 
          vtkMRMLStorableNode::SafeDownCast(nd);
        if ( storableNode && storableNode->GetStorageNode() == NULL  &&
             !storageNodeExists)
          {
          vtkDebugMacro("ProcessReadNodeData: found a storable node's storage node, it didn't exist already, adding the storage node " << storageNode->GetID());
          this->MRMLScene->AddNode( storageNode );
          storableNode->SetAndObserveStorageNodeID( storageNode->GetID() );
          }
        vtkDebugMacro("ProcessReadNodeData: about to call read data, storage node's read state is " << storageNode->GetReadStateAsString());
        if (useURI)
          {
          storageNode->SetURI(req.GetFilename().c_str());
          vtkDebugMacro("ProcessReadNodeData: calling ReadData on the storage node " << storageNode->GetID() << ", uri = " << storageNode->GetURI());
          storageNode->ReadData( nd );
          if (!storageNodeExists)
            {
            storageNode->SetURI(NULL);
            }
          }
        else
          {
          storageNode->SetFileName( req.GetFilename().c_str() );
           vtkDebugMacro("ProcessReadNodeData: calling ReadData on the storage node " << storageNode->GetID() << ", filename = " << storageNode->GetFileName());
          storageNode->ReadData( nd );
          if (!storageNodeExists)
            {
            storageNode->SetFileName( NULL ); // clear temp file name
            }
          }
        // since this was read from a temp location, 
        // mark it as needing to be saved when the scene is saved
        nd->SetModifiedSinceRead(1); 
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
      if (!storageNodeExists)
        {
        storageNode->Delete();
        }
      }
    
    // Delete the file if requested
    if (req.GetDeleteFile())
      {
      int removed;
      removed = itksys::SystemTools::RemoveFile( req.GetFilename().c_str() );
      if (!removed)
        {
        std::stringstream information;
        information << "Unable to delete temporary file "
                    << req.GetFilename() << std::endl;
        vtkWarningMacro( << information.str().c_str() );
        }
      }


  // Get the right type of display node. Only create a display node
  // if one does not exist already
  //
  if ((svnd && !svnd->GetDisplayNode())
      || (vvnd && !vvnd->GetDisplayNode()))
    {
    // Scalar or vector volume node
    if (svnd->GetLabelMap()) 
      {
      disp = vtkMRMLLabelMapVolumeDisplayNode::New();
      }
    else
      {
      disp = vtkMRMLScalarVolumeDisplayNode::New();
      }
    }
  else if ((dtvnd && !dtvnd->GetDisplayNode())
           || (dwvnd && !dwvnd->GetDisplayNode()))
    {
    // Diffusion tensor or a diffusion weighted node
    if (dtvnd)
      {
      vtkMRMLDiffusionTensorVolumeDisplayNode *dtvdn = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
      disp = dtvdn; // assign to superclass pointer
      dwdpn = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      this->MRMLScene->AddNode( dwdpn );
      dtvdn->SetAutoWindowLevel(0);
      dtvdn->SetWindow(0);
      dtvdn->SetLevel(0);
      dtvdn->SetUpperThreshold(0);
      dtvdn->SetLowerThreshold(0);
      dtvdn->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(dwdpn->GetID());
      dwdpn->Delete();
      }
    else
      {
      disp = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
      }
    }
  else if (fbnd)
    {
    // Fiber bundle node
    disp = NULL;
    vtkMRMLFiberBundleDisplayNode *fbdn = fbnd->AddLineDisplayNode();
    fbdn->SetVisibility(1);
    fbdn = fbnd->AddTubeDisplayNode();
    fbdn->SetVisibility(0);
    fbdn = fbnd->AddGlyphDisplayNode();
    fbdn->SetVisibility(0);
    }
  else if (mnd && !mnd->GetDisplayNode())
    {
    // Model node
    disp = vtkMRMLModelDisplayNode::New();
    }
  else if (ltnd || nltnd)
    {
    // Linear transform node
    // (no display node)  
    }
  
  // Set up the display node.  If we already have a display node,
  // just use that one.
  //
  if (disp)
    {
    vtkMRMLNode *dnode = this->MRMLScene->AddNode( disp );
    disp = vtkMRMLDisplayNode::SafeDownCast(dnode);
    int isLabelMap = 0;
    vtkMRMLVolumeDisplayNode *displayNode = NULL;
    if (svnd)
      {
      isLabelMap = svnd->GetLabelMap();
      if (isLabelMap)
        {
        displayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(disp);
        }
      else
        {
        displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(disp);
        }
      }
    else
      {
      displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(disp);
      }
    if (displayNode)
      {
      displayNode->SetDefaultColorMap();
      vtkMRMLVolumeNode *vnd = vtkMRMLVolumeNode::SafeDownCast(nd);
      vtkMRMLScalarVolumeDisplayNode *svdnd = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode);
      } 
    if (svnd) 
      {
      svnd->SetAndObserveDisplayNodeID( disp->GetID() );
      }
    else if (vvnd) 
      {
      vvnd->SetAndObserveDisplayNodeID( disp->GetID() );
      }
    else if (dtvnd) 
      {
      dtvnd->SetAndObserveDisplayNodeID( disp->GetID() );
      // add slice display nodes
      dtvnd->AddSliceGlyphDisplayNodes();

      }
    else if (dwvnd) 
      {
      dwvnd->SetAndObserveDisplayNodeID( disp->GetID() );
      }
    else if (mnd) 
      {
      mnd->SetAndObserveDisplayNodeID( disp->GetID() );
      }
    disp->Delete();
    }
    
  // Cause the any observers to fire (we may have avoided calling
  // modified on the node)
  //
  nd->Modified();

  // If scalar volume, set the volume as the active volume and
  // propagate selection.
  //
  // Models are always displayed when loaded above.
  // 
  // Tensors? Vectors?
  if (req.GetDisplayData())
    {
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

void vtkSlicerApplicationLogic::ProcessWriteNodeData(WriteDataRequest& req)
{
    vtkWarningMacro("ProcessWriteNodeData: we just wrote out, not doing anything here...");
}

void vtkSlicerApplicationLogic::ProcessReadSceneData(ReadDataRequest& req)
{
  if (req.GetSourceNodes().size() != req.GetTargetNodes().size())
    {
    // Can't do ID remapping if the two node lists are different
    // sizes. Just import the scene. (This is where we would put to
    // the code to load into a node heirarchy (with a corresponding
    // change in the conditional above)).
    this->MRMLScene->SetURL( req.GetFilename().c_str() );
    this->MRMLScene->Import();

    // Delete the file if requested
    if (req.GetDeleteFile())
      {
      int removed;
      removed = itksys::SystemTools::RemoveFile( req.GetFilename().c_str() );
      if (!removed)
        {
        std::stringstream information;
        information << "Unable to delete temporary file "
                    << req.GetFilename() << std::endl;
        vtkWarningMacro( << information.str().c_str() );
        }
      }

    return;
    }
  
  vtkSmartPointer<vtkMRMLScene> miniscene = vtkMRMLScene::New();
  miniscene->SetURL( req.GetFilename().c_str() );
  miniscene->Import();


  // iterate over the list of nodes specified to read
  std::vector<std::string>::const_iterator tit;
  std::vector<std::string>::const_iterator sit;

  tit = req.GetTargetNodes().begin();
  sit = req.GetSourceNodes().begin();

  while (sit != req.GetSourceNodes().end())
    {
    vtkMRMLNode *source;
    vtkMRMLNode *target;

    source = miniscene->GetNodeByID( (*sit).c_str() );
    target = this->MRMLScene->GetNodeByID( (*tit).c_str() );

    if (source && target)
      {
      target->Copy(source);

      // if the source node is a model hierachy node, then also copy
      // and remap any child nodes of the target that are not in the
      // target list (nodes that had no source equivalent before the
      // module ran).
      vtkMRMLModelHierarchyNode *smhnd
        = vtkMRMLModelHierarchyNode::SafeDownCast(source);
      vtkMRMLModelHierarchyNode *tmhnd
        = vtkMRMLModelHierarchyNode::SafeDownCast(target);
      if (smhnd && tmhnd)
        {
        // get the model node and display node BEFORE we add nodes to
        // the target scene
        vtkMRMLModelNode *smnd = smhnd->GetModelNode();
        vtkMRMLDisplayNode *sdnd = smhnd->GetDisplayNode();
        
        // add the model and display referenced by source model hierarchy node
        if (smnd)
          {
          // set the model node to be modified, as it was read from a temp
          // location
          smnd->SetModifiedSinceRead(1); 
          // get display node BEFORE we add nodes to the target scene
          vtkMRMLDisplayNode *sdnd = smnd->GetDisplayNode();
          
          vtkMRMLNode *tmodel = this->MRMLScene->CopyNode(smnd);
          vtkMRMLModelNode *mnd = vtkMRMLModelNode::SafeDownCast( tmodel );
          tmhnd->SetModelNodeID( mnd->GetID() );

          if (sdnd)
            {
            vtkMRMLNode *tdnd = this->MRMLScene->CopyNode(sdnd);
            mnd->SetAndObserveDisplayNodeID( tdnd->GetID() );
            }
          }
        
        if (sdnd)
          {
          vtkMRMLNode *dnd = this->MRMLScene->CopyNode(sdnd);
          tmhnd->SetAndObserveDisplayNodeID( dnd->GetID() );
          }
        
        // add any children model hierarchy nodes, rinse, repeat
        //
        // need a way to recurse - JVM
        for (int n=0;
             n<miniscene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
             n++)
          {
          vtkMRMLModelHierarchyNode * mhnd = vtkMRMLModelHierarchyNode
            ::SafeDownCast(miniscene->GetNthNodeByClass(n,
                                                "vtkMRMLModelHierarchyNode"));
          if (mhnd)
            {
            // is this model hierarchy node in our source list
            // already? if so skip it
            std::vector<std::string>::const_iterator ssit
              = std::find(req.GetSourceNodes().begin(),
                          req.GetSourceNodes().end(), mhnd->GetID());
            if (ssit == req.GetSourceNodes().end())
              {
              // not in source list, so we may need to add it
              if (mhnd->GetParentNode() == smhnd)
                {
                // get the model and display node BEFORE we add nodes
                // to the target scene
                vtkMRMLModelNode *smnd = mhnd->GetModelNode();
                vtkMRMLDisplayNode *sdnd = mhnd->GetDisplayNode();
                
                vtkMRMLNode *tchild = this->MRMLScene->CopyNode(mhnd);
                vtkMRMLModelHierarchyNode *tcmhd
                  = vtkMRMLModelHierarchyNode::SafeDownCast( tchild );
                tcmhd->SetParentNodeID( tmhnd->GetID() );
                
                if (smnd)
                  {
                  // set it as modified
                  smnd->SetModifiedSinceRead(1);
                  // get display node BEFORE we add nodes to the target scene
                  vtkMRMLDisplayNode *sdnd = smnd->GetDisplayNode();

                  vtkMRMLNode *tmodel = this->MRMLScene->CopyNode(smnd);
                  vtkMRMLModelNode *mnd =vtkMRMLModelNode::SafeDownCast(tmodel);
                  tcmhd->SetModelNodeID( mnd->GetID() );

                  if (sdnd)
                    {
                    vtkMRMLNode *tdnd = this->MRMLScene->CopyNode(sdnd);
                    mnd->SetAndObserveDisplayNodeID( tdnd->GetID() );
                    }
                  }
                
                if (sdnd)
                  {
                  vtkMRMLNode *tdnd = this->MRMLScene->CopyNode(sdnd);
                  tcmhd->SetAndObserveDisplayNodeID( tdnd->GetID() );
                  }
                }
              }
            }
          }
        }
      }
    else if (!source)
      {
      std::stringstream information;
      information << "Node " << (*sit) << " not found in scene file "
                  << req.GetFilename() << std::endl;
      vtkWarningMacro( << information.str().c_str() );
      }
    else if (!target)
      {
      std::stringstream information;
      information << "Node " << (*tit) << " not found in current scene."
                  << std::endl;
      vtkWarningMacro( << information.str().c_str() );
      }
    
    ++sit;
    ++tit;
    }

  // Delete the file if requested
  if (req.GetDeleteFile())
    {
    int removed;
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

void vtkSlicerApplicationLogic::ProcessWriteSceneData(WriteDataRequest& req)
{
  if (req.GetSourceNodes().size() != req.GetTargetNodes().size())
    {
    // Can't do ID remapping if the two node lists are different
    // sizes. Just commit the scene. (This is where we would put to
    // the code to load into a node heirarchy (with a corresponding
    // change in the conditional above)).
    this->MRMLScene->SetURL( req.GetFilename().c_str() );
    this->MRMLScene->Commit();

    // Delete the file if requested
    if (req.GetDeleteFile())
      {
      int removed;
      removed = itksys::SystemTools::RemoveFile( req.GetFilename().c_str() );
      if (!removed)
        {
        std::stringstream information;
        information << "Unable to delete temporary file "
                    << req.GetFilename() << std::endl;
        vtkWarningMacro( << information.str().c_str() );
        }
      }
    
    return;
    }
  
  // Delete the file if requested
  if (req.GetDeleteFile())
    {
    int removed;
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
