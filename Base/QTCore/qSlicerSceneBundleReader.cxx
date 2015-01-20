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

// MRML Logic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkNew.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

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

  // TODO: switch to QTemporaryDir in Qt5.
  QString unpackPath( QDir::tempPath() +
                        QString("/__BundleLoadTemp") +
                          QDateTime::currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz") );

  qDebug() << "Unpacking bundle to " << unpackPath;

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
  return res;
}
