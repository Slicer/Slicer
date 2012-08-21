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

// QtCore includes
#include "qSlicerSceneIO.h"
#include "qSlicerSceneIOOptionsWidget.h"

// Logic includes
#include <vtkSlicerCamerasModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

class qSlicerSceneIOPrivate
{
public:
  vtkSmartPointer<vtkSlicerCamerasModuleLogic> CamerasLogic;
};

//-----------------------------------------------------------------------------
qSlicerSceneIO::qSlicerSceneIO(vtkSlicerCamerasModuleLogic* camerasLogic,
                               QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSceneIOPrivate)
{
  Q_D(qSlicerSceneIO);
  d->CamerasLogic = camerasLogic;
}

//-----------------------------------------------------------------------------
qSlicerSceneIO::~qSlicerSceneIO()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSceneIO::description()const
{
  return "MRML Scene";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneIO::fileType()const
{
  return QString("SceneFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneIO::extensions()const 
{
  return QStringList() << "*.mrml";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerSceneIO::options()const
{
  return new qSlicerSceneIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerSceneIO::load(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerSceneIO);
  Q_ASSERT(properties.contains("fileName"));
  QString file = properties["fileName"].toString();
  this->mrmlScene()->SetURL(file.toLatin1());
  bool clear = properties.value("clear", false).toBool();
  int res = 0;
  if (clear)
    {
    res = this->mrmlScene()->Connect();
    }
  else
    {
    bool wasCopying = d->CamerasLogic->GetCopyImportedCameras();
    bool copyCameras = properties.value("copyCameras", wasCopying).toBool();
    d->CamerasLogic->SetCopyImportedCameras(copyCameras);
    res = this->mrmlScene()->Import();
    d->CamerasLogic->SetCopyImportedCameras(wasCopying);
    }
  return res;
}
