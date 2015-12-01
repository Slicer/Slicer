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
#include <QBitArray>
#include <QSettings>

// CTK includes
#include <ctkVTKPythonQtWrapperFactory.h>

// PythonQt includes
#include <PythonQt.h>

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "qSlicerUtils.h"
#include "qSlicerCorePythonManager.h"
#include "qSlicerScriptedUtils_p.h"
#include "vtkSlicerConfigure.h"

// VTK includes
#include <vtkPythonUtil.h>
#include <vtkVersion.h>

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::qSlicerCorePythonManager(QObject* _parent)
  : Superclass(_parent)
{
  this->Factory = 0;
  int flags = this->initializationFlags();
  flags &= ~(PythonQt::IgnoreSiteModule); // Clear bit
  this->setInitializationFlags(flags);
}

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::~qSlicerCorePythonManager()
{
  if (this->Factory)
    {
    delete this->Factory;
    this->Factory = 0;
    }
}

//-----------------------------------------------------------------------------
QStringList qSlicerCorePythonManager::pythonPaths()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (!app)
    {
    return Superclass::pythonPaths();
    }

  QStringList paths;
  paths << Superclass::pythonPaths();
  paths << app->slicerHome() + "/" Slicer_BIN_DIR "/" + app->intDir();
  paths << app->slicerHome() + "/" Slicer_BIN_DIR "/Python";

  paths << QSettings().value("Python/AdditionalPythonPaths").toStringList();
  paths << app->slicerHome() + "/" Slicer_LIB_DIR;

#ifdef Slicer_BUILD_QTLOADABLEMODULES
  bool appendQtLoadableModulePythonPaths = true;
#else
  bool appendQtLoadableModulePythonPaths = app->isInstalled();
#endif
  if (appendQtLoadableModulePythonPaths)
    {
    paths << app->slicerHome() + "/" Slicer_QTLOADABLEMODULES_LIB_DIR;
    paths << app->slicerHome() + "/" Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR;
    }

#ifdef Slicer_BUILD_QTSCRIPTEDMODULES
  bool appendQtScriptedModulePythonPaths = true;
#else
  bool appendQtScriptedModulePythonPaths = app->isInstalled();
#endif
  if(appendQtScriptedModulePythonPaths)
    {
    paths << app->slicerHome() + "/" Slicer_QTSCRIPTEDMODULES_LIB_DIR;
    }

  QString executableExtension = qSlicerUtils::executableExtension();
  if (!app->isInstalled())
    {
    // Add here python path specific to the BUILD tree
#if defined(Q_WS_WIN)
      QString vtkLibSubDir("bin");
#else
      QString vtkLibSubDir("lib");
#endif
#ifdef CMAKE_INTDIR
    paths << VTK_DIR "/" + vtkLibSubDir + "/" CMAKE_INTDIR "/";
#else
    paths << VTK_DIR "/" + vtkLibSubDir + "/";
#endif
    paths << QString("%1/Wrapping/Python").arg(VTK_DIR);
#ifdef CMAKE_INTDIR
    paths << CTK_DIR "/CTK-build/bin/" CMAKE_INTDIR "/";
#else
    paths << CTK_DIR "/CTK-build/bin/";
#endif
    paths << QString("%1/CTK-build/bin/Python").arg(CTK_DIR);
    }
  else
    {
    // Add here python path specific to the INSTALLED tree
#if defined(Q_WS_WIN)
    QString pythonLibSubDirectory("/Lib");
    paths << app->slicerHome() + "/lib/Python" + pythonLibSubDirectory;
    paths << app->slicerHome() + "/lib/Python" + pythonLibSubDirectory + "/lib-dynload";
    paths << app->slicerHome() + "/lib/Python" + pythonLibSubDirectory + "/lib-tk";
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
    // On unix-like system, setting PYTHONHOME is enough to have the following path automatically
    // appended to PYTHONPATH: ../lib/pythonX.Y.zip, ../lib/pythonX.Y/,
    // and ../lib/pythonX.Y/{lib-tk, lib-old, lib-dynload}
    // See http://docs.python.org/c-api/intro.html#embedding-python
    QString pythonLibSubDirectory("/lib/python" Slicer_PYTHON_VERSION_DOT);
#endif
    paths << app->slicerHome() + "/lib/Python" + pythonLibSubDirectory + "/site-packages";
    }

  return paths;
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::preInitialization()
{
  Superclass::preInitialization();
  this->Factory = new ctkVTKPythonQtWrapperFactory;
  this->addWrapperFactory(this->Factory);
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (app)
    {
    // Add object to python interpreter context
    this->addObjectToPythonMain("_qSlicerCoreApplicationInstance", app);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::addVTKObjectToPythonMain(const QString& name, vtkObject * object)
{
  // Split name using '.'
  QStringList moduleNameList = name.split('.', QString::SkipEmptyParts);

  // Remove the last part
  QString attributeName = moduleNameList.takeLast();

  bool success = qSlicerScriptedUtils::setModuleAttribute(
        moduleNameList.join("."),
        attributeName,
        vtkPythonUtil::GetObjectFromPointer(object));
  if (!success)
    {
    qCritical() << "qSlicerCorePythonManager::addVTKObjectToPythonMain - "
                   "Failed to add VTK object:" << name;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::appendPythonPath(const QString& path)
{
  // TODO Make sure PYTHONPATH is updated
  this->executeString(QString("import sys; sys.path.append('%1'); del sys").arg(path));
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::appendPythonPaths(const QStringList& paths)
{
  foreach(const QString& path, paths)
    {
    this->appendPythonPath(path);
    }
}
