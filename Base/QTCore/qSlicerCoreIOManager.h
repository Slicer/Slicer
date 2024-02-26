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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerCoreIOManager_h
#define __qSlicerCoreIOManager_h

// Qt includes
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariantMap>

// CTK includes
#include <ctkPimpl.h>

// QtCore includes
#include <qSlicerIO.h>
#include "qSlicerBaseQTCoreExport.h"

class vtkMRMLMessageCollection;
class vtkMRMLNode;
class vtkMRMLStorableNode;
class vtkMRMLStorageNode;
class vtkCollection;
class vtkObject;
class qSlicerCoreIOManagerPrivate;
class qSlicerFileReader;
class qSlicerFileWriter;
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreIOManager : public QObject
{
  Q_OBJECT;
  Q_PROPERTY(QString defaultSceneFileType READ defaultSceneFileType WRITE setDefaultSceneFileType)

public:
  qSlicerCoreIOManager(QObject* parent = nullptr);
  ~qSlicerCoreIOManager() override;

  /// Return the most likely file type (SegmentationFile, TextFile, ...) for reading a \a file
  Q_INVOKABLE qSlicerIO::IOFileType fileType(const QString& file) const;
  /// Return all supported file types for reading a \a file
  Q_INVOKABLE QList<qSlicerIO::IOFileType> fileTypes(const QString& file) const;

  /// Return the most likely file description (SegmentationFile, TextFile, ...) for reading a \a file
  Q_INVOKABLE qSlicerIO::IOFileType fileTypeFromDescription(const QString& fileDescription) const;

  /// Return the file description ("Volume", "Transform", etc.) associated with a \a file
  /// Usually the description is a short text of one or two words
  /// e.g. Volume, Model, ...
  Q_INVOKABLE QStringList fileDescriptions(const QString& file) const;

  /// Returns descriptions for a file type available across all readers.
  /// Usually there is only one reader for a file type.
  QStringList fileDescriptionsByType(const qSlicerIO::IOFileType fileType) const;

  /// Return best file writer for this object
  qSlicerFileWriter* writer(vtkObject* object, const QString& extension = QString()) const;

  /// Return the file type of the best file writer for the input VTK \a object.
  Q_INVOKABLE qSlicerIO::IOFileType fileWriterFileType(vtkObject* object, const QString& extension = QString()) const;

  Q_INVOKABLE QStringList fileWriterDescriptions(const qSlicerIO::IOFileType& fileType) const;
  Q_INVOKABLE QStringList fileWriterExtensions(vtkObject* object) const;
  /// Return a string list of all the writable file extensions for all
  /// registered types of storage nodes. Includes the leading dot.
  Q_INVOKABLE QStringList allWritableFileExtensions() const;
  /// Return a string list of all the readable file extensions for all
  /// registered types of storage nodes. Includes the leading dot.
  Q_INVOKABLE QStringList allReadableFileExtensions() const;

  /// Return the file read options for the best reader associated with a \a file type
  qSlicerIOOptions* fileOptions(const QString& fileDescription) const;

  /// Return the file write options of the best file writer for the input VTK \a object.
  qSlicerIOOptions* fileWriterOptions(vtkObject* object, const QString& extension) const;

  /// Returns a full extension for this storable node that is recognised by Slicer IO.
  /// Consults the storage node for a list of known suffixes, if no match
  /// is found and the .* extension exists, return the Qt completeSuffix string.
  /// If .* is not in the complete list of known suffixes, returns an empty suffix.
  /// Always includes the leading dot.
  Q_INVOKABLE QString completeSlicerWritableFileNameSuffix(vtkMRMLStorableNode* node) const;

  /// Generate a regular expression that can ensure a filename has a valid
  /// extension. Example of supported extensions:
  /// "", "*", ".*", ".jpg", ".png" ".tar.gz"...
  /// An empty extension or "*" means any filename (or directory) is valid
  Q_INVOKABLE static QRegExp fileNameRegExp(const QString& extension = QString());

  /// Remove characters that are likely to cause problems in a filename
  Q_INVOKABLE static QString forceFileNameValidCharacters(const QString& filename);

  /// If \a fileName ends with an extension that is associated with \a object,
  /// then return that extension. Otherwise return an empty string.
  /// If there are multiple candidates (such as for "something.seg.nrrd" both
  /// ".nrrd" and ".seg.nrrd" extensions match) then the longest is returned.
  Q_INVOKABLE QString extractKnownExtension(const QString& fileName, vtkObject* object);

  /// If \a fileName ends with an extension that is associated with \a object,
  /// then return a stripped version of \a fileName, where that extension
  /// has been chopped off. If the extension is duplicated in the
  /// tail of \a fileName, then all duplicates are stripped away.
  Q_INVOKABLE QString stripKnownExtension(const QString& fileName, vtkObject* object);

  /// Load a list of nodes corresponding to \a fileType. A given \a fileType corresponds
  /// to a specific reader qSlicerIO.
  /// A map of QVariant allows to specify which \a parameters should be passed to the reader.
  /// The function return 0 if it fails.
  /// The map associated with most of the \a fileType should contains either
  /// fileName (QString or QStringList) or fileNames (QStringList).
  /// More specific parameters could also be set. For example, the volume reader qSlicerVolumesIO
  /// could also be called with the following parameters: LabelMap (bool), Center (bool)
  /// If a valid pointer is passed to userMessages additional error or warning information may be returned in it.
  /// \note Make also sure the case of parameter name is respected
  /// \sa qSlicerIO::IOProperties, qSlicerIO::IOFileType, saveNodes()
  Q_INVOKABLE virtual bool loadNodes(const qSlicerIO::IOFileType& fileType,
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkCollection* loadedNodes = nullptr,
                                     vtkMRMLMessageCollection* userMessages = nullptr);

  /// Utility function that loads a bunch of files. The "fileType" attribute should
  /// in the parameter map for each node to load.
  /// If a valid pointer is passed to userMessages additional error or warning information may be returned in it.
  virtual bool loadNodes(const QList<qSlicerIO::IOProperties>& files,
                         vtkCollection* loadedNodes = nullptr,
                         vtkMRMLMessageCollection* userMessages = nullptr);

  /// Load a list of node corresponding to \a fileType and return the first loaded node.
  /// This function is provided for convenience and is equivalent to call loadNodes
  /// with a vtkCollection parameter and retrieve the first element.
  /// If a valid pointer is passed to userMessages additional error or warning information may be returned in it.
  vtkMRMLNode* loadNodesAndGetFirst(qSlicerIO::IOFileType fileType,
                                    const qSlicerIO::IOProperties& parameters,
                                    vtkMRMLMessageCollection* userMessages = nullptr);

  /// Load/import a scene corresponding to \a fileName
  /// This function is provided for convenience and is equivalent to call
  /// loadNodes function with QString("SceneFile").
  /// If a valid pointer is passed to userMessages additional error or warning information may be returned in it.
  Q_INVOKABLE bool loadScene(const QString& fileName,
                             bool clear = true,
                             vtkMRMLMessageCollection* userMessages = nullptr);

  /// Convenient function to load a file. All the options (e.g. filetype) are
  /// chosen by default.
  /// If a valid pointer is passed to userMessages additional error or warning information may be returned in it.
  Q_INVOKABLE bool loadFile(const QString& fileName, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Save nodes (or scene) using the registered writers.
  /// Return true on success, false otherwise.
  /// Attributes are typically:
  /// For all: QString fileName (or QStringList fileNames)
  /// For nodes: QString nodeID, bool useCompression
  /// If a valid pointer is passed to \a userMessages additional error or warning information may be returned in it.
  /// If a valid pointer is passed to \a scene, writers will be told to use that scene instead of the current scene.
  /// \sa qSlicerNodeWriter, qSlicerIO::IOProperties, qSlicerIO::IOFileType,
  /// loadNodes(), exportNodes()
  Q_INVOKABLE bool saveNodes(qSlicerIO::IOFileType fileType,
                             const qSlicerIO::IOProperties& parameters,
                             vtkMRMLMessageCollection* userMessages = nullptr,
                             vtkMRMLScene* scene = nullptr);

  /// Export nodes using the registered writers. Return true on success, false otherwise.
  /// Unlike saveNodes(), this function creates a temporary scene while saving, in order to to avoid modifying storage
  /// nodes in the current scene. The list \a parameterMaps should consist of maps that each specify a "nodeID" (ID of a
  /// node in the main scene), a "fileName" (an absolute file path), a "fileFormat" (e.g. "NRRD (.nrrd)"), and any other
  /// options that the associated writer may end up using. \param parameterMaps For each node to exported, a map of
  /// parameters that will get passed to qSlicerCoreIOManager::saveNodes. \param hardenTransforms Whether to temporarily
  /// apply transform hardening before export. \param userMessages If a valid pointer is passed, then error messages may
  /// be returned in it. \sa qSlicerNodeWriter, qSlicerIO::IOProperties, qSlicerIO::IOFileType, vtkMRMLStorageNode,
  /// saveNodes().
  Q_INVOKABLE bool exportNodes(const QList<qSlicerIO::IOProperties>& parameterMaps,
                               bool hardenTransforms,
                               vtkMRMLMessageCollection* userMessages = nullptr);

  /// Export nodes using the registered writers with an API that is usable from Python.
  /// It only allows exporting all nodes with the same parameters.
  /// Return true on success, false otherwise.
  /// \sa exportNodes().
  Q_INVOKABLE bool exportNodes(const QStringList& nodeIDs,
                               const QStringList& fileNames,
                               const qSlicerIO::IOProperties& commonParameterMap,
                               bool hardenTransforms,
                               vtkMRMLMessageCollection* userMessages = nullptr);

  /// Save a scene corresponding to \a fileName
  /// This function is provided for convenience and is equivalent to call
  /// saveNodes function with QString("SceneFile") with the fileName
  /// and screenShot set as properties.
  /// If a valid pointer is passed to userMessages additional error or warning information may be returned in it.
  Q_INVOKABLE bool saveScene(const QString& fileName,
                             QImage screenShot,
                             vtkMRMLMessageCollection* userMessages = nullptr);

  /// Create default storage nodes for all storable nodes that are to be saved
  /// with the scene and do not have a storage node already
  /// File name is set based on node name, using use default file extension,
  /// with special characters in the node name percent-encoded.
  /// This method can be used to ensure a storage node exists before writing
  /// a storable node to file by calling storableNode->GetStorageNode()->WriteData(storableNode).
  Q_INVOKABLE void addDefaultStorageNodes();

  /// Register the reader/writer \a io
  /// Note also that the IOManager takes ownership of \a io
  void registerIO(qSlicerIO* io);

  /// @{
  /// Return the number of registered \a qSlicerIO associated with \a fileType.
  ///
  /// \a registerIO()
  /// \a qSlicerScriptedLoadableModule::registerIO()
  /// \a qSlicerScriptedLoadableModule::readers()
  /// \a qSlicerScriptedLoadableModule::writers()
  Q_INVOKABLE int registeredFileReaderCount(const qSlicerIO::IOFileType& fileType) const;
  Q_INVOKABLE int registeredFileWriterCount(const qSlicerIO::IOFileType& fileType) const;
  /// }@

  /// Create and add default storage node
  Q_INVOKABLE static vtkMRMLStorageNode* createAndAddDefaultStorageNode(vtkMRMLStorableNode* node);

  /// This function should be used from python scripted module willing to interface with
  /// the qSlicerCoreIOManager. It will emit the signal newFileLoaded().
  /// \sa newFileLoaded()
  Q_INVOKABLE void emitNewFileLoaded(const QVariantMap& loadedFileParameters);

  /// This function should be used from python scripted module willing to interface with
  /// the qSlicerCoreIOManager. It will emit the signal fileSaved().
  /// \sa fileSaved()
  Q_INVOKABLE void emitFileSaved(const QVariantMap& savedFileParameters);

  /// Defines the file format that should be offered by default when the scene is saved.
  Q_INVOKABLE QString defaultSceneFileType() const;

  /// Iterates through readers looking at the fileInfoList to see if there is an entry that can serve as
  /// an archetype for loading multiple fileInfos.  If so, the reader removes the recognized
  /// fileInfos from the list and sets the ioProperties so that the corresponding
  /// loader will read these files. The archetypeEntry will contain the fileInfo
  /// for the archetype and the method returns true.  If no pattern is recognized
  /// the method returns false.
  /// The specific motivating use case is when the file
  /// list contains a set of related files, such as a list of image files that
  /// are recognized as a volume. But other cases could also make sense, such as when
  /// a file format has a set or related files such as textures or material files
  /// for a surface model.
  /// \sa qSlicerDataDialog
  /// \sa qSlicerFileReader
  Q_INVOKABLE bool examineFileInfoList(QFileInfoList& fileInfoList,
                                       QFileInfo& archetypeEntry,
                                       QString& readerDescription,
                                       qSlicerIO::IOProperties& ioProperties) const;

public slots:

  /// Defines the file format that should be offered by default when the scene is saved.
  /// Valid options are defined in qSlicerSceneWriter (for example, "MRML Scene (.mrml)"
  /// or "Medical Reality Bundle (.mrb)").
  void setDefaultSceneFileType(QString);

signals:

  /// This signal is emitted each time a file is loaded using loadNodes()
  /// The \a loadedFileParameters QVariant map contains the parameters
  /// passed to the reader and also the \a fileType and \a nodeIDs keys respectively
  /// associated with a QString and a QStringList.
  /// \sa loadNodes(const qSlicerIO::IOFileType&, const qSlicerIO::IOProperties&, vtkCollection*)
  void newFileLoaded(const qSlicerIO::IOProperties& loadedFileParameters);

  /// This signal is emitted each time a file is saved using saveNodes()
  /// The \a savedFileParameters QVariant map contains the parameters
  /// passed to the writer.
  /// \sa saveNodes()
  void fileSaved(const qSlicerIO::IOProperties& savedFileParameters);

protected:
  /// Returns the list of registered readers
  const QList<qSlicerFileReader*>& readers() const;

  /// Returns the list of registered writers
  const QList<qSlicerFileWriter*>& writers() const;
  /// Returns the list of registered writers for a given fileType
  QList<qSlicerFileWriter*> writers(const qSlicerIO::IOFileType& fileType) const;

  /// Returns the list of registered readers or writers associated with \a fileType
  QList<qSlicerFileReader*> readers(const qSlicerIO::IOFileType& fileType) const;
  qSlicerFileReader* reader(const QString& ioDescription) const;

protected:
  QScopedPointer<qSlicerCoreIOManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCoreIOManager);
  Q_DISABLE_COPY(qSlicerCoreIOManager);
};

#endif
