/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// Slicer includes
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLColorLogic.h"
#include "vtkSlicerConfigure.h" // For Slicer_BUILD_CLI_SUPPORT
#include "vtkSlicerTask.h"

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLColorTableStorageNode.h>
#ifdef Slicer_BUILD_CLI_SUPPORT
# include <vtkMRMLCommandLineModuleNode.h>
#endif
#include <vtkMRMLDiffusionTensorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkMRMLDiffusionWeightedVolumeDisplayNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLDoubleArrayStorageNode.h>
#include <vtkMRMLFreeSurferModelOverlayStorageNode.h>
#include <vtkMRMLFreeSurferModelStorageNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLNRRDStorageNode.h>
#include <vtkMRMLNonlinearTransformNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformStorageNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// ITKSYS includes
#include <itksys/SystemTools.hxx>

// STD includes
#include <algorithm>

#ifdef ITK_USE_PTHREADS
# include <unistd.h>
# include <sys/time.h>
# include <sys/resource.h>
#endif

#include <queue>

//----------------------------------------------------------------------------
class ProcessingTaskQueue : public std::queue<vtkSmartPointer<vtkSlicerTask> > {};
class ModifiedQueue : public std::queue<vtkSmartPointer<vtkObject> > {};

//----------------------------------------------------------------------------
class DataRequest
{
public:
  DataRequest(const std::string& node, const std::string& filename,
              int displayData, int deleteFile, int uid = 0)
  {
    m_TargetNodes.push_back(node);
    m_Filename = filename;
    m_DisplayData = displayData;
    m_DeleteFile = deleteFile;
    m_IsScene = false;
    m_UID = uid;
  }

  DataRequest(const char *node, const char *filename, int displayData,
              int deleteFile, int uid = 0)
  {
    m_TargetNodes.push_back(node);
    m_Filename = filename;
    m_DisplayData = displayData;
    m_DeleteFile = deleteFile;
    m_IsScene = false;
    m_UID = uid;
  }

  DataRequest(const std::vector<std::string>& targetNodes,
              const std::vector<std::string>& sourceNodes,
              const std::string& filename,
              int displayData, int deleteFile,
              int uid = 0)
  {
    m_TargetNodes = targetNodes;
    m_SourceNodes = sourceNodes;
    m_Filename = filename;
    m_DisplayData = displayData;
    m_DeleteFile = deleteFile;
    m_IsScene = true;
    m_UID = uid;
  }

  DataRequest()
    : m_Filename(""), m_DisplayData( false ), m_DeleteFile( false ),
      m_IsScene( false ), m_UID(0)
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
  int GetIsScene() const { return m_IsScene; }
  int GetUID()const{return m_UID;}

protected:
  std::vector<std::string> m_TargetNodes;
  std::vector<std::string> m_SourceNodes;
  std::string m_Filename;
  int m_DisplayData;
  int m_DeleteFile;
  bool m_IsScene;
  int m_UID;
};

//----------------------------------------------------------------------------
class ReadDataRequest: public DataRequest
{
public:
  ReadDataRequest(const std::string& node, const std::string& filename,
                  int displayData, int deleteFile, int uid = 0)
    : DataRequest( node, filename, displayData, deleteFile, uid)
  {
  }

  ReadDataRequest(const char *node, const char *filename, int displayData,
                  int deleteFile, int uid = 0)
    : DataRequest(node, filename, displayData, deleteFile, uid)
  {
  }

  ReadDataRequest(const std::vector<std::string>& targetNodes,
                  const std::vector<std::string>& sourceNodes,
                  const std::string& filename,
                  int displayData, int deleteFile,
                  int uid = 0)
    : DataRequest(targetNodes, sourceNodes, filename,
                  displayData, deleteFile, uid)
  {
  }

  ReadDataRequest()
  {
  }

};
class ReadDataQueue : public std::queue<ReadDataRequest> {} ;

//----------------------------------------------------------------------------
class WriteDataRequest: public DataRequest
{
public:
  WriteDataRequest(const std::string& node, const std::string& filename,
                   int displayData, int deleteFile, int uid = 0)
    : DataRequest( node, filename, displayData, deleteFile, uid)
  {
  }

