#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkFetchMIServerCollection.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkFetchMIServerCollection );
vtkCxxRevisionMacro(vtkFetchMIServerCollection, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkFetchMIServerCollection::vtkFetchMIServerCollection ( ) {
}


//----------------------------------------------------------------------------
void vtkFetchMIServerCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
vtkFetchMIServerCollection::~vtkFetchMIServerCollection ( )
{
  this->RemoveAllItems();
}



//---------------------------------------------------------------------------
int vtkFetchMIServerCollection::IsKnownServiceType ( const char *svctype )
{
  // NOTE TO DEVELOPERS: build this out as new web service types are added.
  // add your new service types here.
  int retval = 0;
  if ( !(strcmp (svctype, "XND" ) ) )
    {
    retval = 1;
    }
  if ( !(strcmp (svctype, "HID" )))
    {
    retval = 1;
    }
  return (retval);
}


//---------------------------------------------------------------------------
void vtkFetchMIServerCollection::AddServerByName ( vtkFetchMIServer *w, const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "AddServerByName got null name.");
    return;
    }
  w->SetName ( name );
  this->vtkCollection::AddItem (w);
}


//---------------------------------------------------------------------------
void vtkFetchMIServerCollection::DeleteServerByName ( const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "DeleteServerByName got null name.");
    return;
    }

  vtkFetchMIServer *w;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    w = vtkFetchMIServer::SafeDownCast (this->GetItemAsObject(i));
    if ( w != NULL )
      {
      if ( !strcmp ( w->GetName(), name ) )
        {
        this->RemoveItem ( i );
        w->Delete();
        break;
        }
      }
    }
}


//---------------------------------------------------------------------------
vtkFetchMIServer *vtkFetchMIServerCollection::FindServerByName (const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "FindServerByName got null name.");
    return NULL;
    }

  vtkFetchMIServer *w;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    w = vtkFetchMIServer::SafeDownCast (this->GetItemAsObject(i));
    if ( w != NULL )
      {
      if ( !strcmp ( w->GetName(), name ) )
        {
        return ( w );
        }
      }
    }
  return ( NULL );
}
