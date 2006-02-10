
#include "vtkKWApplication.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkObjectFactory.h"
#include "vtkKWWindowBase.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI ( ) {

    NumGUIs = 0;
    // Create the application
    this->KWapp = vtkKWApplication::New ( );
    this->KWapp->SetName ( "3DSlicer" );
    this->KWapp->RestoreApplicationSettingsFromRegistry ( );
    this->KWapp->SetHelpDialogStartingPage ( "http://www.slicer.org" );
    this->GUICollection = vtkSlicerGUICollection::New ( );


    // could initialize Tcl here, no?

}


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( ) {

    if ( this->GUICollection ) {
       this->GUICollection->Delete ( );
    }
    if ( this->KWapp ) {
        this->KWapp->Delete ( );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetKwApplication ( vtkKWApplication *app ) {
    this->KWapp = app;
}


//---------------------------------------------------------------------------
vtkKWApplication* vtkSlicerApplicationGUI::GetKwApplication ( ) {
    return this->KWapp;
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddWindow ( vtkKWWindowBase *win ) {
    this->KWapp->AddWindow ( win );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUI ( vtkSlicerComponentGUI *gui ) {

    // Create if it doesn't exist already
    if ( this->GUICollection == NULL ) {
        this->GUICollection = vtkSlicerGUICollection::New ( );
    } 
    // Add a gui
    this->GUICollection->AddItem ( gui );
    this->NumGUIs = this->GUICollection->GetNumberOfItems ( );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureApplication ( ) {

    this->KWapp->PromptBeforeExitOn ( );
    this->KWapp->SupportSplashScreenOn ( );
    this->KWapp->SplashScreenVisibilityOn ( );
    this->KWapp->SaveUserInterfaceGeometryOn ( );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::CloseWindows ( ) {
    int n, i;
    vtkKWWindowBase *win;
    
    n= this->KWapp->GetNumberOfWindows ( );
    for (i=0; i<n; i++) {
        win = this->KWapp->GetNthWindow ( n );
        win->Close ( );
    }
}


//---------------------------------------------------------------------------
int vtkSlicerApplicationGUI::StartApplication ( ) {

    int ret = 0;

    // Start the application & event loop here
    this->KWapp->Start ( );
    ret = this->KWapp->GetExitStatus ( );

    // Clean up and exit
    this->CloseWindows ( );
    this->KWapp->Delete ( );
    return ret;
}
