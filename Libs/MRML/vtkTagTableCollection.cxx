#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkTagTableCollection.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkTagTableCollection );
vtkCxxRevisionMacro(vtkTagTableCollection, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkTagTableCollection::vtkTagTableCollection ( ) {
}


//----------------------------------------------------------------------------
void vtkTagTableCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}

//---------------------------------------------------------------------------
vtkTagTableCollection::~vtkTagTableCollection ( )
{
  this->RemoveAllItems();
}


//---------------------------------------------------------------------------
void vtkTagTableCollection::AddTableByName ( vtkTagTable *t, const char *name )
{
  t->SetName ( name );
  this->vtkCollection::AddItem (t);
}


//---------------------------------------------------------------------------
void vtkTagTableCollection::DeleteTableByName ( const char *name )
{
  vtkTagTable *t;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    t = vtkTagTable::SafeDownCast (this->GetItemAsObject(i));
    if ( t != NULL )
      {
      if ( !strcmp ( t->GetName(), name ) )
        {
        this->RemoveItem ( i );
        t->Delete();
        break;
        }
      }
    }
}


//---------------------------------------------------------------------------
vtkTagTable *vtkTagTableCollection::FindTagTableByName (const char *name ) {

  vtkTagTable *t;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    t = vtkTagTable::SafeDownCast (this->GetItemAsObject(i));
    if ( t != NULL )
      {
      if ( !strcmp ( t->GetName(), name ) )
        {
        return ( t );
        }
      }
    }
  return ( NULL );

}
