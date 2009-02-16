#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkFetchMIWriterCollection.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkFetchMIWriterCollection );
vtkCxxRevisionMacro(vtkFetchMIWriterCollection, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkFetchMIWriterCollection::vtkFetchMIWriterCollection ( ) {
}


//----------------------------------------------------------------------------
void vtkFetchMIWriterCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
vtkFetchMIWriterCollection::~vtkFetchMIWriterCollection ( )
{
  this->RemoveAllItems();
}


//---------------------------------------------------------------------------
void vtkFetchMIWriterCollection::AddWriterByName ( vtkFetchMIWriter *w, const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "AddWriterByName got null name.");
    return;
    }
  w->SetName ( name );
  this->vtkCollection::AddItem (w);
}


//---------------------------------------------------------------------------
void vtkFetchMIWriterCollection::DeleteWriterByName ( const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "DeleteWriterByName got null name.");
    return;
    }

  vtkFetchMIWriter *w;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    w = vtkFetchMIWriter::SafeDownCast (this->GetItemAsObject(i));
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
vtkFetchMIWriter *vtkFetchMIWriterCollection::FindWriterByName (const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "FindWriterByName got null name.");
    return NULL;
    }

  vtkFetchMIWriter *w;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    w = vtkFetchMIWriter::SafeDownCast (this->GetItemAsObject(i));
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
