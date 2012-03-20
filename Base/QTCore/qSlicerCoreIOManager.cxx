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

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerSceneIO.h"
#include "qSlicerSlicer2SceneReader.h"
#include "qSlicerXcedeCatalogIO.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerCoreIOManagerPrivate
{
public:
  qSlicerCoreIOManagerPrivate();
  ~qSlicerCoreIOManagerPrivate();
  vtkMRMLScene* currentScene()const;

  qSlicerIO* reader(const QString& fileName)const;
  QList<qSlicerIO*> readers(const QString& fileName)const;

  QSettings*        ExtensionFileType;
  QList<qSlicerIO*> Readers;
};

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::qSlicerCoreIOManagerPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::~qSlicerCoreIOManagerPrivate()
{
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerCoreIOManagerPrivate::currentScene()const
{
  return qSlicerCoreApplication::application()->mrmlScene();
}

//-----------------------------------------------------------------------------
qSlicerIO* qSlicerCoreIOManagerPrivate::reader(const QString& fileName)const
{
  QList<qSlicerIO*> matchingReaders = this->readers(fileName);
  return matchingReaders.count() ? matchingReaders[0] : 0;
}

//-----------------------------------------------------------------------------
QList<qSlicerIO*> qSlicerCoreIOManagerPrivate::readers(const QString& fileName)const
{
  QList<qSlicerIO*> matchingReaders;
  // Some readers ("DICOM (*)" or "Scalar Overlay (*.*))" can support any file,
  // they are called generic readers. They might not be the best choice to read
  // the file as it might exist a more specific reader to read it.
  // So let's add generic readers at the end of the reader list.
  QList<qSlicerIO*> genericReaders;
  foreach(qSlicerIO* reader, this->Readers)
    {
    QStringList matchingNameFilters = reader->supportedNameFilters(fileName);
    if (matchingNameFilters.count() == 0)
      {
      continue;
      }
    // Generic readers must be added to the end
    foreach(const QString& nameFilter, matchingNameFilters)
      {
      if (nameFilter.contains( "*.*" ) || nameFilter.contains("(*)"))
        {
        genericReaders << reader;
        continue;
        }
      if (!matchingReaders.contains(reader))
        {
        matchingReaders << reader;
        }
      }
    }
  foreach(qSlicerIO* reader, genericReaders)
    {
    if (!matchingReaders.contains(reader))
      {
      matchingReaders << reader;
      }
    }
  return matchingReaders;
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::qSlicerCoreIOManager(QObject* _parent)
  :QObject(_parent)
  , d_ptr(new qSlicerCoreIOManagerPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::~qSlicerCoreIOManager()
{
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager::fileType(const QString& fileName)const
{
  QList<qSlicerIO::IOFileType> matchingFileTypes = this->fileTypes(fileName);
  return matchingFileTypes.count() ? matchingFileTypes[0] : qSlicerIO::NoFile;
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager
::fileTypeFromDescription(const QString& fileDescription)const
{
  qSlicerIO* reader = this->io(fileDescription);
  return reader ? reader->fileType() : qSlicerIO::NoFile;
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
  foreach(qSlicerIO* reader, d->readers(fileName))
    {
    matchingDescriptions << reader->description();
    }
  return matchingDescriptions;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreIOManager::fileDescriptions(const qSlicerIO::IOFileType fileType)const
{
  QStringList matchingDescriptions;
  foreach(qSlicerIO* reader, this->ios())
    {
    if (reader->fileType() == fileType)
      {
      matchingDescriptions << reader->description();
      }
    }
  return matchingDescriptions;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::fileOptions(const QString& readerDescription)const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerIO* reader = this->io(readerDescription);
  if (!reader)
    {
    return 0;
    }
  reader->setMRMLScene(d->currentScene());
  return reader->options();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadScene(const QString& fileName, bool clear)
{
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  properties["clear"] = clear;
  return this->loadNodes(qSlicerIO::SceneFile, properties);
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadNodes(const qSlicerIO::IOFileType& fileType,
#if QT_VERSION < 0x040700
                                     const QVariantMap& parameters,
#else
                                     const qSlicerIO::IOProperties& parameters,
#endif
                                     vtkCollection* loadedNodes)
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
      res &= this->loadNodes(fileType, fileParameters, loadedNodes);
      }
    return res;
    }
  Q_ASSERT(!parameters["fileName"].toString().isEmpty());

  const QList<qSlicerIO*>& readers = this->ios(fileType);

  // If no readers were able to read and load the file(s), success will remain false
  bool success = false;

  QStringList nodes;
  foreach (qSlicerIO* reader, readers)
    {
    reader->setMRMLScene(d->currentScene());
    if (!reader->canLoadFile(parameters["fileName"].toString()))
      {
      continue;
      }
    if (!reader->load(parameters))
      {
      continue;
      }
    qDebug() << reader->description() << "Reader has successfully read the file"
             << parameters["fileName"].toString();
    nodes << reader->loadedNodes();
    success = true;
    break;
    }

  if (loadedNodes)
    {
    foreach(const QString& node, nodes)
      {
      loadedNodes->AddItem(
        d->currentScene()->GetNodeByID(node.toLatin1()));
      }
    }

  return success;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::loadNodes(const QList<qSlicerIO::IOProperties>& files,
                                     vtkCollection* loadedNodes)
{
  bool res = true;
  foreach(qSlicerIO::IOProperties fileProperties, files)
    {
    res = this->loadNodes(static_cast<qSlicerIO::IOFileType>(
                            fileProperties["fileType"].toInt()),
                          fileProperties, loadedNodes)
      && res;
    }
  return res;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerCoreIOManager::loadNodesAndGetFirst(
  qSlicerIO::IOFileType fileType,
  const qSlicerIO::IOProperties& parameters)
{
  vtkSmartPointer<vtkCollection> loadedNodes = vtkSmartPointer<vtkCollection>::New();
  this->loadNodes(fileType, parameters, loadedNodes);

  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(0));
  Q_ASSERT(node);

  return node;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreIOManager::saveNodes(qSlicerIO::IOFileType fileType,
                                     const qSlicerIO::IOProperties& parameters)
{
  Q_D(qSlicerCoreIOManager);

  Q_ASSERT(parameters.contains("fileName"));

  const QList<qSlicerIO*>& writers = this->ios(fileType);

  QStringList nodes;
  foreach (qSlicerIO* writer, writers)
    {
    writer->setMRMLScene(d->currentScene());
    if (!writer->save(parameters))
      {
      continue;
      }
    qDebug() << "reader has read the file" << parameters["fileName"].toString();
    nodes << writer->savedNodes();
    break;
    }

  if (nodes.count())
    {
    return false;
    }

  //if (savedNodes)
  //{
  //foreach(const QString& node, nodes)
  //{
  //loadedNodes->AddItem(
  //d->currentScene()->GetNodeByID(node.toLatin1()));
  //}
  //}
  return true;
}

//-----------------------------------------------------------------------------
const QList<qSlicerIO*>& qSlicerCoreIOManager::ios()const
{
  Q_D(const qSlicerCoreIOManager);
  return d->Readers;
}

//-----------------------------------------------------------------------------
QList<qSlicerIO*> qSlicerCoreIOManager::ios(const qSlicerIO::IOFileType& fileType)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerIO*> res;
  foreach(qSlicerIO* io, d->Readers)
    {
    if (io->fileType() == fileType)
      {
      res << io;
      }
    }
  return res;
}

//-----------------------------------------------------------------------------
qSlicerIO* qSlicerCoreIOManager::io(const QString& ioDescription)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerIO*> res;
  foreach(qSlicerIO* io, d->Readers)
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
  qDebug() << "RegisterIO" << QLatin1String(io->metaObject()->className());
  d->Readers << io;

  // Reparent - this will make sure the object is destroyed properly
  io->setParent(this);
}
