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

#ifndef __qSlicerCoreApplication_h
#define __qSlicerCoreApplication_h

// Qt includes
#include <QApplication>
#include <QMetaType>
#include <QProcessEnvironment>
#include <QStringList>
#include <QVariant>

// CTK includes
#include <ctkVTKObject.h>

// SlicerCore includes
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT
#include "qSlicerBaseQTCoreExport.h"

#ifdef Slicer_BUILD_DICOM_SUPPORT
class ctkDICOMDatabase;
#endif
class ctkErrorLogAbstractModel;
class QSettings;
class qSlicerCoreIOManager;
class qSlicerCoreCommandOptions;
class qSlicerCoreApplicationPrivate;
class qSlicerModuleManager;
#ifdef Slicer_USE_PYTHONQT
class qSlicerCorePythonManager;
class ctkPythonConsole;
#endif
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
class qSlicerExtensionsManagerModel;
#endif
class vtkDataIOManagerLogic;
class vtkSlicerApplicationLogic;
class vtkMRMLAbstractLogic;
class vtkMRMLApplicationLogic;
class vtkMRMLMessageCollection;
class vtkMRMLRemoteIOLogic;
class vtkMRMLScene;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCoreApplication : public QApplication
{
  Q_OBJECT
  QVTK_OBJECT
  Q_ENUMS(ReturnCode)

  /// This property holds the path where the Slicer application is.
  /// For example, for an installed Slicer on Windows, the path can be
  /// "C:\Program Files (x86)\Slicer 4.4.0\".
  /// \sa slicerHome(), temporaryPath, isInstalled
  Q_PROPERTY(QString slicerHome READ slicerHome CONSTANT)
  Q_PROPERTY(QString defaultScenePath READ defaultScenePath WRITE setDefaultScenePath)
  Q_PROPERTY(QString slicerSharePath READ slicerSharePath CONSTANT)
  Q_PROPERTY(QString temporaryPath READ temporaryPath WRITE setTemporaryPath)
  Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath)
  Q_PROPERTY(QString startupWorkingPath READ startupWorkingPath CONSTANT)
  Q_PROPERTY(QString launcherExecutableFilePath READ launcherExecutableFilePath CONSTANT)
  Q_PROPERTY(QString launcherSettingsFilePath READ launcherSettingsFilePath CONSTANT)
  Q_PROPERTY(QString slicerDefaultSettingsFilePath READ slicerDefaultSettingsFilePath CONSTANT)
  Q_PROPERTY(QString slicerUserSettingsFilePath READ slicerUserSettingsFilePath CONSTANT)
  Q_PROPERTY(QString slicerRevisionUserSettingsFilePath READ slicerRevisionUserSettingsFilePath CONSTANT)
  Q_PROPERTY(QString extensionsInstallPath READ extensionsInstallPath WRITE setExtensionsInstallPath)
  Q_PROPERTY(QString intDir READ intDir CONSTANT)
  Q_PROPERTY(bool isInstalled READ isInstalled CONSTANT)
  Q_PROPERTY(QString releaseType READ releaseType CONSTANT)
  Q_PROPERTY(QString repositoryUrl READ repositoryUrl CONSTANT)
  Q_PROPERTY(QString repositoryBranch READ repositoryBranch CONSTANT)
  Q_PROPERTY(QString repositoryRevision READ repositoryRevision CONSTANT)
  Q_PROPERTY(QString revision READ revision CONSTANT)
  Q_PROPERTY(int majorVersion READ majorVersion CONSTANT)
  Q_PROPERTY(int minorVersion READ minorVersion CONSTANT)
  Q_PROPERTY(QString documentationBaseUrl READ documentationBaseUrl)
  Q_PROPERTY(QString documentationVersion READ documentationVersion CONSTANT)
  Q_PROPERTY(QString documentationLanguage READ documentationLanguage)
  Q_PROPERTY(QString platform READ platform CONSTANT)
  Q_PROPERTY(QString arch READ arch CONSTANT)
  Q_PROPERTY(QString os READ os CONSTANT)
  Q_PROPERTY(bool isCustomMainApplication READ isCustomMainApplication CONSTANT)
  Q_PROPERTY(QString mainApplicationName READ mainApplicationName CONSTANT)
  Q_PROPERTY(QString mainApplicationRepositoryUrl READ mainApplicationRepositoryUrl CONSTANT)
  Q_PROPERTY(QString mainApplicationRepositoryRevision READ mainApplicationRepositoryRevision CONSTANT)
  Q_PROPERTY(QString mainApplicationRevision READ mainApplicationRevision CONSTANT)
  Q_PROPERTY(int mainApplicationMajorVersion READ mainApplicationMajorVersion CONSTANT)
  Q_PROPERTY(int mainApplicationMinorVersion READ mainApplicationMinorVersion CONSTANT)
  Q_PROPERTY(int mainApplicationPatchVersion READ mainApplicationPatchVersion CONSTANT)

