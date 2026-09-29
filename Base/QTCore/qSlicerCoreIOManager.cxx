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

// Qt includes
#include <QDebug>
#include <QHash>
#include <QPointer>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileReader.h"
#include "qSlicerFileWriter.h"
#include "qSlicerVTKFileReader.h"
#include "qSlicerVTKFileWriter.h"

// CTK includes
#include <ctkUtils.h>

// Slicer Logic includes
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLFileIOManager.h>
#include <vtkMRMLFileReader.h>
#include <vtkMRMLFileWriter.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

namespace
{

//-----------------------------------------------------------------------------
/// VTK reader that calls a Qt-based reader. It allows using legacy Qt-based readers
/// (such as scripted readers) in vtkMRMLFileIOManager.
class vtkSlicerQtFileReaderAdapter : public vtkMRMLFileReader
{
public:
  static vtkSlicerQtFileReaderAdapter* New();
  vtkTypeMacro(vtkSlicerQtFileReaderAdapter, vtkMRMLFileReader);

  void SetQtReader(qSlicerFileReader* reader)
  {
    this->QtReader = reader;
    if (!reader)
    {
      return;
    }
    this->SetFileType(reader->fileType().toStdString());
    this->SetDescription(reader->description().toStdString());
    std::vector<std::string> nameFilters;
    ctk::qListToSTLVector(reader->extensions(), nameFilters);
    this->SetNameFilters(nameFilters);
  }

  /// Qt-based reader that performs all tasks
  qSlicerFileReader* GetQtReader() const { return this->QtReader; }

  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override
  {
    // Qt-based readers that delegate tasks to a VTK-based reader use its options description
    vtkMRMLFileIOHandler* handler = this->QtReader ? this->QtReader->ioHandler() : nullptr;
    if (handler && handler != this)
    {
      handler->GetOptionsDescription(description);
    }
  }

  void SetScene(vtkMRMLScene* scene) override
  {
    this->Superclass::SetScene(scene);
    if (this->QtReader)
    {
      this->QtReader->setMRMLScene(scene);
    }
  }

  double CanLoadFileConfidence(const std::string& filePath) override
  {
    if (!this->QtReader || filePath.empty())
    {
      return 0.0;
    }
    return this->QtReader->canLoadFileConfidence(QString::fromStdString(filePath));
  }

  bool CanLoadFile(const std::string& filePath) override
  {
    if (!this->QtReader || filePath.empty())
    {
      return false;
    }
    return this->QtReader->canLoadFile(QString::fromStdString(filePath));
  }

  bool Load(vtkMRMLIOProperties* properties) override
  {
    this->ClearLoadedNodeIDs();
    if (!this->QtReader)
    {
      return false;
    }
    this->QtReader->setMRMLScene(this->GetScene());
    this->QtReader->userMessages()->ClearMessages();
    bool success = this->QtReader->load(qSlicerIO::fromVTKProperties(properties));
    std::vector<std::string> loadedNodeIDs;
    ctk::qListToSTLVector(this->QtReader->loadedNodes(), loadedNodeIDs);
    this->SetLoadedNodeIDs(loadedNodeIDs);
    this->GetUserMessages()->AddMessages(this->QtReader->userMessages());
    return success;
  }

  std::string ExamineFileList(std::vector<std::string>& fileList, vtkMRMLIOProperties* ioProperties) override
  {
    if (!this->QtReader || !ioProperties)
    {
      return std::string();
    }
    QFileInfoList fileInfoList;
    for (const std::string& fileName : fileList)
    {
      fileInfoList << QFileInfo(QString::fromStdString(fileName));
    }
    QFileInfo archetypeFileInfo;
    qSlicerIO::IOProperties qtProperties = qSlicerIO::fromVTKProperties(ioProperties);
    if (!this->QtReader->examineFileInfoList(fileInfoList, archetypeFileInfo, qtProperties))
    {
      return std::string();
    }
    fileList.clear();
    for (const QFileInfo& fileInfo : fileInfoList)
    {
      fileList.push_back(fileInfo.absoluteFilePath().toStdString());
    }
    vtkNew<vtkMRMLIOProperties> updatedProperties;
    qSlicerIO::toVTKProperties(qtProperties, updatedProperties);
    ioProperties->Update(updatedProperties);
    return archetypeFileInfo.absoluteFilePath().toStdString();
  }

protected:
  vtkSlicerQtFileReaderAdapter() = default;
  ~vtkSlicerQtFileReaderAdapter() override = default;
  QPointer<qSlicerFileReader> QtReader;
};
vtkStandardNewMacro(vtkSlicerQtFileReaderAdapter);

//-----------------------------------------------------------------------------
/// VTK writer that calls a Qt-based writer. It allows using legacy Qt-based writers
/// (such as scripted writers) in vtkMRMLFileIOManager.
class vtkSlicerQtFileWriterAdapter : public vtkMRMLFileWriter
{
public:
  static vtkSlicerQtFileWriterAdapter* New();
  vtkTypeMacro(vtkSlicerQtFileWriterAdapter, vtkMRMLFileWriter);

