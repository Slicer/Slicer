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

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// Qt includes
#include <QDebug>
#ifdef Slicer_USE_PYTHONQT
# include <QDir>
# include <QVariant>
#endif

// CTK includes
#ifdef Slicer_USE_PYTHONQT
# include <ctkScopedCurrentDir.h>
#endif

// Slicer includes
#include "qSlicerLoadableModule.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCoreApplication.h"
# include "qSlicerCorePythonManager.h"
#endif

//-----------------------------------------------------------------------------
class qSlicerLoadableModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerLoadableModule::qSlicerLoadableModule(QObject* _parentObject)
  : Superclass(_parentObject)
  , d_ptr(new qSlicerLoadableModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModule::~qSlicerLoadableModule() = default;

//-----------------------------------------------------------------------------
bool qSlicerLoadableModule::importModulePythonExtensions(
    qSlicerCorePythonManager * pythonManager,
    const QString& intDir,const QString& modulePath,
    bool isEmbedded)
{
  Q_UNUSED(intDir);
#ifdef Slicer_USE_PYTHONQT
  if(!pythonManager)
    {
    return false;
    }

  QString pythonModuleDir = QFileInfo(modulePath).absoluteFilePath();
  if (!QFileInfo(pythonModuleDir).isDir())
    {
    pythonModuleDir = QFileInfo(pythonModuleDir).absolutePath();
    }

  // Update current application directory, so that *PythonD modules can be loaded
  ctkScopedCurrentDir scopedCurrentDir(pythonModuleDir);

  if (!isEmbedded)
    {
    QStringList paths; paths << scopedCurrentDir.currentPath();
    pythonManager->appendPythonPaths(paths);
    }

  pythonManager->executeString(QString(
        "from slicer.util import importVTKClassesFromDirectory;"
        "importVTKClassesFromDirectory(%1, 'slicer', filematch='vtkSlicer*ModuleLogicPython.*');"
        "importVTKClassesFromDirectory(%1, 'slicer', filematch='vtkSlicer*ModuleMRMLPython.*');"
        "importVTKClassesFromDirectory(%1, 'slicer', filematch='vtkSlicer*ModuleMRMLDisplayableManagerPython.*');"
        "importVTKClassesFromDirectory(%1, 'slicer', filematch='vtkSlicer*ModuleVTKWidgetsPython.*');"
        ).arg(qSlicerCorePythonManager::toPythonStringLiteral(scopedCurrentDir.currentPath())));
  pythonManager->executeString(QString(
        "from slicer.util import importQtClassesFromDirectory;"
        "importQtClassesFromDirectory(%1, 'slicer', filematch='qSlicer*PythonQt.*');"
        ).arg(qSlicerCorePythonManager::toPythonStringLiteral(scopedCurrentDir.currentPath())));
  return !pythonManager->pythonErrorOccured();
#else
  Q_UNUSED(isEmbedded);
  Q_UNUSED(modulePath);
  Q_UNUSED(pythonManager);
  return false;
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerLoadableModule::addModuleToSlicerModules(
    qSlicerCorePythonManager * pythonManager,
    qSlicerAbstractModule * module,
    const QString& moduleName)
{
#ifdef Slicer_USE_PYTHONQT
  if(!pythonManager || !module || moduleName.isEmpty())
    {
    return false;
    }
  pythonManager->addObjectToPythonMain("_tmp_module_variable", module);
  pythonManager->executeString(
        QString("import __main__;"
                "setattr( slicer.modules, '%1', __main__._tmp_module_variable);"
                "del __main__._tmp_module_variable").arg(
          moduleName.toLower()));
  return !pythonManager->pythonErrorOccured();
#else
  Q_UNUSED(pythonManager);
  Q_UNUSED(module);
  Q_UNUSED(moduleName);
  return false;
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerLoadableModule::addModuleNameToSlicerModuleNames(
    qSlicerCorePythonManager * pythonManager,
    const QString& moduleName)
{
#ifdef Slicer_USE_PYTHONQT
  if(!pythonManager || moduleName.isEmpty())
    {
    return false;
    }
  pythonManager->executeString(
        QString("import __main__;"
                "setattr( slicer.moduleNames, '%1', '%2')").arg(
          moduleName.toLower()).arg(moduleName));
  return !pythonManager->pythonErrorOccured();
#else
  Q_UNUSED(pythonManager);
  Q_UNUSED(moduleName);
  return false;
#endif
}

//-----------------------------------------------------------------------------
void qSlicerLoadableModule::setup()
{
#ifndef QT_NO_DEBUG
  Q_D(qSlicerLoadableModule);
  // Q_ASSERT(d != 0);
#endif

#ifdef Slicer_USE_PYTHONQT
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (app && !qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // By convention, if the module is not embedded,
    // "<MODULEPATH>/Python" will be appended to PYTHONPATH
    if (!Self::importModulePythonExtensions(
          app->corePythonManager(), app->intDir(), this->path(),
          app->isEmbeddedModule(this->path())))
      {
      qWarning() << "qSlicerLoadableModule::setup - Failed to import module" << this->name() << "python extensions";
      }
    }
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModule::helpText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::helpText() is not implemented";
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModule::acknowledgementText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::acknowledgementText - Not implemented";
  return QString();
}
