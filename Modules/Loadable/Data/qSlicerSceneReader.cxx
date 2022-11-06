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
#include <QVersionNumber>

#include "qSlicerSceneReader.h"
#include "qSlicerSceneIOOptionsWidget.h"

// Logic includes
#include <vtkSlicerCamerasModuleLogic.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>

// CTK includes
#include <ctkUtils.h>

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
  return tr("MRML Scene");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneReader::fileType()const
{
  return QString(/*no tr*/"SceneFile");
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
  bool success = false;
  if (clear)
    {
    qDebug("Clear and import into main MRML scene");
    success = this->mrmlScene()->Connect(this->userMessages());
    if (success)
      {
      // Set default scene file format to .mrml
      qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
      coreIOManager->setDefaultSceneFileType(tr("MRML Scene") + " (.mrml)");
      }
    }
  else
    {
    if (properties.value("copyCameras", true).toBool() == false)
      {
      qWarning() << Q_FUNC_INFO << ": copyCameras=false property is ignored, cameras are now always replaced in the scene";
      }
    success = this->mrmlScene()->Import(this->userMessages());
    }

  // Display warning message if scene file was created with a different application or with a future application version
  std::string currentApplication;
  int currentMajor = 0;
  int currentMinor = 0;
  int currentPatch = 0;
  int currentRevision = 0;
  std::string loadedApplication;
  int loadedMajor = 0;
  int loadedMinor = 0;
  int loadedPatch = 0;
  int loadedRevision = 0;
  if (vtkMRMLScene::ParseVersion(this->mrmlScene()->GetVersion(), currentApplication,
    currentMajor, currentMinor, currentPatch, currentRevision)
    && vtkMRMLScene::ParseVersion(this->mrmlScene()->GetLastLoadedVersion(), loadedApplication,
      loadedMajor, loadedMinor, loadedPatch, loadedRevision))
    {
    QStringList sceneVersionWarningMessages;
    if (loadedApplication != currentApplication)
      {
      sceneVersionWarningMessages << tr("The scene file was saved with %1 application (this application is %2).")
        .arg(QString::fromUtf8(loadedApplication.c_str()))
        .arg(QString::fromUtf8(currentApplication.c_str()));
      }
    QVersionNumber loadedSceneVersion(loadedMajor, loadedMinor, loadedPatch);
    QVersionNumber currentVersion(currentMajor, currentMinor, currentPatch);
    if (loadedSceneVersion > currentVersion)
      {
      sceneVersionWarningMessages << tr("The scene file was created with a newer version of the application (%1) than the current version (%2).")
        .arg(loadedSceneVersion.toString())
        .arg(currentVersion.toString());
      }
    if (!sceneVersionWarningMessages.isEmpty())
      {
      sceneVersionWarningMessages.push_front(tr("The scene may not load correctly.").arg(file));
      this->userMessages()->AddMessage(vtkCommand::WarningEvent, sceneVersionWarningMessages.join(" ").toStdString());
      }
    }

  // If there were scene loading errors then log the list of extensions that were installed when the scene was saved.
  // It may provide useful hints for why the extension load failed.
  if (!success
    || this->userMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0
    || this->userMessages()->GetNumberOfMessagesOfType(vtkCommand::WarningEvent) > 0)
    {
    std::string extensions = this->mrmlScene()->GetExtensions() ? this->mrmlScene()->GetExtensions() : "";
    std::string lastLoadedExtensions = this->mrmlScene()->GetLastLoadedExtensions() ? this->mrmlScene()->GetLastLoadedExtensions() : "";
    if (extensions != lastLoadedExtensions)
      {
      QStringList extensionsList = QString::fromStdString(extensions).split(";");
      QStringList lastLoadedExtensionsList = QString::fromStdString(lastLoadedExtensions).split(";");
      QSet<QString> notInstalledExtensions = ctk::qStringListToQSet(lastLoadedExtensionsList).subtract(ctk::qStringListToQSet(extensionsList));
      if (!notInstalledExtensions.isEmpty())
        {
        QString extensionsInformation =
          tr("These extensions were installed when the scene was saved but not installed now: %1."
             " These extensions may be required for successful loading of the scene.")
          .arg(ctk::qSetToQStringList(notInstalledExtensions).join(", "));
        this->userMessages()->AddMessage(vtkCommand::MessageEvent, extensionsInformation.toStdString());
        }
      }
    }

  return success;
}
