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
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QMainWindow>

#include "vtkSlicerConfigure.h" // For Slicer_USE_*, Slicer_BUILD_*_SUPPORT

// CTK includes
#include <ctkColorDialog.h>
#include <ctkErrorLogModel.h>
#include <ctkErrorLogFDMessageHandler.h>
#include <ctkErrorLogQtMessageHandler.h>
#include <ctkErrorLogStreamMessageHandler.h>
#include <ctkITKErrorLogMessageHandler.h>
#include <ctkMessageBox.h>
#include <ctkSettings.h>
#ifdef Slicer_USE_QtTesting
#include <ctkQtTestingUtility.h>
#include <ctkXMLEventObserver.h>
#include <ctkXMLEventSource.h>
#endif
#include <ctkToolTipTrapper.h>
#include <ctkVTKErrorLogMessageHandler.h>

// QTGUI includes
#include "qSlicerAbstractModule.h"
#include "qSlicerApplication.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreApplication_p.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerPythonManager.h"
#endif
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
# include "qSlicerExtensionsManagerDialog.h"
# include "qSlicerSettingsExtensionsPanel.h"
#endif
#include "qSlicerSettingsCachePanel.h"
#include "qSlicerSettingsGeneralPanel.h"
#ifdef Slicer_BUILD_I18N_SUPPORT
# include "qSlicerSettingsInternationalizationPanel.h"
#endif
#include "qSlicerSettingsModulesPanel.h"
#include "qSlicerSettingsStylesPanel.h"
#include "qSlicerSettingsViewsPanel.h"
#include "qSlicerSettingsDeveloperPanel.h"

// qMRMLWidget includes
#include "qMRMLEventBrokerConnection.h"

// qMRML includes
#ifdef Slicer_USE_QtTesting
#include <qMRMLCheckableNodeComboBoxEventPlayer.h>
#include <qMRMLNodeComboBoxEventPlayer.h>
#include <qMRMLNodeComboBoxEventTranslator.h>
#include <qMRMLTreeViewEventPlayer.h>
#include <qMRMLTreeViewEventTranslator.h>
#endif

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include <vtkSystemInformation.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
class qSlicerApplicationPrivate : public qSlicerCoreApplicationPrivate
{
  Q_DECLARE_PUBLIC(qSlicerApplication);
protected:
  qSlicerApplication* const q_ptr;
public:
  typedef qSlicerCoreApplicationPrivate Superclass;

  qSlicerApplicationPrivate(qSlicerApplication& object,
                            qSlicerCommandOptions * commandOptions,
                            qSlicerIOManager * ioManager);
  virtual ~qSlicerApplicationPrivate();

  /// Convenient method regrouping all initialization code
  virtual void init();

  /// Initialize application style
  void initStyle();

  virtual QSettings* newSettings();

  /// ErrorLogModel - It should exist only one instance of the ErrorLogModel
  QSharedPointer<ctkErrorLogModel>            ErrorLogModel;

  QWeakPointer<qSlicerLayoutManager> LayoutManager;
  ctkToolTipTrapper*      ToolTipTrapper;
  ctkSettingsDialog*      SettingsDialog;
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerExtensionsManagerDialog* ExtensionsManagerDialog;
#endif
#ifdef Slicer_USE_QtTesting
  ctkQtTestingUtility*    TestingUtility;
#endif
};


//-----------------------------------------------------------------------------
// qSlicerApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::qSlicerApplicationPrivate(
    qSlicerApplication& object,
    qSlicerCommandOptions * commandOptions,
    qSlicerIOManager * ioManager)
  : qSlicerCoreApplicationPrivate(object, commandOptions, ioManager), q_ptr(&object)
{
  this->ToolTipTrapper = 0;
  this->SettingsDialog = 0;
#ifdef Slicer_USE_QtTesting
  this->TestingUtility = 0;
#endif
}

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::~qSlicerApplicationPrivate()
{
  delete this->SettingsDialog;
  this->SettingsDialog = 0;
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  delete this->ExtensionsManagerDialog;
  this->ExtensionsManagerDialog =0;
#endif
#ifdef Slicer_USE_QtTesting
  delete this->TestingUtility;
  this->TestingUtility = 0;
#endif
}

