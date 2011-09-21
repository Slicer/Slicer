
#include "vtkSlicerApplication.h"

#include "vtkModelMirrorGUI.h"

// vtkSlicer includes
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerApplicationGUI.h"

// KWWidgets includes
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWEvent.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"

// STD includes
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <iterator>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkModelMirrorGUI);


//----------------------------------------------------------------------------
vtkModelMirrorGUI::vtkModelMirrorGUI()
{
  this->UpdatingMRML = 0;
  this->UpdatingGUI = 0;
    
  this->Logic = NULL;
  this->ModelSelector = NULL;
  this->ModelNameEntry = NULL;
  this->GoButton = NULL;
  this->ModelMirrorIcons = NULL;
  this->AxialMirrorButton = NULL;
  this->SaggitalMirrorButton = NULL;
  this->CoronalMirrorButton = NULL;

  this->SpecificationFrame = NULL;
  this->Raised = false;

  // Try to load supporting libraries dynamically.  This is needed
  // since the toplevel is a loadable module but the other libraries
  // didn't get loaded
  Tcl_Interp* interp = this->GetApplication()->GetMainInterp();
  if (interp)
    {
    Vtkslicermodelmirrormodulelogic_Init(interp);
    }
  else
    {
    vtkErrorMacro("Failed to obtain reference to application TCL interpreter");
    }
}

//----------------------------------------------------------------------------
vtkModelMirrorGUI::~vtkModelMirrorGUI()
{
    this->RemoveMRMLNodeObservers ( );
    this->RemoveLogicObservers ( );

    this->UpdatingMRML = 0;
    this->UpdatingGUI = 0;

    if ( this->SpecificationFrame )
      {
      this->SpecificationFrame->SetParent ( NULL );
      this->SpecificationFrame->Delete();
      this->SpecificationFrame = NULL;
      }
    if ( this->ModelSelector )
      {
      this->ModelSelector->SetParent (NULL );
      this->ModelSelector->Delete();
      this->ModelSelector = NULL;
      }
    if ( this->ModelNameEntry )
      {
      this->ModelNameEntry->SetParent ( NULL );
      this->ModelNameEntry->Delete();
      this->ModelNameEntry = NULL;
      }
    if ( this->GoButton )
      {
      this->GoButton->SetParent ( NULL );
      this->GoButton->Delete();
      this->GoButton = NULL;
      }
    if ( this->AxialMirrorButton )
      {
      this->AxialMirrorButton->SetParent ( NULL );
      this->AxialMirrorButton->Delete();
      this->AxialMirrorButton = NULL;
      }
    if ( this->SaggitalMirrorButton )
      {
      this->SaggitalMirrorButton->SetParent ( NULL );
      this->SaggitalMirrorButton->Delete();
      this->SaggitalMirrorButton = NULL;      
      }
    if ( this->CoronalMirrorButton )
      {
      this->CoronalMirrorButton->SetParent ( NULL );
      this->CoronalMirrorButton->Delete();
      this->CoronalMirrorButton = NULL;      
      }
    if ( this->ModelMirrorIcons )
      {
      this->ModelMirrorIcons->Delete();
      this->ModelMirrorIcons = NULL;
      }
    
    if ( this->Logic )
      {
      this->Logic->Delete();
      this->Logic = NULL;
      }

    this->Raised = false;
//    this->SetAndObserveMRMLScene ( NULL );    
}



//----------------------------------------------------------------------------
void vtkModelMirrorGUI::Enter()
{
  
  
  //--- mark as currently being visited.
  this->Raised = true;

  //--- mark as visited at least once.
  this->Visited = true;

  //--- only build when first visited.
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->AddObserver ( vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand() );

    //--- Do a parallel thing in Logic
    this->Logic->Enter();

    //--- Set up GUI observers 
    vtkIntArray *guiEvents = this->NewObservableEvents ( );
    if ( guiEvents != NULL )
      {
      this->SetAndObserveMRMLSceneEvents ( this->MRMLScene, guiEvents );
      guiEvents->Delete();
      }
    }

  this->AddGUIObservers();    
  this->CreateModuleEventBindings();

  //--- make GUI reflect current MRML state
  this->UpdateGUI();
}


//----------------------------------------------------------------------------
void vtkModelMirrorGUI::Exit ( )
{

  //--- mark as no longer selected.
  this->Raised = false;

  this->RemoveGUIObservers();
  this->ReleaseModuleEventBindings();
//  this->SetAndObserveMRMLScene ( NULL );
  
  //--- Do a parallel thing in Logic
  this->Logic->Exit();
}


