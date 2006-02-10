
#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUIUpdate.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerComponentGUI);
vtkCxxRevisionMacro(vtkSlicerComponentGUI, "$Revision: 1.0 $");



//---------------------------------------------------------------------------
vtkSlicerComponentGUI::vtkSlicerComponentGUI ( ) {

    // Every gui has a helper class called LogicCommand
    // whose execute method propagates logic changes
    // into the GUI state.
    this->LogicCommand = vtkSlicerGUIUpdate::New ( );
    this->LogicCommand->SetGUI ( this );

    this->SlicerApp = NULL;
    this->KWapp = NULL;
    this->Win = NULL;
    this->Logic = NULL;
}



//---------------------------------------------------------------------------
vtkSlicerComponentGUI::~vtkSlicerComponentGUI ( ) {

    if ( this->LogicCommand ) {
        this->LogicCommand->Delete ( );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetWindow ( vtkKWWindowBase *win ) {
    this->Win = win;
}
//---------------------------------------------------------------------------
vtkKWWindowBase* vtkSlicerComponentGUI::GetWindow ( ) {
    return this->Win;
}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetSlicerApplication ( vtkSlicerApplicationGUI *app ) {
    
    this->SlicerApp = app;
    if ( app->GetKwApplication () ) {
        this->SetKwApplication ( app->GetKwApplication ( ) );
    }
}
//---------------------------------------------------------------------------
vtkSlicerApplicationGUI* vtkSlicerComponentGUI::GetSlicerApplication ( ) {
    return this->SlicerApp;
}




//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetKwApplication ( vtkKWApplication *app ) {
    this->KWapp = app;
}
//---------------------------------------------------------------------------
vtkKWApplication* vtkSlicerComponentGUI::GetKwApplication ( ) {
    return this->KWapp;
}





//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetLogic ( vtkSlicerApplicationLogic *logic ) {
    this->Logic = logic;
}

//---------------------------------------------------------------------------
vtkSlicerApplicationLogic* vtkSlicerComponentGUI::GetLogic ( ) {
    return this->Logic;
}




//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::ProcessCallbackCommandEvents ( vtkObject *caller,
                                                           unsigned long event,
                                                           void *callData ) {
    //    this->Logic->Get/Set commands
}

//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::ProcessLogicEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData ) {
    //    this->GUI->Get/Set commands
}






