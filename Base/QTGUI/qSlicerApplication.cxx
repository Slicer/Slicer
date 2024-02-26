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
#include <QDesktopServices>
#include <QFile>
#include <QLabel>
#include <QtGlobal>
#include <QMainWindow>
#include <QDialog>
#include <QRandomGenerator>
#include <QSurfaceFormat>
#include <QSysInfo>
#include <QTextCodec>
#include <QVBoxLayout>

#if defined(Q_OS_WIN32)
# include <QtPlatformHeaders\QWindowsWindowFunctions> // for setHasBorderInFullScreen
#endif

#include "vtkSlicerConfigure.h"        // For Slicer_USE_*, Slicer_BUILD_*_SUPPORT
#include <vtkSlicerVersionConfigure.h> // For Slicer_VERSION_FULL

// CTK includes
#include <ctkColorDialog.h>
#include <ctkErrorLogModel.h>
#include <ctkErrorLogFDMessageHandler.h>
#include <ctkErrorLogQtMessageHandler.h>
#include <ctkErrorLogStreamMessageHandler.h>
#include <ctkITKErrorLogMessageHandler.h>
#include <ctkMessageBox.h>
#include <ctkPimpl.h>
#ifdef Slicer_USE_PYTHONQT
# include "ctkPythonConsole.h"
#endif
#include <ctkSettings.h>
#ifdef Slicer_USE_QtTesting
# include <ctkQtTestingUtility.h>
# include <ctkXMLEventObserver.h>
# include <ctkXMLEventSource.h>
#endif
#ifdef Slicer_BUILD_DICOM_SUPPORT
# include <ctkDICOMBrowser.h>
#endif
#include <ctkToolTipTrapper.h>
#include <ctkVTKErrorLogMessageHandler.h>

// QTGUI includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleRepresentation.h"
#include "qSlicerApplication.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerCoreApplication_p.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerPythonManager.h"
# include "qSlicerSettingsPythonPanel.h"
#endif
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
# include "qSlicerExtensionsManagerDialog.h"
# include "qSlicerExtensionsManagerModel.h"
# include "qSlicerSettingsExtensionsPanel.h"
#endif
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
# include "qSlicerApplicationUpdateManager.h"
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
#include "qSlicerSettingsUserInformationPanel.h"

// qMRMLWidget includes
#include "qMRMLEventBrokerConnection.h"

// qMRML includes
#ifdef Slicer_USE_QtTesting
# include <qMRMLCheckableNodeComboBoxEventPlayer.h>
# include <qMRMLNodeComboBoxEventPlayer.h>
# include <qMRMLNodeComboBoxEventTranslator.h>
# include <qMRMLTreeViewEventPlayer.h>
# include <qMRMLTreeViewEventTranslator.h>
#endif

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include <vtkSystemInformation.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSMP.h> // For VTK_SMP_BACKEND

#ifdef Slicer_USE_PYTHONQT

// Custom Python completer that temporarily disables logging when reading list of attributes.
// Disabling logging is needed because when Python enumerates all the methods then it executes
// the getter function of every property, which may log error/warning messages (because
// some properties are not set yet or the property is deprecated).

//-----------------------------------------------------------------------------
class ctkSlicerPythonConsoleCompleter : public ctkPythonConsoleCompleter
{
public:
  ctkSlicerPythonConsoleCompleter(ctkAbstractPythonManager& pythonManager, qSlicerApplication* app)
    : ctkPythonConsoleCompleter(pythonManager)
    , Application(app)
  {
  }

  void updateCompletionModel(const QString& completion) override
  {
    bool wasLoggingEnabled = this->setLoggingEnabled(false);
    ctkPythonConsoleCompleter::updateCompletionModel(completion);
    this->setLoggingEnabled(wasLoggingEnabled);
  }

  int cursorOffset(const QString& completion) override
  {
    bool wasLoggingEnabled = this->setLoggingEnabled(false);
    int result = ctkPythonConsoleCompleter::cursorOffset(completion);
    this->setLoggingEnabled(wasLoggingEnabled);
    return result;
  }

protected:
  bool setLoggingEnabled(bool enable)
  {
    if (!this->Application || !this->Application->errorLogModel())
    {
      return false;
    }
    ctkErrorLogAbstractMessageHandler* qtMessageHandler = this->Application->errorLogModel()->msgHandler("Qt");
    if (!qtMessageHandler)
    {
      return false;
    }
    return !qtMessageHandler->blockSignals(!enable);
  }

  qSlicerApplication* Application{ nullptr };
};
#endif

//-----------------------------------------------------------------------------
class qSlicerApplicationPrivate : public qSlicerCoreApplicationPrivate
{
  Q_DECLARE_PUBLIC(qSlicerApplication);

protected:
  qSlicerApplication* const q_ptr;

public:
  typedef qSlicerCoreApplicationPrivate Superclass;

  qSlicerApplicationPrivate(qSlicerApplication& object,
                            qSlicerCommandOptions* commandOptions,
                            qSlicerIOManager* ioManager);
  ~qSlicerApplicationPrivate() override;

  /// Convenient method regrouping all initialization code
  void init() override;

  /// Initialize application style
  void initStyle();

  QSettings* newSettings() override;

  QPointer<qSlicerLayoutManager> LayoutManager;
  ctkToolTipTrapper* ToolTipTrapper;
  // If MainWindow exists and the dialog is displayed then the MainWindow
  // must be set as parent to ensure correct Z order;
  // but that also transfers the ownership of the object, therefore we use QPointer
  // to keep track if the object is deleted already by the MainWindow.
  QPointer<ctkSettingsDialog> SettingsDialog;
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  QPointer<qSlicerExtensionsManagerDialog> ExtensionsManagerDialog;
  bool IsExtensionsManagerDialogOpen;
#endif
#ifdef Slicer_USE_QtTesting
  ctkQtTestingUtility* TestingUtility;
#endif
#ifdef Slicer_USE_PYTHONQT
  ctkErrorLogLevel::LogLevel PythonConsoleLogLevel;
#endif
};

