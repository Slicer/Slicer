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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Qt includes
#include <QFileInfo>
#include <QRegExp>

/// QtCore includes
#include "qSlicerIO.h"

/// MRML includes
#include <vtkMRMLScene.h>

/// VTK includes
#include <vtkWeakPointer.h>

const char *extensionsRegExp =
  "^(.*)\\(([a-zA-Z0-9_.*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

//-----------------------------------------------------------------------------
class qSlicerIOPrivate
{
public:
  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  
  QStringList LoadedNodes;
  QStringList SavedNodes;
};

//----------------------------------------------------------------------------
qSlicerIO::qSlicerIO(QObject* _parent)
  :QObject(_parent)
  , d_ptr(new qSlicerIOPrivate)
{
  qRegisterMetaType<qSlicerIO::IOFileType>("qSlicerIO::IOFileType");
  qRegisterMetaType<qSlicerIO::IOProperties>("qSlicerIO::IOProperties");
}

//----------------------------------------------------------------------------
qSlicerIO::~qSlicerIO()
{
}

/*
//----------------------------------------------------------------------------
vtkStdString qSlicerIO::description()const
{
  // FIXME: abstract method ?
  return "File";
}

//----------------------------------------------------------------------------
IOFileType qSlicerIO::fileType()const
{
  // FIXME: abstract method ?
  return NoFile;
}
*/

//----------------------------------------------------------------------------
QStringList qSlicerIO::extensions()const
{
  return QStringList() << "*.*";
}

//----------------------------------------------------------------------------
bool qSlicerIO::canLoadFile(const QString& fileName)const
{
  QFileInfo file(fileName);
  if (!file.isFile() || 
      !file.isReadable() || 
      file.suffix().contains('~'))
    {
    return false;
    }
  foreach(const QString& filter, this->extensions())
    {
    foreach(QString extension, qSlicerIO::extractExtensions(filter))
      {
      QRegExp regExp(extension, Qt::CaseInsensitive, QRegExp::Wildcard);
      Q_ASSERT(regExp.isValid());
      if (regExp.exactMatch(file.absoluteFilePath()))
        {
        return true;
        }
      }
    }
  return false;
}

//----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerIO::options()const
{
  return 0;
}

//----------------------------------------------------------------------------
void qSlicerIO::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerIO);
  d->MRMLScene = scene;
}

//----------------------------------------------------------------------------
vtkMRMLScene* qSlicerIO::mrmlScene()const
{
  Q_D(const qSlicerIO);
  return d->MRMLScene;
}

//----------------------------------------------------------------------------
bool qSlicerIO::load(const IOProperties& properties)
{
  Q_D(qSlicerIO);
  Q_UNUSED(properties);
  d->LoadedNodes.clear();
  return false;
}

//----------------------------------------------------------------------------
bool qSlicerIO::save(const IOProperties& properties)
{
  Q_UNUSED(properties);
  return false;
}

//----------------------------------------------------------------------------
void qSlicerIO::setLoadedNodes(const QStringList& nodes)
{
  Q_D(qSlicerIO);
  d->LoadedNodes = nodes;
}

//----------------------------------------------------------------------------
QStringList qSlicerIO::loadedNodes()const
{
  Q_D(const qSlicerIO);
  return d->LoadedNodes;
}

//----------------------------------------------------------------------------
void qSlicerIO::setSavedNodes(const QStringList& nodes)
{
  Q_D(qSlicerIO);
  d->SavedNodes = nodes;
}

//----------------------------------------------------------------------------
QStringList qSlicerIO::savedNodes()const
{
  Q_D(const qSlicerIO);
  return d->SavedNodes;
}

//-----------------------------------------------------------------------------
QStringList qSlicerIO::extractExtensions(const QString &filter)
{
  QRegExp regexp(QString::fromLatin1(extensionsRegExp));
  QString f = filter;
  if (regexp.indexIn(f) >= 0)
    {
    f = regexp.cap(2);
    }
  return f.split(QLatin1Char(' '), QString::SkipEmptyParts);
}