//----------------------------------------------------------------------------
vtkIntArray *vtkModelMirrorGUI::NewObservableEvents()
{
  if ( !this->Visited )
    {
    return NULL;
    }
  
 vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkModelMirrorGUI::TearDownGUI ( )
{
  if ( !this->Built )
    {
    return;
    }

  this->RemoveObservers ( vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand() );
  this->RemoveGUIObservers ( );
  this->ModelSelector->SetMRMLScene ( NULL );
  this->ReleaseModuleEventBindings();
  this->Logic->SetModelMirrorNode ( NULL );
  this->SetLogic ( NULL );
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkModelMirrorGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


//---------------------------------------------------------------------------
void vtkModelMirrorGUI::AddGUIObservers ( ) 
{
  if ( !this->Built )
    {
    return;
    }

  //--- include this to enable lazy building
  if ( !this->Visited )
    {
    return;
    }

  if ( this->ModelSelector )
    {
    if  (this->MRMLScene != NULL )
      {
      this->ModelSelector->SetMRMLScene ( this->MRMLScene );
      }
    this->ModelSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                             ( vtkCommand *) this->GUICallbackCommand );
    this->ModelSelector->UpdateMenu();
    }

  if ( this->ModelNameEntry )
    {
    this->ModelNameEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *) this->GUICallbackCommand );
    }
  if ( this->GoButton )
    {
    this->GoButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkModelMirrorGUI::RemoveGUIObservers ( )
{
  if ( !this->Built )
    {
    return;
    }

  //--- include this to enable lazy building
  if ( !this->Visited )
    {
    return;
    }
  if ( this->ModelSelector  )
    {
    this->ModelSelector->SetMRMLScene ( NULL );
    this->ModelSelector ->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                                            ( vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ModelNameEntry )
    {
    this->ModelNameEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *) this->GUICallbackCommand );
    }
  if ( this->GoButton )
    {
    this->GoButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}




//---------------------------------------------------------------------------
void vtkModelMirrorGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in.
}

//---------------------------------------------------------------------------
void vtkModelMirrorGUI::RemoveLogicObservers ( ) {
    // Fill in
}




//---------------------------------------------------------------------------
void vtkModelMirrorGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *vtkNotUsed(callData))
{

  if ( !this->Built )
    {
    return;
    }
  if ( this->Logic->GetModelMirrorNode() == NULL )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got NULL ModelMirrorNode" );
    return;
    }
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got NULL Module Logic" );
    return;
    }

  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  vtkSlicerNodeSelectorWidget *s = vtkSlicerNodeSelectorWidget::SafeDownCast ( caller );

  if ( s == this->ModelSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast ( this->ModelSelector->GetSelected() );
    if ( model != NULL )
      {
      this->Logic->GetModelMirrorNode()->SetInputModel ( model );
      return;
      }
    }

  if (b != NULL && b == this->GoButton && event == vtkKWPushButton::InvokedEvent )
    {

    if ( this->Logic->GetModelMirrorNode()->GetInputModel() == NULL )
      {
      vtkKWMessageDialog *d = vtkKWMessageDialog::New();
      d->SetParent ( this->UIPanel->GetPageWidget ( "ModelMirror" ));
      d->SetStyleToMessage ();
      std::string msg = "Please select a model to mirror.";
      d->SetText ( msg.c_str() );
      d->Create();
      d->Invoke();
      d->Delete();
      return;
      }

    // swallow the event
    if (this->GUICallbackCommand != NULL)
      {
      this->GUICallbackCommand->SetAbortFlag(1);
      }
    else
      {
      vtkErrorMacro("Unable to get the gui call back command that calls process widget events, event = " << event << " is not swallowed here");
      }
    
    this->Logic->CreateMirrorModel();

    this->Logic->CreateMirrorTransform();
    if ( this->Logic->GetMirrorTransformNode() != NULL )
      {
      if ( this->Logic->HardenTransform() )
        {
        //--- clean up 
        this->Logic->DeleteMirrorTransform();
        if ( this->Logic->FlipNormals() )
          {
          if ( this->Logic->PositionInHierarchy() )
            {
            return;
            }
          }
        }
      }

    
    vtkErrorMacro ( "Unable to create the new mirror model." );
    vtkKWMessageDialog *d2 = vtkKWMessageDialog::New();
    d2->SetParent ( this->UIPanel->GetPageWidget ( "ModelMirror" ));
    d2->SetStyleToMessage ();
    std::string msg = "Error creating the mirror model..";
    d2->SetText ( msg.c_str() );
    d2->Create();
    d2->Invoke();
    d2->Delete();
    }

  if ( e && event == vtkKWEntry::EntryValueChangedEvent )
    {
    if ( e->GetValue() != NULL && (strcmp(e->GetValue(), "" ) ))
      {
      this->Logic->GetModelMirrorNode()->SetNewModelName ( e->GetValue() );
      this->UpdateGUI ( );
      }
    return;
    }
}