//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::init()
{
  Q_Q(qSlicerApplication);

  ctkVTKConnectionFactory::setInstance(new qMRMLConnectionFactory);

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    // Note: qSlicerCoreApplication class takes ownership of the pythonManager and
    // will be responsible to delete it
    q->setCorePythonManager(new qSlicerPythonManager());
    }
#endif

  this->Superclass::init();

  this->initStyle();

  this->ToolTipTrapper = new ctkToolTipTrapper(q);
  this->ToolTipTrapper->setToolTipsTrapped(false);
  this->ToolTipTrapper->setToolTipsWordWrapped(true);

  //----------------------------------------------------------------------------
  // Instantiate ErrorLogModel
  //----------------------------------------------------------------------------
  this->ErrorLogModel = QSharedPointer<ctkErrorLogModel>(new ctkErrorLogModel);
  this->ErrorLogModel->setLogEntryGrouping(true);
  this->ErrorLogModel->setTerminalOutputs(
        this->CoreCommandOptions->disableTerminalOutputs() ?
          ctkErrorLogTerminalOutput::None : ctkErrorLogTerminalOutput::All);
#if defined (Q_OS_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
  // Must not register ctkErrorLogFDMessageHandler when building a window-based
  // (non-console) application because this handler would not
  // let the application to quit when the last window is closed.
#else
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogFDMessageHandler);
#endif
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogQtMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogStreamMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkITKErrorLogMessageHandler);
  this->ErrorLogModel->registerMsgHandler(new ctkVTKErrorLogMessageHandler);
  this->ErrorLogModel->setAllMsgHandlerEnabled(true);

  q->setupFileLogging();

  if (!this->CoreCommandOptions->displayMessageAndExit())
    {
    q->displayApplicationInformations();
    }

  //----------------------------------------------------------------------------
  // Settings Dialog
  //----------------------------------------------------------------------------
  this->SettingsDialog = new ctkSettingsDialog(0);
  this->SettingsDialog->setResetButton(true);

  qSlicerSettingsGeneralPanel* generalPanel = new qSlicerSettingsGeneralPanel;
  this->SettingsDialog->addPanel("General", generalPanel);

  qSlicerSettingsModulesPanel * settingsModulesPanel = new qSlicerSettingsModulesPanel;
  this->SettingsDialog->addPanel("Modules", settingsModulesPanel);

  qSlicerSettingsStylesPanel* settingsStylesPanel =
    new qSlicerSettingsStylesPanel(generalPanel);
  this->SettingsDialog->addPanel("Appearance", settingsStylesPanel);

  qSlicerSettingsViewsPanel* settingsViewsPanel =
    new qSlicerSettingsViewsPanel(generalPanel);
  this->SettingsDialog->addPanel("Views", settingsViewsPanel);

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerSettingsExtensionsPanel * settingsExtensionsPanel = new qSlicerSettingsExtensionsPanel;
  this->SettingsDialog->addPanel("Extensions", settingsExtensionsPanel);
#endif
  qSlicerSettingsCachePanel* cachePanel = new qSlicerSettingsCachePanel;
  cachePanel->setCacheManager(this->MRMLScene->GetCacheManager());
  this->SettingsDialog->addPanel("Cache", cachePanel);

#ifdef Slicer_BUILD_I18N_SUPPORT
  qSlicerSettingsInternationalizationPanel* qtInternationalizationPanel =
      new qSlicerSettingsInternationalizationPanel;
  this->SettingsDialog->addPanel("Internationalization", qtInternationalizationPanel);
#endif

  qSlicerSettingsDeveloperPanel* developerPanel = new qSlicerSettingsDeveloperPanel;
  this->SettingsDialog->addPanel("Developer", developerPanel);

  QObject::connect(this->SettingsDialog, SIGNAL(restartRequested()),
                   q, SLOT(restart()));

  //----------------------------------------------------------------------------
  // Dialogs
  //----------------------------------------------------------------------------
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  this->ExtensionsManagerDialog = new qSlicerExtensionsManagerDialog(0);
#endif

  //----------------------------------------------------------------------------
  // Test Utility
  //----------------------------------------------------------------------------