public:

  typedef QApplication Superclass;
  qSlicerCoreApplication(int &argc, char **argv);
  ~qSlicerCoreApplication() override;

  /// Return a reference to the application singleton.
  /// It returns nullptr if the current application is not based on qSlicerCoreApplication class
  /// (for example, in Qt Designer executable loads widget plugins).
  static qSlicerCoreApplication* application();

  /// Used in addition to existing QCoreApplication attribute.
  /// \sa Qt::ApplicationAttribute
  enum ApplicationAttribute
    {
    AA_DisablePython = 1000,
    AA_EnableTesting
    };

  /// \sa QCoreApplication::setAttribute
  static void setAttribute(qSlicerCoreApplication::ApplicationAttribute attribute, bool on = true);

  /// \sa QCoreApplication::testAttribute
  static bool testAttribute(qSlicerCoreApplication::ApplicationAttribute attribute);

  /// \brief Returns the environment without the Slicer specific values.
  ///
  /// Path environment variables like `PATH`, `LD_LIBRARY_PATH` or `PYTHONPATH`
  /// will not contain values found in the launcher settings.
  ///
  /// Similarly `key=value` environment variables also found in the launcher
  /// settings are excluded. Note that if a value was associated with a key prior
  /// starting Slicer, it will not be set in the environment returned by this
  /// function.
  ///
  /// The function excludes both the Slicer launcher settings and the revision
  /// specific launcher settings.
  ///
  /// \sa launcherSettingsFilePath(), launcherRevisionSpecificUserSettingsFilePath()
  /// \sa repositoryRevision()
  /// \sa environment()
  Q_INVOKABLE QProcessEnvironment startupEnvironment() const;

  /// Current working directory at the time the application was started.
  QString startupWorkingPath() const;

  /// \brief Returns the current environment.
  ///
  /// The returned environment contains all values found in the launcher
  /// settings.
  ///
  /// \note Environment variables set from python updating `os.environ` or
  /// set from c++ directly calling `putenv()` will **NOT** be found in the
  /// environment returned by this function.
  ///
  /// \sa setEnvironmentVariable(const QString& key, const QString& value);
  Q_INVOKABLE QProcessEnvironment environment() const;

  /// \brief Convenient function to set an environment variable.
  ///
  /// \note Using this function will ensure that the environment is up-to-date for
  /// processes started using QProcess or other alternative methods.
  Q_INVOKABLE void setEnvironmentVariable(const QString& key, const QString& value);

  /// Returns True if environment variable identified by \a key is set to \a value
  Q_INVOKABLE bool isEnvironmentVariableValueSet(const QString& key, const QString& value);

  /// Convenient function allowing to prepend \a value to environment variable identified by
  /// by \a key using \a separator
  Q_INVOKABLE void prependEnvironmentVariable(const QString& key, const QString& value, QChar separator = ';');

  /// Convenient function allowing to append \a value to environment variable identified by
  /// by \a key using \a separator
  Q_INVOKABLE void appendEnvironmentVariable(const QString& key, const QString& value, QChar separator = ';');

  /// Parse arguments
  /// \note If exitWhenDone is True, it's your responsibility to exit the application
  void parseArguments(bool& exitWhenDone);

  enum ReturnCode{
    ExitNotRequested = -1,
    ExitSuccess = EXIT_SUCCESS,
    ExitFailure = EXIT_FAILURE
  };
  /// Return exit code that may be set before the main event loop started or after
  /// it exited.
  ///
  /// After parsing arguments and before starting the event loop using exec(),
  /// returnCode is set if early exit was requested or if there was a parsing error.
  ///
  /// After exiting the event loop, returnCode is set if there was an error
  /// during cleanup performed in onAboutToQuit().
  ///
  /// Returns -1 if the application has not been asked to exit.
  /// EXIT_SUCCESS (0) if the application must return in success.
  /// EXIT_FAILURE (1) if the application failed.
  ///
  /// \sa exec()
  int returnCode()const;

  /// Enters the main event loop and waits until exit(), quit() or terminate() is called.
  ///
  /// To ensure that python exceptions occurring during the module unloading performed
  /// in onAboutToQuit() are considered, it is important to start the event loop directly
  /// calling this function.
  ///
  /// Note that the override of return code if an exception is raised during module
  /// cleanup happens only if testing mode is enabled.
  ///
  /// \sa QApplication::exec(), returnCode()
  /// \sa qSlicerCoreCommandOptions::isTestingEnabled()
  static int exec();

  /// Get MRML Scene
  Q_INVOKABLE vtkMRMLScene* mrmlScene() const;

  /// Get application logic
  Q_INVOKABLE vtkSlicerApplicationLogic* applicationLogic() const;

  // Convenience method for getting a module logic from the application logic.
  Q_INVOKABLE vtkMRMLAbstractLogic* moduleLogic(const QString& moduleName)const;

  /// Get slicer home directory
  /// \sa slicerHome
  QString slicerHome() const;

  /// Get default scene directory
  ///
  /// This returns the full path where scenes are saved to by default
  ///
  QString defaultScenePath() const;

  /// Set default slicer scene directory
  void setDefaultScenePath(const QString& path);

  /// Get slicer share directory
  ///
  /// This returns the partial path where slicer resources are located, which
  /// is normally of the form <code>"share/Slicer-<i>version</i>"</code>.
  ///
  /// \sa slicerSharePath, slicerHome()
  QString slicerSharePath() const;

  /// Returns True if module identified by \a moduleFileName is a descendant of slicer home.
  /// \sa slicerHome()
  bool isEmbeddedModule(const QString& moduleFileName)const;

  /// Get slicer default temporary directory
  QString defaultTemporaryPath() const;

  /// Get slicer temporary directory
  QString temporaryPath() const;

  /// Set slicer temporary directory
  void setTemporaryPath(const QString& path);

  /// Get default cache directory.
  /// \sa cachePath()
  QString defaultCachePath() const;

  /// Get cache directory.
  /// It is a temporary folder that contains files that can be useful to be kept between application sessions.
  /// For example, files downloaded from network, which may not need to be downloaded again if stored locally.
  QString cachePath() const;

  /// Set cache directory.
  /// \sa cachePath()
  void setCachePath(const QString& path);

  /// If any, return slicer launcher executable file path.
  QString launcherExecutableFilePath()const;

  /// If any, return slicer launcher settings file path.
  QString launcherSettingsFilePath()const;

  /// If any, return slicer user settings file path specific to a given revision of Slicer.
  QString launcherRevisionSpecificUserSettingsFilePath()const;

  /// If any, return slicer default settings file path.
  /// \sa defaultSettings()
  QString slicerDefaultSettingsFilePath()const;

  /// Return slicer user settings file path.
  /// \sa userSettings()
  QString slicerUserSettingsFilePath()const;

  /// Return slicer settings file path specific to a given revision of Slicer.
  /// \sa revisionUserSettings()
  QString slicerRevisionUserSettingsFilePath()const;

  /// Get slicer default extensions path
  QString defaultExtensionsInstallPath() const;

  /// Get slicer extension directory
  QString extensionsInstallPath() const;

  /// Set slicer extension directory
  void setExtensionsInstallPath(const QString& path);

  void gatherExtensionsHistoryInformationOnStartup();

  /// If any, this method return the build intermediate directory
  /// See $(IntDir) on http://msdn.microsoft.com/en-us/library/c02as0cs%28VS.71%29.aspx
  QString intDir()const;

  /// Return true is this instance of Slicer is running from an installed directory
  bool isInstalled()const;

  /// \brief Return the release type of this instance of Slicer.
  ///
  /// Release type can be `Experimental`, `Nightly` or `Stable`.
  QString releaseType()const;

  /// Associate a module with a node type.
  /// It is currently only used for determining which module can edit a specific node.
  /// If multiple modules are registered for the same class then the node widget's
  /// nodeEditable method is used for determining which module is the most suitable for editing.
  Q_INVOKABLE void addModuleAssociatedNodeType(const QString& nodeClassName, const QString& moduleName);

  /// Remove association between a module and a node type.
  Q_INVOKABLE void removeModuleAssociatedNodeType(const QString& nodeClassName, const QString& moduleName);

  /// List of all modules that are associated with the specified node type.
  Q_INVOKABLE QStringList modulesAssociatedWithNodeType(const QString& nodeClassName) const;

  /// List of all node types that are associated with any module.
  Q_INVOKABLE QStringList allModuleAssociatedNodeTypes() const;

