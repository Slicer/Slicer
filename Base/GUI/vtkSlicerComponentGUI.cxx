#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUIUpdate.h"
#include "vtkSlicerLogicUpdate.h"
#include "vtkKWApplication.h"


//---------------------------------------------------------------------------
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
    this->Logic = NULL;
    this->GUIName = NULL;
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
    this->GUIName = NULL;
    this->SetLogic(NULL);
}








