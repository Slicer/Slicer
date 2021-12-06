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
#include <QElapsedTimer>
#include <QFileInfo>

// CTK includes
#include <ctkUtils.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileReader.h"
#include "qSlicerFileWriter.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkDataFileFormatHelper.h> // for GetFileExtensionFromFormatString()
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkGeneralTransform.h>

//-----------------------------------------------------------------------------
class qSlicerCoreIOManagerPrivate
{
public:
  qSlicerCoreIOManagerPrivate();
  ~qSlicerCoreIOManagerPrivate();
  vtkMRMLScene* currentScene()const;

  qSlicerFileReader* reader(const QString& fileName)const;
  QList<qSlicerFileReader*> readers(const QString& fileName)const;

  QList<qSlicerFileWriter*> writers(const qSlicerIO::IOFileType &fileType, const qSlicerIO::IOProperties& parameters)const;

  QSettings*        ExtensionFileType;
  QList<qSlicerFileReader*> Readers;
  QList<qSlicerFileWriter*> Writers;
  QMap<qSlicerIO::IOFileType, QStringList> FileTypes;

  QString DefaultSceneFileType;
};

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::qSlicerCoreIOManagerPrivate() = default;

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::~qSlicerCoreIOManagerPrivate() = default;

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerCoreIOManagerPrivate::currentScene()const
{
  return qSlicerCoreApplication::application()->mrmlScene();
}

