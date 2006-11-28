#include <sstream>
#include "vtkObjectFactory.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkKWNotebook.h"
#include "vtkKWFrame.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWindowBase.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWLogDialog.h"
#include "vtkKWLogWidget.h"
#include "vtkKWInternationalization.h"
#include "vtkKWTclInteractor.h"

#include "vtkOutputWindow.h"
#include "itkOutputWindow.h"

#ifdef WIN32
#include "vtkKWWin32RegistryHelper.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

#include "itksys/SystemTools.hxx"

#include <queue>

#include "vtkSlicerTask.h"

const char *vtkSlicerApplication::ModulePathRegKey = "ModulePath";
const char *vtkSlicerApplication::TemporaryDirectoryRegKey = "TemporaryDirectory";
const char *vtkSlicerApplication::ConfirmDeleteRegKey = "ConfirmDelete";

vtkSlicerApplication *vtkSlicerApplication::Instance = NULL;


//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerApplication, "$Revision: 1.0 $");

class ProcessingTaskQueue : public std::queue<vtkSmartPointer<vtkSlicerTask> > {};
class ModifiedQueue : public std::queue<vtkSmartPointer<vtkObject> > {};
typedef std::pair<std::string, std::string> AddRecordType;
class DisplayMessageQueue : public std::queue<AddRecordType> {};


//----------------------------------------------------------------------------


// Slicer needs its own version of itk::OutputWindow to ensure that
// only the application thread controlling the gui tries to display a
// message. 
namespace itk {

class SlicerOutputWindow : public OutputWindow
{
public:
  /** Standard class typedefs. */
  typedef SlicerOutputWindow        Self;
  typedef OutputWindow  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(SlicerOutputWindow,OutputWindow);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  void DisplayDebugText(const char* t)
    { 
      vtkSlicerApplication::GetInstance()->DebugMessage(t); 
    }
  void DisplayWarningText(const char* t)
    { 
      vtkSlicerApplication::GetInstance()->WarningMessage(t); 
    }
  void DisplayErrorText(const char* t)
    { 
      vtkSlicerApplication::GetInstance()->ErrorMessage(t); 
    }
  void DisplayText(const char* t)
    { 
      vtkSlicerApplication::GetInstance()->InformationMessage(t); 
    }
  void DisplayGenericWarningText(const char* t)
    { 
      this->DisplayWarningText(t); 
    }
  
protected:
  SlicerOutputWindow()
    { 
    }

private:
  SlicerOutputWindow(const SlicerOutputWindow&);
  void operator=(const SlicerOutputWindow&);
};

}; // end namespace itk



// Slicer needs its own version of vtkKWOutputWindow to ensure that
// only the application thread controlling the gui tries to display a
// message.
//
// NOTE: it looks as though as long as Slicer constructs the dialog
// window early enough, then we do not need our own version of the
// output window for VTK.  The virtual overrides of
// InformationMessage(), WarningMessage(), etc. in
// vtkSlicerApplication are enough to ensure that we display messages
// in a thread safe manner.
//
// 
// class vtkSlicerOutputWindow : public vtkOutputWindow
// {
// public:
//   vtkTypeMacro(vtkSlicerOutputWindow,vtkOutputWindow);
//   static vtkSlicerOutputWindow* New();
//
//   void DisplayDebugText(const char* t)
//     { 
//       this->Application->DebugMessage(t); 
//     }
//   void DisplayWarningText(const char* t)
//     { 
//       this->Application->WarningMessage(t); 
//     }
//   void DisplayErrorText(const char* t)
//     { 
//       this->Application->ErrorMessage(t); 
//     }
//   void DisplayText(const char* t)
//     { 
//       this->Application->InformationMessage(t); 
//     }
//   void DisplayGenericWarningText(const char* t)
//     { 
//       this->DisplayWarningText(t); 
//     }
//  
//   void SetApplication(vtkSlicerApplication *app)
//     { 
//       this->Application = app; 
//     }
//
// protected:
//   vtkSlicerOutputWindow()
//     { 
//       this->Application = NULL; 
//     }
//   vtkSlicerApplication *Application;
//
// private:
//   vtkSlicerOutputWindow(const vtkSlicerOutputWindow&);
//   void operator=(const vtkSlicerOutputWindow&);
// };
//
//
// vtkStandardNewMacro(vtkSlicerOutputWindow);


