#include "qSlicerModuleManager.h" 

#include "qSlicerModuleFactory.h"
#include "qSlicerAbstractModule.h"

#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
class qSlicerModuleManager::qInternal
{
public:
  typedef QHash<QString, qSlicerAbstractModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractModule*>::iterator       ModuleListIterator;
  
  qInternal()
    {
    }
    
  QHash<QString, qSlicerAbstractModule*> ModuleList;  // Store Pair<ModuleName, ModuleObject>
  qSlicerModuleFactory                   ModuleFactory; 
};

//----------------------------------------------------------------------------
qSlicerModuleManager* qSlicerModuleManager::instance()
{
  return Self::Instance; 
}
 
//----------------------------------------------------------------------------
void qSlicerModuleManager::classInitialize()
{
  Self::Instance = new qSlicerModuleManager;
}

//----------------------------------------------------------------------------
void qSlicerModuleManager::classFinalize()
{
  delete Self::Instance;
}

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager()
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerModuleManager::~qSlicerModuleManager()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerModuleManager::printAdditionalInfo()
{
  qDebug() << "qSlicerModuleManager (" << this << ")"; 
  qDebug() << "ModuleList"; 
  
  qInternal::ModuleListConstIterator iter = this->Internal->ModuleList.constBegin();
  while(iter != this->Internal->ModuleList.constEnd())
    {
    qDebug() << "Name:" << iter.key(); 
    iter.value()->printAdditionalInfo();
    ++iter;
    }
  this->Internal->ModuleFactory.printAdditionalInfo(); 
}

//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::loadModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = this->Internal->ModuleFactory.getModuleName(moduleTitle); 
  if (moduleName.isEmpty())
    {
    return 0; 
    }
  
  return this->loadModuleByName(moduleName);
}

//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::loadModuleByName(const QString& moduleName)
{
  // Check if module has been loaded already
  qInternal::ModuleListConstIterator iter = this->Internal->ModuleList.constFind(moduleName); 
  if (iter != this->Internal->ModuleList.constEnd())
    {
    return iter.value();
    }
  
  // Instanciate the module
  qSlicerAbstractModule * module = this->Internal->ModuleFactory.createModule(moduleName); 
  if (!module)
    {
    qWarning() << "Failed to load module: " << moduleName;
    return 0; 
    }
  
  // Set the MRML scene
  module->setMRMLScene(vtkMRMLScene::GetActiveScene()); 
  
  // Update internal Map
  this->Internal->ModuleList[moduleName] = module;
  
  return module; 
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::unLoadModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = this->Internal->ModuleFactory.getModuleName(moduleTitle); 
  if (moduleName.isEmpty())
    {
    return; 
    }
    
  this->unLoadModuleByName(moduleName);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::unLoadModuleByName(const QString& moduleName)
{
  qInternal::ModuleListConstIterator iter = 
    this->Internal->ModuleList.find( moduleName ); 
  if (iter == this->Internal->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << moduleName << " - Module wasn't loaded";
    return;
    }
  // Tells Qt to delete the object when appropriate
  iter.value()->deleteLater(); 
  // Remove the object from the list
  this->Internal->ModuleList.remove(iter.key());
}


//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::getModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = this->Internal->ModuleFactory.getModuleName(moduleTitle); 
  if (moduleName.isEmpty())
    {
    return 0; 
    }
  return this->getModuleByName(moduleName); 
}

//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::getModuleByName(const QString& moduleName)
{
  qInternal::ModuleListConstIterator iter = 
    this->Internal->ModuleList.find( moduleName ); 
  if ( iter == this->Internal->ModuleList.constEnd() )
    {
    return 0;
    }
  return iter.value(); 
}

//---------------------------------------------------------------------------
const QString qSlicerModuleManager::moduleTitle(const QString& moduleName)
{
  return this->Internal->ModuleFactory.getModuleTitle(moduleName);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::showModule(const QString& moduleTitle)
{
  qDebug() << "Show module:" << moduleTitle;
  qSlicerAbstractModule * module = this->getModule(moduleTitle);
  if (!module)
    {
    return; 
    }
  //module->show(); 
  module->setParentVisible(true);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::hideModule(const QString& moduleTitle)
{
  qDebug() << "Hide module:" << moduleTitle;
  qSlicerAbstractModule * module = this->getModule(moduleTitle);
  if (!module)
    {
    return; 
    }
  //module->hide(); 
  module->setParentVisible(false);
}

//----------------------------------------------------------------------------
// Implementation of qSlicerModuleManagerInitialize class.
//----------------------------------------------------------------------------
qSlicerModuleManagerInitialize::qSlicerModuleManagerInitialize()
{
  if(++Self::Count == 1)
    { qSlicerModuleManager::classInitialize(); }
}

//----------------------------------------------------------------------------
qSlicerModuleManagerInitialize::~qSlicerModuleManagerInitialize()
{
  if(--Self::Count == 0)
    { qSlicerModuleManager::classFinalize(); }
}

//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int qSlicerModuleManagerInitialize::Count;
qSlicerModuleManager* qSlicerModuleManager::Instance;
