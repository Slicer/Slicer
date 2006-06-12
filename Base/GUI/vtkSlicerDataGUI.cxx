#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerDataGUI.h"

#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataGUI );
vtkCxxRevisionMacro ( vtkSlicerDataGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDataGUI::vtkSlicerDataGUI ( )
{

    // class not yet defined!
    //this->Logic = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerDataGUI::~vtkSlicerDataGUI ( )
{

    this->RemoveMRMLNodeObservers ( );
    this->RemoveLogicObservers ( );
    // class not yet defined!
    //this->SetModuleLogic ( NULL );
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
void vtkSlicerDataGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveLogicObservers ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveGUIObservers ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddGUIObservers ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
    // Fill in
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
void vtkSlicerDataGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Exit ( )
{
    // Fill in
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
    
    // HELP FRAME
    vtkKWFrameWithLabel *modHelpFrame = vtkKWFrameWithLabel::New ( );
    modHelpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
    modHelpFrame->Create ( );
    modHelpFrame->CollapseFrame ( );
    modHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());

    // ---
    // DISPLAY FRAME            
    vtkKWFrameWithLabel *modDisplayFrame = vtkKWFrameWithLabel::New ( );
    modDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
    modDisplayFrame->Create ( );
    //modDisplayFrame->SetBackgroundColor ( style->GetGUIBgColor() );
    modDisplayFrame->SetLabelText ("Display");
    modDisplayFrame->SetDefaultLabelFontWeightToNormal( );
    modDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  modDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());

    modHelpFrame->Delete ( );
    modDisplayFrame->Delete ( );
}





