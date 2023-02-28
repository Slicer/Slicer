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

// For:
//  - Slicer_QTLOADABLEMODULES_LIB_DIR
//  - Slicer_USE_PYTHONQT
#include "vtkSlicerConfigure.h"

// Slicer includes
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerUtils.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactoryItem::qSlicerLoadableModuleFactoryItem() = default;

#include "qSlicerCorePythonManager.h"
namespace
{

//-----------------------------------------------------------------------------
bool importModulePythonExtensions(
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
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerLoadableModuleFactoryItem::instanciator()
{
  qSlicerAbstractCoreModule * module =
      ctkFactoryPluginItem<qSlicerAbstractCoreModule>::instanciator();
  module->setPath(this->path());

  qSlicerCoreApplication * app = qSlicerCoreApplication::application();

#ifdef Slicer_USE_PYTHONQT
  if (app && !qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // By convention, if the module is not embedded,
    // "<MODULEPATH>/Python" will be appended to PYTHONPATH
    if (!importModulePythonExtensions(
          app->corePythonManager(), app->intDir(), this->path(),
          app->isEmbeddedModule(this->path())))
      {
      qWarning() << "qSlicerLoadableModule::setup - Failed to instantiate module" << module->name() << "python extensions";
      }
    }
#endif

  module->setInstalled(qSlicerUtils::isPluginInstalled(this->path(), app->slicerHome()));
  module->setBuiltIn(qSlicerUtils::isPluginBuiltIn(this->path(), app->slicerHome(), app->revision()));

  return module;
}

//-----------------------------------------------------------------------------
class qSlicerLoadableModuleFactoryPrivate
{
public:
  /// Return a list of module paths
  QStringList modulePaths() const;
};

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleFactoryPrivate Methods

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableModuleFactoryPrivate::modulePaths() const
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  Q_ASSERT(app);
  Q_ASSERT(!app->slicerHome().isEmpty());

  QStringList defaultQTModulePaths;

#ifdef Slicer_BUILD_QTLOADABLEMODULES
  bool appendDefaultQTModulePaths = true;
#else
  bool appendDefaultQTModulePaths = app->isInstalled();
#endif
  if (appendDefaultQTModulePaths)
    {
    defaultQTModulePaths << app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR;
    if (!app->intDir().isEmpty())
      {
      // On Win32, *both* paths have to be there, since scripts are installed
      // in the install location, and exec/libs are *automatically* installed
      // in intDir.
      defaultQTModulePaths << app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR + "/" + app->intDir();
      }
    }

  QSettings * settings = app->revisionUserSettings();
  QStringList additionalModulePaths = app->toSlicerHomeAbsolutePaths(settings->value("Modules/AdditionalPaths").toStringList());
  QStringList qtModulePaths =  additionalModulePaths + defaultQTModulePaths;

  //qDebug() << "qtModulePaths:" << qtModulePaths;

  return qtModulePaths;
}

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleFactory Methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactory::qSlicerLoadableModuleFactory()
  : d_ptr(new qSlicerLoadableModuleFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactory::~qSlicerLoadableModuleFactory() = default;

//-----------------------------------------------------------------------------
void qSlicerLoadableModuleFactory::registerItems()
{
  Q_D(qSlicerLoadableModuleFactory);

  this->registerAllFileItems(d->modulePaths());
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleFactory::fileNameToKey(const QString& fileName)const
{
  return qSlicerLoadableModuleFactory::extractModuleName(fileName);
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleFactory::extractModuleName(const QString& libraryName)
{
  return qSlicerUtils::extractModuleNameFromLibraryName(libraryName);
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactoryItem* qSlicerLoadableModuleFactory::createFactoryFileBasedItem()
{
  return new qSlicerLoadableModuleFactoryItem();
}

//-----------------------------------------------------------------------------
bool qSlicerLoadableModuleFactory::isValidFile(const QFileInfo& file)const
{
  if (!Superclass::isValidFile(file))
    {
    return false;
    }
  return qSlicerUtils::isLoadableModule(file.absoluteFilePath());
}
