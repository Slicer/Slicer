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
#include <QFile>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QQueue>
#include <QSettings>
#include <QShowEvent>
#include <QSignalMapper>
#include <QTimer>
#include <QToolButton>

#include "vtkSlicerConfigure.h" // For Slicer_BUILD_DICOM_SUPPORT, Slicer_USE_PYTHONQT, Slicer_USE_QtTesting

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

// SlicerApp includes
#include "qSlicerApplication.h"
#include "qSlicerAppMainWindow.h"
#include "ui_qSlicerAppMainWindow.h"
#include "qSlicerAbstractModule.h"
#if defined Slicer_USE_QtTesting && defined Slicer_BUILD_CLI_SUPPORT
#include "qSlicerCLIModuleWidgetEventPlayer.h"
#endif
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulesMenu.h"
#include "qSlicerAppMainWindowCore.h"
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
class qSlicerAppMainWindowPrivate: public Ui_qSlicerAppMainWindow
{
  Q_DECLARE_PUBLIC(qSlicerAppMainWindow);
protected:
  qSlicerAppMainWindow* const q_ptr;
public:
  typedef qSlicerAppMainWindowPrivate Self;
  qSlicerAppMainWindowPrivate(qSlicerAppMainWindow& object);
  void setupUi(QMainWindow * mainWindow);

  void readSettings();
  void writeSettings();

  void setupRecentlyLoadedMenu(const QList<qSlicerIO::IOProperties>& fileProperties);

  void filterRecentlyLoadedFileProperties();

  static QList<qSlicerIO::IOProperties> readRecentlyLoadedFiles();
  static void writeRecentlyLoadedFiles(const QList<qSlicerIO::IOProperties>& fileProperties);

  bool confirmClose();

  qSlicerAppMainWindowCore*       Core;
  qSlicerModuleSelectorToolBar*   ModuleSelectorToolBar;
  QStringList                     FavoriteModules;
  qSlicerLayoutManager*           LayoutManager;
  QQueue<qSlicerIO::IOProperties> RecentlyLoadedFileProperties;

  QByteArray                      StartupState;
};

//-----------------------------------------------------------------------------
// qSlicerAppMainWindowPrivate methods

