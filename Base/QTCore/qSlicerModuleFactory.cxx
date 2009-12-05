#include "qSlicerModuleFactory.h"

// SlicerQT includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCoreApplication.h"

// SlicerQTModules includes
#include "qSlicerTransformsModule.h"
#include "qSlicerCamerasModule.h"

// qCTK includes
#include <qCTKAbstractQObjectFactory.h>
#include <qCTKAbstractPluginFactory.h>
#include <qCTKAbstractLibraryFactory.h>

// QT includes
#include <QHash>
#include <QDirIterator>
#include <QRegExp>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerModuleFactoryPrivate : public qCTKPrivate<qSlicerModuleFactory>
{
  typedef qSlicerModuleFactoryPrivate Self;
  
  typedef QHash<QString, QString>::const_iterator MapConstIterator;
  typedef QHash<QString, QString>::iterator       MapIterator;

  typedef QHash<QString, int>::const_iterator     Map2ConstIterator;
  typedef QHash<QString, int>::iterator           Map2Iterator;

  class qSlicerFactoryLoadableCmdLineModuleItem;
  class qSlicerFactoryExecutableCmdLineModuleItem;

  typedef qCTKAbstractQObjectFactory<qSlicerAbstractModule>  CoreModuleFactoryType;
  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule>  LoadableModuleFactoryType;
  typedef qCTKAbstractLibraryFactory
    <qSlicerAbstractModule,qSlicerFactoryLoadableCmdLineModuleItem> CmdLineLoadableModuleFactoryType;
  typedef qCTKAbstractPluginFactory
    <qSlicerAbstractModule, qSlicerFactoryExecutableCmdLineModuleItem> CmdLineExecutableModuleFactoryType;

  // List of available factory
  enum FactoryType
    {
    CoreModule              = 0x0,
    LoadableModule          = 0x1,
    CmdLineLoadableModule   = 0x2,
    CmdLineExecutableModule = 0x3,
    CmdLinePythonModule     = 0x4,
    };

  qSlicerModuleFactoryPrivate()
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
  // If instantiate is true, instantiate a module. Otherwise uninstantiate it.
  qSlicerAbstractModule* instantiateModule(const QString& moduleName, bool instantiate);

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
  QCTK_INIT_PRIVATE(qSlicerModuleFactory);
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
  QCTK_D(qSlicerModuleFactory);
  
  //this->Superclass::printAdditionalInfo();
  qDebug() << "qSlicerModuleFactory (" << this << ")";
  qDebug() << "LoadableModuleSearchPaths:" << d->LoadableModuleSearchPaths;
  qDebug() << "CmdLineModuleSearchPaths:" << d->CmdLineModuleSearchPaths;
  qDebug() << "MapTitleToName";
  qSlicerModuleFactoryPrivate::MapConstIterator iter = d->MapTitleToName.constBegin();
  while(iter != d->MapTitleToName.constEnd())
    {
    qDebug() << "Title:" << iter.key() << "-> Name:" << iter.value();
    ++iter;
    }
  qDebug() << "MapNameToTitle";
  iter = d->MapNameToTitle.constBegin();
  while(iter != d->MapNameToTitle.constEnd())
    {
    qDebug() << "Name:" << iter.key() << "-> Title:" << iter.value();
    ++iter;
    }
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::coreModuleNames() const
{
  return const_cast<qSlicerModuleFactoryPrivate*>(qctk_d())->CoreModuleFactory.names();
}

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerModuleFactory::registerCoreModule()
{
  QCTK_D(qSlicerModuleFactory);
  
  // Extract title from class name
  const QString moduleTitle = ClassType::staticTitle();
  QString moduleName = ClassType::staticMetaObject.className();

  if (d->CoreModuleFactory.registerQObject<ClassType>())
    {
    d->updateInternalMaps(moduleTitle, moduleName);

    // Keep track of the factory type associated with the module
    d->MapModuleNameToFactoryType[moduleName] = qSlicerModuleFactoryPrivate::CoreModule;

    qDebug() << "qSlicerModuleFactory::registerCoreModule - title:" << moduleTitle;
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerLoadableModules()
{ 
  qctk_d()->registerLibraries(qSlicerModuleFactoryPrivate::LoadableModule,
    this->loadableModuleSearchPaths());
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerLoadableModule(const QFileInfo& fileInfo)
{
  qctk_d()->registerLibrary(qSlicerModuleFactoryPrivate::LoadableModule, fileInfo);
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::loadableModuleNames() const
{
  return const_cast<qSlicerModuleFactoryPrivate*>(qctk_d())->LoadableModuleFactory.names();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleFactory, QStringList, loadableModuleSearchPaths,
                                                LoadableModuleSearchPaths);
                                                
//-----------------------------------------------------------------------------
void qSlicerModuleFactory::setLoadableModuleSearchPaths(const QStringList& paths)
{
  QCTK_D(qSlicerModuleFactory);
  d->LoadableModuleSearchPaths.clear();
  d->LoadableModuleSearchPaths << paths;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModules()
{
  qctk_d()->registerLibraries(qSlicerModuleFactoryPrivate::CmdLineLoadableModule,
    this->cmdLineModuleSearchPaths());
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModule(const QFileInfo& fileInfo)
{
  qctk_d()->registerLibrary(qSlicerModuleFactoryPrivate::CmdLineLoadableModule, fileInfo);
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::commandLineModuleNames() const
{
  return const_cast<qSlicerModuleFactoryPrivate*>(qctk_d())->CmdLineLoadableModuleFactory.names();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleFactory, QStringList, cmdLineModuleSearchPaths, CmdLineModuleSearchPaths);

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::setCmdLineModuleSearchPaths(const QStringList& paths)
{
  QCTK_D(qSlicerModuleFactory);
  d->CmdLineModuleSearchPaths.clear();
  d->CmdLineModuleSearchPaths << paths;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::moduleNames() const
{
  QStringList names;
  names << this->coreModuleNames();
  names << this->loadableModuleNames();
  names << this->commandLineModuleNames();
  return names;
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::getModuleTitle(const QString & moduleName) const
{
  QCTK_D(const qSlicerModuleFactory);
  // Lookup module name
  qSlicerModuleFactoryPrivate::MapConstIterator iter = d->MapNameToTitle.constFind(moduleName);

  if (iter == d->MapNameToTitle.constEnd())
    {
    //qCritical() << "Failed to retrieve module title given its name:" << moduleName;
    return QString();
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::getModuleName(const QString & moduleTitle) const
{
  QCTK_D(const qSlicerModuleFactory);
  // Lookup module name
  qSlicerModuleFactoryPrivate::MapConstIterator iter = d->MapTitleToName.constFind(moduleTitle);

  if (iter == d->MapTitleToName.constEnd())
    {
    //qCritical() << "Failed to retrieve module name given its title:" << moduleTitle;
    return QString();
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactory::instantiateModule(const QString& moduleName)
{
  qSlicerAbstractModule* module = qctk_d()->instantiateModule(moduleName, true);
  Q_ASSERT(module);
  if (!module)
    {
    return 0;
    }
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::uninstantiateModule(const QString& moduleName)
{
  qctk_d()->instantiateModule(moduleName, false);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::uninstantiateAll()
{
  foreach(const QString& name, this->moduleNames())
    {
    qDebug() << "Uninstantiating:" << name;
    this->uninstantiateModule(name);
    }
}

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryPrivate methods

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactoryPrivate::instantiateModule(const QString& moduleName,
                                                                          bool instantiate)
{
  // Retrieve the factoryType associated with the module
  Self::Map2ConstIterator iter = this->MapModuleNameToFactoryType.constFind(moduleName);

  Q_ASSERT(iter != this->MapModuleNameToFactoryType.constEnd());
  if (iter == this->MapModuleNameToFactoryType.constEnd())
    {
    qWarning() << "Failed to retrieve factory type for module:" << moduleName;
    return 0;
    }

  qSlicerAbstractModule* module = 0;

  int factoryType = iter.value();

  if (factoryType == CoreModule)
    {
    if (instantiate)
      {
      // Try to instantiate a core module
      module = this->CoreModuleFactory.instantiate(moduleName);
      }
    else
      {
      this->CoreModuleFactory.uninstantiate(moduleName);
      }
    }
  else if (factoryType == LoadableModule)
    {
    if (instantiate)
      {
      // Try to instantiate a loadable module
      module = this->LoadableModuleFactory.instantiate(moduleName);
      module->setName(moduleName);
      }
    else
      {
      this->LoadableModuleFactory.uninstantiate(moduleName);
      }
    }
  else if (factoryType == CmdLineLoadableModule)
    {
    if (instantiate)
      {
      // Try to instantiate a loadable command line module
      module = this->CmdLineLoadableModuleFactory.instantiate(moduleName);
      module->setName(moduleName);
      }
    else
      {
      this->CmdLineLoadableModuleFactory.uninstantiate(moduleName);
      }
    }
  else if (factoryType == CmdLineExecutableModule)
    {
    if (instantiate)
      {
      // Try to instantiate an executable command line module
      qDebug() << "CmdLineExecutableModule - instantiate - Not implemented";
      }
    else
      {
      qDebug() << "CmdLineExecutableModule - uninstantiate - Not implemented";
      }
    }
  else if (factoryType == CmdLinePythonModule)
    {
    if (instantiate)
      {
      // Try to instantiate a python command line module
      qDebug() << "CmdLinePythonModule - instantiate - Not implemented";
      }
    else
      {
      qDebug() << "CmdLinePythonModule - uninstantiate - Not implemented";
      }
    }

  if (instantiate && !module)
    {
    //qCritical() << "Failed to instantiate module:" << moduleName;
    }
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryPrivate::updateInternalMaps(
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
void qSlicerModuleFactoryPrivate::registerLibraries(int factoryType, const QStringList& paths)
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
void qSlicerModuleFactoryPrivate::registerLibrary(int factoryType, const QFileInfo& fileInfo)
{
  QString libraryName = fileInfo.fileName();
  qDebug() << "Attempt to register library" << fileInfo.filePath();
  if (!QLibrary::isLibrary(libraryName))
    {
    //qDebug() << "-->Skiped";
    return;
    }

  if (factoryType == Self::LoadableModule)
    {
    if (this->LoadableModuleFactory.registerLibrary(libraryName, fileInfo.filePath()))
      {
      // Keep track of the factory type associated with the module
      this->MapModuleNameToFactoryType[libraryName] = factoryType;

      // Instantiate the object and retrieve module title
      qSlicerAbstractModule* module = this->instantiateModule(libraryName, true);
      Q_ASSERT(module);
      this->updateInternalMaps(module->title(), libraryName);
      qDebug() << "qSlicerModuleFactory::registerLoadableModule - title:" << module->title();
      }
    }
  else if (factoryType == Self::CmdLineLoadableModule)
    {
    if (this->CmdLineLoadableModuleFactory.registerLibrary(libraryName, fileInfo.filePath()))
      {
      // Keep track of the factory type associated with the module
      this->MapModuleNameToFactoryType[libraryName] = factoryType;

      // Instantiate the object and retrieve module title
      qSlicerAbstractModule* module = this->instantiateModule(libraryName, true);
      Q_ASSERT(module);

      module->initialize(qSlicerCoreApplication::application()->appLogic());

      this->updateInternalMaps(module->title(), libraryName);
      qDebug() << "qSlicerModuleFactory::registerCmdLineModule - title:" << module->title();
      }
    }
}

//-----------------------------------------------------------------------------
// Internal classes

//-----------------------------------------------------------------------------
class qSlicerModuleFactoryPrivate::qSlicerFactoryLoadableCmdLineModuleItem :
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
    module->setupUi();
    return module;
    }
};

//-----------------------------------------------------------------------------
class qSlicerModuleFactoryPrivate::qSlicerFactoryExecutableCmdLineModuleItem :
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
    qDebug() << "CmdLineExecutableModuleItem::instantiate - name:" << this->path();

    return 0;
    }

private:
  QString          Path;
};
