/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
///  vtkSlicerApplicationLogic - the main logic to manage the application
///
/// The Main entry point for the slicer3 application.
/// -- manages the connection to the mrml scene
/// -- manages the creation of Views and Slices (logic only)
/// -- serves as central point for dispatching events
/// There is a corresponding vtkSlicerApplicationGUI class that provides
/// a user interface to this class by observing this class.
//

#ifndef __vtkSlicerApplicationLogic_h
#define __vtkSlicerApplicationLogic_h

// Slicer includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkCollection.h>

// ITK includes
#include <itkPlatformMultiThreader.h>

// STL includes
#include <mutex>

class vtkMRMLSelectionNode;
class vtkMRMLInteractionNode;
class vtkMRMLRemoteIOLogic;
class vtkDataIOManagerLogic;
class vtkPersonInformation;
class vtkSlicerTask;
class ModifiedQueue;
class ProcessingTaskQueue;
class ReadDataQueue;
class ReadDataRequest;
class WriteDataQueue;
class WriteDataRequest;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerApplicationLogic
  : public vtkMRMLApplicationLogic
{
  public:

  /// The Usual vtk class functions
  static vtkSlicerApplicationLogic *New();
  vtkTypeMacro(vtkSlicerApplicationLogic, vtkMRMLApplicationLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Update the data IO, local and remote, with the new scene
  /// For stand alone applications, follow the set up steps in
  /// qSlicerCoreApplicationPrivate::initDataIO() to set up the
  /// remote IO logic and data manager logic and then call this
  /// method to hook them into the scene.
  /// \sa qSlicerCoreApplicationPrivate::initDataIO()
  /// \sa vtkMRMLRemoteIOLogic::AddDataIOToScene()
  void SetMRMLSceneDataIO(vtkMRMLScene *scene,
                          vtkMRMLRemoteIOLogic *remoteIOLogic,
                          vtkDataIOManagerLogic *dataIOManagerLogic);


  /// Perform the default behaviour related to selecting a fiducial list
  /// (display it in the Fiducials GUI)
  void PropagateFiducialListSelection();

  /// Create a thread for processing
  void CreateProcessingThread();

  /// Shutdown the processing thread
  void TerminateProcessingThread();
  /// List of events potentially fired by the application logic
  enum RequestEvents
    {
      RequestModifiedEvent = vtkMRMLApplicationLogic::RequestInvokeEvent + 1,
      RequestReadDataEvent,
      RequestWriteDataEvent,
      /// Event fired when a readData, writeData or readScene request
      /// has been processed.
      /// The uid of the request is passed as callData.
      /// \todo Add support for "modified" request.
      RequestProcessedEvent
    };

  /// Schedule a task to run in the processing thread. Returns true if
  /// task was successfully scheduled. ScheduleTask() is called from the
  /// main thread to run something in the processing thread.
  int ScheduleTask( vtkSlicerTask* );

  /// Request a Modified call on an object.  This method allows a
  /// processing thread to request a Modified call on an object to be
  /// performed in the main thread.  This allows the call to Modified
  /// to trigger GUI changes. RequestModified() is called from the
  /// processing thread to modify an object in the main thread.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered.
  /// \todo Fire RequestProcessedEvent when processing Modified requests.
  /// \sa RequestReadData(), RequestWriteData()
  vtkMTimeType RequestModified(vtkObject *);

  /// Request that data be read from a file and set it on the referenced
  /// node.  The request will be sent to the main thread which will be
  /// responsible for reading the data, setting it on the referenced
  /// node, and updating the display.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered. When the request is processed,
  /// RequestProcessedEvent is invoked with the request UID as calldata.
  /// \sa RequestReadScene(), RequestWriteData(), RequestModified()
  vtkMTimeType RequestReadFile(const char *refNode, const char *filename,
    int displayData = false, int deleteFile = false);

  /// Request setting of parent transform.
  /// The request will executed on the main thread.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered. When the request is processed,
  /// RequestProcessedEvent is invoked with the request UID as calldata.
  /// \sa RequestReadScene(), RequestWriteData(), RequestModified()
  vtkMTimeType RequestUpdateParentTransform(const std::string &updatedNode, const std::string& parentTransformNode);

  /// Request setting of subject hierarchy location (will have the same parent and same level as sibling node).
  /// The request will executed on the main thread.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered. When the request is processed,
  /// RequestProcessedEvent is invoked with the request UID as calldata.
  /// \sa RequestReadScene(), RequestWriteData(), RequestModified()
  vtkMTimeType RequestUpdateSubjectHierarchyLocation(const std::string &updatedNode, const std::string& siblingNode);

  /// Request adding a node reference
  /// The request will executed on the main thread.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered. When the request is processed,
  /// RequestProcessedEvent is invoked with the request UID as calldata.
  /// \sa RequestReadScene(), RequestWriteData(), RequestModified()
  vtkMTimeType RequestAddNodeReference(const std::string &referencingNode, const std::string& referencedNode, const std::string& role);

  /// Return the number of items that need to be read from the queue
  /// (this allows code that invokes command line modules to know when
  /// multiple items are being returned and have all been returned).
  unsigned int GetReadDataQueueSize();


  /// Request that data be written from a file to a remote destination.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered.  When the request is processed,
  /// RequestProcessedEvent is invoked with the request UID as calldata.
  /// \sa RequestReadData(), RequestReadScene()
  vtkMTimeType RequestWriteData(const char *refNode, const char *filename);

  /// Request that a scene be read from a file. Mappings of node IDs in
  /// the file (sourceIDs) to node IDs in the main scene
  /// (targetIDs) can be specified. Only nodes listed in sourceIDs are
  /// loaded back into the main scene.  Hierarchical nodes will be
  /// handled specially, in that only the top node needs to be listed
  /// in the sourceIds.
  /// Return the request UID (monotonically increasing) of the request or 0 if
  /// the request failed to be registered. When the request is processed,
  /// RequestProcessedEvent is invoked with the request UID as calldata.
  /// \sa RequestReadData(), RequestWriteData()
  vtkMTimeType RequestReadScene(const std::string& filename,
                       std::vector<std::string> &targetIDs,
                       std::vector<std::string> &sourceIDs,
                       int displayData = false,
                       int deleteFile = false);

  /// Process a request on the Modified queue.  This method is called
  /// in the main thread of the application because calls to Modified()
  /// can cause an update to the GUI. (Method needs to be public to fit
  /// in the event callback chain.)
  void ProcessModified();

  /// Process a request to read data and set it on a referenced node.
  /// This method is called in the main thread of the application
  /// because calls to load data will cause a Modified() on a node
  /// which can force a render.
  void ProcessReadData();

  /// Process a request to write data from a referenced node.
  void ProcessWriteData();

  /// These routings act as place holders so that test scripts can
  /// turn on and off tracing.  These are just hooks
  /// for use with external tracing tool (such as AQTime)
  void SetTracingOn () { this->Tracing = 1; }
  void SetTracingOff () { this->Tracing = 0; }

  /// Return True if \a filePath is a descendant of \a applicationHomeDir.
  /// \note On MacOSX extensions are installed in the "<Slicer_EXTENSIONS_DIRBASENAME>-<slicerRevision>"
  /// folder being a sub directory of the application dir, an extra test is performed to make sure the
  /// tested filePath doesn't belong to that "<Slicer_EXTENSIONS_DIRBASENAME>-<slicerRevision>" folder.
  /// If this is the case, False will be returned.
  static bool IsEmbeddedModule(const std::string& filePath, const std::string& applicationHomeDir,
                               const std::string& slicerRevision);

  /// Return \a true if the plugin identified with its \a filePath is loaded from an install tree.
  /// \warning Since internally the function looks for the existence of CMakeCache.txt, it will
  /// return an incorrect result if the plugin is installed in the build tree of
  /// an other project.
  static bool IsPluginInstalled(const std::string& filePath, const std::string& applicationHomeDir);

  /// Return \a true if the plugin identified with its \a filePath is a built-in Slicer module.
  static bool IsPluginBuiltIn(const std::string& filePath, const std::string& applicationHomeDir);

  /// Get share directory associated with \a moduleName located in \a filePath
  static std::string GetModuleShareDirectory(const std::string& moduleName, const std::string& filePath);

  /// Get Slicer-X.Y share directory associated with module located in \a filePath
  static std::string GetModuleSlicerXYShareDirectory(const std::string& filePath);

  /// Get Slicer-X.Y lib directory associated with module located in \a filePath
  static std::string GetModuleSlicerXYLibDirectory(const std::string& filePath);

  /// Get information about the current user (name, login, email, organization, etc.)
  /// Values are allowed to be modified.
  vtkPersonInformation* GetUserInformation();

protected:

  vtkSlicerApplicationLogic();
  ~vtkSlicerApplicationLogic() override;

   /// Callback used by a MultiThreader to start a processing thread
  static itk::ITK_THREAD_RETURN_TYPE ProcessingThreaderCallback( void * );

   /// Callback used by a MultiThreader to start a networking thread
  static itk::ITK_THREAD_RETURN_TYPE NetworkingThreaderCallback( void * );

  /// Task processing loop that is run in the processing thread
  void ProcessProcessingTasks();

  /// Networking Task processing loop that is run in a networking thread
  void ProcessNetworkingTasks();

  /// Process a request to read data into a scene.  This method is
  /// called by ProcessReadData() in the application main thread
  /// because calls to load data will cause a Modified() on a node
  /// which can force a render.
  void ProcessReadSceneData( ReadDataRequest &req );
  void ProcessWriteSceneData( WriteDataRequest &req );

  /// Set background thread (background processing, networking) priority, which
  /// can be set via an environment variable SLICER_BACKGROUND_THREAD_PRIORITY.
  /// Value of the variable must be an integer
  /// specifying background threads priority (default: 20).
  virtual void SetCurrentThreadPriorityToBackground();

private:
  vtkSlicerApplicationLogic(const vtkSlicerApplicationLogic&);
  void operator=(const vtkSlicerApplicationLogic&);

  itk::PlatformMultiThreader::Pointer ProcessingThreader;
  std::mutex ProcessingThreadActiveLock;
  std::mutex ProcessingTaskQueueLock;
  std::mutex ModifiedQueueActiveLock;
  std::mutex ModifiedQueueLock;
  std::mutex ReadDataQueueActiveLock;
  std::mutex ReadDataQueueLock;
  std::mutex WriteDataQueueActiveLock;
  std::mutex WriteDataQueueLock;
  vtkTimeStamp RequestTimeStamp;
  int ProcessingThreadId;
  std::vector<int> NetworkingThreadIDs;
  int ProcessingThreadActive;
  int ModifiedQueueActive;
  int ReadDataQueueActive;
  int WriteDataQueueActive;

  ProcessingTaskQueue* InternalTaskQueue;
  ModifiedQueue*       InternalModifiedQueue;
  ReadDataQueue*       InternalReadDataQueue;
  WriteDataQueue*      InternalWriteDataQueue;

  vtkPersonInformation* UserInformation;

  /// For use with external tracing tool (such as AQTime)
  int Tracing;
};

#endif
