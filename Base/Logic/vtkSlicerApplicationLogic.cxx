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

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLModelNode.h"

#include "itksys/SystemTools.hxx"

#include <queue>

#include "vtkSlicerTask.h"

vtkCxxRevisionMacro(vtkSlicerApplicationLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerApplicationLogic);

class ProcessingTaskQueue : public std::queue<vtkSmartPointer<vtkSlicerTask> > {};


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

    this->InternalTaskQueue = new ProcessingTaskQueue;
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
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::TerminateProcessingThread()
{
  if (this->ProcessingThreadId != -1)
    {
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
  // pull out the reference to the appLogic
  vtkSlicerApplicationLogic *appLogic
    = (vtkSlicerApplicationLogic*)
    (((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);

  // Tell the appLogic to start processing any tasks slated for the
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
        std::cout << "Number of queued tasks: " << (*this->InternalTaskQueue).size() << std::endl;
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

  std::cout << "Scheduling a task ";
  // only schedule a task if the processing task is up
  this->ProcessingThreadActiveLock->Lock();
  active = this->ProcessingThreadActive;
  this->ProcessingThreadActiveLock->Unlock();

  if (active)
    {
    this->ProcessingTaskQueueLock->Lock();
    (*this->InternalTaskQueue).push( task );
    std::cout << (*this->InternalTaskQueue).size() << std::endl;
    this->ProcessingTaskQueueLock->Unlock();
    
    return true;
    }

  // could not schedule the task
  return false;
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerApplicationLogic:             " << this->GetClassName() << "\n"; 
} 