//---------------------------------------------------------------------------
vtkSlicerApplication::vtkSlicerApplication ( ) {

    strcpy(this->ModulePath, "");

    strcpy(this->ConfirmDelete, "");
    
    // configure the application before creating
    this->SetName ( "3D Slicer Version 3.0 Alpha" );

#ifdef _WIN32
    vtkKWWin32RegistryHelper *regHelper = 
        vtkKWWin32RegistryHelper::SafeDownCast( this->GetRegistryHelper() );
    regHelper->SetOrganization("NA-MIC");
#endif

    this->RestoreApplicationSettingsFromRegistry ( );
    this->SetHelpDialogStartingPage ( "http://www.slicer.org" );

    this->ModuleGUICollection = vtkSlicerGUICollection::New ( );
    vtkKWFrameWithLabel::SetDefaultLabelFontWeightToNormal( );
    this->MainLayout = vtkSlicerGUILayout::New ( );
    this->SlicerTheme = vtkSlicerTheme::New ( );

    this->ProcessingThreader = itk::MultiThreader::New();
    this->ProcessingThreadId = -1;
    this->ProcessingThreadActive = false;
    this->ProcessingThreadActiveLock = itk::MutexLock::New();
    this->ProcessingTaskQueueLock = itk::MutexLock::New();

    this->ModifiedQueueActive = false;
    this->ModifiedQueueActiveLock = itk::MutexLock::New();
    this->ModifiedQueueLock = itk::MutexLock::New();

    this->DisplayMessageQueueActive = false;
    this->DisplayMessageQueueActiveLock = itk::MutexLock::New();
    this->DisplayMessageQueueLock = itk::MutexLock::New();
    
    this->InternalTaskQueue = new ProcessingTaskQueue;
    this->InternalModifiedQueue = new ModifiedQueue;
    this->InternalDisplayMessageQueue = new DisplayMessageQueue;

    // Override the type of output windows used for VTK and ITK.  Note
    // that in the VTK case, we are currently bypassing the output
    // window mechanism provided by KWWidgets.  In KWWidgets, there
    // are calls to InstallOutputWindow()/RestoreOutputWindow() to
    // manage a KWWidget specific output window.  In the Slicer case,
    // we need a different type of output window to ensure that only
    // the main thread updates the gui.
    //
    //
    // NOTE: it looks as though as long as Slicer constructs the dialog
    // window early enough, then we do not need our own version of the
    // output window for VTK.  The virtual overrides of
    // InformationMessage(), WarningMessage(), etc. in
    // vtkSlicerApplication are enough to ensure that we display messages
    // in a thread safe manner.
    
    //vtkSlicerOutputWindow *vtkoutput = vtkSlicerOutputWindow::New();
    //vtkoutput->SetApplication(this);
    //vtkOutputWindow::SetInstance( vtkoutput );
    
    itk::SlicerOutputWindow::SetInstance( itk::SlicerOutputWindow::New() );

}



