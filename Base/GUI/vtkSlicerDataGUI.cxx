#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataGUI );
vtkCxxRevisionMacro ( vtkSlicerDataGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDataGUI::vtkSlicerDataGUI ( )
{
  this->MRMLTreeWidget = vtkSlicerMRMLTreeWidget::New();
  this->SceneSnapshotWidget = vtkSlicerSceneSnapshotWidget::New();
  this->RecordSnapshotWidget = vtkSlicerRecordSnapshotWidget::New();

  NACLabel = NULL;
  NAMICLabel = NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerDataGUI::~vtkSlicerDataGUI ( )
{
  if (this->MRMLTreeWidget)
    {
    this->MRMLTreeWidget->RemoveWidgetObservers ( );
    this->MRMLTreeWidget->SetParent (NULL );
    this->MRMLTreeWidget->Delete ( );
    }
  if (this->SceneSnapshotWidget)
    {
    this->SceneSnapshotWidget->RemoveWidgetObservers ( );
    this->SceneSnapshotWidget->SetParent (NULL );
    this->SceneSnapshotWidget->Delete ( );
    }
  if (this->RecordSnapshotWidget)
    {
    this->RecordSnapshotWidget->RemoveWidgetObservers ( );
    this->RecordSnapshotWidget->SetParent (NULL );
    this->RecordSnapshotWidget->Delete ( );
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

  this->Built = false;
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerDataGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveGUIObservers ( )
{
  this->MRMLTreeWidget->RemoveObservers (vtkSlicerMRMLTreeWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddGUIObservers ( )
{
  this->MRMLTreeWidget->AddObserver (vtkSlicerMRMLTreeWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  const char *moduleName;
  vtkMRMLNode *node = (vtkMRMLNode *)callData;
  if (node->IsA("vtkMRMLVolumeNode"))
    {
    moduleName = "Volumes";
    }
  else if (node->IsA("vtkMRMLModelNode"))
    {
    moduleName = "Models";
    }
  else if (node->IsA("vtkMRMLTransformNode"))
    {
    moduleName = "Transforms";
    }
  else if (node->IsA("vtkMRMLFiducialListNode"))
    {
    moduleName = "Fiducials";
    }

  this->InvokeEvent(vtkSlicerModuleGUI::ModuleSelectedEvent, (void *)moduleName);
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
    // Fill in
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ReleaseModuleEventBindings ( )
{
  
}




//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Enter ( )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    this->AddObserver ( vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand() );
    }
    this->CreateModuleEventBindings();
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Data", "Data", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Data Module displays and permits operations on the MRML tree. ";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Data" );
    this->BuildHelpAndAboutFrame ( page, help, about );

    this->NACLabel = vtkKWLabel::New();
    this->NACLabel->SetParent ( this->GetLogoFrame() );
    this->NACLabel->Create();
    this->NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

    this->NAMICLabel = vtkKWLabel::New();
    this->NAMICLabel->SetParent ( this->GetLogoFrame() );
    this->NAMICLabel->Create();
    this->NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

    this->NCIGTLabel = vtkKWLabel::New();
    this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
    this->NCIGTLabel->Create();
    this->NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
    this->BIRNLabel = vtkKWLabel::New();
    this->BIRNLabel->SetParent ( this->GetLogoFrame() );
    this->BIRNLabel->Create();
    this->BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );
    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  

    // INSPECT FRAME
    vtkSlicerModuleCollapsibleFrame *displayModifyFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayModifyFrame->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
    displayModifyFrame->Create ( );
    displayModifyFrame->ExpandFrame ( );
    displayModifyFrame->SetLabelText ("Display & modify scene");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayModifyFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());
    
    this->MRMLTreeWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->MRMLTreeWidget->SetParent ( displayModifyFrame->GetFrame() );
    this->MRMLTreeWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->MRMLTreeWidget->GetWidgetName(), displayModifyFrame->GetFrame()->GetWidgetName());

    // Snapshot FRAME
    vtkSlicerModuleCollapsibleFrame *snapshotFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    snapshotFrame->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
    snapshotFrame->Create ( );
    snapshotFrame->ExpandFrame ( );
    snapshotFrame->SetLabelText ("Scene Snapshots");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  snapshotFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());

    this->SceneSnapshotWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->SceneSnapshotWidget->SetParent ( snapshotFrame->GetFrame() );
    this->SceneSnapshotWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->SceneSnapshotWidget->GetWidgetName(), snapshotFrame->GetFrame()->GetWidgetName());


    //this->RecordSnapshotWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    //this->RecordSnapshotWidget->SetParent ( snapshotFrame->GetFrame() );
    //this->RecordSnapshotWidget->Create ( );
    //app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  //this->RecordSnapshotWidget->GetWidgetName(), snapshotFrame->GetFrame()->GetWidgetName());


    displayModifyFrame->Delete ( );
    snapshotFrame->Delete ( );
}





