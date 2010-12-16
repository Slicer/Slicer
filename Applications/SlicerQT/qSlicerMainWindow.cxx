/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QCloseEvent>
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QToolButton>

// CTK includes
#include <ctkConfirmExitDialog.h>
#include <ctkSettingsDialog.h>

// SlicerQt includes
#include "qSlicerMainWindow.h" 
#include "ui_qSlicerMainWindow.h" 
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerExtensionsWizard.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModulePanel.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerIOManager.h"
#include "qSlicerSettingsModulesPanel.h"
#include "qSlicerSettingsPanel.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
class qSlicerMainWindowPrivate: public Ui_qSlicerMainWindow
{
  Q_DECLARE_PUBLIC(qSlicerMainWindow);
protected:
  qSlicerMainWindow* const q_ptr;
public:
  qSlicerMainWindowPrivate(qSlicerMainWindow& object);
  void setupUi(QMainWindow * mainWindow);
  
  void readSettings();
  void writeSettings();
  bool confirmClose();

  qSlicerMainWindowCore*        Core;
  qSlicerModuleSelectorToolBar* ModuleSelectorToolBar;
  QStringList                   ModuleToolBarList;
  //QSignalMapper*                ModuleToolBarMapper;
  ctkSettingsDialog*            SettingsDialog;
};

//-----------------------------------------------------------------------------
// qSlicerMainWindowPrivate methods

