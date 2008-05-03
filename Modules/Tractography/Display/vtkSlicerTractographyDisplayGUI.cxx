#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerTractographyDisplayGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerFiberBundleLogic.h"
#include "vtkSlicerFiberBundleDisplayWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWTopLevel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTractographyDisplayGUI );
vtkCxxRevisionMacro ( vtkSlicerTractographyDisplayGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerTractographyDisplayGUI::vtkSlicerTractographyDisplayGUI ( )
{

  this->Logic = NULL;
  this->LoadTractographyButton = NULL;
  this->LoadTractographyDirectoryButton = NULL;
  this->SaveTractographyButton = NULL;
  this->FiberBundleSelectorWidget = NULL;
  this->FiberBundleDisplayWidget = NULL;
  this->AllFiberBundlesDisplayWidget = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerTractographyDisplayGUI::~vtkSlicerTractographyDisplayGUI ( )
{
  this->RemoveGUIObservers();

  this->SetLogic ( NULL );

  if (this->LoadTractographyButton ) 
    {
    this->LoadTractographyButton->SetParent(NULL);
    this->LoadTractographyButton->Delete ( );
    }    
  if (this->LoadTractographyDirectoryButton ) 
    {
    this->LoadTractographyDirectoryButton->SetParent(NULL);
    this->LoadTractographyDirectoryButton->Delete ( );
    }    
  if (this->SaveTractographyButton ) 
    {
    this->SaveTractographyButton->SetParent(NULL);
    this->SaveTractographyButton->Delete ( );
    }
  if (this->FiberBundleSelectorWidget ) 
    {
    this->FiberBundleSelectorWidget->SetParent(NULL);
    this->FiberBundleSelectorWidget->Delete ( );
    }

  if (this->FiberBundleDisplayWidget ) 
    {
    this->FiberBundleDisplayWidget->SetParent(NULL);
    this->FiberBundleDisplayWidget->Delete ( );
    }

  if (this->AllFiberBundlesDisplayWidget ) 
    {
    this->AllFiberBundlesDisplayWidget->SetParent(NULL);
    this->AllFiberBundlesDisplayWidget->Delete ( );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerTractographyDisplayGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::RemoveGUIObservers ( )
{
  if (this->LoadTractographyButton)
    {
    this->LoadTractographyButton->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
   }
  if (this->LoadTractographyDirectoryButton)
    {
    this->LoadTractographyDirectoryButton->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveTractographyButton)
    {
    this->SaveTractographyButton->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::AddGUIObservers ( )
{
  this->LoadTractographyButton->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadTractographyDirectoryButton->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveTractographyButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent,  (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  vtkKWLoadSaveDialog *loadSaveDialog = vtkKWLoadSaveDialog::SafeDownCast(caller);
  if (loadSaveDialog && loadSaveDialog == this->LoadTractographyButton->GetLoadSaveDialog() &&
      event == vtkKWTopLevel::WithdrawEvent  )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadTractographyButton->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerFiberBundleLogic* fiberBundleLogic = this->Logic;
      
      vtkMRMLFiberBundleNode *fiberBundleNode = fiberBundleLogic->AddFiberBundle( fileName );

      if ( fiberBundleNode == NULL ) 
        {
        // TODO: generate an error...
        vtkErrorMacro("Unable to read model file " << fileName);
        }
      else
        {
        this->LoadTractographyButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        
        }

      }
    
    // reset the file browse button text
    this->LoadTractographyButton->SetText ("Load Tractography");

    return;

    }
  else if (loadSaveDialog && loadSaveDialog == this->LoadTractographyDirectoryButton->GetLoadSaveDialog() &&
           event == vtkKWTopLevel::WithdrawEvent )
    {

    // If a directory has been selected for loading...
    const char *fileName = this->LoadTractographyDirectoryButton->GetFileName();
    if ( fileName ) 
      {
      vtkSlicerFiberBundleLogic* fiberBundleLogic = this->Logic;
      std::vector<std::string> suffix;
      suffix.push_back(std::string(".vtk"));
      suffix.push_back(std::string(".vtp"));
      if (fiberBundleLogic->AddFiberBundles( fileName, suffix) == 0)
        {
        // TODO: generate an error...
        }
      else
        {
        this->LoadTractographyDirectoryButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        }

      }

    // reset the file browse button text
    this->LoadTractographyButton->SetText ("Load Tractography");

    return;
    }
  else if (loadSaveDialog && loadSaveDialog == this->SaveTractographyButton->GetLoadSaveDialog()  && 
           event == vtkKWTopLevel::WithdrawEvent  )
      {
      // If a file has been selected for saving...
      const char *fileName = this->SaveTractographyButton->GetFileName();
      if ( fileName ) 
      {
 
      vtkSlicerFiberBundleLogic* fiberBundleLogic = this->Logic;
        vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(this->FiberBundleSelectorWidget->GetSelected());

        if ( !fiberBundleLogic->SaveFiberBundle( fileName, fiberBundleNode ))
          {
         // TODO: generate an error...
          }
        else
          {
          this->SaveTractographyButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");           
          }
       }

 
       return;
      } 
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::Exit ( )
{
    // Fill in
}



//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::SetModuleLogic ( vtkSlicerLogic *logic )
{
  this->SetLogic( dynamic_cast<vtkSlicerFiberBundleLogic*> (logic) );
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    // Define your help text here.
    const char *help = "**Tractography Module:** Load, save and adjust display parameters of fiber bundles. ";
  
    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Tractography", "Tractography", NULL );
    
    // HELP FRAME
    vtkSlicerModuleCollapsibleFrame *modHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modHelpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
    modHelpFrame->Create ( );
    modHelpFrame->CollapseFrame ( );
    modHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Tractography")->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( modHelpFrame->GetFrame() );
    this->HelpText->Create ( );
    this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
    this->HelpText->SetVerticalScrollbarVisibility ( 1 );
    this->HelpText->GetWidget()->SetText ( help );
    this->HelpText->GetWidget()->SetReliefToFlat ( );
    this->HelpText->GetWidget()->SetWrapToWord ( );
    this->HelpText->GetWidget()->ReadOnlyOn ( );
    this->HelpText->GetWidget()->QuickFormattingOn ( );
    this->HelpText->GetWidget()->SetBalloonHelpString ( "" );
    app->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                  this->HelpText->GetWidgetName ( ) );

    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *modLoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modLoadFrame->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
    modLoadFrame->Create ( );
    modLoadFrame->SetLabelText ("Load");
    modLoadFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modLoadFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Tractography")->GetWidgetName());

    // add a file browser 
    this->LoadTractographyButton = vtkKWLoadSaveButton::New ( );
    this->LoadTractographyButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadTractographyButton->Create ( );
    this->LoadTractographyButton->SetText ("Load Tractography");
    this->LoadTractographyButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
    this->LoadTractographyButton->GetLoadSaveDialog()->SetFileTypes(
                                                             "{ {model} {*.*} }");
    app->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
                this->LoadTractographyButton->GetWidgetName());

   // add a file browser 
    this->LoadTractographyDirectoryButton = vtkKWLoadSaveButton::New ( );
    this->LoadTractographyDirectoryButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadTractographyDirectoryButton->Create ( );
    this->LoadTractographyDirectoryButton->SetText ("Load Tractography Directory");
    this->LoadTractographyDirectoryButton->GetLoadSaveDialog()->ChooseDirectoryOn();
    app->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
                this->LoadTractographyDirectoryButton->GetWidgetName());

  
    // DISPLAY FRAME            
    vtkSlicerModuleCollapsibleFrame *modDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
    modDisplayFrame->Create ( );
    modDisplayFrame->SetLabelText ("Display");
    modDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Tractography")->GetWidgetName());
    

//     this->AllFiberBundlesDisplayWidget = vtkSlicerAllFiberBundlesDisplayWidget::New ( );
//     this->AllFiberBundlesDisplayWidget->SetMRMLScene(this->Logic->GetMRMLScene() );
//     this->AllFiberBundlesDisplayWidget->SetParent ( modDisplayFrame->GetFrame() );
//     this->AllFiberBundlesDisplayWidget->Create ( );
//     //this->AllFiberBundlesDisplayWidget->DebugOn ( );
//     app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
//                   this->AllFiberBundlesDisplayWidget->GetWidgetName(), 
//                   modDisplayFrame->GetFrame()->GetWidgetName());


    this->FiberBundleDisplayWidget = vtkSlicerFiberBundleDisplayWidget::New ( );
    this->FiberBundleDisplayWidget->SetMRMLScene(this->Logic->GetMRMLScene() );
    this->FiberBundleDisplayWidget->SetParent ( modDisplayFrame->GetFrame() );
    this->FiberBundleDisplayWidget->Create ( );
    //this->FiberBundleDisplayWidget->DebugOn ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->FiberBundleDisplayWidget->GetWidgetName(), 
                  modDisplayFrame->GetFrame()->GetWidgetName());

    // ---
    // Save FRAME            
    vtkSlicerModuleCollapsibleFrame *modelSaveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modelSaveFrame->SetParent ( this->UIPanel->GetPageWidget ( "Tractography" ) );
    modelSaveFrame->Create ( );
    modelSaveFrame->SetLabelText ("Save");
    modelSaveFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modelSaveFrame->GetWidgetName(), 
                  this->UIPanel->GetPageWidget ( "Tractography" )->GetWidgetName());

    // selector for save
    this->FiberBundleSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->FiberBundleSelectorWidget->SetParent ( modelSaveFrame->GetFrame() );
    this->FiberBundleSelectorWidget->Create ( );
    this->FiberBundleSelectorWidget->SetNodeClass("vtkMRMLFiberBundleNode", NULL, NULL, NULL);
    this->FiberBundleSelectorWidget->SetMRMLScene(this->Logic->GetMRMLScene());
    this->FiberBundleSelectorWidget->UpdateMenu();
    //this->FiberBundleSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->FiberBundleSelectorWidget->SetBorderWidth(2);
    this->FiberBundleSelectorWidget->SetPadX(2);
    this->FiberBundleSelectorWidget->SetPadY(2);
    this->FiberBundleSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->FiberBundleSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->FiberBundleSelectorWidget->SetLabelText( "FiberBundle To Save: ");
    this->FiberBundleSelectorWidget->SetBalloonHelpString("select a FiberBundle from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->FiberBundleSelectorWidget->GetWidgetName());

    this->SaveTractographyButton = vtkKWLoadSaveButton::New ( );
    this->SaveTractographyButton->SetParent ( modelSaveFrame->GetFrame() );
    this->SaveTractographyButton->Create ( );
    this->SaveTractographyButton->SetText ("Save Tractography");
    this->SaveTractographyButton->GetLoadSaveDialog()->SaveDialogOn();
    this->SaveTractographyButton->GetLoadSaveDialog()->SetFileTypes(
                                                              "{ {Tractography} {*.*} }");
    this->SaveTractographyButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
      "OpenPath");
     app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->SaveTractographyButton->GetWidgetName());
    
    modLoadFrame->Delete ( );
    modHelpFrame->Delete ( );
    modDisplayFrame->Delete ( );
    modelSaveFrame->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::Init ( )
{
  vtkMRMLScene *scene = this->Logic->GetMRMLScene();

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  this->Logic->SetAndObserveMRMLSceneEvents(scene, events);
  events->Delete();
}