//-----------------------------------------------------------------------------
// qSlicerApplicationPrivate methods

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::qSlicerApplicationPrivate(qSlicerApplication& object,
                                                     qSlicerCommandOptions* commandOptions,
                                                     qSlicerIOManager* ioManager)
  : qSlicerCoreApplicationPrivate(object, commandOptions, ioManager)
  , q_ptr(&object)
{
  this->ToolTipTrapper = nullptr;
  this->SettingsDialog = nullptr;
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  this->ExtensionsManagerDialog = nullptr;
  this->IsExtensionsManagerDialogOpen = false;
#endif
#ifdef Slicer_USE_QtTesting
  this->TestingUtility = nullptr;
#endif
#ifdef Slicer_USE_PYTHONQT
  this->PythonConsoleLogLevel = ctkErrorLogLevel::Warning;
#endif
}

//-----------------------------------------------------------------------------
qSlicerApplicationPrivate::~qSlicerApplicationPrivate()
{
  // Delete settings dialog. deleteLater would cause memory leaks on exit.
  // Settings dialog is displayed then MainWindow becomes its parent and
  // thus MainWindow is responsible for deleting it.
  // Set the parent to 'nullptr' removes this responsibility.
  if (this->SettingsDialog)
  {
    this->SettingsDialog->setParent(nullptr);
    delete this->SettingsDialog;
  }

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  if (this->ExtensionsManagerDialog)
  {
    this->ExtensionsManagerDialog->setParent(nullptr);
    delete this->ExtensionsManagerDialog;
    this->ExtensionsManagerDialog = nullptr;
  }
#endif
#ifdef Slicer_USE_QtTesting
  delete this->TestingUtility;
  this->TestingUtility = nullptr;
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
    // qSlicerCoreApplication class takes ownership of the pythonManager and
    // will be responsible to delete it
    q->setCorePythonManager(new qSlicerPythonManager());
    // qSlicerCoreApplication does not take ownership of PythonConsole, therefore
    // we have to delete it in the destructor if it is not deleted already
    // and not owned by a widget (it is owned and deleted by a widget if it is added
    // to the GUI)
    q->setPythonConsole(new ctkPythonConsole());
  }
#endif

  this->Superclass::init();

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    // Initialize method prints the welcome message and a prompt, so we need to set these colors now.
    // All the other colors will be set later in the main window.
    QPalette palette = qSlicerApplication::application()->palette();
    q->pythonConsole()->setWelcomeTextColor(palette.color(QPalette::Disabled, QPalette::WindowText));
    q->pythonConsole()->setPromptColor(palette.color(QPalette::Highlight));
    q->pythonConsole()->initialize(q->pythonManager());
    ctkSlicerPythonConsoleCompleter* completer = new ctkSlicerPythonConsoleCompleter(*q->pythonManager(), q);
    q->pythonConsole()->setCompleter(completer);
    QStringList autocompletePreferenceList;
    autocompletePreferenceList << "slicer"
                               << "slicer.mrmlScene"
                               << "qt.QPushButton";
    q->pythonConsole()->completer()->setAutocompletePreferenceList(autocompletePreferenceList);
    foreach (QAction* action, q->pythonConsole()->actions())
    {
      if (action->shortcut() == QKeySequence("Ctrl+H"))
      {
        // Remove action as "Ctrl+H" is reserved for going to the "Home" module
        q->pythonConsole()->removeAction(action);
        action->setParent(nullptr);
        break;
      }
    }
  }
#endif

  this->initStyle();

  this->ToolTipTrapper = new ctkToolTipTrapper(q);
  this->ToolTipTrapper->setToolTipsTrapped(false);
  this->ToolTipTrapper->setToolTipsWordWrapped(true);

  //----------------------------------------------------------------------------
  // Instantiate ErrorLogModel
  //----------------------------------------------------------------------------
  this->ErrorLogModel = QSharedPointer<ctkErrorLogModel>(new ctkErrorLogModel);
  this->ErrorLogModel->setLogEntryGrouping(true);
  this->ErrorLogModel->setTerminalOutputs(this->CoreCommandOptions->disableTerminalOutputs()
                                            ? ctkErrorLogTerminalOutput::None
                                            : ctkErrorLogTerminalOutput::All);
#if defined(Q_OS_WIN32) && !defined(Slicer_BUILD_WIN32_CONSOLE)
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

#ifdef Slicer_USE_PYTHONQT
  // Make ITK, VTK, Qt error messages show up in the Python console
  QSettings* userSettings = q->userSettings();
  ctkErrorLogLevel::LogLevel level =
    ctkErrorLogLevel::logLevelFromString(userSettings->value("Python/ConsoleLogLevel").toString());
  if (level >= 0)
  {
    q->setPythonConsoleLogLevel(level);
  }
  QObject::connect(
    this->ErrorLogModel.data(),
    SIGNAL(entryAdded(QDateTime, QString, ctkErrorLogLevel::LogLevel, QString, ctkErrorLogContext, QString)),
    q,
    SLOT(logToPythonConsole(QDateTime, QString, ctkErrorLogLevel::LogLevel, QString, ctkErrorLogContext, QString)));
