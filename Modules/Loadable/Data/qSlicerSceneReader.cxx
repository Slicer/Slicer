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
#include <QDebug>
#include <QMessageBox>

#include "qSlicerSceneReader.h"
#include "qSlicerSceneIOOptionsWidget.h"

// Logic includes
#include <vtkSlicerCamerasModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

class qSlicerSceneReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerCamerasModuleLogic> CamerasLogic;
};

//-----------------------------------------------------------------------------
qSlicerSceneReader::qSlicerSceneReader(vtkSlicerCamerasModuleLogic* camerasLogic,
                               QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSceneReaderPrivate)
{
  Q_D(qSlicerSceneReader);
  d->CamerasLogic = camerasLogic;
}

//-----------------------------------------------------------------------------
qSlicerSceneReader::~qSlicerSceneReader() = default;

//-----------------------------------------------------------------------------
QString qSlicerSceneReader::description()const
{
  return "MRML Scene";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneReader::fileType()const
{
  return QString("SceneFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneReader::extensions()const
{
  return QStringList() << "*.mrml";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerSceneReader::options()const
{
  return new qSlicerSceneIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerSceneReader::load(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerSceneReader);
  Q_ASSERT(properties.contains("fileName"));
  QString file = properties["fileName"].toString();
  this->mrmlScene()->SetURL(file.toUtf8());
  bool clear = properties.value("clear", false).toBool();
  int res = 0;
  if (clear)
    {
    qDebug("Clear and import into main MRML scene");
    res = this->mrmlScene()->Connect();
    if (res)
      {
      // Set default scene file format to .mrml
      qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
      coreIOManager->setDefaultSceneFileType("MRML Scene (.mrml)");
      }
    }
  else
    {
    if (properties.value("copyCameras", true).toBool() == false)
      {
      qWarning() << Q_FUNC_INFO << ": copyCameras=false property is ignored, cameras are now always replaced in the scene";
      }
    res = this->mrmlScene()->Import();
    }

  if (this->mrmlScene()->GetLastLoadedVersion() &&
     this->mrmlScene()->GetVersion() &&
      strcmp(this->mrmlScene()->GetLastLoadedVersion(),
             this->mrmlScene()->GetVersion()) > 0 )
    {
      std::string msg = "Warning: scene file " + file.toStdString() + " has version " +
        std::string(this->mrmlScene()->GetLastLoadedVersion()) + " greater than Slicer4 version " +
        std::string(this->mrmlScene()->GetVersion()) + ".";

      QMessageBox::warning(nullptr, tr("Reading MRML Scene..."),
                              tr(msg.c_str()) );
    }

  return res;
}
