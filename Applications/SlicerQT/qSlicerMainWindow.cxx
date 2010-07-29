// Qt includes
#include <QDebug>
//#include <QSignalMapper>
#include <QStringList>
#include <QToolButton>

// SlicerQt includes
#include "qSlicerMainWindow.h" 
#include "ui_qSlicerMainWindow.h" 
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModulePanel.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"
#include "qSlicerModuleSelectorToolBar.h"

// MRML includes
#include <vtkMRMLScene.h>
//-----------------------------------------------------------------------------
class qSlicerMainWindowPrivate: public ctkPrivate<qSlicerMainWindow>, public Ui_qSlicerMainWindow
{
public:
  CTK_DECLARE_PUBLIC(qSlicerMainWindow);
  qSlicerMainWindowPrivate();
  void setupUi(QMainWindow * mainWindow);

  qSlicerMainWindowCore*        Core;
  qSlicerModuleSelectorToolBar* ModuleSelector;
  QStringList                   ModuleToolBarList;
  //QSignalMapper*                ModuleToolBarMapper;
};

//-----------------------------------------------------------------------------
// qSlicerMainWindowPrivate methods

qSlicerMainWindowPrivate::qSlicerMainWindowPrivate()
{
  this->Core = 0;
  this->ModuleSelector = 0;
  this->ModuleToolBarList << "Data"  << "Volumes" << "Models" << "Transforms"
                          << "Fiducials" << "Editor" << "Measurements"
                          << "Colors";
  //this->ModuleToolBarMapper = 0;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  CTK_P(qSlicerMainWindow);
  
  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  // Update the list of modules when they are loaded
  //this->ModuleToolBarMapper = new QSignalMapper(p);
  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);

  QObject::connect(moduleManager,
                   SIGNAL(moduleLoaded(qSlicerAbstractCoreModule*)),
                   p, SLOT(onModuleLoaded(qSlicerAbstractCoreModule*)));

  QObject::connect(moduleManager,
                   SIGNAL(moduleAboutToBeUnloaded(qSlicerAbstractCoreModule*)),
                   p, SLOT(onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule*)));

  //QObject::connect(this->ModuleToolBarMapper, SIGNAL(mapped(const QString&)),
  //                 this->ModulePanel, SLOT(setModule(const QString&)));

  // Create a Module selector
  this->ModuleSelector = new qSlicerModuleSelectorToolBar("Module Selector",p);
  p->insertToolBar(this->ModuleToolBar, this->ModuleSelector);

  // Connect the selector with the module panel
  QObject::connect(this->ModuleSelector, SIGNAL(moduleSelected(const QString&)),
                   this->ModulePanel, SLOT(setModule(const QString&)));

  // Instanciate and assign the layout manager to the slicer application
  qSlicerLayoutManager* layoutManager = new qSlicerLayoutManager(this->CentralWidget);
  qSlicerApplication::application()->setLayoutManager(layoutManager);

  // Layout manager should also listen the MRML scene
  layoutManager->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   layoutManager,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  // Listen to the scene
  p->qvtkConnect(qSlicerApplication::application()->mrmlScene(), vtkCommand::ModifiedEvent,
                 p, SLOT(onMRMLSceneModified(vtkObject*)));
  p->onMRMLSceneModified(qSlicerApplication::application()->mrmlScene());
}

//-----------------------------------------------------------------------------
// qSlicerMainWindow methods

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *_parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerMainWindow);
  CTK_D(qSlicerMainWindow);
  d->setupUi(this);
  
  // Main window core helps to coordinate various widgets and panels
  d->Core = new qSlicerMainWindowCore(this);
  
  this->setupMenuActions();
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerMainWindow, qSlicerMainWindowCore*, core, Core);

//-----------------------------------------------------------------------------
qSlicerModuleSelectorToolBar* qSlicerMainWindow::moduleSelector()const
{
  CTK_D(const qSlicerMainWindow);
  return d->ModuleSelector;
}

