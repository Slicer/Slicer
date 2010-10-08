#ifndef __vtkSlicerApplication_h
#define __vtkSlicerApplication_h

#include "vtkSlicerConfigure.h"

#ifdef Slicer_USE_QT
// QT includes
#include <QStringList>
#endif

#include "vtkStringArray.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWApplication.h"
#include "vtkKWRegistryHelper.h" /// really could have been avoided :(
#include "itkMutexLock.h"

#include "vtkSlicerApplicationGUI.h"

#ifdef Slicer_USE_QT
//BTX
class qSlicerApplication;
class qSlicerModulePanel;
//ETX
#endif

class vtkSlicerModuleGUI;
class vtkSlicerGUILayout;
class vtkSlicerTheme;
class vtkSlicerGUICollection;
//BTX
class DisplayMessageQueue;
//ETX


/// Description:
/// Contains slicer's style, application and collection of associated guis.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplication : public vtkKWApplication
{
 public:
    vtkTypeRevisionMacro ( vtkSlicerApplication, vtkKWApplication );

    /// 
    /// This is a singleton pattern New.  There will only be ONE
    /// reference to a vtkSlicerApplication object per process.  Clients that
    /// call this must call Delete on the object so that the reference
    /// counting will work.   The single instance will be unreferenced when
    /// the program exits.
    static vtkSlicerApplication* New();

    /// 
    /// Get the singleton
    static vtkSlicerApplication* GetInstance();

    /// 
    /// Do one tcl event and enter the event loop, allowing the application
    /// interface to actually run.
    /// - override the virtual method from vtkKWApplication to add event broker
    virtual void DoOneTclEvent();

    /// 
    /// Get the layout, theme, GUI collection and main application GUI
    vtkGetObjectMacro ( DefaultGeometry, vtkSlicerGUILayout );
    vtkGetObjectMacro ( SlicerTheme, vtkSlicerTheme );
    vtkGetObjectMacro ( ModuleGUICollection, vtkSlicerGUICollection );
    vtkGetObjectMacro ( ApplicationGUI, vtkSlicerApplicationGUI );
    vtkSetObjectMacro ( ApplicationGUI, vtkSlicerApplicationGUI );

    /// 
    /// This method collects GUIs added to Slicer.
    virtual void AddModuleGUI ( vtkSlicerModuleGUI *gui );
    virtual void RemoveModuleGUI ( vtkSlicerModuleGUI *gui );
    virtual vtkSlicerModuleGUI* GetModuleGUIByName ( const char *name );

    /// 
    /// Set/Get MRML scene
    void SetMRMLScene( vtkMRMLScene* scene);
    vtkMRMLScene* GetMRMLScene();

#ifdef Slicer_USE_QT
    /// 
    /// Initialize qt core Modules
    //BTX
    void InitializeQtCoreModules();
    //ETX

    /// 
    /// Initialize qt loadable Modules
    //BTX
    void InitializeQtLoadableModules();
    //ETX

    /// 
    /// Initialize qt command line Modules
    //BTX
    void InitializeQtCommandLineModules();
    //ETX

    /// 
    /// Initialize a list of QtModules given a list of names
    //BTX
    void InitializeQtModules(const QStringList& names);
    //ETX

    /// 
    /// Initialize a qt Module given its name
    //BTX
    void InitializeQtModule(const QString& moduleName);
    //ETX

    /// 
    /// Return the module panel
    //BTX
    void SetModulePanel(qSlicerModulePanel*);
    qSlicerModulePanel* modulePanel();
    //ETX
#endif

    /// 
    /// Overloaded method from vtkKWApplication allowing to show/hide Qt modules
    /// when a Kw dialog is popup
    //BTX
    virtual void RegisterDialogUp(vtkKWWidget *ptr);
    virtual void UnRegisterDialogUp(vtkKWWidget *ptr);
    //ETX

    /// 
    /// These methods manage windows associated with the application
    virtual void CloseAllWindows ( ) ;

    /// 
    /// Sets application behavior.
    virtual void ConfigureApplication ( );
    virtual int StartApplication ( );

    /// 
    /// installs rules to specify look & feel.
    virtual void InstallTheme ( vtkKWTheme *theme );

  /// 
  /// methods to access registry from wrapped languages
  int HasRegistry(const char *key);
  void RequestRegistry(const char *key);
  const char *GetRegistryHolder();
  void SetRegistry(const char *key, char *value);

  /// 
  /// Save/Retrieve the application settings to/from registry.
  /// Do not call that method before the application name is known and the
  /// proper registry level set (if any).
  virtual void RestoreApplicationSettingsFromRegistry();
  virtual void SaveApplicationSettingsToRegistry();
  virtual void ConfigureRemoteIOSettingsFromRegistry();
  virtual void UpdateRemoteIOSettingsForRegistry();
  /// 
  /// Perform test after the Application Registry is read and values
  /// are set to determine whether File System is full -- if so, prompts
  /// users to remove files from Temporary Directory and from Cache
  /// Directory -- then does a check again and warns if the write to
  /// disk fails, recommends freeing up some disk space.
  virtual int FullFileSystemCheck ( );

  /// 
  /// Some constants
  //BTX
  static const char *ConfirmDeleteRegKey;
  static const char *ModulePathsRegKey;
  static const char *ColorFilePathsRegKey;
  static const char *PotentialModulePathsRegKey;
  static const char *PotentialColorFilePathsRegKey;
  static const char *ExtensionsInstallPathRegKey;
  static const char *TemporaryDirectoryRegKey;
  static const char *WebBrowserRegKey;
  static const char *UnzipRegKey;
  static const char *ZipRegKey;
  static const char *RmRegKey;
  static const char *HomeModuleRegKey;
  static const char *LoadModulesRegKey;
  static const char *IgnoreModulesRegKey;
  static const char *LoadCommandLineModulesRegKey;
  static const char *RedirectModuleStreamsRegKey;
  static const char *DeleteTemporaryFilesRegKey;
  static const char *EnableDaemonRegKey;
  static const char *ApplicationFontFamilyRegKey;
  static const char *ApplicationFontSizeRegKey;
  static const char *ApplicationWindowWidthRegKey;
  static const char *ApplicationWindowHeightRegKey;
  static const char *ApplicationSlicesFrameHeightRegKey;
  static const char *ApplicationLayoutTypeRegKey;
  static const char *ApplicationLayoutCompareViewRowsRegKey;
  static const char *ApplicationLayoutCompareViewColumnsRegKey;
  static const char *ApplicationLayoutLightboxRowsRegKey;
  static const char *ApplicationLayoutLightboxColumnsRegKey;
  static const char *EnableAsynchronousIORegKey;
  static const char *UseWelcomeModuleAtStartupRegKey;
  static const char *EnableForceRedownloadRegKey;
  static const char *EnableRemoteCacheOverwritingRegKey;
  static const char *RemoteCacheDirectoryRegKey;
  static const char *RemoteCacheLimitRegKey;
  static const char *RemoteCacheFreeBufferSizeRegKey;
  //ETX


  /// 
  /// Set/Get the application font family
  void SetApplicationFontFamily ( const char *family);
  const char *GetApplicationFontFamily ( ) const;

  /// 
  /// Set/Get the application font size
  void SetApplicationFontSize ( const char *size );
  const char *GetApplicationFontSize ( ) const;

  /// Descrition:
  /// Set/Get the confirm delete flag
  void SetConfirmDelete(const char* state);
  const char* GetConfirmDelete() const;

  /// 
  /// Set/Get the search paths for modules.
  /// This is a list of paths delimited by a specific separator: ';' on
  /// Windows, ':' on Unix/MacOSX platforms.
  void SetModulePaths(const char *paths);
  const char* GetModulePaths() const;


  /// 
  /// Set/Get the potential search paths for modules.
  /// This is a list of directories that can be used as module paths.
  /// Each item in this list is a directory and a boolean flag (0 or 1)
  /// specifying if that directory is actually to be used as a module path
  /// (see ModulePaths, which is the subset of the paths in PotentialModulePaths
  /// that are enabled, with a different delimiter between each path).
  /// This variable is used for GUI purposes, in that it lets people keep a
  /// list of directories and enable/disable them at will, without having
  /// to re-enter/re-pick them one by one using a file browser. It is used
  /// by the vtkSlicerApplicationSettingsInterface and computed with help its
  /// vtkKWDirectoryPresetSelector internal class. Each element is separated by
  /// a '|' delimiter (ex: "c:/temp|0|d:/foo/bar|1", where "c:/temp" is disabled
  /// and d:/foo/bar is enabled; at this point, the value of ModulePaths should
  /// actually be "d:/foo/bar").
  void SetPotentialModulePaths(const char *paths);
  const char* GetPotentialModulePaths() const;

  /// 
  /// Helper method to append a single path the existing potential
  /// modules paths.
  void AppendPotentialModulePath(const char *path, bool enabled);

  /// 
  /// Set/Get the potential search paths for color files
  /// based on SetPotentialModulePaths
  void SetColorFilePaths(const char *paths);
  const char *GetColorFilePaths() const;
  void SetPotentialColorFilePaths(const char *paths);
  const char* GetPotentialColorFilePaths() const;

  /// 
  /// Set/Get the cache path for modules.
  void SetExtensionsInstallPath(const char *path);
  const char* GetExtensionsInstallPath();

  /// 
  /// Set/Get a user's home module.
  void SetHomeModule (const char *name);
  const char *GetHomeModule() const;

  /// 
  /// Set/Get an executable firefox browser for modules that need one.
  void SetWebBrowser ( const char *browser);
  const char* GetWebBrowser () const;

  /// 
  /// Set/Get an executable zip and unzip for modules that need one
  void SetUnzip ( const char *unzip );
  const char *GetUnzip() const;
  void SetZip (const char *zip);
  const char *GetZip() const;

  /// 
  /// Set/Get an executable rm for modules that need one
  void SetRm ( const char *rm );
  const char *GetRm() const;

  /// 
  /// Set/Get a directory for temporary file storage
  void SetTemporaryDirectory(const char *path);
  const char* GetTemporaryDirectory() const;

  /// 
  /// Set/Get a directory for the remote file cache
  void SetRemoteCacheDirectory(const char *path);
  const char* GetRemoteCacheDirectory() const;

  /// 
  /// Set/Get the binary location
  void SetBinDir(const char* path);
  const char* GetBinDir() const;

  /// 
  /// Set/Get the application window size
  /// for saving in the registry.
  void SetApplicationWindowSize (int width, int height );
  vtkGetMacro (ApplicationWindowWidth, int);
  vtkSetMacro (ApplicationWindowWidth, int);
  vtkGetMacro (ApplicationWindowHeight, int);
  vtkSetMacro (ApplicationWindowHeight, int);

  /// 
  /// Saves the application window size for registry setting
  void SaveApplicationWindowConfiguration ( );

  /// 
  /// Set/Get the application layout for saving in
  /// the registry.
  vtkGetMacro (ApplicationSlicesFrameHeight, int );
  vtkSetMacro (ApplicationSlicesFrameHeight, int );
  vtkGetMacro (ApplicationLayoutType, int );
  vtkSetMacro (ApplicationLayoutType, int );
  vtkGetMacro (ApplicationLayoutCompareViewRows, int );
  vtkSetMacro (ApplicationLayoutCompareViewRows, int );
  vtkGetMacro (ApplicationLayoutCompareViewColumns, int );
  vtkSetMacro (ApplicationLayoutCompareViewColumns, int );
  vtkGetMacro (ApplicationLayoutLightboxRows, int );
  vtkSetMacro (ApplicationLayoutLightboxRows, int );
  vtkGetMacro (ApplicationLayoutLightboxColumns, int );
  vtkSetMacro (ApplicationLayoutLightboxColumns, int );

  /// 
  /// Set/Get if modules should be loaded (i.e. loadable modules)
  vtkSetMacro(LoadModules, int);
  vtkGetMacro(LoadModules, int);
  vtkBooleanMacro(LoadModules, int);

  /// 
  /// Set/Get names of modules that should be ignored (of all loadable modules)
  vtkSetObjectMacro(IgnoreModules, vtkStringArray);
  vtkGetObjectMacro(IgnoreModules, vtkStringArray);

  /// 
  /// Set/Get names of all loadable modules
  vtkSetObjectMacro(LoadableModules, vtkStringArray);
  vtkGetObjectMacro(LoadableModules, vtkStringArray);


  /// 
  /// Set/Get if command line modules should be loaded (i.e. CLI plugins)
  vtkSetMacro(LoadCommandLineModules, int);
  vtkGetMacro(LoadCommandLineModules, int);
  vtkBooleanMacro(LoadCommandLineModules, int);

  /// 
  /// Set/Get if command line modules should be loaded (i.e. CLI plugins)
  void SetRedirectModuleStreams(int);
  vtkGetMacro(RedirectModuleStreams, int);
  vtkBooleanMacro(RedirectModuleStreams, int);

  /// 
  /// Set/Get if command line modules should be loaded (i.e. CLI plugins)
  void SetDeleteTemporaryFiles(int);
  vtkGetMacro(DeleteTemporaryFiles, int);
  vtkBooleanMacro(DeleteTemporaryFiles, int);

  /// 
  /// Set/Get if the slicer daemon should be loaded
  vtkSetMacro(EnableDaemon, int);
  vtkGetMacro(EnableDaemon, int);
  vtkBooleanMacro(EnableDaemon, int);

  /// 
  /// Set/Get if asynch IO should be used
  void SetEnableAsynchronousIO ( int );
  vtkGetMacro(EnableAsynchronousIO, int);
  vtkBooleanMacro(EnableAsynchronousIO, int);

  /// 
  /// Control whether the welcome module is shown at startup.
  void SetUseWelcomeModuleAtStartup (int );
  vtkGetMacro (UseWelcomeModuleAtStartup, int );
  vtkBooleanMacro (UseWelcomeModuleAtStartup, int );

  /// 
  /// Set/Get if re-downloads to the cache should be forced
  void SetEnableForceRedownload (int);
  vtkGetMacro(EnableForceRedownload, int);
  vtkBooleanMacro(EnableForceRedownload, int);

  /// 
  /// Set/Get if should force overwriting cache files
  void SetEnableRemoteCacheOverwriting (int);
  vtkGetMacro(EnableRemoteCacheOverwriting, int);
  vtkBooleanMacro(EnableRemoteCacheOverwriting, int);

  /// 
  /// Control the remote cache directory size, in Mb
  void SetRemoteCacheLimit ( int);
  vtkGetMacro (RemoteCacheLimit, int);

  /// 
  /// Control the remote cache directory free buffer size, in Mb
  void SetRemoteCacheFreeBufferSize ( int );
  vtkGetMacro (RemoteCacheFreeBufferSize, int);

  /// 
  /// Evaluate a string as a tcl expression
  const char *Evaluate(const char *expression);

  /// 
  /// Request that a message be placed in the logger
  /// widget. RequestDisplayMessage() allows records to be added to the
  /// logger widget from a separate thread. First argument is the type
  /// of message ("Error", "Warning", "Information", "Debug").  Second
  /// argument is the message to display.
  bool RequestDisplayMessage( const char *type, const char* message );

  /// 
  /// Process a request to place a message on the log widget. This
  /// method is called in the main thread of the application because
  /// calls to the log widget can cause an update to the GUI. (Method
  /// needs to be public to fit in the event callback chain.)
  void ProcessDisplayMessage();

  /// 
  /// Put a message in the logger widget.  These methods actually
  /// schedule the message display so that only the application
  /// associated with the user interface attempts to display the
  /// message. THese methods delegate to RequestDisplayMessage().
  virtual void WarningMessage(const char* message);
  virtual void ErrorMessage(const char* message);
  virtual void DebugMessage(const char* message);
  virtual void InformationMessage(const char* message);

  /// 
  /// Override the KWWidgets default behavior of setting the 'transient'
  /// flag on the interactor, which prevents it from being hidden
  /// by the main application window
  virtual void DisplayTclInteractor(vtkKWTopLevel *master);
  virtual void DisplayLogDialog(vtkKWTopLevel *master);

//BTX
  /// Pass/cast PyObject* in place of void*
  virtual void InitializePython(void* mod, void* dict);
  virtual void* GetPythonModule();
  virtual void* GetPythonDictionary();
//ETX

  /// 
  /// Add additional copyright messages
  virtual void AddAboutCopyrights(ostream &);

  /// 
  /// Control the state of the splash screen
  vtkSetMacro (UseSplashScreen, int);
  vtkGetMacro (UseSplashScreen, int);


  /// 
  /// Control stereo render capability
  vtkSetMacro (StereoEnabled, int);
  vtkGetMacro (StereoEnabled, int);

  /// 
  /// Control the state of the splash screen
  void SplashMessage (const char * message);

  /// 
  /// Entry point to the interpreter that can be used
  /// by an external application tro trigger
  /// performance analysis
  /// Also entry point for KWWidget callbacks that need to execute
  /// an arbitrary script with arguments
  const char *TraceScript (const char *script) {return (this->Script(script));};
  const char *Eval (const char *script) {return (this->Script(script));};
  const char *Eval (const char *script, const char *args) {return (this->Script("%s %s", script, args));};

  virtual vtkKWColorPickerDialog* GetColorPickerDialog();

  /// 
  /// Getter method to reference information about this build.
  const char* GetPlatform();
  const char* GetBuildDate();
  const char* GetSvnUrl();
  const char* GetSvnRevision();

///   void TestQtSlicerWebKit(const char *url);

 protected:
  vtkSlicerApplication ( );
  virtual ~vtkSlicerApplication ( );

  /// 
  /// Contains the default geometry of the ApplicationGUI
  vtkSlicerGUILayout *DefaultGeometry;
  /// 
  /// Contains the description for Slicer GUI's look and feel
  vtkSlicerTheme *SlicerTheme;
  /// 
  /// The main application GUI.
  vtkSlicerApplicationGUI *ApplicationGUI;
  /// 
  /// Collections of GUIs
  vtkSlicerGUICollection *ModuleGUICollection;

//BTX
  void StringToArray(std::string string, char separator, vtkStringArray *array);
  void ArrayToString(vtkStringArray *array, std::string sep, char *string, int maxLength );
//ETX

  char ConfirmDelete[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ModulePaths[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ColorFilePaths[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char PotentialModulePaths[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char PotentialColorFilePaths[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ExtensionsInstallPath[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ExtensionsInstallPathDefault[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char WebBrowser [vtkKWRegistryHelper::RegistryKeyValueSizeMax ];
  char Unzip [vtkKWRegistryHelper::RegistryKeyValueSizeMax ];
  char Zip [vtkKWRegistryHelper::RegistryKeyValueSizeMax ];
  char Rm [vtkKWRegistryHelper::RegistryKeyValueSizeMax ];
  char TemporaryDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char HomeModule [ vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ApplicationFontSize [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ApplicationFontFamily [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char IgnoreModuleNames [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char BinDir [vtkKWRegistryHelper::RegistryKeyValueSizeMax];

  char Platform [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char BuildDate [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char SvnUrl [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char SvnRevision [vtkKWRegistryHelper::RegistryKeyValueSizeMax];

  int ApplicationWindowWidth;
  int ApplicationWindowHeight;
  int ApplicationSlicesFrameHeight;
  int ApplicationLayoutType;
  int ApplicationLayoutCompareViewRows;
  int ApplicationLayoutCompareViewColumns;
  int ApplicationLayoutLightboxRows;
  int ApplicationLayoutLightboxColumns;

  char RegistryHolder [vtkKWRegistryHelper::RegistryKeyValueSizeMax];

  vtkStringArray *IgnoreModules;
  vtkStringArray *LoadableModules;

  int LoadModules;
  int LoadCommandLineModules;
  int RedirectModuleStreams;
  int DeleteTemporaryFiles;
  int EnableDaemon;
  void* PythonModule;
  void* PythonDictionary;

  int EnableAsynchronousIO;
  int EnableForceRedownload;
  int EnableRemoteCacheOverwriting;
  char RemoteCacheDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  int RemoteCacheLimit;
  int RemoteCacheFreeBufferSize;

  int UseWelcomeModuleAtStartup;

  /// 
  /// print out local vars
  void PrintSelf ( ostream& os, vtkIndent indent );

private:
  vtkSlicerApplication ( const vtkSlicerApplication& ); /// Not implemented.
  void operator = ( const vtkSlicerApplication& ); //Not implemented.

  //BTX
  /// 
  /// Helper method to setup Platform, Build Date, SVN URL and SVN Revision
  void InitializeSlicer3Version();
  //ETX

  //BTX
  itk::MutexLock::Pointer DisplayMessageQueueActiveLock;
  itk::MutexLock::Pointer DisplayMessageQueueLock;
  std::string NameSeparator;
  //ETX

  bool DisplayMessageQueueActive;

  DisplayMessageQueue* InternalDisplayMessageQueue;

  static vtkSlicerApplication* Instance;

  int UseSplashScreen;
  int StereoEnabled;

  /// have we added the mrml color table nodes to the color picker dialog yet?
  int ColorSwatchesAdded;

//BTX
  class vtkInternal;
  vtkInternal* Internal;
//ETX

};

#endif
