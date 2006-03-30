/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkToolkits.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWWindow.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWFrame.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWWindow.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerStyle.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplicationLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  ) {


    //---  
    // widgets used in the Slice module
    this->MainSlicerWin = vtkKWWindow::New ( );
    
    this->SliceGUI = NULL;
    this->VolumesGUI = NULL;
    this->ModelsGUI = NULL;
    this->VolumesModuleGUIID = 0;
    this->ModelsModuleGUIID = 0;
    
    // Control frames that comprise the Main Slicer GUI
    this->LogoFrame = vtkKWFrame::New();
    this->SlicerControlFrame = vtkKWFrame::New();
    this->ModuleControlFrame = vtkKWFrame::New();
    this->SliceControlFrame = vtkKWFrame::New();    
    this->ViewControlFrame = vtkKWFrame::New();    
    this->DefaultSlice0Frame = vtkKWFrame::New ();
    this->DefaultSlice1Frame = vtkKWFrame::New ();
    this->DefaultSlice2Frame = vtkKWFrame::New ();

    // ui panel used to display a GUI page for each module.
    this->ui_panel = vtkKWUserInterfacePanel::New();
    this->HomeButton = vtkKWPushButton::New();
    this->DataButton = vtkKWPushButton::New();
    this->VolumesButton = vtkKWPushButton::New();
    this->ModelsButton = vtkKWPushButton::New();
    this->ModulesButton = vtkKWMenuButton::New();

    this->InitDefaultGUIPanelDimensions ( );
    this->InitDefaultSlicePanelDimensions ( );
    this->InitDefaultMainViewerDimensions ( );
    this->InitDefaultSlicerWindowDimensions ( );
}