//-----------------------------------------------------------------------------
qSlicerFileReader* qSlicerCoreIOManagerPrivate::reader(const QString& fileName)const
{
  QList<qSlicerFileReader*> matchingReaders = this->readers(fileName);
  return matchingReaders.count() ? matchingReaders[0] : 0;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileReader*> qSlicerCoreIOManagerPrivate::readers(const QString& fileName)const
{
  // Use a map so that we can access readers sorted by confidence.
  // The more specific the filter that was matched, the higher confidence
  // that the reader is more appropriate (e.g., *.seg.nrrd is more specific than *.nrrd;
  // *.nrrd is more specific than *.*)
  QMultiMap<int, qSlicerFileReader*> matchingReadersSortedByConfidence;
  foreach(qSlicerFileReader* reader, this->Readers)
    {
    // reader->supportedNameFilters will return the length of the longest matched file extension
    // in longestExtensionMatch variable.
    int longestExtensionMatch = 0;
    QStringList matchedNameFilters = reader->supportedNameFilters(fileName, &longestExtensionMatch);
    if (!matchedNameFilters.empty() && reader->canLoadFile(fileName))
      {
      matchingReadersSortedByConfidence.insert(longestExtensionMatch, reader);
      }
    }
  // Put matching readers in a list, with highest confidence readers pushed to the front
  QList<qSlicerFileReader*> matchingReaders;
  QMapIterator<int, qSlicerFileReader*> i(matchingReadersSortedByConfidence);
  while (i.hasNext())
    {
    i.next();
    matchingReaders.push_front(i.value());
    }
  return matchingReaders;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileWriter*> qSlicerCoreIOManagerPrivate::writers(
    const qSlicerIO::IOFileType& fileType, const qSlicerIO::IOProperties& parameters)const
{
  QString fileName = parameters.value("fileName").toString();
  QString nodeID = parameters.value("nodeID").toString();

  vtkObject * object = this->currentScene()->GetNodeByID(nodeID.toUtf8());
  QFileInfo file(fileName);

  QList<qSlicerFileWriter*> matchingWriters;
  // Some writers ("Slicer Data Bundle (*)" can support any file,
  // they are called generic writers. The following code ensures
  // that writers associated with specific file extension are
  // considered first.
  QList<qSlicerFileWriter*> genericWriters;
  foreach(qSlicerFileWriter* writer, this->Writers)
    {
    if (writer->fileType() != fileType)
      {
      continue;
      }
    QStringList matchingNameFilters;
    foreach(const QString& nameFilter, writer->extensions(object))
      {
      foreach(const QString& extension, ctk::nameFilterToExtensions(nameFilter))
        {
        // HACK - See https://github.com/Slicer/Slicer/issues/3322
        QString extensionWithStar(extension);
        if (!extensionWithStar.startsWith("*"))
          {
          extensionWithStar.prepend("*");
          }
        QRegExp regExp(extensionWithStar, Qt::CaseInsensitive, QRegExp::Wildcard);
        Q_ASSERT(regExp.isValid());
        if (regExp.exactMatch(file.absoluteFilePath()))
          {
          matchingNameFilters << nameFilter;
          }
        }
      }
    if (matchingNameFilters.count() == 0)
      {
      continue;
      }
    // Generic readers must be added to the end
    foreach(const QString& nameFilter, matchingNameFilters)
      {
      if (nameFilter.contains( "*.*" ) || nameFilter.contains("(*)"))
        {
        genericWriters << writer;
        continue;
        }
      if (!matchingWriters.contains(writer))
        {
        matchingWriters << writer;
        }
      }
    }
  foreach(qSlicerFileWriter* writer, genericWriters)
    {
    if (!matchingWriters.contains(writer))
      {
      matchingWriters << writer;
      }
    }
  return matchingWriters;
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::qSlicerCoreIOManager(QObject* _parent)
  :QObject(_parent)
  , d_ptr(new qSlicerCoreIOManagerPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::~qSlicerCoreIOManager() = default;

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager::fileType(const QString& fileName)const
{
  QList<qSlicerIO::IOFileType> matchingFileTypes = this->fileTypes(fileName);
  return matchingFileTypes.count() ? matchingFileTypes[0] : QString("NoFile");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager
::fileTypeFromDescription(const QString& fileDescription)const
{
  qSlicerFileReader* reader = this->reader(fileDescription);
  return reader ? reader->fileType() : QString("NoFile");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager
::fileWriterFileType(vtkObject* object, const QString& format/*=QString()*/)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerIO::IOFileType> matchingFileTypes;
  // closest match is the writer that supports the node type but not
  // that specific extension
  QString closestMatch = QString("NoFile");
  foreach (const qSlicerFileWriter* writer, d->Writers)
    {
    if (writer->canWriteObject(object))
      {
      closestMatch = writer->fileType();
      if (format.isEmpty() || writer->extensions(object).contains(format))
        {
        return writer->fileType();
        }
      }
    }
  return closestMatch;
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOFileType> qSlicerCoreIOManager::fileTypes(const QString& fileName)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerIO::IOFileType> matchingFileTypes;
  foreach (const qSlicerIO* matchingReader, d->readers(fileName))
    {
    matchingFileTypes << matchingReader->fileType();
    }
  return matchingFileTypes;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileDescriptions(const QString& fileName)const
{
  Q_D(const qSlicerCoreIOManager);
  QStringList matchingDescriptions;
  foreach(qSlicerFileReader* reader, d->readers(fileName))
    {
    matchingDescriptions << reader->description();
    }
  return matchingDescriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::
fileDescriptionsByType(const qSlicerIO::IOFileType fileType)const
{
  QStringList matchingDescriptions;
  foreach(qSlicerFileReader* reader, this->readers())
    {
    if (reader->fileType() == fileType)
      {
      matchingDescriptions << reader->description();
      }
    }
  return matchingDescriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileWriterDescriptions(
  const qSlicerIO::IOFileType& fileType)const
{
  QStringList matchingDescriptions;
  foreach(qSlicerFileWriter* writer, this->writers(fileType))
    {
    matchingDescriptions << writer->description();
    }
  return matchingDescriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileWriterExtensions(
  vtkObject* object)const
{
  Q_D(const qSlicerCoreIOManager);
  QStringList matchingExtensions;
  foreach(qSlicerFileWriter* writer, d->Writers)
    {
    if (writer->canWriteObject(object))
      {
      matchingExtensions << writer->extensions(object);
      }
    }
  matchingExtensions.removeDuplicates();
  return matchingExtensions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::allWritableFileExtensions()const
{
  Q_D(const qSlicerCoreIOManager);

  QStringList extensions;

  if (!d->currentScene())
    {
    qWarning() << "allWritableFileExtensions: manager has no scene defined";
    return extensions;
    }
  // check for all extensions that can be used to write storable nodes
  int numRegisteredNodeClasses = d->currentScene()->GetNumberOfRegisteredNodeClasses();
  for (int i = 0; i < numRegisteredNodeClasses; ++i)
    {
    vtkMRMLNode *mrmlNode = d->currentScene()->GetNthRegisteredNodeClass(i);
    if (mrmlNode && mrmlNode->IsA("vtkMRMLStorageNode"))
      {
      vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(mrmlNode);
      if (snode)
        {
        vtkNew<vtkStringArray> supportedFileExtensions;
        snode->GetFileExtensionsFromFileTypes(snode->GetSupportedWriteFileTypes(), supportedFileExtensions.GetPointer());
        const int formatCount = supportedFileExtensions->GetNumberOfValues();
        for (int formatIt = 0; formatIt < formatCount; ++formatIt)
          {
          QString extension = QString::fromStdString(supportedFileExtensions->GetValue(formatIt));
          extensions << extension;
          }
        }
      }
    }
  extensions.removeDuplicates();
  return extensions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::allReadableFileExtensions()const
{
  Q_D(const qSlicerCoreIOManager);

  QStringList extensions;

  if (!d->currentScene())
    {
    qWarning() << "allReadableFileExtensions: manager has no scene defined";
    return extensions;
    }
  // check for all extensions that can be used to read storable nodes
  int numRegisteredNodeClasses = d->currentScene()->GetNumberOfRegisteredNodeClasses();
  for (int i = 0; i < numRegisteredNodeClasses; ++i)
    {
    vtkMRMLNode *mrmlNode = d->currentScene()->GetNthRegisteredNodeClass(i);
    if (mrmlNode && mrmlNode->IsA("vtkMRMLStorageNode"))
      {
      vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(mrmlNode);
      if (snode)
        {
        vtkNew<vtkStringArray> supportedFileExtensions;
        snode->GetFileExtensionsFromFileTypes(snode->GetSupportedReadFileTypes(), supportedFileExtensions.GetPointer());
        const int formatCount = supportedFileExtensions->GetNumberOfValues();
        for (int formatIt = 0; formatIt < formatCount; ++formatIt)
          {
          QString extension = QString::fromStdString(supportedFileExtensions->GetValue(formatIt));
          extensions << extension;
          }
        }
      }
    }
  extensions.removeDuplicates();
  return extensions;
}

//-----------------------------------------------------------------------------
QRegExp qSlicerCoreIOManager::fileNameRegExp(const QString& extension /*= QString()*/)
{
  QRegExp regExp("[A-Za-z0-9\\ \\-\\_\\.\\(\\)\\$\\!\\~\\#\\'\\%\\^\\{\\}]{1,255}");

  if (!extension.isEmpty())
    {
    regExp.setPattern(regExp.pattern() + extension);
    }
  return regExp;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::forceFileNameValidCharacters(const QString& filename)
{
  // Remove characters that are likely to cause problems in filename
  QString sanitizedFilename;
  QRegExp regExp = fileNameRegExp();

  for (int i = 0; i < filename.size(); ++i)
    {
    if (regExp.exactMatch(QString(filename[i])))
      {
      sanitizedFilename += filename[i];
      }
    }

  // Remove leading and trailing spaces
  sanitizedFilename = sanitizedFilename.trimmed();

  return sanitizedFilename;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::extractKnownExtension(const QString& fileName, vtkObject* object)
{
  foreach(const QString& nameFilter, this->fileWriterExtensions(object))
    {
    QString extension = QString::fromStdString(
      vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.toUtf8()));
    if (!extension.isEmpty() && fileName.endsWith(extension))
      {
      return extension;
      }
    }
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::stripKnownExtension(const QString& fileName, vtkObject* object)
{
  QString strippedFileName(fileName);

  QString knownExtension = extractKnownExtension(fileName, object);
  if (!knownExtension.isEmpty())
    {
    strippedFileName.chop(knownExtension.length());

    // recursively chop any further copies of the extension,
    // which sometimes appear when the filename+extension is
    // constructed from a filename that already had an extension
    if (strippedFileName.endsWith(knownExtension))
      {
      return stripKnownExtension(strippedFileName, object);
      }
    }
  return strippedFileName;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::fileOptions(const QString& readerDescription)const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerFileReader* reader = this->reader(readerDescription);
  if (!reader)
    {
    return nullptr;
    }
  reader->setMRMLScene(d->currentScene());
  return reader->options();
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::fileWriterOptions(
  vtkObject* object, const QString& extension)const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerFileWriter* bestWriter = nullptr;
  foreach(qSlicerFileWriter* writer, d->Writers)
    {
    if (writer->canWriteObject(object))
      {
      if (writer->extensions(object).contains(extension))
        {
        writer->setMRMLScene(d->currentScene());
        bestWriter = writer;
        }
      }
    }
  return bestWriter ? bestWriter->options() : nullptr;
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::completeSlicerWritableFileNameSuffix(vtkMRMLStorableNode *node)const
{
  vtkMRMLStorageNode* storageNode = node->GetStorageNode();
  if (!storageNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: no storage node is available";
    return QString(".");
    }
  QString ext = QString::fromStdString(storageNode->GetSupportedFileExtension(nullptr, false, true));
  if (!ext.isEmpty())
    {
    // found
    return ext;
    }
  // otherwise return an empty suffix
  return QString(".");
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadScene(const QString& fileName, bool clear, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  properties["clear"] = clear;
  return this->loadNodes(QString("SceneFile"), properties, nullptr, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadFile(const QString& fileName, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  return this->loadNodes(this->fileType(fileName), properties, nullptr, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadNodes(const qSlicerIO::IOFileType& fileType,
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkCollection* loadedNodes,
                                     vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  Q_D(qSlicerCoreIOManager);

  Q_ASSERT(parameters.contains("fileName"));
  if (parameters["fileName"].type() == QVariant::StringList)
    {
    bool res = true;
    QStringList fileNames = parameters["fileName"].toStringList();
    QStringList names = parameters["name"].toStringList();
    int nameId = 0;
    foreach(const QString& fileName, fileNames)
      {
      qSlicerIO::IOProperties fileParameters = parameters;
      fileParameters["fileName"] = fileName;
      if (!names.isEmpty())
        {
        fileParameters["name"] = nameId < names.size() ? names[nameId] : names.last();
        ++nameId;
        }
      res &= this->loadNodes(fileType, fileParameters, loadedNodes, userMessages);
      }
    return res;
    }
  Q_ASSERT(!parameters["fileName"].toString().isEmpty());

  qSlicerIO::IOProperties loadedFileParameters = parameters;
  loadedFileParameters.insert("fileType", fileType);

  const QList<qSlicerFileReader*>& readers = this->readers(fileType);

  // If no readers were able to read and load the file(s), success will remain false
  bool success = false;
  int numberOfUserMessagesBefore = userMessages ? userMessages->GetNumberOfMessages() : 0;
  QString userMessagePrefix = QString("Loading %1 - ").arg(parameters["fileName"].toString());

  QStringList nodes;
  foreach (qSlicerFileReader* reader, readers)
    {
    QElapsedTimer timeProbe;
    timeProbe.start();
    reader->userMessages()->ClearMessages();
    reader->setMRMLScene(d->currentScene());
    if (!reader->canLoadFile(parameters["fileName"].toString()))
      {
      continue;
      }
    bool currentFileSuccess = reader->load(parameters);
    if (userMessages)
      {
      userMessages->AddMessages(reader->userMessages(), userMessagePrefix.toStdString());
      }
    if (!currentFileSuccess)
      {
      continue;
      }
    float elapsedTimeInSeconds = timeProbe.elapsed() / 1000.0;
    qDebug() << reader->description() << "Reader has successfully read the file"
             << parameters["fileName"].toString()
             << QString("[%1s]").arg(
                  QString::number(elapsedTimeInSeconds,'f', 2));
    nodes << reader->loadedNodes();
    success = true;
    break;
    }

  if (!success && userMessages != nullptr && userMessages->GetNumberOfMessages() == numberOfUserMessagesBefore)
    {
    // Make sure that at least one message is logged if reading failed.
    userMessages->AddMessage(vtkCommand::ErrorEvent, (QString(tr("%1 load failed.")).arg(userMessagePrefix)).toStdString());
    }

  loadedFileParameters.insert("nodeIDs", nodes);

  emit newFileLoaded(loadedFileParameters);

  if (loadedNodes)
    {
    foreach(const QString& node, nodes)
      {
      vtkMRMLNode* loadedNode = d->currentScene()->GetNodeByID(node.toUtf8());
      if (!loadedNode)
        {
        qWarning() << Q_FUNC_INFO << " error: cannot find node by ID " << node;
        continue;
        }
      loadedNodes->AddItem(loadedNode);
      }
    }

  return success;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadNodes(const QList<qSlicerIO::IOProperties>& files,
          vtkCollection* loadedNodes, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  bool success = true;
  foreach(qSlicerIO::IOProperties fileProperties, files)
    {
    int numberOfUserMessagesBefore = userMessages ? userMessages->GetNumberOfMessages() : 0;
    success = this->loadNodes(
      static_cast<qSlicerIO::IOFileType>(fileProperties["fileType"].toString()),
      fileProperties, loadedNodes, userMessages)
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
  const qSlicerIO::IOProperties& parameters, vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  vtkNew<vtkCollection> loadedNodes;
  this->loadNodes(fileType, parameters, loadedNodes.GetPointer(), userMessages);

  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(0));
  Q_ASSERT(node);

  return node;
}

//-----------------------------------------------------------------------------
vtkMRMLStorageNode* qSlicerCoreIOManager::createAndAddDefaultStorageNode(
    vtkMRMLStorableNode* node)
{
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid input node";
    return nullptr;
    }
  if (!node->AddDefaultStorageNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: error while adding default storage node";
    return nullptr;
    }
  return node->GetStorageNode();
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::emitNewFileLoaded(const QVariantMap& loadedFileParameters)
{
  emit this->newFileLoaded(loadedFileParameters);
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::addDefaultStorageNodes()
{
  Q_D(qSlicerCoreIOManager);
  int numNodes = d->currentScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(d->currentScene()->GetNthNode(i));
    if (!storableNode)
      {
      continue;
      }
    if (!storableNode->GetSaveWithScene())
      {
      continue;
      }
    vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
    if (storageNode)
      {
      // this node already has a storage node
      continue;
      }
    storableNode->AddDefaultStorageNode();
    storageNode = storableNode->GetStorageNode();
    if (!storageNode)
      {
      // no need for storage node to store this node
      // (some nodes can be saved either into the scene or into a separate file)
      continue;
      }
    std::string fileName(storageNode->GetFileName() ? storageNode->GetFileName() : "");
    if (!fileName.empty())
      {
      // filename is already set
      continue;
      }
    if (!storableNode->GetName())
      {
      // no node name is specified, cannot create a default file name
      continue;
      }
    // Default storage node usually has empty file name (if Save dialog is not opened yet)
    // file name is encoded to handle : or / characters in the node names
    std::string fileBaseName = vtkMRMLApplicationLogic::PercentEncode(storableNode->GetName());
    std::string extension = storageNode->GetDefaultWriteFileExtension();
    std::string storageFileName = fileBaseName + std::string(".") + extension;
    }
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::saveNodes(qSlicerIO::IOFileType fileType,
  const qSlicerIO::IOProperties& parameters,
  vtkMRMLMessageCollection* userMessages/*=nullptr*/,
  vtkMRMLScene* scene/*=nullptr*/)
{
  Q_D(qSlicerCoreIOManager);

  if (!scene)
    {
    scene = d->currentScene();
    }

  Q_ASSERT(parameters.contains("fileName"));

  // HACK - See https://github.com/Slicer/Slicer/issues/3322
  //        Sort writers to ensure generic ones are last.
  const QList<qSlicerFileWriter*> writers = d->writers(fileType, parameters);
  if (writers.isEmpty())
    {
    qWarning() << "No writer found to write file" << parameters.value("fileName")
               << "of type" << fileType;
    return false;
    }

  QStringList nodes;
  bool writeSuccess=false;
  foreach (qSlicerFileWriter* writer, writers)
    {
    writer->setMRMLScene(scene);
    writer->userMessages()->ClearMessages();
    bool currentWriterSuccess = writer->write(parameters);
    if (userMessages)
      {
      userMessages->AddMessages(writer->userMessages());
      }
    if (!currentWriterSuccess)
      {
      continue;
      }
    nodes << writer->writtenNodes();
    writeSuccess = true;
    break;
    }

  if (!writeSuccess)
    {
    // no appropriate writer was found
    return false;
    }

  if (nodes.count() == 0 &&
      fileType != QString("SceneFile"))
    {
    // the writer did not report error
    // but did not report any successfully written nodes either
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::exportNodes(
  const QList<QString>& nodeIDs,
  const QList<QString>& filePaths,
  const qSlicerIO::IOProperties& parameters,
  vtkMRMLMessageCollection* userMessages/*=nullptr*/
)
{
  Q_D(qSlicerCoreIOManager);

  // Validate parameters
  if (!parameters.contains("fileFormat"))
    {
    qCritical() << Q_FUNC_INFO << " failed: fileFormat must be included as a parameter to exportNodes";
    return false;
    }
  if (nodeIDs.size() != filePaths.size())
    {
    qCritical() << Q_FUNC_INFO << " failed: nodeIDs and filePaths must have the same size.";
    return false;
    }

  // Create a temporary scene to use for exporting only and to be destroyed when done exporting
  vtkNew<vtkMRMLScene> temporaryScene;
  d->currentScene()->CopyDefaultNodesToScene(temporaryScene);
  d->currentScene()->CopyRegisteredNodesToScene(temporaryScene);
  d->currentScene()->CopySingletonNodesToScene(temporaryScene);
  temporaryScene->SetDataIOManager(d->currentScene()->GetDataIOManager());

  // Copy parameters map; we will need to set the nodeID parameter to correspond to nodes in the temporary scene
  qSlicerIO::IOProperties temporarySceneParameters = parameters;

  bool success = true;
  for (int i = 0; i<nodeIDs.size(); ++i)
    {

    // Copy over each node to be exported
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(d->currentScene()->GetNodeByID(nodeIDs[i].toUtf8()));
    if (!storableNode)
      {
      if (userMessages)
        {
        userMessages->AddMessage(vtkCommand::ErrorEvent,
          (tr("Unable to find a storable node with ID %1").arg(nodeIDs[i])).toStdString()
        );
        }
      success = false;
      continue;
      }
    vtkMRMLStorableNode* temporaryStorableNode = vtkMRMLStorableNode::SafeDownCast(temporaryScene->AddNewNodeByClass(storableNode->GetClassName()));
    if (!temporaryStorableNode)
      {
      qCritical() << Q_FUNC_INFO << "Unable to add node to temporary scene";
      if (userMessages)
        {
        userMessages->AddMessage(vtkCommand::ErrorEvent,
          (tr("Error encountered while exporting %1").arg(storableNode->GetName())).toStdString()
        );
        }
      success = false;
      continue;
      }
    // We will do a shallow copy, unless tranform hardening was requested. Transform hardening
    // can sometimes affect the underlying data of the transformable node, so it's worth doing
    // a deep copy to be certain that the original node is not modified during export.
    bool hardenTransforms = parameters.contains("hardenTransforms") && parameters["hardenTransforms"].toBool();
    temporaryStorableNode->CopyContent(storableNode, /*deepCopy=*/hardenTransforms);


    // Put transforms in the temporaryScene, if transform hardening was requested. Then apply hardening
    if (hardenTransforms)
      {

      vtkMRMLTransformableNode* nodeAsTransformable = vtkMRMLTransformableNode::SafeDownCast(storableNode);
      vtkMRMLTransformableNode* temporaryNodeAsTransformable = vtkMRMLTransformableNode::SafeDownCast(temporaryStorableNode);

      if (!nodeAsTransformable)
        {
        continue;
        }
      if (!temporaryNodeAsTransformable)
        {
        qCritical() << Q_FUNC_INFO << " failed: Node is tranformable but its copy is not... this should never happen.";
        return false;
        }

      vtkMRMLTransformNode* parentTransform = nodeAsTransformable->GetParentTransformNode();
      if (parentTransform)
        {
        vtkSmartPointer<vtkGeneralTransform> generalTransform =  vtkSmartPointer<vtkGeneralTransform>::New();
        parentTransform->GetTransformFromWorld(generalTransform);
        vtkMRMLTransformNode* compositeTransformNode =
          vtkMRMLTransformNode::SafeDownCast(temporaryScene->AddNewNodeByClass("vtkMRMLTransformNode"));
        if (!compositeTransformNode)
          {
          qCritical() << Q_FUNC_INFO << "Unable to add a transform node to temporary scene";
          if (userMessages)
            {
            userMessages->AddMessage(vtkCommand::ErrorEvent,
              (tr("Error encountered while exporting %1").arg(storableNode->GetName())).toStdString()
            );
            }
          success = false;
          continue;
          }
        compositeTransformNode->SetAndObserveTransformFromParent(generalTransform);
        temporaryNodeAsTransformable->SetAndObserveTransformNodeID(compositeTransformNode->GetID());
        }

      temporaryNodeAsTransformable->HardenTransform();
      }


    // Prepare temporary scene node ID and prepare filename for saving
    temporarySceneParameters["nodeID"] = temporaryStorableNode->GetID();
    temporarySceneParameters["fileName"] = filePaths[i];

    // Deduce "fileType" from "fileFormat" parameter; saveNodes will want both
    qSlicerIO::IOFileType fileType = this->fileWriterFileType(storableNode, parameters.value("fileFormat").toString());

    // Finally, applying saving logic to the the temporary scene
    if ( ! this->saveNodes(fileType, temporarySceneParameters, userMessages, temporaryScene) )
      {
      if (userMessages)
        {
        userMessages->AddMessage(vtkCommand::ErrorEvent,
          (tr("Error encountered while exporting %1").arg(storableNode->GetName())).toStdString()
        );
        }
      success = false;
      }

      // Pick up any user messages that were saved to temporaryStorableNode's storage node
      if (temporaryStorableNode->GetStorageNode()
          && temporaryStorableNode->GetStorageNode()->GetUserMessages())
        {
        userMessages->AddMessages(temporaryStorableNode->GetStorageNode()->GetUserMessages());
        }

    }
  return success;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::saveScene(const QString& fileName, QImage screenShot,
  vtkMRMLMessageCollection* userMessages/*=nullptr*/)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  properties["screenShot"] = screenShot;

  return this->saveNodes(QString("SceneFile"), properties, userMessages);
}

//-----------------------------------------------------------------------------
const QList<qSlicerFileReader*>& qSlicerCoreIOManager::readers()const
{
  Q_D(const qSlicerCoreIOManager);
  return d->Readers;
}

//-----------------------------------------------------------------------------
const QList<qSlicerFileWriter*>& qSlicerCoreIOManager::writers()const
{
  Q_D(const qSlicerCoreIOManager);
  return d->Writers;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileReader*> qSlicerCoreIOManager::readers(const qSlicerIO::IOFileType& fileType)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerFileReader*> res;
  foreach(qSlicerFileReader* io, d->Readers)
    {
    if (io->fileType() == fileType)
      {
      res << io;
      }
    }
  return res;
}

//-----------------------------------------------------------------------------
QList<qSlicerFileWriter*> qSlicerCoreIOManager::writers(const qSlicerIO::IOFileType& fileType)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerFileWriter*> res;
  foreach(qSlicerFileWriter* io, d->Writers)
    {
    if (io->fileType() == fileType)
      {
      res << io;
      }
    }
  return res;
}

//-----------------------------------------------------------------------------
qSlicerFileReader* qSlicerCoreIOManager::reader(const QString& ioDescription)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerFileReader*> res;
  foreach(qSlicerFileReader* io, d->Readers)
    {
    if (io->description() == ioDescription)
      {
      res << io;
      }
    }
  Q_ASSERT(res.count() < 2);
  return res.count() ? res[0] : 0;
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::registerIO(qSlicerIO* io)
{
  Q_ASSERT(io);
  Q_D(qSlicerCoreIOManager);
  qSlicerFileReader* fileReader = qobject_cast<qSlicerFileReader*>(io);
  qSlicerFileWriter* fileWriter = qobject_cast<qSlicerFileWriter*>(io);
  if (fileWriter)
    {
    d->Writers << fileWriter;
    }
  else if (fileReader)
    {
    d->Readers << fileReader;
    }

  // Reparent - this will make sure the object is destroyed properly
  if (io)
    {
    io->setParent(this);
    }
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::defaultSceneFileType()const
{
  Q_D(const qSlicerCoreIOManager);
  return d->DefaultSceneFileType;
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::setDefaultSceneFileType(QString fileType)
{
  Q_D(qSlicerCoreIOManager);
  d->DefaultSceneFileType = fileType;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::examineFileInfoList(QFileInfoList &fileInfoList, QFileInfo &archetypeFileInfo, QString &readerDescription, qSlicerIO::IOProperties &ioProperties)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerFileReader*> res;
  foreach(qSlicerFileReader* reader, d->Readers)
    {
    // TODO: currently the first reader that accepts the list will be used, but nothing
    // guarantees that the first reader is the most suitable choice (e.g., volume reader
    // grabs all file sequences, while they may not be sequence of frames but sequence of models, etc.).
    // There should be a mechanism (e.g., using confidence values or based on most specific extension)
    // to decide which reader should be used.
    // Multiple readers cannot be returned because they might not remove exactly the same set of files from the info list.
    if (reader->examineFileInfoList(fileInfoList, archetypeFileInfo, ioProperties))
      {
      readerDescription = reader->description();
      return true;
      }
    }
  return false;
}
