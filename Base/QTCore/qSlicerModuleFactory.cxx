/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerModuleFactory.h"

// SlicerQT includes
#include "qSlicerAbstractLoadableModule.h"
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
class qSlicerModuleFactoryPrivate : public qCTKPrivate<qSlicerModuleFactory>
{
public:
  typedef qSlicerModuleFactoryPrivate Self;
  
  typedef QHash<QString, QString>::const_iterator MapConstIterator;
  typedef QHash<QString, QString>::iterator       MapIterator;

  typedef QHash<QString, qCTKAbstractFactory<qSlicerAbstractModule>*>::const_iterator     Map2ConstIterator;
  typedef QHash<QString, qCTKAbstractFactory<qSlicerAbstractModule>*>::iterator           Map2Iterator;

  class qSlicerFactoryLoadableCmdLineModuleItem;
  class qSlicerFactoryExecutableCmdLineModuleItem;

  typedef qCTKAbstractQObjectFactory<qSlicerAbstractModule>  CoreModuleFactoryType;
  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule>  LoadableModuleFactoryType;
  //typedef qSlicerPluginFactory LoadableModuleFactoryType;
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

  qSlicerModuleFactoryPrivate();

  // Description:
  // Keep track of the mapping between module title and module name
  void updateInternalMaps(const QString& moduleTitle, const QString& moduleName);

  // Description:
  // Convenient function used to register a library in the appropriate factory.
  // Either using the LoadableModuleFactory or the CmdLineLoadableModuleFactory
  void registerLibraries(qCTKAbstractFactory<qSlicerAbstractModule>*, const QStringList& paths);

  // Description:
  // Convenient function used to register a library in the appropriate factory
  void registerLibrary(qCTKAbstractFactory<qSlicerAbstractModule>*, QFileInfo libraryFileInfo);

  // Description:
  // If instantiate is true, instantiate a module. Otherwise uninstantiate it.
  qSlicerAbstractModule* instantiateModule(qCTKAbstractFactory<qSlicerAbstractModule>* factory, const QString& moduleName);

  void uninstantiateModule(const QString& moduleName);

  QHash<QString, QString> MapTitleToName;
  QHash<QString, QString> MapNameToTitle;
  QStringList             LoadableModuleSearchPaths;
  QStringList             CmdLineModuleSearchPaths;

  //
  CoreModuleFactoryType*              CoreModuleFactory;
  LoadableModuleFactoryType*          LoadableModuleFactory;
  CmdLineLoadableModuleFactoryType*   CmdLineLoadableModuleFactory;

  //QHash<QString, int> MapModuleNameToFactoryType;
  QHash<QString, qCTKAbstractFactory<qSlicerAbstractModule>* > ModuleNameToFactoryCache;
};

//-----------------------------------------------------------------------------
qSlicerModuleFactory::qSlicerModuleFactory()
{
  QCTK_INIT_PRIVATE(qSlicerModuleFactory);
}

