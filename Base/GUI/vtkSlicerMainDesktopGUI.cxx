
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWFrame.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"
#include "vtkSlicerMainDesktopGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerMainDesktopGUI );
vtkCxxRevisionMacro ( vtkSlicerMainDesktopGUI, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerMainDesktopGUI::vtkSlicerMainDesktopGUI ( ) {
    this->myScale = NULL;
    this->myLabel = NULL;
    this->myFrame = NULL;
    this->myWindow = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMainDesktopGUI::~vtkSlicerMainDesktopGUI ( ) {

    if ( this->myScale ) {
        this->myScale->Delete ( );
    }
    if ( this->myLabel ) {
        this->myLabel->Delete ( );
    }
    if ( this->myWindow ) {
        this->myWindow->Close ( );
        this->myWindow->Delete ( );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::MakeWindow ( ) {

    // Make a window
    if ( !this->myWindow ) {
        this->myWindow = vtkKWWindow::New ( );
    }
    this->SetWindow ( this->myWindow );
    this->myWindow->SecondaryPanelVisibilityOff ( );
    this->myWindow->MainPanelVisibilityOff ( );
    // Add new window to the application
    this->SlicerApplication->AddWindow ( this->myWindow );
    this->myWindow->Create ( );
}


//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::MakeWidgets ( ) {

    // make simple scale widget for now.
    this->myFrame = this->myWindow->GetViewFrame ( );
    if ( !this->myScale ) {
        this->myScale = vtkKWScale::New ( );
    }
    this->myScale->SetParent ( this->myFrame );
    this->myScale->Create ( );
    this->myScale->SetResolution (0.5);
    this->myScale->SetValue ( 0.0 );

    if ( !this->myLabel ) {
        this->myLabel = vtkKWLabel::New ( );
    }
    this->myLabel->SetParent (this->myFrame );
    this->myLabel->Create ( );
    char str[256];
    //sprintf (str, "logic state=%lf", this->Logic->GetMyState ( ) );
    sprintf (str, "logic state=unknown");
    this->myLabel->SetText ( str );

    // pack in the window's view frame
    vtkKWApplication *kwapp = this->GetKWApplication ( );
    kwapp->Script("pack %s -side left -anchor c ", this->myScale->GetWidgetName ( ) );
    kwapp->Script("pack %s -side left -anchor c ", this->myLabel->GetWidgetName ( ) );

    // why do we delete like this?
    //this->myScale->Delete ( );
    //this->myLabel->Delete ( );


}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::AddGUIObservers ( ) {
    
    this->AddCallbackCommandObserver (this->myScale, vtkKWScale::ScaleValueChangingEvent );
}




//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::AddLogicObservers ( ) {

    this->Logic->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->LogicCommand );
}





//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::ProcessCallbackCommandEvents ( vtkObject *caller,
                                                             unsigned long event,
                                                             void *callData )
{
    // process GUI events.
    vtkKWScale *scalewidget = vtkKWScale::SafeDownCast(caller);
    if (caller == scalewidget && event == vtkKWScale::ScaleValueChangingEvent )
        {
#if 0
            if (  this->Logic->GetMyState( ) != scalewidget->GetValue( ) ) {
                this->Logic->SetMyState ( scalewidget->GetValue() );
                this->Logic->Modified( );
            }
#endif
        }
    // always do this?
    this->Superclass::ProcessCallbackCommandEvents ( caller, event, callData );
}




//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::ProcessLogicEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) 
{
    // process Logic changes
    if (caller == static_cast <vtkObject *> (this->Logic) && event == vtkCommand::ModifiedEvent ) {
        char str[256];
#if 0
        sprintf (str, "logic state=%lf", this->Logic->GetMyState ( ) );
        this->myLabel->SetText ( str );
        if ( this->myScale->GetValue ( )  != this->Logic->GetMyState( ) ) {
            this->myScale->SetValue ( this->Logic->GetMyState ( ) );
        }
#endif

    }
}





//---------------------------------------------------------------------------
int vtkSlicerMainDesktopGUI::BuildGUI ( vtkSlicerApplicationGUI *app ) {

    // Assign the GUI to an existing vtkSlicerApplicationGUI
    this->SetSlicerApplication ( app );
    
    // Create and pack all stuff here. Define and
    // call new methods if necessary.
    this->MakeWindow ( );

    // Make a scale in the window.
    this->MakeWidgets ( );

    // Add the new GUI to the application's collection of GUIs.
    this->SlicerApplication->AddGUI ( this );

    // Display the window.
    this->myWindow->Display ( );

    // Should test here to see if the gui is created.

    return 0;

}