//---------------------------------------------------------------------------
vtkSlicerApplication::~vtkSlicerApplication ( ) {

    if ( this->MainLayout )
      {
      this->MainLayout->Delete ( );
      this->MainLayout = NULL;
      }
    if ( this->SlicerTheme )
      {
      this->SlicerTheme->Delete ( );
      this->SlicerTheme = NULL;
      }
    if ( this->ModuleGUICollection )
      {
      this->ModuleGUICollection->RemoveAllItems ( );
      this->ModuleGUICollection->Delete ( );
      this->ModuleGUICollection = NULL;
      }

    // Note that TerminateThread does not kill a thread, it only waits
    // for the thread to finish.  We need to signal the thread that we
    // want to terminate
    if (this->ProcessingThreadId != -1 && this->ProcessingThreader)
      {
      // Signal the processingThread that we are terminating. 
      this->ProcessingThreadActiveLock->Lock();
      this->ProcessingThreadActive = false;
      this->ProcessingThreadActiveLock->Unlock();
      
      // Wait for the thread to finish and clean up the state of the threader
      this->ProcessingThreader->TerminateThread( this->ProcessingThreadId );
      
      this->ProcessingThreadId = -1;
      }
    
    delete this->InternalTaskQueue;
    this->InternalTaskQueue = 0;
    
    delete this->InternalModifiedQueue;
    this->InternalModifiedQueue = 0;

    delete this->InternalDisplayMessageQueue;
    this->InternalDisplayMessageQueue = 0;
}


// Up the reference count so it behaves like New
vtkSlicerApplication* vtkSlicerApplication::New()
{
  vtkSlicerApplication* ret = vtkSlicerApplication::GetInstance();
  ret->Register(NULL);
  return ret;
}


// Return the single instance of the vtkSlicerApplication
vtkSlicerApplication* vtkSlicerApplication::GetInstance()
{
  if(!vtkSlicerApplication::Instance)
    {
    // Try the factory first
    vtkSlicerApplication::Instance = (vtkSlicerApplication*)
      vtkObjectFactory::CreateInstance("vtkSlicerApplication");
    // if the factory did not provide one, then create it here
    if(!vtkSlicerApplication::Instance)
      {
      vtkSlicerApplication::Instance = new vtkSlicerApplication;
      }
    }
  // return the instance
  return vtkSlicerApplication::Instance;
}



//---------------------------------------------------------------------------
const char *vtkSlicerApplication::Evaluate(const char *expression) {
    return (this->Script(expression));
}


//---------------------------------------------------------------------------
void vtkSlicerApplication::AddModuleGUI ( vtkSlicerModuleGUI *gui ) {

    // Create if it doesn't exist already
    if ( this->ModuleGUICollection == NULL ) {
        this->ModuleGUICollection = vtkSlicerGUICollection::New ( );
    } 
    // Add a gui
    this->ModuleGUICollection->AddItem ( gui );
}

//---------------------------------------------------------------------------
void vtkSlicerApplication::RemoveModuleGUI ( vtkSlicerModuleGUI *gui ) {

    // Create if it doesn't exist already
    if ( this->ModuleGUICollection == NULL ) {
        this->ModuleGUICollection = vtkSlicerGUICollection::New ( );
    } 
    // Remove a gui
    this->ModuleGUICollection->RemoveItem ( gui );
}

