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

#ifdef Slicer_USE_PYTHONQT
# include "qSlicerScriptedUtils_p.h" // For importModulePythonExtensions
#endif

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerLoadableModuleFactoryItem::instanciator()
{
  qSlicerAbstractCoreModule* module = ctkFactoryPluginItem<qSlicerAbstractCoreModule>::instanciator();
  module->setPath(this->path());

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (!app)
  {
    return nullptr;
  }

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    // By convention, if the module is not embedded,
    // "<MODULEPATH>/Python" will be appended to PYTHONPATH
    if (!qSlicerScriptedUtils::importModulePythonExtensions(
          app->corePythonManager(), app->intDir(), this->path(), app->isEmbeddedModule(this->path())))
    {
      qWarning() << "qSlicerLoadableModuleFactory - Failed to instantiate module" << module->name()
                 << "python extensions";
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
  if (!app)
  {
    return QStringList();
  }
  if (app->slicerHome().isEmpty())
  {
    qCritical() << Q_FUNC_INFO << ": Application home directory is expected to be set";
    return QStringList();
  }

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

  QSettings* settings = app->revisionUserSettings();
  QStringList additionalModulePaths =
    app->toSlicerHomeAbsolutePaths(settings->value("Modules/AdditionalPaths").toStringList());
  QStringList qtModulePaths = additionalModulePaths + defaultQTModulePaths;

  // qDebug() << "qtModulePaths:" << qtModulePaths;

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
QString qSlicerLoadableModuleFactory::fileNameToKey(const QString& fileName) const
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
bool qSlicerLoadableModuleFactory::isValidFile(const QFileInfo& file) const
{
  if (!Superclass::isValidFile(file))
  {
    return false;
  }
  return qSlicerUtils::isLoadableModule(file.absoluteFilePath());
}