#ifdef Slicer_USE_QtTesting
  this->TestingUtility = new ctkQtTestingUtility(0);
  this->TestingUtility->addEventObserver(
      "xml", new ctkXMLEventObserver(this->TestingUtility));
  ctkXMLEventSource* eventSource = new ctkXMLEventSource(this->TestingUtility);
  eventSource->setRestoreSettingsAuto(
      qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting));
  this->TestingUtility->addEventSource("xml", eventSource);

  // Translator and Player for MRML widget
  this->TestingUtility->addPlayer(
      new qMRMLCheckableNodeComboBoxEventPlayer());
  this->TestingUtility->addPlayer(
      new qMRMLNodeComboBoxEventPlayer());
  this->TestingUtility->addTranslator(
      new qMRMLNodeComboBoxEventTranslator());
  this->TestingUtility->addPlayer(
      new qMRMLTreeViewEventPlayer());
  this->TestingUtility->addTranslator(
      new qMRMLTreeViewEventTranslator());

  // Player for the CLI Module || cannot be added for the moment ...
#endif
}
/*
#if !defined (QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loaderV2,
    (QIconEngineFactoryInterfaceV2_iid, QLatin1String("/iconengines"), Qt::CaseInsensitive))
#endif
*/
//-----------------------------------------------------------------------------
void qSlicerApplicationPrivate::initStyle()
{
  // Force showing the icons in the menus even if the native OS style
  // discourages it
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
}

//-----------------------------------------------------------------------------
QSettings* qSlicerApplicationPrivate::newSettings()
{
  Q_Q(qSlicerApplication);
  return new ctkSettings(q);
}

//-----------------------------------------------------------------------------
// qSlicerApplication methods

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &_argc, char **_argv)
  : Superclass(new qSlicerApplicationPrivate(*this, new qSlicerCommandOptions, 0), _argc, _argv)
{
  Q_D(qSlicerApplication);
  d->init();
  // Note: Since QWidget/QDialog requires a QApplication to be successfully instantiated,
  //       qSlicerIOManager is not added to the constructor initialization list.
  //       Indeed, internally qSlicerIOManager registers qSlicerDataDialog, ...
  d->CoreIOManager = QSharedPointer<qSlicerIOManager>(new qSlicerIOManager);
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app = qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
bool qSlicerApplication::notify(QObject *receiver, QEvent *event)
{
  try
    {
    return QApplication::notify(receiver, event);
    }
  catch( std::exception& exception )
    {
    QString errorMessage;
    errorMessage = tr("Slicer has caught an internal error.\n\n");
    errorMessage += tr("You may be able to continue from this point, but results are undefined.\n\n");
    errorMessage += tr("Suggested action is to save your work and restart.");
    errorMessage += tr("\n\nIf you have a repeatable sequence of steps that causes this message, ");
    errorMessage += tr("please report the issue following instructions available at http://slicer.org\n\n\n");
    errorMessage += tr("The message detail is:\n\n");
    errorMessage += tr("Exception thrown in event: ") + exception.what();
    qCritical() << errorMessage;
    QMessageBox::critical(this->mainWindow(),tr("Internal Error"), errorMessage);
    }
  return false;
}

//-----------------------------------------------------------------------------
ctkErrorLogModel* qSlicerApplication::errorLogModel()const
{
  Q_D(const qSlicerApplication);
  return d->ErrorLogModel.data();
}

//-----------------------------------------------------------------------------
qSlicerCommandOptions* qSlicerApplication::commandOptions()
{
  qSlicerCommandOptions* _commandOptions =
    dynamic_cast<qSlicerCommandOptions*>(this->coreCommandOptions());
  Q_ASSERT(_commandOptions);
  return _commandOptions;
}

//-----------------------------------------------------------------------------
qSlicerIOManager* qSlicerApplication::ioManager()
{
  qSlicerIOManager* _ioManager = dynamic_cast<qSlicerIOManager*>(this->coreIOManager());
  Q_ASSERT(_ioManager);
  return _ioManager;
}

#ifdef Slicer_USE_PYTHONQT
//-----------------------------------------------------------------------------
qSlicerPythonManager* qSlicerApplication::pythonManager()
{
  qSlicerPythonManager* _pythonManager = 0;
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    _pythonManager = qobject_cast<qSlicerPythonManager*>(this->corePythonManager());
    Q_ASSERT(_pythonManager);
    }

  return _pythonManager;
}
#endif