#endif

  q->setupFileLogging();
  q->logApplicationInformation();

  //----------------------------------------------------------------------------
  // Settings Dialog
  //----------------------------------------------------------------------------
  // mainWindow is still nullptr, so the parent will be set later.
  this->SettingsDialog = new ctkSettingsDialog(nullptr);
  this->SettingsDialog->setResetButton(true);
  // Some settings panels are quite large, show maximize button to allow resizing with a single click
  this->SettingsDialog->setWindowFlags(this->SettingsDialog->windowFlags() | Qt::WindowMaximizeButtonHint);

  qSlicerSettingsGeneralPanel* generalPanel = new qSlicerSettingsGeneralPanel;
  this->SettingsDialog->addPanel(qSlicerApplication::tr("General"), generalPanel);

  qSlicerSettingsModulesPanel* settingsModulesPanel = new qSlicerSettingsModulesPanel;
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Modules"), settingsModulesPanel);

  qSlicerSettingsStylesPanel* settingsStylesPanel = new qSlicerSettingsStylesPanel(generalPanel);
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Appearance"), settingsStylesPanel);

  qSlicerSettingsViewsPanel* settingsViewsPanel = new qSlicerSettingsViewsPanel(generalPanel);
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Views"), settingsViewsPanel);

  qSlicerSettingsUserInformationPanel* settingsUserPanel = new qSlicerSettingsUserInformationPanel;
  settingsUserPanel->setUserInformation(this->AppLogic->GetUserInformation());
  this->SettingsDialog->addPanel(qSlicerApplication::tr("User"), settingsUserPanel);

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerSettingsExtensionsPanel* settingsExtensionsPanel = new qSlicerSettingsExtensionsPanel;
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Extensions"), settingsExtensionsPanel);
#endif
  qSlicerSettingsCachePanel* cachePanel = new qSlicerSettingsCachePanel;
  cachePanel->setCacheManager(this->MRMLScene->GetCacheManager());
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Cache"), cachePanel);

#ifdef Slicer_BUILD_I18N_SUPPORT
  qSlicerSettingsInternationalizationPanel* qtInternationalizationPanel = new qSlicerSettingsInternationalizationPanel;
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Internationalization"), qtInternationalizationPanel);
#endif

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    q->settingsDialog()->addPanel(qSlicerApplication::tr("Python"), new qSlicerSettingsPythonPanel);
  }
#endif

  qSlicerSettingsDeveloperPanel* developerPanel = new qSlicerSettingsDeveloperPanel;
  this->SettingsDialog->addPanel(qSlicerApplication::tr("Developer"), developerPanel);

  QObject::connect(this->SettingsDialog, SIGNAL(restartRequested()), q, SLOT(restart()));

  //----------------------------------------------------------------------------
  // Test Utility
  //----------------------------------------------------------------------------
#ifdef Slicer_USE_QtTesting
  this->TestingUtility = new ctkQtTestingUtility(nullptr);
  this->TestingUtility->addEventObserver("xml", new ctkXMLEventObserver(this->TestingUtility));
  ctkXMLEventSource* eventSource = new ctkXMLEventSource(this->TestingUtility);
  eventSource->setRestoreSettingsAuto(qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting));
  this->TestingUtility->addEventSource("xml", eventSource);

  // Translator and Player for MRML widget
  this->TestingUtility->addPlayer(new qMRMLCheckableNodeComboBoxEventPlayer());
  this->TestingUtility->addPlayer(new qMRMLNodeComboBoxEventPlayer());
  this->TestingUtility->addTranslator(new qMRMLNodeComboBoxEventTranslator());
  this->TestingUtility->addPlayer(new qMRMLTreeViewEventPlayer());
  this->TestingUtility->addTranslator(new qMRMLTreeViewEventTranslator());

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

#ifdef Slicer_USE_PYTHONQT
CTK_GET_CPP(qSlicerApplication, ctkErrorLogLevel::LogLevel, pythonConsoleLogLevel, PythonConsoleLogLevel);
CTK_SET_CPP(qSlicerApplication, ctkErrorLogLevel::LogLevel, setPythonConsoleLogLevel, PythonConsoleLogLevel);
#endif

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int& _argc, char** _argv)
  : Superclass(new qSlicerApplicationPrivate(*this, new qSlicerCommandOptions, nullptr), _argc, _argv)
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
#ifdef Slicer_USE_PYTHONQT
  // We have to delete PythonConsole if it is not deleted already
  // and not owned by a widget (it is owned and deleted by a widget if it is added
  // to the GUI).
  ctkPythonConsole* pythonConsolePtr = this->pythonConsole();
  if (pythonConsolePtr)
  {
    if (pythonConsolePtr->parent() == nullptr)
    {
      delete pythonConsolePtr;
    }
  }
