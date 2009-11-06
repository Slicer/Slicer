#include "qSlicerModuleFactory.h" 

#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qCTKAbstractObjectFactory.h"
#include "qCTKAbstractPluginFactory.h"
#include "qCTKAbstractLibraryFactory.h"

#include "qSlicerTransformsModule.h"
#include "qSlicerCamerasModule.h"

#include <QHash>
#include <QDirIterator>
#include <QRegExp>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerModuleFactory::qInternal
{
public:
  typedef QHash<QString, QString>::const_iterator MapConstIterator;
  typedef QHash<QString, QString>::iterator       MapIterator;
  
  typedef qCTKAbstractObjectFactory<qSlicerAbstractModule>  CoreModuleFactoryType;
  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule>  LoadableModuleFactoryType;
  //typedef qCTKAbstractLibraryFactory<qSlicerAbstractModule> CmdLineModuleFactoryType;
  
  qInternal(){}
  
  enum FactoryType 
    {
    LoadableModule = 0x0, 
    CmdLineModule = 0x1,
    };
  
  // Description:
  // Keep track of the mapping between module title and module name
  void updateInternalMaps(const QString& moduleTitle, const QString& moduleName);
  
  // Description:
  // Convenient function used to register a library in the appropriate factory.
  // Either using the loadableModuleFactory or the CmdLineModuleFactory
  void registerLibraries(int factoryType, const QStringList& paths);
  
  // Description:
  // Convenient function used to register a library in the appropriate factory
  void registerLibrary(int factoryType, const QFileInfo& fileInfo);
  
  // Description:
  // Instanciate a module.
  // Note: Try to instanciate a coreModule, if it fails, try to instanciate a loadableModule, 
  // if if fails, attempt to instanciate a commandLineModule
  qSlicerAbstractModule* instanciateModule(const QString& moduleName);
    
  QHash<QString, QString> MapTitleToName;
  QHash<QString, QString> MapNameToTitle;
  QStringList             LoadableModuleSearchPaths;
  QStringList             CmdLineModuleSearchPaths;
  
  //
  CoreModuleFactoryType      CoreModuleFactory;
  LoadableModuleFactoryType  LoadableModuleFactory;
  //CmdLineModuleFactoryType   CmdLineModuleFactory;
};

//-----------------------------------------------------------------------------
qSlicerModuleFactory::qSlicerModuleFactory()
{
  this->Internal = new qInternal;
  
  this->registerCoreModules(); 
}

