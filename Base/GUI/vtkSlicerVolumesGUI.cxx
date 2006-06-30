#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumesGUI );
vtkCxxRevisionMacro ( vtkSlicerVolumesGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::vtkSlicerVolumesGUI ( )
{

    this->Logic = NULL;
    this->VolumeNode = NULL;
    this->SelectedVolumeID = NULL;

    this->LoadVolumeButton = NULL;
    this->VolumeDisplayWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::~vtkSlicerVolumesGUI ( )
{

    
  if (this->SelectedVolumeID)
    {
        delete [] this->SelectedVolumeID;
        this->SelectedVolumeID = NULL;
    }
    if (this->LoadVolumeButton )
      {
          this->LoadVolumeButton->Delete ( );
      }

    if (this->VolumeDisplayWidget)
      {
          this->VolumeDisplayWidget->RemoveWidgetObservers ( );
          this->VolumeDisplayWidget->Delete ( );
      }

    this->SetModuleLogic ( NULL );
    this->SetMRMLNode ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerVolumesGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "VolumeNode: " << this->GetVolumeNode ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveGUIObservers ( )
{
    // Fill in
    this->LoadVolumeButton->RemoveObservers ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddGUIObservers ( )
{

    // Fill in
    // observer load volume button
    this->LoadVolumeButton->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessGUIEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

    // Find the widget, and process the events from it...
    vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);
    if (filebrowse == this->LoadVolumeButton  && event == vtkCommand::ModifiedEvent )
      {
        // If a file has been selected for loading...
        char *fileName = filebrowse->GetFileName();
        if ( fileName ) 
          {
             vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      
             vtkMRMLVolumeNode *volumeNode = volumeLogic->AddArchetypeVolume( fileName );

             if ( volumeNode == NULL ) 
               {
               // TODO: generate an error...
               }
             else
               {
               filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");

               this->ApplicationLogic->GetSelectionNode()->SetActiveVolumeID( volumeNode->GetID() );
               this->ApplicationLogic->PropagateVolumeSelection();
               this->VolumeDisplayWidget->SetVolumeNode(volumeNode);               
             }
           }
           return;
        }
} 



//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessLogicEvents ( vtkObject *caller,
                                               unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::BuildGUI ( )
{
    // Fill in *placeholder GUI*
    
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a volume loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Volumes", "Volumes", NULL );
    
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Volumes" );

    // HELP FRAME
    vtkKWFrameWithLabel *volHelpFrame = vtkKWFrameWithLabel::New ( );
    volHelpFrame->SetParent ( page );
    volHelpFrame->Create ( );
    volHelpFrame->CollapseFrame ( );
    volHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  volHelpFrame->GetWidgetName(), page->GetWidgetName());

    // ---
    // LOAD FRAME            
    vtkKWFrameWithLabel *volLoadFrame = vtkKWFrameWithLabel::New ( );
    volLoadFrame->SetParent ( page );
    volLoadFrame->Create ( );
    volLoadFrame->SetLabelText ("Load");
    volLoadFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  volLoadFrame->GetWidgetName(), page->GetWidgetName());
    // add a file browser 
    this->LoadVolumeButton = vtkKWLoadSaveButton::New ( );
    this->LoadVolumeButton->SetParent ( volLoadFrame->GetFrame() );
    this->LoadVolumeButton->Create ( );
    this->LoadVolumeButton->SetText ("Load Volume");
    this->LoadVolumeButton->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {volume} {*.*} }");
    this->LoadVolumeButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");

    app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->LoadVolumeButton->GetWidgetName());
    
    // ---
    // DISPLAY FRAME            
    this->VolumeDisplayWidget = vtkSlicerVolumeDisplayWidget::New ( );
    this->VolumeDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
    this->VolumeDisplayWidget->SetParent ( page );
    this->VolumeDisplayWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->VolumeDisplayWidget->GetWidgetName(), page->GetWidgetName());
    
    volLoadFrame->Delete();
    volHelpFrame->Delete();
}





