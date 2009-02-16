#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkFetchMIParserCollection.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkFetchMIParserCollection );
vtkCxxRevisionMacro(vtkFetchMIParserCollection, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkFetchMIParserCollection::vtkFetchMIParserCollection ( ) {
}


//----------------------------------------------------------------------------
void vtkFetchMIParserCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
vtkFetchMIParserCollection::~vtkFetchMIParserCollection ( )
{
  this->RemoveAllItems();
}


//---------------------------------------------------------------------------
void vtkFetchMIParserCollection::AddParserByName ( vtkFetchMIParser *p, const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "AddParserByName got null name.");
    return;
    }

  p->SetName ( name );
  this->vtkCollection::AddItem (p);
}


//---------------------------------------------------------------------------
void vtkFetchMIParserCollection::DeleteParserByName ( const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "DeleteParserByName got null name.");
    return;
    }

  vtkFetchMIParser *p;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    p = vtkFetchMIParser::SafeDownCast (this->GetItemAsObject(i));
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
vtkFetchMIParser *vtkFetchMIParserCollection::FindParserByName (const char *name )
{
  if ( name == NULL )
    {
    vtkErrorMacro ( "FindParserByName got null name.");
    return NULL;
    }
  vtkFetchMIParser *p;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    p = vtkFetchMIParser::SafeDownCast (this->GetItemAsObject(i));
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