//-----------------------------------------------------------------------------
qSlicerModuleFactory::~qSlicerModuleFactory()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCoreModules()
{
  this->registerCoreModule<qSlicerTransformsModule>();
  this->registerCoreModule<qSlicerCamerasModule>();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::printAdditionalInfo()
{
  //this->Superclass::printAdditionalInfo();
  qDebug() << "qSlicerModuleFactory (" << this << ")"; 
  qDebug() << "LoadableModuleSearchPaths:" << this->Internal->LoadableModuleSearchPaths; 
  qDebug() << "CmdLineModuleSearchPaths:" << this->Internal->CmdLineModuleSearchPaths; 
  qDebug() << "MapTitleToName";
  qInternal::MapConstIterator iter = this->Internal->MapTitleToName.constBegin();
  while(iter != this->Internal->MapTitleToName.constEnd())
    {
    qDebug() << "Title:" << iter.key() << "-> Name:" << iter.value(); 
    ++iter;
    }
  qDebug() << "MapNameToTitle"; 
  iter = this->Internal->MapNameToTitle.constBegin();
  while(iter != this->Internal->MapNameToTitle.constEnd())
    {
    qDebug() << "Name:" << iter.key() << "-> Title:" << iter.value(); 
    ++iter;
    }
}

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerModuleFactory::registerCoreModule()
{
  // Extract title from class name
  const QString moduleTitle = ClassType::staticModuleTitle();
  QString moduleName = ClassType::staticMetaObject.className(); 

  if (this->Internal->CoreModuleFactory.registerQObject<ClassType>())
    {
    this->Internal->updateInternalMaps(moduleTitle, moduleName);
    qDebug() << "qSlicerModuleFactory::registerCoreModule - title:" << moduleTitle; 
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerLoadableModules()
{
  this->Internal->registerLibraries(qInternal::LoadableModule, 
    this->loadableModuleSearchPaths());
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerLoadableModule(const QFileInfo& fileInfo)
{
  this->Internal->registerLibrary(qInternal::LoadableModule, fileInfo);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModules()
{
  // TODO Not yet implemented
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModule(const QFileInfo& fileInfo)
{
  // TODO Not yet implemented
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::setLoadableModuleSearchPaths(const QStringList& paths)
{
  this->Internal->LoadableModuleSearchPaths.clear();
  this->Internal->LoadableModuleSearchPaths << paths; 
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::loadableModuleSearchPaths()
{
  return this->Internal->LoadableModuleSearchPaths; 
}


//-----------------------------------------------------------------------------
void qSlicerModuleFactory::setCmdLineModuleSearchPaths(const QStringList& paths)
{
  this->Internal->CmdLineModuleSearchPaths.clear();
  this->Internal->CmdLineModuleSearchPaths << paths; 
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::cmdLineModuleSearchPaths()
{
  return this->Internal->CmdLineModuleSearchPaths; 
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::coreModuleNames()
{
  return this->Internal->CoreModuleFactory.names();
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::loadableModuleNames()
{
  return this->Internal->LoadableModuleFactory.names();
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::commandLineModuleNames()
{
  // TODO Not yet implemented
  return QStringList();
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::getModuleTitle(const QString & moduleName)
{
  // Lookup module name
  qInternal::MapConstIterator iter = this->Internal->MapNameToTitle.constFind(moduleName); 
  
  if (iter == this->Internal->MapNameToTitle.constEnd())
    {
    //qCritical() << "Failed to retrieve module title given its name:" << moduleName; 
    return QString(); 
    }
  return iter.value(); 
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::getModuleName(const QString & moduleTitle)
{
  // Lookup module name
  qInternal::MapConstIterator iter = this->Internal->MapTitleToName.constFind(moduleTitle); 
  
  if (iter == this->Internal->MapTitleToName.constEnd())
    {
    //qCritical() << "Failed to retrieve module name given its title:" << moduleTitle; 
    return QString(); 
    }
  return iter.value(); 
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactory::initializeModule(const QString& moduleName)
{
  
  qSlicerAbstractModule* module = this->Internal->instanciateModule(moduleName);
  Q_ASSERT(module);
  if (!module)
    {
    return 0; 
    }
  // Initialize module
  module->initialize();
  
  // Get module title
  QString moduleTitle = this->getModuleTitle(moduleName);
  if (moduleTitle.isEmpty())
    {
    return 0; 
    }
    
  module->setWindowTitle(moduleTitle); 
  
  qDebug() << module << " - title:" << moduleTitle; 
  return module; 
}

//-----------------------------------------------------------------------------
// Internal methods

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactory::qInternal::instanciateModule(const QString& moduleName)
{
  // Try to instanciate a core module
  qSlicerAbstractModule* module = this->CoreModuleFactory.instanciate(moduleName); 
  if (!module)
    {
    // Try to instanciate a loadable module
    module = this->LoadableModuleFactory.instanciate(moduleName);
    if (!module)
      {
      //qCritical() << "Failed to instanciate module:" << moduleName; 
      return 0;
      }
    }
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::qInternal::updateInternalMaps(const QString& moduleTitle, const QString& moduleName)
{
  // Keep track of the relation Title -> moduleName
  this->MapTitleToName[moduleTitle] = moduleName;

  // Keep track of the relation moduleName -> Title
  this->MapNameToTitle[moduleName] = moduleTitle;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::qInternal::registerLibraries(int factoryType, const QStringList& paths)
{
  if (paths.isEmpty())
    {
    qWarning() << "SearchPaths is empty"; 
    return;
    }
  
  foreach (QString path, paths)
    {
    QDirIterator it(path);
    while (it.hasNext())
      {
      it.next();
      if (it.fileInfo().isFile() && it.fileInfo().isExecutable())
        {
        this->registerLibrary(factoryType, it.fileInfo());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::qInternal::registerLibrary(int factoryType, const QFileInfo& fileInfo)
{
  QString libraryName = fileInfo.fileName();
  if (!QLibrary::isLibrary(libraryName))
    {
    qDebug() << "Failed to register:" << libraryName << " - Library name incorrect."; 
    return;
    }
  
  if (factoryType == qInternal::LoadableModule)
    {
    if (this->LoadableModuleFactory.registerLibrary(libraryName, fileInfo.filePath()))
      {
      // Instanciate the object and retrieve nodule title
      qSlicerAbstractModule* module = this->instanciateModule(libraryName);
      Q_ASSERT(module);
      this->updateInternalMaps(module->moduleTitle(), libraryName);
      qDebug() << "qSlicerModuleFactory::registerLibrary - title:" << module->moduleTitle(); 
      }
    }
  else if (factoryType == qInternal::CmdLineModule)
    {
    // TODO  
    }
}
