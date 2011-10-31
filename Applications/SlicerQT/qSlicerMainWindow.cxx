/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QTimer>
#include <QToolButton>
#include <QMenu>

// CTK includes
#include <ctkConfirmExitDialog.h>
#include <ctkSettingsDialog.h>
#include <ctkVTKMagnifyView.h>
#include <ctkVTKSliceView.h>

// SlicerQt includes
#include "qSlicerMainWindow.h"
#include "ui_qSlicerMainWindow.h"
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerExtensionsWizard.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerIOManager.h"
#include "qSlicerSettingsModulesPanel.h"
#include "qSlicerSettingsGeneralPanel.h"

#ifdef Slicer_USE_PYTHONQT
#include "qSlicerSettingsPythonPanel.h"
#endif

// qMRML includes
#include <qMRMLSliceWidget.h>

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes

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
  qSlicerLayoutManager*         LayoutManager;

  QByteArray                    StartupState;
};

//-----------------------------------------------------------------------------
// qSlicerMainWindowPrivate methods

qSlicerMainWindowPrivate::qSlicerMainWindowPrivate(qSlicerMainWindow& object)
  : q_ptr(&object)
{
  this->Core = 0;
  this->ModuleSelectorToolBar = 0;
  // Title of the modules sorted by appearance order
  this->ModuleToolBarList << "Home" << "Data"  << "Volumes" << "Models" << "Transforms"
                          << "Annotations" << "Editor" << "Measurements";

  this->LayoutManager = 0;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(qSlicerMainWindow);

  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  //----------------------------------------------------------------------------
  // ModuleManager
  //----------------------------------------------------------------------------
  // Update the list of modules when they are loaded
  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  if (!moduleManager)
    {
    qWarning() << "No module manager is created.";
    }

  QObject::connect(moduleManager,
                   SIGNAL(moduleLoaded(qSlicerAbstractCoreModule*)),
                   q, SLOT(onModuleLoaded(qSlicerAbstractCoreModule*)));

  QObject::connect(moduleManager,
                   SIGNAL(moduleAboutToBeUnloaded(qSlicerAbstractCoreModule*)),
                   q, SLOT(onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule*)));

  //----------------------------------------------------------------------------
  // ModuleSelector ToolBar
  //----------------------------------------------------------------------------
  // Create a Module selector
  this->ModuleSelectorToolBar = new qSlicerModuleSelectorToolBar("Module Selector",q);
  this->ModuleSelectorToolBar->setObjectName(QString::fromUtf8("ModuleSelectorToolBar"));
  this->ModuleSelectorToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  this->ModuleSelectorToolBar->setModuleManager(moduleManager);
  q->insertToolBar(this->ModuleToolBar, this->ModuleSelectorToolBar);

  // Connect the selector with the module panel
  this->ModulePanel->setModuleManager(moduleManager);
  QObject::connect(this->ModuleSelectorToolBar, SIGNAL(moduleSelected(QString)),
                   this->ModulePanel, SLOT(setModule(QString)));

  //----------------------------------------------------------------------------
  // MouseMode ToolBar
  //----------------------------------------------------------------------------
  // MouseMode toolBar should listen the MRML scene
  this->MouseModeToolBar->setApplicationLogic(
    qSlicerApplication::application()->applicationLogic());
  this->MouseModeToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->MouseModeToolBar,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
  //----------------------------------------------------------------------------
  // Capture tool bar
  //----------------------------------------------------------------------------
  // Capture bar needs to listen to the MRML scene and the layout
  this->CaptureToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->CaptureToolBar,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
  this->CaptureToolBar->setMRMLScene(
      qSlicerApplication::application()->mrmlScene());

  QObject::connect(this->CaptureToolBar,
                   SIGNAL(screenshotButtonClicked()),
                   qSlicerApplication::application()->ioManager(),
                   SLOT(openScreenshotDialog()));

  // to get the scene views module dialog to pop up when a button is pressed
  // in the capture tool bar, we emit a signal, and the
  // io manager will deal with the sceneviews module
  QObject::connect(this->CaptureToolBar,
                   SIGNAL(sceneViewButtonClicked()),
                   qSlicerApplication::application()->ioManager(),
                   SLOT(openSceneViewsDialog()));

  //----------------------------------------------------------------------------
  // Hide toolbars by default
  //----------------------------------------------------------------------------
  // Hide the Layout toolbar by default
  // The setVisibility slot of the toolbar is connected QAction::triggered signal
  // It's done for a long list of reasons. If you change this behavior, make sure
  // minimizing the application and restore it doesn't hide the module panel. check
  // also the geometry and the state of the menu qactions are correctly restored when
  // loading slicer.
  this->actionWindowToolbarsUndoRedo->trigger();
  this->actionWindowToolbarsLayout->trigger();

  // Color of the spacing between views:
  QFrame* layoutFrame = new QFrame(this->CentralWidget);
  QHBoxLayout* centralLayout = new QHBoxLayout(this->CentralWidget);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->addWidget(layoutFrame);

  QColor windowColor = this->CentralWidget->palette().color(QPalette::Window);
  QPalette centralPalette = this->CentralWidget->palette();
  centralPalette.setColor(QPalette::Window, QColor(95, 95, 113));
  this->CentralWidget->setAutoFillBackground(true);
  this->CentralWidget->setPalette(centralPalette);

  // Restore the palette for the children
  centralPalette.setColor(QPalette::Window, windowColor);
  layoutFrame->setPalette(centralPalette);
  layoutFrame->setAttribute(Qt::WA_NoSystemBackground, true);
  layoutFrame->setAttribute(Qt::WA_TranslucentBackground, true);

  //----------------------------------------------------------------------------
  // Layout Manager
  //----------------------------------------------------------------------------
  // Instanciate and assign the layout manager to the slicer application
  this->LayoutManager = new qSlicerLayoutManager(layoutFrame);
  this->LayoutManager->setScriptedDisplayableManagerDirectory(
      qSlicerApplication::application()->slicerHome() + "/bin/Python/mrmlDisplayableManager");
  qSlicerApplication::application()->setLayoutManager(this->LayoutManager);
  // Layout manager should also listen the MRML scene
  this->LayoutManager->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->LayoutManager,
                   SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(this->LayoutManager, SIGNAL(layoutChanged(int)),
                   q, SLOT(onLayoutChanged(int)));

  // Add menus for configuring compare view
  QMenu *compareMenu = new QMenu(q->tr("Select number of viewers..."));
  compareMenu->addAction(this->actionViewLayoutCompare_2_viewers);
  compareMenu->addAction(this->actionViewLayoutCompare_3_viewers);
  compareMenu->addAction(this->actionViewLayoutCompare_4_viewers);
  compareMenu->addAction(this->actionViewLayoutCompare_5_viewers);
  compareMenu->addAction(this->actionViewLayoutCompare_6_viewers);
  compareMenu->addAction(this->actionViewLayoutCompare_7_viewers);
  compareMenu->addAction(this->actionViewLayoutCompare_8_viewers);
  this->actionViewLayoutCompare->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareActionTriggered(QAction*)));

  // ... and for widescreen version of compare view as well
  compareMenu = new QMenu(q->tr("Select number of viewers..."));
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_2_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_3_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_4_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_5_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_6_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_7_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareWidescreen_8_viewers);
  this->actionViewLayoutCompareWidescreen->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareWidescreenActionTriggered(QAction*)));

  // ... and for the grid version of the compare views
  compareMenu = new QMenu(q->tr("Select number of viewers..."));
  compareMenu->addAction(this->actionViewLayoutCompareGrid_2x2_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareGrid_3x3_viewers);
  compareMenu->addAction(this->actionViewLayoutCompareGrid_4x4_viewers);
  this->actionViewLayoutCompareGrid->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareGridActionTriggered(QAction*)));


  // Capture tool bar needs to listen to the layout manager
  QObject::connect(this->LayoutManager,
                   SIGNAL(activeMRMLThreeDViewNodeChanged(vtkMRMLViewNode*)),
                   this->CaptureToolBar,
                   SLOT(setActiveMRMLThreeDViewNode(vtkMRMLViewNode*)));
  this->CaptureToolBar->setActiveMRMLThreeDViewNode(
      this->LayoutManager->activeMRMLThreeDViewNode());

  //----------------------------------------------------------------------------
  // View Toolbar
  //----------------------------------------------------------------------------
  // Populate the View ToolBar with all the layouts of the layout manager
  QToolButton* layoutButton = new QToolButton(q);
  layoutButton->setMenu(this->MenuLayout);
  layoutButton->setPopupMode(QToolButton::InstantPopup);
  layoutButton->setDefaultAction(this->actionViewLayoutConventional);
  QObject::connect(this->MenuLayout, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->MenuLayout, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));
  this->ViewToolBar->addWidget(layoutButton);

  //----------------------------------------------------------------------------
  // Viewers Toolbar
  //----------------------------------------------------------------------------
  // Viewers toolBar should listen the MRML scene
  this->ViewersToolBar->setApplicationLogic(
    qSlicerApplication::application()->applicationLogic());
  this->ViewersToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  QObject::connect(qSlicerApplication::application(),
                   SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ViewersToolBar,
                   SLOT(setMRMLScene(vtkMRMLScene*)));

  //----------------------------------------------------------------------------
  // Undo/Redo Toolbar
  //----------------------------------------------------------------------------
  // Listen to the scene to enable/disable the undo/redo toolbuttons
  //q->qvtkConnect(qSlicerApplication::application()->mrmlScene(), vtkCommand::ModifiedEvent,
  //               q, SLOT(onMRMLSceneModified(vtkObject*)));
  //q->onMRMLSceneModified(qSlicerApplication::application()->mrmlScene());

  //----------------------------------------------------------------------------
  // Icons in the menu
  //----------------------------------------------------------------------------
  // Customize QAction icons with standard pixmaps
  // TODO: all these icons are a little bit too much.
  QIcon networkIcon = q->style()->standardIcon(QStyle::SP_DriveNetIcon);
  QIcon informationIcon = q->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  QIcon questionIcon = q->style()->standardIcon(QStyle::SP_MessageBoxQuestion);

  this->actionHelpBrowseTutorials->setIcon(networkIcon);
  this->actionHelpInterfaceDocumentation->setIcon(networkIcon);
  this->actionHelpSlicerPublications->setIcon(networkIcon);
  this->actionHelpAboutSlicerQT->setIcon(informationIcon);
  this->actionHelpReportBugOrFeatureRequest->setIcon(questionIcon);
  this->actionHelpVisualBlog->setIcon(networkIcon);
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
    this->LayoutManager->setLayout(settings.value("layout").toInt());
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
    settings.setValue("layout", this->LayoutManager->layout());
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
  d->StartupState = this->saveState();
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
  d->actionWindowToolbarsCapture->setChecked(d->CaptureToolBar->isVisibleTo(this));
  d->actionWindowToolbarsViewers->setChecked(d->ViewersToolBar->isVisibleTo(this));
}

