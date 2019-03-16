// MRML includes
#include "vtkTagTableCollection.h"
#include "vtkTagTable.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkTagTableCollection );

//---------------------------------------------------------------------------
vtkTagTableCollection::vtkTagTableCollection ( )  = default;


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
  if ( name == nullptr )
    {
    vtkErrorMacro ( "vtkTagTableCollection::AddTableByName got nullptr name." );
    return;
    }
  t->SetName ( name );
  this->vtkCollection::AddItem (t);
}


//---------------------------------------------------------------------------
void vtkTagTableCollection::DeleteTableByName ( const char *name )
{

  if ( name == nullptr )
    {
    vtkErrorMacro ( "vtkTagTableCollection::DeleteTableByName got nullptr name." );
    return;
    }
  vtkTagTable *t;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    t = vtkTagTable::SafeDownCast (this->GetItemAsObject(i));
    if ( t != nullptr )
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
void vtkTagTableCollection::SetRestoreSelectionStateForAllTables ( int val )
{
  if ( val != 1 && val != 0 )
    {
    vtkWarningMacro ( "vtkTagTableColleciton::SetRestoreSelectionSTateForAllTables: got inappropriate value for state (not 1 or 0)." );
    return;
    }
  vtkTagTable *t;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    t = vtkTagTable::SafeDownCast (this->GetItemAsObject(i));
    if ( t != nullptr )
      {
      t->SetRestoreSelectionState ( val );
      }
    }
}

//---------------------------------------------------------------------------
vtkTagTable *vtkTagTableCollection::GetNextTable()
{
  return vtkTagTable::SafeDownCast(this->GetNextItemAsObject());
}

//---------------------------------------------------------------------------
vtkTagTable *vtkTagTableCollection::GetNextTable(vtkCollectionSimpleIterator &cookie)
{
  return vtkTagTable::SafeDownCast(this->GetNextItemAsObject(cookie));
}

//---------------------------------------------------------------------------
void vtkTagTableCollection::ClearAllTagTables ( )
{
  vtkTagTable *t;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    t = vtkTagTable::SafeDownCast (this->GetItemAsObject(i));
    if ( t != nullptr )
      {
      t->ClearTagTable();
      }
    }
}



//---------------------------------------------------------------------------
vtkTagTable *vtkTagTableCollection::FindTagTableByName (const char *name )
{

  if ( name == nullptr )
    {
    vtkErrorMacro ( "vtkTagTableCollection::FindTagTableByName got nullptr name." );
    return nullptr;
    }
  vtkTagTable *t;
  for ( int i=0; i < this->GetNumberOfItems(); i++ )
    {
    t = vtkTagTable::SafeDownCast (this->GetItemAsObject(i));
    if ( t != nullptr )
      {
      if ( !strcmp ( t->GetName(), name ) )
        {
        return ( t );
        }
      }
    }
  return ( nullptr );

}
