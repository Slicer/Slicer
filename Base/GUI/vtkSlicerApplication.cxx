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

#ifdef WIN32
#include "vtkKWWin32RegistryHelper.h"
#endif

#include "itksys/SystemTools.hxx"

const char *vtkSlicerApplication::ModulePathRegKey = "ModulePath";
const char *vtkSlicerApplication::TemporaryDirectoryRegKey = "TemporaryDirectory";


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplication);
vtkCxxRevisionMacro(vtkSlicerApplication, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplication::vtkSlicerApplication ( ) {

    strcpy(this->ModulePath, "");

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
    2, "RunTime", vtkSlicerApplication::ModulePathRegKey, "%s", 
    this->ModulePath);

  this->SetRegistryValue(
    2, "RunTime", vtkSlicerApplication::TemporaryDirectoryRegKey, "%s", 
    this->TemporaryDirectory);
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