//-----------------------------------------------------------------------------
qSlicerModuleFactory::~qSlicerModuleFactory()
{
  this->uninstantiateAll();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::printAdditionalInfo()
{
  QCTK_D(qSlicerModuleFactory);
  
  //this->Superclass::printAdditionalInfo();
  qDebug() << "qSlicerModuleFactory (" << this << ")";
  qDebug() << "LoadableModuleSearchPaths:" << d->LoadableModuleSearchPaths;
  qDebug() << "CmdLineModuleSearchPaths:" << d->CmdLineModuleSearchPaths;
  qDebug() << "MapTitleToName:";
  qSlicerModuleFactoryPrivate::MapConstIterator iter = d->MapTitleToName.constBegin();
  while(iter != d->MapTitleToName.constEnd())
    {
    qDebug() << "Title:" << iter.key() << "-> Name:" << iter.value();
    ++iter;
    }
  qDebug() << "MapNameToTitle:";
  iter = d->MapNameToTitle.constBegin();
  while(iter != d->MapNameToTitle.constEnd())
    {
    qDebug() << "Name:" << iter.key() << "-> Title:" << iter.value();
    ++iter;
    }
}

//-----------------------------------------------------------------------------
bool qSlicerModuleFactory::isRegistered(const QString& moduleName)const
{
  QCTK_D(const qSlicerModuleFactory);
  return d->MapNameToTitle.contains(moduleName);
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::coreModuleNames() const
{
  return qctk_d()->CoreModuleFactory->names();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCoreModules()
{
  this->registerCoreModule<qSlicerTransformsModule>();
  this->registerCoreModule<qSlicerCamerasModule>();
}

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerModuleFactory::registerCoreModule()
{
  QCTK_D(qSlicerModuleFactory);
  
  QString moduleName;
  if (!d->CoreModuleFactory->registerQObject<ClassType>(moduleName))
    {
    qDebug() << "Failed to register module: " << moduleName; 
    return;
    }
  
  // Keep track of the factory type associated with the module
  //d->MapModuleNameToFactoryType[moduleName] = qSlicerModuleFactoryPrivate::CoreModule;
  d->ModuleNameToFactoryCache[moduleName] = d->CoreModuleFactory;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::loadableModuleNames() const
{
  return qctk_d()->LoadableModuleFactory->names();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerLoadableModules()
{ 
  qctk_d()->registerLibraries(qctk_d()->LoadableModuleFactory,
                              this->loadableModuleSearchPaths());
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerLoadableModule(const QFileInfo& fileInfo)
{
  qctk_d()->registerLibrary(qctk_d()->LoadableModuleFactory, fileInfo);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleFactory, QStringList, loadableModuleSearchPaths,
                                                LoadableModuleSearchPaths);
         
//-----------------------------------------------------------------------------
void qSlicerModuleFactory::setLoadableModuleSearchPaths(const QStringList& paths)
{
  QCTK_D(qSlicerModuleFactory);
  d->LoadableModuleSearchPaths = paths;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModuleFactory::commandLineModuleNames() const
{
  return qctk_d()->CmdLineLoadableModuleFactory->names();
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModules()
{
  qctk_d()->registerLibraries(qctk_d()->CmdLineLoadableModuleFactory,
                              this->cmdLineModuleSearchPaths());
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::registerCmdLineModule(const QFileInfo& fileInfo)
{
  qctk_d()->registerLibrary(qctk_d()->CmdLineLoadableModuleFactory, fileInfo);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleFactory, QStringList, cmdLineModuleSearchPaths, CmdLineModuleSearchPaths);

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::setCmdLineModuleSearchPaths(const QStringList& paths)
{
  QCTK_D(qSlicerModuleFactory);
  d->CmdLineModuleSearchPaths = paths;
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
QString qSlicerModuleFactory::moduleTitle(const QString & moduleName) const
{
  QCTK_D(const qSlicerModuleFactory);
  // Lookup module name
  qSlicerModuleFactoryPrivate::MapConstIterator iter = 
    d->MapNameToTitle.constFind(moduleName);

  if (iter == d->MapNameToTitle.constEnd())
    {
    //qCritical() << "Failed to retrieve module title given its name:" << moduleName;
    return QString();
    }
  return iter.value();
}

//-----------------------------------------------------------------------------
QString qSlicerModuleFactory::moduleName(const QString & moduleTitle) const
{
  QCTK_D(const qSlicerModuleFactory);
  // Lookup module name
  qSlicerModuleFactoryPrivate::MapConstIterator iter = 
    d->MapTitleToName.constFind(moduleTitle);

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
  QCTK_D(qSlicerModuleFactory);
  // Retrieve the factoryType associated with the module
  qSlicerModuleFactoryPrivate::Map2ConstIterator iter = 
    d->ModuleNameToFactoryCache.constFind(moduleName);

  Q_ASSERT(iter != d->ModuleNameToFactoryCache.constEnd());
  if (iter == d->ModuleNameToFactoryCache.constEnd())
    {
    qWarning() << "Failed to retrieve factory type for module:" << moduleName;
    return 0;
    }

  qSlicerAbstractModule* module = 
    d->instantiateModule(*iter, moduleName);
  Q_ASSERT(module);
  
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::instantiateCoreModules()
{
  QCTK_D(qSlicerModuleFactory);  
  qSlicerAbstractModule* module = 0;
  foreach(const QString& moduleName, this->coreModuleNames())
    {
    module = d->instantiateModule(d->CoreModuleFactory, moduleName);
    Q_ASSERT(module);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::instantiateLoadableModules()
{
  QCTK_D(qSlicerModuleFactory);  
  qSlicerAbstractModule* module = 0;
  foreach(const QString& moduleName, this->loadableModuleNames())
    {
    module = d->instantiateModule(d->LoadableModuleFactory, moduleName);
    Q_ASSERT(module);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::instantiateCmdLineModules()
{
  QCTK_D(qSlicerModuleFactory);  
  qSlicerAbstractModule* module = 0;
  foreach(const QString& moduleName, this->commandLineModuleNames())
    {
    module = d->instantiateModule(d->CmdLineLoadableModuleFactory, moduleName);
    Q_ASSERT(module);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::uninstantiateModule(const QString& moduleName)
{
  qDebug() << "Uninstantiating:" << moduleName;
  qctk_d()->uninstantiateModule(moduleName);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactory::uninstantiateAll()
{
  foreach(const QString& name, this->moduleNames())
    {
    this->uninstantiateModule(name);
    }
}

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryPrivate methods

//-----------------------------------------------------------------------------

qSlicerModuleFactoryPrivate::qSlicerModuleFactoryPrivate()
{
  this->CoreModuleFactory = new CoreModuleFactoryType;
  this->LoadableModuleFactory = new LoadableModuleFactoryType;
  this->CmdLineLoadableModuleFactory = new CmdLineLoadableModuleFactoryType;

  // Set the list of required symbols for CmdLineLoadableModule,
  // if one of these symbols can't be resolved, the library won't be registered.
  QStringList cmdLineModuleSymbols;
  cmdLineModuleSymbols << "XMLModuleDescription";
  this->CmdLineLoadableModuleFactory->setSymbols(cmdLineModuleSymbols);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleFactoryPrivate::instantiateModule(
  qCTKAbstractFactory<qSlicerAbstractModule>* factory, const QString& moduleName)
{
  qSlicerAbstractModule* module = 0;
  // Try to instantiate a module
  module = factory->instantiate(moduleName);
  Q_ASSERT(module);
  module->setName(moduleName);
  
  QString moduleTitle = module->title();
  // Keep track of the relation Title -> moduleName
  this->MapTitleToName[moduleTitle] = moduleName;

  // Keep track of the relation moduleName -> Title
  this->MapNameToTitle[moduleName] = moduleTitle;
  
  return module;
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryPrivate::uninstantiateModule(const QString& moduleName)
{
  QCTK_P(qSlicerModuleFactory);
  // Retrieve the factoryType associated with the module
  Self::Map2ConstIterator iter = this->ModuleNameToFactoryCache.constFind(moduleName);

  Q_ASSERT(iter != this->ModuleNameToFactoryCache.constEnd());
  if (iter == this->ModuleNameToFactoryCache.constEnd())
    {
    qWarning() << "Failed to retrieve factory type for module:" << moduleName;
    return;
    }
  QString moduleTitle = p->moduleTitle(moduleName);
  
  iter.value()->uninstantiate(moduleName);
  
  this->MapTitleToName.remove(moduleTitle);
  this->MapNameToTitle.remove(moduleName);
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryPrivate::registerLibraries(
  qCTKAbstractFactory<qSlicerAbstractModule>* factory, const QStringList& paths)
{
  // warning
  if (paths.isEmpty())
    {
    qWarning() << "SearchPaths is empty";
    return;
    }

  // process each path at a time
  foreach (QString path, paths)
    {
    QDirIterator it(path);
    while (it.hasNext())
      {// for each item in a directory, check if it's a file at least
      it.next();
      if (it.fileInfo().isFile())
        {
        this->registerLibrary(factory, it.fileInfo());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerModuleFactoryPrivate::registerLibrary(
  qCTKAbstractFactory<qSlicerAbstractModule>* factory, 
  QFileInfo libraryFileInfo)
{
  if (libraryFileInfo.isSymLink())
    {
    // symLinkTarget() handles links pointing to symlinks. how about a symlink 
    // pointing to a symlink ?
    libraryFileInfo = QFileInfo(libraryFileInfo.symLinkTarget());
    }
  
  // Seems ok for loadableModuleFactory, 
  // I'm not sure it's correct for cmdLineLoadableModuleFactory
  // Isn't it better to go in LoadableModuleFactoryType::registerLibrary() 
  // directly ?
  if (!QLibrary::isLibrary(libraryFileInfo.fileName()))
    {
    //qDebug() << "-->Skiped";
    return;
    }
  qDebug() << "Attempt to register library" << libraryFileInfo.fileName();
  
  LoadableModuleFactoryType* loadableModuleFactory  =
    dynamic_cast<LoadableModuleFactoryType*>(factory);
  CmdLineLoadableModuleFactoryType* cmdLineLoadableModuleFactory =
    dynamic_cast<CmdLineLoadableModuleFactoryType*>(factory);
  if (loadableModuleFactory)
    {
    Q_ASSERT( loadableModuleFactory == this->LoadableModuleFactory);
    QString libraryName;
    if (!loadableModuleFactory->registerLibrary(libraryFileInfo, libraryName))
      {
      qDebug() << "Failed to register module: " << libraryName; 
      return;
      }
    // Keep track of the factory type associated with the module
    this->ModuleNameToFactoryCache[libraryName] = this->LoadableModuleFactory;

    // Instantiate the object and retrieve module title
    //qSlicerAbstractModule* module = this->instantiateModule(libraryName);
    //Q_ASSERT(module);
    //this->updateInternalMaps(module->title(), libraryName);
    //qDebug() << "qSlicerModuleFactory::registerLoadableModule - title:" << module->title();
    }
  else if (cmdLineLoadableModuleFactory)
    {
    Q_ASSERT( cmdLineLoadableModuleFactory == this->CmdLineLoadableModuleFactory);
    QString pluginName;
    if (!cmdLineLoadableModuleFactory->registerLibrary(libraryFileInfo, pluginName))
      {
      qDebug() << "Failed to register module: " << pluginName; 
      return;
      }
      
    // Keep track of the factory type associated with the module
    this->ModuleNameToFactoryCache[pluginName] = this->CmdLineLoadableModuleFactory;

    // Instantiate the object and retrieve module title
    //qSlicerAbstractModule* module = this->instantiateModule(pluginName);
    //Q_ASSERT(module);
    //module->initialize(qSlicerCoreApplication::application()->appLogic());

    //this->updateInternalMaps(module->title(), libraryName);
    //qDebug() << "qSlicerModuleFactory::registerCmdLineModule - title:" << module->title();
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

    module->setTempDirectory(qSlicerCoreApplication::application()->tempDirectory());
    
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
