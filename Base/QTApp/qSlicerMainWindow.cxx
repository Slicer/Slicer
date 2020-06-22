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

#include "qSlicerMainWindow_p.h"

// Qt includes
#include <QAction>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QQueue>
#include <QSettings>
#include <QShowEvent>
#include <QSignalMapper>
#include <QStyle>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>

// CTK includes
#include <ctkErrorLogWidget.h>
#include <ctkMessageBox.h>
#ifdef Slicer_USE_PYTHONQT
# include <ctkPythonConsole.h>
#endif
#ifdef Slicer_USE_QtTesting
#include <ctkQtTestingUtility.h>
#endif
#include <ctkSettingsDialog.h>
#include <ctkVTKSliceView.h>
#include <ctkVTKWidgetsUtils.h>

// SlicerApp includes
#include "qSlicerAboutDialog.h"
#include "qSlicerActionsDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#if defined Slicer_USE_QtTesting && defined Slicer_BUILD_CLI_SUPPORT
#include "qSlicerCLIModuleWidgetEventPlayer.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerErrorReportDialog.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulesMenu.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerIOManager.h"

// qMRML includes
#include <qMRMLSliceWidget.h>

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkCollection.h>

namespace
{

//-----------------------------------------------------------------------------
void setThemeIcon(QAction* action, const QString& name)
{
  action->setIcon(QIcon::fromTheme(name, action->icon()));
}

} // end of anonymous namespace

//-----------------------------------------------------------------------------
// qSlicerMainWindowPrivate methods

qSlicerMainWindowPrivate::qSlicerMainWindowPrivate(qSlicerMainWindow& object)
  : q_ptr(&object)
{
#ifdef Slicer_USE_PYTHONQT
  this->PythonConsoleDockWidget = nullptr;
  this->PythonConsoleToggleViewAction = nullptr;
#endif
  this->ErrorLogWidget = nullptr;
  this->ErrorLogToolButton = nullptr;
  this->ModuleSelectorToolBar = nullptr;
  this->LayoutManager = nullptr;
  this->WindowInitialShowCompleted = false;
  this->IsClosing = false;
}