#ifdef Slicer_USE_PYTHONQT
  /// Get python manager
  qSlicerCorePythonManager* corePythonManager()const;

  /// Set the IO manager
  /// \note qSlicerCoreApplication takes ownership of the object
  void setCorePythonManager(qSlicerCorePythonManager* pythonManager);

  /// Get python console
  ctkPythonConsole* pythonConsole()const;

  /// Set the python console
  /// \note qSlicerCoreApplication will not take ownership of the object,
  /// because it will be owned by the widget that it is part of
  /// (either it is part of the main window or a top-level window).
  void setPythonConsole(ctkPythonConsole* pythonConsole);

#endif

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  /// Get extensions manager model
  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;

  /// Set the extensions manager model
  /// \note qSlicerCoreApplication takes ownership of the object
  void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);
#endif

  /// Get errorLogModel
  Q_INVOKABLE ctkErrorLogAbstractModel* errorLogModel()const;

  /// Get the module manager
  Q_INVOKABLE qSlicerModuleManager* moduleManager()const;

  /// Get the IO manager
  Q_INVOKABLE qSlicerCoreIOManager* coreIOManager()const;

  /// Set the IO manager
  /// \note qSlicerCoreApplication takes ownership of the object
  void setCoreIOManager(qSlicerCoreIOManager* ioManager);

  /// Get coreCommandOptions
  qSlicerCoreCommandOptions* coreCommandOptions()const;

  /// Set coreCommandOptions
  /// \note qSlicerCoreApplication takes ownership of the object
  void setCoreCommandOptions(qSlicerCoreCommandOptions* options);

  /// Get slicer application default settings.
  /// \sa slicerDefaultSettingsFilePath()
  Q_INVOKABLE QSettings* defaultSettings()const;

  /// Get slicer application user settings
  /// \note It will also instantiate a QSettings object if required.
  /// \sa slicerUserSettingsFilePath()
  Q_INVOKABLE QSettings* userSettings()const;

  /// This function has been deprecated: userSettings() should be used.
  /// \deprecated
  /// \sa userSettings()
  Q_INVOKABLE QSettings* settings()const;

  /// Get revision specific slicer application user settings
  /// \note It will also instantiate a QSettings object if required.
  /// \sa slicerRevisionSpecificUserSettingsFilePath()
  Q_INVOKABLE QSettings* revisionUserSettings()const;

  /// \brief Return if main application is custom application (not Slicer).
  bool isCustomMainApplication()const;

  /// \brief Return the name of the main application.
  QString mainApplicationName()const;

  /// Return the main application's source repository URL associated with this build.
  /// Useful for custom applications.
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString mainApplicationRepositoryUrl()const;

  /// Return the main application's source repository Revision associated with this build.
  /// Useful for custom applications.
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString mainApplicationRepositoryRevision()const;

  /// Return the main application's user-friendly revision identifier.
  /// Useful for custom applications.
  QString mainApplicationRevision()const;

  /// Return the main application's major version number.
  /// Useful for custom applications.
  int mainApplicationMajorVersion() const;

  /// Return the main application's minor version number.
  /// Useful for custom applications.
  int mainApplicationMinorVersion() const;

  /// Return the main application's patch version number.
  /// Useful for custom applications.
  int mainApplicationPatchVersion() const;

  /// Return the documentation base URL.
  /// By default, https://slicer.readthedocs.io/{language}/{version}
  /// but it can be changed in the application settings (DocumentationBaseURL).
  /// Use "latest" version for Preview (installed preview release) and Experimental (developer build),
  /// and use "majorVersion.minorVersion" for Stable release.
  QString documentationBaseUrl()const;

  /// Return the documentation version that can be used in URLs.
  /// Returns "latest" version for Preview (installed preview release) and Experimental (developer build),
  /// and use "majorVersion.minorVersion" for Stable release.
  QString documentationVersion()const;

  /// Return the documentation language that can be used in URLs.
  /// Returns "en" if internationalization is disabled.
  QString documentationLanguage()const;

  /// Return the documentation base URL.
  /// By default, {documentationbaseurl}/user_guide/modules/{lowercasemodulename}.html
  /// but it can be changed in the application settings (ModuleDocumentationURL).
  Q_INVOKABLE QString moduleDocumentationUrl(const QString &moduleName)const;

  /// Return the copyrights of Slicer
  virtual QString copyrights()const;

  /// Return the acknowledgment text of Slicer
  virtual QString acknowledgment()const;

  /// Return the libraries of Slicer
  virtual QString libraries()const;

  /// Return the Slicer source repository URL associated with this build
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString repositoryUrl()const;

  /// Return the Slicer source repository Branch associated with this build
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString repositoryBranch()const;

  /// Return the Slicer source repository Revision associated with this build
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString repositoryRevision()const;

  /// Return Slicer's user-friendly revision identifier.
  QString revision()const;

  /// Return the Slicer major version number
  int majorVersion() const;

  /// Return the Slicer minor version number
  int minorVersion() const;

  /// Return the \a platform associated to this build
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString platform()const;

  /// Return the \a arch associated to this build
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString arch()const;

  /// Return the \a os associated to this build
  /// \sa qSlicerCoreApplicationPrivate::discoverRepository
  QString os()const;

