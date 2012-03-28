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

// SlicerQt includes
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerUtils.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCorePythonManager.h"
#endif

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactoryItem::qSlicerLoadableModuleFactoryItem()
{

}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerLoadableModuleFactoryItem::instanciator()
{
  qSlicerAbstractCoreModule * module =
      ctkFactoryPluginItem<qSlicerAbstractCoreModule>::instanciator();
  module->setPath(this->path());

  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  module->setInstalled(qSlicerUtils::isPluginInstalled(this->path(), app->slicerHome()));

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // By convention, if the module is not embedded,
    // "<MODULEPATH>/Python" will be appended to PYTHONPATH
    if (!qSlicerCoreApplication::application()->isEmbeddedModule(module->path()))
      {
      QDir modulePathWithoutIntDir = QFileInfo(module->path()).dir();
      QString intDir = qSlicerCoreApplication::application()->intDir();
      if (intDir ==  modulePathWithoutIntDir.dirName())
        {
        modulePathWithoutIntDir.cdUp();
        }
      QString pythonPath = modulePathWithoutIntDir.filePath("Python");
      QStringList paths; paths << QFileInfo(module->path()).dir().absolutePath() << pythonPath;
      qSlicerCorePythonManager * pythonManager = qSlicerCoreApplication::application()->corePythonManager();
      pythonManager->appendPythonPaths(paths);
      // Update current application directory, so that *PythonD modules can be loaded
      ctkScopedCurrentDir scopedCurrentDir(modulePathWithoutIntDir.absolutePath());
      pythonManager->executeString(QString(
            "from slicer.util import importVTKClassesFromDirectory;"
            "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer*ModuleLogic.py');"
            "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer*ModuleMRML.py');"
            ).arg(pythonPath));
      pythonManager->executeString(QString(
            "from slicer.util import importQtClassesFromDirectory;"
            "importQtClassesFromDirectory('%1', 'slicer.modulewidget', filematch='qSlicer*PythonQt.*');"
            ).arg(modulePathWithoutIntDir.absolutePath()));
      }
    }
#endif
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

  QStringList additionalModulePaths = QSettings().value("Modules/AdditionalPaths").toStringList();
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
qSlicerLoadableModuleFactory::~qSlicerLoadableModuleFactory()
{
}

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
