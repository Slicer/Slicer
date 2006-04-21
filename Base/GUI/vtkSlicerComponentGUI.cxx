#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLScene.h"

#include "vtkKWApplication.h"



vtkCxxRevisionMacro ( vtkSlicerComponentGUI, "$Revision: 1.0 $" );
vtkStandardNewMacro ( vtkSlicerComponentGUI );


//---------------------------------------------------------------------------
vtkSlicerComponentGUI::vtkSlicerComponentGUI ( )
{
    // Set up callbacks
    this->MRMLCallbackCommand = vtkCallbackCommand::New ( );
    this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->MRMLCallbackCommand->SetCallback( vtkSlicerComponentGUI::MRMLCallback );

    this->LogicCallbackCommand = vtkCallbackCommand::New ( );
    this->LogicCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->LogicCallbackCommand->SetCallback( vtkSlicerComponentGUI::LogicCallback );

    this->GUICallbackCommand = vtkCallbackCommand::New ( );
    this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->GUICallbackCommand->SetCallback( vtkSlicerComponentGUI::GUICallback );
    
    // Set null pointers
    this->MRMLScene = NULL;
    this->ApplicationLogic = NULL;
    this->GUIName = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerComponentGUI::~vtkSlicerComponentGUI ( )
{
    // unregister and set null pointers.
    if ( this->MRMLCallbackCommand )
        {
            this->MRMLCallbackCommand->Delete ( );
            this->MRMLCallbackCommand = NULL;
        }
    if ( this->LogicCallbackCommand )
        {
            this->LogicCallbackCommand->Delete ( );
            this->LogicCallbackCommand = NULL;
        }
    if ( this->GUICallbackCommand )
        {
            this->GUICallbackCommand->Delete ( );
            this->GUICallbackCommand = NULL;
        }
        
    // remove observers if there are any,
    // and set null pointers.
    this->SetMRMLScene ( NULL );
    this->SetApplicationLogic ( NULL );
    this->GUIName = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerComponentGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "MRMLScene: " << this->GetMRMLScene ( ) << "\n";
    os << indent << "ApplicationLogic: " << this->GetApplicationLogic ( ) << "\n";
    os << indent << "GUIName: " << this->GetGUIName ( ) << "\n";
}


//---------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function that relays observed events from 
// the observed MRML node into the GUI's 'ProcessMRMLEvents" mediator method,
// which in turn makes appropriate changes to widgets in the GUI.
//
void vtkSlicerComponentGUI::MRMLCallback ( vtkObject *__caller,
                                           unsigned long eid, void *__clientData, void *callData)
{
    static int inCallback = 0;

    vtkSlicerComponentGUI *self = reinterpret_cast<vtkSlicerComponentGUI *>(__clientData);

    if ( inCallback )
        {
            vtkErrorWithObjectMacro ( self, "In vtkSlicerComponentGUI *!* MRMLCallback called recursively?");
            return;
        }

    vtkDebugWithObjectMacro ( self, "In vtkSlicerComponentGUI MRMLCallback");

    inCallback = 1;
    self->ProcessMRMLEvents ( __caller, eid, callData );
    inCallback = 0;
    
}




//---------------------------------------------------------------------------
// Description:
// the LogicCallback is a static function that relays observed events from 
// the observed logic into the GUI's 'ProcessLogicEvents" mediator method,
// which in turn makes appropriate changes to widgets in the GUI.
//
void vtkSlicerComponentGUI::LogicCallback ( vtkObject *__caller,
                                           unsigned long eid, void *__clientData, void *callData)
{
    static int inCallback = 0;

    vtkSlicerComponentGUI *self = reinterpret_cast<vtkSlicerComponentGUI *>(__clientData);

    if ( inCallback )
        {
            vtkErrorWithObjectMacro ( self, "In vtkSlicerComponentGUI *!* LogicCallback called recursively?");
            return;
        }

    vtkDebugWithObjectMacro ( self, "In vtkSlicerComponentGUI LogicCallback");

    inCallback = 1;
    self->ProcessLogicEvents ( __caller, eid, callData );
    inCallback = 0;

}





//---------------------------------------------------------------------------
// Description:
// the GUICallback is a static function that relays observed events from 
// observed widgets into the GUI's 'ProcessGUIEvents" mediator method, which in
// turn makes appropriate changes to the application layer.
//
void vtkSlicerComponentGUI::GUICallback ( vtkObject *__caller,
                                           unsigned long eid, void *__clientData, void *callData)
{
    static int inCallback = 0;

    vtkSlicerComponentGUI *self = reinterpret_cast<vtkSlicerComponentGUI *>(__clientData);

    if ( inCallback )
        {
            vtkErrorWithObjectMacro ( self, "In vtkSlicerComponentGUI *!* GUICallback called recursively?");
            return;
        }

    vtkDebugWithObjectMacro ( self, "In vtkSlicerComponentGUI GUICallback");

    inCallback = 1;
    self->ProcessGUIEvents ( __caller, eid, callData );
    inCallback = 0;

}



//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetMRML ( vtkObject **nodePtr, vtkObject *node )
{
    // Delete 
    if ( *nodePtr )
        {
            ( *nodePtr )->Delete ( );
            *nodePtr = NULL;
        }

    // Set 
    *nodePtr = node;

    // Register 
    if ( *nodePtr )
        {
            ( *nodePtr )->Register ( this );
        }
}


//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetAndObserveMRML ( vtkObject **nodePtr, vtkObject *node )
{
    // Remove observers and delete
    if ( *nodePtr )
        {
            ( *nodePtr )->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
            ( *nodePtr )->Delete ( );
            *nodePtr = NULL;
        }

    // Set
    *nodePtr = node;

    // Register and add observers
    if ( *nodePtr )
        {
            ( *nodePtr )->Register ( this );
            ( *nodePtr )->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
        }
}


//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetLogic ( vtkObject **logicPtr, vtkObject *logic )
{

    // Delete (check for observers first)
    if ( *logicPtr )
        {
            if ( ( *logicPtr )->HasObserver ( vtkCommand::ModifiedEvent, this->LogicCallbackCommand ) )
                {
                    ( *logicPtr )->RemoveObservers ( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
                }
            ( *logicPtr )->Delete ( );
            *logicPtr = NULL;
        }

    // Set 
    *logicPtr = logic;

    // Register 
    if ( *logicPtr )
        {
            ( *logicPtr )->Register ( this );
        }

}

//---------------------------------------------------------------------------
void vtkSlicerComponentGUI::SetAndObserveLogic ( vtkObject **logicPtr, vtkObject *logic )
{
    // Remove observers and delete
    if ( *logicPtr )
        {
            ( *logicPtr )->RemoveObservers ( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
            ( *logicPtr )->Delete ( );
            *logicPtr = NULL;
        }

    // Set
    *logicPtr = logic;

    // Register and add observers
    if ( *logicPtr )
        {
            ( *logicPtr )->Register ( this );
            ( *logicPtr )->AddObserver ( vtkCommand::ModifiedEvent, this->LogicCallbackCommand );
        }

}








