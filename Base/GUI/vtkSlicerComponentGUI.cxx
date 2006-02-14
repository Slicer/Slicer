
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

    this->SlicerApplication = NULL;
    this->KWApplication = NULL;
    this->KWWindow = NULL;
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
    this->KWWindow = win;
}
//---------------------------------------------------------------------------
vtkKWWindowBase* vtkSlicerComponentGUI::GetWindow ( ) {
    return this->KWWindow;
}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetSlicerApplication ( vtkSlicerApplicationGUI *app ) {
    
    this->SlicerApplication = app;
    if ( app->GetKWApplication () ) {
        this->SetKWApplication ( app->GetKWApplication ( ) );
    }
}
//---------------------------------------------------------------------------
vtkSlicerApplicationGUI* vtkSlicerComponentGUI::GetSlicerApplication ( ) {
    return this->SlicerApplication;
}




//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetKWApplication ( vtkKWApplication *app ) {
    this->KWApplication = app;
}
//---------------------------------------------------------------------------
vtkKWApplication* vtkSlicerComponentGUI::GetKWApplication ( ) {
    return this->KWApplication;
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






