#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include <itksys/SystemTools.hxx> 
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModelDisplayWidget.h"
#include "vtkSlicerModelHierarchyWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerModelInfoWidget.h"

// for pick events
//#include "vtkSlicerViewerWidget.h"
//#include "vtkSlicerViewerInteractorStyle.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWTopLevel.h"

// for scalars
#include "vtkPointData.h"

// for new data module buttons
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerApplicationGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelsGUI );
vtkCxxRevisionMacro ( vtkSlicerModelsGUI, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::vtkSlicerModelsGUI ( )
{

  // classes not yet defined!
  this->Logic = NULL;
  this->ModelHierarchyLogic = NULL;

  //this->ModelNode = NULL;
  this->LoadModelButton = NULL;
  this->ModelDisplayWidget = NULL;
  this->ClipModelsWidget = NULL;
  this->LoadScalarsButton = NULL;
  this->ModelDisplaySelectorWidget = NULL;
  this->ModelHierarchyWidget = NULL;
  this->ModelDisplayFrame = NULL;
  this->ModelInfoWidget = NULL;

  NACLabel = NULL;
  NAMICLabel =NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;

  // for picking
//  this->ViewerWidget = NULL;
//  this->InteractorStyle = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerModelsGUI::~vtkSlicerModelsGUI ( )
{
  this->RemoveGUIObservers();

  this->SetModuleLogic (static_cast<vtkSlicerModelsLogic*>(0));
  this->SetModelHierarchyLogic ( NULL );

  if (this->ModelDisplaySelectorWidget)
    {
    this->ModelDisplaySelectorWidget->SetParent(NULL);
    this->ModelDisplaySelectorWidget->Delete();
    this->ModelDisplaySelectorWidget = NULL;
    }

  if (this->ModelInfoWidget)
    {
    this->ModelInfoWidget->SetParent(NULL);
    this->ModelInfoWidget->Delete();
    this->ModelInfoWidget = NULL;
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
//  this->SetViewerWidget(NULL);   
//  this->SetInteractorStyle(NULL);
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
    this->LoadModelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadScalarsButton)
    {
    this->LoadScalarsButton->RemoveObservers( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );   
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
  if (this->LoadModelButton)
    {
    this->LoadModelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadScalarsButton) 
    {
    this->LoadScalarsButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
    //this->ModelDisplaySelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  if (this->ModelHierarchyWidget)
    {
    this->ModelHierarchyWidget->AddObserver(vtkSlicerModelHierarchyWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }  
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
      //this->ModelDisplaySelectorWidget->SetSelected(model);
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
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  if (b == this->LoadModelButton || b == this->LoadScalarsButton)
    {
    // get the data gui
    vtkSlicerDataGUI *dataGUI = NULL;
    if ( this->GetApplication() != NULL )
      {
      dataGUI = vtkSlicerDataGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Data"));
      }
    if (dataGUI != NULL)
      {
      if (b == this->LoadModelButton)
        {
        dataGUI->RaiseAddModelWindow();
        }
      else if ( b == this->LoadScalarsButton )
        {
        dataGUI->RaiseAddScalarOverlayWindow();
        }
      }
    else
      {
      vtkErrorMacro("Unable to get the Data module interface for loading! Please use File->Add Data");
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
      dialog->SetStyleToMessage();
      std::string msg = std::string("Unable to get the Data module interface for loading! Please use File->Add Data");
      dialog->SetText(msg.c_str());
      dialog->Create ( );
      dialog->Invoke();
      dialog->Delete();
      }
    }
}    

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessLogicEvents ( vtkObject * vtkNotUsed(caller),
                                              unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModelsGUI::ProcessMRMLEvents ( vtkObject * vtkNotUsed(caller),
                                             unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
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
void vtkSlicerModelsGUI::Enter ( vtkMRMLNode *node )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  if (node && this->ModelHierarchyWidget)
    {
    this->ModelHierarchyWidget->UpdateTreeFromMRML();
    this->ModelHierarchyWidget->GetModelDisplaySelectorWidget()->UnconditionalUpdateMenu();
    this->ModelHierarchyWidget->SelectNode(node);
    }
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
    vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Models", "Models", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Models Module loads and adjusts display parameters of models.\n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:Models-Documentation-3.6</a>\nSave models via the File menu, Save button.\nThe Add 3D model or a model directory button will allow you to load any model that Slicer can read, as well as all the VTK models in a directory. Add Scalar Overlay will load a scalar file and associate it with the currently active model.\nYou can adjust the display properties of the models in the Display pane. Select the model you wish to work on from the model selector drop down menu. Scalar overlays are loaded with a default colour look up table, but can be reassigned manually. Once a new scalar overlay is chosen, currently the old color map is still used, so that must be adjusted in conjunction with the overlay.\nClipping is turned on for a model in the Display pane, and the slice planes that will clip the model are selected in the Clipping pane.\nThe Model Hierarchy pane allows you to group models together and set the group's properties.";
    const char *about = "This module was contributed by Nicole Aucoin, SPL, BWH (Ron Kikinis), and Alex Yarmarkovich, Isomics Inc. (Steve Pieper).\nThis work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
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
    this->LoadModelButton = vtkKWPushButton::New();
    this->LoadModelButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadModelButton->Create ( );
    this->LoadModelButton->SetBorderWidth ( 0 );
    this->LoadModelButton->SetReliefToFlat();  
    this->LoadModelButton->SetCompoundModeToLeft ();
    this->LoadModelButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadModelIcon() );
    this->LoadModelButton->SetText (" Add 3D model or a model directory" );
    this->LoadModelButton->SetWidth ( 300 );
    this->LoadModelButton->SetAnchorToWest();
    this->LoadModelButton->SetBalloonHelpString("Use this model loading option to raise a dialog with options to add models to the current scene.");
  
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -ipadx 0 -ipady 0", 
                this->LoadModelButton->GetWidgetName());


    this->LoadScalarsButton = vtkKWPushButton::New();
    this->LoadScalarsButton->SetParent ( modLoadFrame->GetFrame() );
    this->LoadScalarsButton->Create ( );
    this->LoadScalarsButton->SetBorderWidth ( 0 );
    this->LoadScalarsButton->SetReliefToFlat();  
    this->LoadScalarsButton->SetCompoundModeToLeft ();
    this->LoadScalarsButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadScalarOverlayIcon() );
    this->LoadScalarsButton->SetText (" Add scalar overlay" );
    this->LoadScalarsButton->SetWidth ( 300 );
    this->LoadScalarsButton->SetAnchorToWest();
    this->LoadScalarsButton->SetBalloonHelpString("Use this option to add a (FreeSurfer) scalar overlay to an existing model in the scene.");

    // this->LoadScalarsButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {All} {.*} } { {Thickness} {.thickness} } { {Curve} {.curv} } { {Average Curve} {.avg_curv} } { {Sulc} {.sulc} } { {Area} {.area} } { {W} {.w} } { {Parcellation Annotation} {.annot} } { {Volume} {.mgz .mgh} } { {Label} {.label} }");
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -ipadx 0 -ipady 0", 
                this->LoadScalarsButton->GetWidgetName());
    
    // DISPLAY FRAME            
    this->ModelDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->ModelDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    this->ModelDisplayFrame->Create ( );
    this->ModelDisplayFrame->SetLabelText ("Hierarchy & Display");
    this->ModelDisplayFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

 

    this->ModelHierarchyWidget = vtkSlicerModelHierarchyWidget::New ( );
    this->ModelHierarchyWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->ModelHierarchyWidget->SetModelHierarchyLogic(this->GetModelHierarchyLogic());
    this->ModelHierarchyWidget->SetParent ( this->ModelDisplayFrame->GetFrame() );
    this->ModelHierarchyWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelHierarchyWidget->GetWidgetName(), 
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

    // Info FRAME  
    vtkSlicerModuleCollapsibleFrame *infoFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    infoFrame->SetParent ( this->UIPanel->GetPageWidget ( "Models" ) );
    infoFrame->Create ( );
    infoFrame->SetLabelText ("Info");
    infoFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  infoFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Models")->GetWidgetName());

    this->ModelInfoWidget = vtkSlicerModelInfoWidget::New ( );
    this->ModelInfoWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->ModelInfoWidget->SetParent ( infoFrame->GetFrame() );
    this->ModelInfoWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->ModelInfoWidget->GetWidgetName(), 
                  infoFrame->GetFrame()->GetWidgetName());

   //this->ProcessGUIEvents (this->ModelDisplaySelectorWidget,
                          //vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );

    modLoadFrame->Delete ( );
    clipFrame->Delete ( );    
    infoFrame->Delete ( );

    // set up picking
    this->Init();
}

/*
//----------------------------------------------------------------------------
void vtkSlicerModelsGUI::SetViewerWidget ( vtkSlicerViewerWidget *viewerWidget )
{
  this->ViewerWidget = viewerWidget;
}

//----------------------------------------------------------------------------
void vtkSlicerModelsGUI::SetInteractorStyle( vtkSlicerViewerInteractorStyle *interactorStyle )
{
  // note: currently the GUICallbackCommand calls ProcessGUIEvents
  // remove observers
  if (this->InteractorStyle != NULL &&
      this->InteractorStyle->HasObserver(vtkSlicerViewerInteractorStyle::SelectRegionEvent, this->GUICallbackCommand) == 1)
    {
    this->InteractorStyle->RemoveObservers(vtkSlicerViewerInteractorStyle::SelectRegionEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  
  this->InteractorStyle = interactorStyle;

  // add observers
  if (this->InteractorStyle)
    {
    vtkDebugMacro("SetInteractorStyle: Adding observer on interactor style");
    this->InteractorStyle->AddObserver(vtkSlicerViewerInteractorStyle::SelectRegionEvent, (vtkCommand *)this->GUICallbackCommand);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModelsGUI::Init(void)
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (appGUI == NULL)
    {
    return;
    }
  
  // get the viewer widget
  this->SetViewerWidget(appGUI->GetActiveViewerWidget());

  // get the interactor style, to set up plotting events
  if (appGUI->GetActiveViewerWidget() != NULL &&
      appGUI->GetActiveViewerWidget()->GetMainViewer() != NULL &&
      appGUI->GetActiveViewerWidget()->GetMainViewer()->GetRenderWindowInteractor() != NULL &&
      appGUI->GetActiveViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle() != NULL)
    {
    this->SetInteractorStyle(vtkSlicerViewerInteractorStyle::SafeDownCast(appGUI->GetActiveViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle()));
    }
  else
    {
    vtkErrorMacro("Init: unable to get the interactor style, picking will not work.");
    }
}
*/
