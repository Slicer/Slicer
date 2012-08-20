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
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerDoubleArraysIO.h"

// Logic includes
#include "vtkSlicerDoubleArraysLogic.h"

// MRML includes
#include <vtkMRMLDoubleArrayNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerDoubleArraysIOPrivate
{
public:
  vtkSmartPointer<vtkSlicerDoubleArraysLogic> Logic;
};

//-----------------------------------------------------------------------------
qSlicerDoubleArraysIO::qSlicerDoubleArraysIO(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDoubleArraysIOPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDoubleArraysIO
::qSlicerDoubleArraysIO(vtkSlicerDoubleArraysLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDoubleArraysIOPrivate)
{
  this->setLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerDoubleArraysIO::~qSlicerDoubleArraysIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerDoubleArraysIO::setLogic(vtkSlicerDoubleArraysLogic* logic)
{
  Q_D(qSlicerDoubleArraysIO);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerDoubleArraysLogic* qSlicerDoubleArraysIO::logic()const
{
  Q_D(const qSlicerDoubleArraysIO);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerDoubleArraysIO::description()const
{
  return "Double Array";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerDoubleArraysIO::fileType()const
{
  return qSlicerIO::DoubleArrayFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerDoubleArraysIO::extensions()const
{
  return QStringList()
    << "Double Array (*.csv)"
    ;
}

//-----------------------------------------------------------------------------
bool qSlicerDoubleArraysIO::load(const IOProperties& properties)
{
  Q_D(qSlicerDoubleArraysIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }
  Q_ASSERT(d->Logic);
  vtkMRMLDoubleArrayNode* node = d->Logic->AddDoubleArray(
    fileName.toLatin1(),
    name.toLatin1());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}