//---------------------------------------------------------------------------
void vtkModelMirrorGUI::SetSlicerText(const char *txt)
{
  if ( this->GetApplicationGUI() )
    {
    if ( this->GetApplicationGUI()->GetMainSlicerWindow() )
      {
      this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
      }
    }
}





//---------------------------------------------------------------------------
void vtkModelMirrorGUI::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                          unsigned long vtkNotUsed(event),
                                          void *vtkNotUsed(callData))
{
  if ( !this->Raised )
    {
    return;
    }
  if ( !this->Visited )
    {
    return;
    }
  if ( this->Logic->GetModelMirrorNode() == NULL )
    {
    vtkErrorMacro ("ProcessMRMLEvents has a NULL ModelMirrorNode");
    return;
    }

  if ( this->ApplicationGUI == NULL )
    {
    vtkErrorMacro ("ProcessMRMLEvents: ApplicationGUI is null");
    return;    
    }

  //vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
}




//---------------------------------------------------------------------------
void vtkModelMirrorGUI::UpdateGUI ()
{
  if ( !this->Built )
    {
    return;
    }
  
  // update from MRML
  if ( this->UpdatingMRML )
    {
    return;
    }
  if ( this->UpdatingGUI )
    {
    return;
    }
  
  this->UpdatingGUI = 1;

  if (this->Logic->GetModelMirrorNode() != NULL)
    {
    if ( this->ModelNameEntry->GetValue() != NULL && this->Logic->GetModelMirrorNode()->GetNewModelName() != NULL )
      {
      if ( strcmp ( this->ModelNameEntry->GetValue(), this->Logic->GetModelMirrorNode()->GetNewModelName() ) )
        {
        this->ModelNameEntry->SetValue ( this->Logic->GetModelMirrorNode()->GetNewModelName() );
        }
      }
    }
  else
    {
    vtkErrorMacro ("ModelMirrorGUI: UpdateGUI has a NULL ModelMirrorNode." );
    }
  this->UpdatingGUI = 0;
}






