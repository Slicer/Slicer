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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerApplicationHelper.h"
#include "qSlicerStyle.h"
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_FULL

// SlicerApp includes
#include "qSlicerAppMainWindow.h"

// Qt Includes
#include <QResource>
#include <QSettings>

//MRML includes
#include <vtkMRMLLogic.h>

namespace
{

//----------------------------------------------------------------------------
int SlicerAppMain(int argc, char* argv[])
{
  typedef qSlicerAppMainWindow SlicerMainWindowType;
  typedef qSlicerStyle SlicerAppStyle;

  qSlicerApplicationHelper::preInitializeApplication(argv[0], new SlicerAppStyle);

  qSlicerApplication app(argc, argv);

  // Initialize library resources here
  QSettings settingsApplication;
  QSettings settingsRegistry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);


  std::string resourcePath = vtkMRMLLogic::GetApplicationShareDirectory() + "/Icons/qSlicerBaseQTGUILightIcons.rcc";

  if (settingsApplication.value("Styles/Style", "Slicer").toString() == "Dark Slicer")
  {
    resourcePath = vtkMRMLLogic::GetApplicationShareDirectory() + "/Icons/qSlicerBaseQTGUIDarkIcons.rcc";

  }
  else if (settingsApplication.value("Styles/Style", "Slicer").toString() == "Slicer")
  {
#ifdef Q_OS_WIN
    if (settingsRegistry.value("AppsUseLightTheme") == 0)
    {
      resourcePath = vtkMRMLLogic::GetApplicationShareDirectory() + "/Icons/qSlicerBaseQTGIUDarkIcons.rcc";
    }
#endif
  }
  QResource::registerResource(resourcePath.c_str());


  if (app.returnCode() != -1)
  {
    return app.returnCode();
  }

  QScopedPointer<SlicerMainWindowType> window;
  QScopedPointer<QSplashScreen> splashScreen;

  app.setURIArgumentHandlingEnabled(true);

  int exitCode = qSlicerApplicationHelper::postInitializeApplication<SlicerMainWindowType>(
        app, splashScreen, window);
  if (exitCode != 0)
  {
    return exitCode;
  }

  if (!window.isNull())
  {
    QString windowTitle = QString("%1 %2").arg(window->windowTitle()).arg(Slicer_VERSION_FULL);
    window->setWindowTitle(windowTitle);
  }

  return app.exec();
}

} // end of anonymous namespace

#include "qSlicerApplicationMainWrapper.cxx"
