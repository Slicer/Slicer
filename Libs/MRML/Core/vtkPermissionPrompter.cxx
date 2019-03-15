// MRML includes
#include "vtkPermissionPrompter.h"

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro ( vtkPermissionPrompter );

//----------------------------------------------------------------------------
vtkPermissionPrompter::vtkPermissionPrompter()
{
  this->Username = nullptr;
  this->Password = nullptr;
  this->HostName = nullptr;
  this->PromptMessage = nullptr;
  this->PromptTitle = nullptr;
  this->Remember = 0;
}

//----------------------------------------------------------------------------
vtkPermissionPrompter::~vtkPermissionPrompter()
{
  if ( this->Username )
    {
    this->SetUsername ( nullptr );
    }
  if ( this->Password )
    {
    this->SetPassword ( nullptr );
    }
  if ( this->HostName)
    {
    this->SetHostName ( nullptr );
    }
  if ( this->PromptMessage )
    {
    this->SetPromptMessage ( nullptr );
    }
  if ( this->PromptTitle!= nullptr )
    {
    this->SetPromptTitle ( nullptr );
    }
}


//----------------------------------------------------------------------------
int vtkPermissionPrompter::Prompt( const char * vtkNotUsed(message) )
{
  // initialize with guest information
  this->Username = const_cast<char *>("");
  this->Password = const_cast<char *>("");
  this->HostName = const_cast<char *>("");
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