//-----------------------------------------------------------------------------
qSlicerMainWindowPrivate::~qSlicerMainWindowPrivate()
{
  delete this->ErrorLogWidget;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::init()
{
  Q_Q(qSlicerMainWindow);
  this->setupUi(q);

  this->setupStatusBar();
  q->setupMenuActions();
  this->StartupState = q->saveState();
  this->readSettings();
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(qSlicerMainWindow);

  this->Ui_qSlicerMainWindow::setupUi(mainWindow);

  qSlicerApplication * app = qSlicerApplication::application();

  //----------------------------------------------------------------------------
  // Load data shortcuts for backward compatibility
  //----------------------------------------------------------------------------
  QList<QKeySequence> addShortcuts = this->FileAddDataAction->shortcuts();
  addShortcuts << QKeySequence(Qt::CTRL + Qt::Key_A);
  this->FileAddDataAction->setShortcuts(addShortcuts);

  //----------------------------------------------------------------------------
  // Recently loaded files
  //----------------------------------------------------------------------------
  QObject::connect(app->coreIOManager(), SIGNAL(newFileLoaded(qSlicerIO::IOProperties)),
                   q, SLOT(onNewFileLoaded(qSlicerIO::IOProperties)));

  //----------------------------------------------------------------------------
  // Load DICOM
  //----------------------------------------------------------------------------
#ifndef Slicer_BUILD_DICOM_SUPPORT
  this->LoadDICOMAction->setVisible(false);
#endif

  //----------------------------------------------------------------------------
  // ModulePanel
  //----------------------------------------------------------------------------
  this->PanelDockWidget->toggleViewAction()->setText(qSlicerMainWindow::tr("&Module Panel"));
  this->PanelDockWidget->toggleViewAction()->setToolTip(
    qSlicerMainWindow::tr("Collapse/Expand the GUI panel and allows Slicer's viewers to occupy "
          "the entire application window"));
  this->PanelDockWidget->toggleViewAction()->setShortcut(QKeySequence("Ctrl+5"));
  this->ViewMenu->insertAction(this->WindowToolBarsMenu->menuAction(),
                               this->PanelDockWidget->toggleViewAction());

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

  // Ensure the panel dock widget is visible
  QObject::connect(this->ModuleSelectorToolBar, SIGNAL(moduleSelected(QString)),
                   this->PanelDockWidget, SLOT(show()));

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

  // if testing is enabled on the application level, add a time out to the pop ups
  if (qSlicerApplication::application()->testAttribute(qSlicerCoreApplication::AA_EnableTesting))
    {
    this->CaptureToolBar->setPopupsTimeOut(true);
    }

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
  toolBarActions << this->DialogToolBar->toggleViewAction();
  this->WindowToolBarsMenu->addActions(toolBarActions);

  //----------------------------------------------------------------------------
  // Hide toolbars by default
  //----------------------------------------------------------------------------
  // Hide the Layout toolbar by default
  // The setVisibility slot of the toolbar is connected to the
  // QAction::triggered signal.
  // It's done for a long list of reasons. If you change this behavior, make sure
  // minimizing the application and restore it doesn't hide the module panel. check
  // also the geometry and the state of the menu qactions are correctly restored when
  // loading slicer.
  this->UndoRedoToolBar->toggleViewAction()->trigger();
  this->LayoutToolBar->toggleViewAction()->trigger();
  //q->removeToolBar(this->UndoRedoToolBar);
  //q->removeToolBar(this->LayoutToolBar);
  delete this->UndoRedoToolBar;
  this->UndoRedoToolBar = nullptr;
  delete this->LayoutToolBar;
  this->LayoutToolBar = nullptr;

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
  // Instantiate and assign the layout manager to the slicer application
  this->LayoutManager = new qSlicerLayoutManager(layoutFrame);
  this->LayoutManager->setScriptedDisplayableManagerDirectory(
      qSlicerApplication::application()->slicerHome() + "/bin/Python/mrmlDisplayableManager");
  qSlicerApplication::application()->setLayoutManager(this->LayoutManager);
#ifdef Slicer_USE_QtTesting
  // we store this layout manager to the Object state property for QtTesting
  qSlicerApplication::application()->testingUtility()->addObjectStateProperty(
      qSlicerApplication::application()->layoutManager(), QString("layout"));
  qSlicerApplication::application()->testingUtility()->addObjectStateProperty(
      this->ModuleSelectorToolBar->modulesMenu(), QString("currentModule"));
#endif
  // Layout manager should also listen the MRML scene
  // Note: This creates the OpenGL context for each view, so things like
  // multisampling should probably be configured before this line is executed.
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
  QMenu *compareMenu = new QMenu(qSlicerMainWindow::tr("Select number of viewers..."), mainWindow);
  compareMenu->setObjectName("CompareMenuView");
  compareMenu->addAction(this->ViewLayoutCompare_2_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_3_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_4_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_5_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_6_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_7_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompare_8_viewersAction);
  this->ViewLayoutCompareAction->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareActionTriggered(QAction*)));

  // ... and for widescreen version of compare view as well
  compareMenu = new QMenu(qSlicerMainWindow::tr("Select number of viewers..."), mainWindow);
  compareMenu->setObjectName("CompareMenuWideScreen");
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_2_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_3_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_4_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_5_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_6_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_7_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareWidescreen_8_viewersAction);
  this->ViewLayoutCompareWidescreenAction->setMenu(compareMenu);
  QObject::connect(compareMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutCompareWidescreenActionTriggered(QAction*)));

  // ... and for the grid version of the compare views
  compareMenu = new QMenu(qSlicerMainWindow::tr("Select number of viewers..."), mainWindow);
  compareMenu->setObjectName("CompareMenuGrid");
  compareMenu->addAction(this->ViewLayoutCompareGrid_2x2_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareGrid_3x3_viewersAction);
  compareMenu->addAction(this->ViewLayoutCompareGrid_4x4_viewersAction);
  this->ViewLayoutCompareGridAction->setMenu(compareMenu);
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
  layoutButton->setText(qSlicerMainWindow::tr("Layout"));
  layoutButton->setMenu(this->LayoutMenu);
  layoutButton->setPopupMode(QToolButton::InstantPopup);

  layoutButton->setDefaultAction(this->ViewLayoutConventionalAction);

  QObject::connect(this->LayoutMenu, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->LayoutMenu, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));

  QObject::connect(this->menuConventionalQuantitative, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->menuConventionalQuantitative, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));

  QObject::connect(this->menuFourUpQuantitative, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->menuFourUpQuantitative, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));

  QObject::connect(this->menuOneUpQuantitative, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->menuOneUpQuantitative, SIGNAL(triggered(QAction*)),
                   q, SLOT(onLayoutActionTriggered(QAction*)));

  QObject::connect(this->menuThreeOverThreeQuantitative, SIGNAL(triggered(QAction*)),
                   layoutButton, SLOT(setDefaultAction(QAction*)));
  QObject::connect(this->menuThreeOverThreeQuantitative, SIGNAL(triggered(QAction*)),
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

  this->CutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  setThemeIcon(this->FileExitAction, "application-exit");
  setThemeIcon(this->EditUndoAction, "edit-undo");
  setThemeIcon(this->EditRedoAction, "edit-redo");
  setThemeIcon(this->CutAction, "edit-cut");
  setThemeIcon(this->CopyAction, "edit-copy");
  setThemeIcon(this->PasteAction, "edit-paste");
  setThemeIcon(this->EditApplicationSettingsAction, "preferences-system");
  setThemeIcon(this->ModuleHomeAction, "go-home");

  //----------------------------------------------------------------------------
  // Error log widget
  //----------------------------------------------------------------------------
  this->ErrorLogWidget = new ctkErrorLogWidget;
  this->ErrorLogWidget->setErrorLogModel(
    qSlicerApplication::application()->errorLogModel());

  //----------------------------------------------------------------------------
  // Python console
  //----------------------------------------------------------------------------
#ifdef Slicer_USE_PYTHONQT
  if (q->pythonConsole())
    {
    if (QSettings().value("Python/DockableWindow").toBool())
      {
      this->PythonConsoleDockWidget = new QDockWidget(qSlicerMainWindow::tr("Python Interactor"));
      this->PythonConsoleDockWidget->setObjectName("PythonConsoleDockWidget");
      this->PythonConsoleDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
      this->PythonConsoleDockWidget->setWidget(q->pythonConsole());
      this->PythonConsoleToggleViewAction = this->PythonConsoleDockWidget->toggleViewAction();
      // Set default state
      q->addDockWidget(Qt::BottomDockWidgetArea, this->PythonConsoleDockWidget);
      this->PythonConsoleDockWidget->hide();
      }
    else
      {
      ctkPythonConsole* pythonConsole = q->pythonConsole();
      pythonConsole->setWindowTitle("Slicer Python Interactor");
      pythonConsole->resize(600, 280);
      pythonConsole->hide();
      this->PythonConsoleToggleViewAction = new QAction("", this->ViewMenu);
      this->PythonConsoleToggleViewAction->setCheckable(true);
      }
    QObject::connect(q->pythonConsole(), SIGNAL(aboutToExecute(const QString&)),
      q, SLOT(onPythonConsoleUserInput(const QString&)));
    // Set up show/hide action
    this->PythonConsoleToggleViewAction->setText(qSlicerMainWindow::tr("&Python Interactor"));
    this->PythonConsoleToggleViewAction->setToolTip(qSlicerMainWindow::tr(
      "Show Python Interactor window for controlling the application's data, user interface, and internals"));
    this->PythonConsoleToggleViewAction->setShortcut(QKeySequence("Ctrl+3"));
    QObject::connect(this->PythonConsoleToggleViewAction, SIGNAL(toggled(bool)),
      q, SLOT(onPythonConsoleToggled(bool)));
    this->ViewMenu->insertAction(this->WindowToolBarsMenu->menuAction(), this->PythonConsoleToggleViewAction);
    this->PythonConsoleToggleViewAction->setIcon(QIcon(":/python-icon.png"));
    this->DialogToolBar->addAction(this->PythonConsoleToggleViewAction);
    }
  else
    {
    qWarning("qSlicerMainWindowPrivate::setupUi: Failed to create Python console");
    }
#endif
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::readSettings()
{
  Q_Q(qSlicerMainWindow);
  QSettings settings;
  settings.beginGroup("MainWindow");
  q->setToolButtonStyle(settings.value("ShowToolButtonText").toBool()
                        ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
    {
    q->restoreGeometry(settings.value("geometry").toByteArray());
    q->restoreState(settings.value("windowState").toByteArray());
    this->LayoutManager->setLayout(settings.value("layout").toInt());
    }
  settings.endGroup();
  this->FavoriteModules << settings.value("Modules/FavoriteModules").toStringList();

  foreach(const qSlicerIO::IOProperties& fileProperty, Self::readRecentlyLoadedFiles())
    {
    this->RecentlyLoadedFileProperties.enqueue(fileProperty);
    }
  this->filterRecentlyLoadedFileProperties();
  this->setupRecentlyLoadedMenu(this->RecentlyLoadedFileProperties);
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
  Self::writeRecentlyLoadedFiles(this->RecentlyLoadedFileProperties);
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupRecentlyLoadedMenu(const QList<qSlicerIO::IOProperties>& fileProperties)
{
  Q_Q(qSlicerMainWindow);

  this->RecentlyLoadedMenu->setEnabled(fileProperties.size() > 0);
  this->RecentlyLoadedMenu->clear();

  QListIterator<qSlicerIO::IOProperties> iterator(fileProperties);
  iterator.toBack();
  while (iterator.hasPrevious())
    {
    qSlicerIO::IOProperties filePropertie = iterator.previous();
    QString fileName = filePropertie.value("fileName").toString();
    if (fileName.isEmpty())
      {
      continue;
      }
    QAction * action = this->RecentlyLoadedMenu->addAction(
      fileName, q, SLOT(onFileRecentLoadedActionTriggered()));
    action->setProperty("fileParameters", filePropertie);
    action->setEnabled(QFile::exists(fileName));
    }

  // Add separator and clear action
  this->RecentlyLoadedMenu->addSeparator();
  QAction * clearAction = this->RecentlyLoadedMenu->addAction(
    "Clear History", q, SLOT(onFileRecentLoadedActionTriggered()));
  clearAction->setProperty("clearMenu", QVariant(true));
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::filterRecentlyLoadedFileProperties()
{
  int numberOfFilesToKeep = QSettings().value("RecentlyLoadedFiles/NumberToKeep").toInt();

  // Remove extra element
  while (this->RecentlyLoadedFileProperties.size() > numberOfFilesToKeep)
    {
    this->RecentlyLoadedFileProperties.dequeue();
    }
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOProperties> qSlicerMainWindowPrivate::readRecentlyLoadedFiles()
{
  QList<qSlicerIO::IOProperties> fileProperties;

  QSettings settings;
  int size = settings.beginReadArray("RecentlyLoadedFiles/RecentFiles");
  for(int i = 0; i < size; ++i)
    {
    settings.setArrayIndex(i);
    QVariant file = settings.value("file");
    fileProperties << file.toMap();
    }
  settings.endArray();

  return fileProperties;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::writeRecentlyLoadedFiles(const QList<qSlicerIO::IOProperties>& fileProperties)
{
  QSettings settings;
  settings.beginWriteArray("RecentlyLoadedFiles/RecentFiles", fileProperties.size());
  for (int i = 0; i < fileProperties.size(); ++i)
    {
    settings.setArrayIndex(i);
    settings.setValue("file", fileProperties.at(i));
    }
  settings.endArray();
}

//-----------------------------------------------------------------------------
bool qSlicerMainWindowPrivate::confirmCloseApplication()
{
  Q_Q(qSlicerMainWindow);
  vtkMRMLScene* scene = qSlicerApplication::application()->mrmlScene();
  QString question;
  if (scene->GetStorableNodesModifiedSinceRead())
    {
    question = qSlicerMainWindow::tr("Some data have been modified. Do you want to save them before exit?");
    }
  else if (scene->GetModifiedSinceRead())
    {
    question = qSlicerMainWindow::tr("The scene has been modified. Do you want to save it before exit?");
    }
  bool close = false;
  if (!question.isEmpty())
    {
    QMessageBox* messageBox = new QMessageBox(QMessageBox::Warning, qSlicerMainWindow::tr("Save before exit?"), question, QMessageBox::NoButton);
    QAbstractButton* saveButton =
       messageBox->addButton(qSlicerMainWindow::tr("Save"), QMessageBox::ActionRole);
    QAbstractButton* exitButton =
       messageBox->addButton(qSlicerMainWindow::tr("Exit (discard modifications)"), QMessageBox::ActionRole);
    QAbstractButton* cancelButton =
       messageBox->addButton(qSlicerMainWindow::tr("Cancel exit"), QMessageBox::ActionRole);
    Q_UNUSED(cancelButton);
    messageBox->exec();
    if (messageBox->clickedButton() == saveButton)
      {
      // \todo Check if the save data dialog was "applied" and close the
      // app in that case
      this->FileSaveSceneAction->trigger();
      }
    else if (messageBox->clickedButton() == exitButton)
      {
      close = true;
      }
    messageBox->deleteLater();
    }
  else
    {
    close = ctkMessageBox::confirmExit("MainWindow/DontConfirmExit", q);
    }
  return close;
}

//-----------------------------------------------------------------------------
bool qSlicerMainWindowPrivate::confirmCloseScene()
{
  Q_Q(qSlicerMainWindow);
  vtkMRMLScene* scene = qSlicerApplication::application()->mrmlScene();
  QString question;
  if (scene->GetStorableNodesModifiedSinceRead())
    {
    question = qSlicerMainWindow::tr("Some data have been modified. Do you want to save them before closing the scene?");
    }
  else if (scene->GetModifiedSinceRead())
    {
    question = qSlicerMainWindow::tr("The scene has been modified. Do you want to save it before closing the scene?");
    }
  else
    {
    // no unsaved changes, no need to ask confirmation
    return true;
    }

  ctkMessageBox* confirmCloseMsgBox = new ctkMessageBox(q);
  confirmCloseMsgBox->setAttribute(Qt::WA_DeleteOnClose);
  confirmCloseMsgBox->setWindowTitle(qSlicerMainWindow::tr("Save before closing scene?"));
  confirmCloseMsgBox->setText(question);

  // Use AcceptRole&RejectRole instead of Save&Discard because we would
  // like discard changes to be the default behavior.
  confirmCloseMsgBox->addButton(qSlicerMainWindow::tr("Close scene (discard modifications)"), QMessageBox::AcceptRole);
  confirmCloseMsgBox->addButton(qSlicerMainWindow::tr("Save scene"), QMessageBox::RejectRole);
  confirmCloseMsgBox->addButton(QMessageBox::Cancel);

  confirmCloseMsgBox->setDontShowAgainVisible(true);
  confirmCloseMsgBox->setDontShowAgainSettingsKey("MainWindow/DontConfirmSceneClose");
  confirmCloseMsgBox->setIcon(QMessageBox::Question);
  int resultCode = confirmCloseMsgBox->exec();
  if (resultCode == QMessageBox::Cancel)
    {
    return false;
    }
  if (resultCode != QMessageBox::AcceptRole)
    {
    if (!qSlicerApplication::application()->ioManager()->openSaveDataDialog())
      {
      return false;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setupStatusBar()
{
  Q_Q(qSlicerMainWindow);
  this->ErrorLogToolButton = new QToolButton();
  this->ErrorLogToolButton->setDefaultAction(this->WindowErrorLogAction);
  q->statusBar()->addPermanentWidget(this->ErrorLogToolButton);

  QObject::connect(qSlicerApplication::application()->errorLogModel(),
                   SIGNAL(entryAdded(ctkErrorLogLevel::LogLevel)),
                   q, SLOT(onWarningsOrErrorsOccurred(ctkErrorLogLevel::LogLevel)));
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowPrivate::setErrorLogIconHighlighted(bool highlighted)
{
  Q_Q(qSlicerMainWindow);
  QIcon defaultIcon = q->style()->standardIcon(QStyle::SP_MessageBoxCritical);
  QIcon icon = defaultIcon;
  if(!highlighted)
    {
    QIcon disabledIcon;
    disabledIcon.addPixmap(
          defaultIcon.pixmap(QSize(32, 32), QIcon::Disabled, QIcon::On), QIcon::Active, QIcon::On);
    icon = disabledIcon;
    }
  this->WindowErrorLogAction->setIcon(icon);
}

//-----------------------------------------------------------------------------
// qSlicerMainWindow methods

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMainWindowPrivate(*this))
{
  Q_D(qSlicerMainWindow);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(qSlicerMainWindowPrivate* pimpl,
                                     QWidget* windowParent)
  : Superclass(windowParent)
  , d_ptr(pimpl)
{
  // init() is called by derived class.
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
  vtkCollection* sliceLogics = d->LayoutManager ? d->LayoutManager->mrmlSliceLogics() : nullptr;
  for (int i = 0; i < sliceLogics->GetNumberOfItems(); ++i)
    {
    vtkMRMLSliceLogic* sliceLogic = vtkMRMLSliceLogic::SafeDownCast(sliceLogics->GetItemAsObject(i));
    if (!sliceLogic)
      {
      continue;
      }
    sliceLogic->GetSliceCompositeNode()->SetReferenceBackgroundVolumeID(nullptr);
    sliceLogic->GetSliceCompositeNode()->SetReferenceForegroundVolumeID(nullptr);
    sliceLogic->GetSliceCompositeNode()->SetReferenceLabelVolumeID(nullptr);
    }
}

//-----------------------------------------------------------------------------
qSlicerModuleSelectorToolBar* qSlicerMainWindow::moduleSelector()const
{
  Q_D(const qSlicerMainWindow);
  return d->ModuleSelectorToolBar;
}

#ifdef Slicer_USE_PYTHONQT
//---------------------------------------------------------------------------
ctkPythonConsole* qSlicerMainWindow::pythonConsole()const
{
  return qSlicerCoreApplication::application()->pythonConsole();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onPythonConsoleUserInput(const QString& cmd)
{
  if (!cmd.isEmpty())
    {
    qDebug("Python console user input: %s", qPrintable(cmd));
    }
}
#endif

//---------------------------------------------------------------------------
ctkErrorLogWidget* qSlicerMainWindow::errorLogWidget()const
{
  Q_D(const qSlicerMainWindow);
  return d->ErrorLogWidget;
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileAddDataAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileLoadDataAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileImportSceneAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openAddSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileLoadSceneAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openLoadSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileAddVolumeAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openAddVolumesDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileAddTransformAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openAddTransformDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileSaveSceneAction_triggered()
{
  qSlicerApplication::application()->ioManager()->openSaveDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileExitAction_triggered()
{
  this->close();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_SDBSaveToDirectoryAction_triggered()
{
  // Q_D(qSlicerMainWindow);
  // open a file dialog to let the user choose where to save
  QString tempDir = qSlicerCoreApplication::application()->temporaryPath();
  QString saveDirName = QFileDialog::getExistingDirectory(
    this, tr("Slicer Data Bundle Directory (Select Empty Directory)"),
    tempDir, QFileDialog::ShowDirsOnly);
  if (saveDirName.isEmpty())
    {
    std::cout << "No directory name chosen!" << std::endl;
    return;
    }
  // pass in a screen shot
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewport();
  QImage screenShot = ctk::grabVTKWidget(widget);
  qSlicerIO::IOProperties properties;
  properties["fileName"] = saveDirName;
  properties["screenShot"] = screenShot;
  qSlicerCoreApplication::application()->coreIOManager()
    ->saveNodes(QString("SceneFile"), properties);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_SDBSaveToMRBAction_triggered()
{
  //
  // open a file dialog to let the user choose where to save
  // make sure it was selected and add a .mrb to it if needed
  //
  QString fileName = QFileDialog::getSaveFileName(
    this, tr("Save Data Bundle File"),
    "", tr("Medical Reality Bundle (*.mrb)"));

  if (fileName.isEmpty())
    {
    std::cout << "No directory name chosen!" << std::endl;
    return;
    }

  if ( !fileName.endsWith(".mrb") )
    {
    fileName += QString(".mrb");
    }
  qSlicerIO::IOProperties properties;
  properties["fileName"] = fileName;
  qSlicerCoreApplication::application()->coreIOManager()
    ->saveNodes(QString("SceneFile"), properties);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_FileCloseSceneAction_triggered()
{
  Q_D(qSlicerMainWindow);
  if (d->confirmCloseScene())
    {
    qSlicerCoreApplication::application()->mrmlScene()->Clear(false);
    // Make sure we don't remember the last scene's filename to prevent
    // accidentally overwriting the scene.
    qSlicerCoreApplication::application()->mrmlScene()->SetURL("");
    // Set default scene file format to .mrml
    qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
    coreIOManager->setDefaultSceneFileType("MRML Scene (.mrml)");
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_EditRecordMacroAction_triggered()
{
#ifdef Slicer_USE_QtTesting
  qSlicerApplication::application()->testingUtility()->recordTestsBySuffix(QString("xml"));
#endif
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_EditPlayMacroAction_triggered()
{
#ifdef Slicer_USE_QtTesting
  qSlicerApplication::application()->testingUtility()->openPlayerDialog();
#endif
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_EditUndoAction_triggered()
{
  qSlicerApplication::application()->mrmlScene()->Undo();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_EditRedoAction_triggered()
{
  qSlicerApplication::application()->mrmlScene()->Redo();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_ModuleHomeAction_triggered()
{
  this->setHomeModuleCurrent();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::setLayout(int layout)
{
  qSlicerApplication::application()->layoutManager()->setLayout(layout);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::setLayoutNumberOfCompareViewRows(int num)
{
  qSlicerApplication::application()->layoutManager()->setLayoutNumberOfCompareViewRows(num);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::setLayoutNumberOfCompareViewColumns(int num)
{
  qSlicerApplication::application()->layoutManager()->setLayoutNumberOfCompareViewColumns(num);
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::on_WindowErrorLogAction_triggered()
{
  Q_D(qSlicerMainWindow);
  d->ErrorLogWidget->show();
  d->ErrorLogWidget->activateWindow();
  d->ErrorLogWidget->raise();
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::onPythonConsoleToggled(bool toggled)
{
  Q_D(qSlicerMainWindow);
#ifdef Slicer_USE_PYTHONQT
  ctkPythonConsole* pythonConsole = this->pythonConsole();
  if (!pythonConsole)
    {
    qCritical() << Q_FUNC_INFO << " failed: python console is not available";
    return;
    }
  if (d->PythonConsoleDockWidget)
    {
    // Dockable Python console
    if (toggled)
      {
      d->PythonConsoleDockWidget->activateWindow();
      QTextEdit* textEditWidget = pythonConsole->findChild<QTextEdit*>();
      if (textEditWidget)
        {
        textEditWidget->setFocus();
        }
      }
    }
  else
    {
    // Independent Python console
    if (toggled)
      {
      pythonConsole->show();
      pythonConsole->activateWindow();
      pythonConsole->raise();
      }
    else
      {
      pythonConsole->hide();
      }
    }
#else
  Q_UNUSED(toggled);
#endif
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_WindowToolbarsResetToDefaultAction_triggered()
{
  Q_D(qSlicerMainWindow);
  this->restoreState(d->StartupState);
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::onFileRecentLoadedActionTriggered()
{
  Q_D(qSlicerMainWindow);

  QAction* loadRecentFileAction = qobject_cast<QAction*>(this->sender());
  Q_ASSERT(loadRecentFileAction);

  // Clear menu if it applies
  if (loadRecentFileAction->property("clearMenu").isValid())
    {
    d->RecentlyLoadedFileProperties.clear();
    d->setupRecentlyLoadedMenu(d->RecentlyLoadedFileProperties);
    return;
    }

  QVariant fileParameters = loadRecentFileAction->property("fileParameters");
  Q_ASSERT(fileParameters.isValid());

  qSlicerIO::IOProperties fileProperties = fileParameters.toMap();
  qSlicerIO::IOFileType fileType =
      static_cast<qSlicerIO::IOFileType>(
        fileProperties.find("fileType").value().toString());

  qSlicerApplication* app = qSlicerApplication::application();
  app->coreIOManager()->loadNodes(fileType, fileProperties);
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::closeEvent(QCloseEvent *event)
{
  Q_D(qSlicerMainWindow);

  // This is necessary because of a Qt bug on MacOS.
  // (https://bugreports.qt.io/browse/QTBUG-43344).
  // This flag prevents a second close event to be handled.
  if (d->IsClosing)
    {
    return;
    }
  d->IsClosing = true;

  if (d->confirmCloseApplication())
    {
    // Proceed with closing the application

    // Exit current module to leave it a chance to change the UI (e.g. layout)
    // before writing settings.
    d->ModuleSelectorToolBar->selectModule("");

    d->writeSettings();
    event->accept();

    QTimer::singleShot(0, qApp, SLOT(closeAllWindows()));
    }
  else
    {
    // Request is cancelled, application will not be closed
    event->ignore();
    d->IsClosing = false;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::showEvent(QShowEvent *event)
{
  Q_D(qSlicerMainWindow);
  this->Superclass::showEvent(event);
  if (!d->WindowInitialShowCompleted)
    {
    d->WindowInitialShowCompleted = true;
    this->disclaimer();
    this->pythonConsoleInitialDisplay();
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
    // See https://issues.slicer.org/view.php?id=4641
    // qSlicerApplication::application()->gatherExtensionsHistoryInformationOnStartup();
#endif
    emit initialWindowShown();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::pythonConsoleInitialDisplay()
{
  Q_D(qSlicerMainWindow);
#ifdef Slicer_USE_PYTHONQT
  qSlicerApplication * app = qSlicerApplication::application();
  if (qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    return;
    }
  if (app->commandOptions()->showPythonInteractor() && d->PythonConsoleDockWidget)
    {
    d->PythonConsoleDockWidget->show();
    }
#endif
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

  QString message = QString(Slicer_DISCLAIMER_AT_STARTUP);
  if (message.isEmpty())
    {
    // No disclaimer message to show, skip the popup
    return;
    }

  // Replace "%1" in the text by the name and version of the application
  message = message.arg(app->applicationName() + " " + app->applicationVersion());

  ctkMessageBox* disclaimerMessage = new ctkMessageBox(this);
  disclaimerMessage->setAttribute( Qt::WA_DeleteOnClose, true );
  disclaimerMessage->setText(message);
  disclaimerMessage->setIcon(QMessageBox::Information);
  disclaimerMessage->setDontShowAgainSettingsKey("MainWindow/DontShowDisclaimerMessage");
  QTimer::singleShot(0, disclaimerMessage, SLOT(exec()));
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::setupMenuActions()
{
  Q_D(qSlicerMainWindow);

  d->ViewLayoutConventionalAction->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  d->ViewLayoutConventionalWidescreenAction->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalWidescreenView);
  d->ViewLayoutConventionalQuantitativeAction->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalQuantitativeView);
  d->ViewLayoutConventionalPlotAction->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView);
  d->ViewLayoutFourUpAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
  d->ViewLayoutFourUpQuantitativeAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpQuantitativeView);
  d->ViewLayoutFourUpPlotAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpPlotView);
  d->ViewLayoutFourUpPlotTableAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpPlotTableView);
  d->ViewLayoutFourUpTableAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpTableView);
  d->ViewLayoutDual3DAction->setData(vtkMRMLLayoutNode::SlicerLayoutDual3DView);
  d->ViewLayoutTriple3DAction->setData(vtkMRMLLayoutNode::SlicerLayoutTriple3DEndoscopyView);
  d->ViewLayoutOneUp3DAction->setData(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
  d->ViewLayout3DTableAction->setData(vtkMRMLLayoutNode::SlicerLayout3DTableView);
  d->ViewLayoutOneUpQuantitativeAction->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpQuantitativeView);
  d->ViewLayoutOneUpPlotAction->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpPlotView);
  d->ViewLayoutOneUpRedSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  d->ViewLayoutOneUpYellowSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
  d->ViewLayoutOneUpGreenSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  d->ViewLayoutTabbed3DAction->setData(vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
  d->ViewLayoutTabbedSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);
  d->ViewLayoutCompareAction->setData(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  d->ViewLayoutCompareWidescreenAction->setData(vtkMRMLLayoutNode::SlicerLayoutCompareWidescreenView);
  d->ViewLayoutCompareGridAction->setData(vtkMRMLLayoutNode::SlicerLayoutCompareGridView);
  d->ViewLayoutThreeOverThreeAction->setData(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeView);
  d->ViewLayoutThreeOverThreeQuantitativeAction->setData(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeQuantitativeView);
  d->ViewLayoutThreeOverThreePlotAction->setData(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreePlotView);
  d->ViewLayoutFourOverFourAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourOverFourView);
  d->ViewLayoutTwoOverTwoAction->setData(vtkMRMLLayoutNode::SlicerLayoutTwoOverTwoView);
  d->ViewLayoutSideBySideAction->setData(vtkMRMLLayoutNode::SlicerLayoutSideBySideView);
  d->ViewLayoutFourByThreeSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourByThreeSliceView);
  d->ViewLayoutFourByTwoSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutFourByTwoSliceView);
  d->ViewLayoutFiveByTwoSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutFiveByTwoSliceView);
  d->ViewLayoutThreeByThreeSliceAction->setData(vtkMRMLLayoutNode::SlicerLayoutThreeByThreeSliceView);

  d->ViewLayoutCompare_2_viewersAction->setData(2);
  d->ViewLayoutCompare_3_viewersAction->setData(3);
  d->ViewLayoutCompare_4_viewersAction->setData(4);
  d->ViewLayoutCompare_5_viewersAction->setData(5);
  d->ViewLayoutCompare_6_viewersAction->setData(6);
  d->ViewLayoutCompare_7_viewersAction->setData(7);
  d->ViewLayoutCompare_8_viewersAction->setData(8);

  d->ViewLayoutCompareWidescreen_2_viewersAction->setData(2);
  d->ViewLayoutCompareWidescreen_3_viewersAction->setData(3);
  d->ViewLayoutCompareWidescreen_4_viewersAction->setData(4);
  d->ViewLayoutCompareWidescreen_5_viewersAction->setData(5);
  d->ViewLayoutCompareWidescreen_6_viewersAction->setData(6);
  d->ViewLayoutCompareWidescreen_7_viewersAction->setData(7);
  d->ViewLayoutCompareWidescreen_8_viewersAction->setData(8);

  d->ViewLayoutCompareGrid_2x2_viewersAction->setData(2);
  d->ViewLayoutCompareGrid_3x3_viewersAction->setData(3);
  d->ViewLayoutCompareGrid_4x4_viewersAction->setData(4);

  d->WindowErrorLogAction->setIcon(
    this->style()->standardIcon(QStyle::SP_MessageBoxCritical));

  if (this->errorLogWidget())
    {
    d->setErrorLogIconHighlighted(false);
    this->errorLogWidget()->installEventFilter(this);
    }
#ifdef Slicer_USE_PYTHONQT
  if (this->pythonConsole())
    {
    this->pythonConsole()->installEventFilter(this);
    }
#endif

  qSlicerApplication * app = qSlicerApplication::application();

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  d->ViewExtensionsManagerAction->setVisible(
    app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool());
#else
  d->ViewExtensionsManagerAction->setVisible(false);
  d->WindowToolBarsMenu->removeAction(d->ViewExtensionsManagerAction);
#endif

#if defined Slicer_USE_QtTesting && defined Slicer_BUILD_CLI_SUPPORT
  if (app->commandOptions()->enableQtTesting() ||
      app->userSettings()->value("QtTesting/Enabled").toBool())
    {
    d->EditPlayMacroAction->setVisible(true);
    d->EditRecordMacroAction->setVisible(true);
    app->testingUtility()->addPlayer(new qSlicerCLIModuleWidgetEventPlayer());
    }
#endif
  Q_UNUSED(app);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_LoadDICOMAction_triggered()
{
  qSlicerLayoutManager * layoutManager = qSlicerApplication::application()->layoutManager();

  if (!layoutManager)
    {
    return;
    }
  layoutManager->setCurrentModule("DICOM");
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onWarningsOrErrorsOccurred(ctkErrorLogLevel::LogLevel logLevel)
{
  Q_D(qSlicerMainWindow);
  if(logLevel > ctkErrorLogLevel::Info)
    {
    d->setErrorLogIconHighlighted(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_EditApplicationSettingsAction_triggered()
{
  ctkSettingsDialog* const settingsDialog =
    qSlicerApplication::application()->settingsDialog();

  // Reload settings to apply any changes that have been made outside of the
  // dialog (e.g. changes to module paths due to installing extensions). See
  // http://na-mic.org/Mantis/view.php?id=3658.
  settingsDialog->reloadSettings();

  // Now show the dialog
  settingsDialog->exec();
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onNewFileLoaded(const qSlicerIO::IOProperties& fileProperties)
{
  Q_D(qSlicerMainWindow);

  d->RecentlyLoadedFileProperties.removeAll(fileProperties);

  d->RecentlyLoadedFileProperties.enqueue(fileProperties);

  d->filterRecentlyLoadedFileProperties();

  d->setupRecentlyLoadedMenu(d->RecentlyLoadedFileProperties);

  // Keep the settings up-to-date
  qSlicerMainWindowPrivate::writeRecentlyLoadedFiles(d->RecentlyLoadedFileProperties);
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_CopyAction_triggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != nullptr)
    {
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyPress,
                                           Qt::Key_C,
                                           Qt::ControlModifier));
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyRelease,
                                           Qt::Key_C,
                                           Qt::ControlModifier));
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_PasteAction_triggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != nullptr)
    {
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyPress,
                                           Qt::Key_V,
                                           Qt::ControlModifier));
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyRelease,
                                           Qt::Key_V,
                                           Qt::ControlModifier));
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_CutAction_triggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != nullptr)
    {
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyPress,
                                           Qt::Key_X,
                                           Qt::ControlModifier));
    QApplication::postEvent(focused,
                            new QKeyEvent( QEvent::KeyRelease,
                                           Qt::Key_X,
                                           Qt::ControlModifier));
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::on_ViewExtensionsManagerAction_triggered()
{
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerApplication * app = qSlicerApplication::application();
  app->openExtensionsManagerDialog();
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
    QAction* beforeAction = nullptr; // 0 means insert at end
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
    action->setParent(d->ModuleToolBar);
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onModuleAboutToBeUnloaded(const QString& moduleName)
{
  Q_D(qSlicerMainWindow);

  if (d->ModuleSelectorToolBar->selectedModule() == moduleName)
    {
    d->ModuleSelectorToolBar->selectModule("");
    }

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
  //d->EditUndoAction->setEnabled(scene && scene->GetNumberOfUndoLevels());
  //d->EditRedoAction->setEnabled(scene && scene->GetNumberOfRedoLevels());
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);
  bool found = false;
  // std::cerr << "onLayoutActionTriggered: " << action->text().toStdString() << std::endl;
  foreach(QAction* maction, d->LayoutMenu->actions())
    {
    if (action->text() == maction->text())
      {
      found = true;
      break;
      }
    }

  foreach(QAction* maction, d->menuConventionalQuantitative->actions())
    {
    if (action->text() == maction->text())
      {
      found = true;
      break;
      }
    }

  foreach(QAction* maction, d->menuFourUpQuantitative->actions())
    {
    if (action->text() == maction->text())
      {
      found = true;
      break;
      }
    }

  foreach(QAction* maction, d->menuOneUpQuantitative->actions())
    {
    if (action->text() == maction->text())
      {
      found = true;
      break;
      }
    }

  foreach(QAction* maction, d->menuThreeOverThreeQuantitative->actions())
    {
    if (action->text() == maction->text())
      {
      found = true;
      break;
      }
    }

  if (found && !action->data().isNull())
    {
    this->setLayout(action->data().toInt());
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutCompareActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);

  // std::cerr << "onLayoutCompareActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->setLayout(d->ViewLayoutCompareAction->data().toInt());
  this->setLayoutNumberOfCompareViewRows(action->data().toInt());
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutCompareWidescreenActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);

  // std::cerr << "onLayoutCompareWidescreenActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->setLayout(d->ViewLayoutCompareWidescreenAction->data().toInt());
  this->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}

//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutCompareGridActionTriggered(QAction* action)
{
  Q_D(qSlicerMainWindow);

  // std::cerr << "onLayoutCompareGridActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->setLayout(d->ViewLayoutCompareGridAction->data().toInt());
  this->setLayoutNumberOfCompareViewRows(action->data().toInt());
  this->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}


//---------------------------------------------------------------------------
void qSlicerMainWindow::onLayoutChanged(int layout)
{
  Q_D(qSlicerMainWindow);
  // Trigger the action associated with the new layout
  foreach(QAction* action, d->LayoutMenu->actions())
    {
    if (action->data().toInt() == layout)
      {
      action->trigger();
      }
    }

  foreach(QAction* action, d->menuConventionalQuantitative->actions())
    {
    if (action->data().toInt() == layout)
      {
      action->trigger();
      }
    }

  foreach(QAction* action, d->menuFourUpQuantitative->actions())
    {
    if (action->data().toInt() == layout)
      {
      action->trigger();
      }
    }

  foreach(QAction* action, d->menuOneUpQuantitative->actions())
    {
    if (action->data().toInt() == layout)
      {
      action->trigger();
      }
    }

  foreach(QAction* action, d->menuThreeOverThreeQuantitative->actions())
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
  if (object == this->errorLogWidget())
    {
    if (event->type() == QEvent::ActivationChange
        && this->errorLogWidget()->isActiveWindow())
      {
      d->setErrorLogIconHighlighted(false);
      }
    }
#ifdef Slicer_USE_PYTHONQT
  if (object == this->pythonConsole())
    {
    if (event->type() == QEvent::Hide)
      {
      bool wasBlocked = d->PythonConsoleToggleViewAction->blockSignals(true);
      d->PythonConsoleToggleViewAction->setChecked(false);
      d->PythonConsoleToggleViewAction->blockSignals(wasBlocked);
      }
    }
#endif
  return this->Superclass::eventFilter(object, event);
}
