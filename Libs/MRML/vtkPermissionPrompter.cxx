
#include "vtkPermissionPrompter.h"
#include "vtkCallbackCommand.h"


vtkStandardNewMacro ( vtkPermissionPrompter );
vtkCxxRevisionMacro ( vtkPermissionPrompter, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkPermissionPrompter::vtkPermissionPrompter()
{
  this->UserName = NULL;
  this->Password = NULL;
  this->Remember = 1;
}


//----------------------------------------------------------------------------
vtkPermissionPrompter::~vtkPermissionPrompter()
{
  if ( this->UserName )
    {
    delete [] this->UserName;
    }
  if ( this->Password )
    {
    delete [] this->Password;
    }
}


//----------------------------------------------------------------------------
void vtkPermissionPrompter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );

  os << indent << "UserName: " << (this->UserName ? this->UserName : "(none)") << "\n";
  os << indent << "Password: " << (this->Password ? this->Password : "(none)") << "\n";
  os << indent << "Remember: " << this->Remember << "\n";
}
