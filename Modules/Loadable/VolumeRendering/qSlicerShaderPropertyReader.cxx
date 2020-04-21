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

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/

// Qt includes
#include <QFileInfo>

// Slicer includes
#include "qSlicerShaderPropertyReader.h"

// Logic includes
#include "vtkSlicerVolumeRenderingLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include "vtkMRMLShaderPropertyNode.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerShaderPropertyReaderPrivate
{
  public:
  vtkSmartPointer<vtkSlicerVolumeRenderingLogic> VolumeRenderingLogic;
};

//-----------------------------------------------------------------------------
qSlicerShaderPropertyReader::qSlicerShaderPropertyReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerShaderPropertyReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerShaderPropertyReader::qSlicerShaderPropertyReader(vtkSlicerVolumeRenderingLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerShaderPropertyReaderPrivate)
{
  this->setVolumeRenderingLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerShaderPropertyReader::~qSlicerShaderPropertyReader() = default;

//-----------------------------------------------------------------------------
void qSlicerShaderPropertyReader::setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic)
{
  Q_D(qSlicerShaderPropertyReader);
  d->VolumeRenderingLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic* qSlicerShaderPropertyReader::volumeRenderingLogic()const
{
  Q_D(const qSlicerShaderPropertyReader);
  return d->VolumeRenderingLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerShaderPropertyReader::description()const
{
  return "GPU Shader Property";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerShaderPropertyReader::fileType()const
{
  return QString("ShaderPropertyFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerShaderPropertyReader::extensions()const
{
  return QStringList()
    << "Shader Property (*.sp)";
}

//-----------------------------------------------------------------------------
bool qSlicerShaderPropertyReader::load(const IOProperties& properties)
{
  Q_D(qSlicerShaderPropertyReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();
  // Name is ignored
  //QString name = QFileInfo(fileName).baseName();
  //if (properties.contains("name"))
  //  {
  //  name = properties["name"].toString();
  //  }
  if (d->VolumeRenderingLogic.GetPointer() == nullptr)
    {
    return false;
    }
  vtkMRMLShaderPropertyNode* node =
    d->VolumeRenderingLogic->AddShaderPropertyFromFile(fileName.toUtf8());
  QStringList loadedNodes;
  if (node)
    {
    loadedNodes << QString(node->GetID());
    }
  this->setLoadedNodes(loadedNodes);
  return node != nullptr;
}