  WriteDataRequest(const char *node, const char *filename, int displayData,
                   int deleteFile, int uid = 0)
    : DataRequest(node, filename, displayData, deleteFile, uid)
  {
  }

  WriteDataRequest(const std::vector<std::string>& targetNodes,
                   const std::vector<std::string>& sourceNodes,
                   const std::string& filename,
                   int displayData, int deleteFile,
                   int uid = 0)
    : DataRequest(targetNodes, sourceNodes, filename,
                  displayData, deleteFile, uid)
  {
  }

  WriteDataRequest()
  {
  }
};
class WriteDataQueue : public std::queue<WriteDataRequest> {} ;

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerApplicationLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerApplicationLogic);

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::vtkSlicerApplicationLogic()
{
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

  this->ModifiedQueueLock->Lock();
  while (!(*this->InternalModifiedQueue).empty())
    {
    vtkObject *obj = (*this->InternalModifiedQueue).front();
    (*this->InternalModifiedQueue).pop();
    obj->Delete(); // decrement ref count
    }
  this->ModifiedQueueLock->Unlock();
  delete this->InternalModifiedQueue;
  delete this->InternalReadDataQueue;
  delete this->InternalWriteDataQueue;
}

//----------------------------------------------------------------------------
unsigned int vtkSlicerApplicationLogic::GetReadDataQueueSize()
{
  return static_cast<unsigned int>( (*this->InternalReadDataQueue).size() );
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PropagateFiducialListSelection()
{
  if(!this->GetMRMLScene() || !this->GetSelectionNode())
    {
    return;
    }
  //char *ID = this->GetSelectionNode()->GetActiveFiducialListID();

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

    int delay = 1000;
    this->InvokeEvent(vtkSlicerApplicationLogic::RequestModifiedEvent, &delay);
    this->InvokeEvent(vtkSlicerApplicationLogic::RequestReadDataEvent, &delay);
    this->InvokeEvent(vtkSlicerApplicationLogic::RequestWriteDataEvent, &delay);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::TerminateProcessingThread()
{
  if (this->ProcessingThreadId != -1)
    {
    std::cout << "vtkSlicerApplicationLogic::TerminateProcessingThread()" << std::endl;
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

//----------------------------------------------------------------------------
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
  int which = PRIO_PROCESS;
  id_t pid;
  int priority = 20;
  int ret;

  pid = getpid();
  ret = setpriority(which, pid, priority);
  ret = ret; // dummy code to use the return value and avoid a compiler warning
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

//----------------------------------------------------------------------------
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
  int which = PRIO_PROCESS;
  id_t pid;
  int priority = 20;
  int ret;

  pid = getpid();
  ret = setpriority(which, pid, priority);
  ret = ret; // dummy code to use the return value and avoid a compiler warning
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

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
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
    obj->Register(this);
    this->ModifiedQueueLock->Lock();
    this->RequestTimeStamp.Modified();
    int uid = static_cast<int>(this->RequestTimeStamp.GetMTime());
    (*this->InternalModifiedQueue).push( obj );
//     std::cout << " [" << (*this->InternalModifiedQueue).size()
//               << "] " << std::endl;
    this->ModifiedQueueLock->Unlock();
    return uid;
    }

  // could not request the Modified
  return 0;
}

//----------------------------------------------------------------------------
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
    this->RequestTimeStamp.Modified();
    int uid = static_cast<int>(this->RequestTimeStamp.GetMTime());
    (*this->InternalReadDataQueue).push(
      ReadDataRequest(refNode, filename, displayData, deleteFile, uid) );
//     std::cout << " [" << (*this->InternalReadDataQueue).size()
//               << "] " << std::endl;
    this->ReadDataQueueLock->Unlock();
    return uid;
    }

  // could not request the record be added to the queue
  return 0;
}

//----------------------------------------------------------------------------
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
    this->RequestTimeStamp.Modified();
    int uid = static_cast<int>(this->RequestTimeStamp.GetMTime());
    (*this->InternalWriteDataQueue).push(
      WriteDataRequest(refNode, filename, displayData, deleteFile, uid) );
//     std::cout << " [" << (*this->InternalWriteDataQueue).size()
//               << "] " << std::endl;
    this->WriteDataQueueLock->Unlock();
    return uid;
    }

  // could not request the record be added to the queue
  return 0;
}


