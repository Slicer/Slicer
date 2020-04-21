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

// Slicer includes
#include "qSlicerDoubleArraysReader.h"

// Logic includes
#include "vtkSlicerDoubleArraysLogic.h"

// MRML includes
#include <vtkMRMLDoubleArrayNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerDoubleArraysReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerDoubleArraysLogic> Logic;
};

//-----------------------------------------------------------------------------
qSlicerDoubleArraysReader::qSlicerDoubleArraysReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDoubleArraysReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDoubleArraysReader
::qSlicerDoubleArraysReader(vtkSlicerDoubleArraysLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDoubleArraysReaderPrivate)
{
  this->setLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerDoubleArraysReader::~qSlicerDoubleArraysReader() = default;

//-----------------------------------------------------------------------------
void qSlicerDoubleArraysReader::setLogic(vtkSlicerDoubleArraysLogic* logic)
{
  Q_D(qSlicerDoubleArraysReader);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerDoubleArraysLogic* qSlicerDoubleArraysReader::logic()const
{
  Q_D(const qSlicerDoubleArraysReader);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerDoubleArraysReader::description()const
{
  return "Double Array";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerDoubleArraysReader::fileType()const
{
  return QString("DoubleArrayFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerDoubleArraysReader::extensions()const
{
  return QStringList()
    << "Double Array (*.mcsv)"
    ;
}

//-----------------------------------------------------------------------------
bool qSlicerDoubleArraysReader::load(const IOProperties& properties)
{
  Q_D(qSlicerDoubleArraysReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }
  Q_ASSERT(d->Logic);
  vtkMRMLDoubleArrayNode* node = d->Logic->AddDoubleArray(
    fileName.toUtf8(),
    name.toUtf8());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != nullptr;
}
