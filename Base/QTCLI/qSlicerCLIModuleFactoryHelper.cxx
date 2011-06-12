/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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

// SlicerQT includes
#include "qSlicerCoreApplication.h"

// For:
//  - Slicer_PLUGINS_BIN_DIR
#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
const QStringList qSlicerCLIModuleFactoryHelper::modulePaths()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  Q_ASSERT(app);

  // slicerHome shouldn't be empty
  Q_ASSERT(!app->slicerHome().isEmpty());

  QStringList defaultCmdLineModulePaths;

  defaultCmdLineModulePaths << app->slicerHome() + "/" + Slicer_PLUGINS_BIN_DIR;
  if (!app->intDir().isEmpty())
     {
     // On Win32, *both* paths have to be there, since scripts are installed
     // in the install location, and exec/libs are *automatically* installed
     // in intDir.
     defaultCmdLineModulePaths << app->slicerHome() + "/" + Slicer_PLUGINS_BIN_DIR + "/" + app->intDir();
     }

  QStringList additionalModulePaths = QSettings().value("Modules/AdditionalPaths").toStringList();
  QStringList cmdLineModulePaths = additionalModulePaths + defaultCmdLineModulePaths;
  foreach(const QString& path, cmdLineModulePaths)
    {
    app->addLibraryPath(path);
    }
  return cmdLineModulePaths; 
}
