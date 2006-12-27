#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerTractographyDisplayGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerTractographyLogic.h"
//#include "vtkSlicerFiberBundleDisplayWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTractographyDisplayGUI );
vtkCxxRevisionMacro ( vtkSlicerTractographyDisplayGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerTractographyDisplayGUI::vtkSlicerTractographyDisplayGUI ( )
{

    // classes not yet defined!
    //this->Logic = NULL;
    //this->FiberBundleNode = NULL;
    this->LoadTractographyButton = NULL;
    this->LoadTractographyDirectoryButton = NULL;
    this->SaveTractographyButton = NULL;
    this->FiberBundleSelectorWidget = NULL;
    //this->FiberBundleDisplayWidget = NULL;
    //this->ClipTractographyWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTractographyDisplayGUI::~vtkSlicerTractographyDisplayGUI ( )
{
  this->RemoveGUIObservers();

  //this->SetModuleLogic ( NULL );

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
  /*if (this->FiberBundleDisplayWidget ) 
    {
    this->FiberBundleDisplayWidget->SetParent(NULL);
    this->FiberBundleDisplayWidget->Delete ( );
    }*/
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerTractographyDisplayGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "FiberBundleNode: " << this->GetFiberBundleNode ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::RemoveGUIObservers ( )
{
  if (this->LoadTractographyButton)
    {
    this->LoadTractographyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadTractographyDirectoryButton)
    {
    this->LoadTractographyDirectoryButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveTractographyButton)
    {
    this->SaveTractographyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::AddGUIObservers ( )
{
  this->LoadTractographyButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadTractographyDirectoryButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveTractographyButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerTractographyDisplayGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);
  if (filebrowse == this->LoadTractographyButton  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a file has been selected for loading...
    char *fileName = filebrowse->GetFileName();
    if ( fileName ) 
      {
      //vtkSlicerTractographyLogic* modelLogic = this->Logic;
      
      //vtkMRMLFiberBundleNode *modelNode = modelLogic->AddFiberBundle( fileName );

  //     if ( modelNode == NULL ) 
//         {
//         // TODO: generate an error...
//         vtkErrorMacro("Unable to read model file " << fileName);
//         // reset the file browse button text
//         this->LoadTractographyButton->SetText ("Load Tractography");
//         }
//       else
//         {
//         filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        
//         }

      }
    return;
    }
    else if (filebrowse == this->LoadTractographyDirectoryButton  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a file has been selected for loading...
    char *fileName = filebrowse->GetFileName();
    if ( fileName ) 
      {
      //vtkSlicerTractographyLogic* modelLogic = this->Logic;

//       if (modelLogic->AddTractography( fileName, ".vtk") == 0)
//         {
//         // TODO: generate an error...
//         }
//       else
//         {
//         filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        
//         }

      }
    return;
    }
  else if (filebrowse == this->SaveTractographyButton  && event == vtkKWPushButton::InvokedEvent )
      {
      // If a file has been selected for saving...
      char *fileName = filebrowse->GetFileName();
      if ( fileName ) 
      {
 
//       vtkSlicerTractographyLogic* TractographyLogic = this->Logic;
//         vtkMRMLFiberBundleNode *volNode = vtkMRMLFiberBundleNode::SafeDownCast(this->FiberBundleSelectorWidget->GetSelected());

//         if ( !TractographyLogic->SaveTractography( fileName, volNode ))
//           {
//          // TODO: generate an error...
//           }
//         else
//           {
//           filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");           
//           }
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
void vtkSlicerTractographyDisplayGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  // Define your help text here.
  const char *help = "**Tractography Module:** Load, save and adjust display parameters of models. ";
  
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
    /*
    this->FiberBundleDisplayWidget = vtkSlicerFiberBundleDisplayWidget::New ( );
    this->FiberBundleDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
    this->FiberBundleDisplayWidget->SetParent ( modDisplayFrame->GetFrame() );
    this->FiberBundleDisplayWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->FiberBundleDisplayWidget->GetWidgetName(), 
                  modDisplayFrame->GetFrame()->GetWidgetName());
    */

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
    this->FiberBundleSelectorWidget->SetMRMLScene(this->GetMRMLScene());
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





