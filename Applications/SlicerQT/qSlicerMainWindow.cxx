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
#include <QDebug>
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
  this->ModuleSelector->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  p->insertToolBar(this->ModuleToolBar, this->ModuleSelector);

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
  p->showUndoRedoToolBar(false);
  p->showLayoutToolBar(false);

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
  QObject::connect(layoutManager, SIGNAL(activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode*)),
                   this->MRMLThreeDViewsControllerWidget,
                   SLOT(setActiveMRMLThreeDViewNode(vtkMRMLViewNode*)));
  this->MRMLThreeDViewsControllerWidget->setActiveMRMLThreeDViewNode(
      layoutManager->activeMRMLThreeDViewNode());


  // Populate the View ToolBar
  QToolButton* layoutButton = new QToolButton(p);
  layoutButton->setMenu(this->MenuLayout);
  layoutButton->setPopupMode(QToolButton::InstantPopup);
  layoutButton->setDefaultAction(this->actionViewLayoutConventional);
  QObject::connect(this->MenuLayout, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  this->ViewToolBar->addWidget(layoutButton);

  // Listen to the scene
  p->qvtkConnect(qSlicerApplication::application()->mrmlScene(), vtkCommand::ModifiedEvent,
                 p, SLOT(onMRMLSceneModified(vtkObject*)));
  p->onMRMLSceneModified(qSlicerApplication::application()->mrmlScene());

  // Customize QAction icons with standard pixmaps
  QIcon networkIcon = p->style()->standardIcon(QStyle::SP_DriveNetIcon);
  QIcon informationIcon = p->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  QIcon criticalIcon = p->style()->standardIcon(QStyle::SP_MessageBoxCritical);
  QIcon warningIcon = p->style()->standardIcon(QStyle::SP_MessageBoxWarning);
  QIcon questionIcon = p->style()->standardIcon(QStyle::SP_MessageBoxQuestion);

  this->actionHelpBrowseTutorials->setIcon(networkIcon);
  this->actionHelpInterfaceDocumentation->setIcon(networkIcon);
  this->actionHelpSlicerPublications->setIcon(networkIcon);
  this->actionHelpAboutSlicerQT->setIcon(informationIcon);

  this->actionFeedbackReportBug->setIcon(criticalIcon);
  this->actionFeedbackReportUsabilityIssue->setIcon(warningIcon);
  this->actionFeedbackMakeFeatureRequest->setIcon(questionIcon);
  this->actionFeedbackCommunitySlicerVisualBlog->setIcon(networkIcon);
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

  qSlicerMainWindow_connect(HelpKeyboardShortcuts);
  qSlicerMainWindow_connect(HelpBrowseTutorials);
  qSlicerMainWindow_connect(HelpInterfaceDocumentation);
  qSlicerMainWindow_connect(HelpSlicerPublications);
  qSlicerMainWindow_connect(HelpAboutSlicerQT);

  qSlicerMainWindow_connect(FeedbackReportBug);
  qSlicerMainWindow_connect(FeedbackReportUsabilityIssue);
  qSlicerMainWindow_connect(FeedbackMakeFeatureRequest);
  qSlicerMainWindow_connect(FeedbackCommunitySlicerVisualBlog);

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

  // Add action to ToolBar
  if (d->ModuleToolBarList.contains(module->title()))
    {
    d->ModuleToolBar->addAction(action);
    }
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

//---------------------------------------------------------------------------
void qSlicerMainWindow::showMainToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsLoadSave->setChecked(visible);
  d->MainToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showUndoRedoToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsUndoRedo->setChecked(visible);
  d->UndoRedoToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showViewToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsView->setChecked(visible);
  d->ViewToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showLayoutToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsLayout->setChecked(visible);
  d->LayoutToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showMouseModeToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsMouseMode->setChecked(visible);
  d->MouseModeToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showModuleToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsModules->setChecked(visible);
  d->ModuleToolBar->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::showModuleSelectorToolBar(bool visible)
{
  CTK_D(qSlicerMainWindow);
  // set the action state just in case the slot has been called by something
  // else than the toolbar QAction
  d->actionWindowToolbarsModuleSelector->setChecked(visible);
  d->ModuleSelector->setVisible(visible);
}
