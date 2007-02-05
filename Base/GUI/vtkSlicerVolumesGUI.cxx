#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenu.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumesGUI );
vtkCxxRevisionMacro ( vtkSlicerVolumesGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::vtkSlicerVolumesGUI ( )
{

    this->Logic = NULL;
    this->VolumeNode = NULL;
    this->SelectedVolumeID = NULL;

    this->VolumeSelectorWidget = NULL;
    this->LoadVolumeButton = NULL;
    this->SaveVolumeButton = NULL;
    this->VolumeDisplayWidget = NULL;

    this->HelpFrame = NULL;
    this->LoadFrame = NULL;
    this->DisplayFrame = NULL;
    this->SaveFrame = NULL;

    this->NameEntry = NULL;
    this->CenterImageMenu = NULL;
    this->LabelMapCheckButton = NULL;
    this->ApplyButton=NULL;

    NACLabel = vtkKWLabel::New();
    NAMICLabel = vtkKWLabel::New();
    NCIGTLabel = vtkKWLabel::New();
    BIRNLabel = vtkKWLabel::New();

}


//---------------------------------------------------------------------------
vtkSlicerVolumesGUI::~vtkSlicerVolumesGUI ( )
{
  this->RemoveGUIObservers();
    
  if (this->SelectedVolumeID)
    {
    delete [] this->SelectedVolumeID;
    this->SelectedVolumeID = NULL;
    }
  if (this->LoadVolumeButton )
    {
    this->LoadVolumeButton->SetParent(NULL );
    this->LoadVolumeButton->Delete ( );
    }
  if (this->SaveVolumeButton )
    {
    this->SaveVolumeButton->SetParent(NULL );
    this->SaveVolumeButton->Delete ( );
    }

  if (this->VolumeDisplayWidget)
    {
    this->VolumeDisplayWidget->SetParent(NULL );
    this->VolumeDisplayWidget->Delete ( );
    }
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->SetParent(NULL );
    this->VolumeSelectorWidget->Delete ( );
    }
  if (this->CenterImageMenu)
    {
    this->CenterImageMenu->SetParent(NULL );
    this->CenterImageMenu->Delete ( );
    }
  if (this->LabelMapCheckButton)
    {
    this->LabelMapCheckButton->SetParent(NULL );
    this->LabelMapCheckButton->Delete ( );
    }
  if (this->ApplyButton)
    {
    this->ApplyButton->SetParent(NULL );
    this->ApplyButton->Delete ( );
    }
  if (this->NameEntry)
    {
    this->NameEntry->SetParent(NULL );
    this->NameEntry->Delete ( );
    }
  if ( this->HelpFrame )
    {
    this->HelpFrame->SetParent (NULL);
    this->HelpFrame->Delete ( );
    this->HelpFrame = NULL;
    }
  if ( this->LoadFrame )
    {
    this->LoadFrame->SetParent ( NULL );
    this->LoadFrame->Delete ( );
    this->LoadFrame = NULL;
    }
  if ( this->SaveFrame )
    {
    this->SaveFrame->SetParent ( NULL );
    this->SaveFrame->Delete ( );
    this->SaveFrame = NULL;
    }
  if ( this->DisplayFrame )
    {
    this->DisplayFrame->SetParent ( NULL );
    this->DisplayFrame->Delete ( );
    this->DisplayFrame = NULL;
    }

  if ( this->NACLabel )
    {
    this->NACLabel->SetParent ( NULL );
    this->NACLabel->Delete();
    this->NACLabel = NULL;
    }
  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  if ( this->NCIGTLabel )
    {
    this->NCIGTLabel->SetParent ( NULL );
    this->NCIGTLabel->Delete();
    this->NCIGTLabel = NULL;
    }
  if ( this->BIRNLabel )
    {
    this->BIRNLabel->SetParent ( NULL );
    this->BIRNLabel->Delete();
    this->BIRNLabel = NULL;
    }

  this->SetModuleLogic ( NULL );
   vtkSetMRMLNodeMacro (this->VolumeNode, NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerVolumesGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "VolumeNode: " << this->GetVolumeNode ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    os << indent << "HelpFrame: " << this->GetHelpFrame ( ) << "\n";
    os << indent << "LoadFrame: " << this->GetLoadFrame ( ) << "\n";
    os << indent << "DisplayFrame: " << this->GetDisplayFrame ( ) << "\n";
    os << indent << "SaveFrame: " << this->GetSaveFrame ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::RemoveGUIObservers ( )
{
    // Fill in
    if (this->LoadVolumeButton)
      {
      this->LoadVolumeButton->GetWidget()->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
      }
    if (this->SaveVolumeButton)
      {
      this->SaveVolumeButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
      }
    if (this->ApplyButton)
      {
      this->ApplyButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
      }
}


//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::AddGUIObservers ( )
{

    // Fill in
    // observer load volume button
    this->LoadVolumeButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SaveVolumeButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessGUIEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
  if (this->LoadVolumeButton->GetWidget() == vtkKWLoadSaveButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {
    const char * filename = this->LoadVolumeButton->GetWidget()->GetFileName();
    if (filename)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      this->NameEntry->GetWidget()->SetValue(name.c_str());
      }
    else
      {
      this->NameEntry->GetWidget()->SetValue("");
      }
    this->LoadVolumeButton->GetWidget()->SetText ("Select Volume File");
    }  
  else if (this->ApplyButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a file has been selected for loading...
    char *fileName = this->LoadVolumeButton->GetWidget()->GetFileName();
    if ( fileName ) 
      {

       vtkKWMenuButton *mb = this->CenterImageMenu->GetWidget();
       int centered;
       if ( !strcmp (mb->GetValue(), "Centered") )   
         {
         centered = 1;
         }
       else 
         {
         centered = 0;
         }

       int labelMap;
       if ( this->LabelMapCheckButton->GetSelectedState() )
         {
         labelMap = 1;
         }
       else
         {
         labelMap = 0;
         }

      vtkSlicerVolumesLogic* volumeLogic = this->Logic;
      volumeLogic->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

      vtkMRMLVolumeNode *volumeNode = volumeLogic->AddArchetypeVolume( fileName, centered, labelMap, this->NameEntry->GetWidget()->GetValue() );
      if ( volumeNode == NULL ) 
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->LoadFrame->GetFrame() );
        dialog->SetStyleToMessage();
        std::string msg = std::string("Unable to read volume file ") + std::string(fileName);
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();
        }
      else
        {
        this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");

        this->ApplicationLogic->GetSelectionNode()->SetActiveVolumeID( volumeNode->GetID() );
        this->ApplicationLogic->PropagateVolumeSelection();
        this->VolumeDisplayWidget->SetVolumeNode(volumeNode);               
        }
      volumeLogic->RemoveObservers(vtkCommand::ProgressEvent,  this->GUICallbackCommand);
      }

      return;
    }
    else if (this->SaveVolumeButton == vtkKWLoadSaveButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
      {
      // If a file has been selected for saving...
      char *fileName = this->SaveVolumeButton->GetFileName();
      if ( fileName ) 
      {
        vtkSlicerVolumesLogic* volumeLogic = this->Logic;
        vtkMRMLVolumeNode *volNode = vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
        if ( !volumeLogic->SaveArchetypeVolume( fileName, volNode ))
          {
         // TODO: generate an error...
          }
        else
          {
          this->SaveVolumeButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");           
          }
       }
       return;
    }
} 



//---------------------------------------------------------------------------
void vtkSlicerVolumesGUI::ProcessLogicEvents ( vtkObject *caller,
                                               unsigned long event, void *callData )
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    double progress = *((double *)callData);
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100*progress);
    }
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

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a volume loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Volumes", "Volumes", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Volumes Module loads, saves and adjusts display parameters of volume data.";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Volumes" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    this->NACLabel->SetParent ( this->GetLogoFrame() );
    this->NACLabel->Create();
    this->NACLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetNACLogo() );

    this->NAMICLabel->SetParent ( this->GetLogoFrame() );
    this->NAMICLabel->Create();
    this->NAMICLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetNAMICLogo() );    

    this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
    this->NCIGTLabel->Create();
    this->NCIGTLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetNCIGTLogo() );
    
    this->BIRNLabel->SetParent ( this->GetLogoFrame() );
    this->BIRNLabel->Create();
    this->BIRNLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetBIRNLogo() );
    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  
    
    // ---
    // LOAD FRAME            
    LoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->LoadFrame->SetParent ( page );
    this->LoadFrame->Create ( );
    this->LoadFrame->SetLabelText ("Load");
    this->LoadFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->LoadFrame->GetWidgetName(), page->GetWidgetName());
    // add a file browser 
    this->LoadVolumeButton = vtkKWLoadSaveButtonWithLabel::New ( );
    this->LoadVolumeButton->SetParent ( this->LoadFrame->GetFrame() );
    this->LoadVolumeButton->Create ( );
    this->LoadVolumeButton->SetWidth(20);
    this->LoadVolumeButton->GetWidget()->SetText ("Select Volume File");
    this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {volume} {*.*} }");
    this->LoadVolumeButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");
    app->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
                this->LoadVolumeButton->GetWidgetName());
    // volume name
    this->NameEntry = vtkKWEntryWithLabel::New();
    this->NameEntry->SetParent(this->LoadFrame->GetFrame());
    this->NameEntry->Create();
    this->NameEntry->SetWidth(20);
    this->NameEntry->SetLabelWidth(12);
    this->NameEntry->SetLabelText("Volume Name:");
    this->NameEntry->GetWidget()->SetValue ( "" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->NameEntry->GetWidgetName());

    // center image
    this->CenterImageMenu = vtkKWMenuButtonWithLabel::New();
    this->CenterImageMenu->SetParent(this->LoadFrame->GetFrame());
    this->CenterImageMenu->Create();
    this->CenterImageMenu->SetWidth(20);
    this->CenterImageMenu->SetLabelWidth(12);
    this->CenterImageMenu->SetLabelText("Image Origin:");
    this->CenterImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "Centered");
    this->CenterImageMenu->GetWidget()->GetMenu()->AddRadioButton ( "From File");
    this->CenterImageMenu->GetWidget()->SetValue ( "Centered" );
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->CenterImageMenu->GetWidgetName());

    // is this a lable map?
    this->LabelMapCheckButton = vtkKWCheckButton::New();
    this->LabelMapCheckButton->SetParent(this->LoadFrame->GetFrame());
    this->LabelMapCheckButton->Create();
    this->LabelMapCheckButton->SelectedStateOff();
    this->LabelMapCheckButton->SetText("Label Map");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->LabelMapCheckButton->GetWidgetName());

    // Apply button
    this->ApplyButton = vtkKWPushButton::New();
    this->ApplyButton->SetParent(this->LoadFrame->GetFrame());
    this->ApplyButton->Create();
    this->ApplyButton->SetText("Apply");
    this->ApplyButton->SetWidth ( 20);
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->ApplyButton->GetWidgetName());


    // ---
    // DISPLAY FRAME            
    DisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
    this->DisplayFrame->SetParent ( page );
    this->DisplayFrame->Create ( );
    this->DisplayFrame->SetLabelText ("Display");
    this->DisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->DisplayFrame->GetWidgetName(), page->GetWidgetName());

    this->VolumeDisplayWidget = vtkSlicerVolumeDisplayWidget::New ( );
    this->VolumeDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
    this->VolumeDisplayWidget->SetParent ( this->DisplayFrame->GetFrame() );
    this->VolumeDisplayWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->VolumeDisplayWidget->GetWidgetName(), this->DisplayFrame->GetFrame()->GetWidgetName());

    // ---
    // Save FRAME            
    SaveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->SaveFrame->SetParent ( page );
    this->SaveFrame->Create ( );
    this->SaveFrame->SetLabelText ("Save");
    this->SaveFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->SaveFrame->GetWidgetName(), page->GetWidgetName());

    // selector for save
    this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->VolumeSelectorWidget->SetParent ( this->SaveFrame->GetFrame() );
    this->VolumeSelectorWidget->Create ( );
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->VolumeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->VolumeSelectorWidget->SetBorderWidth(2);
    this->VolumeSelectorWidget->SetPadX(2);
    this->VolumeSelectorWidget->SetPadY(2);
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->VolumeSelectorWidget->SetLabelText( "Volume To Save: ");
    this->VolumeSelectorWidget->SetBalloonHelpString("select a volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->VolumeSelectorWidget->GetWidgetName());

    this->SaveVolumeButton = vtkKWLoadSaveButton::New ( );
    this->SaveVolumeButton->SetParent ( this->SaveFrame->GetFrame() );
    this->SaveVolumeButton->Create ( );
    this->SaveVolumeButton->SetText ("Save Volume");
    this->SaveVolumeButton->GetLoadSaveDialog()->SaveDialogOn();
    this->SaveVolumeButton->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {volume} {*.*} }");
    this->SaveVolumeButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");
     app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->SaveVolumeButton->GetWidgetName());
    
}





