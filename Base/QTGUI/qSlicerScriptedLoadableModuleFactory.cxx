/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QDirIterator>
#include <QSettings>
#include <QStringList>

// SlicerQt includes
#include "qSlicerScriptedLoadableModuleFactory.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerScriptedLoadableModule.h"

// For:
//  - Slicer_QTLOADABLEMODULES_LIB_DIR
#include "vtkSlicerConfigure.h"

//----------------------------------------------------------------------------
// ctkFactoryScriptedItem methods

//----------------------------------------------------------------------------
bool ctkFactoryScriptedItem::load()
{
  return true;
}

//----------------------------------------------------------------------------
qSlicerAbstractCoreModule* ctkFactoryScriptedItem::instanciator()
{
  // Using a scoped pointer ensures the memory will be cleaned if instantiator
  // fails before returning the module. See QScopedPointer::take()
  QScopedPointer<qSlicerScriptedLoadableModule> module(new qSlicerScriptedLoadableModule());

  bool ret = module->setPythonSource(this->path());
  if (!ret)
    {
    return 0;
    }

  //qDebug() << "module->category()" << module->category();
  //qDebug() << "module->contributor()" << module->contributor();
  //qDebug() << "module->helpText()" << module->helpText();
  //qDebug() << "module->acknowledgementText()" << module->acknowledgementText();
  module->setPath(this->path());

  return module.take();
}

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleFactoryPrivate

//-----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleFactoryPrivate
{
public:
  ///
  /// Return a list of module paths
  QStringList modulePaths() const;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleFactoryPrivate Methods

//-----------------------------------------------------------------------------
QStringList qSlicerScriptedLoadableModuleFactoryPrivate::modulePaths() const
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  Q_ASSERT(app);

  // slicerHome shouldn't be empty
  Q_ASSERT(!app->slicerHome().isEmpty());
  
  QStringList defaultQTModulePaths;
  defaultQTModulePaths << app->slicerHome() + "/" + Slicer_QTSCRIPTEDMODULES_LIB_DIR;
  if (!app->intDir().isEmpty())
    {
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
    defaultQTModulePaths << app->slicerHome() + "/" + Slicer_QTSCRIPTEDMODULES_LIB_DIR + "/" + app->intDir();
    }

  // Add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QStringList additionalModulePaths = QSettings().value("Modules/AdditionalPaths").toStringList();
  QStringList qtModulePaths = additionalModulePaths + defaultQTModulePaths;
  foreach(const QString& path, qtModulePaths)
    {
    app->addLibraryPath(path);
    }

//   foreach (QString path, app->libraryPaths())
//     {
//     qDebug() << "scriptedModulePaths:" << path;
//     }

//  qDebug() << "scriptedModulePaths:" << qtModulePaths;
  
  return qtModulePaths; 
}

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleFactory Methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleFactory::qSlicerScriptedLoadableModuleFactory()
  : d_ptr(new qSlicerScriptedLoadableModuleFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleFactory::~qSlicerScriptedLoadableModuleFactory()
{
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleFactory::isValidFile(const QFileInfo& file)const
{
  // Skip if current file isn't a python file
  return ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule>::isValidFile(file) &&
    file.suffix().compare("py", Qt::CaseInsensitive) == 0;
}

//----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerScriptedLoadableModuleFactory
::createFactoryFileBasedItem()
{
  return new ctkFactoryScriptedItem();
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleFactory::registerItems()
{
  Q_D(qSlicerScriptedLoadableModuleFactory);
  this->registerAllFileItems(d->modulePaths());
}
