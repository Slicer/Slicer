#include "vtkObjectFactory.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkKWNotebook.h"
#include "vtkKWFrame.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWWindowBase.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplication);
vtkCxxRevisionMacro(vtkSlicerApplication, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplication::vtkSlicerApplication ( ) {

    // configure the application before creating
    this->SetName ( "3D Slicer Version 3.0 Alpha" );
    this->RestoreApplicationSettingsFromRegistry ( );
    this->SetHelpDialogStartingPage ( "http://www.slicer.org" );

    this->GUICollection = vtkSlicerGUICollection::New ( );
    this->SlicerStyle = vtkSlicerStyle::New ( );
    this->NumberOfGUIs = 0;

    // could initialize Tcl here, no?

}


//---------------------------------------------------------------------------
vtkSlicerApplication::~vtkSlicerApplication ( ) {

    this->CloseAllWindows ( );
    if ( this->GUICollection ) {
       this->GUICollection->Delete ( );
    }
    this->SlicerStyle(NULL);

}



//---------------------------------------------------------------------------
void vtkSlicerApplication::AddGUI ( vtkSlicerComponentGUI *gui ) {

    // Create if it doesn't exist already
    if ( this->GUICollection == NULL ) {
        this->GUICollection = vtkSlicerGUICollection::New ( );
    } 
    // Add a gui
    this->GUICollection->AddItem ( gui );
    this->NumberOfGUIs = this->GUICollection->GetNumberOfItems ( );
}



//---------------------------------------------------------------------------
void vtkSlicerApplication::ConfigureApplication ( ) {

    this->PromptBeforeExitOn ( );
    this->SupportSplashScreenOn ( );
    this->SplashScreenVisibilityOn ( );
    this->SaveUserInterfaceGeometryOn ( );
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