//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( ) {

    if ( this->MainSlicerWin ) {
        this->MainSlicerWin->Delete ( );
    }
    this->DeleteGUIs ( );
    this->DeleteGUIPanelWidgets ( );
    this->DeleteFrames ( );
    this->Logic = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( ) {

    // add SlicerControlButton Observers here.
    this->GetSliceGUI ( )->AddGUIObservers ( );
    this->GetVolumesGUI ( )->AddGUIObservers ( );
    this->GetModelsGUI ( )->AddGUIObservers ( );

    // add observers onto the buttons and menubutton in the SlicerControl frame
    this->HomeButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    this->DataButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    this->VolumesButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    this->ModelsButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    this->ModulesButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );    
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddLogicObservers ( ) {

    this->GetSliceGUI ( )->AddLogicObservers ( );
    this->GetVolumesGUI ( )->AddLogicObservers ( );
    this->GetModelsGUI ( )->AddLogicObservers ( );
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMrmlObservers ( ) {

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) {
    

    // This is just a placeholder:
    // Actually, these events want to set "activeModule" in the logic;
    // using this->Logic->SetActiveModule ( ) which is currently commented out.
    // Observers on that logic should raise and lower the appropriate page.
    // So for now, the GUI is controlling the GUI instead of going thru the logic.
    //---
    vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
    vtkKWMenuButton *menub = vtkKWMenuButton::SafeDownCast (caller );
    if ( pushb == this->HomeButton && event == vtkCommand::ModifiedEvent ) {
        this->ui_panel->RaisePage ( this->VolumesModuleGUIID );
    } else if (pushb == this->DataButton && event == vtkCommand::ModifiedEvent ) {
        //
    } else if (pushb == this->VolumesButton && event == vtkCommand::ModifiedEvent ) {
        this->ui_panel->RaisePage ( this->VolumesModuleGUIID );
    } else if (pushb == this->ModelsButton && event == vtkCommand::ModifiedEvent ) {
        this->ui_panel->RaisePage ( this->ModelsModuleGUIID );
    }
    if ( menub == this->ModulesButton && event == vtkCommand::ModifiedEvent ) {
        this->ui_panel->RaisePage ( this->ModulesButton->GetValue() );
    }
    

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessMrmlEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) {
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessLogicEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) {

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUI ( ) {

    // Set up the conventional window: 3Dviewer, slice widgets, UI panel for now.
    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( this->MainSlicerWin != NULL ) {

            // set up Slicer's main window
            this->MainSlicerWin->SecondaryPanelVisibilityOn ( );
            this->MainSlicerWin->MainPanelVisibilityOn ( );
            app->AddWindow ( this->MainSlicerWin );
            this->MainSlicerWin->Create ( );        

            // configure default size of GUI
            this->ConfigureMainSlicerWindow ( );
            this->ConfigureMainViewer ( );
            this->ConfigureSliceViewers ( );
            this->ConfigureGUIPanel ( );

            // ---
            // SLICE WIDGET
            // create a slice widget
            this->SliceGUI = vtkSlicerSliceGUI::New ( );
            this->SliceGUI->SetApplication ( (vtkSlicerApplication *)this->GetApplication() );
            this->SliceGUI->BuildGUI ( this->DefaultSlice0Frame,
                                       this->DefaultSlice1Frame,
                                       this->DefaultSlice2Frame );
            // Build main GUI panel
            this->BuildLogoGUI ( );
            this->BuildSlicerControlGUI ( );
            this->BuildModuleControlGUI ( );

            // Turn off the tabs for pages in the ModuleControlGUI
            this->MainSlicerWin->GetMainNotebook( )->AlwaysShowTabsOff ( );
            this->MainSlicerWin->GetMainNotebook( )->ShowIconsOff ( );
            this->BuildSliceControlGUI ( );
            this->BuildViewControlGUI ( );
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayMainSlicerWindow ( ) {

    this->MainSlicerWin->Display ( );
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteGUIs ( ) {
  if ( this->SliceGUI ) {
    this->SliceGUI->Delete ( );
  }
  if ( this->VolumesGUI ) {
      this->VolumesGUI->Delete ();
  }
  if ( this->ModelsGUI ) {
      this->ModelsGUI->Delete ( );
  }
  
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteGUIPanelWidgets ( ) {
    if ( this->HomeButton ) {
        this->HomeButton->Delete ();
    }
    if ( this->DataButton) {
        this->DataButton->Delete ();
    }
    if ( this->VolumesButton ) {
        this->VolumesButton->Delete ();
    }
    if ( this->ModelsButton) {
        this->ModelsButton->Delete ();
    }
    if ( this->ModulesButton ) {
        this->ModulesButton->Delete();
    }
    if ( this->ui_panel ) {
        this->ui_panel->Delete ();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteFrames ( ) {
    if ( this->LogoFrame ) {
        this->LogoFrame->Delete ();
    }
    if ( this->SlicerControlFrame ) {
        this->SlicerControlFrame->Delete ();
    }
    if ( this->ModuleControlFrame ) {
        this->ModuleControlFrame->Delete ( );
    }
    if ( this->SliceControlFrame ) {
        this->SliceControlFrame->Delete ( );
    }
    if ( this->ViewControlFrame ) {
        this->ViewControlFrame->Delete ( );
    }
    if ( this->DefaultSlice0Frame ) {
        this->DefaultSlice0Frame->Delete ();
    }
    if ( this->DefaultSlice1Frame ) {
        this->DefaultSlice1Frame->Delete ();
    }
    if ( this->DefaultSlice2Frame ) {
        this->DefaultSlice2Frame->Delete ();
    }

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitDefaultGUIPanelDimensions ( ) {
    // specify dims of GUI Panel components here for now.
    this->SetDefaultLogoFrameHeight ( 40 );
    this->SetDefaultSlicerControlFrameHeight ( 60 );
    this->SetDefaultModuleControlFrameHeight ( 500 );
    this->SetDefaultSliceControlFrameHeight ( 60 );
    this->SetDefaultViewControlFrameHeight ( 240 );
    // entire GUI panel height and width
    int h = this->GetDefaultLogoFrameHeight ( ) +
        this->GetDefaultSlicerControlFrameHeight ( ) +
        this->GetDefaultModuleControlFrameHeight ( ) +
        this->GetDefaultSliceControlFrameHeight ( ) +
        this->GetDefaultViewControlFrameHeight ( );
    this->SetDefaultGUIPanelHeight ( h );
    this->SetDefaultGUIPanelWidth ( 300 );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitDefaultSlicePanelDimensions ( ) {
    // constrain the slice windows to be a particular size
    this->SetDefaultSliceGUIFrameHeight ( 120 );
    this->SetDefaultSliceGUIFrameWidth ( 102 );
    this->SetDefaultSliceWindowHeight ( 100 );
    this->SetDefaultSliceWindowWidth ( 100 );
    int w = 3.0 * this->GetDefaultSliceGUIFrameWidth ( );


}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitDefaultMainViewerDimensions ( ) {
    int h = this->GetDefaultLogoFrameHeight ( ) +
        this->GetDefaultSlicerControlFrameHeight ( ) +
        this->GetDefaultModuleControlFrameHeight ( ) +
        this->GetDefaultSliceControlFrameHeight ( ) +
        this->GetDefaultViewControlFrameHeight ( );
    int w = 3.0 * this->GetDefaultSliceGUIFrameWidth ( );
    // set up default Slicer Window size here for now
    this->SetDefaultMainViewerHeight ( h - this->GetDefaultSliceGUIFrameHeight () );
    this->SetDefaultMainViewerWidth ( w );
}

    
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitDefaultSlicerWindowDimensions ( ) {

    int hbuf = 10;
    int vbuf = 60;
    // make room for window chrome or whatever; have to play with this buffer.
    int h = this->GetDefaultLogoFrameHeight ( ) +
        this->GetDefaultSlicerControlFrameHeight ( ) +
        this->GetDefaultModuleControlFrameHeight ( ) +
        this->GetDefaultSliceControlFrameHeight ( ) +
        this->GetDefaultViewControlFrameHeight ( ) + hbuf;
    int w = 3.0 * this->GetDefaultSliceGUIFrameWidth ( );
    w = w + this->GetDefaultGUIPanelWidth ( ) + hbuf;
    this->SetDefaultSlicerWindowWidth ( w + this->GetDefaultGUIPanelWidth ( ) );
    this->SetDefaultSlicerWindowHeight ( h );
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildModuleControlGUI ( ) {

    // and individual modules should add pages to the UIpanel and build their gui.
    // stuffed here for now; 
    
    // Create the volumes GUI.
    this->VolumesModuleGUIID=this->ui_panel->AddPage ( "Volumes", "Volumes", NULL);
    this->VolumesGUI = vtkSlicerVolumesGUI::New ( );
    this->VolumesGUI->SetApplication ( (vtkSlicerApplication *)this->GetApplication() );
    this->VolumesGUI->BuildGUI ( ui_panel->GetPageWidget ("Volumes" )) ;
    // Create the models GUI.
    this->ModelsModuleGUIID=this->ui_panel->AddPage ( "Models", "Models", NULL);
    this->ModelsGUI = vtkSlicerModelsGUI::New ( );
    this->ModelsGUI->SetApplication ( (vtkSlicerApplication *)this->GetApplication() );
    this->ModelsGUI->BuildGUI ( ui_panel->GetPageWidget ("Models" ) );



}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildLogoGUI ( ) {
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSlicerControlGUI ( ) {
    const char* modules[] = { "Data", "Volumes", "Models" };

    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();
        this->HomeButton->SetParent ( this->SlicerControlFrame );
        this->HomeButton->Create ( );
        this->HomeButton->SetWidth ( 7 );
        //this->HomeButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->HomeButton->SetText ( "Home" );

        this->DataButton->SetParent ( this->SlicerControlFrame );
        this->DataButton->Create ( );
        this->DataButton->SetWidth ( 7 );
        //this->DataButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->DataButton->SetText ("Data");

        this->VolumesButton->SetParent ( this->SlicerControlFrame );
        this->VolumesButton->Create ( );
        this->VolumesButton->SetWidth ( 7 );
        //this->VolumesButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->VolumesButton->SetText("Volumes");

        this->ModelsButton->SetParent ( this->SlicerControlFrame );
        this->ModelsButton->Create ( );
        this->ModelsButton->SetWidth ( 7 );
        //this->ModelsButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->ModelsButton->SetText("Models");

        this->ModulesButton->SetParent ( this->SlicerControlFrame );
        this->ModulesButton->Create ( );
        this->ModulesButton->SetWidth ( 10 );
        //this->ModulesButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->ModulesButton->SetValue ("Volumes");
        this->ModulesButton->IndicatorVisibilityOn ( );
        for ( int i=0; i < sizeof(modules)/sizeof(modules[0]); i++) {
            this->ModulesButton->AddRadioButton( modules[i] );
        }

        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->HomeButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->DataButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->VolumesButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->ModelsButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 0", this->ModulesButton->GetWidgetName( ) );
    }
}





//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSliceControlGUI ( ) {
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildViewControlGUI ( ) {
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainSlicerWindow ( ) {

    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        if ( this->MainSlicerWin != NULL ) {
            this->MainSlicerWin->MainPanelVisibilityOn ();
            this->MainSlicerWin->SecondaryPanelVisibilityOn ();
            this->MainSlicerWin->SetSize ( this->GetDefaultSlicerWindowWidth ( ),
                           this->GetDefaultSlicerWindowHeight () );
        }
    }

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainViewer ( ) {
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( this->MainSlicerWin != NULL ) {
            //this->MainSlicerWin->GetViewPanelFrame()->SetBackgroundColor (style->GetViewerBgColor() );
            this->MainSlicerWin->GetViewPanelFrame()->SetWidth ( this->GetDefaultMainViewerWidth() );
        }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureSliceViewers ( ) {
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( this->MainSlicerWin != NULL ) {
            //this->MainSlicerWin->GetSecondaryPanelFrame( )->SetBackgroundColor (style->GetGUIBgColor() );
            this->MainSlicerWin->GetSecondaryPanelFrame()->SetWidth ( 3.0 * this->GetDefaultSliceGUIFrameWidth () );
            this->MainSlicerWin->GetSecondaryPanelFrame()->SetHeight ( this->GetDefaultSliceGUIFrameHeight () );
            this->MainSlicerWin->GetSecondaryNotebook( )->AlwaysShowTabsOff ( );

            // Parent and configure Slice0 frame
            this->DefaultSlice0Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice0Frame->Create ( );
            this->DefaultSlice0Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            this->DefaultSlice0Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );

            // Parent and configure Slice1 frame
            this->DefaultSlice1Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice1Frame->Create ( );
            this->DefaultSlice1Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            this->DefaultSlice1Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );

            // Parent and configure Slice2 frame
            this->DefaultSlice2Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice2Frame->Create ( );
            this->DefaultSlice2Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            this->DefaultSlice2Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );
            
            // pack them.
            app->Script ("pack %s -side left  -padx 0 -pady 0", this->DefaultSlice0Frame->GetWidgetName( ) );
            app->Script ("pack %s -side left  -padx 0 -pady 0", this->DefaultSlice1Frame->GetWidgetName( ) );
            app->Script ("pack %s -side left  -padx 0 -pady 0", this->DefaultSlice2Frame->GetWidgetName( ) );


        }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureGUIPanel ( ) {

    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( this->MainSlicerWin != NULL ) {

            this->LogoFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->LogoFrame->Create( );
            this->LogoFrame->SetReliefToGroove ( );
            //this->LogoFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->LogoFrame->SetHeight ( this->GetDefaultLogoFrameHeight ( ) );

            this->SlicerControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SlicerControlFrame->Create( );
            this->SlicerControlFrame->SetReliefToGroove ( );
            //this->SlicerControlFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->SlicerControlFrame->SetHeight ( this->GetDefaultSlicerControlFrameHeight ( ) );

            this->SliceControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SliceControlFrame->Create( );
            this->SliceControlFrame->SetReliefToGroove ( );
            //this->SliceControlFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->SliceControlFrame->SetHeight ( this->GetDefaultSliceControlFrameHeight ( ) );
            
            this->ViewControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->ViewControlFrame->Create( );
            this->ViewControlFrame->SetReliefToGroove ( );
            //this->ViewControlFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->ViewControlFrame->SetHeight ( this->GetDefaultViewControlFrameHeight ( ) );

            // pack logo and slicer control frames
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->LogoFrame->GetWidgetName( ) );
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->SlicerControlFrame->GetWidgetName( ) );

            // UI PANEL FRAME
            // ---
            // frame already created by KWWidgets; just configure.
            //this->MainSlicerWin->GetMainPanelFrame( )->SetBackgroundColor ( style->GetGUIBgColor() );
            this->MainSlicerWin->GetMainPanelFrame()->SetWidth ( this->GetDefaultGUIPanelWidth() );
            this->MainSlicerWin->GetMainPanelFrame()->SetHeight ( this->GetDefaultModuleControlFrameHeight() );
            //this->MainSlicerWin->GetMainPanelFrame( )->SetBackgroundColor ( style->GetGUIBgColor() );
            this->MainSlicerWin->GetMainNotebook( )->AlwaysShowTabsOff ( );
            //this->MainSlicerWin->GetMainNotebook( )->SetBackgroundColor ( style->GetGUIBgColor() );
            // create and configure frames  in UI panel
            this->ui_panel->SetName ("SlicerModuleUI");
            this->ui_panel->SetUserInterfaceManager ( this->MainSlicerWin->GetMainUserInterfaceManager ( ) );
            this->ui_panel->Create ( );

            // pack slice and view control frames
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->SliceControlFrame->GetWidgetName( ) );
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->ViewControlFrame->GetWidgetName( ) );
        }
    }

}