#ifdef Slicer_USE_QtTesting
//-----------------------------------------------------------------------------
ctkQtTestingUtility* qSlicerApplication::testingUtility()
{
  Q_D(const qSlicerApplication);
  return d->TestingUtility;
}
#endif

//-----------------------------------------------------------------------------
void qSlicerApplication::setLayoutManager(qSlicerLayoutManager* layoutManager)
{
  Q_D(qSlicerApplication);
  d->LayoutManager = layoutManager;
  if (this->applicationLogic())
    {
    this->applicationLogic()->SetSliceLogics(
      d->LayoutManager? d->LayoutManager.data()->mrmlSliceLogics() : 0);
    if (d->LayoutManager)
      {
      d->LayoutManager.data()->setMRMLColorLogic(this->applicationLogic()->GetColorLogic());
      }
    }
}

//-----------------------------------------------------------------------------
qSlicerLayoutManager* qSlicerApplication::layoutManager()const
{
  Q_D(const qSlicerApplication);
  return d->LayoutManager.data();
}

//-----------------------------------------------------------------------------
QMainWindow* qSlicerApplication::mainWindow()const
{
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    QMainWindow* window = qobject_cast<QMainWindow*>(widget);
    if (window)
      {
      return window;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::handlePreApplicationCommandLineArguments()
{
  this->Superclass::handlePreApplicationCommandLineArguments();

  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::handleCommandLineArguments()
{
  qSlicerCommandOptions* options = this->commandOptions();
  Q_ASSERT(options);

  if (options->disableMessageHandlers())
    {
    this->errorLogModel()->disableAllMsgHandler();
    }

  this->Superclass::handleCommandLineArguments();

  this->setToolTipsEnabled(!options->disableToolTips());
}

//-----------------------------------------------------------------------------
void qSlicerApplication::onSlicerApplicationLogicModified()
{
  if (this->layoutManager())
    {
    this->layoutManager()->setMRMLColorLogic(
      this->applicationLogic()->GetColorLogic());
    }
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setToolTipsEnabled(bool enable)
{
  Q_D(qSlicerApplication);
  d->ToolTipTrapper->setToolTipsTrapped(!enable);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::confirmRestart(QString reason)
{
  if (reason.isEmpty())
    {
    reason = tr("Are you sure you want to restart?");
    }

  ctkMessageBox confirmDialog;
  confirmDialog.setText(reason);
  confirmDialog.setIcon(QMessageBox::Question);
  confirmDialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  confirmDialog.setDontShowAgainSettingsKey( "MainWindow/DontConfirmRestart" );
  bool restartConfirmed = (confirmDialog.exec() == QMessageBox::Ok);

  if (restartConfirmed)
    {
    this->restart();
    }
}

//-----------------------------------------------------------------------------
QString qSlicerApplication::nodeModule(vtkMRMLNode* node)const
{
  QString nodeClassName = node->GetClassName();
  if (node->IsA("vtkMRMLCameraNode") ||
      node->IsA("vtkMRMLViewNode"))
    {
    return "Cameras";
    }
  else if (node->IsA("vtkMRMLSliceNode") ||
           node->IsA("vtkMRMLSliceCompositeNode") ||
           node->IsA("vtkMRMLSliceLayerNode"))
    {
    return "SliceController";
    }
  else if (node->IsA("vtkMRMLMarkupsNode") ||
           node->IsA("vtkMRMLMarkupsDisplayNode") ||
           node->IsA("vtkMRMLMarkupsStorageNode") ||
           node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    return "Markups";
    }
  else if (node->IsA("vtkMRMLAnnotationNode") ||
           node->IsA("vtkMRMLAnnotationDisplayNode") ||
           node->IsA("vtkMRMLAnnotationStorageNode") ||
           node->IsA("vtkMRMLAnnotationHierarchyNode"))
    {
    return "Annotations";
    }
  else if (node->IsA("vtkMRMLTransformNode") ||
           node->IsA("vtkMRMLTransformStorageNode"))
    {
    return "Transforms";
    }
  else if (node->IsA("vtkMRMLColorNode"))
    {
    return "Colors";
    }
  else if (nodeClassName.contains("vtkMRMLFiberBundle"))
    {
    return "TractographyDisplay";
    }
  else if (node->IsA("vtkMRMLModelNode") ||
           node->IsA("vtkMRMLModelDisplayNode") ||
           node->IsA("vtkMRMLModelHierarchyNode") ||
           node->IsA("vtkMRMLModelStorageNode"))
    {
    return "Models";
    }
  else if (node->IsA("vtkMRMLSceneViewNode") ||
           node->IsA("vtkMRMLSceneViewStorageNode"))
    {
    return "SceneViews";
    }
  else if (node->IsA("vtkMRMLVolumeNode") ||
           node->IsA("vtkMRMLVolumeDisplayNode") ||
           node->IsA("vtkMRMLVolumeArchetypeStorageNode") ||
           node->IsA("vtkMRMLVolumeHeaderlessStorageNode"))
    {
    return "Volumes";
    }
  else if (node->IsA("vtkMRMLVolumePropertyNode") ||
           node->IsA("vtkMRMLVolumePropertyStorageNode") ||
           node->IsA("vtkMRMLVolumeRenderingDisplayNode"))
    {
    return "VolumeRendering";
    }
  qWarning() << "Couldn't find a module for node class" << node->GetClassName();
  return "data";
}

//-----------------------------------------------------------------------------
void qSlicerApplication::openNodeModule(vtkMRMLNode* node)
{
  QString moduleName = this->nodeModule(node);
  qSlicerAbstractCoreModule* module = this->moduleManager()->module(moduleName);
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (moduleWithAction)
    {
    moduleWithAction->action()->trigger();
    }
}

// --------------------------------------------------------------------------
ctkSettingsDialog* qSlicerApplication::settingsDialog()const
{
  Q_D(const qSlicerApplication);
  return d->SettingsDialog;
}

// --------------------------------------------------------------------------
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
void qSlicerApplication::openExtensionsManagerDialog()
{
  Q_D(qSlicerApplication);
  if (!d->ExtensionsManagerDialog->extensionsManagerModel() &&
      this->mainWindow())
    {
    // The first time the dialog is open, resize it.
    d->ExtensionsManagerDialog->resize(this->mainWindow()->size());
    }
  d->ExtensionsManagerDialog->setExtensionsManagerModel(
        this->extensionsManagerModel());
  if (d->ExtensionsManagerDialog->exec() == QDialog::Accepted)
    {
    this->confirmRestart();
    }
}
#endif

// --------------------------------------------------------------------------
int qSlicerApplication::numberOfRecentLogFilesToKeep()
{
  Q_D(qSlicerApplication);

  QSettings* revisionUserSettings = this->revisionUserSettings();

  // Read number of log files to store value. If this value is missing,
  // then the group considered non-existent
  bool groupExists = false;
  int numberOfFilesToKeep = revisionUserSettings->value(
    "LogFiles/NumberOfFilesToKeep").toInt(&groupExists);
  if (!groupExists)
    {
    // Get default value from the ErrorLogModel if value is not set in settings
    numberOfFilesToKeep = d->ErrorLogModel->numberOfFilesToKeep();
    }
  else
    {
    d->ErrorLogModel->setNumberOfFilesToKeep(numberOfFilesToKeep);
    }

  return numberOfFilesToKeep;
}

// --------------------------------------------------------------------------
QStringList qSlicerApplication::recentLogFiles()
{
  QSettings* revisionUserSettings = this->revisionUserSettings();
  QStringList logFilePaths;
  revisionUserSettings->beginGroup("LogFiles");
  int numberOfFilesToKeep = numberOfRecentLogFilesToKeep();
  for (int fileNumber = 0; fileNumber < numberOfFilesToKeep; ++fileNumber)
    {
    QString paddedFileNumber = QString("%1").arg(fileNumber, 3, 10, QChar('0')).toUpper();
    QString filePath = revisionUserSettings->value(paddedFileNumber, "").toString();
    if (!filePath.isEmpty())
      {
      logFilePaths.append(filePath);
      }
    }
  revisionUserSettings->endGroup();
  return logFilePaths;
}

// --------------------------------------------------------------------------
QString qSlicerApplication::currentLogFile()const
{
  Q_D(const qSlicerApplication);
  return d->ErrorLogModel->filePath();
}

// --------------------------------------------------------------------------
void qSlicerApplication::setupFileLogging()
{
  Q_D(qSlicerApplication);

  d->ErrorLogModel->setFileLoggingEnabled(true);

  int numberOfFilesToKeep = numberOfRecentLogFilesToKeep();

  // Read saved log file paths into a list so that it can be shifted and
  // written out along with the new log file name.
  QStringList logFilePaths = recentLogFiles();

  // Add new log file path for the current session
  QString tempDir = this->temporaryPath();
  QString currentLogFilePath = tempDir + QString("/Slicer_") +
    this->repositoryRevision() + QString("_") +
    QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") +
    QString(".log");
  logFilePaths.prepend(currentLogFilePath);

  // Save settings
  int fileNumber = 0;
  QSettings* revisionUserSettings = this->revisionUserSettings();
  revisionUserSettings->beginGroup("LogFiles");
  revisionUserSettings->setValue("NumberOfFilesToKeep", numberOfFilesToKeep);
  foreach (QString filePath, logFilePaths)
    {
    // If the file is to keep then save it in the settings
    if (fileNumber < numberOfFilesToKeep)
      {
      QString paddedFileNumber = QString("%1").arg(fileNumber, 3, 10, QChar('0')).toUpper();
      revisionUserSettings->setValue(paddedFileNumber, filePath);
      }
    // Otherwise delete file
    else
      {
      QFile::remove(filePath);
      }
    ++fileNumber;
    }
  revisionUserSettings->endGroup();

  // Set current log file path
  d->ErrorLogModel->setFilePath(currentLogFilePath);
}

// --------------------------------------------------------------------------
void qSlicerApplication::displayApplicationInformations() const
{
  // Log essential information about the application version and the host computer.
  // This helps in reproducing reported problems.

  QStringList titles = QStringList()
      << "Session start time "
      << "Slicer version "
      << "Operating system "
      << "Memory "
      << "CPU "
      << "Developer mode enabled "
      << "Prefer executable CLI "
      << "Additional module paths ";

  int titleWidth = 0;
  foreach(const QString& title, titles)
    {
    if (title.length() > titleWidth)
      {
      titleWidth = title.length();
      }
    }
  titleWidth += 2;

  // Session start time
  qDebug("%s: %s",
         qPrintable(titles.at(0).leftJustified(titleWidth, '.')),
         qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

  // Slicer version
  qDebug("%s: %s (revision %s) %s - %s",
         qPrintable(titles.at(1).leftJustified(titleWidth, '.')),
         Slicer_VERSION_FULL, qPrintable(this->repositoryRevision()),
         qPrintable(this->platform()),
         this->isInstalled() ? "installed" : "not installed");

  // Operating system
  vtkNew<vtkSystemInformation> systemInfo;
  systemInfo->RunCPUCheck();
  systemInfo->RunOSCheck();
  systemInfo->RunMemoryCheck();

  qDebug("%s: %s / %s / %s - %s",
         qPrintable(titles.at(2).leftJustified(titleWidth, '.')),
         systemInfo->GetOSName() ? systemInfo->GetOSName() : "unknown",
         systemInfo->GetOSRelease() ? systemInfo->GetOSRelease() : "unknown",
         systemInfo->GetOSVersion() ? systemInfo->GetOSVersion() : "unknown" ,
         systemInfo->Is64Bits() ? "64-bit" : "32-bit");

  // Memory
  size_t totalPhysicalMemoryMb = systemInfo->GetTotalPhysicalMemory();
  size_t totalVirtualMemoryMb = systemInfo->GetTotalVirtualMemory();
#if defined(_WIN32)
  // On Windows vtkSystemInformation::GetTotalVirtualMemory() returns the virtual address space,
  // while memory allocation fails if total page file size is reached. Therefore,
  // total page file size is a better indication of actually available memory for the process.
  // The issue has been fixed in kwSys release at the end of 2014, therefore when VTK is upgraded then
  // this workaround may not be needed anymore.
#if defined(_MSC_VER) && _MSC_VER < 1300
  MEMORYSTATUS ms;
  ms.dwLength = sizeof(ms);
  GlobalMemoryStatus(&ms);
  unsigned long totalPhysicalBytes = ms.dwTotalPhys;
  totalPhysicalMemoryMb = totalPhysicalBytes>>10>>10;
  unsigned long totalVirtualBytes = ms.dwTotalPageFile;
  totalVirtualMemoryMb = totalVirtualBytes>>10>>10;
#else
  MEMORYSTATUSEX ms;
  ms.dwLength = sizeof(ms);
  if (GlobalMemoryStatusEx(&ms))
    {
    DWORDLONG totalPhysicalBytes = ms.ullTotalPhys;
    totalPhysicalMemoryMb = totalPhysicalBytes>>10>>10;
    DWORDLONG totalVirtualBytes = ms.ullTotalPageFile;
    totalVirtualMemoryMb = totalVirtualBytes>>10>>10;
    }
#endif
#endif
  qDebug() << qPrintable(QString("%0: %1 MB physical, %2 MB virtual")
                         .arg(titles.at(3).leftJustified(titleWidth, '.'))
                         .arg(totalPhysicalMemoryMb)
                         .arg(totalVirtualMemoryMb));

  // CPU
  unsigned int numberOfPhysicalCPU = systemInfo->GetNumberOfPhysicalCPU();
#if defined(_WIN32)
  // On Windows number of physical CPUs are computed incorrectly by vtkSystemInformation::GetNumberOfPhysicalCPU(),
  // if hyperthreading is enabled (typically 0 is reported), therefore get it directly from the OS instead.
  SYSTEM_INFO info;
  info.dwNumberOfProcessors = 0;
  GetSystemInfo (&info);
  numberOfPhysicalCPU = (unsigned int) info.dwNumberOfProcessors;
#endif

  qDebug("%s: %s %.3f MHz, %d cores",
         qPrintable(titles.at(4).leftJustified(titleWidth, '.')),
         systemInfo->GetVendorString() ? systemInfo->GetVendorString() : "unknown",
         systemInfo->GetProcessorClockFrequency()/1000,
         numberOfPhysicalCPU);

  QSettings settings;

  // Developer mode enabled
  bool developerModeEnabled = settings.value("Developer/DeveloperMode", false).toBool();
  qDebug("%s: %s",
         qPrintable(titles.at(5).leftJustified(titleWidth, '.')),
         developerModeEnabled ? "yes" : "no");

  // Prefer executable CLI
  bool preferExecutableCli = settings.value("Modules/PreferExecutableCLI", false).toBool();
  qDebug("%s: %s",
         qPrintable(titles.at(6).leftJustified(titleWidth, '.')),
         preferExecutableCli ? "yes" : "no");

  // Additional module paths
  QStringList additionalModulePaths =
      this->revisionUserSettings()->value("Modules/AdditionalPaths").toStringList();

  qDebug("%s: %s",
         qPrintable(titles.at(7).leftJustified(titleWidth, '.')),
         additionalModulePaths.isEmpty() ? "(none)" : qPrintable(additionalModulePaths.join(", ")));

}
