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
#include <QStringList>
#include <QDirIterator>

// SlicerQt includes
#include "qSlicerScriptedLoadableModuleFactory.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerScriptedLoadableModule.h"

// For:
//  - Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR
#include "vtkSlicerConfigure.h"

//----------------------------------------------------------------------------
// ctkFactoryScriptedItem methods

//----------------------------------------------------------------------------
ctkFactoryScriptedItem::ctkFactoryScriptedItem(const QString& path):
    ctkAbstractFactoryFileBasedItem<qSlicerAbstractCoreModule>(path)
{

}

//----------------------------------------------------------------------------
bool ctkFactoryScriptedItem::load()
{
  return true;
}

//----------------------------------------------------------------------------
qSlicerAbstractCoreModule* ctkFactoryScriptedItem::instanciator()
{
  // Using a scoped pointer ensures the memory will be cleaned if instanciator
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
  
  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  QStringList defaultQTModulePaths;
  defaultQTModulePaths << app->slicerHome() + "/"
                                             + Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR;

   if (!app->intDir().isEmpty())
     {
     defaultQTModulePaths << app->slicerHome() + "/" + Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR + "/" + app->intDir();
     }

  // Add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QStringList qtModulePaths = /*userModulePaths + PathSep + */defaultQTModulePaths;
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

//----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleFactory::registerScript(const QString& key, const QFileInfo& file)
{
  // Check if already registered
  if (this->item(key))
    {
    return false;
    }
  QSharedPointer<ctkFactoryScriptedItem> _item =
    QSharedPointer<ctkFactoryScriptedItem>(new ctkFactoryScriptedItem(file.filePath()));
  if (_item.isNull())
    {
    return false;
    }

  _item->setVerbose(this->verbose());
  return this->registerItem(key, _item);
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleFactory::registerItems()
{
  Q_D(qSlicerScriptedLoadableModuleFactory);

  QStringList modulePaths = d->modulePaths();
  
  if (modulePaths.isEmpty())
    {
    qWarning() << "No scripted module paths provided";
    return;
    }

  // Process one path at a time
  foreach (QString path, modulePaths)
    {
    QDirIterator it(path);
    while (it.hasNext())
      {
      it.next();
      QFileInfo fileInfo = it.fileInfo();
      //qDebug() << "Verifying scripted loadable module:" << fileInfo.fileName();
      
      // Skip if item isn't a file
      if (!fileInfo.isFile()) { continue; }
      
      if (fileInfo.isSymLink())
        {
        // symLinkTarget() handles links pointing to symlinks.
        // How about a symlink pointing to a symlink ?
        fileInfo = QFileInfo(fileInfo.symLinkTarget());
        }
      // Skip if current file isn't a python file
      if (fileInfo.suffix().compare("py", Qt::CaseInsensitive) != 0) { continue; }

      if (this->verbose())
        {
        qDebug() << "Attempt to register loadable module:" << fileInfo.fileName();
        }

      QString moduleName = this->fileNameToKey(fileInfo.fileName());
      if (!this->registerScript(moduleName, fileInfo))
        {
        if (this->verbose())
          {
          qDebug() << "Failed to register module: " << moduleName;
          }
        continue;
        }
      }
    }
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModuleFactory::fileNameToKey(const QString& fileName)
{
  return qSlicerScriptedLoadableModuleFactory::extractModuleName(fileName);
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModuleFactory::extractModuleName(const QString& fileName)
{
  return QFileInfo(fileName).baseName().toLower();
}

