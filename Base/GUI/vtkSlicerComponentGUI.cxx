#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUIUpdate.h"
#include "vtkSlicerLogicUpdate.h"
#include "vtkSlicerMrmlUpdate.h"
#include "vtkKWApplication.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerComponentGUI);
vtkCxxRevisionMacro(vtkSlicerComponentGUI, "$Revision: 1.0 $");



//---------------------------------------------------------------------------
vtkSlicerComponentGUI::vtkSlicerComponentGUI ( ) {

    // Every gui has a helper class called LogicCommand
    // whose execute method propagates logic changes
    // into the GUI state.
    this->LogicCommand = vtkSlicerLogicUpdate::New ( );
    this->LogicCommand->SetGUI ( this );
    this->GUICommand = vtkSlicerGUIUpdate::New ( );
    this->GUICommand->SetGUI ( this );
    this->MrmlCommand = vtkSlicerMrmlUpdate::New ( );
    this->MrmlCommand->SetGUI ( this );
    this->Logic = NULL;
    this->Mrml = NULL;

}



//---------------------------------------------------------------------------
vtkSlicerComponentGUI::~vtkSlicerComponentGUI ( ) {

    if ( this->LogicCommand ) {
        this->LogicCommand->Delete ( );
        this->LogicCommand = NULL;
    }
    if (this->GUICommand ) {
        this->GUICommand->Delete ( );
        this->GUICommand = NULL;
    }
    if (this->MrmlCommand ) {
        this->MrmlCommand->Delete ( );
        this->MrmlCommand = NULL;
    }
    this->Logic = NULL;
    this->Mrml = NULL;
    this->Parent = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::BuildGUI ( ) {

    // instance, configure and pack all GUI elements.
}


//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetParent ( vtkKWFrame *frame ) {
    this->Parent = frame;
    
}




//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::ProcessLogicEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData ) {

}


//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::ProcessGUIEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData ) {

}


//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::ProcessMrmlEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData ) {

}