//-----------------------------------------------------------------------------
qSlicerMainWindow::~qSlicerMainWindow()
{
  Q_D(qSlicerMainWindow);
  // When quitting the application, the modules are unloaded (~qSlicerCoreApplication)
  // in particular the Colors module which deletes vtkMRMLColorLogic and removes
  // all the color nodes from the scene. If a volume was loaded in the views,
  // it would then try to render it with no color node and generate warnings.
  // There is no need to render anything so remove the volumes from the views.
  // It is maybe not the best place to do that but I couldn't think of anywhere
  // else.
  vtkCollection* sliceLogics = d->LayoutManager ? d->LayoutManager->mrmlSliceLogics() : 0;
  for (int i = 0; i < sliceLogics->GetNumberOfItems(); ++i)
    {
    vtkMRMLSliceLogic* sliceLogic = vtkMRMLSliceLogic::SafeDownCast(sliceLogics->GetItemAsObject(i));
    if (!sliceLogic)
      {
      continue;
      }
    sliceLogic->GetSliceCompositeNode()->SetReferenceBackgroundVolumeID(0);
    sliceLogic->GetSliceCompositeNode()->SetReferenceForegroundVolumeID(0);
    sliceLogic->GetSliceCompositeNode()->SetReferenceLabelVolumeID(0);
    }
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
  if (event->isAccepted())
    {
    QTimer::singleShot(0, qApp, SLOT(closeAllWindows()));
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

  qSlicerMainWindowCore_connect(SDBSaveToDirectory);
  qSlicerMainWindowCore_connect(SDBZipDirectory);
  qSlicerMainWindowCore_connect(SDBZipToDCM);

  qSlicerMainWindowCore_connect(FileCloseScene);
  this->connect(d->actionFileExit, SIGNAL(triggered()),
                this, SLOT(close()));

  qSlicerMainWindowCore_connect(EditUndo);
  qSlicerMainWindowCore_connect(EditRedo);

  qSlicerMainWindow_connect(EditApplicationSettings);

  qSlicerMainWindow_connect(ViewExtensionManager);

  d->actionViewLayoutConventional->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  d->actionViewLayoutConventionalWidescreen->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalWidescreenView);
  d->actionViewLayoutFourUp->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
  d->actionViewLayoutDual3D->setData(vtkMRMLLayoutNode::SlicerLayoutDual3DView);
  d->actionViewLayoutTriple3D->setData(vtkMRMLLayoutNode::SlicerLayoutTriple3DEndoscopyView);
  d->actionViewLayoutOneUp3D->setData(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
  d->actionViewLayoutOneUpRedSlice->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  d->actionViewLayoutOneUpYellowSlice->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
  d->actionViewLayoutOneUpGreenSlice->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  d->actionViewLayoutTabbed3D->setData(vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
  d->actionViewLayoutTabbedSlice->setData(vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);
  d->actionViewLayoutCompare->setData(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  d->actionViewLayoutCompareWidescreen->setData(vtkMRMLLayoutNode::SlicerLayoutCompareWidescreenView);
  d->actionViewLayoutCompareGrid->setData(vtkMRMLLayoutNode::SlicerLayoutCompareGridView);
  d->actionViewLayoutThreeOverThree->setData(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeView);
  d->actionViewLayoutFourOverFour->setData(vtkMRMLLayoutNode::SlicerLayoutFourOverFourView);

  d->actionViewLayoutCompare_2_viewers->setData(2);
  d->actionViewLayoutCompare_3_viewers->setData(3);
  d->actionViewLayoutCompare_4_viewers->setData(4);
  d->actionViewLayoutCompare_5_viewers->setData(5);
  d->actionViewLayoutCompare_6_viewers->setData(6);
  d->actionViewLayoutCompare_7_viewers->setData(7);
  d->actionViewLayoutCompare_8_viewers->setData(8);

  d->actionViewLayoutCompareWidescreen_2_viewers->setData(2);
  d->actionViewLayoutCompareWidescreen_3_viewers->setData(3);
  d->actionViewLayoutCompareWidescreen_4_viewers->setData(4);
  d->actionViewLayoutCompareWidescreen_5_viewers->setData(5);
  d->actionViewLayoutCompareWidescreen_6_viewers->setData(6);
  d->actionViewLayoutCompareWidescreen_7_viewers->setData(7);
  d->actionViewLayoutCompareWidescreen_8_viewers->setData(8);

  d->actionViewLayoutCompareGrid_2x2_viewers->setData(2);
  d->actionViewLayoutCompareGrid_3x3_viewers->setData(3);
  d->actionViewLayoutCompareGrid_4x4_viewers->setData(4);


  qSlicerMainWindowCore_connect(WindowErrorLog);
  qSlicerMainWindowCore_connect(WindowPythonInteractor);

  qSlicerMainWindowCore_connect(HelpKeyboardShortcuts);
  qSlicerMainWindowCore_connect(HelpBrowseTutorials);
  qSlicerMainWindowCore_connect(HelpInterfaceDocumentation);
  qSlicerMainWindowCore_connect(HelpSlicerPublications);
  qSlicerMainWindowCore_connect(HelpAboutSlicerQT);

  qSlicerMainWindowCore_connect(HelpReportBugOrFeatureRequest);
  qSlicerMainWindowCore_connect(HelpVisualBlog);

  //connect ToolBars actions
  connect(d->actionWindowToolbarsModuleSelector, SIGNAL(triggered(bool)),
          d->ModuleSelectorToolBar, SLOT(setVisible(bool)));
  connect(d->actionWindowToolbarsResetToDefault, SIGNAL(triggered()),
          this, SLOT(restoreToolbars()));

  // Module ToolBar actions
  connect(d->actionModuleHome, SIGNAL(triggered()),
          this, SLOT(setHomeModuleCurrent()));

}
#undef qSlicerMainWindowCore_connect

//---------------------------------------------------------------------------
void qSlicerMainWindow::onEditApplicationSettingsActionTriggered()
{
  qSlicerApplication::application()->settingsDialog()->exec();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onViewExtensionManagerActionTriggered()
{
  qSlicerExtensionsWizard extensionsManager(this);
  extensionsManager.exec();
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
      return;
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onMRMLSceneModified(vtkObject* sender)
{
  Q_UNUSED(sender);
  //Q_D(qSlicerMainWindow);
  //
  //vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sender);
  //if (scene && scene->GetIsUpdating())
  //  {
  //  return;
  //  }
  //d->actionEditUndo->setEnabled(scene && scene->GetNumberOfUndoLevels());
  //d->actionEditRedo->setEnabled(scene && scene->GetNumberOfRedoLevels());
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);
  bool found = false;
  // std::cerr << "onLayoutActionTriggered: " << action->text().toStdString() << std::endl;
  foreach(QAction* maction, d->MenuLayout->actions())
    {
    if (action->text() == maction->text())
      {
      found = true;
      break;
      }
    }

  if (found)
    {
    this->core()->setLayout(action->data().toInt());
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutCompareActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);

  // std::cerr << "onLayoutCompareActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->core()->setLayout(d->actionViewLayoutCompare->data().toInt());
  this->core()->setLayoutNumberOfCompareViewRows(action->data().toInt());
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutCompareWidescreenActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);

  // std::cerr << "onLayoutCompareWidescreenActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->core()->setLayout(d->actionViewLayoutCompareWidescreen->data().toInt());
  this->core()->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutCompareGridActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);

  // std::cerr << "onLayoutCompareGridActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->core()->setLayout(d->actionViewLayoutCompareGrid->data().toInt());
  this->core()->setLayoutNumberOfCompareViewRows(action->data().toInt());
  this->core()->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}


//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutChanged(int layout)
{
  Q_D(qSlicerMainWindow);
  // std::cerr << "onLayoutChanged: " << layout << std::endl;

  // Trigger the action associated with the new layout
  foreach(QAction* action, d->MenuLayout->actions())
    {
    if (action->data().toInt() == layout)
      {
      action->trigger();
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::setHomeModuleCurrent()
{
  Q_D(qSlicerMainWindow);
  QSettings settings;
  QString homeModule = settings.value("Modules/HomeModule").toString();
  d->ModuleSelectorToolBar->selectModule(homeModule);
}


//---------------------------------------------------------------------------
void qSlicerMainWindow::restoreToolbars()
{
  Q_D(qSlicerMainWindow);
  this->restoreState(d->StartupState);
}