//----------------------------------------------------------------------------
int vtkSlicerApplicationLogic::RequestReadScene(
    const std::string& filename,
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
    this->RequestTimeStamp.Modified();
    int uid = static_cast<int>(this->RequestTimeStamp.GetMTime());
    (*this->InternalReadDataQueue).push(
      ReadDataRequest(targetIDs, sourceIDs, filename, displayData, deleteFile, uid) );
    this->ReadDataQueueLock->Unlock();

    return uid;
    }

  // could not request the record be added to the queue
  return 0;
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessModified()
{
  // Check to see if we should be shutting down
  this->ModifiedQueueActiveLock->Lock();
  int active = this->ModifiedQueueActive;
  this->ModifiedQueueActiveLock->Unlock();

  if (!active)
    {
    return;
    }

  vtkSmartPointer<vtkObject> obj = 0;
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

  // Modify the object
  //  - decrement reference count that was increased when it was added to the queue
  if (obj.GetPointer())
    {
    obj->Modified();
    obj->Delete();
    obj = 0;
    }

  // schedule the next timer sooner in case there is stuff in the queue
  // otherwise for a while later
  int delay = (*this->InternalModifiedQueue).size() > 0 ? 0: 200;
  this->InvokeEvent(vtkSlicerApplicationLogic::RequestModifiedEvent, &delay);
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessReadData()
{
  // Check to see if we should be shutting down
  this->ReadDataQueueActiveLock->Lock();
  int active = this->ReadDataQueueActive;
  this->ReadDataQueueActiveLock->Unlock();

  if (!active)
    {
    return;
    }
  ReadDataRequest req;
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

  int delay = (*this->InternalReadDataQueue).size() > 0 ? 0: 200;
  // schedule the next timer sooner in case there is stuff in the queue
  // otherwise for a while later
  this->InvokeEvent(vtkSlicerApplicationLogic::RequestReadDataEvent, &delay);
  if (req.GetUID())
    {
    this->InvokeEvent(vtkSlicerApplicationLogic::RequestProcessedEvent,
                      reinterpret_cast<void*>(req.GetUID()));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessWriteData()
{
  // Check to see if we should be shutting down
  this->WriteDataQueueActiveLock->Lock();
  int active = this->WriteDataQueueActive;
  this->WriteDataQueueActiveLock->Unlock();

  if (!active)
    {
    return;
    }

  WriteDataRequest req;
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
  // schedule the next timer sooner in case there is stuff in the queue
  // otherwise for a while later
  int delay = (*this->InternalWriteDataQueue).size() > 0 ? 0: 200;
  this->InvokeEvent(vtkSlicerApplicationLogic::RequestWriteDataEvent, &delay);
  if (req.GetUID())
    {
    this->InvokeEvent(vtkSlicerApplicationLogic::RequestProcessedEvent,
                      reinterpret_cast<void*>(req.GetUID()));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessReadNodeData(ReadDataRequest& req)
{
  // What type of node is the data really? Or is it a scene
  vtkMRMLNode *nd = 0;
  vtkMRMLDisplayNode *disp = 0;
  vtkMRMLStorageNode *storageNode = 0;
  vtkMRMLScalarVolumeNode *svnd = 0;
  vtkMRMLVectorVolumeNode *vvnd = 0;
  vtkMRMLDiffusionTensorVolumeNode *dtvnd = 0;
  vtkMRMLDiffusionWeightedVolumeNode *dwvnd = 0;
  vtkMRMLModelNode *mnd = 0;
  vtkMRMLLinearTransformNode *ltnd = 0;
  vtkMRMLNonlinearTransformNode *nltnd = 0;
  vtkMRMLDisplayableNode *fbnd = 0;
  vtkMRMLColorTableNode *cnd = 0;
  vtkMRMLDoubleArrayNode *dand = 0;
#ifdef Slicer_BUILD_CLI_SUPPORT
  vtkMRMLCommandLineModuleNode *clp = 0;
#endif

  nd = this->GetMRMLScene()->GetNodeByID( req.GetNode().c_str() );

  vtkDebugMacro("ProcessReadNodeData: read data request node id = " << nd->GetID());

  // volumes may inherit from each other,
  // should be in the reverse order of inheritance
  if (vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(nd))
    {
    dwvnd = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(nd);
    }
  else if (vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(nd))
    {
     dtvnd = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(nd);
    }
  else if (vtkMRMLVectorVolumeNode::SafeDownCast(nd))
    {
    vvnd  = vtkMRMLVectorVolumeNode::SafeDownCast(nd);
    }
  else if (vtkMRMLScalarVolumeNode::SafeDownCast(nd))
    {
    svnd  = vtkMRMLScalarVolumeNode::SafeDownCast(nd);
    }


  mnd   = vtkMRMLModelNode::SafeDownCast(nd);
  ltnd  = vtkMRMLLinearTransformNode::SafeDownCast(nd);
  nltnd  = vtkMRMLNonlinearTransformNode::SafeDownCast(nd);
  fbnd  = vtkMRMLDisplayableNode::SafeDownCast(nd);
  cnd = vtkMRMLColorTableNode::SafeDownCast(nd);
  dand = vtkMRMLDoubleArrayNode::SafeDownCast(nd);
#ifdef Slicer_BUILD_CLI_SUPPORT
  clp = vtkMRMLCommandLineModuleNode::SafeDownCast(nd);
#endif

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
      else if (fbnd && fbnd->IsA("vtkMRMLFiberBundleNode"))
        {
        vtkDebugMacro("ProcessReadNodeData: node is a vtkMRMLFiberBundleNode");
        // Load a fiber bundle node
        storageNode = fbnd->CreateDefaultStorageNode();
        }
      else if (cnd)
        {
        // load in a color node
        storageNode = vtkMRMLColorTableStorageNode::New();
        }
      else if (dand)
        {
        // load in a color node
        storageNode = vtkMRMLDoubleArrayStorageNode::New();
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
        vtkMRMLStorableNode *storableNode1 =
          vtkMRMLStorableNode::SafeDownCast(nd);
        if ( storableNode1 && storableNode1->GetStorageNode() == NULL  &&
             !storageNodeExists)
          {
          vtkDebugMacro("ProcessReadNodeData: found a storable node's storage node, it didn't exist already, adding the storage node " << storageNode->GetID());
          this->GetMRMLScene()->AddNode( storageNode );
          storableNode1->SetAndObserveStorageNodeID( storageNode->GetID() );
          }
        vtkDebugMacro("ProcessReadNodeData: about to call read data, storage node's read state is " << storageNode->GetReadStateAsString());
        if (useURI)
          {
          storageNode->SetURI(req.GetFilename().c_str());
          vtkDebugMacro("ProcessReadNodeData: calling ReadData on the storage node " << storageNode->GetID() << ", uri = " << storageNode->GetURI());
          storageNode->ReadData( nd, /*temporary*/true );
          if (!storageNodeExists)
            {
            storageNode->SetURI(NULL);
            }
          }
        else
          {
          storageNode->SetFileName( req.GetFilename().c_str() );
           vtkDebugMacro("ProcessReadNodeData: calling ReadData on the storage node " << storageNode->GetID() << ", filename = " << storageNode->GetFileName());
          storageNode->ReadData( nd, /*temporary*/true );
          if (!storageNodeExists)
            {
            storageNode->SetFileName( NULL ); // clear temp file name
            }
          }
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

#ifdef Slicer_BUILD_CLI_SUPPORT
    // if the node was a CommandLineModule node, then read the file
    // (no storage node for these, yet)
    if (clp)
      {
      clp->ReadParameterFile(req.GetFilename());
      }
#endif

    // Delete the file if requested
    if (req.GetDeleteFile())
      {
      int removed;
      // is it a shared memory location?
      if (req.GetFilename().find("slicer:") != std::string::npos)
        {
        removed = 1;
        }
      else
        {
        removed = itksys::SystemTools::RemoveFile( req.GetFilename().c_str() );
        }
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
    if (svnd)
      {
      if (svnd->GetLabelMap())
        {
        disp = vtkMRMLLabelMapVolumeDisplayNode::New();
        }
      else
        {
        disp = vtkMRMLScalarVolumeDisplayNode::New();
        }
      }
    else
      {
      disp = vtkMRMLVectorVolumeDisplayNode::New();
      }
    }
  else if ((dtvnd && !dtvnd->GetDisplayNode())
           || (dwvnd && !dwvnd->GetDisplayNode()))
    {
    // Diffusion tensor or a diffusion weighted node
    if (dtvnd)
      {
      vtkMRMLDiffusionTensorVolumeDisplayNode *dtvdn = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
//       dtvdn->SetWindow(0);
//       dtvdn->SetLevel(0);
//       dtvdn->SetUpperThreshold(0);
//       dtvdn->SetLowerThreshold(0);
//       dtvdn->SetAutoWindowLevel(1);
      disp = dtvdn; // assign to superclass pointer
      }
    else
      {
      disp = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
      }
    }
  else if (fbnd && fbnd->IsA("vtkMRMLFiberBundleNode") && !fbnd->GetDisplayNode())
    {
    // Fiber bundle node
    fbnd->CreateDefaultDisplayNodes();
    disp = NULL;
    }
  else if (mnd && !mnd->GetDisplayNode())
    {
    // Model node
    vtkMRMLModelDisplayNode *modelDisplayNode = vtkMRMLModelDisplayNode::New();
    disp = modelDisplayNode;
    if (mnd->GetPolyData())
      {
      modelDisplayNode->SetInputPolyData(mnd->GetPolyData());
      }
    if (mnd->GetPolyData() &&
        mnd->GetPolyData()->GetPointData() &&
        mnd->GetPolyData()->GetPointData()->GetScalars())
      {
      vtkDebugMacro("Made a new model display node, there are scalars defined on the model - setting them visible and using the first one as the selected overlay");
      disp->SetScalarVisibility(1);
      disp->SetActiveScalarName(mnd->GetPolyData()->GetPointData()->GetAttribute(0)->GetName());
      // use the fs red green colour node for now
      disp->SetAndObserveColorNodeID("vtkMRMLFreeSurferProceduralColorNodeRedGreen");
      }
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
    vtkMRMLNode *dnode = this->GetMRMLScene()->AddNode( disp );
    disp->SetAndObserveColorNodeID(this->GetColorLogic()->GetDefaultVolumeColorNodeID());
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
      vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(disp)->AddSliceGlyphDisplayNodes( dtvnd );
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
    if (vtkMRMLScalarVolumeNode::SafeDownCast(nd) != NULL)
      {
      svnd = vtkMRMLScalarVolumeNode::SafeDownCast(nd);
      }
    else
      {
      svnd = NULL;
      }
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
      if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(svnd->GetDisplayNode()) != NULL)
        {
        // make sure win/level gets calculated
        svnd->GetDisplayNode()->Modified();
        }
//      if (!strcmp(svnd->GetClassName(), "vtkMRMLScalarVolumeNode"))
        {
        this->PropagateVolumeSelection();
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessWriteNodeData(WriteDataRequest& vtkNotUsed(req))
{
    vtkWarningMacro("ProcessWriteNodeData: we just wrote out, not doing anything here...");
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessReadSceneData(ReadDataRequest& req)
{
  if (req.GetSourceNodes().size() != req.GetTargetNodes().size())
    {
    // Can't do ID remapping if the two node lists are different
    // sizes. Just import the scene. (This is where we would put to
    // the code to load into a node heirarchy (with a corresponding
    // change in the conditional above)).
    this->GetMRMLScene()->SetURL( req.GetFilename().c_str() );
    this->GetMRMLScene()->Import();

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

  vtkNew<vtkMRMLScene> miniscene;
  miniscene->SetURL( req.GetFilename().c_str() );
  miniscene->Import();

  // iterate over the list of nodes specified to read
  std::vector<std::string>::const_iterator tit;
  std::vector<std::string>::const_iterator sit;

  tit = req.GetTargetNodes().begin();
  sit = req.GetSourceNodes().begin();

  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  while (sit != req.GetSourceNodes().end())
    {
    vtkMRMLNode *source;
    vtkMRMLNode *target;

    source = miniscene->GetNodeByID( (*sit).c_str() );
    target = this->GetMRMLScene()->GetNodeByID( (*tit).c_str() );

    if (source && target)
      {
      // save old storage info (in case user has custom file name already
      // defined for this node, don't use the one from the miniscene since it
      // was only used to read/write the temp area).
      vtkMRMLStorableNode *storableTarget = vtkMRMLStorableNode::SafeDownCast(target);
      if ( storableTarget )
        {
        const char *oldStorageNodeID = storableTarget->GetStorageNodeID();
        target->Copy(source);
        storableTarget->SetAndObserveStorageNodeID(oldStorageNodeID);
        }
      else
        {
        target->Copy(source);
        }

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
          //smnd->SetModifiedSinceRead(1);
          // get display node BEFORE we add nodes to the target scene
          vtkMRMLDisplayNode *sdnd1 = smnd->GetDisplayNode();

          vtkMRMLNode *tmodel = this->GetMRMLScene()->CopyNode(smnd);
          vtkMRMLStorableNode::SafeDownCast(tmodel)->SetAndObserveStorageNodeID(NULL);
          vtkMRMLModelNode *mnd = vtkMRMLModelNode::SafeDownCast( tmodel );
          tmhnd->SetModelNodeID( mnd->GetID() );

          if (sdnd1)
            {
            vtkMRMLNode *tdnd = this->GetMRMLScene()->CopyNode(sdnd1);
            mnd->SetAndObserveDisplayNodeID( tdnd->GetID() );
            }
          }

        if (sdnd)
          {
          vtkMRMLNode *dnd = this->GetMRMLScene()->CopyNode(sdnd);
          tmhnd->SetAndObserveDisplayNodeID( dnd->GetID() );
          }

        // add any children model hierarchy nodes, rinse, repeat
        //
        // keep a map of model hierarchy node ids so that can update the parent node references
        std::map<std::string, std::string> parentNodeIDMapper;
        // hopefully the parents will have been read first, but if not 
        // keep a list of model hierarchy nodes that failed to have their parent node reference remapped
        std::vector<vtkMRMLModelHierarchyNode *> childNodesThatNeedParentsIDsRemapped; 
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
              // not in source list, so we may need to add it,
              // if it's a child, grandchild etc of the top level node that we're importing
              if (strcmp(mhnd->GetTopParentNode()->GetID(), smhnd->GetID()) == 0)
                {
                // get the model and display node BEFORE we add nodes
                // to the target scene
                vtkMRMLModelNode *smnd1 = mhnd->GetModelNode();
                vtkMRMLDisplayNode *sdnd1 = mhnd->GetDisplayNode();

                vtkMRMLNode *tchild = this->GetMRMLScene()->CopyNode(mhnd);
                // keep track of any node id change in case other nodes use this as a parent
                parentNodeIDMapper[std::string(mhnd->GetID())] = std::string(tchild->GetID());
                vtkMRMLModelHierarchyNode *tcmhd
                  = vtkMRMLModelHierarchyNode::SafeDownCast( tchild );
                // check for a parent node id in the mapper (as long as it doesn't already 
                // point to the source node), default to the top level one though
                std::string parentNodeID = std::string(tmhnd->GetID());
                if (tcmhd->GetParentNodeID() != NULL && 
                    strcmp(tcmhd->GetParentNodeID(),smhnd->GetID()) != 0)
                  {
                  std::map<std::string,std::string>::iterator pIt = parentNodeIDMapper.find(std::string(tcmhd->GetParentNodeID()));
                  if (pIt != parentNodeIDMapper.end())
                    {
                    parentNodeID = pIt->second;
                    vtkDebugMacro("Remapped parent node id to " << parentNodeID.c_str());
                    }
                  else
                    {
                    childNodesThatNeedParentsIDsRemapped.push_back(tcmhd);
                    }
                  }
                tcmhd->SetParentNodeID( parentNodeID.c_str() );

                if (smnd1)
                  {
                  // set it as modified
                  //smnd1->SetModifiedSinceRead(1);
                  // get display node BEFORE we add nodes to the target scene
                  vtkMRMLDisplayNode *sdnd2 = smnd1->GetDisplayNode();

                  vtkMRMLNode *tmodel = this->GetMRMLScene()->CopyNode(smnd1);
                  vtkMRMLStorableNode::SafeDownCast(tmodel)->SetAndObserveStorageNodeID(NULL);
                  vtkMRMLModelNode *mnd =vtkMRMLModelNode::SafeDownCast(tmodel);
                  tcmhd->SetModelNodeID( mnd->GetID() );

                  if (sdnd2)
                    {
                    vtkMRMLNode *tdnd = this->GetMRMLScene()->CopyNode(sdnd2);
                    mnd->SetAndObserveDisplayNodeID( tdnd->GetID() );
                    }
                  }

                if (sdnd1)
                  {
                  vtkMRMLNode *tdnd = this->GetMRMLScene()->CopyNode(sdnd1);
                  tcmhd->SetAndObserveDisplayNodeID( tdnd->GetID() );
                  }
                }
              }
            }
          }
        if (childNodesThatNeedParentsIDsRemapped.size() > 0)
          {
          // iterate through all the imported hierarchies that failed and double check their parent node ids
          for (unsigned int i = 0; i < childNodesThatNeedParentsIDsRemapped.size(); i++)
            {
            std::map<std::string,std::string>::iterator pIt = parentNodeIDMapper.find(childNodesThatNeedParentsIDsRemapped[i]->GetParentNodeID());
            if (pIt != parentNodeIDMapper.end())
              {
              vtkDebugMacro("Remapping child node " << childNodesThatNeedParentsIDsRemapped[i]->GetName() << " parent node id from " << childNodesThatNeedParentsIDsRemapped[i]->GetParentNodeID() << " to " << pIt->second.c_str());
              childNodesThatNeedParentsIDsRemapped[i]->SetParentNodeID(pIt->second.c_str());
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

  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);

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

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessWriteSceneData(WriteDataRequest& req)
{
  if (req.GetSourceNodes().size() != req.GetTargetNodes().size())
    {
    // Can't do ID remapping if the two node lists are different
    // sizes. Just commit the scene. (This is where we would put to
    // the code to load into a node heirarchy (with a corresponding
    // change in the conditional above)).
    this->GetMRMLScene()->SetURL( req.GetFilename().c_str() );
    this->GetMRMLScene()->Commit();

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

//----------------------------------------------------------------------------
bool vtkSlicerApplicationLogic::IsEmbeddedModule(const std::string& filePath,
                                                 const std::string& applicationHomeDir,
                                                 const std::string& slicerRevision)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return false;
    }
  if (applicationHomeDir.empty())
    {
    vtkGenericWarningMacro( << "applicationHomeDir is an empty string !");
    return false;
    }
  std::string extensionPath = itksys::SystemTools::GetFilenamePath(filePath);
  bool isEmbedded = itksys::SystemTools::StringStartsWith(extensionPath.c_str(), applicationHomeDir.c_str());
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  // On MacOSX extensions are installed in the "<Slicer_EXTENSIONS_DIRBASENAME>-<slicerRevision>"
  // folder being a sub directory of the application dir, an extra test is required to make sure the
  // tested filePath doesn't belong to that "<Slicer_EXTENSIONS_DIRBASENAME>-<slicerRevision>" folder.
  // BUG 2848: Since package name can be rename from "Slicer.app" to "Something.app", let's compare
  // using ".app/Contents/" instead of "Slicer_BUNDLE_LOCATION" which is "Slicer.app/Contents/"
  if (isEmbedded && extensionPath.find(".app/Contents/" Slicer_EXTENSIONS_DIRBASENAME "-" + slicerRevision) != std::string::npos)
    {
    isEmbedded = false;
    }
#endif
  return isEmbedded;
}

//----------------------------------------------------------------------------
bool vtkSlicerApplicationLogic::IsPluginInstalled(const std::string& filePath,
                                                  const std::string& applicationHomeDir)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return false;
    }
  if (applicationHomeDir.empty())
    {
    vtkGenericWarningMacro( << "applicationHomeDir is an empty string !");
    return false;
    }

  std::string path = itksys::SystemTools::GetFilenamePath(filePath);
  std::string canonicalPath = itksys::SystemTools::GetRealPath(path.c_str());

  if (itksys::SystemTools::StringStartsWith(canonicalPath.c_str(), applicationHomeDir.c_str()))
    {
    return !itksys::SystemTools::FileExists(
          std::string(applicationHomeDir).append("/CMakeCache.txt").c_str(), true);
    }

  std::string root;
  std::string canonicalPathWithoutRoot =
      itksys::SystemTools::SplitPathRootComponent(canonicalPath.c_str(), &root);
  do
    {
    if (itksys::SystemTools::FileExists(
          (root + canonicalPathWithoutRoot + "/CMakeCache.txt").c_str(), true))
      {
      return false;
      }
    canonicalPathWithoutRoot = itksys::SystemTools::GetParentDirectory(canonicalPathWithoutRoot.c_str());
    }
  while(!canonicalPathWithoutRoot.empty());

  return true;
}

namespace
{
//----------------------------------------------------------------------------
std::string GetModuleHomeDirectory(const std::string& filePath,
                                   std::string& slicerSubDir,
                                   std::string& moduleTypeSubDir)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return std::string();
    }

  // In the current implementation, we assume the path to a module and its resources
  // will respectively have the following structure:
  //   ../lib/Slicer-X.Y/<module-type>/module
  // and
  //   ../share/Slicer-X.Y/<module-type>/<module-name>/

  std::vector<std::string> components;
  itksys::SystemTools::SplitPath(filePath.c_str(), components, true);

  // components[components.size() - 1] -> fileName
  // components[components.size() - 2 - offset] -> {qt-scripted-modules, qt-loadable-modules, ...}
  // components[components.size() - 3 - offset] -> Slicer-X.Y
  // components[components.size() - 4 - offset] -> lib
  // components[0 .. components.size() - 4 - offset] -> Common path to lib and share directory

  if (components.size() < 5)
    {
    // At least 5 components are expected to be able to compute the module home directory
    vtkGenericWarningMacro( << "Failed to compute module home directory given filePath: " << filePath);
    return std::string();
    }

  // offset == 1 if there is an intermediate build directory
  int offset = 0;
  std::string intDir(".");
  std::string possibleIntDir = components.at(components.size() - 2);
  if (!itksys::SystemTools::Strucmp(possibleIntDir.c_str(), "Debug") ||
      !itksys::SystemTools::Strucmp(possibleIntDir.c_str(), "Release") ||
      !itksys::SystemTools::Strucmp(possibleIntDir.c_str(), "RelWithDebInfo") ||
      !itksys::SystemTools::Strucmp(possibleIntDir.c_str(), "MinSizeRel"))
    {
    offset = 1;
    intDir = possibleIntDir;
    }

  moduleTypeSubDir = components.at(components.size() - 2 - offset);
  slicerSubDir = components.at(components.size() - 3 - offset);

  std::string homeDirectory =
      itksys::SystemTools::JoinPath(components.begin(), components.end() - 4 - offset);
  return homeDirectory;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
std::string vtkSlicerApplicationLogic::GetModuleShareDirectory(const std::string& moduleName,
                                                               const std::string& filePath)
{
  if (moduleName.empty())
    {
    vtkGenericWarningMacro( << "moduleName is an empty string !");
    return std::string();
    }
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return std::string();
    }

  std::string slicerSubDir;
  std::string moduleTypeSubDir;
  std::string shareDirectory = GetModuleHomeDirectory(filePath, slicerSubDir, moduleTypeSubDir);

  if (shareDirectory.empty())
    {
    return std::string();
    }

  shareDirectory.append("/share");
  shareDirectory.append("/");
  shareDirectory.append(slicerSubDir);
  shareDirectory.append("/");
  shareDirectory.append(moduleTypeSubDir);
  shareDirectory.append("/");
  shareDirectory.append(moduleName);

  return shareDirectory;
}

//----------------------------------------------------------------------------
std::string vtkSlicerApplicationLogic::GetModuleSlicerXYShareDirectory(const std::string& filePath)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return std::string();
    }
  std::string slicerSubDir;
  std::string moduleTypeSubDir;
  std::string shareDirectory = GetModuleHomeDirectory(filePath, slicerSubDir, moduleTypeSubDir);

  if (shareDirectory.empty())
    {
    return std::string();
    }

  shareDirectory.append("/share");
  shareDirectory.append("/");
  shareDirectory.append(slicerSubDir);
  return shareDirectory;
}

//----------------------------------------------------------------------------
std::string vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory(const std::string& filePath)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return std::string();
    }
  std::string slicerSubDir;
  std::string moduleTypeSubDir;
  std::string libDirectory = GetModuleHomeDirectory(filePath, slicerSubDir, moduleTypeSubDir);
  libDirectory.append("/lib");
  libDirectory.append("/");
  libDirectory.append(slicerSubDir);
  return libDirectory;
}
