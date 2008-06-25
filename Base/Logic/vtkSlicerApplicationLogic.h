/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerApplicationLogic - the main logic to manage the application
// .SECTION Description
// The Main entry point for the slicer3 application.
// -- manages the connection to the mrml scene
// -- manages the creation of Views and Slices (logic only)
// -- serves as central point for dispatching events
// There is a corresponding vtkSlicerApplicationGUI class that provides
// a user interface to this class by observing this class.
//

#ifndef __vtkSlicerApplicationLogic_h
#define __vtkSlicerApplicationLogic_h

#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLInteractionNode.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkSlicerSliceLogic.h"

#include "vtkCollection.h"

#include "itkMultiThreader.h"
#include "itkMutexLock.h"

//BTX
class ProcessingTaskQueue;
class ModifiedQueue;
class ReadDataQueue;
class ReadDataRequest;
class WriteDataQueue;
class WriteDataRequest;
class vtkSlicerTask;
//ETX

//BTX
class SliceLogicMap;
//ETX

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerApplicationLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerApplicationLogic *New();
  vtkTypeRevisionMacro(vtkSlicerApplicationLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
    

  // Description:
  // Connect to the given URL.  Disconnect any currently active 
  // connection to switch to a new connection.  A NULL pointer means
  // to disconnect current and not have a current connection
  // (creates a blank scene for manipulation).
  // Return code tells if connection was completed successfully.
  void Connect (const char *URL) {
    if (this->MRMLScene)
     {
      this->MRMLScene->SetURL(URL);
      this->MRMLScene->Connect();
      }
  };

  // Description:
  // Commit your current scene modifications to the connected URL
  // Return code tells result of commit.
  int Commit () {
    if (this->MRMLScene)
      {
      return (this->MRMLScene->Commit());
      }
    return (0);
  };
  // Description:
  // Commit your current scene modifications to specified URL
  // Return code tells result of commit.
  int Commit (const char *URL) {
    if (this->MRMLScene)
      {
      return (this->MRMLScene->Commit(URL));
      }
    return (0);
  };


  // Description:
  // Additional methods here to manipulate the application:
 
  // Info needed: 
  //   SlicerVersion 
  //   others?


  //
  // Views, Slices, Modules
  // -- these are the fundamental slicer elements
  // -- these are dynamic (discovered, create, deleted at run time)
  //

  // Description:
  // Views are the 3D viewports into the mrml scene
  // SlicerLogic maintains the list of currently active views
  //vtkSetObjectMacro (Views,vtkCollection);
  vtkGetObjectMacro (Views,vtkCollection);

  // Description:
  // the ActiveView is the default destination of UI events
  //vtkSetObjectMacro (ActiveView,vtkSlicerViewLogic);
  //vtkGetObjectMacro (ActiveView,vtkSlicerViewLogic);

  ///// Slices
  // Description:
  // Slices are the 2D viewports that show composited layers
  // of volume data from a particular slice definition.
  //vtkSetObjectMacro (Slices,vtkCollection);
  //vtkGetObjectMacro (Slices,vtkCollection);

  // Description:
  // the ActiveSlice is the default destination of UI events
  vtkSetObjectMacro (ActiveSlice,vtkSlicerSliceLogic);
  vtkGetObjectMacro (ActiveSlice,vtkSlicerSliceLogic);

  // Description:
  // the SelectionNode 
  vtkSetObjectMacro (SelectionNode,vtkMRMLSelectionNode);
  vtkGetObjectMacro (SelectionNode,vtkMRMLSelectionNode);

  // Description:
  // the InteractionNode 
  vtkSetObjectMacro (InteractionNode,vtkMRMLInteractionNode);
  vtkGetObjectMacro (InteractionNode,vtkMRMLInteractionNode);

  // Description:
  // Perform the default behavior related to selecting a volume
  // (in this case, making it the background for all SliceCompositeNodes)
  void PropagateVolumeSelection(int fit);
  void PropagateVolumeSelection() {this->PropagateVolumeSelection(1);}; 

  // Description:
  // Perform the default behaviour related to selecting a fiducial list
  // (display it in the Fiducials GUI)
  void PropagateFiducialListSelection();
  
  ///// Modules
  // Description:
  // Modules are additional pieces of Slicer functionality
  // that are loaded and managed at run time
  //vtkSetObjectMacro (Modules,vtkCollection);
  vtkGetObjectMacro (Modules,vtkCollection);

  // Description:
  // the ActiveModule is the default destination of UI events
  //vtkSetObjectMacro (ActiveModule,vtkSlicerModule);
  //vtkGetObjectMacro (ActiveModule,vtkSlicerModule);

  // Description:
  // Creates a selection node if needed
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );
  virtual void ProcessMRMLEvents () { this->ProcessMRMLEvents( NULL, vtkCommand::NoEvent, NULL ); };

  // Description:
  // Create a thread for processing
  void CreateProcessingThread();

  // Description:
  // Create the default number (three for red, yellow, and green slice viewers) of vtkSlicerLogic objects
  void CreateSliceLogics();
  void DeleteSliceLogics();

  // Description:
  // Shutdown the processing thread 
  void TerminateProcessingThread();
  
  // Description:
  // Schedule a task to run in the processing thread. Returns true if
  // task was successfully scheduled. ScheduleTask() is called from the
  // main thread to run something in the processing thread.
  int ScheduleTask( vtkSlicerTask* );

  // Description:
  // Request a Modified call on an object.  This method allows a
  // processing thread to request a Modified call on an object to be
  // performed in the main thread.  This allows the call to Modified
  // to trigger GUI changes. RequestModified() is called from the
  // processing thread to modify an object in the main thread.
  int RequestModified( vtkObject * );

  // Description:
  // Request that data be read from a file and set it on the referenced
  // node.  The request will be sent to the main thread which will be
  // responsible for reading the data, setting it on the referenced
  // node, and updating the display.
  int RequestReadData(const char *refNode, const char *filename,
                       int displayData = false,
                       int deleteFile=false);

  // Description:
  // Request that data be written from a file to a remote destination.
  int RequestWriteData(const char *refNode, const char *filename,
                       int displayData = false,
                       int deleteFile=false);
  // Description:
  // Request that a scene be read from a file. Mappings of node IDs in
  // the file (sourceIDs) to node IDs in the main scene
  // (targetIDs) can be specified. Only nodes listed in sourceIDs are
  // loaded back into the main scene.  Hierarchical nodes will be
  // handled specially, in that only the top node needs to be listed
  // in the sourceIds.
  //
  //BTX
  int RequestReadScene(const std::string& filename,
                       std::vector<std::string> &targetIDs,
                       std::vector<std::string> &sourceIDs,
                       int displayData = false,
                       int deleteFile = false);
  //ETX
  
  // Description:
  // Process a request on the Modified queue.  This method is called
  // in the main thread of the application because calls to Modified()
  // can cause an update to the GUI. (Method needs to be public to fit
  // in the event callback chain.)
  void ProcessModified();

  // Description:
  // Process a request to read data and set it on a referenced node.
  // This method is called in the main thread of the application
  // because calls to load data will cause a Modified() on a node
  // which can force a render.
  void ProcessReadData();
  // Description:
  // Process a request to write data from a referenced node.
  void ProcessWriteData();

  // Description:
  // Add slice logic to the STL::MAP
  void AddSliceLogic(char *layoutName, vtkSlicerSliceLogic *sliceLogic);

  // Description:
  // Add slice logic to the STL::MAP using the slice logic's name for
  // the key.
  void AddSliceLogic(vtkSlicerSliceLogic *sliceLogic);

  // Description:
  // Get the slice logic for a particular layout "Red", "Green", "Yellow"
  vtkSlicerSliceLogic* GetSliceLogic(char *layoutName);

  // Description:
  // Remove a slice logic from the managed set
  void RemoveSliceLogic(vtkSlicerSliceLogic *sliceLogic);
  void RemoveSliceLogic(char *layoutName);
  
  //
  // Transient Application State
  // -- these are elements that are inherently part of the
  //    currently running application and are not stored in 
  //    the mrml tree
  // -- any state that is expected to be saved and restored
  //    must be either part of the mrml scene or
  //    stored in the registry part of the GUI layer
  //

  void ClearCollections ( );


  // Description:
  // These routings act as place holders so that test scripts can
  // turn on and off tracing.  These are just hooks
  // for use with external tracing tool (such as AQTime)
  void SetTracingOn () { this->Tracing = 1; };
  void SetTracingOff () { this->Tracing = 0; };