#ifdef Slicer_BUILD_DICOM_SUPPORT
  /// Return the application's main DICOM database.
  Q_INVOKABLE ctkDICOMDatabase* dicomDatabase() const;

  /// Return the application's main DICOM database as a shared pointer
  /// (not Python-wrappable).
  QSharedPointer<ctkDICOMDatabase> dicomDatabaseShared() const;
#endif

  /// Return list of folders where the application looks for translations (*.qm files)
  Q_INVOKABLE static QStringList translationFolders();

  /// Load translations from all *.qm files in the specified folders.
  /// \sa loadLanguage()
  Q_INVOKABLE static void loadTranslations(const QString& dir);

  /// Load translations from all *.qm files in translation folders.
  /// \sa translationFolders(), loadTranslations
  Q_INVOKABLE static void loadLanguage();

  /// Load certificates bundled into '<slicerHome>/<SLICER_SHARE_DIR>/Slicer.crt'.
  /// For more details, see Slicer/Base/QTCore/Resources/Certs/README
  /// Returns \a False if 'Slicer.crt' failed to be loaded.
  /// \sa QSslSocket::defaultCaCertificates()
  Q_INVOKABLE static bool loadCaCertificates(const QString& slicerHome);

  Q_INVOKABLE int registerResource(const QByteArray& data);

  /// Print message on console.
  /// If error is true then the message is printed on stderr, otherwise on stdout.
  Q_INVOKABLE void showConsoleMessage(QString message, bool error=true) const;

  /// Converts relative path to absolute path using slicerHome directory.
  /// Returns absolute path unchanged.
  Q_INVOKABLE QString toSlicerHomeAbsolutePath(const QString& path) const;

  /// Converts paths within slicerHome directory to relative paths.
  /// Leaves other paths unchanged.
  Q_INVOKABLE QString toSlicerHomeRelativePath(const QString& path) const;

  /// Converts relative path to absolute path using slicerHome directory.
  /// Returns absolute path unchanged.
  Q_INVOKABLE QStringList toSlicerHomeAbsolutePaths(const QStringList& path) const;

  /// Converts paths within slicerHome directory to relative paths.
  /// Leaves other paths unchanged.
  Q_INVOKABLE QStringList toSlicerHomeRelativePaths(const QStringList& path) const;

