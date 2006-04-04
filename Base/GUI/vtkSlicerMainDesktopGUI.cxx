
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
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkToolkits.h"
#include "vtkSlicerMainDesktopGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerLogic.h"
#include "vtkMRMLVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerMainDesktopGUI );
vtkCxxRevisionMacro ( vtkSlicerMainDesktopGUI, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerMainDesktopGUI::vtkSlicerMainDesktopGUI ( ) {

    this->Scale = NULL;
    this->Window = NULL;
    this->FileBrowseButton = NULL;
    //this->FileBrowse = NULL;
    this->ImageViewer = NULL;
    this->WindowLevelPresetSelector = NULL;
    this->RenderWidget = NULL;
    this->Window = NULL;

    this->ApplicationLogic = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMainDesktopGUI::~vtkSlicerMainDesktopGUI ( ) {

    if ( this->Scale ) {
        this->Scale->Delete ( );
    }
    //if (this->FileBrowse ) {
     //   this->FileBrowse->Delete ( );
    //}
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
    this->SetApplicationLogic(NULL);

}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::MakeWindow ( ) {

    // Make slicer's main window on the desktop
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

    // for starters, the goal is to make a simple file browse,
    // image viewer and slice slider in the ViewFrame.
    
    // add a LOAD file browser (comes with a popup file browser/selector)
    this->FileBrowseButton = vtkKWLoadSaveButton::New ( );
    this->FileBrowseButton->SetParent (this->Window->GetViewFrame ( ) );
    this->FileBrowseButton->Create ( );
    this->FileBrowseButton->SetText ("Choose a file to load");
    //this->FileBrowseButton->GetLoadSaveDialog()->SaveDialogOff ( );

    // Should be in TK format. Default is: "{{Text Document} {.txt}}"
    this->FileBrowseButton->GetLoadSaveDialog()->SetFileTypes(
        "{ {MRML Document} {.mrml .xml} }");

    // and add a render widget
    this->RenderWidget = vtkKWRenderWidget::New();
    this->RenderWidget->SetParent ( this->Window->GetViewFrame ( ) );
    this->RenderWidget->Create();
    this->RenderWidget->CornerAnnotationVisibilityOn();
    
    // Create an image viewer
    // Use the render window and renderer of the renderwidget
    this->ImageViewer = vtkImageViewer2::New();
    this->ImageViewer->SetRenderWindow(this->RenderWidget->GetRenderWindow());
    this->ImageViewer->SetRenderer(this->RenderWidget->GetRenderer());
    this->ImageViewer->SetupInteractor( this->RenderWidget->GetRenderWindow()->GetInteractor());

    // Create a scale to control the slice number displayed
    this->Scale = vtkKWScale::New();
    this->Scale->SetParent(this->Window->GetViewFrame ( ) );
    this->Scale->Create();

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
    
    // original way; using kwobject command observers...
    //this->AddCallbackCommandObserver (this->Scale, vtkKWScale::ScaleValueChangingEvent );
    //this->AddCallbackCommandObserver (this->FileBrowseButton, vtkKWLoadSaveButton::ModifiedEvent);

    // try a parallel implementation, using vtkObject observers...
    this->FileBrowseButton->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICommand );
    this->Scale->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );

}




//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::AddLogicObservers ( ) {

    this->ApplicationLogic->GetMRMLScene()->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->LogicCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::UpdateLogicWithGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) 
{

    // Try processing GUI events the vtkObject way.
    // One of two things can happen here...
    // 1. the filebrowsebutton has selected data to be read (or cancelled), or
    // 2. the slider is being dragged to cause the slice image to update.
    //
    vtkKWScale *scalewidget = vtkKWScale::SafeDownCast(caller);
    vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);
    //this->FileBrowseButton->GetLoadSaveDialog ( )

    if (scalewidget == this->Scale && event == vtkCommand::ModifiedEvent )
        {
            // Set the current slice in the image viewer.
            if ( this->ImageViewer->GetSlice ( ) != this->Scale->GetValue ( ) ) {
                this->ImageViewer->SetSlice((int) this->Scale->GetValue ( ) );
            }
        }
    else if (filebrowse == this->FileBrowseButton  && event == vtkCommand::ModifiedEvent )
        {
            // If a file has been selected for loading...
            if ( this->FileBrowseButton->GetFileName ( ) ) {
                this->ApplicationLogic->Connect ( filebrowse->GetFileName ( ) );
            }
        }

    // hmmm. what if we don't do this?
    // this->Superclass::ProcessCallbackCommandEvents ( caller, event, callData );

}



//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::UpdateGUIWithLogicEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) 
{

    // process Logic changes
    // In this example, only thing that will come from the logic is new image data.
    vtkMRMLScene *mrml = vtkMRMLScene::SafeDownCast(caller);
    
    if (mrml == (this->ApplicationLogic->GetMRMLScene ( ) ) && event == vtkCommand::ModifiedEvent )
        {
            // If the MRML scene has changed, get the 0th volume node.
            // and set that as input into the ImageViewer.
            vtkMRMLVolumeNode* volumenode = vtkMRMLVolumeNode::SafeDownCast (this->ApplicationLogic->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLVolumeNode" ) );
            if ( volumenode )
              {
              this->ImageViewer->SetInput ( volumenode->GetImageData( ) );
              double *range = volumenode->GetImageData()->GetScalarRange ( );
              this->ImageViewer->SetColorWindow ( range [1] - range [0] );
              this->ImageViewer->SetColorLevel (0.5 * (range [1] - range [0] ));
              }
            else
              {
              this->ImageViewer->SetInput (NULL);
              }
            this->ImageViewer->Render();

            // configure window, level, camera, etc.
            this->RenderWidget->ResetCamera ( );
            vtkCornerAnnotation *ca = this->RenderWidget->GetCornerAnnotation ( );
            ca->SetImageActor (this->ImageViewer->GetImageActor ( ) );
            ca->SetWindowLevel (this->ImageViewer->GetWindowLevel ( ) );

            // set the range of the slider
            this->Scale->SetRange ( this->ImageViewer->GetSliceMin ( ), this->ImageViewer->GetSliceMax ( ) );
            this->Scale->SetValue (this->ImageViewer->GetSlice ( ) );
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




//---------------------------------------------------------------------------
void vtkSlicerMainDesktopGUI::ProcessCallbackCommandEvents ( vtkObject *caller,
                                                             unsigned long event,
                                                             void *callData )
{
    // process GUI events.
    vtkKWScale *scalewidget = vtkKWScale::SafeDownCast(caller);
    vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);

    if (caller == scalewidget && event == vtkKWScale::ScaleValueChangingEvent )
        {
        }
    else if (caller == filebrowse /* && event = vtkKWLoadSaveButtonWithLabel::ModifiedEvent */ )
        {
        }

    // always do this?
    this->Superclass::ProcessCallbackCommandEvents ( caller, event, callData );
}






