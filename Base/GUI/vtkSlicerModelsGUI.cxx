#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerModelDisplayWidget.h"
#include "vtkSlicerModelHierarchyWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWTopLevel.h"

// for scalars
#include "vtkPointData.h"

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
  this->ClipModelsWidget = NULL;
  this->LoadScalarsButton = NULL;
  this->ModelDisplaySelectorWidget = NULL;
  this->ModelHierarchyWidget = NULL;
  this->ModelDisplayFrame = NULL;

  NACLabel = NULL;
  NAMICLabel =NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::~vtkSlicerModelsGUI ( )
{
  this->RemoveGUIObservers();

  this->SetModuleLogic ( NULL );

  if (this->ModelDisplaySelectorWidget)
    {
    this->ModelDisplaySelectorWidget->SetParent(NULL);
    this->ModelDisplaySelectorWidget->Delete();
    this->ModelDisplaySelectorWidget = NULL;
    }

  if (this->ModelHierarchyWidget)
    {
    this->ModelHierarchyWidget->SetParent(NULL);
    this->ModelHierarchyWidget->Delete();
    this->ModelHierarchyWidget = NULL;
    }

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
  if (this->ClipModelsWidget ) 
    {
    this->ClipModelsWidget->SetParent(NULL);
    this->ClipModelsWidget->Delete ( );
    }
  if (this->LoadScalarsButton )
    {
    this->LoadScalarsButton->SetParent(NULL);
    this->LoadScalarsButton->Delete ( );
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
  if (this->ModelDisplayFrame)
    {
    this->ModelDisplayFrame->SetParent ( NULL );
    this->ModelDisplayFrame->Delete();
    }
  this->Built = false;
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
    this->LoadModelButton->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );    
    }
  if (this->LoadModelDirectoryButton)
    {
    this->LoadModelDirectoryButton->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );    
    }
  if (this->SaveModelButton)
    {
    this->SaveModelButton->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadScalarsButton)
    {
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );    
    }
  if (this->ModelDisplaySelectorWidget)
    {
    this->ModelDisplaySelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ModelHierarchyWidget)
    { 
    this->ModelHierarchyWidget->RemoveObservers(vtkSlicerModelHierarchyWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::AddGUIObservers ( )
{
  this->LoadModelButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadModelDirectoryButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveModelButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ModelDisplaySelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ModelHierarchyWidget->AddObserver(vtkSlicerModelHierarchyWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{

  if (vtkSlicerModelHierarchyWidget::SafeDownCast(caller) == this->ModelHierarchyWidget && 
      event == vtkSlicerModelHierarchyWidget::SelectedEvent)
    {
    vtkMRMLModelNode *model = reinterpret_cast<vtkMRMLModelNode *>(callData);
    if (model != NULL && model->GetDisplayNode() != NULL)
      {
      this->ModelDisplaySelectorWidget->SetSelected(model);
      if (this->ModelDisplayFrame)
        {
        this->ModelDisplayFrame->ExpandFrame();
        this->ModelDisplayFrame->Raise();
        }
      //this->ModelDisplayWidget->SetModelDisplayNode(model->GetDisplayNode());
      //this->ModelDisplayWidget->SetModelNode(model);
      }
    return;
    }

  if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->ModelDisplaySelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLModelNode *model = 
        vtkMRMLModelNode::SafeDownCast(this->ModelDisplaySelectorWidget->GetSelected());

    if (model != NULL && model->GetDisplayNode() != NULL)
      {
      this->ModelDisplayWidget->SetModelDisplayNode(model->GetModelDisplayNode());
      this->ModelDisplayWidget->SetModelNode(model);
      }
    return;
    }

  if (this->LoadModelButton->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadModelButton->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerModelsLogic* modelLogic = this->Logic;
      
      vtkMRMLModelNode *modelNode = modelLogic->AddModel( fileName );
      if ( modelNode == NULL ) 
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
        dialog->SetStyleToMessage();
        std::string msg = std::string("Unable to read model file ") + std::string(fileName);
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();

        vtkErrorMacro("Unable to read model file " << fileName);
        // reset the file browse button text
        }
      else
        {
         this->LoadModelButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");        
        }
      }
      this->LoadModelButton->SetText ("Load Model");
    return;
    }
    else if (this->LoadModelDirectoryButton->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadModelDirectoryButton->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerModelsLogic* modelLogic = this->Logic;
      
      if (modelLogic->AddModels( fileName, ".vtk") == 0)
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
        dialog->SetStyleToMessage();
        std::string msg = std::string("Unable to read models directory ") + std::string(fileName);
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();
        }
      else
        {
        this->LoadModelDirectoryButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");        
        }
      }
    this->LoadModelDirectoryButton->SetText ("Load Model Directory");
    return;
    }
  else if (this->SaveModelButton->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
      {
      // If a file has been selected for saving...
      const char *fileName = this->SaveModelButton->GetFileName();
      if ( fileName ) 
      {
        vtkSlicerModelsLogic* ModelLogic = this->Logic;
        vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(this->ModelSelectorWidget->GetSelected());
        if ( !ModelLogic->SaveModel( fileName, modelNode ))
          {
          vtkErrorMacro("ModelsGUI: unable to save model to file " << fileName);
          }
        else
          {
          this->SaveModelButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");           
          }
       }
       return;
    } 
  else if (this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent)
    {
    // If a scalar file has been selected for loading...
    const char *fileName = this->LoadScalarsButton->GetWidget()->GetFileName();
    if ( fileName ) 
      {
      // get the model from the display widget rather than this gui's save
      // model selector
      vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(this->ModelDisplaySelectorWidget->GetSelected());
      if (modelNode != NULL)
        {
        vtkDebugMacro("vtkSlicerModelsGUI: loading scalar for model " << modelNode->GetName());
        // load the scalars
        vtkSlicerModelsLogic* modelLogic = this->Logic;
        if (!modelLogic->AddScalar(fileName, modelNode))
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
          dialog->SetStyleToMessage();
          std::string msg = std::string("Unable to read scalars file ") + std::string(fileName);
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          
          vtkErrorMacro("Error loading scalar overlay file " << fileName);
          this->LoadScalarsButton->GetWidget()->SetText ("None");
          }
        else
          {
          this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
          // set the active scalar in the display node to this one
          // - is done in the model storage node         
          }
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
void vtkSlicerModelsGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ReleaseModuleEventBindings ( )
{
  
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::Enter ( )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
    this->CreateModuleEventBindings();
}



//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Models", "Models", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Models Module loads, saves and adjusts display parameters of models. ";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Models" );
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
    this->LoadModelButton->GetLoadSaveDialog()->SetTitle("Open Model");
    this->LoadModelButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
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

  
    // DISPLAY FRAME            
    this->ModelDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->ModelDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    this->ModelDisplayFrame->Create ( );
    this->ModelDisplayFrame->SetLabelText ("Display");
    this->ModelDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->LoadScalarsButton = vtkKWLoadSaveButtonWithLabel::New();
    this->LoadScalarsButton->SetParent ( this->ModelDisplayFrame->GetFrame() );
    this->LoadScalarsButton->Create ( );
    this->LoadScalarsButton->SetLabelText ("Load FreeSurfer Overlay: ");
    this->LoadScalarsButton->GetWidget()->SetText ("None");
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open FreeSurfer Overlay");
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
    this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {All} {.*} } { {Thickness} {.thickness} } { {Curve} {.curv} } { {Average Curve} {.avg_curv} } { {Sulc} {.sulc} } { {Area} {.area} } { {W} {.w} } { {Parcellation Annotation} {.annot} } { {Volume} {.mgz .mgh} }");
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 4", 
                this->LoadScalarsButton->GetWidgetName());

    this->ModelDisplaySelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->ModelDisplaySelectorWidget->SetParent ( this->ModelDisplayFrame->GetFrame() );
    this->ModelDisplaySelectorWidget->Create ( );
    this->ModelDisplaySelectorWidget->SetNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
    this->ModelDisplaySelectorWidget->SetChildClassesEnabled(0);
    this->ModelDisplaySelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->ModelDisplaySelectorWidget->SetBorderWidth(2);
    // this->ModelDisplaySelectorWidget->SetReliefToGroove();
    this->ModelDisplaySelectorWidget->SetPadX(2);
    this->ModelDisplaySelectorWidget->SetPadY(2);
    this->ModelDisplaySelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->ModelDisplaySelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->ModelDisplaySelectorWidget->SetLabelText( "Model Select: ");
    this->ModelDisplaySelectorWidget->SetBalloonHelpString("select a model from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   this->ModelDisplaySelectorWidget->GetWidgetName());


    this->ModelDisplayWidget = vtkSlicerModelDisplayWidget::New ( );
    this->ModelDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
    this->ModelDisplayWidget->SetParent ( this->ModelDisplayFrame->GetFrame() );
    this->ModelDisplayWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelDisplayWidget->GetWidgetName(), 
                  this->ModelDisplayFrame->GetFrame()->GetWidgetName());

    // Clip FRAME  
    vtkSlicerModuleCollapsibleFrame *clipFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    clipFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    clipFrame->Create ( );
    clipFrame->SetLabelText ("Clipping");
    clipFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  clipFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->ClipModelsWidget = vtkSlicerClipModelsWidget::New ( );
    this->ClipModelsWidget->SetMRMLScene(this->GetMRMLScene() );
    this->ClipModelsWidget->SetParent ( clipFrame->GetFrame() );
    this->ClipModelsWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ClipModelsWidget->GetWidgetName(), 
                  clipFrame->GetFrame()->GetWidgetName());

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
                                                              "{ {Model} {.*} }");
    this->SaveModelButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");
     app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->SaveModelButton->GetWidgetName());

     // Hierarchy FRAME  
    vtkSlicerModuleCollapsibleFrame *hierFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    hierFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    hierFrame->Create ( );
    hierFrame->SetLabelText ("Model Hierarchy");
    hierFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  hierFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->ModelHierarchyWidget = vtkSlicerModelHierarchyWidget::New ( );
    this->ModelHierarchyWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->ModelHierarchyWidget->SetParent ( hierFrame->GetFrame() );
    this->ModelHierarchyWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelHierarchyWidget->GetWidgetName(), 
                  hierFrame->GetFrame()->GetWidgetName());
    

   this->ProcessGUIEvents (this->ModelDisplaySelectorWidget,
                          vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );


    /*
    vtkMRMLNode *selected = this->ModelDisplaySelectorWidget->GetSelected();
    if (selected)
      {
      this->ModelSelectorWidget->SetSelected(NULL);
      this->ModelSelectorWidget->SetSelected(selected);
      }
    */

    modLoadFrame->Delete ( );
    clipFrame->Delete ( );
    modelSaveFrame->Delete();
    hierFrame->Delete ( );
}





