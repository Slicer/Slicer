#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkSlicerWidget.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLScene.h"

#include "vtkKWApplication.h"



vtkCxxRevisionMacro ( vtkSlicerWidget, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerWidget::vtkSlicerWidget ( )
{
    // Set up callbacks
    this->MRMLCallbackCommand = vtkCallbackCommand::New ( );
    this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->MRMLCallbackCommand->SetCallback( vtkSlicerWidget::MRMLCallback );

    this->GUICallbackCommand = vtkCallbackCommand::New ( );
    this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->GUICallbackCommand->SetCallback( vtkSlicerWidget::GUICallback );
    
    // Set null pointers
    this->MRMLScene = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerWidget::~vtkSlicerWidget ( )
{
  // unregister and set null pointers.
  if ( this->MRMLCallbackCommand )
    {
    this->MRMLCallbackCommand->Delete ( );
    this->MRMLCallbackCommand = NULL;
    }
  if ( this->GUICallbackCommand )
    {
    this->GUICallbackCommand->Delete ( );
    this->GUICallbackCommand = NULL;
    }
  
  // remove observers if there are any,
  // and set null pointers.
  this->SetMRMLScene ( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "MRMLScene: " << this->GetMRMLScene ( ) << "\n";
}


//---------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function that relays observed events from 
// the observed MRML node into the GUI's 'ProcessMRMLEvents" mediator method,
// which in turn makes appropriate changes to widgets in the GUI.
//
void vtkSlicerWidget::MRMLCallback ( vtkObject *__caller,
                                    unsigned long eid, void *__clientData, void *callData)
{
  static int inCallback = 0;
  
  vtkSlicerWidget *self = reinterpret_cast<vtkSlicerWidget *>(__clientData);
  
  if ( inCallback )
    {
    vtkErrorWithObjectMacro ( self, "In vtkSlicerWidget *!* MRMLCallback called recursively?");
    return;
    }
  
  vtkDebugWithObjectMacro ( self, "In vtkSlicerWidget MRMLCallback");
  
  inCallback = 1;
  self->ProcessMRMLEvents ( __caller, eid, callData );
  inCallback = 0;
}

//---------------------------------------------------------------------------
// Description:
// the GUICallback is a static function that relays observed events from 
// observed widgets into the GUI's 'ProcessWidgetEvents" mediator method, which in
// turn makes appropriate changes to the application layer.
//
void vtkSlicerWidget::GUICallback ( vtkObject *__caller,
                                   unsigned long eid, void *__clientData, void *callData)
{
  static int inCallback = 0;
  
  vtkSlicerWidget *self = reinterpret_cast<vtkSlicerWidget *>(__clientData);
  
  if ( inCallback )
    {
    vtkErrorWithObjectMacro ( self, "In vtkSlicerWidget *!* GUICallback called recursively?");
    return;
    }
  
  vtkDebugWithObjectMacro ( self, "In vtkSlicerWidget GUICallback");
  
  inCallback = 1;
  self->ProcessWidgetEvents ( __caller, eid, callData );
  inCallback = 0;
  
}



//---------------------------------------------------------------------------
void vtkSlicerWidget::SetMRML ( vtkObject **nodePtr, vtkObject *node )
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
void vtkSlicerWidget::SetAndObserveMRML ( vtkObject **nodePtr, vtkObject *node )
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