qSlicerMainWindowPrivate::qSlicerMainWindowPrivate(qSlicerMainWindow& object)
  : q_ptr(&object)
{
  this->Core = 0;
  this->ModuleSelectorToolBar = 0;
  this->ModuleToolBarList << "Data"  << "Volumes" << "Models" << "Transforms"
                          << "Fiducials" << "Editor" << "Measurements"
                          << "Colors";
  //this->ModuleToolBarMapper = 0;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(qSlicerMainWindow);
  
  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  // Update the list of modules when they are loaded
  //this->ModuleToolBarMapper = new QSignalMapper(p);
  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);

  QObject::connect(moduleManager,
                   SIGNAL(moduleLoaded(qSlicerAbstractCoreModule*)),
                   q, SLOT(onModuleLoaded(qSlicerAbstractCoreModule*)));

  QObject::connect(moduleManager,
                   SIGNAL(moduleAboutToBeUnloaded(qSlicerAbstractCoreModule*)),
                   q, SLOT(onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule*)));

  //QObject::connect(this->ModuleToolBarMapper, SIGNAL(mapped(const QString&)),
  //                 this->ModulePanel, SLOT(setModule(const QString&)));

  // Create a Module selector
  this->ModuleSelectorToolBar = new qSlicerModuleSelectorToolBar("Module Selector",q);
  this->ModuleSelectorToolBar->setObjectName(QString::fromUtf8("ModuleSelectorToolBar"));
  this->ModuleSelectorToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  q->insertToolBar(this->ModuleToolBar, this->ModuleSelectorToolBar);

  // Connect the selector with the module panel
  QObject::connect(this->ModuleSelectorToolBar, SIGNAL(moduleSelected(const QString&)),
                   this->ModulePanel, SLOT(setModule(const QString&)));

  // MouseMode toolBar should listen the MRML scene
  this->MouseModeToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->MouseModeToolBar,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  // Hide the Layout toolbar by default
  this->actionWindowToolbarsUndoRedo->setChecked(false);
  this->actionWindowToolbarsLayout->setChecked(false);

  // Instanciate and assign the layout manager to the slicer application
  qSlicerLayoutManager* layoutManager = new qSlicerLayoutManager(this->CentralWidget);
  layoutManager->setScriptedDisplayableManagerDirectory(
      qSlicerApplication::application()->slicerHome() + "/bin/Python/mrmlDisplayableManager");
  qSlicerApplication::application()->setLayoutManager(layoutManager);

  // Layout manager should also listen the MRML scene
  layoutManager->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   layoutManager,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
  // Slices controller toolbar listens to the MRML scene
  this->MRMLSlicesControllerToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->MRMLSlicesControllerToolBar,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
  this->MRMLSlicesControllerToolBar->setMRMLSliceLogics(layoutManager->mrmlSliceLogics());

  // ThreeDViews controller toolbar listens to LayoutManager
  // TODO The current active view could be a property of the layoutNode ?
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->MRMLThreeDViewsControllerWidget,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
  this->MRMLThreeDViewsControllerWidget->setMRMLScene(
      qSlicerApplication::application()->mrmlScene());
  QObject::connect(layoutManager, SIGNAL(activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode*)),
                   this->MRMLThreeDViewsControllerWidget,
                   SLOT(setActiveMRMLThreeDViewNode(vtkMRMLViewNode*)));
  this->MRMLThreeDViewsControllerWidget->setActiveMRMLThreeDViewNode(
      layoutManager->activeMRMLThreeDViewNode());
  QObject::connect(layoutManager, SIGNAL(activeThreeDRendererChanged(vtkRenderer*)),
                   this->MRMLThreeDViewsControllerWidget,
                   SLOT(setActiveThreeDRenderer(vtkRenderer*)));
  this->MRMLThreeDViewsControllerWidget->setActiveThreeDRenderer(
      layoutManager->activeThreeDRenderer());
  
  QObject::connect(this->MRMLThreeDViewsControllerWidget,
                   SIGNAL(screenshotButtonClicked()),
                   qSlicerApplication::application()->ioManager(),
                   SLOT(openScreenshotDialog()));

  // Populate the View ToolBar
  QToolButton* layoutButton = new QToolButton(q);
  layoutButton->setMenu(this->MenuLayout);
  layoutButton->setPopupMode(QToolButton::InstantPopup);
  layoutButton->setDefaultAction(this->actionViewLayoutConventional);
  QObject::connect(this->MenuLayout, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  this->ViewToolBar->addWidget(layoutButton);

  // Listen to the scene
  q->qvtkConnect(qSlicerApplication::application()->mrmlScene(), vtkCommand::ModifiedEvent,
                 q, SLOT(onMRMLSceneModified(vtkObject*)));
  q->onMRMLSceneModified(qSlicerApplication::application()->mrmlScene());

  // Customize QAction icons with standard pixmaps
  QIcon networkIcon = q->style()->standardIcon(QStyle::SP_DriveNetIcon);
  QIcon informationIcon = q->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  QIcon criticalIcon = q->style()->standardIcon(QStyle::SP_MessageBoxCritical);
  QIcon warningIcon = q->style()->standardIcon(QStyle::SP_MessageBoxWarning);
  QIcon questionIcon = q->style()->standardIcon(QStyle::SP_MessageBoxQuestion);

  this->actionHelpBrowseTutorials->setIcon(networkIcon);
  this->actionHelpInterfaceDocumentation->setIcon(networkIcon);
  this->actionHelpSlicerPublications->setIcon(networkIcon);
  this->actionHelpAboutSlicerQT->setIcon(informationIcon);
  this->actionFeedbackReportBug->setIcon(criticalIcon);
  this->actionFeedbackReportUsabilityIssue->setIcon(warningIcon);
  this->actionFeedbackMakeFeatureRequest->setIcon(questionIcon);
  this->actionFeedbackCommunitySlicerVisualBlog->setIcon(networkIcon);
  
  // Initialize the Settings widget
  this->SettingsDialog = new ctkSettingsDialog(q);
  this->SettingsDialog->addPanel("Application settings", new qSlicerSettingsPanel);
  this->SettingsDialog->addPanel("Modules settings", new qSlicerSettingsModulesPanel);
  this->SettingsDialog->setSettings(qSlicerCoreApplication::application()->settings());
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::readSettings()
{
  Q_Q(qSlicerMainWindow);
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
    {
    q->restoreGeometry(settings.value("geometry").toByteArray());
    q->restoreState(settings.value("windowState").toByteArray());
    }
  settings.endGroup();
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::writeSettings()
{
  Q_Q(qSlicerMainWindow);
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
    {
    settings.setValue("geometry", q->saveGeometry());
    settings.setValue("windowState", q->saveState());
    }
  settings.endGroup();
}

//-----------------------------------------------------------------------------
bool qSlicerMainWindowPrivate::confirmClose()
{
  Q_Q(qSlicerMainWindow);
  bool close = true;
  QSettings settings;
  bool confirm = settings.value("MainWindow/ConfirmExit", true).toBool();
  if (confirm)
    {
    ctkConfirmExitDialog dialog(q);
    close = (dialog.exec() == QDialog::Accepted);
    settings.setValue("MainWindow/ConfirmExit", !dialog.dontShowAnymore());
    }
  return close;
}

//-----------------------------------------------------------------------------
// qSlicerMainWindow methods

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *_parent):Superclass(_parent)
  , d_ptr(new qSlicerMainWindowPrivate(*this))
{
  Q_D(qSlicerMainWindow);
  d->setupUi(this);

  // Main window core helps to coordinate various widgets and panels
  d->Core = new qSlicerMainWindowCore(this);

  this->setupMenuActions();
  d->readSettings();

  // reading settings might have enable/disable toolbar visibility, need to
  // synchronize with the menu actions
  d->actionWindowToolbarsLoadSave->setChecked(d->MainToolBar->isVisibleTo(this));
  d->actionWindowToolbarsUndoRedo->setChecked(d->UndoRedoToolBar->isVisibleTo(this));
  d->actionWindowToolbarsLayout->setChecked(d->LayoutToolBar->isVisibleTo(this));
  d->actionWindowToolbarsView->setChecked(d->ViewToolBar->isVisibleTo(this));
  d->actionWindowToolbarsModuleSelector->setChecked(d->ModuleSelectorToolBar->isVisibleTo(this));
  d->actionWindowToolbarsModules->setChecked(d->ModuleToolBar->isVisibleTo(this));
  d->actionWindowToolbarsMouseMode->setChecked(d->MouseModeToolBar->isVisibleTo(this));
}

//-----------------------------------------------------------------------------
qSlicerMainWindow::~qSlicerMainWindow()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerMainWindow, qSlicerMainWindowCore*, core, Core);

