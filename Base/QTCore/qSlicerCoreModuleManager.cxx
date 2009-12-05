#include "qSlicerCoreModuleManager.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleFactory.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
struct qSlicerCoreModuleManagerPrivate: public qCTKPrivate<qSlicerCoreModuleManager>
{
  QCTK_DECLARE_PUBLIC(qSlicerCoreModuleManager);

  // Description:
  // Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractModule* module);

  typedef QHash<QString, qSlicerAbstractModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractModule*>::iterator       ModuleListIterator;

  QHash<QString, qSlicerAbstractModule*> ModuleList;  // Store Pair<ModuleName, ModuleObject>
  qSlicerModuleFactory                   ModuleFactory;
};

//-----------------------------------------------------------------------------
qSlicerCoreModuleManager::qSlicerCoreModuleManager()
{
  QCTK_INIT_PRIVATE(qSlicerCoreModuleManager);
  //QCTK_D(qSlicerCoreModuleManager);
}

//-----------------------------------------------------------------------------
void qSlicerCoreModuleManager::printAdditionalInfo()
{
  QCTK_D(qSlicerCoreModuleManager);
  qDebug() << "qSlicerCoreModuleManager (" << this << ")";
  qDebug() << "ModuleList";

  qSlicerCoreModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constBegin();
  while(iter != d->ModuleList.constEnd())
    {
    qDebug() << "Name:" << iter.key();
    iter.value()->printAdditionalInfo();
    ++iter;
    }
  d->ModuleFactory.printAdditionalInfo();
}

//---------------------------------------------------------------------------
qSlicerModuleFactory* qSlicerCoreModuleManager::factory()
{
  return &qctk_d()->ModuleFactory;
}

//---------------------------------------------------------------------------
bool qSlicerCoreModuleManager::isLoaded(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = qctk_d()->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool qSlicerCoreModuleManager::loadModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = qctk_d()->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return 0;
    }

  return this->loadModuleByName(moduleName);
}

//---------------------------------------------------------------------------
bool qSlicerCoreModuleManager::loadModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerCoreModuleManager);
  // Check if module has been loaded already
  qSlicerCoreModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constFind(moduleName);
  if (iter != d->ModuleList.constEnd())
    {
    //return iter.value();
    return true;
    }

  // Instantiate the module
  qSlicerAbstractModule * module = d->ModuleFactory.instantiateModule(moduleName);
  if (!module)
    {
    qWarning() << "Failed to instanciate module: " << moduleName;
    return 0;
    }

  // Update internal Map
  d->ModuleList[moduleName] = module;

  // Initialize module
  module->initialize(qSlicerCoreApplication::application()->appLogic());

  // Retrieve module title
  QString moduleTitle = d->ModuleFactory.getModuleTitle(moduleName);
  Q_ASSERT(!moduleTitle.isEmpty());
  if (moduleTitle.isEmpty())
    {
    qWarning() << "Failed to retrieve module title: " << moduleName;
    return 0;
    }

  // Set module title
  module->setWindowTitle(moduleTitle);
  qDebug() << module << " - title:" << moduleTitle;

  // Set the MRML scene
  module->setMRMLScene(qSlicerCoreApplication::application()->mrmlScene());

  // Module should also be aware if current MRML scene has changed
  this->connect(qSlicerCoreApplication::application(),
                SIGNAL(currentMRMLSceneChanged(vtkMRMLScene*)),
                module,
                SLOT(setMRMLScene(vtkMRMLScene*)));

  // Handle post-load initialization
  //d->onModuleLoaded(module);
  
  return true;
  //return module;
}

//---------------------------------------------------------------------------
bool qSlicerCoreModuleManager::unLoadModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = qctk_d()->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return false;
    }

  return this->unLoadModuleByName(moduleName);
}

//---------------------------------------------------------------------------
bool qSlicerCoreModuleManager::unLoadModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerCoreModuleManager);
  qSlicerCoreModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( moduleName );
  if (iter == d->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << moduleName << " - Module wasn't loaded";
    return false;
    }

  qSlicerAbstractModule * module = iter.value();
  Q_ASSERT(module);
  
  // Handle pre-unload
  //d->onModuleAboutToBeUnloaded(module);
  
  // Tells Qt to delete the object when appropriate
  module->deleteLater();

  // Remove the object from the list
  d->ModuleList.remove(iter.key());

  return true;
}


//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerCoreModuleManager::getModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = qctk_d()->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return 0;
    }
  return this->getModuleByName(moduleName);
}

//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerCoreModuleManager::getModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerCoreModuleManager);
  qSlicerCoreModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( moduleName );
  if ( iter == d->ModuleList.constEnd() )
    {
    return 0;
    }
  return iter.value();
}

//---------------------------------------------------------------------------
const QString qSlicerCoreModuleManager::moduleTitle(const QString& moduleName)
{
  return qctk_d()->ModuleFactory.getModuleTitle(moduleName);
}