public slots:

  /// Restart the application with the arguments passed at startup time
  /// \sa QCoreApplication::arguments()
  static void restart();

  bool unregisterResource(int handle);

  /// Calls setRenderPaused(pause) on the current layout manager.
  /// Emits pauseRenderRequested() if pause is true and resumeRenderRequested() if pause is false.
  /// The caller is responsible for making sure that each setRenderPaused(true) is paired with
  /// setRenderPaused(false).
  /// Implemented in qSlicerApplication
  /// \sa qSlicerApplication::setRenderPaused()
  virtual void setRenderPaused(bool pause) { Q_UNUSED(pause); };
  /// Equivalent to setRenderPaused(true)
  /// \sa setRenderPaused
  virtual void pauseRender() {};
  /// Equivalent to setRenderPaused(false)
  /// \sa setRenderPaused
  virtual void resumeRender() {};

  /// Load files into the application.
  /// \param userMessages if specified then loading errors are returned via this object.
  /// \return Returns true on success.
  virtual bool loadFiles(const QStringList& filePaths, vtkMRMLMessageCollection* userMessages=nullptr);

  /// Open URL in the the application.
  /// Emits urlReceived signal that modules (such as DICOM module) can handle.
  /// \param url URL string to open
  virtual void openUrl(const QString& url);

