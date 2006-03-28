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
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkToolkits.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWFrame.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWWindow.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerGUI.h"
#include "vtkSlicerStyle.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSLicerModelsGUI.h"
#include "vtkSlicerApplicationLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  ) {


    //---  
    // widgets used in the Slice module
    this->SliceGUI = NULL;
    this->VolumesGUI = NULL;
    this->ModelsGUI = NULL;

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

    this->DeleteSliceGUI ( );
    this->DeleteGUIPanelWidgets ( );
    this->DeleteFrames ( );
    this->Logic = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteSliceGUI ( ) {
  if ( this->SliceGUI ) {
    this->SliceGUI->Delete ( );
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
void vtkSlicerApplicationGUI::AddGUIObservers ( ) {

    // add SlicerControlButton Observers here.
    this->GetSliceGUI ( )->AddGUIObservers ( );
    this->GetVolumesGUI ( )->AddGUIObservers ( );
    //this->GetModelsGUI ( )->AddGUIObservers ( );
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddLogicObservers ( ) {

    this->GetSliceGUI ( )->AddLogicObservers ( );
    this->GetVolumesGUI ( )->AddLogicObservers ( );
    //this->GetModelsGUI ( )->AddLogicObservers ( );
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMrmlObservers ( ) {

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) {

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
        vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();
        
        if ( app->GetMainSlicerWin() != NULL ) {
            vtkKWWindow *win = app->GetMainSlicerWin();

            // configure default size of GUI
            this->ConfigureMainSlicerWindow ( );
            this->ConfigureMainViewer ( );
            this->ConfigureSliceViewers ( );
            this->ConfigureGUIPanel ( );

            // ---
            // SLICE WIDGET
            // create a slice widget
            this->SliceGUI = vtkSlicerSliceGUI::New ( );
            this->SliceGUI->SetApplication ( (vtkSlicerGUI *)this->GetApplication() );
            this->SliceGUI->BuildGUI ( );
            // Parent Slice0 frame
            this->SliceGUI->GetSliceWidget(0)->GetSliceFrame()->SetParent ( this->DefaultSlice0Frame );
            app->Script ("pack %s -side top -fill both  -padx 0 -pady 0", this->SliceGUI->GetSliceWidget(0)->GetSliceFrame()->GetWidgetName( ) );
            // Parent Slice1 frame
            this->SliceGUI->GetSliceWidget(1)->GetSliceFrame()->SetParent ( this->DefaultSlice1Frame );
            app->Script ("pack %s -side top -fill both -padx 0 -pady 0", this->SliceGUI->GetSliceWidget(1)->GetSliceFrame()->GetWidgetName( ) );
            // Parent Slice2 frame
            this->SliceGUI->GetSliceWidget(2)->GetSliceFrame()->SetParent ( this->DefaultSlice2Frame );
            app->Script ("pack %s -side top -fill both  -padx 0 -pady 0", this->SliceGUI->GetSliceWidget(2)->GetSliceFrame()->GetWidgetName( ) );

            this->BuildLogoGUI ( );
            this->BuildSlicerControlGUI ( );
            this->BuildModuleControlGUI ( );
            // Turn off the tabs for pages in the ModuleControlGUI
            win->GetMainNotebook( )->AlwaysShowTabsOff ( );
            this->BuildSliceControlGUI ( );
            this->BuildViewControlGUI ( );
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildModuleControlGUI ( ) {

            // method contains just the UPpanel setup, 
            // and individual modules should add pages to the UIpanel.
            this->AddGUIPanelForAllModules ( "Volumes" );
            this->BuildGUIForAllModules ( );



}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildLogoGUI ( ) {
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSlicerControlGUI ( ) {
    const char* modules[] = { "data", "volumes", "models" };

    if ( this->GetApplication() != NULL ) {
        vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();
        this->HomeButton->SetParent ( this->SlicerControlFrame );
        this->HomeButton->Create ( );
        this->HomeButton->SetWidth ( 7 );
        //this->HomeButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->HomeButton->SetText ( "home" );

        this->DataButton->SetParent ( this->SlicerControlFrame );
        this->DataButton->Create ( );
        this->DataButton->SetWidth ( 7 );
        //this->DataButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->DataButton->SetText ("data");

        this->VolumesButton->SetParent ( this->SlicerControlFrame );
        this->VolumesButton->Create ( );
        this->VolumesButton->SetWidth ( 7 );
        //this->VolumesButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->VolumesButton->SetText("volumes");

        this->ModelsButton->SetParent ( this->SlicerControlFrame );
        this->ModelsButton->Create ( );
        this->ModelsButton->SetWidth ( 7 );
        //this->ModelsButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->ModelsButton->SetText("models");

        this->ModulesButton->SetParent ( this->SlicerControlFrame );
        this->ModulesButton->Create ( );
        this->ModulesButton->SetWidth ( 10 );
        //this->ModulesButton->SetBackgroundColor ( style->GetGUIBgColor() );
        this->ModulesButton->SetValue ("Volumes");
        this->ModulesButton->IndicatorVisibilityOn ( );
        for ( int i=0; i < sizeof(modules)/sizeof(modules[0]); i++) {
            this->ModulesButton->AddRadioButton( modules[i] );
        }

        app->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -ipady 1", this->HomeButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -ipady 1", this->DataButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -ipady 1", this->VolumesButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -ipady 1", this->ModelsButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -ipady 0", this->ModulesButton->GetWidgetName( ) );
    }
}





//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSliceControlGUI ( ) {
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildViewControlGUI ( ) {
}





//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIPanelForAllModules ( char* modules ){

  // if we add extra pages to the ui_panel, 
  // tabs are displayed, even if we request
  // that they not be. What to do?

  char *a = modules;
  char str[128];
  int i = 0;
  
  sprintf ( str, "%sModuleGUI", a );
  this->ui_panel->AddPage ( str, str, NULL);

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUIForAllModules (  ){

    // Generate list programmatically...
    
    // Create the volumes GUI.
    this->VolumesGUI = vtkSlicerVolumesGUI::New ( );
    this->VolumesGUI->SetApplication ( (vtkSlicerGUI *)this->GetApplication() );
    this->VolumesGUI->BuildGUI ( ui_panel->GetPageWidget ("VolumesModuleGUI" )) ;

    // Create the models GUI.
    //this->ModelsGUI = vtkSlicerVolumesGUI::New ( );
    //this->ModelsGUI->SetApplication ( (vtkSlicerGUI *)this->GetApplication() );
    //this->ModelsGUI->BuildGUI ( ui_panel->GetPageWidget ("VolumesModuleGUI" );

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainSlicerWindow ( ) {

    if ( this->GetApplication() != NULL ) {
        vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
        if ( app->GetMainSlicerWin() != NULL ) {
            vtkKWWindow *win = app->GetMainSlicerWin();
            win->MainPanelVisibilityOn ();
            win->SecondaryPanelVisibilityOn ();
            win->SetSize ( this->GetDefaultSlicerWindowWidth ( ),
                           this->GetDefaultSlicerWindowHeight () );
        }
    }

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainViewer ( ) {
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( app->GetMainSlicerWin() != NULL ) {
            vtkKWWindow *win = app->GetMainSlicerWin();
            //win->GetViewPanelFrame()->SetBackgroundColor (style->GetViewerBgColor() );
            win->GetViewPanelFrame()->SetWidth ( this->GetDefaultMainViewerWidth() );
        }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureSliceViewers ( ) {
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( app->GetMainSlicerWin() != NULL ) {
            vtkKWWindow *win = app->GetMainSlicerWin();
            //win->GetSecondaryPanelFrame( )->SetBackgroundColor (style->GetGUIBgColor() );
            win->GetSecondaryPanelFrame()->SetWidth ( 3.0 * this->GetDefaultSliceGUIFrameWidth () );
            win->GetSecondaryPanelFrame()->SetHeight ( this->GetDefaultSliceGUIFrameHeight () );
            win->GetSecondaryNotebook( )->AlwaysShowTabsOff ( );

            // Parent and configure Slice0 frame
            this->DefaultSlice0Frame->SetParent ( win->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice0Frame->Create ( );
            this->DefaultSlice0Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            this->DefaultSlice0Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );

            // Parent and configure Slice1 frame
            this->DefaultSlice1Frame->SetParent ( win->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice1Frame->Create ( );
            this->DefaultSlice1Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            this->DefaultSlice1Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );

            // Parent and configure Slice2 frame
            this->DefaultSlice2Frame->SetParent ( win->GetSecondaryPanelFrame ( ) );
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
        vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( app->GetMainSlicerWin() != NULL ) {
            vtkKWWindow *win = app->GetMainSlicerWin();

            this->LogoFrame->SetParent ( win->GetMainPanelFrame ( ) );
            this->LogoFrame->Create( );
            this->LogoFrame->SetReliefToGroove ( );
            //this->LogoFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->LogoFrame->SetHeight ( this->GetDefaultLogoFrameHeight ( ) );

            this->SlicerControlFrame->SetParent ( win->GetMainPanelFrame ( ) );
            this->SlicerControlFrame->Create( );
            this->SlicerControlFrame->SetReliefToGroove ( );
            //this->SlicerControlFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->SlicerControlFrame->SetHeight ( this->GetDefaultSlicerControlFrameHeight ( ) );

            this->SliceControlFrame->SetParent ( win->GetMainPanelFrame ( ) );
            this->SliceControlFrame->Create( );
            this->SliceControlFrame->SetReliefToGroove ( );
            //this->SliceControlFrame->SetBackgroundColor ( style->GetGUIBgColor() );
            this->SliceControlFrame->SetHeight ( this->GetDefaultSliceControlFrameHeight ( ) );
            
            this->ViewControlFrame->SetParent ( win->GetMainPanelFrame ( ) );
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
            //win->GetMainPanelFrame( )->SetBackgroundColor ( style->GetGUIBgColor() );
            win->GetMainPanelFrame()->SetWidth ( this->GetDefaultGUIPanelWidth() );
            win->GetMainPanelFrame()->SetHeight ( this->GetDefaultModuleControlFrameHeight() );
            //win->GetMainPanelFrame( )->SetBackgroundColor ( style->GetGUIBgColor() );
            win->GetMainNotebook( )->AlwaysShowTabsOff ( );
            //win->GetMainNotebook( )->SetBackgroundColor ( style->GetGUIBgColor() );
            // create and configure frames  in UI panel
            this->ui_panel->SetName ("SlicerModuleUI");
            this->ui_panel->SetUserInterfaceManager ( win->GetMainUserInterfaceManager ( ) );
            this->ui_panel->Create ( );

            // pack slice and view control frames
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->SliceControlFrame->GetWidgetName( ) );
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->ViewControlFrame->GetWidgetName( ) );
        }
    }

}


