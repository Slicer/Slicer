#include "qSlicerModuleFactory.h"

#include "qSlicerAbstractModule.h"
#include "qSlicerCLIModule.h"
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
struct qSlicerModuleFactory::qInternal
{
  typedef QHash<QString, QString>::const_iterator MapConstIterator;
  typedef QHash<QString, QString>::iterator       MapIterator;

  typedef QHash<QString, int>::const_iterator     Map2ConstIterator;
  typedef QHash<QString, int>::iterator           Map2Iterator;

  class qSlicerFactoryLoadableCmdLineModuleItem;
  class qSlicerFactoryExecutableCmdLineModuleItem;

  typedef qCTKAbstractObjectFactory<qSlicerAbstractModule>  CoreModuleFactoryType;
  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule>  LoadableModuleFactoryType;
  typedef qCTKAbstractLibraryFactory<qSlicerAbstractModule, qSlicerFactoryLoadableCmdLineModuleItem> CmdLineLoadableModuleFactoryType;
  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule, qSlicerFactoryExecutableCmdLineModuleItem> CmdLineExecutableModuleFactoryType;

  // List of existing factory
  enum FactoryType
    {
    CoreModule              = 0x0,
    LoadableModule          = 0x1,
    CmdLineLoadableModule   = 0x2,
    CmdLineExecutableModule = 0x3,
    CmdLinePythonModule     = 0x4,
    };

  qInternal()
    {
    // Set the list of required symbols for CmdLineLoadableModule,
    // if one of these symbols can't be resolved, the library won't be registered.
    QStringList cmdLineModuleSymbols;
    cmdLineModuleSymbols << "XMLModuleDescription";
    this->CmdLineLoadableModuleFactory.setSymbols(cmdLineModuleSymbols);
    }

  // Description:
  // Keep track of the mapping between module title and module name
  void updateInternalMaps(const QString& moduleTitle, const QString& moduleName);

  // Description:
  // Convenient function used to register a library in the appropriate factory.
  // Either using the LoadableModuleFactory or the CmdLineLoadableModuleFactory
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
  CoreModuleFactoryType              CoreModuleFactory;
  LoadableModuleFactoryType          LoadableModuleFactory;
  CmdLineLoadableModuleFactoryType   CmdLineLoadableModuleFactory;

  QHash<QString, int> MapModuleNameToFactoryType;
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