  void SetQtWriter(qSlicerFileWriter* writer)
  {
    this->QtWriter = writer;
    if (!writer)
    {
      return;
    }
    this->SetFileType(writer->fileType().toStdString());
    this->SetDescription(writer->description().toStdString());
    // Name filters are not set, because writers may support different formats for different objects
    // (they are retrieved by GetNameFiltersForObject).
  }

  /// Qt-based writer that performs all tasks
  qSlicerFileWriter* GetQtWriter() const { return this->QtWriter; }

  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override
  {
    // Qt-based writers that delegate tasks to a VTK-based writer use its options description
    vtkMRMLFileIOHandler* handler = this->QtWriter ? this->QtWriter->ioHandler() : nullptr;
    if (handler && handler != this)
    {
      handler->GetOptionsDescription(description);
    }
  }

  void SetScene(vtkMRMLScene* scene) override
  {
    this->Superclass::SetScene(scene);
    if (this->QtWriter)
    {
      this->QtWriter->setMRMLScene(scene);
    }
  }

  double CanWriteObjectConfidence(vtkObject* object) override
  {
    if (!this->QtWriter)
    {
      return 0.0;
    }
    return this->QtWriter->canWriteObjectConfidence(object);
  }

  bool CanWriteObject(vtkObject* object) override
  {
    if (!this->QtWriter)
    {
      return false;
    }
    return this->QtWriter->canWriteObject(object);
  }

  std::vector<std::string> GetNameFiltersForObject(vtkObject* object) override
  {
    if (!this->QtWriter)
    {
      return std::vector<std::string>();
    }
    std::vector<std::string> nameFilters;
    ctk::qListToSTLVector(this->QtWriter->extensions(object), nameFilters);
    return nameFilters;
  }

  bool Write(vtkMRMLIOProperties* properties) override
  {
    this->ClearWrittenNodeIDs();
    if (!this->QtWriter)
    {
      return false;
    }
    this->QtWriter->setMRMLScene(this->GetScene());
    this->QtWriter->userMessages()->ClearMessages();
    bool success = this->QtWriter->write(qSlicerIO::fromVTKProperties(properties));
    std::vector<std::string> writtenNodeIDs;
    ctk::qListToSTLVector(this->QtWriter->writtenNodes(), writtenNodeIDs);
    this->SetWrittenNodeIDs(writtenNodeIDs);
    this->GetUserMessages()->AddMessages(this->QtWriter->userMessages());
    return success;
  }

protected:
  vtkSlicerQtFileWriterAdapter() = default;
  ~vtkSlicerQtFileWriterAdapter() override = default;
  QPointer<qSlicerFileWriter> QtWriter;
};
vtkStandardNewMacro(vtkSlicerQtFileWriterAdapter);

} // namespace

//-----------------------------------------------------------------------------
class qSlicerCoreIOManagerPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCoreIOManager);

protected:
  qSlicerCoreIOManager* const q_ptr;

