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
#include <QKeySequence>
#include <QSettings>
#include <QShowEvent>
#include <QTimer>
#include <QToolButton>
#include <QMenu>

#include "qSlicerApplication.h" // Indirectly includes vtkSlicerConfigure.h

// CTK includes
#include <ctkErrorLogWidget.h>
#include <ctkMessageBox.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif
#include <ctkSettingsDialog.h>
#include <ctkVTKSliceView.h>

// SlicerQt includes
#include "qSlicerMainWindow.h"
#include "ui_qSlicerMainWindow.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreCommandOptions.h"
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
# include "qSlicerExtensionsManagerDialog.h"
#endif
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerMainWindowCore.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerIOManager.h"

// qMRML includes
#include <qMRMLSliceWidget.h>

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkCollection.h>

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
  QStringList                   FavoriteModules;
  qSlicerLayoutManager*         LayoutManager;

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerExtensionsManagerDialog* ExtensionsManagerDialog;
#endif

  QByteArray                    StartupState;
};

//-----------------------------------------------------------------------------
// qSlicerMainWindowPrivate methods

qSlicerMainWindowPrivate::qSlicerMainWindowPrivate(qSlicerMainWindow& object)
  : q_ptr(&object)
{
  this->Core = 0;
  this->ModuleSelectorToolBar = 0;
  this->LayoutManager = 0;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(qSlicerMainWindow);

  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  //----------------------------------------------------------------------------
  // ModulePanel
  //----------------------------------------------------------------------------
  this->PanelDockWidget->toggleViewAction()->setText("&Module Panel");
  this->PanelDockWidget->toggleViewAction()->setToolTip("Collapse/Expand the GUI panel and allows Slicer's viewers to occupy the entire application window");
  this->PanelDockWidget->toggleViewAction()->setShortcut(QKeySequence("Ctrl+5"));
  this->menuView->insertAction(this->menuWindowToolBars->menuAction(), this->PanelDockWidget->toggleViewAction());

  //----------------------------------------------------------------------------
  // ModuleManager
  //----------------------------------------------------------------------------
  // Update the list of modules when they are loaded
  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  if (!moduleManager)
    {
    qWarning() << "No module manager is created.";
    }

  QObject::connect(moduleManager,SIGNAL(moduleLoaded(QString)),
                   q, SLOT(onModuleLoaded(QString)));

  QObject::connect(moduleManager, SIGNAL(moduleAboutToBeUnloaded(QString)),
                   q, SLOT(onModuleAboutToBeUnloaded(QString)));

  //----------------------------------------------------------------------------
  // ModuleSelector ToolBar
  //----------------------------------------------------------------------------
  // Create a Module selector
  this->ModuleSelectorToolBar = new qSlicerModuleSelectorToolBar("Module Selection",q);
  this->ModuleSelectorToolBar->setObjectName(QString::fromUtf8("ModuleSelectorToolBar"));
  this->ModuleSelectorToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  this->ModuleSelectorToolBar->setModuleManager(moduleManager);
  q->insertToolBar(this->ModuleToolBar, this->ModuleSelectorToolBar);
  this->ModuleSelectorToolBar->stackUnder(this->ModuleToolBar);

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

  QList<QAction*> toolBarActions;
  toolBarActions << this->MainToolBar->toggleViewAction();
  //toolBarActions << this->UndoRedoToolBar->toggleViewAction();
  toolBarActions << this->ModuleSelectorToolBar->toggleViewAction();
  toolBarActions << this->ModuleToolBar->toggleViewAction();
  toolBarActions << this->ViewToolBar->toggleViewAction();
  //toolBarActions << this->LayoutToolBar->toggleViewAction();
  toolBarActions << this->MouseModeToolBar->toggleViewAction();
  toolBarActions << this->CaptureToolBar->toggleViewAction();
  toolBarActions << this->ViewersToolBar->toggleViewAction();

  this->menuWindowToolBars->insertActions(
    this->actionWindowToolbarsResetToDefault, toolBarActions);
  this->menuWindowToolBars->insertSeparator(
    this->actionWindowToolbarsResetToDefault);
  //----------------------------------------------------------------------------
  // Hide toolbars by default
  //----------------------------------------------------------------------------
  // Hide the Layout toolbar by default
  // The setVisibility slot of the toolbar is connected QAction::triggered signal
  // It's done for a long list of reasons. If you change this behavior, make sure
  // minimizing the application and restore it doesn't hide the module panel. check
  // also the geometry and the state of the menu qactions are correctly restored when
  // loading slicer.
  this->UndoRedoToolBar->toggleViewAction()->trigger();
  this->LayoutToolBar->toggleViewAction()->trigger();
  //q->removeToolBar(this->UndoRedoToolBar);
  //q->removeToolBar(this->LayoutToolBar);
  delete this->UndoRedoToolBar;
  this->UndoRedoToolBar = 0;
  delete this->LayoutToolBar;
  this->LayoutToolBar = 0;

  // Color of the spacing between views:
  QFrame* layoutFrame = new QFrame(this->CentralWidget);
  layoutFrame->setObjectName("CentralWidgetLayoutFrame");
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

  // TODO: When module will be managed by the layoutManager, this should be
  //       revisited.
  QObject::connect(this->LayoutManager, SIGNAL(selectModule(QString)),
                   this->ModuleSelectorToolBar, SLOT(selectModule(QString)));

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

  // Authorize Drops action from outside
  q->setAcceptDrops(true);

  //----------------------------------------------------------------------------
  // View Toolbar
  //----------------------------------------------------------------------------
  // Populate the View ToolBar with all the layouts of the layout manager
  QToolButton* layoutButton = new QToolButton(q);
  layoutButton->setText(q->tr("Layout"));
  layoutButton->setMenu(this->MenuLayout);
  layoutButton->setPopupMode(QToolButton::InstantPopup);
  layoutButton->setDefaultAction(this->actionViewLayoutConventional);
  QObject::connect(this->MenuLayout, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->MenuLayout, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));

  this->ViewToolBar->addWidget(layoutButton);
  QObject::connect(this->ViewToolBar,
                   SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   layoutButton, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

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

  //----------------------------------------------------------------------------
  // Dialogs
  //----------------------------------------------------------------------------

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  this->ExtensionsManagerDialog = new qSlicerExtensionsManagerDialog(q);
#endif
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
  this->FavoriteModules << settings.value("Modules/FavoriteModules").toStringList();
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
  return ctkMessageBox::confirmExit("MainWindow/DontConfirmExit", q);
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
    // Exit current module to leave it a chance to change the UI (e.g. layout)
    // before writting settings.
    d->ModuleSelectorToolBar->selectModule("");

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
void qSlicerMainWindow::showEvent(QShowEvent *event)
{
  this->Superclass::showEvent(event);
  if (!event->spontaneous())
    {
    this->disclaimer();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::disclaimer()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (app->testAttribute(qSlicerCoreApplication::AA_EnableTesting) ||
      !app->coreCommandOptions()->pythonCode().isEmpty() ||
      !app->coreCommandOptions()->pythonScript().isEmpty())
    {
    return;
    }
  QString message = QString("Thank you for using %1!\n\n"
                            "This software is not intended for clinical use.")
    .arg(app->applicationName() + " " + app->applicationVersion());

  ctkMessageBox* disclaimerMessage = new ctkMessageBox(this);
  disclaimerMessage->setAttribute( Qt::WA_DeleteOnClose, true );
  disclaimerMessage->setText(message);
  disclaimerMessage->setIcon(QMessageBox::Information);
  disclaimerMessage->setDontShowAgainSettingsKey("MainWindow/DontShowDisclaimerMessage");
  QTimer::singleShot(0, disclaimerMessage, SLOT(exec()));
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
  qSlicerMainWindowCore_connect(FileLoadData);
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
  d->actionViewLayoutConventionalQuantitative->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalQuantitativeView);
  d->actionViewLayoutFourUp->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
  d->actionViewLayoutFourUpQuantitative->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpQuantitativeView);
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
  d->actionViewLayoutTwoOverTwo->setData(vtkMRMLLayoutNode::SlicerLayoutTwoOverTwoView);

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

  connect(d->actionWindowErrorLog, SIGNAL(triggered(bool)),
          d->Core, SLOT(onWindowErrorLogActionTriggered(bool)));
  connect(d->actionWindowPythonInteractor, SIGNAL(triggered(bool)),
          d->Core, SLOT(onWindowPythonInteractorActionTriggered(bool)));
  if (d->Core->errorLogWidget())
    {
    d->Core->errorLogWidget()->installEventFilter(this);
    }
  if (d->Core->pythonConsole())
    {
    d->Core->pythonConsole()->installEventFilter(this);
    }

  qSlicerMainWindowCore_connect(HelpKeyboardShortcuts);
  qSlicerMainWindowCore_connect(HelpBrowseTutorials);
  qSlicerMainWindowCore_connect(HelpInterfaceDocumentation);
  qSlicerMainWindowCore_connect(HelpSlicerPublications);
  qSlicerMainWindowCore_connect(HelpAboutSlicerQT);

  qSlicerMainWindowCore_connect(HelpReportBugOrFeatureRequest);
  qSlicerMainWindowCore_connect(HelpVisualBlog);

  //connect ToolBars actions
  connect(d->actionWindowToolbarsResetToDefault, SIGNAL(triggered()),
          this, SLOT(restoreToolbars()));

  // Main ToolBar actions (where are actions for load data and save?
  connect(d->actionLoadDICOM, SIGNAL(triggered()),
          this, SLOT(loadDICOMActionTriggered()));
  // Module ToolBar actions
  connect(d->actionModuleHome, SIGNAL(triggered()),
          this, SLOT(setHomeModuleCurrent()));

  d->actionViewExtensionManager->setVisible(QSettings().value("Extensions/ManagerEnabled").toBool());

#ifndef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  d->actionViewExtensionManager->setVisible(false);
#endif
#ifndef Slicer_USE_PYTHONQT
  d->actionWindowPythonInteractor->setVisible(false);
#endif

}
#undef qSlicerMainWindowCore_connect


