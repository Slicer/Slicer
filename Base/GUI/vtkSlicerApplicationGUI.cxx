/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

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

    NumberOfGUIs = 0;
    // Create the application
    this->KWApplication = vtkKWApplication::New ( );
    this->KWApplication->SetName ( "3D Slicer Version 3.0 Alpha" );
    this->KWApplication->RestoreApplicationSettingsFromRegistry ( );
    this->KWApplication->SetHelpDialogStartingPage ( "http://www.slicer.org" );
    this->GUICollection = vtkSlicerGUICollection::New ( );

    this->Logic = NULL;

    // could initialize Tcl here, no?

}


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( ) {

    if ( this->GUICollection ) {
       this->GUICollection->Delete ( );
    }
    if ( this->KWApplication ) {
        this->KWApplication->Delete ( );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddWindow ( vtkKWWindowBase *win ) {
    this->KWApplication->AddWindow ( win );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUI ( vtkSlicerComponentGUI *gui ) {

    // Create if it doesn't exist already
    if ( this->GUICollection == NULL ) {
        this->GUICollection = vtkSlicerGUICollection::New ( );
    } 
    // Add a gui
    this->GUICollection->AddItem ( gui );
    this->NumberOfGUIs = this->GUICollection->GetNumberOfItems ( );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureApplication ( ) {

    this->KWApplication->PromptBeforeExitOn ( );
    this->KWApplication->SupportSplashScreenOn ( );
    this->KWApplication->SplashScreenVisibilityOn ( );
    this->KWApplication->SaveUserInterfaceGeometryOn ( );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::CloseWindows ( ) {
    int n, i;
    vtkKWWindowBase *win;
    
    n= this->KWApplication->GetNumberOfWindows ( );
    for (i=0; i<n; i++) {
        win = this->KWApplication->GetNthWindow ( n );
        win->Close ( );
    }
}


//---------------------------------------------------------------------------
int vtkSlicerApplicationGUI::StartApplication ( ) {

    int ret = 0;

    // Start the application & event loop here
    this->KWApplication->Start ( );
    ret = this->KWApplication->GetExitStatus ( );

    // Clean up and exit
    this->CloseWindows ( );
    this->KWApplication->Delete ( );
    return ret;
}