protected:

  vtkSlicerApplicationLogic();
  ~vtkSlicerApplicationLogic();
  vtkSlicerApplicationLogic(const vtkSlicerApplicationLogic&);
  void operator=(const vtkSlicerApplicationLogic&);

  // Description:
  // Callback used by a MultiThreader to start a processing thread
  static ITK_THREAD_RETURN_TYPE ProcessingThreaderCallback( void * );

  // Description:
  // Callback used by a MultiThreader to start a networking thread
  static ITK_THREAD_RETURN_TYPE NetworkingThreaderCallback( void * );
  
  // Description:
  // Task processing loop that is run in the processing thread
  void ProcessProcessingTasks();

  // Description:
  // Networking Task processing loop that is run in a networking thread
  void ProcessNetworkingTasks();

  // Description:
  // Process a request to read data into a node.  This method is
  // called by ProcessReadData() in the application main thread
  // because calls to load data will cause a Modified() on a node
  // which can force a render.
  //BTX
  void ProcessReadNodeData( ReadDataRequest &req );
  void ProcessWriteNodeData( WriteDataRequest &req );
  //EXT
  

  // Description:
  // Process a request to read data into a scene.  This method is
  // called by ProcessReadData() in the application main thread
  // because calls to load data will cause a Modified() on a node
  // which can force a render.
  //BTX
  void ProcessReadSceneData( ReadDataRequest &req );
  void ProcessWriteSceneData( WriteDataRequest &req );
  //EXT
  
