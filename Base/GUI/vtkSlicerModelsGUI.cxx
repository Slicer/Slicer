#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerModelDisplayWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelsGUI );
vtkCxxRevisionMacro ( vtkSlicerModelsGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::vtkSlicerModelsGUI ( )
{

    // classes not yet defined!
    this->Logic = NULL;
    //this->ModelNode = NULL;
    this->LoadModelButton = NULL;
    this->LoadModelDirectoryButton = NULL;
    this->SaveModelButton = NULL;
    this->ModelSelectorWidget = NULL;
    this->ModelDisplayWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::~vtkSlicerModelsGUI ( )
{
  this->RemoveGUIObservers();

  this->SetModuleLogic ( NULL );

  if (this->LoadModelButton ) 
    {
    this->LoadModelButton->SetParent(NULL);
    this->LoadModelButton->Delete ( );
    }    
  if (this->LoadModelDirectoryButton ) 
    {
    this->LoadModelDirectoryButton->SetParent(NULL);
    this->LoadModelDirectoryButton->Delete ( );
    }    
  if (this->SaveModelButton ) 
    {
    this->SaveModelButton->SetParent(NULL);
    this->SaveModelButton->Delete ( );
    }
  if (this->ModelSelectorWidget ) 
    {
    this->ModelSelectorWidget->SetParent(NULL);
    this->ModelSelectorWidget->Delete ( );
    }
  if (this->ModelDisplayWidget ) 
    {
    this->ModelDisplayWidget->SetParent(NULL);
    this->ModelDisplayWidget->Delete ( );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerModelsGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "ModelNode: " << this->GetModelNode ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::RemoveGUIObservers ( )
{
  if (this->LoadModelButton)
    {
    this->LoadModelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadModelDirectoryButton)
    {
    this->LoadModelDirectoryButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveModelButton)
    {
    this->SaveModelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::AddGUIObservers ( )
{
  this->LoadModelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadModelDirectoryButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveModelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);
  if (filebrowse == this->LoadModelButton  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a file has been selected for loading...
    char *fileName = filebrowse->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerModelsLogic* modelLogic = this->Logic;
      
      vtkMRMLModelNode *modelNode = modelLogic->AddModel( fileName );
      if ( modelNode == NULL ) 
        {
        // TODO: generate an error...
        vtkErrorMacro("Unable to read model file " << fileName);
        // reset the file browse button text
        this->LoadModelButton->SetText ("Load Model");
        }
      else
        {
        filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        
        }
      }
    return;
    }
    else if (filebrowse == this->LoadModelDirectoryButton  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a file has been selected for loading...
    char *fileName = filebrowse->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerModelsLogic* modelLogic = this->Logic;
      
      if (modelLogic->AddModels( fileName, ".vtk") == 0)
        {
        // TODO: generate an error...
        }
      else
        {
        filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        
        }
      }
    return;
    }
  else if (filebrowse == this->SaveModelButton  && event == vtkKWPushButton::InvokedEvent )
      {
      // If a file has been selected for saving...
      char *fileName = filebrowse->GetFileName();
      if ( fileName ) 
      {
        vtkSlicerModelsLogic* ModelLogic = this->Logic;
        vtkMRMLModelNode *volNode = vtkMRMLModelNode::SafeDownCast(this->ModelSelectorWidget->GetSelected());
        if ( !ModelLogic->SaveModel( fileName, volNode ))
          {
         // TODO: generate an error...
          }
        else
          {
          filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");           
          }
       }
       return;
    } 
}    

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::Exit ( )
{
    // Fill in
}




//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  // Define your help text here.
  const char *help = "**Models Module:** Load, save and adjust display parameters of models. ";
  
    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Models", "Models", NULL );
    
    // HELP FRAME
    vtkSlicerModuleCollapsibleFrame *modHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modHelpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    modHelpFrame->Create ( );
    modHelpFrame->CollapseFrame ( );
    modHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( modHelpFrame->GetFrame() );
    this->HelpText->Create ( );
    this->HelpText->SetText ( help );
    this->HelpText->SetReliefToFlat ( );
    this->HelpText->SetWrapToWord ( );
    this->HelpText->ReadOnlyOn ( );
    this->HelpText->QuickFormattingOn ( );
    this->HelpText->SetBalloonHelpString ( "" );
    app->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                  this->HelpText->GetWidgetName ( ) );

    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *modLoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modLoadFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    modLoadFrame->Create ( );
    modLoadFrame->SetLabelText ("Load");
    modLoadFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modLoadFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    // add a file browser 
    this->LoadModelButton = vtkKWLoadSaveButton::New ( );
    this->LoadModelButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadModelButton->Create ( );
    this->LoadModelButton->SetText ("Load Model");
    this->LoadModelButton->GetLoadSaveDialog()->SetFileTypes(
                                                             "{ {model} {*.*} }");
    app->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
                this->LoadModelButton->GetWidgetName());

   // add a file browser 
    this->LoadModelDirectoryButton = vtkKWLoadSaveButton::New ( );
    this->LoadModelDirectoryButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadModelDirectoryButton->Create ( );
    this->LoadModelDirectoryButton->SetText ("Load Model Directory");
    this->LoadModelDirectoryButton->GetLoadSaveDialog()->ChooseDirectoryOn();
    app->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
                this->LoadModelDirectoryButton->GetWidgetName());

    // ---
    // DISPLAY FRAME               
    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *modDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    modDisplayFrame->Create ( );
    modDisplayFrame->SetLabelText ("Display");
    modDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->ModelDisplayWidget = vtkSlicerModelDisplayWidget::New ( );
    this->ModelDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
    this->ModelDisplayWidget->SetParent ( modDisplayFrame->GetFrame() );
    this->ModelDisplayWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelDisplayWidget->GetWidgetName(), 
                  modDisplayFrame->GetFrame()->GetWidgetName());
    // ---
    // Save FRAME            
    vtkSlicerModuleCollapsibleFrame *modelSaveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modelSaveFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    modelSaveFrame->Create ( );
    modelSaveFrame->SetLabelText ("Save");
    modelSaveFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modelSaveFrame->GetWidgetName(), 
                  this->UIPanel->GetPageWidget ( "Models" )->GetWidgetName());

    // selector for save
    this->ModelSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->ModelSelectorWidget->SetParent ( modelSaveFrame->GetFrame() );
    this->ModelSelectorWidget->Create ( );
    this->ModelSelectorWidget->SetNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
    this->ModelSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->ModelSelectorWidget->SetBorderWidth(2);
    this->ModelSelectorWidget->SetPadX(2);
    this->ModelSelectorWidget->SetPadY(2);
    this->ModelSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->ModelSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->ModelSelectorWidget->SetLabelText( "Model To Save: ");
    this->ModelSelectorWidget->SetBalloonHelpString("select a Model from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->ModelSelectorWidget->GetWidgetName());

    this->SaveModelButton = vtkKWLoadSaveButton::New ( );
    this->SaveModelButton->SetParent ( modelSaveFrame->GetFrame() );
    this->SaveModelButton->Create ( );
    this->SaveModelButton->SetText ("Save Model");
    this->SaveModelButton->GetLoadSaveDialog()->SaveDialogOn();
    this->SaveModelButton->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {Model} {*.*} }");
    this->SaveModelButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");
     app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->SaveModelButton->GetWidgetName());
    
    modLoadFrame->Delete ( );
    modHelpFrame->Delete ( );
    modDisplayFrame->Delete ( );
    modelSaveFrame->Delete();
}





