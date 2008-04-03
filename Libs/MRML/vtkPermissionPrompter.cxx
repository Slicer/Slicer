
#include "vtkPermissionPrompter.h"
#include "vtkCallbackCommand.h"


vtkStandardNewMacro ( vtkPermissionPrompter );
vtkCxxRevisionMacro ( vtkPermissionPrompter, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkPermissionPrompter::vtkPermissionPrompter()
{
  this->Username = NULL;
  this->Password = NULL;
  this->HostName = NULL;
  this->PromptMessage = NULL;
  this->PromptTitle = NULL;
  this->Remember = 0;
}


//----------------------------------------------------------------------------
vtkPermissionPrompter::~vtkPermissionPrompter()
{
  if ( this->Username )
    {
    this->SetUsername ( NULL );
    }
  if ( this->Password )
    {
    this->SetPassword ( NULL );
    }
  if ( this->HostName)
    {
    this->SetHostName ( NULL );
    }
  if ( this->PromptMessage )
    {
    this->SetPromptMessage ( NULL );
    }
  if ( this->PromptTitle!= NULL )
    {
    this->SetPromptTitle ( NULL );
    }
}


//----------------------------------------------------------------------------
int vtkPermissionPrompter::Prompt( const char *message )
{
  // initialize with guest information
  this->Username = "";
  this->Password = "";
  this->HostName = "";
  return -1;
}


//----------------------------------------------------------------------------
void vtkPermissionPrompter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );

  os << indent << "Username: " << (this->Username ? this->Username : "(none)") << "\n";
  os << indent << "Password: " << (this->Password ? this->Password : "(none)") << "\n";
  os << indent << "HostName: " << (this->HostName ? this->HostName : "(none)") << "\n";
  os << indent << "PromptMessage: " << (this->PromptMessage ? this->PromptMessage : "(none)" ) << "\n";
  os << indent << "PromptTitle: " << (this->PromptTitle ? this->PromptTitle : "(none)" ) << "\n";
  os << indent << "Remember: " << this->Remember << "\n";
}