private:
  // Description:
  // STL::MAP to hold Logics for main slice viewers.
  // pointers to these so that they may be reassigned
  // if three viewers are deleted and recreated, as
  // they are during view reconfiguration for instance.
  //BTX
  SliceLogicMap *InternalSliceLogicMap;
  //ETX
  
  // for now, make these generic collections
  // - maybe they should be subclassed to be type-specific?
  vtkCollection *Views;
  //vtkCollection *Slices;
  vtkCollection *Modules;

  //vtkSlicerViewLogic *ActiveView;
  vtkSlicerSliceLogic *ActiveSlice;
  vtkMRMLSelectionNode *SelectionNode;
  vtkMRMLInteractionNode *InteractionNode;
  //vtkSlicerModuleLogic *ActiveModule;

  //BTX
  itk::MultiThreader::Pointer ProcessingThreader;
  itk::MutexLock::Pointer ProcessingThreadActiveLock;
  itk::MutexLock::Pointer ProcessingTaskQueueLock;
  itk::MutexLock::Pointer ModifiedQueueActiveLock;
  itk::MutexLock::Pointer ModifiedQueueLock;
  itk::MutexLock::Pointer ReadDataQueueActiveLock;
  itk::MutexLock::Pointer ReadDataQueueLock;
  itk::MutexLock::Pointer WriteDataQueueActiveLock;
  itk::MutexLock::Pointer WriteDataQueueLock;
  //ETX
  int ProcessingThreadId;
  //BTX
  std::vector<int> NetworkingThreadIDs;
  //ETX
  int ProcessingThreadActive;
  int ModifiedQueueActive;
  int ReadDataQueueActive;
  int WriteDataQueueActive;

  ProcessingTaskQueue* InternalTaskQueue;
  ModifiedQueue* InternalModifiedQueue;
  ReadDataQueue* InternalReadDataQueue;
  WriteDataQueue* InternalWriteDataQueue;
  
  // Transient Application State
  
  // For use with external tracing tool (such as AQTime)
  int Tracing;
};

#endif

