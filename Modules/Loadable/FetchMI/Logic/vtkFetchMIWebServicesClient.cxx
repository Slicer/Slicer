
#include "vtkObjectFactory.h"
#include "vtkFetchMIWebServicesClient.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIWebServicesClient );
vtkCxxRevisionMacro ( vtkFetchMIWebServicesClient, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIWebServicesClient::vtkFetchMIWebServicesClient ( )
{
  this->Name = NULL;
  this->URIHandler = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIWebServicesClient::~vtkFetchMIWebServicesClient ( )
{
  if ( this->Name )
    {
    this->SetName ( NULL);
    }
  if ( this->URIHandler )
    {
    this->SetURIHandler ( NULL );
    }
}


//---------------------------------------------------------------------------
void vtkFetchMIWebServicesClient::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
}