//---------------------------------------------------------------------------
void vtkModelMirrorGUI::BuildGUI ( ) 
{
  //--- include this to enable lazy building
  if ( !this->Visited )
    {
    return;
    }
  
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildGUI: got NULL MainSlicerWindow");
    return;
    }
  win->SetStatusText ( "Building Interface for ModelMirror Module...." );
  app->Script ( "update idletasks" );


  if ( this->MRMLScene != NULL )
    {
    vtkMRMLModelMirrorNode* m = vtkMRMLModelMirrorNode::New();
    this->MRMLScene->RegisterNodeClass(m);
    m->Delete();

    vtkMRMLModelMirrorNode* modelMirrorNode = this->Logic->GetModelMirrorNode();
    if (modelMirrorNode == NULL )
      {
      modelMirrorNode = vtkMRMLModelMirrorNode::New();
      this->Logic->SetModelMirrorNode( modelMirrorNode );
      }
    modelMirrorNode->Delete(); 
    }
  else
    {
    vtkErrorMacro("GUI is being built before MRML Scene is set");
    return;
    }
      
  this->UIPanel->AddPage ( "ModelMirror", "ModelMirror", NULL );

  // HELP FRAME
  const char* about = "ModelMirror was developed by Wendy Plesniak with help from Marianna Jakab and Steve Pieper. This work was supported by NA-MIC, NAC, BIRN, NCIGT, Harvard CTSC, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\n";
  
  const char *help = "For detailed documentation on this module, please see: <a>http://www.slicer.org/slicerWiki/index.php/Modules:ModelMirror-Documentation-3.6</a>.\n\n**ModelMirror** is a module for creating a new model which is a mirrored version of another selected polygonal model, using a selected mirroring axis. \n\n **Notes about performance:.** The method used in this module recomputes normal vectors for models comprised of polygons or triangle strips ONLY. \n\n **Usage:** To use this module, select an existing model from the scene, and select the desired mirroring axis using the set of radiobutton icons. Select a name for the model to be generated and then click the **Create Mirror** button. The new model will be created, added to the scene, and displayed in the 3D viewer.\n\n";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ModelMirror" );
  this->BuildHelpAndAboutFrame ( page, help, about );
  vtkKWLabel *NACLabel = vtkKWLabel::New();
  NACLabel->SetParent ( this->GetLogoFrame() );
  NACLabel->Create();
  NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

  vtkKWLabel *NAMICLabel = vtkKWLabel::New();
  NAMICLabel->SetParent ( this->GetLogoFrame() );
  NAMICLabel->Create();
  NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

  vtkKWLabel *NCIGTLabel = vtkKWLabel::New();
  NCIGTLabel->SetParent ( this->GetLogoFrame() );
  NCIGTLabel->Create();
  NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
  vtkKWLabel *BIRNLabel = vtkKWLabel::New();
  BIRNLabel->SetParent ( this->GetLogoFrame() );
  BIRNLabel->Create();
  BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );

  vtkKWLabel *CTSCLabel = vtkKWLabel::New();
  CTSCLabel->SetParent ( this->GetLogoFrame() );
  CTSCLabel->Create();
  CTSCLabel->SetImageToIcon (this->GetAcknowledgementIcons()->GetCTSCLogo() );


  app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky e", NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky e",  NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky e",  BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky e",  NCIGTLabel->GetWidgetName());                  
    app->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 2 -sticky w",  CTSCLabel->GetWidgetName());                  
    app->Script ( "grid columnconfigure %s 0 -weight 0", this->GetLogoFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 0", this->GetLogoFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 1", this->GetLogoFrame()->GetWidgetName() );

  NACLabel->Delete();
  NAMICLabel->Delete();
  NCIGTLabel->Delete();
  BIRNLabel->Delete();
  CTSCLabel->Delete();  

  // create icons
  this->ModelMirrorIcons = vtkModelMirrorIcons::New();

  // MAIN UI FRAME
  this->SpecificationFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  SpecificationFrame->SetParent ( this->UIPanel->GetPageWidget ( "ModelMirror" ) );
  SpecificationFrame->Create ( );
  SpecificationFrame->ExpandFrame ( );
  SpecificationFrame->SetLabelText ("Mirror Surface Models");
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2 -in %s",
                  SpecificationFrame->GetWidgetName(), this->UIPanel->GetPageWidget("ModelMirror")->GetWidgetName());

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( this->SpecificationFrame->GetFrame() );
  f->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",  f->GetWidgetName() );

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetAnchorToEast();
  l1->SetText ( "Model to mirror:" );
  
  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetAnchorToEast();
  l2->SetText  ("New model name:" );

  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( f );
  l3->Create();
  l3->SetAnchorToEast();
  l3->SetText ( "Set mirror type:" );

  vtkKWLabel *l4 = vtkKWLabel::New();
  l4->SetParent ( f );
  l4->Create();
  l4->SetAnchorToEast();
  l4->SetText ( "" );

  this->ModelSelector = vtkSlicerNodeSelectorWidget::New();
  this->ModelSelector->SetParent( f );
  this->ModelSelector->Create();
  this->ModelSelector->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->ModelSelector->SetChildClassesEnabled(0);
  this->ModelSelector->SetShowHidden (1);
  this->ModelSelector->SetMRMLScene(this->GetMRMLScene());
  this->ModelSelector->GetWidget()->GetWidget()->SetWidth (24 );
  this->ModelSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ModelSelector->SetBorderWidth(2);
  this->ModelSelector->SetPadX(2);
  this->ModelSelector->SetPadY(2);
  this->ModelSelector->SetLabelText( "");
  this->ModelSelector->SetBalloonHelpString("Select a surface model to mirror.");

  this->ModelNameEntry = vtkKWEntry::New();
  this->ModelNameEntry->SetParent ( f );
  this->ModelNameEntry->Create();
  this->ModelNameEntry->SetWidth ( 24 );
  this->ModelNameEntry->SetCommandTriggerToAnyChange();
  this->ModelNameEntry->SetBalloonHelpString ( "Specify a name for the model to be created." );  

  vtkKWFrame *f2 = vtkKWFrame::New();
  f2->SetParent ( f );
  f2->Create();

  this->AxialMirrorButton = vtkKWPushButton::New();
  this->AxialMirrorButton->SetParent ( f2 );
  this->AxialMirrorButton->Create();
  this->AxialMirrorButton->SetBorderWidth ( 0 );
  this->AxialMirrorButton->SetReliefToFlat();
  this->AxialMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetAxialHIIcon() );
  this->AxialMirrorButton->SetCommand ( this, "SetMirrorPlaneToAxial" );
  this->AxialMirrorButton->SetBalloonHelpString ( "Mirror selected model around axial plane ((AP/RL)" );
  this->Logic->GetModelMirrorNode()->SetMirrorPlane(vtkMRMLModelMirrorNode::AxialMirror );
  
  this->SaggitalMirrorButton = vtkKWPushButton::New();
  this->SaggitalMirrorButton->SetParent ( f2 );
  this->SaggitalMirrorButton->Create();
  this->SaggitalMirrorButton->SetBorderWidth ( 0 );
  this->SaggitalMirrorButton->SetReliefToFlat();
  this->SaggitalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetSaggitalLOIcon() );
  this->SaggitalMirrorButton->SetCommand ( this, "SetMirrorPlaneToSaggital" );
  this->SaggitalMirrorButton->SetBalloonHelpString ( "Mirror selected model around saggital plane (SI/AP)" );

  this->CoronalMirrorButton = vtkKWPushButton::New();
  this->CoronalMirrorButton->SetParent ( f2 );
  this->CoronalMirrorButton->Create();
  this->CoronalMirrorButton->SetBorderWidth ( 0 );
  this->CoronalMirrorButton->SetReliefToFlat();
  this->CoronalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetCoronalLOIcon() );
  this->CoronalMirrorButton->SetCommand ( this, "SetMirrorPlaneToCoronal" );
  this->CoronalMirrorButton->SetBalloonHelpString ( "Mirror selected model around coronal plane ( RL/SI)" );

  app->Script ( "pack %s %s %s -side left -anchor w -padx 2 -pady 0",
                this->AxialMirrorButton->GetWidgetName(),
                this->SaggitalMirrorButton->GetWidgetName(),
                this->CoronalMirrorButton->GetWidgetName() );

  this->GoButton = vtkKWPushButton::New();
  this->GoButton->SetParent ( f );
  this->GoButton->Create();
  this->GoButton->SetText ("Create Mirror" );
  this->GoButton->SetBalloonHelpString ( "Create a new model as a mirror of selected model.");


  app->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 4", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 4", this->ModelSelector->GetWidgetName() );

  app->Script ( "grid %s -row 1 -column 0 -sticky e -padx 2 -pady 4", l3->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky w -padx 3 -pady 4", f2->GetWidgetName() );

  app->Script ( "grid %s -row 2 -column 0 -sticky e -padx 2 -pady 4", l2->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 1 -sticky ew -padx 5 -pady 4", this->ModelNameEntry->GetWidgetName() );

  app->Script ( "grid %s -row 3 -column 0 -sticky e -padx 5 -pady 10", l4->GetWidgetName() );
  app->Script ( "grid %s -row 3 -column 1 -sticky ew -padx 5 -pady 10", this->GoButton->GetWidgetName() );

  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  l3->Delete();
  l4->Delete();
  f2->Delete();
  f->Delete();

  this->Init();
  this->Built = true;

}


//---------------------------------------------------------------------------
void vtkModelMirrorGUI::SetMirrorPlaneToAxial()
{
//--- mrml 
  this->Logic->GetModelMirrorNode()->SetMirrorPlane( vtkMRMLModelMirrorNode::AxialMirror );
//--- gui
  this->AxialMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetAxialHIIcon() );
  this->SaggitalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetSaggitalLOIcon() );
  this->CoronalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetCoronalLOIcon() );
}

//---------------------------------------------------------------------------
void vtkModelMirrorGUI::SetMirrorPlaneToSaggital()
{
//--- mrml 
  this->Logic->GetModelMirrorNode()->SetMirrorPlane( vtkMRMLModelMirrorNode::SaggitalMirror );
//---gui
  this->AxialMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetAxialLOIcon() );
  this->SaggitalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetSaggitalHIIcon() );
  this->CoronalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetCoronalLOIcon() );

}


//---------------------------------------------------------------------------
void vtkModelMirrorGUI::SetMirrorPlaneToCoronal()
{
//--- mrml
  this->Logic->GetModelMirrorNode()->SetMirrorPlane( vtkMRMLModelMirrorNode::CoronalMirror );
//--- gui
  this->AxialMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetAxialLOIcon() );
  this->SaggitalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetSaggitalLOIcon() );
  this->CoronalMirrorButton->SetImageToIcon ( this->ModelMirrorIcons->GetCoronalHIIcon() );

}

//---------------------------------------------------------------------------
void vtkModelMirrorGUI::Init ( )
{
}