qSlicerAppMainWindowPrivate::qSlicerAppMainWindowPrivate(qSlicerAppMainWindow& object)
  : q_ptr(&object)
{
  this->Core = 0;
  this->ModuleSelectorToolBar = 0;
  this->LayoutManager = 0;
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::setupUi(QMainWindow * mainWindow)
{
  Q_Q(qSlicerAppMainWindow);

  this->Ui_qSlicerAppMainWindow::setupUi(mainWindow);

  qSlicerApplication * app = qSlicerApplication::application();

  //----------------------------------------------------------------------------
  // Recently loaded files
  //----------------------------------------------------------------------------
  QObject::connect(app->coreIOManager(), SIGNAL(newFileLoaded(qSlicerIO::IOProperties)),
                   q, SLOT(onNewFileLoaded(qSlicerIO::IOProperties)));

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
#ifdef Slicer_USE_QtTesting
  // we store this layout manager to the Object state property for QtTesting
  qSlicerApplication::application()->testingUtility()->addObjectStateProperty(
      qSlicerApplication::application()->layoutManager(), QString("layout"));
  qSlicerApplication::application()->testingUtility()->addObjectStateProperty(
      this->ModuleSelectorToolBar->modulesMenu(), QString("currentModule"));
#endif
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
  compareMenu->setObjectName("CompareMenuView");
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
  compareMenu->setObjectName("CompareMenuWideScreen");
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
  compareMenu->setObjectName("CompareMenuGrid");
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
  this->actionHelpAboutSlicerApp->setIcon(informationIcon);
  this->actionHelpReportBugOrFeatureRequest->setIcon(questionIcon);
  this->actionHelpVisualBlog->setIcon(networkIcon);
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::readSettings()
{
  Q_Q(qSlicerAppMainWindow);
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

  foreach(const qSlicerIO::IOProperties& fileProperty, Self::readRecentlyLoadedFiles())
    {
    this->RecentlyLoadedFileProperties.enqueue(fileProperty);
    }
  this->filterRecentlyLoadedFileProperties();
  this->setupRecentlyLoadedMenu(this->RecentlyLoadedFileProperties);
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::writeSettings()
{
  Q_Q(qSlicerAppMainWindow);
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
void qSlicerAppMainWindowPrivate::setupRecentlyLoadedMenu(const QList<qSlicerIO::IOProperties>& fileProperties)
{
  Q_Q(qSlicerAppMainWindow);

  this->menuRecentlyLoaded->setEnabled(fileProperties.size() > 0);
  this->menuRecentlyLoaded->clear();

  QListIterator<qSlicerIO::IOProperties> iterator(fileProperties);
  iterator.toBack();
  while (iterator.hasPrevious())
    {
    qSlicerIO::IOProperties filePropertie = iterator.previous();
    QString fileName = filePropertie.value("fileName").toString();
    Q_ASSERT(!fileName.isEmpty());
    QAction * action =
        this->menuRecentlyLoaded->addAction(fileName, q, SLOT(onFileRecentLoadedActionTriggered()));
    action->setProperty("fileParameters", filePropertie);
    action->setEnabled(QFile::exists(fileName));
    }

  // Add separator and clear action
  this->menuRecentlyLoaded->addSeparator();
  QAction * clearAction = this->menuRecentlyLoaded->addAction("Clear Menu", q, SLOT(onFileRecentLoadedActionTriggered()));
  clearAction->setProperty("clearMenu", QVariant(true));
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindowPrivate::filterRecentlyLoadedFileProperties()
{
  int numberOfFilesToKeep = QSettings().value("RecentlyLoadedFiles/NumberToKeep").toInt();

  // Remove extra element
  while (this->RecentlyLoadedFileProperties.size() > numberOfFilesToKeep)
    {
    this->RecentlyLoadedFileProperties.dequeue();
    }
}

//-----------------------------------------------------------------------------
QList<qSlicerIO::IOProperties> qSlicerAppMainWindowPrivate::readRecentlyLoadedFiles()
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
void qSlicerAppMainWindowPrivate::writeRecentlyLoadedFiles(const QList<qSlicerIO::IOProperties>& fileProperties)
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
bool qSlicerAppMainWindowPrivate::confirmClose()
{
  Q_Q(qSlicerAppMainWindow);
  vtkMRMLScene* scene = qSlicerApplication::application()->mrmlScene();
  QString question;
  if (scene->GetStorableNodesModifiedSinceRead())
    {
    question = q->tr("Some data have been modified. Do you want to save them before exit?");
    }
  else if (scene->GetModifiedSinceRead())
    {
    question = q->tr("The scene has been modified. Do you want to save it before exit?");
    }
  bool close = false;
  if (!question.isEmpty())
    {
    QMessageBox messageBox(QMessageBox::Warning, q->tr("Save before exit?"), question, QMessageBox::NoButton, q);
    QAbstractButton* saveButton =
       messageBox.addButton(q->tr("Save"), QMessageBox::ActionRole);
    QAbstractButton* exitButton =
       messageBox.addButton(q->tr("Exit (discard modifications)"), QMessageBox::ActionRole);
    QAbstractButton* cancelButton =
       messageBox.addButton(q->tr("Cancel exit"), QMessageBox::ActionRole);
    Q_UNUSED(cancelButton);
    messageBox.exec();
    if (messageBox.clickedButton() == saveButton)
      {
      // \todo Check if the save data dialog was "applied" and close the
      // app in that case
      this->actionFileSaveScene->trigger();
      }
    else if (messageBox.clickedButton() == exitButton)
      {
      close = true;
      }
    }
  else
    {
    close = ctkMessageBox::confirmExit("MainWindow/DontConfirmExit", q);
    }
  return close;
}

//-----------------------------------------------------------------------------
// qSlicerAppMainWindow methods

//-----------------------------------------------------------------------------
qSlicerAppMainWindow::qSlicerAppMainWindow(QWidget *_parent):Superclass(_parent)
  , d_ptr(new qSlicerAppMainWindowPrivate(*this))
{
  Q_D(qSlicerAppMainWindow);
  d->setupUi(this);

  // Main window core helps to coordinate various widgets and panels
  d->Core = new qSlicerAppMainWindowCore(this);

  this->setupMenuActions();
  d->StartupState = this->saveState();
  d->readSettings();

#ifndef Slicer_BUILD_DICOM_SUPPORT
  d->actionLoadDICOM->setVisible(false);
#endif

}

//-----------------------------------------------------------------------------
qSlicerAppMainWindow::~qSlicerAppMainWindow()
{
  Q_D(qSlicerAppMainWindow);
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
CTK_GET_CPP(qSlicerAppMainWindow, qSlicerAppMainWindowCore*, core, Core);

//-----------------------------------------------------------------------------
qSlicerModuleSelectorToolBar* qSlicerAppMainWindow::moduleSelector()const
{
  Q_D(const qSlicerAppMainWindow);
  return d->ModuleSelectorToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindow::onFileRecentLoadedActionTriggered()
{
  Q_D(qSlicerAppMainWindow);

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
void qSlicerAppMainWindow::closeEvent(QCloseEvent *event)
{
  Q_D(qSlicerAppMainWindow);
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
void qSlicerAppMainWindow::showEvent(QShowEvent *event)
{
  this->Superclass::showEvent(event);
  if (!event->spontaneous())
    {
    this->disclaimer();
    }
}

//-----------------------------------------------------------------------------
void qSlicerAppMainWindow::disclaimer()
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
#define qSlicerAppMainWindowCore_connect(ACTION_NAME)   \
  this->connect(                                 \
    d->action##ACTION_NAME, SIGNAL(triggered()), \
    this->core(),                                \
    SLOT(on##ACTION_NAME##ActionTriggered()));
#define qSlicerAppMainWindow_connect(ACTION_NAME)   \
  this->connect(                                 \
    d->action##ACTION_NAME, SIGNAL(triggered()), \
    this,                                        \
    SLOT(on##ACTION_NAME##ActionTriggered()));

//-----------------------------------------------------------------------------
void qSlicerAppMainWindow::setupMenuActions()
{
  Q_D(qSlicerAppMainWindow);

  qSlicerAppMainWindowCore_connect(FileAddData);
  qSlicerAppMainWindowCore_connect(FileLoadData);
  qSlicerAppMainWindowCore_connect(FileImportScene);
  qSlicerAppMainWindowCore_connect(FileLoadScene);
  qSlicerAppMainWindowCore_connect(FileAddVolume);
  qSlicerAppMainWindowCore_connect(FileAddTransform);
  qSlicerAppMainWindowCore_connect(FileSaveScene);

  qSlicerAppMainWindowCore_connect(SDBSaveToDirectory);
  qSlicerAppMainWindowCore_connect(SDBSaveToMRB);
  qSlicerAppMainWindowCore_connect(SDBSaveToDCM);

  qSlicerAppMainWindowCore_connect(FileCloseScene);
  this->connect(d->actionFileExit, SIGNAL(triggered()),
                this, SLOT(close()));

  qSlicerAppMainWindowCore_connect(EditRecordMacro);
  qSlicerAppMainWindowCore_connect(EditPlayMacro);
  qSlicerAppMainWindowCore_connect(EditUndo);
  qSlicerAppMainWindowCore_connect(EditRedo);

  qSlicerAppMainWindow_connect(EditApplicationSettings);
  this->connect(d->actionCut, SIGNAL(triggered()),
                this, SLOT(onCutActionTriggered()));
  this->connect(d->actionCopy, SIGNAL(triggered()),
                this, SLOT(onCopyActionTriggered()));
  this->connect(d->actionPaste, SIGNAL(triggered()),
                this, SLOT(onPasteActionTriggered()));

  qSlicerAppMainWindow_connect(ViewExtensionsManager);

  d->actionViewLayoutConventional->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  d->actionViewLayoutConventionalWidescreen->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalWidescreenView);
  d->actionViewLayoutConventionalQuantitative->setData(vtkMRMLLayoutNode::SlicerLayoutConventionalQuantitativeView);
  d->actionViewLayoutFourUp->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
  d->actionViewLayoutFourUpQuantitative->setData(vtkMRMLLayoutNode::SlicerLayoutFourUpQuantitativeView);
  d->actionViewLayoutDual3D->setData(vtkMRMLLayoutNode::SlicerLayoutDual3DView);
  d->actionViewLayoutTriple3D->setData(vtkMRMLLayoutNode::SlicerLayoutTriple3DEndoscopyView);
  d->actionViewLayoutOneUp3D->setData(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
  d->actionViewLayoutOneUpQuantitative->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpQuantitativeView);
  d->actionViewLayoutOneUpRedSlice->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  d->actionViewLayoutOneUpYellowSlice->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
  d->actionViewLayoutOneUpGreenSlice->setData(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  d->actionViewLayoutTabbed3D->setData(vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
  d->actionViewLayoutTabbedSlice->setData(vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);
  d->actionViewLayoutCompare->setData(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  d->actionViewLayoutCompareWidescreen->setData(vtkMRMLLayoutNode::SlicerLayoutCompareWidescreenView);
  d->actionViewLayoutCompareGrid->setData(vtkMRMLLayoutNode::SlicerLayoutCompareGridView);
  d->actionViewLayoutThreeOverThree->setData(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeView);
  d->actionViewLayoutThreeOverThreeQuantitative->setData(vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeQuantitativeView);
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
#ifdef Slicer_USE_PYTHONQT
  if (d->Core->pythonConsole())
    {
    d->Core->pythonConsole()->installEventFilter(this);
    }
#endif

  qSlicerAppMainWindowCore_connect(HelpKeyboardShortcuts);
  qSlicerAppMainWindowCore_connect(HelpBrowseTutorials);
  qSlicerAppMainWindowCore_connect(HelpInterfaceDocumentation);
  qSlicerAppMainWindowCore_connect(HelpSlicerPublications);
  qSlicerAppMainWindowCore_connect(HelpAboutSlicerApp);

  qSlicerAppMainWindowCore_connect(HelpReportBugOrFeatureRequest);
  qSlicerAppMainWindowCore_connect(HelpVisualBlog);

  //connect ToolBars actions
  connect(d->actionWindowToolbarsResetToDefault, SIGNAL(triggered()),
          this, SLOT(restoreToolbars()));

  // Main ToolBar actions (where are actions for load data and save?
  connect(d->actionLoadDICOM, SIGNAL(triggered()),
          this, SLOT(loadDICOMActionTriggered()));

  qSlicerApplication * app = qSlicerApplication::application();

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  d->actionViewExtensionsManager->setVisible(
        app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool());
#else
  d->actionViewExtensionsManager->setVisible(false);
#endif
#ifndef Slicer_USE_PYTHONQT
  d->actionWindowPythonInteractor->setVisible(false);
#endif

#if defined Slicer_USE_QtTesting && defined Slicer_BUILD_CLI_SUPPORT
  if (app->commandOptions()->enableQtTesting() ||
      app->userSettings()->value("QtTesting/Enabled").toBool())
    {
    d->actionEditPlayMacro->setVisible(true);
    d->actionEditRecordMacro->setVisible(true);
    app->testingUtility()->addPlayer(new qSlicerCLIModuleWidgetEventPlayer());
    }
#endif
  Q_UNUSED(app);
}
#undef qSlicerAppMainWindowCore_connect


//---------------------------------------------------------------------------
void qSlicerAppMainWindow::loadDICOMActionTriggered()
{
//  Q_D(qSlicerAppMainWindow);
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
void qSlicerAppMainWindow::onEditApplicationSettingsActionTriggered()
{
  qSlicerApplication::application()->settingsDialog()->exec();
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onNewFileLoaded(const qSlicerIO::IOProperties& fileProperties)
{
  Q_D(qSlicerAppMainWindow);

  d->RecentlyLoadedFileProperties.removeAll(fileProperties);

  d->RecentlyLoadedFileProperties.enqueue(fileProperties);

  d->filterRecentlyLoadedFileProperties();

  d->setupRecentlyLoadedMenu(d->RecentlyLoadedFileProperties);

  // Keep the settings up-to-date
  qSlicerAppMainWindowPrivate::writeRecentlyLoadedFiles(d->RecentlyLoadedFileProperties);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onCopyActionTriggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != 0)
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
void qSlicerAppMainWindow::onPasteActionTriggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != 0)
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
void qSlicerAppMainWindow::onCutActionTriggered()
{
  QWidget* focused = QApplication::focusWidget();
  if (focused != 0)
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
void qSlicerAppMainWindow::onViewExtensionsManagerActionTriggered()
{
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerApplication * app = qSlicerApplication::application();
  app->openExtensionsManagerDialog();
#endif
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onModuleLoaded(const QString& moduleName)
{
  Q_D(qSlicerAppMainWindow);

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
    action->setParent(d->ModuleToolBar);
    }
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onModuleAboutToBeUnloaded(const QString& moduleName)
{
  Q_D(qSlicerAppMainWindow);

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
void qSlicerAppMainWindow::onMRMLSceneModified(vtkObject* sender)
{
  Q_UNUSED(sender);
  //Q_D(qSlicerAppMainWindow);
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
void qSlicerAppMainWindow::onLayoutActionTriggered(QAction* action)
{
  Q_D(qSlicerAppMainWindow);
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
void qSlicerAppMainWindow::onLayoutCompareActionTriggered(QAction* action)
{
  Q_D(qSlicerAppMainWindow);

  // std::cerr << "onLayoutCompareActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->core()->setLayout(d->actionViewLayoutCompare->data().toInt());
  this->core()->setLayoutNumberOfCompareViewRows(action->data().toInt());
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onLayoutCompareWidescreenActionTriggered(QAction* action)
{
  Q_D(qSlicerAppMainWindow);

  // std::cerr << "onLayoutCompareWidescreenActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->core()->setLayout(d->actionViewLayoutCompareWidescreen->data().toInt());
  this->core()->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onLayoutCompareGridActionTriggered(QAction* action)
{
  Q_D(qSlicerAppMainWindow);

  // std::cerr << "onLayoutCompareGridActionTriggered: " << action->text().toStdString() << std::endl;

  // we need to communicate both the layout change and the number of viewers.
  this->core()->setLayout(d->actionViewLayoutCompareGrid->data().toInt());
  this->core()->setLayoutNumberOfCompareViewRows(action->data().toInt());
  this->core()->setLayoutNumberOfCompareViewColumns(action->data().toInt());
}


//---------------------------------------------------------------------------
void qSlicerAppMainWindow::onLayoutChanged(int layout)
{
  Q_D(qSlicerAppMainWindow);
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
void qSlicerAppMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  qSlicerApplication::application()->ioManager()->dragEnterEvent(event);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::dropEvent(QDropEvent *event)
{
  qSlicerApplication::application()->ioManager()->dropEvent(event);
}

//---------------------------------------------------------------------------
void qSlicerAppMainWindow::setHomeModuleCurrent()
{
  Q_D(qSlicerAppMainWindow);
  QSettings settings;
  QString homeModule = settings.value("Modules/HomeModule").toString();
  d->ModuleSelectorToolBar->selectModule(homeModule);
}


//---------------------------------------------------------------------------
void qSlicerAppMainWindow::restoreToolbars()
{
  Q_D(qSlicerAppMainWindow);
  this->restoreState(d->StartupState);
}

//---------------------------------------------------------------------------
bool qSlicerAppMainWindow::eventFilter(QObject* object, QEvent* event)
{
  Q_D(qSlicerAppMainWindow);
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
