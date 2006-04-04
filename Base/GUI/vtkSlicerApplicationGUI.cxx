/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWWidget.h"
#include "vtkKWWindow.h"
#include "vtkKWWindow.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWUserInterfaceManagerNotebook.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerStyle.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkToolkits.h"
// things for temporary MainViewer teapot display.
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkKWRenderWidget.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkXMLPolyDataReader.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  ) {


    //---  
    // widgets used in the Slice module
    this->MainSlicerWin = vtkKWWindow::New ( );
    
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
    this->HomeButton = vtkKWPushButton::New();
    this->DataButton = vtkKWPushButton::New();
    this->VolumesButton = vtkKWPushButton::New();
    this->ModelsButton = vtkKWPushButton::New();
    this->ModulesButton = vtkKWMenuButton::New();
    this->MainViewer = vtkKWRenderWidget::New ( );

    this->InitDefaultGUIPanelDimensions ( );
    this->InitDefaultSlicePanelDimensions ( );
    this->InitDefaultMainViewerDimensions ( );
    this->InitDefaultSlicerWindowDimensions ( );
}



//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( ) {

    this->RemoveGUIObservers ( );
    this->RemoveMrmlObservers ( );
    this->RemoveLogicObservers ( );

    this->DeleteGUIs ( );
    this->DeleteGUIPanelWidgets ( );
    this->DeleteFrames ( );


    if ( this->MainViewer ) {
        this->MainViewer->Delete ( );
        this->MainViewer = NULL;
    }
    if ( this->MainSlicerWin ) {
        this->MainSlicerWin->Delete ( );
        this->MainSlicerWin = NULL;
    }
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( ) {

    // add SlicerControlButton Observers here.
    this->GetVolumesGUI ( )->AddGUIObservers ( );
    this->GetModelsGUI ( )->AddGUIObservers ( );
    // add observers onto the buttons and menubutton in the SlicerControl frame
    this->HomeButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->DataButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->VolumesButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->ModelsButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->ModulesButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );    
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddLogicObservers ( ) {

    this->GetVolumesGUI ( )->AddLogicObservers ( );
    this->GetModelsGUI ( )->AddLogicObservers ( );
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMrmlObservers ( ) {
    this->GetVolumesGUI ( )->AddMrmlObservers ( );
    this->GetModelsGUI ( )->AddMrmlObservers ( );
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveGUIObservers ( ) {
  if (this->VolumesGUI)
    {
    this->VolumesGUI->RemoveGUIObservers ( );
    }
  if ( this->ModelsGUI ) 
    {
    this->ModelsGUI->RemoveGUIObservers ( );
    }
    this->HomeButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->DataButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->VolumesButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->ModelsButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICommand );
    this->ModulesButton->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );    
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveMrmlObservers ( ) {
  if ( this->VolumesGUI ) 
    {
    this->VolumesGUI->RemoveMrmlObservers ( );
    }
  if ( this->ModelsGUI ) 
    {
    this->ModelsGUI->RemoveMrmlObservers ( );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveLogicObservers ( ) {
  if ( this->VolumesGUI ) 
    {
    this->VolumesGUI->RemoveLogicObservers ( );
    }
  if ( this->ModelsGUI ) 
    {
    this->ModelsGUI->RemoveLogicObservers ( );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) {
    
    // This is just a placeholder:
    // TODO:
    // Actually, these events want to set "activeModule" in the logic;
    // using this->Logic->SetActiveModule ( ) which is currently commented out.
    // Observers on that logic should raise and lower the appropriate page.
    // So for now, the GUI is controlling the GUI instead of going thru the logic.
    //---
    vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
    vtkKWMenuButton *menub = vtkKWMenuButton::SafeDownCast (caller );
    if ( pushb == this->HomeButton && event == vtkKWPushButton::InvokedEvent ) {
        this->VolumesGUI->GetUIPanel()->Raise( );
    } else if (pushb == this->DataButton && event == vtkKWPushButton::InvokedEvent ) {
        //
    } else if (pushb == this->VolumesButton && event == vtkKWPushButton::InvokedEvent ) {
        this->VolumesGUI->GetUIPanel()->Raise ( );
    } else if (pushb == this->ModelsButton && event == vtkKWPushButton::InvokedEvent ) {
        this->ModelsGUI->GetUIPanel()->Raise ( );
    }
    /*
    if ( menub == this->ModulesButton && event == vtkCommand::ModifiedEvent ) {
        if ( this->ModulesButton->GetValue() == this->VolumesGUI->GetModuleUIPageID() ) {
            this->VolumesGUI->GetUIPanel()->Raise ( );
        }
        if ( this->ModulesButton->GetValue() == this->ModelsGUI->GetModuleUIPageID() ) {
            this->ModelsGUI->GetUIPanel()->Raise ( );
        }
    }
    */

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
            this->ConfigureMainViewerPanel ( );
            this->ConfigureSliceViewersPanel ( );
            this->ConfigureGUIPanel ( );

            // Build 3DViewer
            this->BuildMainViewer ( );

            // Build main GUI panel
            this->BuildLogoGUIPanel ( );
            this->BuildSlicerControlGUIPanel ( );
            this->BuildModuleControlGUIPanel ( );

            // Turn off the tabs for pages in the ModuleControlGUI
            this->MainSlicerWin->GetMainNotebook( )->ShowIconsOff ( );
            this->BuildSliceControlGUIPanel ( );
            this->BuildViewControlGUIPanel ( );
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayMainSlicerWindow ( ) {

    this->MainSlicerWin->Display ( );
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteGUIs ( ) {

    if ( this->VolumesGUI ) {
        this->VolumesGUI->Delete ();
        this->VolumesGUI = NULL;
    }
    if ( this->ModelsGUI ) {
        this->ModelsGUI->Delete ( );
        this->ModelsGUI = NULL;
    }

  
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteGUIPanelWidgets ( ) {
    if ( this->HomeButton ) {
        this->HomeButton->Delete ();
        this->HomeButton = NULL;
    }
    if ( this->DataButton) {
        this->DataButton->Delete ();
        this->DataButton = NULL;
    }
    if ( this->VolumesButton ) {
        this->VolumesButton->Delete ();
        this->VolumesButton = NULL;
    }
    if ( this->ModelsButton) {
        this->ModelsButton->Delete ();
        this->ModelsButton = NULL;
    }
    if ( this->ModulesButton ) {
        this->ModulesButton->Delete();
        this->ModulesButton = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteFrames ( ) {
    if ( this->LogoFrame ) {
        this->LogoFrame->Delete ();
        this->LogoFrame = NULL;
    }
    if ( this->SlicerControlFrame ) {
        this->SlicerControlFrame->Delete ();
        this->SlicerControlFrame = NULL;
    }
    if ( this->ModuleControlFrame ) {
        this->ModuleControlFrame->Delete ( );
        this->ModuleControlFrame = NULL;
    }
    if ( this->SliceControlFrame ) {
        this->SliceControlFrame->Delete ( );
        this->SliceControlFrame = NULL;
    }
    if ( this->ViewControlFrame ) {
        this->ViewControlFrame->Delete ( );
        this->ViewControlFrame = NULL;
    }
    if ( this->DefaultSlice0Frame ) {
        this->DefaultSlice0Frame->Delete ();
        this->DefaultSlice0Frame = NULL;
    }
    if ( this->DefaultSlice1Frame ) {
        this->DefaultSlice1Frame->Delete ();
        this->DefaultSlice1Frame = NULL;
    }
    if ( this->DefaultSlice2Frame ) {
        this->DefaultSlice2Frame->Delete ();
        this->DefaultSlice2Frame = NULL;
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
    int w = 3 * this->GetDefaultSliceGUIFrameWidth ( );


}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitDefaultMainViewerDimensions ( ) {
    int h = this->GetDefaultLogoFrameHeight ( ) +
        this->GetDefaultSlicerControlFrameHeight ( ) +
        this->GetDefaultModuleControlFrameHeight ( ) +
        this->GetDefaultSliceControlFrameHeight ( ) +
        this->GetDefaultViewControlFrameHeight ( );
    int w = 3 * this->GetDefaultSliceGUIFrameWidth ( );
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
    int w = 3 * this->GetDefaultSliceGUIFrameWidth ( );
    w = w + this->GetDefaultGUIPanelWidth ( ) + hbuf;
    this->SetDefaultSlicerWindowWidth ( w + this->GetDefaultGUIPanelWidth ( ) );
    this->SetDefaultSlicerWindowHeight ( h );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildModuleControlGUIPanel ( ) {

    // and individual modules should add pages to the UIpanel and build their gui.
    // stuffed here for now; 
    // turn off tabs in notebook.


    // Create the volumes GUI.
    this->VolumesGUI = vtkSlicerVolumesGUI::New ( );
    this->VolumesGUI->SetMrml ( this->GetMrml() );
    this->VolumesGUI->SetApplication ( (vtkSlicerApplication *)this->GetApplication() );
    this->VolumesGUI->GetUIPanel()->SetName ("VolumesUI");
    //this->VolumesGUI->GetUIPanel()->SetParent ( this->ModuleControlFrame );
    this->VolumesGUI->GetUIPanel()->SetUserInterfaceManager (this->MainSlicerWin->GetMainUserInterfaceManager ( ) );
    this->VolumesGUI->GetUIPanel()->Create ( );
    this->VolumesGUI->BuildGUI ( );


    // Create the models GUI.
    this->ModelsGUI = vtkSlicerModelsGUI::New ( );
    this->ModelsGUI->SetMrml ( this->GetMrml() );
    this->ModelsGUI->SetApplication ( (vtkSlicerApplication *)this->GetApplication() );
    this->ModelsGUI->GetUIPanel()->SetName ("ModelsUI");
    //this->ModelsGUI->GetUIPanel()->SetParent ( this->ModuleControlFrame );
    this->ModelsGUI->GetUIPanel()->SetUserInterfaceManager (this->MainSlicerWin->GetMainUserInterfaceManager ( ) );
    this->ModelsGUI->GetUIPanel()->Create ( );
    this->ModelsGUI->BuildGUI ( );
    vtkKWUserInterfaceManagerNotebook *mgr = vtkKWUserInterfaceManagerNotebook::SafeDownCast (this->MainSlicerWin->GetMainUserInterfaceManager() );
    mgr->GetNotebook()->AlwaysShowTabsOff ( );
    mgr->GetNotebook()->ShowOnlyPagesWithSameTagOn ( );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildMainViewer ( ) {

    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();
        vtkKWWindow *win = this->MainSlicerWin;
        if ( this->MainViewer != NULL ) {
            this->MainViewer->SetParent (win->GetViewPanelFrame ( ) );
            this->MainViewer->Create ( );
            app->Script  ("pack %s -side top -fill both -expand y -padx 0 -pady 0",
                          this->MainViewer->GetWidgetName ( ) );
            this->MainViewer->SetRendererBackgroundColor ( style->GetViewerBgColor ( ) );

            // put a teapot in there for now.
            vtkXMLPolyDataReader *rwReader = vtkXMLPolyDataReader::New ( );
            rwReader->SetFileName ( "C:/KWWidgets/Examples/Data/teapot.vtp");
            vtkPolyDataMapper *rwMapper = vtkPolyDataMapper::New ();
            rwMapper->SetInputConnection (rwReader->GetOutputPort() );
            vtkActor *rwActor = vtkActor::New ( );
            rwActor->SetMapper ( rwMapper );
            MainViewer->AddViewProp ( rwActor );
            MainViewer->ResetCamera ( );
        
            rwReader->Delete ();
            rwActor->Delete ();
            rwMapper->Delete ();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildLogoGUIPanel ( ) {
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSlicerControlGUIPanel ( ) {
    const char* modules[] = { "Data", "Volumes", "Models" };

    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();
        this->HomeButton->SetParent ( this->SlicerControlFrame );
        this->HomeButton->Create ( );
        this->HomeButton->SetWidth ( 7 );
        this->HomeButton->SetText ( "Home" );

        this->DataButton->SetParent ( this->SlicerControlFrame );
        this->DataButton->Create ( );
        this->DataButton->SetWidth ( 7 );
        this->DataButton->SetText ("Data");

        this->VolumesButton->SetParent ( this->SlicerControlFrame );
        this->VolumesButton->Create ( );
        this->VolumesButton->SetWidth ( 7 );
        this->VolumesButton->SetText("Volumes");

        this->ModelsButton->SetParent ( this->SlicerControlFrame );
        this->ModelsButton->Create ( );
        this->ModelsButton->SetWidth ( 7 );
        this->ModelsButton->SetText("Models");

        this->ModulesButton->SetParent ( this->SlicerControlFrame );
        this->ModulesButton->Create ( );
        this->ModulesButton->SetWidth ( 10 );
        this->ModulesButton->SetValue ("Volumes");
        this->ModulesButton->IndicatorVisibilityOn ( );
        for ( int i=0; i < sizeof(modules)/sizeof(modules[0]); i++) {
            this->ModulesButton->GetMenu()->AddRadioButton( modules[i] );
        }

        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->HomeButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->DataButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->VolumesButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 1", this->ModelsButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2 -ipady 0", this->ModulesButton->GetWidgetName( ) );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSliceControlGUIPanel ( ) {
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildViewControlGUIPanel ( ) {
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
            //            this->MainSlicerWin->GetMainSplitFrame()->SetFrame1MinimumSize( this->GetDefaultSlicerWindowWidth ( ) );
            this->MainSlicerWin->GetMainSplitFrame()->SetFrame1Size (400 );
            this->MainSlicerWin->GetMainSplitFrame()->SetFrame1MinimumSize (400 );
        }
    }

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainViewerPanel ( ) {
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        if ( this->MainSlicerWin != NULL ) {
            this->MainSlicerWin->GetViewPanelFrame()->SetWidth ( this->GetDefaultMainViewerWidth() );
        }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureSliceViewersPanel ( ) {
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerStyle *style = app->GetSlicerStyle();

        this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame2Size (120);
        this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame2MinimumSize (120);
        
        if ( this->MainSlicerWin != NULL ) {
            this->MainSlicerWin->GetSecondaryPanelFrame()->SetWidth ( 3 * this->GetDefaultSliceGUIFrameWidth () );
            this->MainSlicerWin->GetSecondaryPanelFrame()->SetHeight ( this->GetDefaultSliceGUIFrameHeight () );

            // Parent and configure Slice0 frame
            this->DefaultSlice0Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice0Frame->Create ( );
            //this->DefaultSlice0Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            //this->DefaultSlice0Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );

            // Parent and configure Slice1 frame
            this->DefaultSlice1Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice1Frame->Create ( );
            //this->DefaultSlice1Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            //this->DefaultSlice1Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );

            // Parent and configure Slice2 frame
            this->DefaultSlice2Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice2Frame->Create ( );
            //this->DefaultSlice2Frame->SetHeight ( this->GetDefaultSliceGUIFrameHeight() );
            //this->DefaultSlice2Frame->SetWidth ( this->GetDefaultSliceGUIFrameWidth () );
            
            // pack them.
            app->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", this->DefaultSlice0Frame->GetWidgetName( ) );
            app->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", this->DefaultSlice1Frame->GetWidgetName( ) );
            app->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", this->DefaultSlice2Frame->GetWidgetName( ) );


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

            this->MainSlicerWin->GetMainPanelFrame()->SetWidth ( this->GetDefaultGUIPanelWidth() );
            this->MainSlicerWin->GetMainPanelFrame()->SetHeight ( this->GetDefaultGUIPanelHeight() );

            this->LogoFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->LogoFrame->Create( );
            this->LogoFrame->SetReliefToGroove ( );
            this->LogoFrame->SetHeight ( this->GetDefaultLogoFrameHeight ( ) );

            this->SlicerControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SlicerControlFrame->Create( );
            this->SlicerControlFrame->SetReliefToGroove ( );
            this->SlicerControlFrame->SetHeight ( this->GetDefaultSlicerControlFrameHeight ( ) );

            // pack logo and slicer control frames
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->LogoFrame->GetWidgetName() );
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->SlicerControlFrame->GetWidgetName() );

            // UI PANEL FRAME
            // ---
            // create and configure frames  in UI panel
            /*
            this->ModuleControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->ModuleControlFrame->Create( );
            this->ModuleControlFrame->SetHeight ( this->GetDefaultSlicerControlFrameHeight ( ) );
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->ModuleControlFrame->GetWidgetName() );
            */

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
            
            // pack slice and view control frames
            app->Script ( "pack %s -side bottom -fill x -padx 0 -pady 0", this->ViewControlFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -fill x -padx 0 -pady 0", this->SliceControlFrame->GetWidgetName() );

        }
    }

}


