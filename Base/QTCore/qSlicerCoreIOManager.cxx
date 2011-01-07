/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QStringList>

// CTK includes
#include <ctkLogger.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerIO.h"
#include "qSlicerModuleManager.h"
#include "qSlicerSceneIO.h"
#include "qSlicerSlicer2SceneReader.h"
#include "qSlicerXcedeCatalogIO.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtcore.qSlicerCoreIOManager");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerCoreIOManagerPrivate
{
public:
  qSlicerCoreIOManagerPrivate();
  ~qSlicerCoreIOManagerPrivate();
  vtkMRMLScene* currentScene()const;
  
  qSlicerIO* reader(const QString& fileName)const;

  QSettings*        ExtensionFileType;
  QList<qSlicerIO*> Readers;
};

//-----------------------------------------------------------------------------
qSlicerCoreIOManagerPrivate::qSlicerCoreIOManagerPrivate()
{
  logger.setTrace();
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
  QStringList genericExtensions;
  genericExtensions << "*.*" << "(*)";
  QList<qSlicerIO*> genericReaders;
  foreach(qSlicerIO* reader, this->Readers)
    {
    if (!reader->canLoadFile(fileName))
      {
      continue;
      }
    bool generic = false;
    foreach(QString extension, reader->extensions())
      {
      foreach(QString genericExtension, genericExtensions)
        {
        if (extension.contains(genericExtension))
          {
          genericReaders << reader;
          generic = true;
          }
        }
      }
    if (generic)
      {
      continue;
      }
    return reader;
    }
  foreach(qSlicerIO* reader, genericReaders)
    {
    return reader;
    }
  return 0;
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::qSlicerCoreIOManager(QObject* _parent)
  :QObject(_parent)
  , d_ptr(new qSlicerCoreIOManagerPrivate)
{
  // FIXME move to the application level
  this->registerIO(new qSlicerSceneIO(this));
  this->registerIO(new qSlicerSlicer2SceneReader(this));
  this->registerIO(new qSlicerXcedeCatalogIO(this));
  // end FIXME
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::~qSlicerCoreIOManager()
{
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCoreIOManager::fileType(const QString& fileName)const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerIO* reader = d->reader(fileName);
  if (!reader)
    {
    return  qSlicerIO::NoFile;
    }
  return reader->fileType();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::fileDescription(const QString& fileName)const
{
  Q_D(const qSlicerCoreIOManager);
  qSlicerIO* reader = d->reader(fileName);
  if (!reader)
    {
    return tr("Unknown");
    }
  return reader->description();
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerCoreIOManager::fileOptions(const qSlicerIO::IOFileType& fileType)const
{
  Q_D(const qSlicerCoreIOManager);
  QList<qSlicerIO*> readers = this->ios(fileType);
  if (readers.isEmpty())
    {
    return 0;
    }
  readers[0]->setMRMLScene(d->currentScene());
  return readers[0]->options();
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
                                     const qSlicerIO::IOProperties& parameters,
                                     vtkCollection* loadedNodes)
{ 
  Q_D(qSlicerCoreIOManager);

  Q_ASSERT(parameters.contains("fileName"));
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
        d->currentScene()->GetNodeByID(node.toLatin1().data()));
      }
    }
    
  return success;
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
  //d->currentScene()->GetNodeByID(node.toLatin1().data()));
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
void qSlicerCoreIOManager::registerIO(qSlicerIO* io)
{
  Q_ASSERT(io);
  Q_D(qSlicerCoreIOManager);
  logger.trace(QString("registerIO %1").arg(io->metaObject()->className()));
  d->Readers << io;

  // Reparent - this will make sure the object is destroyed properly
  io->setParent(this);
}
