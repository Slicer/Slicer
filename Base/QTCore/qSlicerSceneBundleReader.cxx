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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QDir>
#include <QDateTime>

// QtCore includes
#include "qSlicerSceneBundleReader.h"

// CTK includes
#include <ctkUtils.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>

// MRML Logic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkNew.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

//-----------------------------------------------------------------------------
qSlicerSceneBundleReader::qSlicerSceneBundleReader(QObject* _parent)
  : Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerSceneBundleReader::description()const
{
  return "MRB Slicer Data Bundle";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneBundleReader::fileType()const
{
  return QString("SceneFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneBundleReader::extensions()const
{
  return QStringList() << "*.mrb" << "*.zip" << "*.xar";
}

//-----------------------------------------------------------------------------
bool qSlicerSceneBundleReader::load(const qSlicerIO::IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString file = properties["fileName"].toString();

  // check for a relative path as the unzip will need an absolute one
  QFileInfo fileInfo(file);
  if (fileInfo.isRelative())
    {
    fileInfo = QFileInfo(QDir::currentPath(), file);
    file = fileInfo.absoluteFilePath();
    }

  // TODO: switch to QTemporaryDir in Qt5.
  QString unpackPath( QDir::tempPath() +
                        QString("/__BundleLoadTemp") +
                          QDateTime::currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz") );

  qDebug() << "Unpacking bundle " << file << " to " << unpackPath;

  if (QFileInfo(unpackPath).isDir())
    {
    if (!ctk::removeDirRecursively(unpackPath))
      {
      return false;
      }
    }

  if (!QDir().mkpath(unpackPath))
    {
    return false;
    }

  vtkNew<vtkMRMLApplicationLogic> appLogic;
  appLogic->SetMRMLScene( this->mrmlScene() );
  std::string mrmlFile = appLogic->UnpackSlicerDataBundle(
                                          file.toLatin1(), unpackPath.toLatin1() );

  this->mrmlScene()->SetURL(mrmlFile.c_str());

  bool clear = false;
  if (properties.contains("clear"))
    {
    clear = properties["clear"].toBool();
    }
  int res = 0;
  if (clear)
    {
    res = this->mrmlScene()->Connect();
    }
  else
    {
    res = this->mrmlScene()->Import();
    }

  if (!ctk::removeDirRecursively(unpackPath))
    {
    return false;
    }

  qDebug() << "Loaded bundle from " << unpackPath;
  // Set default scene file format to mrb
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  coreIOManager->setDefaultSceneFileType("Medical Reality Bundle (.mrb)");

  // since the unpack path has been deleted, reset the scene to where the data bundle is
  QString mrbDirectoryPath = QFileInfo(file).dir().absolutePath();
  QString mrbBaseName = QFileInfo(file).baseName();
  QString resetURL = mrbDirectoryPath + QString("/") + mrbBaseName + QString(".mrml");
  this->mrmlScene()->SetURL(resetURL.toLatin1());
  qDebug() << "Reset scene to point to the MRB directory " << this->mrmlScene()->GetURL();

  // Change root directory to mrb file location
  QDir rootDirectory(this->mrmlScene()->GetRootDirectory());
  this->mrmlScene()->SetRootDirectory(mrbDirectoryPath.toLatin1().constData());

  // Make storage file names relative to root directory by removing
  // the temporary unpack directory
  std::vector<vtkMRMLNode *> storageNodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLStorageNode", storageNodes);
  for (std::vector<vtkMRMLNode *>::iterator storageNodeIt = storageNodes.begin(); storageNodeIt != storageNodes.end(); ++storageNodeIt)
    {
    vtkMRMLStorageNode* storageNode = vtkMRMLStorageNode::SafeDownCast(*storageNodeIt);
    if (!storageNode)
      {
      continue;
      }
    for (int i = -1; i < storageNode->GetNumberOfFileNames(); ++i)
      {
      const char* storageFileNamePtr = NULL;
      if (i < 0)
        {
        storageFileNamePtr = storageNode->GetFileName();
        }
      else
        {
        storageFileNamePtr = storageNode->GetNthFileName(i);
        }
      if (!storageFileNamePtr)
        {
        continue;
        }
      QString storageFileName = QString(storageFileNamePtr);
      if (!storageFileName.startsWith(unpackPath))
        {
        continue;
        }
      storageFileName = rootDirectory.relativeFilePath(storageFileName);
      if (i < 0)
        {
        storageNode->SetFileName(storageFileName.toLatin1().constData());
        }
      else
        {
        storageNode->ResetNthFileName(i, storageFileName.toLatin1().constData());
        }
      }
    }

  // and mark storable nodes as modified since read
  this->mrmlScene()->SetStorableNodesModifiedSinceRead();
  // MRBs come with default scene views, but the paths of storage nodes in there can be still pointing to the bundle extraction directory that was removed. Clear out the file lists at least so that they get reset
  return res;
}
