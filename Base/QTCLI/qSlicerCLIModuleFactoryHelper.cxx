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

// Qt includes
#include <QSettings>

// QtCLI includes
#include "qSlicerCLIModuleFactoryHelper.h"

// Slicer includes
#include "qSlicerCoreApplication.h" // For: Slicer_CLIMODULES_LIB_DIR
#include "qSlicerUtils.h"

//-----------------------------------------------------------------------------
const QStringList qSlicerCLIModuleFactoryHelper::modulePaths()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (!app)
    {
    qCritical("qSlicerCLIModuleFactoryHelper::modulePaths failed: qSlicerCoreApplication is not instantiated");
    return QStringList();
    }

  // slicerHome shouldn't be empty
  if (!app->slicerHome().isEmpty())
    {
    qCritical("qSlicerCLIModuleFactoryHelper::modulePaths failed: app->slicerHome() is empty");
    return QStringList();
    }

  QStringList defaultCmdLineModulePaths;
  if (QFile::exists(app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR))
    {
    defaultCmdLineModulePaths << app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR;
    if (!app->intDir().isEmpty())
       {
       // On Win32, *both* paths have to be there, since scripts are installed
       // in the install location, and exec/libs are *automatically* installed
       // in intDir.
       defaultCmdLineModulePaths << app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR + "/" + app->intDir();
       }
    }

  QSettings * settings = app->revisionUserSettings();
  QStringList additionalModulePaths = settings->value("Modules/AdditionalPaths").toStringList();
  QStringList cmdLineModulePaths = additionalModulePaths + defaultCmdLineModulePaths;
  foreach(const QString& path, cmdLineModulePaths)
    {
    app->addLibraryPath(path);
    }
  return cmdLineModulePaths;
}

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleFactoryHelper::isInstalled(const QString& path)
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  return app ? qSlicerUtils::isPluginInstalled(path, app->slicerHome()) : false;
}

//-----------------------------------------------------------------------------
bool qSlicerCLIModuleFactoryHelper::isBuiltIn(const QString& path)
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  return app ? qSlicerUtils::isPluginBuiltIn(path, app->slicerHome()) : true;
}
