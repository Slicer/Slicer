#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUIUpdate.h"
#include "vtkSlicerLogicUpdate.h"
#include "vtkSlicerMRMLUpdate.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkKWApplication.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerComponentGUI, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerComponentGUI::vtkSlicerComponentGUI ( ) {

    // Every gui has several helper classes called
    // LogicCommand, GUICommand and MRMLCommand
    // whose execute method propagates logic changes
    // into the GUI state, or GUI changes into the logic state,
    // or mrml changes into the GUI state, respectively.
    this->LogicCommand = vtkSlicerLogicUpdate::New ( );
    this->LogicCommand->SetGUI ( this );
    this->GUICommand = vtkSlicerGUIUpdate::New ( );
    this->GUICommand->SetGUI ( this );
    this->MRMLCommand = vtkSlicerMRMLUpdate::New ( );
    this->MRMLCommand->SetGUI ( this );
    // Set null pointers to logic and mrml.
    this->SetApplicationLogic ( NULL );
    this->SetMRMLScene ( NULL );
    this->SetLogic ( NULL );
    // Set GUI's name to be NULL.
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
    if (this->MRMLCommand ) {
        this->MRMLCommand->Delete ( );
        this->MRMLCommand = NULL;
    }
    this->GUIName = NULL;
    this->SetApplicationLogic ( NULL );
    this->SetLogic ( NULL );
    this->SetMRMLScene ( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetApplicationLogic ( vtkSlicerApplicationLogic *logic ) {

    // Don't bother if already set.
    if ( logic == this->ApplicationLogic ) {
        return;
    }
    // Remove observers from application logic
    if ( this->ApplicationLogic != NULL ) {
        this->RemoveApplicationLogicObservers ( );
    }
    // Set pointer and add observers if not null
    this->ApplicationLogic = logic;
    if ( this->ApplicationLogic != NULL ) {
        this->AddApplicationLogicObservers ( );
    }
}




//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetLogic ( vtkSlicerLogic *logic ) {

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



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetMRMLScene ( vtkMRMLScene *mrml ) {

    // Don't bother if already set.
    if ( mrml == this->MRMLScene ) {
        return;
    }
    // Remove observers from application logic
    if ( this->MRMLScene != NULL ) {
        this->RemoveMRMLObservers ( );
    }
    // Set pointer and add observers if not null
    this->MRMLScene = mrml;
    if ( this->MRMLScene != NULL ) {
        this->AddMRMLObservers ( );
    }
}







