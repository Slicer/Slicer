
#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUIUpdate.h"
#include "vtkSlicerLogicUpdate.h"
#include "vtkKWApplication.h"


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
    this->GUICommand = vtkSlicerLogicUpdate::New ( );
    this->GUICommand->SetGUI ( this );
    this->Logic = NULL;
    //this->SetApplication ( app );
    //this->SetParent ( this->GetApplication()->GetNthWindow(0)->GetViewFrame() );
}



//---------------------------------------------------------------------------
vtkSlicerComponentGUI::~vtkSlicerComponentGUI ( ) {

    if ( this->LogicCommand ) {
        this->LogicCommand->Delete ( );
    }
    if (this->GUICommand ) {
        this->GUICommand->Delete ( );
    }
    this->Logic = NULL;
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
void vtkSlicerComponentGUI::SetLogic ( vtkSlicerApplicationLogic *logic ) {
    this->Logic = logic;
}




//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::UpdateGUIWithLogicEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData ) {
    //    this->GUI->Get/Set commands
}


//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::UpdateLogicWithGUIEvents ( vtkObject *caller,
                                                 unsigned long event,
                                                 void *callData ) {
    //    this->Logic->Get/Set commands
}