//---------------------------------------------------------------------------
vtkSlicerModuleGUI* vtkSlicerApplication::GetModuleGUIByName ( const char *name )
{
    if ( this->ModuleGUICollection != NULL ) {
        int n = this->ModuleGUICollection->GetNumberOfItems ( );
        int i;
        for (i = 0; i < n; i ++ ) {
            vtkSlicerModuleGUI *m = vtkSlicerModuleGUI::SafeDownCast(
              this->ModuleGUICollection->GetItemAsObject(i) );
            if ( !strcmp (m->GetGUIName(), name) ) {
                return (m);
            }
        }
    }
    return ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerApplication::ConfigureApplication ( ) {

    this->PromptBeforeExitOn ( );
    this->SupportSplashScreenOn ( );
    this->SplashScreenVisibilityOn ( );
    this->SaveUserInterfaceGeometryOn ( );
}



//---------------------------------------------------------------------------
void vtkSlicerApplication::InstallTheme ( vtkKWTheme *theme )
{
    if ( theme != NULL ) {
        if ( vtkSlicerTheme::SafeDownCast (theme) == this->SlicerTheme ) {
            this->SetTheme (this->SlicerTheme );
        } else {
            this->SetTheme ( theme );
        }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerApplication::CloseAllWindows ( ) {
    int n, i;
    vtkKWWindowBase *win;
    
    n= this->GetNumberOfWindows ( );
    for (i=0; i<n; i++) {
        win = this->GetNthWindow ( n );
        win->Close ( );
    }
}


//---------------------------------------------------------------------------
int vtkSlicerApplication::StartApplication ( ) {

    int ret = 0;

    // Start the application & event loop here
    this->Start ( );
    this->CloseAllWindows ( );

    // Clean up and exit
    ret = this->GetExitStatus ( );
    return ret;
}


//----------------------------------------------------------------------------
void vtkSlicerApplication::RestoreApplicationSettingsFromRegistry()
{
  // Make a good guess before we read from the registry.  Default to a
  // subdirectory called Slicer3 in a standard temp location.
#ifdef _WIN32
  GetTempPath(vtkKWRegistryHelper::RegistryKeyValueSizeMax,
              this->TemporaryDirectory);
#else
  strcpy(this->TemporaryDirectory, "/usr/tmp");
#endif

  // Tk does not understand Windows short path names, so convert to
  // long path names and unix slashes
  std::string temporaryDirectory = this->TemporaryDirectory;
  temporaryDirectory
    = itksys::SystemTools::GetActualCaseForPath(temporaryDirectory.c_str());
  itksys::SystemTools::ConvertToUnixSlashes( temporaryDirectory );
  
  std::vector<std::string> pathcomponents;
  std::string pathWithSlicer;
  itksys::SystemTools::SplitPath(temporaryDirectory.c_str(), pathcomponents);
  pathcomponents.push_back("Slicer3");
  pathWithSlicer = itksys::SystemTools::JoinPath(pathcomponents);
  
  itksys::SystemTools::MakeDirectory(pathWithSlicer.c_str());
  if (pathWithSlicer.size() < vtkKWRegistryHelper::RegistryKeyValueSizeMax)
    {
    strcpy(this->TemporaryDirectory, pathWithSlicer.c_str());
    }
  else
    {
    // path with "Slicer3" attached is too long. Try it without
    // "Slicer3". If still too long, use the original path. (This path
    // may have short names in it and hence will not work with Tk).
    if (temporaryDirectory.size()
        < vtkKWRegistryHelper::RegistryKeyValueSizeMax)
      {
      strcpy(this->TemporaryDirectory, temporaryDirectory.c_str());
      }
    vtkWarningMacro("Default temporary directory path " << pathWithSlicer.c_str()
                    << " is too long to be stored in the registry."
                    << " Using unmodified temporary directory path "
                    << this->TemporaryDirectory);
    }

    
  Superclass::RestoreApplicationSettingsFromRegistry();

  if (this->HasRegistryValue(
    2, "RunTime", vtkSlicerApplication::ConfirmDeleteRegKey))
    {
    this->GetRegistryValue(
      2, "RunTime", vtkSlicerApplication::ConfirmDeleteRegKey,
      this->ConfirmDelete);
    }
  
  if (this->HasRegistryValue(
    2, "RunTime", vtkSlicerApplication::ModulePathRegKey))
    {
    this->GetRegistryValue(
      2, "RunTime", vtkSlicerApplication::ModulePathRegKey,
      this->ModulePath);
    }

  if (this->HasRegistryValue(
    2, "RunTime", vtkSlicerApplication::TemporaryDirectoryRegKey))
    {
    this->GetRegistryValue(
      2, "RunTime", vtkSlicerApplication::TemporaryDirectoryRegKey,
      this->TemporaryDirectory);
    }

}

//----------------------------------------------------------------------------
void vtkSlicerApplication::SaveApplicationSettingsToRegistry()
{ 
  Superclass::SaveApplicationSettingsToRegistry();

  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::ConfirmDeleteRegKey, "%s", 
    this->ConfirmDelete);
  
  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::ModulePathRegKey, "%s", 
    this->ModulePath);

  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::TemporaryDirectoryRegKey, "%s", 
    this->TemporaryDirectory);
}