//---------------------------------------------------------------------------
void qSlicerMainWindow::loadDICOMActionTriggered()
{
//  Q_D(qSlicerMainWindow);
// raise the dicom module....
//  d->ModuleSelectorToolBar->selectModule("DICOM");

  qSlicerLayoutManager * layoutManager = qSlicerApplication::application()->layoutManager();

  if (!layoutManager)
    {
    return;
    }
  layoutManager->setCurrentModule("DICOM");


}


//---------------------------------------------------------------------------
void qSlicerMainWindow::onEditApplicationSettingsActionTriggered()
{
  qSlicerApplication::application()->settingsDialog()->exec();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onViewExtensionManagerActionTriggered()
{
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  Q_D(qSlicerMainWindow);
  qSlicerApplication * app = qSlicerApplication::application();
  d->ExtensionsManagerDialog->setExtensionsManagerModel(app->extensionManagerModel());
  if (d->ExtensionsManagerDialog->exec() == QDialog::Accepted)
    {
    app->confirmRestart();
    }
#endif
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleLoaded(const QString& moduleName)
{
  Q_D(qSlicerMainWindow);

  qSlicerAbstractCoreModule* coreModule =
    qSlicerApplication::application()->moduleManager()->module(moduleName);
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
  int index = d->FavoriteModules.indexOf(module->name());
  if (index != -1)
    {
    // find the location of where to add the action.
    // Note: FavoriteModules is sorted
    QAction* beforeAction = 0; // 0 means insert at end
    foreach(QAction* toolBarAction, d->ModuleToolBar->actions())
      {
      bool isActionAFavoriteModule =
        (d->FavoriteModules.indexOf(toolBarAction->data().toString()) != -1);
      if ( isActionAFavoriteModule &&
          d->FavoriteModules.indexOf(toolBarAction->data().toString()) > index)
        {
        beforeAction = toolBarAction;
        break;
        }
      }
    d->ModuleToolBar->insertAction(beforeAction, action);
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleAboutToBeUnloaded(const QString& moduleName)
{
  Q_D(qSlicerMainWindow);

  foreach(QAction* action, d->ModuleToolBar->actions())
    {
    if (action->data().toString() == moduleName)
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
  //if (scene && scene->IsBatchProcessing())
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
void qSlicerMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  qSlicerApplication::application()->ioManager()->dragEnterEvent(event);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::dropEvent(QDropEvent *event)
{
  qSlicerApplication::application()->ioManager()->dropEvent(event);
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

//---------------------------------------------------------------------------
bool qSlicerMainWindow::eventFilter(QObject* object, QEvent* event)
{
  Q_D(qSlicerMainWindow);
  bool showEvent = (event->type() == QEvent::Show);
  bool hideEvent = (event->type() == QEvent::Close);
  if (showEvent || hideEvent)
    {
    if (object == d->Core->errorLogWidget())
      {
      d->actionWindowErrorLog->setChecked(showEvent);
      }
#ifdef Slicer_USE_PYTHONQT
    else if (object == d->Core->pythonConsole())
      {
      d->actionWindowPythonInteractor->setChecked(showEvent);
      }
#endif
    }
  return this->Superclass::eventFilter(object, event);
}