    // Keep track of the factory type associated with the module
    this->Internal->MapModuleNameToFactoryType[moduleName] = qInternal::CoreModule;

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
  this->Internal->registerLibraries(qInternal::CmdLineLoadableModule,
    this->cmdLineModuleSearchPaths());
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModule(const QFileInfo& fileInfo)
{
  this->Internal->registerLibrary(qInternal::CmdLineLoadableModule, fileInfo);
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
  return this->Internal->CmdLineLoadableModuleFactory.names();
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
  // Retrieve the factoryType associated with the module
  qInternal::Map2ConstIterator iter = this->MapModuleNameToFactoryType.constFind(moduleName);

  qSlicerAbstractModule* module = 0;

  Q_ASSERT(iter != this->MapModuleNameToFactoryType.constEnd());
  if (iter == this->MapModuleNameToFactoryType.constEnd())
    {
    qWarning() << "Failed to retrieve factory type for module:" << moduleName;
    return 0;
    }

  int factoryType = iter.value();

  if (factoryType == CoreModule)
    {
    // Try to instanciate a core module
    module = this->CoreModuleFactory.instanciate(moduleName);
    }
  else if (factoryType == LoadableModule)
    {
    // Try to instanciate a loadable module
    module = this->LoadableModuleFactory.instanciate(moduleName);
    }
  else if (factoryType == CmdLineLoadableModule)
    {
    // Try to instanciate a loadable command line module
    module = this->CmdLineLoadableModuleFactory.instanciate(moduleName);
    }
  else if (factoryType == CmdLineExecutableModule)
    {
    // Try to instanciate an executable command line module
    qDebug() << "CmdLineExecutableModule - instanciate - Not implemented";
    }
  else if (factoryType == CmdLinePythonModule)
    {
    // Try to instanciate a python command line module
    qDebug() << "CmdLinePythonModule - instanciate - Not implemented";
    }

  if (!module)
    {
    //qCritical() << "Failed to instanciate module:" << moduleName;
    }
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::qInternal::updateInternalMaps(
  const QString& moduleTitle, const QString& moduleName)
{
  // Sanity checks
  Q_ASSERT(!moduleTitle.isEmpty() && !moduleName.isEmpty());

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
  qDebug() << "Attempt to register library" << fileInfo.filePath();
  if (!QLibrary::isLibrary(libraryName))
    {
    //qDebug() << "-->Skiped";
    return;
    }

  if (factoryType == qInternal::LoadableModule)
    {
    if (this->LoadableModuleFactory.registerLibrary(libraryName, fileInfo.filePath()))
      {
      // Keep track of the factory type associated with the module
      this->MapModuleNameToFactoryType[libraryName] = factoryType;

      // Instanciate the object and retrieve nodule title
      qSlicerAbstractModule* module = this->instanciateModule(libraryName);
      Q_ASSERT(module);
      this->updateInternalMaps(module->moduleTitle(), libraryName);
      qDebug() << "qSlicerModuleFactory::registerLoadableModule - title:" << module->moduleTitle();
      }
    }
  else if (factoryType == qInternal::CmdLineLoadableModule)
    {
    if (this->CmdLineLoadableModuleFactory.registerLibrary(libraryName, fileInfo.filePath()))
      {
      // Keep track of the factory type associated with the module
      this->MapModuleNameToFactoryType[libraryName] = factoryType;

      // Instanciate the object and retrieve nodule title
      qSlicerAbstractModule* module = this->instanciateModule(libraryName);
      Q_ASSERT(module);
      module->initialize();
      this->updateInternalMaps(module->moduleTitle(), libraryName);
      qDebug() << "qSlicerModuleFactory::registerCmdLineModule - title:" << module->moduleTitle();
      }
    }
}

//-----------------------------------------------------------------------------
// Internal classes

//-----------------------------------------------------------------------------
class qSlicerModuleFactory::qInternal::qSlicerFactoryLoadableCmdLineModuleItem :
  public qCTKFactoryLibraryItem<qSlicerAbstractModule>
{
public:
  qSlicerFactoryLoadableCmdLineModuleItem(const QString& key, const QString& path):
    qCTKFactoryLibraryItem<qSlicerAbstractModule>(key,path){}

protected:
  // To make the cast code cleaner to read.
  //typedef char * (*XMLModuleDescriptionFunction)();
  //typedef int (*ModuleEntryPoint)(int argc, char* argv[]);

  virtual qSlicerAbstractModule* instanciator()
    {
    qSlicerCLIModule * module = new qSlicerCLIModule();

    // Resolve symbols
    char* xmlDescription = (char*)this->symbolAddress("XMLModuleDescription");

    // Retrieve
    //if (!xmlDescription) { xmlDescription = xmlFunction ? (*xmlFunction)() : 0; }

    if (!xmlDescription)
      {
      qWarning() << "Failed to retrieve Xml Description - Path:" << this->path();
      delete module; // Clean memory
      return 0;
      }
    module->setXmlModuleDescription(xmlDescription);
    return module;
    }
};

//-----------------------------------------------------------------------------
class qSlicerModuleFactory::qInternal::qSlicerFactoryExecutableCmdLineModuleItem :
  public qCTKAbstractFactoryItem<qSlicerAbstractModule>
{
public:
  qSlicerFactoryExecutableCmdLineModuleItem(const QString& key, const QString& path):
    qCTKAbstractFactoryItem<qSlicerAbstractModule>(key),Path(path){}

  virtual bool load()
    {
    return false;
    }
  QString path() { return this->Path; }

protected:
  virtual qSlicerAbstractModule* instanciator()
    {
    qDebug() << "CmdLineExecutableModuleItem::instanciate - name:" << this->path();

    return 0;
    }

private:
  QString          Path;
};