void vtkSlicerApplication::SetConfirmDelete(const char* state)
{
    if (state)
    {
        if (strcmp(this->ConfirmDelete, state) != 0
        && strlen(state) < vtkKWRegistryHelper::RegistryKeyValueSizeMax)
        {
            strcpy(this->ConfirmDelete, state);
            this->Modified();
        }
    }
}

const char *vtkSlicerApplication::GetConfirmDelete() const
{
    return this->ConfirmDelete;
}

void vtkSlicerApplication::SetModulePath(const char* path)
{
  if (path)
    {
    if (strcmp(this->ModulePath, path) != 0
        && strlen(path) < vtkKWRegistryHelper::RegistryKeyValueSizeMax)
      {
      strcpy(this->ModulePath, path);
      this->Modified();
      }
    }
}

const char* vtkSlicerApplication::GetModulePath() const
{
  return this->ModulePath;
}


void vtkSlicerApplication::SetTemporaryDirectory(const char* path)
{
  if (path)
    {
    if (strcmp(this->TemporaryDirectory, path) != 0
        && strlen(path) < vtkKWRegistryHelper::RegistryKeyValueSizeMax)
      {
      strcpy(this->TemporaryDirectory, path);
      this->Modified();
      }
    }
}

const char* vtkSlicerApplication::GetTemporaryDirectory() const
{
  return this->TemporaryDirectory;
}


//----------------------------------------------------------------------------
void vtkSlicerApplication::CreateProcessingThread()
{
  if (this->ProcessingThreadId == -1)
    {
    this->ProcessingThreadActiveLock->Lock();
    this->ProcessingThreadActive = true;
    this->ProcessingThreadActiveLock->Unlock();
    
    this->ProcessingThreadId
      = this->ProcessingThreader
      ->SpawnThread(vtkSlicerApplication::ProcessingThreaderCallback,
                    this);

    // Setup the communication channel back to the main thread
    this->ModifiedQueueActiveLock->Lock();
    this->ModifiedQueueActive = true;
    this->ModifiedQueueActiveLock->Unlock();

    this->DisplayMessageQueueActiveLock->Lock();
    this->DisplayMessageQueueActive = true;
    this->DisplayMessageQueueActiveLock->Unlock();

    vtkKWTkUtilities::CreateTimerHandler(this, 100, this, "ProcessModified");
    vtkKWTkUtilities::CreateTimerHandler(this, 100, this, "ProcessDisplayMessage");
    }

}

//----------------------------------------------------------------------------
void vtkSlicerApplication::TerminateProcessingThread()
{
  if (this->ProcessingThreadId != -1)
    {
    this->ModifiedQueueActiveLock->Lock();
    this->ModifiedQueueActive = false;
    this->ModifiedQueueActiveLock->Unlock();

    this->DisplayMessageQueueActiveLock->Lock();
    this->DisplayMessageQueueActive = false;
    this->DisplayMessageQueueActiveLock->Unlock();
    
    this->ProcessingThreadActiveLock->Lock();
    this->ProcessingThreadActive = false;
    this->ProcessingThreadActiveLock->Unlock();

    this->ProcessingThreader->TerminateThread( this->ProcessingThreadId );

    this->ProcessingThreadId = -1;
    }
}


