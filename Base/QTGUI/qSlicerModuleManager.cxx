#include "qSlicerModuleManager.h"

#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerModulePanel.h"

#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
struct qSlicerModuleManager::qInternal
{
  qInternal()
    {
    this->ModulePanel = 0;
    }

  // Description:
  // Instantiate a module panel
  void instantiateModulePanel();

  typedef QHash<QString, qSlicerAbstractModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractModule*>::iterator       ModuleListIterator;

  QHash<QString, qSlicerAbstractModule*> ModuleList;  // Store Pair<ModuleName, ModuleObject>
  qSlicerModuleFactory                   ModuleFactory;
  qSlicerModulePanel*                    ModulePanel;
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
qSlicerModuleFactory* qSlicerModuleManager::factory()
{
  return &this->Internal->ModuleFactory;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::isLoaded(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = this->Internal->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::loadModule(const QString& moduleTitle)
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
bool qSlicerModuleManager::loadModuleByName(const QString& moduleName)
{
  // Check if module has been loaded already
  qInternal::ModuleListConstIterator iter = this->Internal->ModuleList.constFind(moduleName);
  if (iter != this->Internal->ModuleList.constEnd())
    {
    //return iter.value();
    return true;
    }

  // Instantiate the module
  qSlicerAbstractModule * module = this->Internal->ModuleFactory.instantiateModule(moduleName);
  if (!module)
    {
    qWarning() << "Failed to instanciate module: " << moduleName;
    return 0;
    }

  // Update internal Map
  this->Internal->ModuleList[moduleName] = module;

  // Initialize module
  module->initialize(qSlicerApplication::application()->appLogic());

  // Retrieve module title
  QString moduleTitle = this->Internal->ModuleFactory.getModuleTitle(moduleName);
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
  module->setMRMLScene(qSlicerApplication::application()->mrmlScene());

  // Module should also be aware if current MRML scene has changed
  this->connect(qSlicerApplication::application(),
                SIGNAL(currentMRMLSceneChanged(vtkMRMLScene*)),
                module,
                SLOT(setMRMLScene(vtkMRMLScene*)));

  return true;
  //return module;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModule(const QString& moduleTitle)
{
  // Get corresponding module name
  QString moduleName = this->Internal->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return false;
    }

  return this->unLoadModuleByName(moduleName);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModuleByName(const QString& moduleName)
{
  qInternal::ModuleListConstIterator iter =
    this->Internal->ModuleList.find( moduleName );
  if (iter == this->Internal->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << moduleName << " - Module wasn't loaded";
    return false;
    }
  // Tells Qt to delete the object when appropriate
  iter.value()->deleteLater();

  // Remove the object from the list
  this->Internal->ModuleList.remove(iter.key());

  return true;
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
  this->Internal->instantiateModulePanel();
  Q_ASSERT(this->Internal->ModulePanel);

  qDebug() << "Show module:" << moduleTitle;
  qSlicerAbstractModule * module = this->getModule(moduleTitle);
  Q_ASSERT(module);
//   // ----- [To be removed] -----
//   this->setModulePanelVisible(module);
//   if (!module)
//     {
//     qWarning() << "Failed to show module:" << moduleTitle;
//     this->setModulePanelVisible(false);
//     return;
//     }
//   // ----- [/To be removed] -----
  this->Internal->ModulePanel->setModule(module);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::setModulePanelVisible(bool visible)
{
  this->Internal->instantiateModulePanel();
  Q_ASSERT(this->Internal->ModulePanel);

  this->Internal->ModulePanel->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::setModulePanelGeometry(int ax, int ay, int aw, int ah)
{
  this->Internal->instantiateModulePanel();
  Q_ASSERT(this->Internal->ModulePanel);

  this->Internal->ModulePanel->setGeometry(QRect(ax, ay, aw, ah));
}

//---------------------------------------------------------------------------
qSlicerGetInternalCxxMacro(qSlicerModuleManager, qSlicerAbstractModulePanel*,
                           modulePanel, ModulePanel);

//---------------------------------------------------------------------------
// Internal methods
//---------------------------------------------------------------------------
void qSlicerModuleManager::qInternal::instantiateModulePanel()
{
  if (!this->ModulePanel)
    {
    this->ModulePanel =
      new qSlicerModulePanel(0, qSlicerApplication::application()->defaultWindowFlags());
    }
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
