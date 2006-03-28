/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerGUI.h"
#include "vtkSlicerGUICollection.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkKWWindow.h"
#include "vtkKWNotebook.h"
#include "vtkKWFrame.h"
#include "vtkKWUserInterfacePanel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerGUI);
vtkCxxRevisionMacro(vtkSlicerGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerGUI::vtkSlicerGUI ( ) {

    // configure the application before creating
    this->SetName ( "3D Slicer Version 3.0 Alpha" );
    this->RestoreApplicationSettingsFromRegistry ( );
    this->SetHelpDialogStartingPage ( "http://www.slicer.org" );

    this->GUICollection = vtkSlicerGUICollection::New ( );
    this->SlicerStyle = vtkSlicerStyle::New ( );
    this->MainSlicerWin = vtkKWWindow::New ( );
    this->MainSlicerWin->SecondaryPanelVisibilityOn ( );
    this->MainSlicerWin->MainPanelVisibilityOn ( );
    this->AddWindow ( this->MainSlicerWin );
    this->MainSlicerWin->Create ( );
    
    this->NumberOfGUIs = 0;

    // could initialize Tcl here, no?

}


//---------------------------------------------------------------------------
vtkSlicerGUI::~vtkSlicerGUI ( ) {

    this->CloseAllWindows ( );
    if ( this->GUICollection ) {
       this->GUICollection->Delete ( );
    }
    if ( this->MainSlicerWin ) {
        this->MainSlicerWin->Delete ( );
    }
    if ( this->SlicerStyle ) {
        this->SlicerStyle->Delete ( );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerGUI::AddGUI ( vtkSlicerComponentGUI *gui ) {

    // Create if it doesn't exist already
    if ( this->GUICollection == NULL ) {
        this->GUICollection = vtkSlicerGUICollection::New ( );
    } 
    // Add a gui
    this->GUICollection->AddItem ( gui );
    this->NumberOfGUIs = this->GUICollection->GetNumberOfItems ( );
}



//---------------------------------------------------------------------------
void vtkSlicerGUI::ConfigureApplication ( ) {

    this->PromptBeforeExitOn ( );
    this->SupportSplashScreenOn ( );
    this->SplashScreenVisibilityOn ( );
    this->SaveUserInterfaceGeometryOn ( );
}




//---------------------------------------------------------------------------
void vtkSlicerGUI::DisplayMainSlicerWindow ( ) {

    this->MainSlicerWin->Display ( );
}




//---------------------------------------------------------------------------
void vtkSlicerGUI::CloseAllWindows ( ) {
    int n, i;
    vtkKWWindowBase *win;
    
    n= this->GetNumberOfWindows ( );
    for (i=0; i<n; i++) {
        win = this->GetNthWindow ( n );
        win->Close ( );
    }
}


//---------------------------------------------------------------------------
int vtkSlicerGUI::StartApplication ( ) {

    int ret = 0;

    // Start the application & event loop here
    this->Start ( );
    this->CloseAllWindows ( );

    // Clean up and exit
    ret = this->GetExitStatus ( );
    return ret;
}
