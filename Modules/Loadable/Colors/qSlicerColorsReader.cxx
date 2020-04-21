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

// Slicer includes
#include "qSlicerColorsReader.h"

// Logic includes
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerColorsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerColorLogic> ColorLogic;
};

//-----------------------------------------------------------------------------
qSlicerColorsReader::qSlicerColorsReader(
  vtkSlicerColorLogic* _colorLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerColorsReaderPrivate)
{
  this->setColorLogic(_colorLogic);
}

//-----------------------------------------------------------------------------
qSlicerColorsReader::~qSlicerColorsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerColorsReader::setColorLogic(vtkSlicerColorLogic* newColorLogic)
{
  Q_D(qSlicerColorsReader);
  d->ColorLogic = newColorLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerColorLogic* qSlicerColorsReader::colorLogic()const
{
  Q_D(const qSlicerColorsReader);
  return d->ColorLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerColorsReader::description()const
{
  return "Color";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerColorsReader::fileType()const
{
  return QString("ColorTableFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerColorsReader::extensions()const
{
  return QStringList() << "Color (*.txt *.ctbl *.cxml)";
}

//-----------------------------------------------------------------------------
bool qSlicerColorsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerColorsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  if (d->ColorLogic.GetPointer() == nullptr)
    {
    return false;
    }

  vtkMRMLColorNode* node = d->ColorLogic->LoadColorFile(fileName.toUtf8());
  QStringList loadedNodes;
  if (node)
    {
    loadedNodes << QString(node->GetID());
    }
  this->setLoadedNodes(loadedNodes);
  return node != nullptr;
}

// TODO: add the save() method. Use vtkSlicerColorLogic::SaveColor()
