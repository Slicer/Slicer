
#include "vtkObjectFactory.h"
#include "vtkFetchMIClientCaller.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIClientCaller );
vtkCxxRevisionMacro ( vtkFetchMIClientCaller, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIClientCaller::vtkFetchMIClientCaller ( )
{
  this->Name = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIClientCaller::~vtkFetchMIClientCaller ( )
{
  if ( this->Name )
    {
    this->SetName ( NULL);
    }
}

//---------------------------------------------------------------------------
void vtkFetchMIClientCaller::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
}