ITK_THREAD_RETURN_TYPE
vtkSlicerApplication
::ProcessingThreaderCallback( void *arg )
{
  
#ifdef ITK_USE_WIN32_THREADS
  // Adjust the priority of this thread
  SetThreadPriority(GetCurrentThread(),
                    THREAD_PRIORITY_BELOW_NORMAL);
#endif

#ifdef ITK_USE_PTHREADS
  // Adjust the priority of all PROCESS level threads.  Not a perfect solution.
  nice(20);
#endif
    
  // pull out the reference to the app
  vtkSlicerApplication *app
    = (vtkSlicerApplication*)
    (((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);

  // Tell the app to start processing any tasks slated for the
  // processing thread
  app->ProcessTasks();

  return ITK_THREAD_RETURN_VALUE;
}

void vtkSlicerApplication::ProcessTasks()
{
  bool active = true;
  vtkSmartPointer<vtkSlicerTask> task = 0;
  
  while (active)
    {
    // Check to see if we should be shutting down
    this->ProcessingThreadActiveLock->Lock();
    active = this->ProcessingThreadActive;
    this->ProcessingThreadActiveLock->Unlock();

    if (active)
      {
      // pull a task off the queue
      this->ProcessingTaskQueueLock->Lock();
      if ((*this->InternalTaskQueue).size() > 0)
        {
        std::cout << "Number of queued tasks: " << (*this->InternalTaskQueue).size() << std::endl;
        task = (*this->InternalTaskQueue).front();
        (*this->InternalTaskQueue).pop();
        }
      this->ProcessingTaskQueueLock->Unlock();
      
      // process the task (should this be in a separate thread?)
      if (task)
        {
        task->Execute();
        task = 0;
        }
      }

    // busy wait
    itksys::SystemTools::Delay(100);
    }
}

bool vtkSlicerApplication::ScheduleTask( vtkSlicerTask *task )
{
  bool active;

  std::cout << "Scheduling a task ";
  // only schedule a task if the processing task is up
  this->ProcessingThreadActiveLock->Lock();
  active = this->ProcessingThreadActive;
  this->ProcessingThreadActiveLock->Unlock();

  if (active)
    {
    this->ProcessingTaskQueueLock->Lock();
    (*this->InternalTaskQueue).push( task );
    std::cout << (*this->InternalTaskQueue).size() << std::endl;
    this->ProcessingTaskQueueLock->Unlock();
    
    return true;
    }

  // could not schedule the task
  return false;
}


bool vtkSlicerApplication::RequestModified( vtkObject *obj )
{
  bool active;

  //std::cout << "Requesting a modified on " << obj;

  // only request a Modified if the Modified queue is up
  this->ModifiedQueueActiveLock->Lock();
  active = this->ModifiedQueueActive;
  this->ModifiedQueueActiveLock->Unlock();

  if (active)
    {
    this->ModifiedQueueLock->Lock();
    (*this->InternalModifiedQueue).push( obj );
//     std::cout << " [" << (*this->InternalModifiedQueue).size()
//               << "] " << std::endl;
    this->ModifiedQueueLock->Unlock();
    
    return true;
    }

  // could not request the Modified
  return false;
}

bool vtkSlicerApplication::RequestDisplayMessage( const char *type, const char *message )
{
  bool active;

  //std::cout << "Requesting a message be put on the logger " << type << ": " << message << std::endl;

  // only request to add a record to the log if the log queue is up
  this->DisplayMessageQueueActiveLock->Lock();
  active = this->DisplayMessageQueueActive;
  this->DisplayMessageQueueActiveLock->Unlock();

  if (active)
    {
    this->DisplayMessageQueueLock->Lock();
    (*this->InternalDisplayMessageQueue).push( AddRecordType(type, message) );
//     std::cout << " [" << (*this->InternalDisplayMessageQueue).size()
//               << "] " << std::endl;
    this->DisplayMessageQueueLock->Unlock();
    
    return true;
    }

  // could not request the record be added to the queue
  return false;
}


void vtkSlicerApplication::ProcessModified()
{
  bool active = true;
  vtkSmartPointer<vtkObject> obj = 0;
  
  // Check to see if we should be shutting down
  this->ModifiedQueueActiveLock->Lock();
  active = this->ModifiedQueueActive;
  this->ModifiedQueueActiveLock->Unlock();
  
  if (active)
    {
    // pull an object off the queue to modify
    this->ModifiedQueueLock->Lock();
    if ((*this->InternalModifiedQueue).size() > 0)
      {
      obj = (*this->InternalModifiedQueue).front();
      (*this->InternalModifiedQueue).pop();

      // pop off any extra copies of the same object to save some updates
      while (obj == (*this->InternalModifiedQueue).front())
        {
        (*this->InternalModifiedQueue).pop();
        }
      }
    this->ModifiedQueueLock->Unlock();
    
    // Modify the object
    if (obj)
      {
      obj->Modified();
      obj = 0;
      }
    }
  
  // schedule the next timer
  vtkKWTkUtilities::CreateTimerHandler(this, 100, this, "ProcessModified");
}


void vtkSlicerApplication::ProcessDisplayMessage()
{
  bool active = true;
  AddRecordType record;
  
  // Check to see if we should be shutting down
  this->DisplayMessageQueueActiveLock->Lock();
  active = this->DisplayMessageQueueActive;
  this->DisplayMessageQueueActiveLock->Unlock();
  
  if (active)
    {
    // pull an object off the queue 
    this->DisplayMessageQueueLock->Lock();
    if ((*this->InternalDisplayMessageQueue).size() > 0)
      {
      record = (*this->InternalDisplayMessageQueue).front();
      (*this->InternalDisplayMessageQueue).pop();
      }
    this->DisplayMessageQueueLock->Unlock();

    // force the log display
    if (!vtkSlicerApplication::GetInstance()->GetNumberOfWindowsMapped())
      {
      vtkSlicerApplication::GetInstance()->DisplayLogDialog(NULL);
      }
    
    // post the message
    if (record.first == "Error")
      {
      vtkSlicerApplication::GetInstance()->GetLogDialog()->GetLogWidget()->AddErrorRecord( record.second.c_str() );
      }
    else if (record.first == "Warning")
      {
      vtkSlicerApplication::GetInstance()->GetLogDialog()->GetLogWidget()->AddWarningRecord( record.second.c_str() );
      }
    else if (record.first == "Information")
      {
      vtkSlicerApplication::GetInstance()->GetLogDialog()->GetLogWidget()->AddInformationRecord( record.second.c_str() );
      }
    else if (record.first == "Debug")
      {
      vtkSlicerApplication::GetInstance()->GetLogDialog()->GetLogWidget()->AddDebugRecord( record.second.c_str() );
      }
    }
  
  // schedule the next timer
  vtkKWTkUtilities::CreateTimerHandler(this, 100, this, "ProcessDisplayMessage");
}


void
vtkSlicerApplication::WarningMessage(const char* message)
{
  this->RequestDisplayMessage("Warning", message);
}

void
vtkSlicerApplication::ErrorMessage(const char* message)
{
  this->RequestDisplayMessage("Error", message);
}

void
vtkSlicerApplication::DebugMessage(const char* message)
{
  this->RequestDisplayMessage("Debug", message);
}

void
vtkSlicerApplication::InformationMessage(const char* message)
{
  this->RequestDisplayMessage("Information", message);
}


//----------------------------------------------------------------------------
//  override default behavior of KWWidgets so that toplevel window 
//  can be on top of the tcl interactor (i.e. so it's not 'transient')
//
void vtkSlicerApplication::DisplayTclInteractor(vtkKWTopLevel *master)
{
  vtkKWTclInteractor *tcl_interactor = this->GetTclInteractor();
  if (tcl_interactor)
    {
    if (!master)
      {
      master = this->GetNthWindow(0);
      }
    if (master)
      {
      vtksys_stl::string title;
      if (master->GetTitle())
        {
        title += master->GetTitle();
        title += " : ";
        }
      title += ks_("Tcl Interactor Dialog|Title|Tcl Interactor");
      tcl_interactor->SetTitle(title.c_str());
      // The change:
      // tcl_interactor->SetMasterWindow(master);
      }
    tcl_interactor->Display();
    }
}

//----------------------------------------------------------------------------
//  override default behavior of KWWidgets so that toplevel window 
//  can be on top of the log dialog (i.e. so it's not 'transient')
//
void vtkSlicerApplication::DisplayLogDialog(vtkKWTopLevel* master)
{
  if (this->CreateLogDialog())
    {
    // The change:
    //this->LogDialog->SetMasterWindow(master ? master : this->GetNthWindow(0));
    this->LogDialog->Display();
    }
}