//-----------------------------------------------------------------------------
qSlicerModuleSelectorToolBar* qSlicerMainWindow::moduleSelector()const
{
  Q_D(const qSlicerMainWindow);
  return d->ModuleSelectorToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::closeEvent(QCloseEvent *event)
{
  Q_D(qSlicerMainWindow);
  if (d->confirmClose())
    {
    d->writeSettings();
    event->accept();
    }
  else
    {
    event->ignore();
    }
}

//-----------------------------------------------------------------------------
// Helper macro allowing to connect the MainWindow action with the corresponding
// slot in MainWindowCore
#define qSlicerMainWindowCore_connect(ACTION_NAME)   \
  this->connect(                                 \
    d->action##ACTION_NAME, SIGNAL(triggered()), \
    this->core(),                                \
    SLOT(on##ACTION_NAME##ActionTriggered()));
#define qSlicerMainWindow_connect(ACTION_NAME)   \
  this->connect(                                 \
    d->action##ACTION_NAME, SIGNAL(triggered()), \
    this,                                        \
    SLOT(on##ACTION_NAME##ActionTriggered()));

//-----------------------------------------------------------------------------
void qSlicerMainWindow::setupMenuActions()
{
  Q_D(qSlicerMainWindow);
  
  qSlicerMainWindowCore_connect(FileAddData);
  qSlicerMainWindowCore_connect(FileImportScene);
  qSlicerMainWindowCore_connect(FileLoadScene);
  qSlicerMainWindowCore_connect(FileAddVolume);
  qSlicerMainWindowCore_connect(FileAddTransform);
  qSlicerMainWindowCore_connect(FileSaveScene);
  qSlicerMainWindowCore_connect(FileCloseScene);
  this->connect(d->actionFileExit, SIGNAL(triggered()),
                this, SLOT(close()));

  qSlicerMainWindowCore_connect(EditUndo);
  qSlicerMainWindowCore_connect(EditRedo);

  qSlicerMainWindow_connect(ViewExtensionManager);
  qSlicerMainWindow_connect(ViewApplicationSettings);
  qSlicerMainWindowCore_connect(ViewLayoutConventional);
  qSlicerMainWindowCore_connect(ViewLayoutFourUp);
  qSlicerMainWindowCore_connect(ViewLayoutDual3D);
  qSlicerMainWindowCore_connect(ViewLayoutOneUp3D);
  qSlicerMainWindowCore_connect(ViewLayoutOneUpRedSlice);
  qSlicerMainWindowCore_connect(ViewLayoutOneUpYellowSlice);
  qSlicerMainWindowCore_connect(ViewLayoutOneUpGreenSlice);
  qSlicerMainWindowCore_connect(ViewLayoutTabbed3D);
  qSlicerMainWindowCore_connect(ViewLayoutTabbedSlice);
  qSlicerMainWindowCore_connect(ViewLayoutCompare);
  qSlicerMainWindowCore_connect(ViewLayoutSideBySideLightbox);
  
  qSlicerMainWindowCore_connect(WindowPythonInteractor);

  qSlicerMainWindowCore_connect(HelpKeyboardShortcuts);
  qSlicerMainWindowCore_connect(HelpBrowseTutorials);
  qSlicerMainWindowCore_connect(HelpInterfaceDocumentation);
  qSlicerMainWindowCore_connect(HelpSlicerPublications);
  qSlicerMainWindowCore_connect(HelpAboutSlicerQT);

  qSlicerMainWindowCore_connect(FeedbackReportBug);
  qSlicerMainWindowCore_connect(FeedbackReportUsabilityIssue);
  qSlicerMainWindowCore_connect(FeedbackMakeFeatureRequest);
  qSlicerMainWindowCore_connect(FeedbackCommunitySlicerVisualBlog);

  //connect ToolBars actions
  connect(d->actionWindowToolbarsModuleSelector, SIGNAL(toggled(bool)),
          d->ModuleSelectorToolBar, SLOT(setVisible(bool)));
}
#undef qSlicerMainWindowCore_connect

//---------------------------------------------------------------------------
void qSlicerMainWindow::onViewExtensionManagerActionTriggered()
{
  qSlicerExtensionsWizard extensionsManager(this);
  extensionsManager.exec();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onViewApplicationSettingsActionTriggered()
{
  Q_D(qSlicerMainWindow);
  d->SettingsDialog->exec();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleLoaded(qSlicerAbstractCoreModule* coreModule)
{
  Q_D(qSlicerMainWindow);
  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(coreModule);
  if (!module)
    {
    return;
    }

  // Module Selector ToolBar
  d->ModuleSelectorToolBar->addModule(module->name());

  // Module ToolBar
  QAction * action = module->action();
  if (!action || action->icon().isNull())
    {
    return;
    }

  Q_ASSERT(action->data().toString() == module->name());
  Q_ASSERT(action->text() == module->title());

  // Add action to ToolBar if it's an "allowed" action
  int index = d->ModuleToolBarList.indexOf(module->title());
  if (index > 0)
    {
    // find the location of where to add the action. ModelToolBarList is sorted
    QAction* beforeAction = 0;
    foreach(QAction* toolBarAction, d->ModuleToolBar->actions())
      {
      Q_ASSERT(d->ModuleToolBarList.contains(toolBarAction->text()));
      if (d->ModuleToolBarList.indexOf(toolBarAction->text()) > index)
        {
        beforeAction = toolBarAction;
        }
      }
    d->ModuleToolBar->insertAction(beforeAction, action);
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule* module)
{
  Q_D(qSlicerMainWindow);
  if (!module)
    {
    return;
    }

  foreach(QAction* action, d->ModuleToolBar->actions())
    {
    if (action->data().toString() == module->name())
      {
      d->ModuleToolBar->removeAction(action);
      d->ModuleSelectorToolBar->removeModule(module->name());
      //d->ModuleToolBarMapper->removeMappings(action);
      return;
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onMRMLSceneModified(vtkObject* sender)
{
  Q_D(qSlicerMainWindow);

  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sender);
  if (scene->GetIsUpdating())
    {
    return;
    }
  d->actionEditUndo->setEnabled(scene && scene->GetNumberOfUndoLevels());
  d->actionEditRedo->setEnabled(scene && scene->GetNumberOfRedoLevels());
}
