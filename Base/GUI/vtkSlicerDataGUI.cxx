#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerStyle.h"
#include "vtkKWFrameWithLabel.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataGUI );
vtkCxxRevisionMacro ( vtkSlicerDataGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDataGUI::vtkSlicerDataGUI ( ) {

    //this->SetLogic ( NULL );

}


//---------------------------------------------------------------------------
vtkSlicerDataGUI::~vtkSlicerDataGUI ( ) {

    //this->SetLogic ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveGUIObservers ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddGUIObservers ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddLogicObserver ( vtkSlicerModuleLogic *logic, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveLogicObserver ( vtkSlicerModuleLogic *logic, int event ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddLogicObservers ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveLogicObservers ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddMRMLObserver ( vtkMRMLNode *node, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveMRMLObserver ( vtkMRMLNode *node, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddMRMLObservers ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveMRMLObservers ( ) {
    // Fill in
}


    /*
//---------------------------------------------------------------------------
void vtkSlicerDataGUI::SetLogic ( vtkSlicerDataLogic *logic ) {

    // Don't bother if already set.
    if ( logic == this->Logic ) {
        return;
    }
    // Remove observers from application logic
    if ( this->Logic != NULL ) {
        this->RemoveLogicObservers ( );
    }
    // Set pointer and add observers if not null
    this->Logic = logic;
    if ( this->Logic != NULL ) {
        this->AddLogicObservers ( );
    }
}
     */


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessGUIEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData ) {
    // Fill in
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessLogicEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Enter ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Exit ( ) {
    // Fill in
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





