#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerStyle.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumesGUI );
vtkCxxRevisionMacro ( vtkSlicerVolumesGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::vtkSlicerVolumesGUI ( ) {

    //this->SetLogic ( NULL );
    this->LoadVolumeButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::~vtkSlicerVolumesGUI ( ) {

    if (this->LoadVolumeButton ) {
        this->LoadVolumeButton->Delete ( );
        this->LoadVolumeButton = NULL;
    }
    //this->SetLogic ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveGUIObservers ( ) {

    this->LoadVolumeButton->RemoveObservers ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICommand);    
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddGUIObservers ( ) {

    // observer load volume button
    this->LoadVolumeButton->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICommand );    
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddLogicObserver ( vtkSlicerModuleLogic *logic, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveLogicObserver ( vtkSlicerModuleLogic *logic, int event ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddLogicObservers ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveLogicObservers ( ) {
    // Fill in
}
 
//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddMRMLObserver ( vtkMRMLNode *node, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveMRMLObserver ( vtkMRMLNode *node, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddMRMLObservers ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveMRMLObservers ( ) {
    // Fill in
}


    /*
//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::SetLogic ( vtkSlicerVolumesLogic *logic ) {

    // Don't bother if already set.
    if ( logic == this->Logic ) {
        return;
    }
    // Remove observers from application logic
    if ( this->Logic != NULL ) {
        this->RemoveLogicObservers ( );
    }
    // Set pointer and add observers if not null
    this->Logic = logic;
    if ( this->Logic != NULL ) {
        this->AddLogicObservers ( );
    }
}
     */


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessGUIEvents ( vtkObject *caller,
                                                     unsigned long event,
                                                     void *callData ) 
{
    vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);
    if (filebrowse == this->LoadVolumeButton  && event == vtkCommand::ModifiedEvent )
        {
            // If a file has been selected for loading...
            if ( filebrowse->GetFileName ( ) ) {
                this->ApplicationLogic->Connect ( filebrowse->GetFileName ( ) );
            }
        }
}



//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessLogicEvents ( vtkObject *caller,
                                                     unsigned long event,
                                                     void *callData ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event,
                                              void *callData ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::Enter ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::Exit ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::BuildGUI ( ) {

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerStyle *style = app->GetSlicerStyle();
    // ---
    // MODULE GUI FRAME 
    // configure a page for a volume loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Volumes", "Volumes", NULL );
    
    // HELP FRAME
    vtkKWFrameWithLabel *volHelpFrame = vtkKWFrameWithLabel::New ( );
    volHelpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Volumes" ) );
    volHelpFrame->Create ( );
    volHelpFrame->CollapseFrame ( );
    volHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  volHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Volumes")->GetWidgetName());

    // ---
    // LOAD FRAME            
    vtkKWFrameWithLabel *volLoadFrame = vtkKWFrameWithLabel::New ( );
    volLoadFrame->SetParent ( this->UIPanel->GetPageWidget ( "Volumes" ) );
    volLoadFrame->Create ( );
    volLoadFrame->SetLabelText ("Load");
    volLoadFrame->ExpandFrame ( );

    // add a file browser 
    this->LoadVolumeButton = vtkKWLoadSaveButton::New ( );
    this->LoadVolumeButton->SetParent ( volLoadFrame->GetFrame() );
    this->LoadVolumeButton->Create ( );
    this->LoadVolumeButton->SetText ("Choose a file to load");
    this->LoadVolumeButton->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {MRML Document} {.mrml .xml} }");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  volLoadFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Volumes")->GetWidgetName());
    app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->LoadVolumeButton->GetWidgetName());

    // ---
    // DISPLAY FRAME            
    vtkKWFrameWithLabel *volDisplayFrame = vtkKWFrameWithLabel::New ( );
    volDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Volumes" ) );
    volDisplayFrame->Create ( );
    volDisplayFrame->SetLabelText ("Display");
    volDisplayFrame->SetDefaultLabelFontWeightToNormal( );
    volDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  volDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Volumes")->GetWidgetName());


    volLoadFrame->Delete();
    volHelpFrame->Delete();
    volDisplayFrame->Delete();
}