//-----------------------------------------------------------------------------
// Helper macro allowing to connect the MainWindow action with the corresponding
// slot in MainWindowCore
#define qSlicerMainWindow_connect(ACTION_NAME)   \
  this->connect(                                 \
    d->action##ACTION_NAME, SIGNAL(triggered()), \
    this->core(),                                \
    SLOT(on##ACTION_NAME##ActionTriggered()));

//-----------------------------------------------------------------------------
void qSlicerMainWindow::setupMenuActions()
{
  CTK_D(qSlicerMainWindow);
  
  this->connect(
    d->actionFileExit, SIGNAL(triggered()),
    qSlicerApplication::instance(), SLOT(quit()));

  qSlicerMainWindow_connect(FileAddData);
  qSlicerMainWindow_connect(FileImportScene);
  qSlicerMainWindow_connect(FileLoadScene);
  qSlicerMainWindow_connect(FileAddVolume);
  qSlicerMainWindow_connect(FileAddTransform);
  qSlicerMainWindow_connect(FileSaveScene);
  qSlicerMainWindow_connect(FileCloseScene);

  qSlicerMainWindow_connect(EditUndo);
  qSlicerMainWindow_connect(EditRedo);
  
  qSlicerMainWindow_connect(HelpAboutSlicerQT);

  qSlicerMainWindow_connect(ViewLayoutConventional);
  qSlicerMainWindow_connect(ViewLayoutFourUp);
  qSlicerMainWindow_connect(ViewLayoutDual3D);
  qSlicerMainWindow_connect(ViewLayoutOneUp3D);
  qSlicerMainWindow_connect(ViewLayoutOneUpRedSlice);
  qSlicerMainWindow_connect(ViewLayoutOneUpYellowSlice);
  qSlicerMainWindow_connect(ViewLayoutOneUpGreenSlice);
  qSlicerMainWindow_connect(ViewLayoutTabbed3D);
  qSlicerMainWindow_connect(ViewLayoutTabbedSlice);
  qSlicerMainWindow_connect(ViewLayoutCompare);
  qSlicerMainWindow_connect(ViewLayoutSideBySideCompare);
  
  qSlicerMainWindow_connect(WindowPythonInteractor);
    
}
#undef qSlicerMainWindow_connect

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleLoaded(qSlicerAbstractCoreModule* coreModule)
{
  CTK_D(qSlicerMainWindow);
  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(coreModule);
  if (!module)
    {
    return;
    }

  // Module Selector ToolBar
  d->ModuleSelector->addModule(module->name());

  // Module ToolBar
  QAction * action = module->action();
  if (!action || action->icon().isNull())
    {
    return;
    }

  Q_ASSERT(action->data().toString() == module->name());
  Q_ASSERT(action->text() == module->title());

  // here we just want the icons, no text
  //action->setText("");

  // Add action to signal mapper
  //d->ModuleToolBarMapper->setMapping(action, module->name());
  //QObject::connect(action, SIGNAL(triggered()),
  //                 d->ModuleToolBarMapper, SLOT(map()));

  // Update action state
  bool visible = d->ModuleToolBarList.contains(module->title());
  action->setVisible(visible);
  action->setEnabled(visible);

  // Add action to ToolBar
  d->ModuleToolBar->addAction(action);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule* module)
{
  CTK_D(qSlicerMainWindow);
  if (!module)
    {
    return;
    }

  foreach(QAction* action, d->ModuleToolBar->actions())
    {
    if (action->data().toString() == module->name())
      {
      d->ModuleToolBar->removeAction(action);
      d->ModuleSelector->removeModule(module->name());
      //d->ModuleToolBarMapper->removeMappings(action);
      return;
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onMRMLSceneModified(vtkObject* sender)
{
  CTK_D(qSlicerMainWindow);
  
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sender);
  if (scene->GetIsUpdating())
    {
    return;
    }
  d->actionEditUndo->setEnabled(scene && scene->GetNumberOfUndoLevels());
  d->actionEditRedo->setEnabled(scene && scene->GetNumberOfRedoLevels());
}