#endif
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app = qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
bool qSlicerApplication::notify(QObject* receiver, QEvent* event)
{
  try
  {
    return QApplication::notify(receiver, event);
  }
  catch (std::bad_alloc& exception)
  {
    QString errorMessage;
    errorMessage = tr("%1 has caught an application error, ").arg(this->applicationName());
    errorMessage += tr("please save your work and restart.\n\n");
    errorMessage += tr("The application has run out of memory. ");
    if (!QSysInfo::kernelType().compare(/*no tr*/ "winnt"))
    {
      errorMessage +=
        tr("Increasing virtual memory size in system settings or adding more RAM may fix this issue.\n\n");
    }
    else if (!QSysInfo::kernelType().compare(/*no tr*/ "linux"))
    {
      errorMessage += tr("Increasing swap size in system settings or adding more RAM may fix this issue.\n\n");
    }
    else if (!QSysInfo::kernelType().compare(/*no tr*/ "darwin"))
    {
      errorMessage += tr("Increasing free disk space or adding more RAM may fix this issue.\n\n");
    }
    else
    {
      errorMessage += tr("Adding more RAM may fix this issue.\n\n");
    }
    errorMessage += tr("If you have a repeatable sequence of steps that causes this message, ");
    errorMessage += tr("please report the issue following instructions available at https://slicer.org\n\n\n");
    errorMessage += tr("The message detail is:\n\n");
    errorMessage += tr("Exception thrown in event: ") + exception.what();
    qCritical() << errorMessage;
    if (this->testAttribute(AA_EnableTesting))
    {
      this->exit(EXIT_FAILURE);
    }
    else
    {
      QMessageBox::critical(this->mainWindow(), tr("Application Error"), errorMessage);
    }
  }
  catch (std::exception& exception)
  {
    QString errorMessage;
    errorMessage = tr("%1 has caught an application error, ").arg(this->applicationName());
    errorMessage += tr("please save your work and restart.\n\n");
    errorMessage += tr("If you have a repeatable sequence of steps that causes this message, ");
    errorMessage += tr("please report the issue following instructions available at https://slicer.org\n\n\n");
    errorMessage += tr("The message detail is:\n\n");
    errorMessage += tr("Exception thrown in event: ") + exception.what();
    qCritical() << errorMessage;
    if (this->testAttribute(AA_EnableTesting))
    {
      this->exit(EXIT_FAILURE);
    }
    else
    {
      QMessageBox::critical(this->mainWindow(), tr("Application Error"), errorMessage);
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
qSlicerCommandOptions* qSlicerApplication::commandOptions() const
{
  qSlicerCommandOptions* _commandOptions = dynamic_cast<qSlicerCommandOptions*>(this->coreCommandOptions());
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
  if (qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    return nullptr;
  }
  qSlicerPythonManager* _pythonManager = qobject_cast<qSlicerPythonManager*>(this->corePythonManager());
  Q_ASSERT(_pythonManager);
  return _pythonManager;
}

//-----------------------------------------------------------------------------
ctkPythonConsole* qSlicerApplication::pythonConsole()
{
  if (qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    return nullptr;
  }
  return Superclass::pythonConsole();
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
    this->applicationLogic()->SetSliceLogics(d->LayoutManager ? d->LayoutManager.data()->mrmlSliceLogics() : nullptr);
    this->applicationLogic()->SetViewLogics(d->LayoutManager ? d->LayoutManager.data()->mrmlViewLogics() : nullptr);
    if (d->LayoutManager)
    {
      d->LayoutManager.data()->setMRMLColorLogic(this->applicationLogic()->GetColorLogic());
    }
  }
}

//-----------------------------------------------------------------------------
qSlicerLayoutManager* qSlicerApplication::layoutManager() const
{
  Q_D(const qSlicerApplication);
  return d->LayoutManager.data();
}

//-----------------------------------------------------------------------------
QMainWindow* qSlicerApplication::mainWindow() const
{
  foreach (QWidget* widget, this->topLevelWidgets())
  {
    QMainWindow* window = qobject_cast<QMainWindow*>(widget);
    if (window)
    {
      return window;
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::handlePreApplicationCommandLineArguments()
{
  this->Superclass::handlePreApplicationCommandLineArguments();

  qSlicerCoreCommandOptions* options = this->coreCommandOptions();
  Q_ASSERT(options);
  Q_UNUSED(options);
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

  if (options->exitAfterStartup())
  {
#ifdef Slicer_USE_PYTHONQT
    if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
      this->exit(this->corePythonManager()->pythonErrorOccured() ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    else
#endif
    {
      this->quit();
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerApplication::onSlicerApplicationLogicModified()
{
  if (this->layoutManager())
  {
    this->layoutManager()->setMRMLColorLogic(this->applicationLogic()->GetColorLogic());
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

  ctkMessageBox* confirmDialog = new ctkMessageBox(this->mainWindow());
  confirmDialog->setText(reason);
  confirmDialog->setIcon(QMessageBox::Question);
  confirmDialog->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  confirmDialog->setDontShowAgainSettingsKey("MainWindow/DontConfirmRestart");
  bool restartConfirmed = (confirmDialog->exec() == QMessageBox::Ok);
  confirmDialog->deleteLater();

  if (restartConfirmed)
  {
    this->restart();
  }
}

//-----------------------------------------------------------------------------
QString qSlicerApplication::nodeModule(vtkMRMLNode* node, double* confidence /*=nullptr*/) const
{
  QString mostSuitableModuleName = "Data";
  double mostSuitableModuleConfidence = 0.0;

  QString nodeClassName = node->GetClassName();

  // Modules that explicitly support the specified node type
  QStringList moduleNames = this->modulesAssociatedWithNodeType(nodeClassName);

  // Modules that support a parent class of the node
  QStringList classNames = this->allModuleAssociatedNodeTypes();
  foreach (const QString& className, classNames)
  {
    if (node->IsA(className.toUtf8()))
    {
      moduleNames << this->modulesAssociatedWithNodeType(className);
    }
  }

  foreach (const QString& moduleName, moduleNames)
  {
    qSlicerAbstractCoreModule* module = this->moduleManager()->module(moduleName);
    if (!module)
    {
      qWarning() << "Module " << moduleName << " associated with node class " << nodeClassName << " was not found";
      continue;
    }
    qSlicerAbstractModuleRepresentation* widget = module->widgetRepresentation();
    if (!widget)
    {
      qWarning() << "Module " << moduleName << " associated with node class " << nodeClassName
                 << " does not have widget";
      continue;
    }
    double nodeEditableConfidence = widget->nodeEditable(node);
    if (mostSuitableModuleConfidence < nodeEditableConfidence)
    {
      mostSuitableModuleName = moduleName;
      mostSuitableModuleConfidence = nodeEditableConfidence;
    }
  }
  if (confidence)
  {
    *confidence = mostSuitableModuleConfidence;
  }
  return mostSuitableModuleName;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::openNodeModule(vtkMRMLNode* node, QString role /*=QString()*/, QString context /*=QString()*/)
{
  if (!node)
  {
    qWarning() << Q_FUNC_INFO << " failed: node is invalid";
    return;
  }
  QString moduleName = this->nodeModule(node);
  qSlicerAbstractCoreModule* module = this->moduleManager()->module(moduleName);
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!moduleWithAction)
  {
    qWarning() << Q_FUNC_INFO << " failed: suitable module was not found";
    return;
  }
  // Select node (select node before activate because some modules create a default node
  // if activated without selecting a node)
  qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
  if (!widget)
  {
    qWarning() << Q_FUNC_INFO << " failed: suitable module widget was not found";
    return;
  }
  if (!widget->setEditedNode(node, role, context))
  {
    qWarning() << Q_FUNC_INFO << " failed: setEditedNode failed for node type " << node->GetClassName();
  }
  // Activate module widget
  moduleWithAction->action()->trigger();
}

// --------------------------------------------------------------------------
ctkSettingsDialog* qSlicerApplication::settingsDialog() const
{
  Q_D(const qSlicerApplication);
  return d->SettingsDialog;
}

void qSlicerApplication::openSettingsDialog(const QString& settingsPanel /*=QString()*/)
{
  Q_D(qSlicerApplication);

  if (!d->SettingsDialog->parent() && this->mainWindow())
  {
    // Set the parent before displaying the dialog to make sure that
    // when the user clicks on the application screen (even if outside the dialog)
    // then the main window is brought to the front.

    // setParent resets window flags, so save them and then restore
    Qt::WindowFlags windowFlags = d->SettingsDialog->windowFlags();
    d->SettingsDialog->setParent(this->mainWindow());
    d->SettingsDialog->setWindowFlags(windowFlags);

    // Make favorite module list changes update the module toolbar immediately
    // (without application restart).
    ctkSettingsPanel* settingsModulesPanel = d->SettingsDialog->panel(qSlicerApplication::tr("Modules"));
    if (settingsModulesPanel)
    {
      QObject::connect(
        settingsModulesPanel, SIGNAL(favoriteModulesChanged()), this->mainWindow(), SLOT(on_FavoriteModulesChanged()));
    }
  }

  // Reload settings to apply any changes that have been made outside of the
  // dialog (e.g. changes to module paths due to installing extensions). See
  // https://github.com/Slicer/Slicer/issues/3658.
  d->SettingsDialog->reloadSettings();

  if (!settingsPanel.isNull())
  {
    d->SettingsDialog->setCurrentPanel(settingsPanel);
  }

  // Now show the dialog
  d->SettingsDialog->exec();
}

// --------------------------------------------------------------------------
void qSlicerApplication::setHasBorderInFullScreen(bool hasBorder)
{
#if defined(Q_OS_WIN32)
  QWindowsWindowFunctions::setHasBorderInFullScreen(this->mainWindow()->windowHandle(), hasBorder);
#else
  Q_UNUSED(hasBorder);
#endif
}

// --------------------------------------------------------------------------
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
void qSlicerApplication::openExtensionsManagerDialog()
{
  Q_D(qSlicerApplication);
  // While the extensions manager is starting up, GUI events may be processed, causing repeated call of this method.
  // IsExtensionsManagerDialogOpen flag prevents creating multiple extensions manager dialogs.
  if (d->IsExtensionsManagerDialogOpen)
  {
    // already displayed
    return;
  }
  d->IsExtensionsManagerDialogOpen = true;

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  if (!d->ExtensionsManagerDialog)
  {
    d->ExtensionsManagerDialog = new qSlicerExtensionsManagerDialog(this->mainWindow());
  }
  if (!d->ExtensionsManagerDialog->extensionsManagerModel() && this->mainWindow())
  {
    // The first time the dialog is open, resize it.
    d->ExtensionsManagerDialog->resize(this->mainWindow()->size());
  }
  // This call takes most of the startup time
  d->ExtensionsManagerDialog->setExtensionsManagerModel(this->extensionsManagerModel());

  // Allow user to start typing immediately to locate an extension.
  d->ExtensionsManagerDialog->setFocusToSearchBox();

  QApplication::restoreOverrideCursor();

  bool accepted = (d->ExtensionsManagerDialog->exec() == QDialog::Accepted);
  if (accepted)
  {
    this->confirmRestart();
  }
  d->IsExtensionsManagerDialogOpen = false;
}

// --------------------------------------------------------------------------
void qSlicerApplication::openExtensionsCatalogWebsite()
{
  Q_D(qSlicerApplication);
  if (!this->extensionsManagerModel())
  {
    return;
  }
  QUrl url = this->extensionsManagerModel()->extensionsListUrl();
  QDesktopServices::openUrl(url);
}
#endif

// --------------------------------------------------------------------------
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
void qSlicerApplication::openApplicationDownloadWebsite()
{
  Q_D(qSlicerApplication);
  if (!this->applicationUpdateManager())
  {
    return;
  }
  QUrl url = this->applicationUpdateManager()->applicationDownloadPageUrl();
  QDesktopServices::openUrl(url);
}
#endif

// --------------------------------------------------------------------------
int qSlicerApplication::numberOfRecentLogFilesToKeep()
{
  Q_D(qSlicerApplication);

  QSettings* userSettings = this->userSettings();

  // Read number of log files to store value. If this value is missing,
  // then the group considered non-existent
  bool groupExists = false;
  int numberOfFilesToKeep = userSettings->value("LogFiles/NumberOfFilesToKeep").toInt(&groupExists);
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
  QSettings* userSettings = this->userSettings();
  QStringList logFilePaths;
  userSettings->beginGroup("LogFiles");
  int numberOfFilesToKeep = numberOfRecentLogFilesToKeep();
  for (int fileNumber = 0; fileNumber < numberOfFilesToKeep; ++fileNumber)
  {
    QString paddedFileNumber = QString("%1").arg(fileNumber, 3, 10, QChar('0')).toUpper();
    QString filePath = qSlicerCoreApplication::application()->toSlicerHomeAbsolutePath(
      userSettings->value(paddedFileNumber, "").toString());
    if (!filePath.isEmpty())
    {
      logFilePaths.append(filePath);
    }
  }
  userSettings->endGroup();
  return logFilePaths;
}

// --------------------------------------------------------------------------
QString qSlicerApplication::currentLogFile() const
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
  // Force using en-US locale, otherwise for example on a computer with
  // Egyptian Arabic (ar-EG) locale, Arabic numerals may be used.
  QLocale enUsLocale = QLocale(QLocale::English, QLocale::UnitedStates);
  QString currentLogFilePath = QString("%1/%2_%3_%4_%5_%6.log")
                                 .arg(tempDir)
                                 .arg(this->applicationName())
                                 .arg(qSlicerApplication::application()->applicationVersion())
                                 .arg(qSlicerApplication::application()->mainApplicationRevision())
                                 .arg(enUsLocale.toString(QDateTime::currentDateTime(), "yyyyMMdd_hhmmss"))
                                 .arg(QRandomGenerator::global()->generate() % 1000, 3, 10, QLatin1Char('0'));
  logFilePaths.prepend(currentLogFilePath);

  // Save settings
  int fileNumber = 0;
  QSettings* userSettings = this->userSettings();
  userSettings->beginGroup("LogFiles");
  userSettings->setValue("NumberOfFilesToKeep", numberOfFilesToKeep);

  foreach (QString filePath, logFilePaths)
  {
    // If the file is to keep then save it in the settings
    if (fileNumber < numberOfFilesToKeep)
    {
      QString paddedFileNumber = QString("%1").arg(fileNumber, 3, 10, QChar('0')).toUpper();
      userSettings->setValue(paddedFileNumber,
                             qSlicerCoreApplication::application()->toSlicerHomeRelativePath(filePath));
    }
    // Otherwise delete file
    else
    {
      QFile::remove(filePath);
    }
    ++fileNumber;
  }
  userSettings->endGroup();

  // Set current log file path
  d->ErrorLogModel->setFilePath(currentLogFilePath);
}

namespace
{

// --------------------------------------------------------------------------
struct qSlicerScopedTerminalOutputSettings
{
  qSlicerScopedTerminalOutputSettings(ctkErrorLogAbstractModel* errorLogModel,
                                      const ctkErrorLogTerminalOutput::TerminalOutputs& terminalOutputs)
    : ErrorLogModel(errorLogModel)
  {
    this->Saved = errorLogModel->terminalOutputs();
    errorLogModel->setTerminalOutputs(terminalOutputs);
  }
  ~qSlicerScopedTerminalOutputSettings() { this->ErrorLogModel->setTerminalOutputs(this->Saved); }
  ctkErrorLogAbstractModel* ErrorLogModel;
  ctkErrorLogTerminalOutput::TerminalOutputs Saved;
};

} // namespace

// --------------------------------------------------------------------------
void qSlicerApplication::logApplicationInformation() const
{
  // Log essential information about the application version and the host computer.
  // This helps in reproducing reported problems.

  qSlicerScopedTerminalOutputSettings currentTerminalOutputSettings(
    this->errorLogModel(),
    this->commandOptions()->displayApplicationInformation() ? this->errorLogModel()->terminalOutputs()
                                                            : ctkErrorLogTerminalOutput::None);

  QStringList titles = QStringList();
  titles << "Session start time "
         << "Slicer version ";
  if (this->isCustomMainApplication())
  {
    titles << (QString(Slicer_MAIN_PROJECT_APPLICATION_NAME) + " version ");
  }
  titles << "Operating system "
         << "Memory "
         << "CPU "
         << "VTK configuration "
         << "Qt configuration "
         << "Internationalization "
         << "Developer mode "
         << "Application path "
         << "Additional module paths ";

  int titleWidth = 0;
  foreach (const QString& title, titles)
  {
    if (title.length() > titleWidth)
    {
      titleWidth = title.length();
    }
  }
  titleWidth += 2;

  int titleIndex = 0;
  // Session start time
  // Force using en-US locale, otherwise for example on a computer with
  // Egyptian Arabic (ar-EG) locale, Arabic numerals may be used.
  QLocale enUsLocale = QLocale(QLocale::English, QLocale::UnitedStates);
  qDebug("%s: %s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         qPrintable(enUsLocale.toString(QDateTime::currentDateTime(), "yyyyMMdd_hhmmss")));

  // Slicer version
  qDebug("%s: %s (revision %s / %s) %s - %s %s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         Slicer_VERSION_FULL,
         qPrintable(this->revision()),
         qPrintable(this->repositoryRevision()),
         qPrintable(this->platform()),
         this->isInstalled() ? "installed" : "not installed",
#ifdef _DEBUG
         "debug"
#else
         "release"
#endif
  );

  // Custom application version
  if (this->isCustomMainApplication())
  {
    qDebug("%s: %s (revision %s / %s)",
           qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
           Slicer_MAIN_PROJECT_VERSION_FULL,
           qPrintable(Slicer_MAIN_PROJECT_REVISION),
           qPrintable(Slicer_MAIN_PROJECT_WC_REVISION));
  }

  // Operating system
  vtkNew<vtkSystemInformation> systemInfo;
  systemInfo->RunCPUCheck();
  systemInfo->RunOSCheck();
  systemInfo->RunMemoryCheck();

#ifdef Q_OS_WIN32
  qDebug() << qPrintable(QString("%0: %1 / %2 / (Build %3, Code Page %4) - %5")
                           .arg(titles.at(titleIndex++).leftJustified(titleWidth, '.'))
                           .arg(systemInfo->GetOSName() ? systemInfo->GetOSName() : "unknown")
                           .arg(systemInfo->GetOSRelease() ? systemInfo->GetOSRelease() : "unknown")
                           .arg(qSlicerApplication::windowsOSBuildNumber())
                           .arg(qSlicerApplication::windowsActiveCodePage())
                           .arg(systemInfo->Is64Bits() ? "64-bit" : "32-bit"));
#else
  // Get name of the codec that Qt uses for current locale.
  // We log this value to diagnose character encoding issues.
  // There may be problems if the value is not UTF-8
  // or if `locale charmap` command output is not UTF-8.
  QString localeCodecName("unknown");
  QTextCodec* localeCodec = QTextCodec::codecForLocale();
  if (localeCodec)
  {
    localeCodecName = localeCodec->toUnicode(localeCodec->name());
  }
  qDebug("%s: %s / %s / %s / %s - %s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         systemInfo->GetOSName() ? systemInfo->GetOSName() : "unknown",
         systemInfo->GetOSRelease() ? systemInfo->GetOSRelease() : "unknown",
         systemInfo->GetOSVersion() ? systemInfo->GetOSVersion() : "unknown",
         qPrintable(localeCodecName),
         systemInfo->Is64Bits() ? "64-bit" : "32-bit");
#endif

  // Memory
  size_t totalPhysicalMemoryMb = systemInfo->GetTotalPhysicalMemory();
  size_t totalVirtualMemoryMb = systemInfo->GetTotalVirtualMemory();
#if defined(_WIN32)
  // On Windows vtkSystemInformation::GetTotalVirtualMemory() returns the virtual address space,
  // while memory allocation fails if total page file size is reached. Therefore,
  // total page file size is a better indication of actually available memory for the process.
  // The issue has been fixed in kwSys release at the end of 2014, therefore when VTK is upgraded then
  // this workaround may not be needed anymore.
# if defined(_MSC_VER) && _MSC_VER < 1300
  MEMORYSTATUS ms;
  ms.dwLength = sizeof(ms);
  GlobalMemoryStatus(&ms);
  unsigned long totalPhysicalBytes = ms.dwTotalPhys;
  totalPhysicalMemoryMb = totalPhysicalBytes >> 10 >> 10;
  unsigned long totalVirtualBytes = ms.dwTotalPageFile;
  totalVirtualMemoryMb = totalVirtualBytes >> 10 >> 10;
# else
  MEMORYSTATUSEX ms;
  ms.dwLength = sizeof(ms);
  if (GlobalMemoryStatusEx(&ms))
  {
    DWORDLONG totalPhysicalBytes = ms.ullTotalPhys;
    totalPhysicalMemoryMb = totalPhysicalBytes >> 10 >> 10;
    DWORDLONG totalVirtualBytes = ms.ullTotalPageFile;
    totalVirtualMemoryMb = totalVirtualBytes >> 10 >> 10;
  }
# endif
#endif
  qDebug() << qPrintable(QString("%0: %1 MB physical, %2 MB virtual")
                           .arg(titles.at(titleIndex++).leftJustified(titleWidth, '.'))
                           .arg(totalPhysicalMemoryMb)
                           .arg(totalVirtualMemoryMb));

  // CPU
  unsigned int numberOfPhysicalCPU = systemInfo->GetNumberOfPhysicalCPU();
#if defined(_WIN32)
  // On Windows number of physical CPUs are computed incorrectly by vtkSystemInformation::GetNumberOfPhysicalCPU(),
  // if hyperthreading is enabled (typically 0 is reported), therefore get it directly from the OS instead.
  SYSTEM_INFO info;
  info.dwNumberOfProcessors = 0;
  GetSystemInfo(&info);
  numberOfPhysicalCPU = (unsigned int)info.dwNumberOfProcessors;
#endif

  unsigned int numberOfLogicalCPU = systemInfo->GetNumberOfLogicalCPU();

  qDebug("%s: %s %s, %d cores, %d logical processors",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         systemInfo->GetVendorString() ? systemInfo->GetVendorString() : "unknown",
         systemInfo->GetModelName() ? systemInfo->GetModelName() : "unknown",
         numberOfPhysicalCPU,
         numberOfLogicalCPU);

  // VTK configuration
  qDebug("%s: %s rendering, %s threading",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
#ifdef Slicer_VTK_RENDERING_USE_OpenGL2_BACKEND
         "OpenGL2",
#else
         "OpenGL",
#endif
         VTK_SMP_BACKEND);

  // Qt configuration
  QString openGLProfileStr = "unknown";
  QSurfaceFormat surfaceFormat = QSurfaceFormat::defaultFormat();
  switch (surfaceFormat.profile())
  {
    case QSurfaceFormat::NoProfile:
      openGLProfileStr = "no";
      break;
    case QSurfaceFormat::CoreProfile:
      openGLProfileStr = "core";
      break;
    case QSurfaceFormat::CompatibilityProfile:
      openGLProfileStr = "compatibility";
      break;
  }

  qDebug("%s: version %s, %s, requested OpenGL %d.%d (%s profile)",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         QT_VERSION_STR,
#ifdef Slicer_USE_PYTHONQT_WITH_OPENSSL
         "with SSL",
#else
         "no SSL",
#endif
         surfaceFormat.majorVersion(),
         surfaceFormat.minorVersion(),
         qPrintable(openGLProfileStr));

  QSettings settings;

  // Internationalization
#ifdef Slicer_BUILD_I18N_SUPPORT
  bool internationalizationEnabled =
    qSlicerApplication::application()->userSettings()->value("Internationalization/Enabled", true).toBool();
  QString language = qSlicerApplication::application()->userSettings()->value("language").toString();
  qDebug("%s: %s, language=%s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         internationalizationEnabled ? "enabled" : "disabled",
         qPrintable(language));
#else
  qDebug("%s: not supported", qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')));
#endif

  // Developer mode enabled
  bool developerModeEnabled = settings.value("Developer/DeveloperMode", false).toBool();
  qDebug("%s: %s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         developerModeEnabled ? "enabled" : "disabled");

  // Additional module paths
  // These paths are not converted to absolute path, because the raw values are moreuseful for troubleshooting.
  QStringList additionalModulePaths = this->revisionUserSettings()->value("Modules/AdditionalPaths").toStringList();

  qSlicerModuleFactoryManager* moduleFactoryManager = this->moduleManager()->factoryManager();
  foreach (const QString& extensionOrModulePath, this->commandOptions()->additionalModulePaths())
  {
    QStringList modulePaths = moduleFactoryManager->modulePaths(extensionOrModulePath);
    if (!modulePaths.empty())
    {
      additionalModulePaths << modulePaths;
    }
    else
    {
      additionalModulePaths << extensionOrModulePath;
    }
  }

  qDebug("%s: %s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         qPrintable(this->applicationDirPath()));

  qDebug("%s: %s",
         qPrintable(titles.at(titleIndex++).leftJustified(titleWidth, '.')),
         additionalModulePaths.isEmpty() ? "(none)" : qPrintable(additionalModulePaths.join(", ")));
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setRenderPaused(bool pause)
{
  Q_D(qSlicerApplication);

  if (d->LayoutManager)
  {
    d->LayoutManager.data()->setRenderPaused(pause);
  }

  emit renderPaused(pause);
}

//------------------------------------------------------------------------------
void qSlicerApplication::pauseRender()
{
  this->setRenderPaused(true);
}

//------------------------------------------------------------------------------
void qSlicerApplication::resumeRender()
{
  this->setRenderPaused(false);
}

#ifdef Slicer_BUILD_DICOM_SUPPORT
//-----------------------------------------------------------------------------
ctkDICOMBrowser* qSlicerApplication::createDICOMBrowserForMainDatabase()
{
  ctkDICOMBrowser* browser = new ctkDICOMBrowser(this->dicomDatabaseShared());

  // Allow database directory to be stored with a path relative to slicerHome
  browser->setDatabaseDirectoryBase(this->slicerHome());

  return browser;
}
#endif

//------------------------------------------------------------------------------
bool qSlicerApplication::launchDesigner(const QStringList& args /*=QStringList()*/)
{
  QString designerExecutable = this->slicerHome() + "/bin/SlicerDesigner";
#ifdef Q_OS_WIN32
  designerExecutable += ".exe";
#endif
  QProcess process;
  // Some extensions can modify the startup environment so that designer crashes during startup.
  // Therefore, we use the startup environment instead of the current environment.
  process.setProcessEnvironment(this->startupEnvironment());
  process.setProgram(designerExecutable);
  process.setArguments(args);
  return process.startDetached();
}

#ifdef Q_OS_WIN32

typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

//------------------------------------------------------------------------------
unsigned long int qSlicerApplication::windowsOSBuildNumber()
{
  HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
  if (!hMod)
  {
    return 0;
  }
  RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
  if (!fxPtr)
  {
    return 0;
  }
  RTL_OSVERSIONINFOW rovi = { 0 };
  rovi.dwOSVersionInfoSize = sizeof(rovi);
  if (fxPtr(&rovi) != (0x00000000))
  {
    return 0;
  }
  return rovi.dwBuildNumber;
}

//------------------------------------------------------------------------------
unsigned int qSlicerApplication::windowsActiveCodePage()
{
  UINT activeCodePage = GetACP();
  return activeCodePage;
}
#endif

//------------------------------------------------------------------------------
bool qSlicerApplication::isCodePageUtf8()
{
#ifdef Q_OS_WIN32
  return (qSlicerApplication::windowsActiveCodePage() == CP_UTF8);
#else
  return true;
#endif
}

//------------------------------------------------------------------------------
void qSlicerApplication::editNode(vtkObject*, void* callData, unsigned long)
{
  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(callData);
  if (node)
  {
    this->openNodeModule(node);
  }
}

//------------------------------------------------------------------------------
bool qSlicerApplication::loadFiles(const QStringList& filePaths,
                                   vtkMRMLMessageCollection* userMessagesInput /*=nullptr*/)
{
  // Even if the caller does not need messages, we need the message list so that we can display
  // messages to the user.
  vtkSmartPointer<vtkMRMLMessageCollection> userMessages = userMessagesInput;
  if (!userMessages)
  {
    userMessages = vtkSmartPointer<vtkMRMLMessageCollection>::New();
  }

  bool success = Superclass::loadFiles(filePaths, userMessages);
  qSlicerIOManager::showLoadNodesResultDialog(success, userMessages);
  return success;
}

#ifdef Slicer_USE_PYTHONQT
//---------------------------------------------------------------------------
void qSlicerApplication::logToPythonConsole(const QDateTime& currentDateTime,
                                            const QString& threadId,
                                            ctkErrorLogLevel::LogLevel logLevel,
                                            const QString& origin,
                                            const ctkErrorLogContext& context,
                                            const QString& text)
{
  Q_D(qSlicerApplication);
  Q_UNUSED(currentDateTime);
  Q_UNUSED(threadId);
  Q_UNUSED(context);

  if (d->PythonConsoleLogLevel == ctkErrorLogLevel::None // console logging disabled
      || logLevel < d->PythonConsoleLogLevel             // these levels are not displayed
      || origin == "Stream") // Python stream output is already displayed in the console, other output streams should
                             // not appear
  {
    return;
  }

  ctkPythonConsole* pythonConsole = this->pythonConsole();
  if (!pythonConsole)
  {
    return;
  }
  QString prefixedText;
  QStringList lines = text.split('\n');
  foreach (const QString& line, lines)
  {
    if (line.isEmpty())
    {
      continue;
    }
    prefixedText += "[" + origin + "] " + line + "\n";
  }
  if (logLevel < ctkErrorLogLevel::Warning)
  {
    pythonConsole->printOutputMessage(prefixedText);
  }
  else
  {
    pythonConsole->printErrorMessage(prefixedText);
  }
}
#endif
