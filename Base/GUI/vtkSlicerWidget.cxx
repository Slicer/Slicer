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
  this->GUICallbackCommand->SetCallback( vtkSlicerWidget::WidgetCallback );
  
  // Set null pointers
  this->MRMLScene = NULL;
  this->InWidgetCallbackFlag = 0;
  this->InMRMLCallbackFlag = 0;
}


//---------------------------------------------------------------------------
vtkSlicerWidget::~vtkSlicerWidget ( )
{
  // remove observers if there are any,
  // and set null pointers.
  this->SetAndObserveMRMLScene ( NULL );

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
  
}



//---------------------------------------------------------------------------
void vtkSlicerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "MRMLScene: " << this->GetMRMLScene ( ) << "\n";
}


//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
void 
vtkSlicerWidget::MRMLCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
{
  vtkSlicerWidget *self = reinterpret_cast<vtkSlicerWidget *>(clientData);

  if (self->GetInMRMLCallbackFlag())
    {
    vtkErrorWithObjectMacro(self, "In vtkSlicerWidget *********MRMLCallback called recursively?");
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerWidget MRMLCallback");

  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
}

//---------------------------------------------------------------------------
// Description:
// the WidgetCallback is a static function that relays observed events from 
// observed widgets into the GUI's 'ProcessWidgetEvents" mediator method, which in
// turn makes appropriate changes to the application layer.
//
void 
vtkSlicerWidget::WidgetCallback(vtkObject *caller, 
            unsigned long eid, void *clientData, void *callData)
{
  vtkSlicerWidget *self = reinterpret_cast<vtkSlicerWidget *>(clientData);

  if (self->GetInWidgetCallbackFlag())
    {
    vtkErrorWithObjectMacro(self, "In vtkSlicerWidget *********WidgetCallback called recursively?");
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerWidget WidgetCallback");

  self->SetInWidgetCallbackFlag(1);
  self->ProcessWidgetEvents(caller, eid, callData);
  self->SetInWidgetCallbackFlag(0);
}


//---------------------------------------------------------------------------
void vtkSlicerWidget::SetMRML ( vtkObject **nodePtr, vtkObject *node )
{
  // Delete 
  if ( *nodePtr )
    {
    ( *nodePtr )->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
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







