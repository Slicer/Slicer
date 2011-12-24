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
#include "qSlicerVolumeRenderingIO.h"

// Logic includes
#include "vtkSlicerVolumeRenderingLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include "vtkMRMLVolumePropertyNode.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerVolumeRenderingIOPrivate
{
  public:
  vtkSmartPointer<vtkSlicerVolumeRenderingLogic> VolumeRenderingLogic;
};

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingIO::qSlicerVolumeRenderingIO(QObject* _parent)
  : qSlicerIO(_parent)
  , d_ptr(new qSlicerVolumeRenderingIOPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingIO::~qSlicerVolumeRenderingIO()
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingIO::qSlicerVolumeRenderingIO(vtkSlicerVolumeRenderingLogic* logic, QObject* _parent)
  : qSlicerIO(_parent)
  , d_ptr(new qSlicerVolumeRenderingIOPrivate)
{
  this->setVolumeRenderingLogic(logic);
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingIO::setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic)
{
  Q_D(qSlicerVolumeRenderingIO);
  d->VolumeRenderingLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic* qSlicerVolumeRenderingIO::volumeRenderingLogic()const
{
  Q_D(const qSlicerVolumeRenderingIO);
  return d->VolumeRenderingLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingIO::description()const
{
  return "Transfer Function";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVolumeRenderingIO::fileType()const
{
  return qSlicerIO::TransferFunctionFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeRenderingIO::extensions()const
{
  // pic files are bio-rad images (see itkBioRadImageIO)
  return QStringList()
    << "Transfer Function (*.vp)";
}

//-----------------------------------------------------------------------------
bool qSlicerVolumeRenderingIO::load(const IOProperties& properties)
{
  Q_D(qSlicerVolumeRenderingIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();
  // Name is ignored
  //QString name = QFileInfo(fileName).baseName();
  //if (properties.contains("name"))
  //  {
  //  name = properties["name"].toString();
  //  }
  if (d->VolumeRenderingLogic.GetPointer() == 0)
    {
    return false;
    }
  vtkMRMLVolumePropertyNode* node =
    d->VolumeRenderingLogic->AddVolumePropertyFromFile(fileName.toLatin1());
  QStringList loadedNodes;
  if (node)
    {
    loadedNodes << QString(node->GetID());
    }
  this->setLoadedNodes(loadedNodes);
  return node != 0;
}
