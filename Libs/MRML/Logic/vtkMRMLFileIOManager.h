/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLFileIOManager_h
#define __vtkMRMLFileIOManager_h

#include "vtkMRMLLogicExport.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

#include <string>
#include <vector>

class vtkCollection;
class vtkImageData;
class vtkMRMLMessageCollection;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLStorableNode;
class vtkMRMLStorageNode;
class vtkMRMLApplicationLogic;
class vtkMRMLFileIOHandler;
class vtkMRMLFileReader;
class vtkMRMLFileWriter;
class vtkMRMLIOProperties;
class vtkMRMLNodeWriter;
class vtkMRMLOperationProgress;

/// Manages reading and writing of files: keeps the list of registered readers and writers, chooses the reader
/// or writer to use for a file or a node, and loads and saves nodes using them.
/// The application logic owns the file IO manager of the application (vtkMRMLApplicationLogic::GetFileIOManager()).
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#file-reading-and-writing
class VTK_MRML_LOGIC_EXPORT vtkMRMLFileIOManager : public vtkObject
{
public:
  static vtkMRMLFileIOManager* New();
  vtkTypeMacro(vtkMRMLFileIOManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
  {
    /// A reader or writer is registered. Call data: the reader or writer (vtkMRMLFileIOHandler).
    HandlerRegisteredEvent = 24100,
    /// A reader or writer is unregistered. Call data: the reader or writer (vtkMRMLFileIOHandler).
    HandlerUnregisteredEvent,
    /// A file is loaded. Call data: vtkMRMLIOProperties of the loaded file,
    /// which contains "fileType" and the IDs of the loaded nodes in "nodeIDs".
    NewFileLoadedEvent,
    /// A file is saved. Call data: vtkMRMLIOProperties of the saved file.
    FileSavedEvent,
    /// Loading or saving of files is started. Call data: vtkMRMLOperationProgress.
    OperationStartEvent,
    /// Progress of loading or saving files changed. Call data: vtkMRMLOperationProgress.
    /// The number of these events per second is limited by MaximumProgressUpdatesPerSecond (except when processing of a new file starts).
    OperationProgressEvent,
    /// Loading or saving of files is completed. Call data: vtkMRMLOperationProgress.
    OperationEndEvent,
  };

  /// The scene the handlers read into and write from.
  virtual void SetScene(vtkMRMLScene* scene);
  vtkMRMLScene* GetScene() const;

  /// Application logic that owns this manager (not reference counted).
  /// Readers and writers may use it for accessing application-wide logic, such as the color logic.
  void SetApplicationLogic(vtkMRMLApplicationLogic* applicationLogic);
  vtkMRMLApplicationLogic* GetApplicationLogic() const;

  //@{
  /// Add a reader or a writer. The manager keeps a reference to it; registering the same object
  /// twice does nothing.
  void RegisterReader(vtkMRMLFileReader* reader);
  void RegisterWriter(vtkMRMLFileWriter* writer);
  /// Add a reader or a writer (calls RegisterReader() or RegisterWriter(), depending on the type of the handler).
  void RegisterHandler(vtkMRMLFileIOHandler* handler);
  /// Convenience method for creating and registering a vtkMRMLNodeWriter,
  /// which writes nodes of the specified classes using their storage nodes.
  /// Returns the registered writer.
  /// \sa vtkMRMLNodeWriter::CreateNodeWriter()
  vtkMRMLNodeWriter* RegisterNodeWriter(const std::string& description,
                                        const std::string& fileType,
                                        const std::vector<std::string>& nodeClassNames,
                                        bool supportUseCompression = true);
  /// Remove a reader or a writer. Whoever registered a handler is responsible for unregistering it.
  void UnregisterHandler(vtkMRMLFileIOHandler* handler);
  //@}

  //@{
  /// Get a registered reader or writer by its class name (exact match). Returns nullptr if not found.
  vtkMRMLFileReader* GetReaderByClassName(const std::string& className) const;
  vtkMRMLFileWriter* GetWriterByClassName(const std::string& className) const;
  //@}

  //@{
  /// What is registered.
  int GetNumberOfReaders() const;
  vtkMRMLFileReader* GetNthReader(int index) const;
  int GetNumberOfWriters() const;
  vtkMRMLFileWriter* GetNthWriter(int index) const;
  //@}

  //@{
  /// Readers

  /// The readers that can read this file, the one that is surest of itself first. Only readers
  /// that answer a confidence above zero are in the list.
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> GetReadersForFile(const std::string& filePath);
  /// The reader that is surest it can read this file, or nullptr.
  vtkMRMLFileReader* GetReaderForFile(const std::string& filePath);
  /// The file type of the reader that is surest it can read this file ("VolumeFile"), or an empty string.
  std::string GetFileTypeForFile(const std::string& filePath);
  /// All the file types that can be used for reading the file, most likely first.
  std::vector<std::string> GetFileTypesForFile(const std::string& filePath);
  /// The descriptions ("Volume", "Segmentation", ...) of the readers that can read the file, most likely first.
  std::vector<std::string> GetFileDescriptionsForFile(const std::string& filePath);

  /// The reader that has the specified description, or nullptr.
  vtkMRMLFileReader* GetReaderByDescription(const std::string& description);
  /// The file type of the reader that has the specified description, or an empty string.
  std::string GetFileTypeFromDescription(const std::string& description);
  /// Descriptions of all the readers of a file type. Usually there is only one reader for a file type.
  std::vector<std::string> GetFileDescriptionsByType(const std::string& fileType);

  /// Iterates through readers looking at the file list to see if there is an entry that can serve as
  /// an archetype for loading multiple files. If so, the reader removes the recognized
  /// files from the list and sets the ioProperties so that the corresponding
  /// loader will read these files. The "fileName" property is set to the archetype file.
  /// Returns the reader that recognized the files or nullptr if no pattern is recognized.
  /// If multiple readers recognize a group of files then the one with the highest confidence
  /// (see vtkMRMLFileReader::ExamineFileListConfidence) is used.
  /// The specific motivating use case is when the file list contains a set of related files,
  /// such as a list of image files that are recognized as a volume.
  /// In Python, fileList must be a list (it is modified in place).
  vtkMRMLFileReader* ExamineFileList(std::vector<std::string>& fileList, vtkMRMLIOProperties* ioProperties);
  //@}

  //@{
  /// Writers

  /// The writers that can write this node, the one that is surest of itself first.
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> GetWritersForObject(vtkObject* object);
  /// Return the best file writer for this object.
  /// Best match is the writer that supports the node type and the specific format (name filter, such as "NRRD (.nrrd)"),
  /// closest match is the writer that supports the node type but not that specific format.
  /// If there are multiple matches then the one with the highest confidence is returned.
  /// If format is empty then the writer with the highest confidence is returned.
  vtkMRMLFileWriter* GetWriterForObject(vtkObject* object, const std::string& format = std::string());
  /// The file type of the best file writer for this object, or an empty string.
  std::string GetFileWriterFileType(vtkObject* object, const std::string& format = std::string());
  /// Same as GetFileWriterFileType(object).
  std::string GetFileTypeForObject(vtkObject* object);
  /// Descriptions of all writers of a file type.
  std::vector<std::string> GetFileWriterDescriptions(const std::string& fileType);
  /// Name filters of all writers that can write the object, the most confident writer's first.
  std::vector<std::string> GetFileWriterExtensions(vtkObject* object);
  //@}

  //@{
  /// The file types that are registered, and what belongs to each of them.
  std::vector<std::string> GetReaderFileTypes();
  std::vector<std::string> GetWriterFileTypes();
  /// The extensions of every reader (or writer) of a file type: ".mcs", ".mcs.gz".
  std::vector<std::string> GetExtensionsForFileType(const std::string& fileType, bool writers);
  /// The name filters of every reader (or writer) of a file type: "Mimics project (*.mcs)".
  std::vector<std::string> GetNameFiltersForFileType(const std::string& fileType, bool writers);
  /// What a file type is called ("Volume"), from the first handler that has a description.
  std::string GetDescriptionForFileType(const std::string& fileType, bool writers);
  /// The readers of a file type, in the order they were registered.
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> GetReadersForFileType(const std::string& fileType);
  /// The writers of a file type, in the order they were registered.
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> GetWritersForFileType(const std::string& fileType);
  //@}

  //@{
  /// File extensions and names

  /// All the writable file extensions for all registered types of storage nodes. Includes the leading dot.
  std::vector<std::string> GetAllWritableFileExtensions();
  /// All the readable file extensions for all registered types of storage nodes. Includes the leading dot.
  std::vector<std::string> GetAllReadableFileExtensions();

  /// Returns a full extension for this storable node that is recognised by Slicer IO.
  /// Consults the storage node for a list of known suffixes. If no match is found then "." is returned.
  /// Always includes the leading dot.
  std::string GetCompleteSlicerWritableFileNameSuffix(vtkMRMLStorableNode* node);

  /// Characters that are allowed in file names by ForceFileNameValidCharacters,
  /// as a regular expression character set.
  static std::string GetFileNameValidCharactersPattern();

  /// Remove characters that are likely to cause problems in a filename
  static std::string ForceFileNameValidCharacters(const std::string& filename);

  /// Clamp the length of a filename to a maximum number of characters.
  /// The length of the filename extension must also be specified so that it is not included in the shortened section.
  /// If the extension is not known then ExtractKnownExtension() method can be used to get the extension.
  /// If maxLength is negative, the default maximum length (DefaultMaximumFileNameLength) is used.
  std::string ForceFileNameMaxLength(const std::string& filename, int extensionLength, int maxLength = -1);

  /// Default maximum length for a filename. It is used when maximum filename length is enforced by calling
  /// ForceFileNameMaxLength() without providing a specific maximum length value.
  vtkGetMacro(DefaultMaximumFileNameLength, int);
  vtkSetMacro(DefaultMaximumFileNameLength, int);

  /// If fileName ends with an extension that is associated with the object,
  /// then return that extension. Otherwise return an empty string.
  /// If there are multiple candidates (such as for "something.seg.nrrd" both
  /// ".nrrd" and ".seg.nrrd" extensions match) then the longest is returned.
  std::string ExtractKnownExtension(const std::string& fileName, vtkObject* object);

  /// If fileName ends with an extension that is associated with the object,
  /// then return a stripped version of fileName, where that extension
  /// has been chopped off. If the extension is duplicated in the
  /// tail of fileName, then all duplicates are stripped away.
  std::string StripKnownExtension(const std::string& fileName, vtkObject* object);
  //@}

  //@{
  /// Loading

  /// Load nodes using the readers of the file type.
  /// The properties must contain "fileName" (a string or a list of strings) and may contain more specific properties.
  /// For example, the volume reader can be called with the following parameters: "labelmap" (bool), "center" (bool).
  /// Nodes that are loaded are added to loadedNodes (if not nullptr).
  /// If a valid userMessages is passed then additional error or warning information is added to it.
  /// Returns false if loading fails.
  ///
  /// Progress is reported by OperationStartEvent, OperationProgressEvent, and OperationEndEvent.
  /// A list of files is loaded as one operation, which can be aborted between files.
  virtual bool LoadNodes(const std::string& fileType, //
                         vtkMRMLIOProperties* properties,
                         vtkCollection* loadedNodes = nullptr,
                         vtkMRMLMessageCollection* userMessages = nullptr);

  /// Load a list of files. Each item in propertiesList is a vtkMRMLIOProperties object that
  /// specifies the "fileType" property in addition to the properties needed by the reader.
  /// Loading progress is reported (see LoadNodes()).
  virtual bool LoadNodesFromList(vtkCollection* propertiesList, vtkCollection* loadedNodes = nullptr, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Load nodes and return the first loaded node.
  vtkMRMLNode* LoadNodesAndGetFirst(const std::string& fileType, vtkMRMLIOProperties* properties, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Load or import a scene.
  bool LoadScene(const std::string& fileName, bool clear = true, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Load a file. All the options (e.g., file type) are chosen by default.
  bool LoadFile(const std::string& fileName, vtkMRMLMessageCollection* userMessages = nullptr);
  //@}

  //@{
  /// Saving

  /// Save nodes (or scene) using the registered writers.
  /// Progress is reported the same way as for loading (see LoadNodes()).
  /// Properties are typically: "fileName"; for nodes: "nodeID", "useCompression", "fileFormat".
  /// If a valid scene is passed then the writers use that scene instead of the scene of the manager.
  /// Returns true on success.
  bool SaveNodes(const std::string& fileType, vtkMRMLIOProperties* properties, vtkMRMLMessageCollection* userMessages = nullptr, vtkMRMLScene* scene = nullptr);

  /// Export nodes using the registered writers. Unlike SaveNodes(), this function creates a temporary scene
  /// while saving, in order to avoid modifying storage nodes in the current scene.
  /// Saving progress is reported (see SaveNodes()), each exported node counts as one file. Exporting of multiple nodes is abortable.
  /// Each item in propertiesList is a vtkMRMLIOProperties object that specifies a "nodeID" (ID of a node
  /// in the main scene), a "fileName" (an absolute file path), a "fileFormat" (e.g. "NRRD (.nrrd)"),
  /// and any other options that the associated writer may use.
  /// If hardenTransforms is true then transforms are temporarily hardened before export.
  /// If a node cannot be exported then the remaining nodes are still exported.
  /// Returns true if all nodes were exported successfully.
  bool ExportNodes(vtkCollection* propertiesList, bool hardenTransforms, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Export nodes, with the same properties for all nodes.
  bool ExportNodes(const std::vector<std::string>& nodeIDs,
                   const std::vector<std::string>& fileNames,
                   vtkMRMLIOProperties* commonProperties,
                   bool hardenTransforms,
                   vtkMRMLMessageCollection* userMessages = nullptr);

  /// Save the scene. Equivalent to calling SaveNodes with "SceneFile" file type
  /// with the fileName and screenShot set as properties.
  bool SaveScene(const std::string& fileName, vtkImageData* screenShot = nullptr, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Create default storage nodes for all storable nodes that are to be saved
  /// with the scene and do not have a storage node already.
  void AddDefaultStorageNodes();

  /// Create and add default storage node
  static vtkMRMLStorageNode* CreateAndAddDefaultStorageNode(vtkMRMLStorableNode* node);
  //@}

  /// Invoke NewFileLoadedEvent. It may be used by readers that load files without the manager.
  void InvokeNewFileLoadedEvent(vtkMRMLIOProperties* loadedFileProperties);
  /// Invoke FileSavedEvent. It may be used by writers that save files without the manager.
  void InvokeFileSavedEvent(vtkMRMLIOProperties* savedFileProperties);

  /// Maximum number of progress events (OperationProgressEvent) per second while loading or saving files.
  /// Progress changes that would exceed this rate are reported in the next progress event.
  /// 0 means that every progress change is reported. Default: 3.
  vtkGetMacro(MaximumProgressUpdatesPerSecond, double);
  vtkSetMacro(MaximumProgressUpdatesPerSecond, double);

  /// Defines the file format that should be offered by default when the scene is saved.
  /// Valid options are defined by the scene writer (for example, "MRML Scene (.mrml)"
  /// or "Medical Reality Bundle (.mrb)").
  std::string GetDefaultSceneFileType() const;
  void SetDefaultSceneFileType(const std::string& fileType);

protected:
  vtkMRMLFileIOManager();
  ~vtkMRMLFileIOManager() override;

  /// Handlers with their confidence, highest first; ties keep the order of registration.
  /// Handlers with zero confidence are not included.
  template <typename HandlerType>
  static std::vector<vtkSmartPointer<HandlerType>> SortedByConfidence(const std::vector<vtkSmartPointer<HandlerType>>& handlers, //
                                                                      const std::vector<double>& confidences);

  /// Writers that can write the file, writers that match the file extension first and generic writers
  /// (that can write any file) last.
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> GetWritersForFile(const std::string& fileType, vtkMRMLIOProperties* properties, vtkMRMLScene* scene);

  vtkWeakPointer<vtkMRMLScene> Scene;
  vtkWeakPointer<vtkMRMLApplicationLogic> ApplicationLogic;
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> Readers;
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> Writers;

  std::string DefaultSceneFileType;
  int DefaultMaximumFileNameLength{ 1000 };

  /// Start a loading or saving operation, if no operation is in progress (invokes OperationStartEvent).
  /// numberOfFiles is 0 if a single file is processed (progress of processing a file is only known if it is reported).
  /// Returns true if a new operation is started.
  bool StartOperationProgress(const std::string& title, int numberOfFiles);
  /// End the operation (invokes OperationEndEvent), if it was started by StartOperationProgress().
  /// If the operation is not aborted then completion of the operation is reported.
  void EndOperationProgress(bool operationStarted, bool aborted = false);
  /// State of processing (loading or saving) a list of files.
  struct ListProgress
  {
    /// True if processing of the list started an operation (it is not part of another operation)
    bool OperationStarted{ false };
    int NumberOfFiles{ 0 };
    int NumberOfProcessedFiles{ 0 };
    /// Part of the overall progress range that processing of the list covers
    double RangeMinimum{ 0.0 };
    double RangeMaximum{ 1.0 };
    /// True if processing of the list was aborted (some files were not processed)
    bool Aborted{ false };
  };
  /// Start processing a list of files (starts an operation if no operation is in progress).
  /// Each file covers an equal part of the progress range of the list. If the operation is started by the list
  /// then the files of the list are the files of the operation (see vtkMRMLOperationProgress::GetNumberOfFiles()).
  ListProgress StartListProgress(const std::string& title, int numberOfFiles);
  /// Report that processing of the file at fileIndex starts.
  void StartListItemProgress(ListProgress& listProgress, int fileIndex);
  /// Report that processing of the file at fileIndex is completed.
  /// Returns true if processing of the list must be stopped, because the operation is aborted.
  bool EndListItemProgress(ListProgress& listProgress, int fileIndex);
  /// End processing of the list. If processing was aborted then a warning is added to userMessages
  /// (abortedMessageFormat contains %1 for the number of processed files and %2 for the number of files).
  void EndListProgress(ListProgress& listProgress, vtkMRMLMessageCollection* userMessages, const std::string& abortedMessageFormat);

  /// Export a single node to a file. Used by ExportNodes() to export each node of the list.
  ///
  /// The node is copied into temporaryScene (with its display node and, if requested, with its parent transforms
  /// hardened) and saved from there, so that the storage nodes of the main scene are not modified.
  ///
  /// \param properties Export options: "nodeID" (ID of the node in the main scene), "fileName", "fileFormat",
  ///   and any other options that the writer uses.
  /// \param temporaryScene Scene that ExportNodes() created for exporting. It is shared by all nodes of the list.
  /// \param hardenTransforms If true, parent transforms of the node are applied to the exported data.
  /// \param userMessages Errors and warnings to display to the user are added to this collection (may be nullptr).
  /// \return True if the node was exported successfully. False if any error occurred, for example: the "nodeID",
  ///   "fileName", or "fileFormat" property is missing, no storable node exists with the specified "nodeID",
  ///   the node cannot be copied into the temporary scene, or the writer fails to write the file.
  bool ExportNode(vtkMRMLIOProperties* properties, vtkMRMLScene* temporaryScene, bool hardenTransforms, vtkMRMLMessageCollection* userMessages);

  /// Report progress of processing (loading or saving) the current file (0.0-1.0; negative value if only the text changes)
  /// and optionally update the progress text.
  void ReportFileProgress(double fileProgress, const std::string& text = std::string());
  /// Forwards scene import or save progress (percent) as file progress.
  static void OnSceneProgress(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Invoke OperationProgressEvent with the current progress, unless the maximum update rate would be exceeded
  /// (then the event is invoked later). If force is true then the event is invoked regardless of the update rate.
  void InvokeProgressEvent(bool force = false);

  double MaximumProgressUpdatesPerSecond{ 3.0 };
  /// Time of the last invoked progress event (universal time in seconds)
  double LastProgressEventTime{ 0.0 };
  /// True if progress changed since the last progress event
  bool ProgressEventPending{ false };

  /// Progress of the loading or saving operation that is in progress (nullptr if no operation is in progress)
  vtkSmartPointer<vtkMRMLOperationProgress> OperationProgress;
  /// True while the scene is observed for reporting its import progress
  bool SceneProgressObserved{ false };

private:
  vtkMRMLFileIOManager(const vtkMRMLFileIOManager&) = delete;
  void operator=(const vtkMRMLFileIOManager&) = delete;
};

#endif
