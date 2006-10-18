#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"

#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkQueryAtlasGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasGUI );
vtkCxxRevisionMacro ( vtkQueryAtlasGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::vtkQueryAtlasGUI ( )
{
    this->Logic = NULL;
    //this->DebugOn();
}


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::~vtkQueryAtlasGUI ( )
{

    this->SetModuleLogic ( NULL );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "QueryAtlasGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::RemoveGUIObservers ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: RemoveGUIObservers\n");
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddGUIObservers ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: AddGUIObservers\n");
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // nothing to do here yet...
    vtkSlicerNodeSelectorWidget *colorSelector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
    return;
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{    
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::Enter ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: Enter\n");
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::Exit ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: Exit\n");
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  // Define your help text here.
  const char *help = "**Query Atlas:** allows interactive Google, Wikipedia, queries from within the 3D anatomical display.";

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "QueryAtlas", "QueryAtlas", NULL );

    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    
    // HELP FRAME
    vtkSlicerModuleCollapsibleFrame *modHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    modHelpFrame->SetParent (page );
    modHelpFrame->Create ( );
    modHelpFrame->CollapseFrame ( );
    modHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( modHelpFrame->GetFrame() );
    this->HelpText->Create ( );
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
    vtkSlicerModuleCollapsibleFrame *loadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadFrame->SetParent ( page );
    loadFrame->Create ( );
    loadFrame->SetLabelText ("Load Scene");
    loadFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  loadFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());
    
    // ---
    // QUERY FRAME
    vtkSlicerModuleCollapsibleFrame *queryFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    queryFrame->SetParent ( page );
    queryFrame->Create ( );
    queryFrame->SetLabelText ("Query Options");
    queryFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  queryFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());
    

    
    // deleting frame widgets
    queryFrame->Delete ( );
    loadFrame->Delete ( );
    modHelpFrame->Delete ( );
}


