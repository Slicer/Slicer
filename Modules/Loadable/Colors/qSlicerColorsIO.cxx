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

// SlicerQt includes
#include "qSlicerColorsIO.h"

// Logic includes
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerColorsIOPrivate
{
public:
  vtkSmartPointer<vtkSlicerColorLogic> ColorLogic;
};

//-----------------------------------------------------------------------------
qSlicerColorsIO::qSlicerColorsIO(
  vtkSlicerColorLogic* _colorLogic, QObject* _parent)
  : qSlicerIO(_parent)
  , d_ptr(new qSlicerColorsIOPrivate)
{
  this->setColorLogic(_colorLogic);
}

//-----------------------------------------------------------------------------
void qSlicerColorsIO::setColorLogic(vtkSlicerColorLogic* newColorLogic)
{
  Q_D(qSlicerColorsIO);
  d->ColorLogic = newColorLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerColorLogic* qSlicerColorsIO::colorLogic()const
{
  Q_D(const qSlicerColorsIO);
  return d->ColorLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerColorsIO::description()const
{
  return "Color";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerColorsIO::fileType()const
{
  return qSlicerIO::ColorTableFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerColorsIO::extensions()const
{
  return QStringList() << "Color (*.txt *.ctbl)";
}

//-----------------------------------------------------------------------------
bool qSlicerColorsIO::load(const IOProperties& properties)
{
  Q_D(qSlicerColorsIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  if (d->ColorLogic.GetPointer() == 0)
    {
    return false;
    }

  vtkMRMLColorNode* node = d->ColorLogic->LoadColorFile(fileName.toLatin1());
  QStringList loadedNodes;
  if (node)
    {
    loadedNodes << QString(node->GetID());
    }
  this->setLoadedNodes(loadedNodes);
  return node != 0;
}

// TODO: add the save() method. Use vtkSlicerColorLogic::SaveColor()
