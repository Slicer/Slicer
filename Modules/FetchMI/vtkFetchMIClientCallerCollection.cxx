#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkFetchMIClientCallerCollection.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkFetchMIClientCallerCollection );
vtkCxxRevisionMacro(vtkFetchMIClientCallerCollection, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkFetchMIClientCallerCollection::vtkFetchMIClientCallerCollection ( ) {
}


//----------------------------------------------------------------------------
void vtkFetchMIClientCallerCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
vtkFetchMIClientCallerCollection::~vtkFetchMIClientCallerCollection ( )
{
  this->RemoveAllItems();
}


//---------------------------------------------------------------------------
void vtkFetchMIClientCallerCollection::AddClientCallerByName ( vtkFetchMIClientCaller *p, const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "AddClientCallerByName got null name.");
    return;
    }

  p->SetName ( name );
  this->vtkCollection::AddItem (p);
}


//---------------------------------------------------------------------------
void vtkFetchMIClientCallerCollection::DeleteClientCallerByName ( const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "DeleteClientCallerByName got null name.");
    return;
    }

  vtkFetchMIClientCaller *p;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    p = vtkFetchMIClientCaller::SafeDownCast (this->GetItemAsObject(i));
    if ( p != NULL )
      {
      if ( !strcmp ( p->GetName(), name ) )
        {
        this->RemoveItem ( i );
        p->Delete();
        break;
        }
      }
    }
}


//---------------------------------------------------------------------------
vtkFetchMIClientCaller *vtkFetchMIClientCallerCollection::FindClientCallerByName (const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "FindClientCallerByName got null name.");
    return NULL;
    }
  vtkFetchMIClientCaller *p;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    p = vtkFetchMIClientCaller::SafeDownCast (this->GetItemAsObject(i));
    if ( p != NULL )
      {
      if ( !strcmp ( p->GetName(), name ) )
        {
        return ( p );
        }
      }
    }
  return ( NULL );
}
