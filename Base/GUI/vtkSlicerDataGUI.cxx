#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerStyle.h"
#include "vtkKWFrameWithLabel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataGUI );
vtkCxxRevisionMacro ( vtkSlicerDataGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDataGUI::vtkSlicerDataGUI ( ) {

    //this->DataLogic = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerDataGUI::~vtkSlicerDataGUI ( ) {

    //this->DataLogic = NULL;
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveGUIObservers ( ) {

}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveLogicObservers ( ) {
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddGUIObservers ( ) {

}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddLogicObservers ( ) {

}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessGUIEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData ) {
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessLogicEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData ) {

}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Enter ( ) {
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Exit ( ) {
}




//---------------------------------------------------------------------------
void vtkSlicerDataGUI::BuildGUI ( ) {

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerStyle *style = app->GetSlicerStyle();

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





