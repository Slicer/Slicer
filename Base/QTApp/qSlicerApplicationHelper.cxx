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

#include "qSlicerApplicationHelper.h"

// Qt includes
#include <QSettings>

// Slicer includes
#include "qSlicerApplication.h"
#ifdef Slicer_BUILD_CLI_SUPPORT
# include "qSlicerCLIExecutableModuleFactory.h"
# include "qSlicerCLILoadableModuleFactory.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"

#ifdef Slicer_USE_PYTHONQT
# include "qSlicerScriptedLoadableModuleFactory.h"
#endif

//----------------------------------------------------------------------------
qSlicerApplicationHelper::qSlicerApplicationHelper(QObject * parent) : Superclass(parent)
{
}

//----------------------------------------------------------------------------
qSlicerApplicationHelper::~qSlicerApplicationHelper()
{
}

//----------------------------------------------------------------------------
void qSlicerApplicationHelper::setupModuleFactoryManager(qSlicerModuleFactoryManager * moduleFactoryManager)
{
  qSlicerApplication* app = qSlicerApplication::application();
  // Register module factories
  moduleFactoryManager->registerFactory(new qSlicerCoreModuleFactory);

  qSlicerCommandOptions* options = qSlicerApplication::application()->commandOptions();
  if (!options->disableLoadableModules() && !options->runPythonAndExit())
    {
    moduleFactoryManager->registerFactory(new qSlicerLoadableModuleFactory);
    QString loadablePath = app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR + "/";
    moduleFactoryManager->addSearchPath(loadablePath);
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    moduleFactoryManager->addSearchPath(loadablePath + app->intDir());
    }

#ifdef Slicer_USE_PYTHONQT
  if (!options->disableScriptedLoadableModules() &&
      !qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython) &&
      !options->runPythonAndExit())
    {
    moduleFactoryManager->registerFactory(
      new qSlicerScriptedLoadableModuleFactory);
    QString scriptedPath = app->slicerHome() + "/" + Slicer_QTSCRIPTEDMODULES_LIB_DIR + "/";
    moduleFactoryManager->addSearchPath(scriptedPath);
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    moduleFactoryManager->addSearchPath(scriptedPath + app->intDir());
    }
#endif

  QSettings settings;
#ifdef Slicer_BUILD_CLI_SUPPORT
  if (!options->disableCLIModules() && !options->runPythonAndExit())
    {
    QString tempDirectory =
      qSlicerCoreApplication::application()->coreCommandOptions()->tempDirectory();
    bool preferExecutableCLIs =
      settings.value("Modules/PreferExecutableCLI", false).toBool();
    moduleFactoryManager->registerFactory(
      new qSlicerCLILoadableModuleFactory(tempDirectory), preferExecutableCLIs ? 0 : 1);
    // Option to prefer executable CLIs to limit memory consumption.
    moduleFactoryManager->registerFactory(
      new qSlicerCLIExecutableModuleFactory(tempDirectory), preferExecutableCLIs ? 1 : 0);
    QString cliPath = app->slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR + "/";
    moduleFactoryManager->addSearchPath(cliPath);
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    moduleFactoryManager->addSearchPath(cliPath + app->intDir());
#ifdef Q_OS_MAC
    moduleFactoryManager->addSearchPath(app->slicerHome() + "/" + Slicer_CLIMODULES_SUBDIR);
#endif
    }
#endif
  moduleFactoryManager->addSearchPaths(
    settings.value("Modules/AdditionalPaths").toStringList());
  moduleFactoryManager->setModulesToIgnore(
    settings.value("Modules/IgnoreModules").toStringList());
  moduleFactoryManager->setVerboseModuleDiscovery(app->commandOptions()->verboseModuleDiscovery());
}

