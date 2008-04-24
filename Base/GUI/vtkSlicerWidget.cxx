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
  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->AssignOwner( this );
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkSlicerWidget::MRMLCallback);
  this->MRMLCallbackCommand = this->MRMLObserverManager->GetCallbackCommand();

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

  if (this->MRMLObserverManager)
    {
    this->MRMLObserverManager->AssignOwner( NULL );
    this->MRMLObserverManager->Delete();
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

  if (self == NULL)
    {
    return;
    }
  
  if (self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerWidget *********MRMLCallback called recursively?");
#endif
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
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerWidget *********WidgetCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerWidget WidgetCallback");

  self->SetInWidgetCallbackFlag(1);
  self->ProcessWidgetEvents(caller, eid, callData);
  self->SetInWidgetCallbackFlag(0);
}







