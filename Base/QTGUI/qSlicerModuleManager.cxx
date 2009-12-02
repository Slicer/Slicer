#include "qSlicerModuleManager.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerModulePanel.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
struct qSlicerModuleManagerPrivate: public qCTKPrivate<qSlicerModuleManager>
{
  qSlicerModuleManagerPrivate()
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
  QCTK_INIT_PRIVATE(qSlicerModuleManager);
}

//-----------------------------------------------------------------------------
void qSlicerModuleManager::printAdditionalInfo()
{
  QCTK_D(qSlicerModuleManager);
  qDebug() << "qSlicerModuleManager (" << this << ")";
  qDebug() << "ModuleList";

  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constBegin();
  while(iter != d->ModuleList.constEnd())
    {
    qDebug() << "Name:" << iter.key();
    iter.value()->printAdditionalInfo();
    ++iter;
    }
  d->ModuleFactory.printAdditionalInfo();
}

//---------------------------------------------------------------------------
qSlicerModuleFactory* qSlicerModuleManager::factory()
{
  return &qctk_d()->ModuleFactory;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::isLoaded(const QString& moduleTitle)
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
bool qSlicerModuleManager::loadModule(const QString& moduleTitle)
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
bool qSlicerModuleManager::loadModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  // Check if module has been loaded already
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constFind(moduleName);
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
  module->initialize(qSlicerApplication::application()->appLogic());

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
  QString moduleName = qctk_d()->ModuleFactory.getModuleName(moduleTitle);
  if (moduleName.isEmpty())
    {
    return false;
    }

  return this->unLoadModuleByName(moduleName);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( moduleName );
  if (iter == d->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << moduleName << " - Module wasn't loaded";
    return false;
    }
  // Tells Qt to delete the object when appropriate
  iter.value()->deleteLater();

  // Remove the object from the list
  d->ModuleList.remove(iter.key());

  return true;
}


//---------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerModuleManager::getModule(const QString& moduleTitle)
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
qSlicerAbstractModule* qSlicerModuleManager::getModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( moduleName );
  if ( iter == d->ModuleList.constEnd() )
    {
    return 0;
    }
  return iter.value();
}

//---------------------------------------------------------------------------
const QString qSlicerModuleManager::moduleTitle(const QString& moduleName)
{
  return qctk_d()->ModuleFactory.getModuleTitle(moduleName);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::showModule(const QString& moduleTitle)
{
  QCTK_D(qSlicerModuleManager);
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  qDebug() << "Show module:" << moduleTitle;
  qSlicerAbstractModule * module = this->getModule(moduleTitle);
  Q_ASSERT(module);
  d->ModulePanel->setModule(module);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::setModulePanelVisible(bool visible)
{
  QCTK_D(qSlicerModuleManager);
  
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  d->ModulePanel->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::setModulePanelGeometry(int ax, int ay, int aw, int ah)
{
  QCTK_D(qSlicerModuleManager);
  
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  d->ModulePanel->setGeometry(QRect(ax, ay, aw, ah));
}

//---------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleManager, qSlicerAbstractModulePanel*, modulePanel, ModulePanel);

//---------------------------------------------------------------------------
// qSlicerModuleManagerPrivate methods

//---------------------------------------------------------------------------
void qSlicerModuleManagerPrivate::instantiateModulePanel()
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