public:
  qSlicerCoreIOManagerPrivate(qSlicerCoreIOManager& object);
  ~qSlicerCoreIOManagerPrivate();

  /// Get the VTK-based manager (the application logic's manager, if available)
  vtkMRMLFileIOManager* fileIOManager();

  /// Qt-based reader or writer of a VTK-based handler.
  /// If the handler was not registered via registerIO then a generic Qt object is created for it.
  qSlicerFileReader* qtReader(vtkMRMLFileReader* reader);
  qSlicerFileWriter* qtWriter(vtkMRMLFileWriter* writer);

  static void onFileIOManagerEvent(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  vtkSmartPointer<vtkMRMLFileIOManager> FileIOManager;
  /// True if FileIOManager is a standalone manager (because the application logic was not available)
  bool UsingStandaloneFileIOManager{ false };
  vtkNew<vtkCallbackCommand> CallbackCommand;

  /// Qt-based readers and writers of VTK-based handlers
  QHash<vtkMRMLFileIOHandler*, QPointer<qSlicerIO>> QtIOs;
  /// Handlers that were registered in the VTK-based manager by this object (using registerIO)
  QList<vtkSmartPointer<vtkMRMLFileIOHandler>> RegisteredHandlers;

  /// Lists that are returned as references by readers() and writers()
  QList<qSlicerFileReader*> Readers;
  QList<qSlicerFileWriter*> Writers;
};

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::qSlicerCoreIOManagerPrivate(qSlicerCoreIOManager& object)
  : q_ptr(&object)
{
  this->CallbackCommand->SetClientData(this);
  this->CallbackCommand->SetCallback(qSlicerCoreIOManagerPrivate::onFileIOManagerEvent);
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::~qSlicerCoreIOManagerPrivate()
{
  if (this->FileIOManager)
  {
    this->FileIOManager->RemoveObserver(this->CallbackCommand);
  }
}

//-----------------------------------------------------------------------------
vtkMRMLFileIOManager* qSlicerCoreIOManagerPrivate::fileIOManager()
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  vtkMRMLFileIOManager* applicationFileIOManager = (app && app->applicationLogic()) ? app->applicationLogic()->GetFileIOManager() : nullptr;
  if (this->FileIOManager && (!this->UsingStandaloneFileIOManager || !applicationFileIOManager))
  {
    return this->FileIOManager;
  }

  vtkSmartPointer<vtkMRMLFileIOManager> previousFileIOManager = this->FileIOManager;
  if (previousFileIOManager)
  {
    previousFileIOManager->RemoveObserver(this->CallbackCommand);
  }

  if (applicationFileIOManager)
  {
    // Use the file IO manager of the application logic (shared by all readers and writers of the application)
    this->FileIOManager = applicationFileIOManager;
    this->UsingStandaloneFileIOManager = false;
  }
  else
  {
    qCritical() << "qSlicerCoreIOManager: application logic is not available, a temporary file IO manager is used"
                << " until the application logic is created.";
    this->FileIOManager = vtkSmartPointer<vtkMRMLFileIOManager>::New();
    this->FileIOManager->SetScene(app ? app->mrmlScene() : nullptr);
    this->UsingStandaloneFileIOManager = true;
  }
  this->FileIOManager->AddObserver(vtkMRMLFileIOManager::NewFileLoadedEvent, this->CallbackCommand);
  this->FileIOManager->AddObserver(vtkMRMLFileIOManager::FileSavedEvent, this->CallbackCommand);
  this->FileIOManager->AddObserver(vtkMRMLFileIOManager::HandlerUnregisteredEvent, this->CallbackCommand);

  if (previousFileIOManager)
  {
    // Move readers and writers that were registered in the standalone manager
    for (const vtkSmartPointer<vtkMRMLFileIOHandler>& handler : this->RegisteredHandlers)
    {
      previousFileIOManager->Unregister(handler);
      this->FileIOManager->Register(handler);
    }
  }
  return this->FileIOManager;
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManagerPrivate::onFileIOManagerEvent(vtkObject* vtkNotUsed(caller), unsigned long eid, void* clientData, void* callData)
{
  qSlicerCoreIOManagerPrivate* self = reinterpret_cast<qSlicerCoreIOManagerPrivate*>(clientData);
  qSlicerCoreIOManager* q = self->q_ptr;
  switch (eid)
  {
    case vtkMRMLFileIOManager::NewFileLoadedEvent: emit q->newFileLoaded(qSlicerIO::fromVTKProperties(reinterpret_cast<vtkMRMLIOProperties*>(callData))); break;
    case vtkMRMLFileIOManager::FileSavedEvent: emit q->fileSaved(qSlicerIO::fromVTKProperties(reinterpret_cast<vtkMRMLIOProperties*>(callData))); break;
    case vtkMRMLFileIOManager::HandlerUnregisteredEvent:
    {
      vtkMRMLFileIOHandler* handler = reinterpret_cast<vtkMRMLFileIOHandler*>(callData);
      QPointer<qSlicerIO> io = self->QtIOs.take(handler);
      // Delete generic Qt objects that were created for handlers that were not registered via registerIO
      bool registeredViaQt = false;
      for (const auto& registeredHandler : self->RegisteredHandlers)
      {
        if (registeredHandler.GetPointer() == handler)
        {
          registeredViaQt = true;
          break;
        }
      }
      if (registeredViaQt)
      {
        self->RegisteredHandlers.removeAll(vtkSmartPointer<vtkMRMLFileIOHandler>(handler));
      }
      else if (io)
      {
        io->deleteLater();
      }
      break;
    }
    default: break;
  }
}

//-----------------------------------------------------------------------------
qSlicerFileReader* qSlicerCoreIOManagerPrivate::qtReader(vtkMRMLFileReader* reader)
{
  Q_Q(qSlicerCoreIOManager);
  if (!reader)
  {
    return nullptr;
  }
  QPointer<qSlicerIO> io = this->QtIOs.value(reader);
  if (io)
  {
    return qobject_cast<qSlicerFileReader*>(io);
  }
  // Registered via another qSlicerCoreIOManager instance (all instances use the application
  // logic's file IO manager). Not stored in QtIOs, because it is not owned by this instance.
  vtkSlicerQtFileReaderAdapter* adapter = vtkSlicerQtFileReaderAdapter::SafeDownCast(reader);
  if (adapter && adapter->GetQtReader())
  {
    return adapter->GetQtReader();
  }
  qSlicerFileReader* genericReader = new qSlicerVTKFileReader(reader, q);
  this->QtIOs[reader] = genericReader;
  return genericReader;
}

//-----------------------------------------------------------------------------
qSlicerFileWriter* qSlicerCoreIOManagerPrivate::qtWriter(vtkMRMLFileWriter* writer)
{
  Q_Q(qSlicerCoreIOManager);
  if (!writer)
  {
    return nullptr;
  }
  QPointer<qSlicerIO> io = this->QtIOs.value(writer);
  if (io)
  {
    return qobject_cast<qSlicerFileWriter*>(io);
  }
  // Registered via another qSlicerCoreIOManager instance (all instances use the application
  // logic's file IO manager). Not stored in QtIOs, because it is not owned by this instance.
  vtkSlicerQtFileWriterAdapter* adapter = vtkSlicerQtFileWriterAdapter::SafeDownCast(writer);
  if (adapter && adapter->GetQtWriter())
  {
    return adapter->GetQtWriter();
  }
  qSlicerFileWriter* genericWriter = new qSlicerVTKFileWriter(writer, q);
  this->QtIOs[writer] = genericWriter;
  return genericWriter;
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::qSlicerCoreIOManager(QObject* _parent)
  : QObject(_parent)
  , d_ptr(new qSlicerCoreIOManagerPrivate(*this))
{
  // To ensure that these types are known before any qSlicerIO instance is created,
  // they are registered here. This complements the registration in the `qSlicerIO::qSlicerIO`
  // constructor.
  qRegisterMetaType<qSlicerIO::IOFileType>("qSlicerIO::IOFileType");
  qRegisterMetaType<qSlicerIO::IOProperties>("qSlicerIO::IOProperties");
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::~qSlicerCoreIOManager()
{
  Q_D(qSlicerCoreIOManager);
  // Readers and writers registered by this object are deleted with this object,
  // therefore they must be removed from the VTK-based manager.
  if (d->FileIOManager)
  {
    QList<vtkSmartPointer<vtkMRMLFileIOHandler>> registeredHandlers = d->RegisteredHandlers;
    for (const auto& handler : registeredHandlers)
    {
      d->FileIOManager->Unregister(handler);
    }
  }
}

//-----------------------------------------------------------------------------
vtkMRMLFileIOManager* qSlicerCoreIOManager::fileIOManager() const
{
  Q_D(const qSlicerCoreIOManager);
  return const_cast<qSlicerCoreIOManagerPrivate*>(d)->fileIOManager();
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager::fileType(const QString& fileName) const
{
  std::string fileType = this->fileIOManager()->GetFileTypeForFile(fileName.toStdString());
  return fileType.empty() ? QString("NoFile") : QString::fromStdString(fileType);
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager::fileTypeFromDescription(const QString& fileDescription) const
{
  std::string fileType = this->fileIOManager()->GetFileTypeFromDescription(fileDescription.toStdString());
  return fileType.empty() ? QString("NoFile") : QString::fromStdString(fileType);
}

//-----------------------------------------------------------------------------
qSlicerFileWriter* qSlicerCoreIOManager::writer(vtkObject* object, const QString& extension /*=QString()*/) const
{
  Q_D(const qSlicerCoreIOManager);
  vtkMRMLFileWriter* writer = this->fileIOManager()->GetWriterForObject(object, extension.toStdString());
  return const_cast<qSlicerCoreIOManagerPrivate*>(d)->qtWriter(writer);
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager::fileWriterFileType(vtkObject* object, const QString& format /*=QString()*/) const
{
  std::string fileType = this->fileIOManager()->GetFileWriterFileType(object, format.toStdString());
  return fileType.empty() ? QString("NoFile") : QString::fromStdString(fileType);
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOFileType> qSlicerCoreIOManager::fileTypes(const QString& fileName) const
{
  QStringList fileTypes;
  ctk::stlVectorToQList(this->fileIOManager()->GetFileTypesForFile(fileName.toStdString()), fileTypes);
  return fileTypes;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileDescriptions(const QString& fileName) const
{
  QStringList descriptions;
  ctk::stlVectorToQList(this->fileIOManager()->GetFileDescriptionsForFile(fileName.toStdString()), descriptions);
  return descriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileDescriptionsByType(const qSlicerIO::IOFileType fileType) const
{
  QStringList descriptions;
  ctk::stlVectorToQList(this->fileIOManager()->GetFileDescriptionsByType(fileType.toStdString()), descriptions);
  return descriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileWriterDescriptions(const qSlicerIO::IOFileType& fileType) const
{
  QStringList descriptions;
  ctk::stlVectorToQList(this->fileIOManager()->GetFileWriterDescriptions(fileType.toStdString()), descriptions);
  return descriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileWriterExtensions(vtkObject* object) const
{
  QStringList extensions;
  ctk::stlVectorToQList(this->fileIOManager()->GetFileWriterExtensions(object), extensions);
  return extensions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::allWritableFileExtensions() const
{
  QStringList extensions;
  ctk::stlVectorToQList(this->fileIOManager()->GetAllWritableFileExtensions(), extensions);
  return extensions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::allReadableFileExtensions() const
{
  QStringList extensions;
  ctk::stlVectorToQList(this->fileIOManager()->GetAllReadableFileExtensions(), extensions);
  return extensions;
}

//-----------------------------------------------------------------------------
QRegularExpression qSlicerCoreIOManager::fileNameRegularExpression(const QString& extension /*= QString()*/)
{
  QString pattern = QString::fromStdString(vtkMRMLFileIOManager::GetFileNameValidCharactersPattern()) + "{1,255}";
  if (!extension.isEmpty())
  {
    pattern += extension;
  }
  return QRegularExpression(pattern);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::forceFileNameValidCharacters(const QString& filename)
{
  return QString::fromStdString(vtkMRMLFileIOManager::ForceFileNameValidCharacters(filename.toStdString()));
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::forceFileNameMaxLength(const QString& filename, int extensionLength, int maxLength /*=-1*/)
{
  return QString::fromStdString(this->fileIOManager()->ForceFileNameMaxLength(filename.toStdString(), extensionLength, maxLength));
}

//-----------------------------------------------------------------------------
int qSlicerCoreIOManager::defaultMaximumFileNameLength() const
{
  return this->fileIOManager()->GetDefaultMaximumFileNameLength();
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::setDefaultMaximumFileNameLength(int length)
{
  this->fileIOManager()->SetDefaultMaximumFileNameLength(length);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::extractKnownExtension(const QString& fileName, vtkObject* object)
{
  return QString::fromStdString(this->fileIOManager()->ExtractKnownExtension(fileName.toStdString(), object));
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::stripKnownExtension(const QString& fileName, vtkObject* object)
{
  return QString::fromStdString(this->fileIOManager()->StripKnownExtension(fileName.toStdString(), object));
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::fileOptions(const QString& readerDescription) const
{
  qSlicerFileReader* reader = this->reader(readerDescription);
  if (!reader)
  {
    return nullptr;
  }
  reader->setMRMLScene(this->fileIOManager()->GetScene());
  qSlicerIOOptions* options = reader->options();
  if (!options)
  {
    options = this->createGenericOptions(reader->ioHandler());
  }
  return options;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::fileWriterOptions(vtkObject* object, const QString& extension) const
{
  qSlicerFileWriter* bestWriter = this->writer(object, extension);
  if (!bestWriter)
  {
    return nullptr;
  }
  bestWriter->setMRMLScene(this->fileIOManager()->GetScene());
  qSlicerIOOptions* options = bestWriter->options();
  if (!options)
  {
    options = this->createGenericOptions(bestWriter->ioHandler());
  }
  return options;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::createGenericOptions(vtkMRMLFileIOHandler* vtkNotUsed(ioHandler)) const
{
  return nullptr;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::completeSlicerWritableFileNameSuffix(vtkMRMLStorableNode* node) const
{
  return QString::fromStdString(this->fileIOManager()->GetCompleteSlicerWritableFileNameSuffix(node));
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadScene(const QString& fileName, bool clear, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  properties["clear"] = clear;
  return this->loadNodes(QString("SceneFile"), properties, nullptr, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadFile(const QString& fileName, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  return this->loadNodes(this->fileType(fileName), properties, nullptr, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadNodes(const qSlicerIO::IOFileType& fileType,
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkCollection* loadedNodes,
                                     vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkMRMLIOProperties> properties;
  qSlicerIO::toVTKProperties(parameters, properties);
  return this->fileIOManager()->LoadNodes(fileType.toStdString(), properties, loadedNodes, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadNodes(const QList<qSlicerIO::IOProperties>& files, vtkCollection* loadedNodes, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  // Load each file by calling loadNodes(fileType, ...), which may be overridden in subclasses
  bool success = true;
  for (const qSlicerIO::IOProperties& fileProperties : files)
  {
    int numberOfUserMessagesBefore = userMessages ? userMessages->GetNumberOfMessages() : 0;
    success = this->loadNodes(static_cast<qSlicerIO::IOFileType>(fileProperties["fileType"].toString()), fileProperties, loadedNodes, userMessages) //
              && success;
    // Add a separator between nodes
    if (userMessages && userMessages->GetNumberOfMessages() > numberOfUserMessagesBefore)
    {
      userMessages->AddSeparator();
    }
  }
  return success;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerCoreIOManager::loadNodesAndGetFirst(qSlicerIO::IOFileType fileType,
                                                        const qSlicerIO::IOProperties& parameters,
                                                        vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkCollection> loadedNodes;
  this->loadNodes(fileType, parameters, loadedNodes, userMessages);
  return vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(0));
}

//-----------------------------------------------------------------------------
vtkMRMLStorageNode* qSlicerCoreIOManager::createAndAddDefaultStorageNode(vtkMRMLStorableNode* node)
{
  return vtkMRMLFileIOManager::CreateAndAddDefaultStorageNode(node);
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::emitNewFileLoaded(const QVariantMap& loadedFileParameters)
{
  vtkNew<vtkMRMLIOProperties> properties;
  qSlicerIO::toVTKProperties(loadedFileParameters, properties);
  this->fileIOManager()->InvokeNewFileLoadedEvent(properties);
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::emitFileSaved(const QVariantMap& savedFileParameters)
{
  vtkNew<vtkMRMLIOProperties> properties;
  qSlicerIO::toVTKProperties(savedFileParameters, properties);
  this->fileIOManager()->InvokeFileSavedEvent(properties);
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::addDefaultStorageNodes()
{
  this->fileIOManager()->AddDefaultStorageNodes();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::saveNodes(qSlicerIO::IOFileType fileType,
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkMRMLMessageCollection* userMessages /*=nullptr*/,
                                     vtkMRMLScene* scene /*=nullptr*/)
{
  vtkNew<vtkMRMLIOProperties> properties;
  qSlicerIO::toVTKProperties(parameters, properties);
  return this->fileIOManager()->SaveNodes(fileType.toStdString(), properties, userMessages, scene);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::exportNodes(const QStringList& nodeIDs,
                                       const QStringList& fileNames,
                                       const qSlicerIO::IOProperties& commonParameterMap,
                                       bool hardenTransforms,
                                       vtkMRMLMessageCollection* userMessages /*=nullptr*/
)
{
  vtkNew<vtkMRMLIOProperties> commonProperties;
  qSlicerIO::toVTKProperties(commonParameterMap, commonProperties);
  std::vector<std::string> nodeIDsVector;
  ctk::qListToSTLVector(nodeIDs, nodeIDsVector);
  std::vector<std::string> fileNamesVector;
  ctk::qListToSTLVector(fileNames, fileNamesVector);
  return this->fileIOManager()->ExportNodes(nodeIDsVector, fileNamesVector, commonProperties, hardenTransforms, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::exportNodes(const QList<qSlicerIO::IOProperties>& parameterMaps, bool hardenTransforms, vtkMRMLMessageCollection* userMessages /*=nullptr*/
)
{
  vtkNew<vtkCollection> propertiesList;
  for (const qSlicerIO::IOProperties& parameters : parameterMaps)
  {
    vtkNew<vtkMRMLIOProperties> properties;
    qSlicerIO::toVTKProperties(parameters, properties);
    propertiesList->AddItem(properties);
  }
  return this->fileIOManager()->ExportNodes(propertiesList, hardenTransforms, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::saveScene(const QString& fileName, QImage screenShot, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  properties["screenShot"] = screenShot;
  return this->saveNodes(QString("SceneFile"), properties, userMessages);
}

//-----------------------------------------------------------------------------
QList<qSlicerFileReader*> qSlicerCoreIOManager::readers() const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerCoreIOManagerPrivate* dNonConst = const_cast<qSlicerCoreIOManagerPrivate*>(d);
  vtkMRMLFileIOManager* fileIOManager = this->fileIOManager();
  dNonConst->Readers.clear();
  for (int i = 0; i < fileIOManager->GetNumberOfReaders(); ++i)
  {
    dNonConst->Readers << dNonConst->qtReader(fileIOManager->GetNthReader(i));
  }
  return d->Readers;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileWriter*> qSlicerCoreIOManager::writers() const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerCoreIOManagerPrivate* dNonConst = const_cast<qSlicerCoreIOManagerPrivate*>(d);
  vtkMRMLFileIOManager* fileIOManager = this->fileIOManager();
  dNonConst->Writers.clear();
  for (int i = 0; i < fileIOManager->GetNumberOfWriters(); ++i)
  {
    dNonConst->Writers << dNonConst->qtWriter(fileIOManager->GetNthWriter(i));
  }
  return d->Writers;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileReader*> qSlicerCoreIOManager::readers(const qSlicerIO::IOFileType& fileType) const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerFileReader*> res;
  for (vtkMRMLFileReader* reader : this->fileIOManager()->GetReadersForFileType(fileType.toStdString()))
  {
    res << const_cast<qSlicerCoreIOManagerPrivate*>(d)->qtReader(reader);
  }
  return res;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileWriter*> qSlicerCoreIOManager::writers(const qSlicerIO::IOFileType& fileType) const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerFileWriter*> res;
  for (vtkMRMLFileWriter* writer : this->fileIOManager()->GetWritersForFileType(fileType.toStdString()))
  {
    res << const_cast<qSlicerCoreIOManagerPrivate*>(d)->qtWriter(writer);
  }
  return res;
}

//-----------------------------------------------------------------------------
qSlicerFileReader* qSlicerCoreIOManager::reader(const QString& ioDescription) const
{
  Q_D(const qSlicerCoreIOManager);
  vtkMRMLFileReader* reader = this->fileIOManager()->GetReaderByDescription(ioDescription.toStdString());
  return const_cast<qSlicerCoreIOManagerPrivate*>(d)->qtReader(reader);
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::registerIO(qSlicerIO* io)
{
  Q_D(qSlicerCoreIOManager);
  if (!io)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid reader or writer";
    return;
  }
  // Reparent - this will make sure the object is destroyed properly
  io->setParent(this);

  qSlicerFileReader* fileReader = qobject_cast<qSlicerFileReader*>(io);
  qSlicerFileWriter* fileWriter = qobject_cast<qSlicerFileWriter*>(io);
  // An adapter is registered (even if the Qt-based reader or writer delegates all tasks to a
  // VTK-based reader or writer), so that the Qt-based object can be retrieved from the handler.
  vtkSmartPointer<vtkMRMLFileIOHandler> handler;
  if (fileWriter)
  {
    vtkNew<vtkSlicerQtFileWriterAdapter> adapter;
    adapter->SetQtWriter(fileWriter);
    handler = adapter.GetPointer();
  }
  else if (fileReader)
  {
    vtkNew<vtkSlicerQtFileReaderAdapter> adapter;
    adapter->SetQtReader(fileReader);
    handler = adapter.GetPointer();
  }
  if (!handler)
  {
    // Neither a reader nor a writer, nothing to register
    return;
  }

  d->QtIOs[handler] = io;
  d->RegisteredHandlers << handler;
  this->fileIOManager()->Register(handler);

  // If the reader or writer is deleted then remove it from the VTK-based manager
  // (the manager that is used at the time the reader or writer is deleted, as the standalone manager
  // may be replaced by the application logic's manager)
  QObject::connect(io,
                   &QObject::destroyed,
                   this,
                   [this, handler]()
                   {
                     Q_D(qSlicerCoreIOManager);
                     if (d->FileIOManager)
                     {
                       d->FileIOManager->Unregister(handler);
                     }
                   });
}

//-----------------------------------------------------------------------------
int qSlicerCoreIOManager::registeredFileReaderCount(const qSlicerIO::IOFileType& fileType) const
{
  return static_cast<int>(this->fileIOManager()->GetReadersForFileType(fileType.toStdString()).size());
}

//-----------------------------------------------------------------------------
int qSlicerCoreIOManager::registeredFileWriterCount(const qSlicerIO::IOFileType& fileType) const
{
  return static_cast<int>(this->fileIOManager()->GetWritersForFileType(fileType.toStdString()).size());
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::defaultSceneFileType() const
{
  return QString::fromStdString(this->fileIOManager()->GetDefaultSceneFileType());
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::setDefaultSceneFileType(QString fileType)
{
  this->fileIOManager()->SetDefaultSceneFileType(fileType.toStdString());
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::examineFileInfoList(QFileInfoList& fileInfoList, QFileInfo& archetypeFileInfo, QString& readerDescription, qSlicerIO::IOProperties& ioProperties) const
{
  std::vector<std::string> fileList;
  for (const QFileInfo& fileInfo : fileInfoList)
  {
    fileList.push_back(fileInfo.absoluteFilePath().toStdString());
  }
  vtkNew<vtkMRMLIOProperties> properties;
  qSlicerIO::toVTKProperties(ioProperties, properties);
  const bool hadFileName = ioProperties.contains("fileName");
  vtkMRMLFileReader* reader = this->fileIOManager()->ExamineFileList(fileList, properties);
  if (!reader)
  {
    return false;
  }
  archetypeFileInfo = QFileInfo(QString::fromStdString(properties->GetStringProperty("fileName")));
  if (!hadFileName)
  {
    // The archetype is returned in archetypeFileInfo
    properties->RemoveProperty("fileName");
  }
  // Remove files from the list that the reader removed
  QStringList remainingFiles;
  ctk::stlVectorToQList(fileList, remainingFiles);
  QMutableListIterator<QFileInfo> fileInfoIterator(fileInfoList);
  while (fileInfoIterator.hasNext())
  {
    if (!remainingFiles.contains(fileInfoIterator.next().absoluteFilePath()))
    {
      fileInfoIterator.remove();
    }
  }
  readerDescription = QString::fromStdString(reader->GetDescription());
  ioProperties = qSlicerIO::fromVTKProperties(properties);
  return true;
}
