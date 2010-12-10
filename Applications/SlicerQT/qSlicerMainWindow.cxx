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
  qSlicerModuleSelectorToolBar* ModuleSelector;
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
  this->ModuleSelector = 0;
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
  this->ModuleSelector = new qSlicerModuleSelectorToolBar("Module Selector",q);
  this->ModuleSelector->setObjectName(QString::fromUtf8("ModuleSelectorToolBar"));
  this->ModuleSelector->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  q->insertToolBar(this->ModuleToolBar, this->ModuleSelector);

  // Connect the selector with the module panel
  QObject::connect(this->ModuleSelector, SIGNAL(moduleSelected(const QString&)),
                   this->ModulePanel, SLOT(setModule(const QString&)));

  // MouseMode toolBar should listen the MRML scene
  this->MouseModeToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->MouseModeToolBar,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  // Hide the Layout toolbar by default
  q->showUndoRedoToolBar(false);
  q->showLayoutToolBar(false);

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
  return d->ModuleSelector;
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
  connect(d->actionWindowToolbarsLoadSave, SIGNAL(toggled(bool)),
          this, SLOT(showMainToolBar(bool)));
  connect(d->actionWindowToolbarsUndoRedo, SIGNAL(toggled(bool)),
          this, SLOT(showUndoRedoToolBar(bool)));
  connect(d->actionWindowToolbarsLayout, SIGNAL(toggled(bool)),
          this, SLOT(showLayoutToolBar(bool)));
  connect(d->actionWindowToolbarsView, SIGNAL(toggled(bool)),
          this, SLOT(showViewToolBar(bool)));
  connect(d->actionWindowToolbarsMouseMode, SIGNAL(toggled(bool)),
          this, SLOT(showMouseModeToolBar(bool)));
  connect(d->actionWindowToolbarsModules, SIGNAL(toggled(bool)),
          this, SLOT(showModuleToolBar(bool)));
  connect(d->actionWindowToolbarsModuleSelector, SIGNAL(toggled(bool)),
          this, SLOT(showModuleSelectorToolBar(bool)));
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
  d->ModuleSelector->addModule(module->name());

  // Module ToolBar
  QAction * action = module->action();
  if (!action || action->icon().isNull())
    {
    return;
    }

  Q_ASSERT(action->data().toString() == module->name());
  Q_ASSERT(action->text() == module->title());

  // Add action to ToolBar
  if (d->ModuleToolBarList.contains(module->title()))
    {
    d->ModuleToolBar->addAction(action);
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
      d->ModuleSelector->removeModule(module->name());
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

//---------------------------------------------------------------------------
void qSlicerMainWindow::showMainToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsLoadSave->setChecked(visible);
  d->MainToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showUndoRedoToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsUndoRedo->setChecked(visible);
  d->UndoRedoToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showViewToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsView->setChecked(visible);
  d->ViewToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showLayoutToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsLayout->setChecked(visible);
  d->LayoutToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showMouseModeToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsMouseMode->setChecked(visible);
  d->MouseModeToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showModuleToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsModules->setChecked(visible);
  d->ModuleToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showModuleSelectorToolBar(bool visible)
{
  Q_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsModuleSelector->setChecked(visible);
  d->ModuleSelector->setVisible(visible);
}
