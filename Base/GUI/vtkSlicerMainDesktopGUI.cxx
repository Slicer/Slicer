
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWindow.h"
#include "vtkKWScale.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScale.h"
#include "vtkKWWindow.h"
#include "vtkKWWindowLevelPresetSelector.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkKWWidgetsPaths.h"
#include "vtkToolkits.h"


#include "vtkSlicerMainDesktopGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerMainDesktopGUI );
vtkCxxRevisionMacro ( vtkSlicerMainDesktopGUI, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerMainDesktopGUI::vtkSlicerMainDesktopGUI ( ) {

    this->Scale = NULL;
    this->Window = NULL;
    this->FileBrowseButton = NULL;
    this->FileBrowse = NULL;
    this->ImageViewer = NULL;
    this->WindowLevelPresetSelector = NULL;
    this->RenderWidget = NULL;
    this->Frame = NULL;
    this->Window = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMainDesktopGUI::~vtkSlicerMainDesktopGUI ( ) {

    if ( this->Scale ) {
        this->Scale->Delete ( );
    }
    if (this->FileBrowse ) {
        this->FileBrowse->Delete ( );
    }
    if (this->FileBrowseButton ) {
        this->FileBrowseButton->Delete ( );
    }
    if (this->ImageViewer) {
        this->ImageViewer->Delete ( );
    }
    if (this->WindowLevelPresetSelector ) {
        this->WindowLevelPresetSelector->Delete ( );
    }
    if (this->RenderWidget ) {
        this->RenderWidget->Delete ( );
    }
    if ( this->Window ) {
        this->Window->Close ( );
        this->Window->Delete ( );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::MakeWindow ( ) {

    // Make a window
    if ( !this->Window ) {
        this->Window = vtkKWWindow::New ( );
    }
    this->SetWindow ( this->Window );
    this->Window->SecondaryPanelVisibilityOff ( );
    this->Window->MainPanelVisibilityOff ( );
    // Add new window to the application
    this->SlicerApplication->AddWindow ( this->Window );
    this->Window->Create ( );
}


//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::MakeWidgets ( ) {

    // make simple scale widget for now.
    this->Frame = this->Window->GetViewFrame ( );

    // add a file browse.
    this->FileBrowseButton = vtkKWLoadSaveButtonWithLabel::New ( );
    this->FileBrowseButton->SetParent (this->Frame );
    this->FileBrowseButton->Create ( );
    this->FileBrowseButton->SetText ( "Choose a file to load");
    this->FileBrowseButton->GetLoadSaveDialog ( )->SaveDialogOff ( );

    // and add a render widget
    this->RenderWidget = vtkKWRenderWidget::New();
    this->RenderWidget->SetParent(win->GetViewFrame());
    this->RenderWidget->Create();
    this->RenderWidget->CornerAnnotationVisibilityOn();
    this->RenderWidget->SetParent ( this->Frame );
    
    // Create an image viewer
    // Use the render window and renderer of the renderwidget
    this->ImageViewer = vtkImageViewer2::New();
    this->ImageViewer->SetRenderWindow(this->RenderWidget->GetRenderWindow());
    this->ImageViewer->SetRenderer(this->RenderWidget->GetRenderer());
    this->ImageViewer->SetInput(reader->GetOutput());
    this->ImageViewer->SetupInteractor(
    this->RenderWidget->GetRenderWindow()->GetInteractor());

    // Turn on annotations
    vtkCornerAnnotation *ca = this->RenderWidget->GetCornerAnnotation();
    ca->SetImageActor(this->ImageViewer->GetImageActor());
    ca->SetWindowLevel(this->ImageViewer->GetWindowLevel());
    ca->SetText(2, "<slice>");
    ca->SetText(3, "<window>\n<level>");

    // Create a scale to control the slice
    this->Scale = vtkKWScale::New();
    this->Scale->SetParent(win->GetViewPanelFrame());
    this->Scale->Create();
    this->Scale->SetCommand(this, "SetSliceFromScaleCallback");

    // Pack in the window's view frame
    vtkKWApplication *kwapp = this->GetKWApplication ( );
    kwapp->Script("pack %s -expand n -side top -anchor c -padx 2 -pady 2", 
              this->FileBrowseButton->GetWidgetName());
    kwapp->Script("pack %s -expand y -fill both -anchor c -expand y", 
              this->RenderWidget->GetWidgetName());
    kwapp->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2", 
              this->Scale->GetWidgetName());


}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::AddGUIObservers ( ) {
    
    this->AddCallbackCommandObserver (this->Scale, vtkKWScale::ScaleValueChangingEvent );
    this->AddCallbackCommandObserver (this->FileBrowseButton, vtkKWLoadSaveButtonWithLabel::ModifiedEvent);
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
    vtkKWLoadSaveButtonWithLabel *filebrowse = vtkKWLoadSaveButtonWithLabel::SafeDownCast(caller);
    if (caller == scalewidget && event == vtkKWScale::ScaleValueChangingEvent )
        {
            // set the current slice.
#if 0
            if (  this->Logic->GetState( ) != scalewidget->GetValue( ) ) {
                this->Logic->SetState ( scalewidget->GetValue() );
                this->Logic->Modified( );
            }
#endif
        }
    elseif (caller == filebrowse && event = vtkKWLoadSaveButtonWithLabel::ModifiedEvent )
        {
            // set the reader's input.
            this->ReaderLogic->Connect ( filebrowse->GetFileName ( ) );
            // configure the slider's range to match the number of slices.
            // configure call an update on
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

        // if mrml has changed, get info about the new data (num slices, etc.)
        // display stuff in the image and
        // update the range of the scale.
#if 0
        sprintf (str, "logic state=%lf", this->Logic->GetState ( ) );
        this->Label->SetText ( str );
        if ( this->Scale->GetValue ( )  != this->Logic->GetState( ) ) {
            this->Scale->SetValue ( this->Logic->GetState ( ) );
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
    this->Window->Display ( );

    // Should test here to see if the gui is created.

    return 0;

}




