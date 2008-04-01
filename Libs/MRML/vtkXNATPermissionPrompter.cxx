
#include "vtkXNATPermissionPrompter.h"
#include "vtkCallbackCommand.h"


vtkStandardNewMacro ( vtkXNATPermissionPrompter );
vtkCxxRevisionMacro ( vtkXNATPermissionPrompter, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkXNATPermissionPrompter::vtkXNATPermissionPrompter()
{
  this->Host = NULL;
}


//----------------------------------------------------------------------------
vtkXNATPermissionPrompter::~vtkXNATPermissionPrompter()
{
  if ( this->Host )
    {
    delete [] this->Host;
    }
}


//----------------------------------------------------------------------------
void vtkXNATPermissionPrompter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "Host: " << (this->Host ? this->Host : "(none)") << "\n";
}
