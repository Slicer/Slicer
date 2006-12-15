// .NAME vtkSlicerApplication
// .SECTION Description
// Contains slicer's style, application and collection of associated guis.

#ifndef __vtkSlicerApplication_h
#define __vtkSlicerApplication_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWApplication.h"
#include "vtkKWRegistryHelper.h" // really could have been avoided :(
#include "itkMutexLock.h"

class vtkSlicerModuleGUI;
class vtkSlicerGUILayout;
class vtkSlicerTheme;
class vtkSlicerGUICollection;
//BTX
class DisplayMessageQueue;
//ETX

// Description:
// Contains slicer's style, application and collection of associated guis.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplication : public vtkKWApplication
{
 public:
    vtkTypeRevisionMacro ( vtkSlicerApplication, vtkKWApplication );

    // Description:
    // This is a singleton pattern New.  There will only be ONE
    // reference to a vtkSlicerApplication object per process.  Clients that
    // call this must call Delete on the object so that the reference
    // counting will work.   The single instance will be unreferenced when
    // the program exits.
    static vtkSlicerApplication* New();

    // Description:
    // Get the singleton
    static vtkSlicerApplication* GetInstance();

    vtkGetObjectMacro ( MainLayout, vtkSlicerGUILayout );
    vtkGetObjectMacro ( SlicerTheme, vtkSlicerTheme );
    vtkGetObjectMacro ( ModuleGUICollection, vtkSlicerGUICollection );

    // Description:
    // This method collects GUIs added to Slicer.
    virtual void AddModuleGUI ( vtkSlicerModuleGUI *gui );
    virtual void RemoveModuleGUI ( vtkSlicerModuleGUI *gui );
    virtual vtkSlicerModuleGUI* GetModuleGUIByName ( const char *name );

    // Description:
    // These methods manage windows associated with the application
    virtual void CloseAllWindows ( ) ;
    
    // Description:
    // Sets application behavior.
    virtual void ConfigureApplication ( );
    virtual int StartApplication ( );

    // Description:
    // installs rules to specify look & feel.
    virtual void InstallTheme ( vtkKWTheme *theme );

  // Description:
  // Save/Retrieve the application settings to/from registry.
  // Do not call that method before the application name is known and the
  // proper registry level set (if any).
  virtual void RestoreApplicationSettingsFromRegistry();
  virtual void SaveApplicationSettingsToRegistry();

  // Description:
  // Some constants
  //BTX
  static const char *ConfirmDeleteRegKey;
  static const char *ModulePathRegKey;
  static const char *TemporaryDirectoryRegKey;
  static const char *HomeModuleRegKey;
  //ETX

  // Descrition:
  // Set/Get the confirm delete flag
  void SetConfirmDelete(const char* state);
  const char* GetConfirmDelete() const;
  
  // Description:
  // Set/Get the search path for modules.
  void SetModulePath(const char *path);
  const char* GetModulePath() const;

  // Description:
  // Set/Get a user's home module.
  void SetHomeModule (const char *name);
  const char *GetHomeModule() const;
  
  // Description:
  // Set/Get a directory for temporary file storage
  void SetTemporaryDirectory(const char *path);
  const char* GetTemporaryDirectory() const;

  // Description:
  // Evaluate a string as a tcl expression
  const char *Evaluate(const char *expression);

  // Description:
  // Request that a message be placed in the logger
  // widget. RequestDisplayMessage() allows records to be added to the
  // logger widget from a separate thread. First argument is the type
  // of message ("Error", "Warning", "Information", "Debug").  Second
  // argument is the message to display.
  bool RequestDisplayMessage( const char *type, const char* message );
  
  // Description:
  // Process a request to place a message on the log widget. This
  // method is called in the main thread of the application because
  // calls to the log widget can cause an update to the GUI. (Method
  // needs to be public to fit in the event callback chain.)
  void ProcessDisplayMessage();

  // Description:
  // Put a message in the logger widget.  These methods actually
  // schedule the message display so that only the application
  // associated with the user interface attempts to display the
  // message. THese methods delegate to RequestDisplayMessage().
  virtual void WarningMessage(const char* message);
  virtual void ErrorMessage(const char* message);
  virtual void DebugMessage(const char* message);
  virtual void InformationMessage(const char* message);
  
  // Description:
  // Override the KWWidgets default behavior of setting the 'transient'
  // flag on the interactor, which prevents it from being hidden
  // by the main application window
  virtual void DisplayTclInteractor(vtkKWTopLevel *master);
  virtual void DisplayLogDialog(vtkKWTopLevel *master);

 protected:
  vtkSlicerApplication ( );
  virtual ~vtkSlicerApplication ( );
  
  vtkSlicerGUILayout *MainLayout;
  vtkSlicerTheme *SlicerTheme;
  
  // Description:
  // The main application GUI.
  // Description:
  // Collections of GUIs
  vtkSlicerGUICollection *ModuleGUICollection;
  
  char ConfirmDelete[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ModulePath[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char TemporaryDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char HomeModule [ vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  
private:
  vtkSlicerApplication ( const vtkSlicerApplication& ); // Not implemented.
  void operator = ( const vtkSlicerApplication& ); //Not implemented.
    
  //BTX
  itk::MutexLock::Pointer DisplayMessageQueueActiveLock;
  itk::MutexLock::Pointer DisplayMessageQueueLock;
  //ETX
  bool DisplayMessageQueueActive;

  DisplayMessageQueue* InternalDisplayMessageQueue;
  
  static vtkSlicerApplication* Instance;
}; 

#endif
