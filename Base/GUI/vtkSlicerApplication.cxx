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
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWRegistryHelper.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerTheme.h"

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

const char *vtkSlicerApplication::ModulePathRegKey = "ModulePath";
const char *vtkSlicerApplication::TemporaryDirectoryRegKey = "TemporaryDirectory";
const char *vtkSlicerApplication::ConfirmDeleteRegKey = "ConfirmDelete";
const char *vtkSlicerApplication::HomeModuleRegKey = "HomeModule";
const char *vtkSlicerApplication::LoadCommandLineModulesRegKey = "LoadCommandLineModules";

vtkSlicerApplication *vtkSlicerApplication::Instance = NULL;

//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerApplication, "$Revision: 1.0 $");

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

//----------------------------------------------------------------------------
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

    strcpy(this->ConfirmDelete, "");
    
    strcpy(this->ModulePath, "");
    strcpy ( this->HomeModule, "");
    this->LoadCommandLineModules = 1;
   
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
    this->ApplicationGUI = NULL;

    this->DisplayMessageQueueActive = false;
    this->DisplayMessageQueueActiveLock = itk::MutexLock::New();
    this->DisplayMessageQueueLock = itk::MutexLock::New();
    
    this->InternalDisplayMessageQueue = new DisplayMessageQueue;

    this->DisplayMessageQueueActiveLock->Lock();
    this->DisplayMessageQueueActive = true;
    this->DisplayMessageQueueActiveLock->Unlock();

    vtkKWTkUtilities::CreateTimerHandler(
      this, 100, this, "ProcessDisplayMessage");

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
    this->ApplicationGUI = NULL;
    
    delete this->InternalDisplayMessageQueue;
    this->InternalDisplayMessageQueue = 0;

    this->DisplayMessageQueueActiveLock->Lock();
    this->DisplayMessageQueueActive = false;
    this->DisplayMessageQueueActiveLock->Unlock();
    
}

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkSlicerApplication* vtkSlicerApplication::New()
{
  vtkSlicerApplication* ret = vtkSlicerApplication::GetInstance();
  ret->Register(NULL);
  return ret;
}

//----------------------------------------------------------------------------
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
  strcpy(this->TemporaryDirectory, "/tmp");
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
    2, "RunTime", vtkSlicerApplication::HomeModuleRegKey))
    {
    this->GetRegistryValue(
      2, "RunTime", vtkSlicerApplication::HomeModuleRegKey,
      this->HomeModule);
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

  if (this->HasRegistryValue(
    2, "RunTime", vtkSlicerApplication::LoadCommandLineModulesRegKey))
    {
    this->LoadCommandLineModules = this->GetIntRegistryValue(
      2, "RunTime", vtkSlicerApplication::LoadCommandLineModulesRegKey);
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
    2, "RunTime", vtkSlicerApplication::HomeModuleRegKey, "%s", 
    this->HomeModule);

  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::ModulePathRegKey, "%s", 
    this->ModulePath);

  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::TemporaryDirectoryRegKey, "%s", 
    this->TemporaryDirectory);

  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::LoadCommandLineModulesRegKey, "%d", 
    this->LoadCommandLineModules);
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
const char *vtkSlicerApplication::GetConfirmDelete() const
{
    return this->ConfirmDelete;
}

//----------------------------------------------------------------------------
void vtkSlicerApplication::SetHomeModule ( const char *name )
{
  if (name)
    {
    if (strcmp(this->HomeModule, name) != 0
        && strlen(name) < vtkKWRegistryHelper::RegistryKeyValueSizeMax)
      {
      strcpy(this->HomeModule, name);
      this->Modified();

      }
    }
}

//----------------------------------------------------------------------------
const char *vtkSlicerApplication::GetHomeModule () const
{
  return this->HomeModule;
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
const char* vtkSlicerApplication::GetModulePath() const
{
  return this->ModulePath;
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
const char* vtkSlicerApplication::GetTemporaryDirectory() const
{
  return this->TemporaryDirectory;
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void vtkSlicerApplication::ProcessDisplayMessage()
{
  bool active = true;
  AddRecordType record;
  record.first = "";
  
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

//----------------------------------------------------------------------------
void
vtkSlicerApplication::WarningMessage(const char* message)
{
  this->RequestDisplayMessage("Warning", message);
}

//----------------------------------------------------------------------------
void
vtkSlicerApplication::ErrorMessage(const char* message)
{
  this->RequestDisplayMessage("Error", message);
}

//----------------------------------------------------------------------------
void
vtkSlicerApplication::DebugMessage(const char* message)
{
  this->RequestDisplayMessage("Debug", message);
}

//----------------------------------------------------------------------------
void
vtkSlicerApplication::InformationMessage(const char* message)
{
  this->RequestDisplayMessage("Information", message);
}

//----------------------------------------------------------------------------
void vtkSlicerApplication::AddAboutCopyrights(ostream &os)
{
  os << "See http://www.na-mic.org/Wiki/index.php/Slicer3:Acknowledgements" << endl << endl;

  os << "VTK http://www.vtk.org/copyright.php" << endl;
  os << "ITK http://www.itk.org/HTML/Copyright.htm" << endl;
  os << "KWWidgets http://www.kitware.com/Copyright.htm" << endl;
  os << "Tcl/Tk http://www.tcl.tk" << endl;
  os << "Teem:  http://teem.sf.net" << endl;
  os << "Supported by: NA-MIC, NAC, BIRN, NCIGT and the Slicer Community." << endl;
  os << "Special thanks to the NIH and our other supporters." << endl;
  os << "This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149. Information on the National Centers for Biomedical Computing can be obtained from http://nihroadmap.nih.gov/bioinformatics." << endl;

#if 0
  // example of the extra detail needed:
  //
     << tcl_major << "." << tcl_minor << "." << tcl_patch_level << endl
     << "  - Copyright (c) 1989-1994 The Regents of the University of "
     << "California." << endl
     << "  - Copyright (c) 1994 The Australian National University." << endl
     << "  - Copyright (c) 1994-1998 Sun Microsystems, Inc." << endl
     << "  - Copyright (c) 1998-2000 Ajuba Solutions." << endl;
#endif
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

