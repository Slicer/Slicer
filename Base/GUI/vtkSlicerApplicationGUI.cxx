/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#include <sstream>
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkObjectFactory.h"
#include "vtkToolkits.h"
// things for temporary MainViewer teapot display.
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkXMLPolyDataReader.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWWidget.h"
#include "vtkKWWindow.h"

#include "vtkKWSplitFrame.h"
#include "vtkKWUserInterfaceManagerNotebook.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerStyle.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  )
{
    //---  
    // widgets used in the Slice module
    this->MainSlicerWin = vtkKWWindow::New ( );
    
    // Control frames that comprise the Main Slicer GUI
    this->LogoFrame = vtkKWFrame::New();
    this->SlicerControlFrame = vtkKWFrame::New();
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
    this->AlignmentsButton = vtkKWPushButton::New();
    
    this->ModulesMenuButton = vtkKWMenuButton::New();
    this->ModulesLabel = vtkKWLabel::New();
    this->ModulesBack = vtkKWPushButton::New ( );
    this->ModulesNext = vtkKWPushButton::New ( );

    this->MainViewer = vtkKWRenderWidget::New ( );
    
}



//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( )
{

    this->RemoveGUIObservers ( );

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
void vtkSlicerApplicationGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerApplicationGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "MainViewer: " << this->GetMainViewer ( ) << "\n";
    os << indent << "MainSlicerWin: " << this->GetMainSlicerWin ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( )
{

    // add observers onto the buttons and menubutton in the SlicerControl frame
    this->HomeButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DataButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VolumesButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelsButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesMenuButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveGUIObservers ( )
{
    this->HomeButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DataButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VolumesButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelsButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesMenuButton->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessGUIEvents ( vtkObject *caller,
                                                 unsigned long event, void *callData )
{
    
    // This code is just a placeholder until the logic is set up to use properly:
    // For now, the GUI controls the GUI instead of going thru the logic...
    // TODO:
    // Actually, these events want to set "activeModule" in the logic;
    // using this->Logic->SetActiveModule ( ) which is currently commented out.
    // Observers on that logic should raise and lower the appropriate page.
    // So for now, the GUI is controlling the GUI instead of going thru the logic.
    //---
    vtkSlicerModuleGUI * m;
    const char *mName;
    vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
    vtkKWMenuButton *menub = vtkKWMenuButton::SafeDownCast (caller );
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        
    // Process events from top row of buttons
    // For now, Home button takes us to the Volumes module.
    if ( pushb == this->HomeButton && event == vtkKWPushButton::InvokedEvent ) {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Volumes" );
    }
    else if (pushb == this->DataButton && event == vtkKWPushButton::InvokedEvent ) {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Data");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Data" );
    }
    else if (pushb == this->VolumesButton && event == vtkKWPushButton::InvokedEvent ) {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Volumes" );
    }
    else if (pushb == this->ModelsButton && event == vtkKWPushButton::InvokedEvent ) {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Models");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Models" );
    }
    else if (pushb == this->AlignmentsButton && event == vtkKWPushButton::InvokedEvent ) {
        //vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Alignments");
        //if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Alignments" );
    }

    //--- Process events from menubutton
    //--- TODO: change the Logic's "active module" and raise the appropriate UIPanel.
    if ( menub == this->ModulesMenuButton && event == vtkCommand::ModifiedEvent )
        {
            if ( app->GetModuleGUICollection ( ) != NULL )
                {
                    app->GetModuleGUICollection( )->InitTraversal( );
                    m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
                    while (m != NULL )
                        {
                            mName = m->GetUIPanel()->GetName();
                            if ( !strcmp (this->ModulesMenuButton->GetValue(), mName) ) {
                                m->GetUIPanel()->Raise();
                                break;
                            }
                            m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
                        }
                    //this->ModulesMenuButton->SetValue ( "Modules" );
                }
        }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessLogicEvents ( vtkObject *caller,
                                                   unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Enter ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Exit ( )
{
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUI ( )
{
    int i;
    
    // Set up the conventional window: 3Dviewer, slice widgets, UI panel for now.
    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

        app->GetMainLayout()->InitializeLayout ( );

        if ( this->MainSlicerWin != NULL ) {

            // set up Slicer's main window
            this->MainSlicerWin->SecondaryPanelVisibilityOn ( );
            this->MainSlicerWin->MainPanelVisibilityOn ( );
            app->AddWindow ( this->MainSlicerWin );
            this->MainSlicerWin->Create ( );        

            // Construct menu bar and set up global key bindings

            // TODO: Hook up undo, redo, set home
            //i = this->MainSlicerWin->GetFileMenu()->AddCommand (  );
            i = this->MainSlicerWin->GetEditMenu()->AddCommand ("Set Home", NULL, NULL);
            this->MainSlicerWin->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+H");
            i = this->MainSlicerWin->GetEditMenu()->AddCommand ( "Undo", NULL, NULL );
            this->MainSlicerWin->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+Z");
            i = this->MainSlicerWin->GetEditMenu()->AddCommand ( "Redo", NULL, NULL );
            this->MainSlicerWin->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+Y");
            //i = this->MainSlicerWin->GetViewMenu()->AddCommand ( ? );
            //i = this->MainSlicerWin->GetWindowMenu()->AddCommand ( ? );
            //i = this->MainSlicerWin->GetHelpMenu()->AddCommand ( ? );

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
            //            this->BuildModuleControlGUIPanel ( );

            // Turn off the tabs for pages in the ModuleControlGUI
            this->MainSlicerWin->GetMainNotebook( )->ShowIconsOff ( );
            this->BuildSliceControlGUIPanel ( );
            this->BuildViewControlGUIPanel ( );
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayMainSlicerWindow ( )
{

    this->MainSlicerWin->Display ( );
}






//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteGUIPanelWidgets ( )
{
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
    if ( this->AlignmentsButton ) {
        this->AlignmentsButton->Delete ( );
        this->AlignmentsButton = NULL;
    }
    if ( this->ModulesMenuButton ) {
        this->ModulesMenuButton->Delete();
        this->ModulesMenuButton = NULL;
    }
    if ( this->ModulesLabel ) {
        this->ModulesLabel->Delete ( );
        this->ModulesLabel = NULL;
    }
    if ( this->ModulesBack ) {
        this->ModulesBack->Delete ( );
        this->ModulesBack = NULL;
    }
    if ( this->ModulesNext ) {
        this->ModulesNext->Delete ( );
        this->ModulesNext = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteFrames ( )
{
    if ( this->LogoFrame ) {
        this->LogoFrame->Delete ();
        this->LogoFrame = NULL;
    }
    if ( this->SlicerControlFrame ) {
        this->SlicerControlFrame->Delete ();
        this->SlicerControlFrame = NULL;
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
void vtkSlicerApplicationGUI::BuildMainViewer ( )
{

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
void vtkSlicerApplicationGUI::BuildLogoGUIPanel ( )
{
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSlicerControlGUIPanel ( )
{
    const char* mName;
    vtkSlicerModuleGUI *m;
    
    if ( this->GetApplication( )  != NULL ) {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        vtkKWFrame *f1 = vtkKWFrame::New ( );
        vtkKWFrame *f2 = vtkKWFrame::New ( );

        f1->SetParent (this->SlicerControlFrame );
        f1->Create ( );
        f2->SetParent (this->SlicerControlFrame );
        f2->Create ( );
        
        //--- home button
        this->HomeButton->SetParent ( f1 );
        this->HomeButton->Create ( );
        this->HomeButton->SetWidth ( 9 );
        this->HomeButton->SetText ( "Home" );
        this->HomeButton->SetBalloonHelpString ("Use this button to go to your Home module.");
        //--- data module
        this->DataButton->SetParent ( f1 );
        this->DataButton->Create ( );
        this->DataButton->SetWidth ( 9 );
        this->DataButton->SetText ("Data");
        this->DataButton->SetBalloonHelpString ("Use this button to go to the Data module.");
        //--- volumes module
        this->VolumesButton->SetParent ( f1 );
        this->VolumesButton->Create ( );
        this->VolumesButton->SetWidth ( 9 );
        this->VolumesButton->SetText("Volumes");
        this->VolumesButton->SetBalloonHelpString ("Use this button to go to the Volumes module.");
        // --- models module
        this->ModelsButton->SetParent ( f1 );
        this->ModelsButton->Create ( );
        this->ModelsButton->SetWidth ( 9 );
        this->ModelsButton->SetText("Models");
        this->ModelsButton->SetBalloonHelpString ("Use this button to go to the Models module.");
        // --- alignments module
        this->AlignmentsButton->SetParent ( f1 );
        this->AlignmentsButton->Create ( );
        this->AlignmentsButton->SetWidth ( 9 );
        this->AlignmentsButton->SetText("Alignments");
        this->AlignmentsButton->SetBalloonHelpString ("Use this button to go to the Alignments module.");
        
        //--- ALL modules menu button label
        this->ModulesLabel->SetParent ( f2 );
        this->ModulesLabel->Create ( );
        this->ModulesLabel->SetText ( "    Modules:");
        this->ModulesLabel->SetWidth ( 9 );

        //--- All modules menu button
        this->ModulesMenuButton->SetParent ( f2 );
        this->ModulesMenuButton->Create ( );
        this->ModulesMenuButton->SetWidth ( 28 );
        this->ModulesMenuButton->IndicatorVisibilityOn ( );
        this->ModulesMenuButton->SetBalloonHelpString ("Use this pull-down menu to select a Slicer module.");
        //--- ALL modules pull-down menu 
        if ( app->GetModuleGUICollection ( ) != NULL ) {
            app->GetModuleGUICollection( )->InitTraversal( );
            m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
            while ( m != NULL ) {
                mName = m->GetUIPanel( )->GetName( );
                this->ModulesMenuButton->GetMenu( )->AddRadioButton( mName );
                m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
            }
        }
        //--- TODO: make the initial value be module user sets as "home"
        this->ModulesMenuButton->SetValue ("Volumes");
        
        //--- Next and previous module button
        this->ModulesNext->SetParent ( f2 );
        this->ModulesNext->Create ( );
        this->ModulesNext->SetText ( " > " );
        this->ModulesNext->SetAnchorToCenter ( );
        this->ModulesNext->SetWidth ( 2 );
        this->ModulesNext->SetBalloonHelpString ("Use this button to navigate to the next module in your use history.");
        this->ModulesBack->SetParent ( f2 );
        this->ModulesBack->Create ( );
        this->ModulesBack->SetText ( " < " );
        this->ModulesBack->SetAnchorToCenter ( );
        this->ModulesBack->SetWidth ( 2 );
        this->ModulesBack->SetBalloonHelpString ("Use this button to navigate to the previous module in your use history.");
        
        //--- pack everything up.
        app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0", f1->GetWidgetName( ) );
        app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0", f2->GetWidgetName( ) );
        
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2", this->HomeButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2", this->DataButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipadx 1 -pady 2", this->VolumesButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -pady 2", this->ModelsButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipadx 1 -pady 2", this->AlignmentsButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipadx 1 -pady 2", this->ModulesLabel->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipadx 1 -pady 2", this->ModulesMenuButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 2 -ipady 2 -pady 2", this->ModulesBack->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipady 2 -pady 2", this->ModulesNext->GetWidgetName( ) );

        f1->Delete ( );
        f2->Delete ( );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSliceControlGUIPanel ( )
{
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildViewControlGUIPanel ( )
{
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainSlicerWindow ( )
{

    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
        if ( this->MainSlicerWin != NULL ) {
            this->MainSlicerWin->MainPanelVisibilityOn ();
            this->MainSlicerWin->SecondaryPanelVisibilityOn ();
            this->MainSlicerWin->SetSize ( app->GetMainLayout()->GetDefaultSlicerWindowWidth ( ),
                           app->GetMainLayout()->GetDefaultSlicerWindowHeight () );
            //            this->MainSlicerWin->GetMainSplitFrame()->SetFrame1MinimumSize( this->GetDefaultSlicerWindowWidth ( ) );
            // Configure the minimum width of Slicer's GUI panel.
            // Panel can be expanded and collapsed entirely, but
            // can't be resized by hand to a value smaller than what's set.
            this->MainSlicerWin->GetMainSplitFrame()->SetFrame1Size (325 );
            this->MainSlicerWin->GetMainSplitFrame()->SetFrame1MinimumSize (325 );
        }
    }

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainViewerPanel ( )
{
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );


        if ( this->MainSlicerWin != NULL ) {
            this->MainSlicerWin->GetViewPanelFrame()->SetWidth ( app->GetMainLayout()->GetDefaultMainViewerWidth() );
        }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureSliceViewersPanel ( )
{
    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );

        this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame2Size (120);
        this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame2MinimumSize (120);
        
        if ( this->MainSlicerWin != NULL ) {
            this->MainSlicerWin->GetSecondaryPanelFrame()->SetWidth ( 3 * app->GetMainLayout()->GetDefaultSliceGUIFrameWidth () );
            this->MainSlicerWin->GetSecondaryPanelFrame()->SetHeight ( app->GetMainLayout()->GetDefaultSliceGUIFrameHeight () );

            // Parent and configure Slice0 frame
            this->DefaultSlice0Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice0Frame->Create ( );

            // Parent and configure Slice1 frame
            this->DefaultSlice1Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice1Frame->Create ( );

            // Parent and configure Slice2 frame
            this->DefaultSlice2Frame->SetParent ( this->MainSlicerWin->GetSecondaryPanelFrame ( ) );
            this->DefaultSlice2Frame->Create ( );
            
            // pack them.
            app->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", this->DefaultSlice0Frame->GetWidgetName( ) );
            app->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", this->DefaultSlice1Frame->GetWidgetName( ) );
            app->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", this->DefaultSlice2Frame->GetWidgetName( ) );


        }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureGUIPanel ( )
{

    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );

        if ( this->MainSlicerWin != NULL ) {

            this->MainSlicerWin->GetMainPanelFrame()->SetWidth ( app->GetMainLayout()->GetDefaultGUIPanelWidth() );
            this->MainSlicerWin->GetMainPanelFrame()->SetHeight ( app->GetMainLayout()->GetDefaultGUIPanelHeight() );

            this->LogoFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->LogoFrame->Create( );
            this->LogoFrame->SetReliefToGroove ( );
            this->LogoFrame->SetHeight ( app->GetMainLayout()->GetDefaultLogoFrameHeight ( ) );

            this->SlicerControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SlicerControlFrame->Create( );
            this->SlicerControlFrame->SetReliefToGroove ( );
            this->SlicerControlFrame->SetHeight ( app->GetMainLayout()->GetDefaultSlicerControlFrameHeight ( ) );

            // pack logo and slicer control frames
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->LogoFrame->GetWidgetName() );
            app->Script ( "pack %s -side top -fill x -padx 0 -pady 0", this->SlicerControlFrame->GetWidgetName() );

            this->SliceControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SliceControlFrame->Create( );
            this->SliceControlFrame->SetReliefToGroove ( );
            this->SliceControlFrame->SetHeight ( app->GetMainLayout()->GetDefaultSliceControlFrameHeight ( ) );
            
            this->ViewControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->ViewControlFrame->Create( );
            this->ViewControlFrame->SetReliefToGroove ( );
            this->ViewControlFrame->SetHeight ( app->GetMainLayout()->GetDefaultViewControlFrameHeight ( ) );
            
            // pack slice and view control frames
            app->Script ( "pack %s -side bottom -fill x -padx 0 -pady 0", this->ViewControlFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -fill x -padx 0 -pady 0", this->SliceControlFrame->GetWidgetName() );

        }
    }

}