protected:

  /// Process command line arguments **before** the application event loop is started.
  /// \sa handleCommandLineArguments()
  /// \sa qSlicerApplication::startupCompleted()
  virtual void handlePreApplicationCommandLineArguments();

  /// Set MRML Scene
  /// \sa vtkSlicerApplicationLogic::SetMRMLSceneDataIO
  virtual void setMRMLScene(vtkMRMLScene * scene);

protected slots:

  /// Process command line arguments **atfer** the application event loop is started.
  /// \sa handlePreApplicationCommandLineArguments()
  /// \sa qSlicerApplication::startupCompleted()
  virtual void handleCommandLineArguments();

  virtual void onSlicerApplicationLogicModified();
  virtual void onUserInformationModified();
  void onSlicerApplicationLogicRequest(vtkObject*, void* , unsigned long);
  void processAppLogicModified();
  void processAppLogicReadData();
  void processAppLogicWriteData();

  /// Editing of a MRML node has been requested.
  /// Implemented in qSlicerApplication.
  virtual void editNode(vtkObject*, void*, unsigned long) {};

  /// Set the ReturnCode flag and call QCoreApplication::exit()
  void terminate(int exitCode = qSlicerCoreApplication::ExitSuccess);

  /// Perform application cleanup following a call to QCoreApplication::exit().
  virtual void onAboutToQuit();

  /// Called when the application logic requests a delayed event invocation.
  /// When the singleton application logic fires the RequestInvokeEvent,
  /// \sa invokeEvent(), vtkMRMLApplicationLogic::InvokeRequest
  /// \sa onSlicerApplicationLogicRequest(), processAppLogicModified()
  void requestInvokeEvent(vtkObject* caller, void* callData);

  /// a timer is created, and on timeout, \a invokeEvent() is called to
  /// propagate the requested event invocation.
  /// \sa invokeEventRequested(), requestInvokeEvent(), invokeEvent()
  void scheduleInvokeEvent(unsigned int delay, void* caller, unsigned long event, void* callData);

  /// Internal method called only when a special QTimer times out.
  /// The timer contains dynamic properties describing an event to invoke on
  /// a specific object.
  /// \sa requestInvokeEvent
  void invokeEvent();

signals:
  void mrmlSceneChanged(vtkMRMLScene* mrmlScene);

  /// Signal is emitted when a url argument is processed with the slicer:// protocol
  /// The url string is emitted with the signal.
  /// Modules can connect to this signal to handle url arguments.
  void urlReceived(QString url);

  /// Internal method used to move an invocation from a thread to the main thread.
  /// \sa requestInvokeEvent(), scheduleInvokeEvent()
  void invokeEventRequested(unsigned int delay, void* caller,
                            unsigned long event, void* callData);

protected:
  qSlicerCoreApplication(qSlicerCoreApplicationPrivate* pimpl, int &argc, char **argv);
  QScopedPointer<qSlicerCoreApplicationPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCoreApplication);
  Q_DISABLE_COPY(qSlicerCoreApplication);
};

Q_DECLARE_METATYPE(qSlicerCoreApplication::ReturnCode)

/// \brief Safe replacement of qSlicerCoreApplication::pauseRender/resumeRender.
///
/// SlicerRenderBlocker can be used wherever you would otherwise use
/// a pair of calls to app->pauseRender() and app->resumeRender().
/// It pauses rendering in its constructor and in the destructor it
/// restores previous rendering state.
///
class Q_SLICER_BASE_QTCORE_EXPORT SlicerRenderBlocker
{
public:
  qSlicerCoreApplication* Application;
  SlicerRenderBlocker()
  {
    this->Application = qSlicerCoreApplication::application();
    if (this->Application)
      {
      this->Application->pauseRender();
      }
  };
  ~SlicerRenderBlocker()
  {
    if (this->Application)
      {
      this->Application->resumeRender();
      }
  }
};

#endif
