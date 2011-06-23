/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerFiberBundleIO.h"

// Logic includes
#include "vtkSlicerFiberBundleLogic.h"

// MRML includes
#include <vtkMRMLFiberBundleNode.h>

//-----------------------------------------------------------------------------
qSlicerFiberBundleIO::qSlicerFiberBundleIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerFiberBundleIO::description()const
{
  return "FiberBundle";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerFiberBundleIO::fileType()const
{
  return qSlicerIO::DTIFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerFiberBundleIO::extensions()const
{
  return QStringList("*.*");
}

//-----------------------------------------------------------------------------
bool qSlicerFiberBundleIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QStringList fileNames;
  if (properties.contains("suffix"))
    {
    QStringList suffixList = properties["suffix"].toStringList();
    suffixList.removeDuplicates();
    // here filename describes a directory
    Q_ASSERT(QFileInfo(fileName).isDir());
    QDir dir(fileName);
    // suffix should be of style: *.png
    fileNames = dir.entryList(suffixList);
    }
  else
    {
    fileNames << fileName;
    }

  vtkSlicerFiberBundleLogic* fiberBundleLogic =
    vtkSlicerFiberBundleLogic::SafeDownCast(
      qSlicerCoreApplication::application()->moduleManager()
      ->module("tractographydisplay")->logic());
  Q_ASSERT(fiberBundleLogic);

  QStringList nodes;
  foreach(QString file, fileNames)
    {
    vtkMRMLFiberBundleNode* node = fiberBundleLogic->AddFiberBundle(file.toLatin1().data(), 1);
    if (node)
      {
      nodes << node->GetID();
      }
    }
  this->setLoadedNodes(nodes);

  return nodes.size() > 0;
}
