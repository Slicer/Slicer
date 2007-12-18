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
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkSlicerTask.h"
#include "vtkMRMLNRRDStorageNode.h"

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

//  std::cout << "Requesting " << filename << " be read into node " << refNode << ", display data = " << (displayData?"true":"false") <<  std::endl;

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


void vtkSlicerApplicationLogic::ProcessReadNodeData(ReadDataRequest& req)
{
  // What type of node is the data really? Or is it a scene
  vtkMRMLNode *nd = 0;
  vtkMRMLDisplayNode *disp = 0;
  vtkMRMLStorageNode *in = 0;
  vtkMRMLScalarVolumeNode *svnd = 0;
  vtkMRMLVectorVolumeNode *vvnd = 0;
  vtkMRMLDiffusionTensorVolumeNode *dtvnd = 0;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *dwdpn = NULL;
  vtkMRMLDiffusionWeightedVolumeNode *dwvnd = 0;
  vtkMRMLModelNode *mnd = 0;
  vtkMRMLLinearTransformNode *ltnd = 0;
  vtkMRMLFiberBundleNode *fbnd = 0;
  vtkMRMLColorTableNode *cnd = 0;
  
  nd = this->MRMLScene->GetNodeByID( req.GetNode().c_str() );
  
  svnd  = vtkMRMLScalarVolumeNode::SafeDownCast(nd);
  vvnd  = vtkMRMLVectorVolumeNode::SafeDownCast(nd);
  dtvnd = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(nd);
  dwvnd = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(nd);
  mnd   = vtkMRMLModelNode::SafeDownCast(nd);
  ltnd  = vtkMRMLLinearTransformNode::SafeDownCast(nd);
  fbnd  = vtkMRMLFiberBundleNode::SafeDownCast(nd);
  cnd = vtkMRMLColorTableNode::SafeDownCast(nd);
  
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
      in = vin;
      }
    else if (dtvnd || dwvnd)
      {
      // Load a diffusion tensor or a diffusion weighted node
      //
      // Need to maintain the original coordinate frame established by 
      // the images sent to the execution model 
      vtkMRMLNRRDStorageNode *nin = vtkMRMLNRRDStorageNode::New();
      nin->SetCenterImage(0);
      in = nin;
      }
    else if (fbnd)
      {
      // Load a fiber bundle node
      in = vtkMRMLFiberBundleStorageNode::New();
      }
    else if (cnd)
      {
      // load in a color node
      in = vtkMRMLColorTableStorageNode::New();
      }
    else if (mnd)
      {
      // Load a model node
      in = vtkMRMLModelStorageNode::New();
      }
    else if (ltnd)
      {
      // Load a linear transform node
      
      // no storage node for transforms, need to read a scene (should
      // have been in ProcessReadSceneData()
      }
    
    // Have the storage node read the data into the current node
    if (in)
      {
      try
        {
        vtkMRMLDisplayableNode *displayableNode = 
          vtkMRMLDisplayableNode::SafeDownCast(nd);
        if (displayableNode)
          {
          this->MRMLScene->AddNode( in );
          displayableNode->SetReferenceStorageNodeID( in->GetID() );
          }
        in->SetFileName( req.GetFilename().c_str() );
        in->ReadData( nd );
        in->SetFileName( NULL ); // clear temp file name
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
      in->Delete();
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
  else if (ltnd)
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
      if ( vnd )
        {
          vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesLogic::New();
          volumesLogic->CalculateAutoLevels (vnd->GetImageData(), svdnd);
          volumesLogic->Delete();
        }
      } 
    if (svnd) svnd->SetAndObserveDisplayNodeID( disp->GetID() );
    else if (vvnd) vvnd->SetAndObserveDisplayNodeID( disp->GetID() );
    else if (dtvnd) dtvnd->SetAndObserveDisplayNodeID( disp->GetID() );
    else if (dwvnd) dwvnd->SetAndObserveDisplayNodeID( disp->GetID() );
    else if (mnd) mnd->SetAndObserveDisplayNodeID( disp->GetID() );
    
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
