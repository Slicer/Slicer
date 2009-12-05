#include "qSlicerModuleManager.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleFactory.h"
#include "qSlicerModulePanel.h"

// MRML includes
#include <vtkMRMLScene.h>

// QT includes
#include <QToolBar>
#include <QAction>
#include <QSignalMapper>

//-----------------------------------------------------------------------------
struct qSlicerModuleManagerPrivate: public qCTKPrivate<qSlicerModuleManager>
{
  QCTK_DECLARE_PUBLIC(qSlicerModuleManager);
  
  qSlicerModuleManagerPrivate()
    {
    this->ModulePanel = 0;
    this->ModuleToolBar = 0;
    }

  // Description:
  // Instantiate a module panel
  void instantiateModulePanel();

  // Description:
  // Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractModule* module);

  // Description:
  // Handle pre-unload operation
  void onModuleAboutToBeUnloaded(qSlicerAbstractModule* module);

  typedef QHash<QString, qSlicerAbstractModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractModule*>::iterator       ModuleListIterator;

  QHash<QString, qSlicerAbstractModule*> ModuleList;  // Store Pair<ModuleName, ModuleObject>
  qSlicerModuleFactory                   ModuleFactory;
  qSlicerModulePanel*                    ModulePanel;
  QToolBar*                              ModuleToolBar;
  QSignalMapper*                         ShowModuleActionMapper;
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
  QCTK_D(qSlicerModuleManager);
  d->ShowModuleActionMapper = new QSignalMapper(this);
  
  this->connect(d->ShowModuleActionMapper,
                SIGNAL(mapped(const QString&)),
                SLOT(showModuleByName(const QString&)));
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

  // Handle post-load initialization
  d->onModuleLoaded(module);
  
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

  qSlicerAbstractModule * module = iter.value();
  Q_ASSERT(module);
  
  // Handle pre-unload
  d->onModuleAboutToBeUnloaded(module);
  
  // Tells Qt to delete the object when appropriate
  module->deleteLater();

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

  qDebug() << "Show module by title:" << moduleTitle;
  qSlicerAbstractModule * module = this->getModule(moduleTitle);
  Q_ASSERT(module);
  d->ModulePanel->setModule(module);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::showModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  qDebug() << "Show module by name:" << moduleName;
  qSlicerAbstractModule * module = this->getModuleByName(moduleName);
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
QCTK_SET_CXX(qSlicerModuleManager, QToolBar*, setModuleToolBar, ModuleToolBar);

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

//---------------------------------------------------------------------------
void qSlicerModuleManagerPrivate::onModuleLoaded(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);
  if (!this->ModuleToolBar)
    {
    return;
    }
  QAction * action = module->showModuleAction();
  if (action)
    {
    // Add action to signal mapper
    this->ShowModuleActionMapper->setMapping(action, module->name());
    QObject::connect(action, SIGNAL(triggered()), this->ShowModuleActionMapper, SLOT(map()));

    // Update action state
    bool visible = module->isShowModuleActionVisibleByDefault();
    action->setVisible(visible);
    action->setEnabled(visible);

    // Add action to ToolBar
    this->ModuleToolBar->addAction(action);
    }
}


//---------------------------------------------------------------------------
void qSlicerModuleManagerPrivate::onModuleAboutToBeUnloaded(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);
  if (!this->ModuleToolBar)
    {
    return;
    }
  QAction * action = module->showModuleAction();
  if (action)
    {
    this->ModuleToolBar->removeAction(action);
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
