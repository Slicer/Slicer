
#include "vtkObjectFactory.h"
#include "vtkSlicerModuleNavigator.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerModuleNavigator );
vtkCxxRevisionMacro ( vtkSlicerModuleNavigator, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerModuleNavigator::vtkSlicerModuleNavigator ( )
{
  this->InitializeLists ( );
}



//---------------------------------------------------------------------------
vtkSlicerModuleNavigator::~vtkSlicerModuleNavigator ( )
{
  this->DeleteLists ( );
}



//---------------------------------------------------------------------------
void vtkSlicerModuleNavigator::InitializeLists ( )
{
  this->ModuleNavigationList = NULL;
  this->ModuleHistoryList = NULL;
  
  this->LastModule = NULL;
  this->CurrentModule = NULL;
  this->NextModule = NULL;
  this->PreviousModule = NULL;
  this->HomeModule = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerModuleNavigator::DeleteLists ( )
{
  struct  ModuleNameEntry *m = this->ModuleNavigationList;
  struct  ModuleNameEntry *mnext;
  
  while ( m != NULL )
    {
    mnext= m->Next;
    delete m;
    m=mnext;
    }

  m = this->ModuleHistoryList;
  while ( m != NULL )
    {
    mnext = m->Next;
    delete m;
    m=mnext;
    }

  this->InitializeLists ( );
}



//---------------------------------------------------------------------------
void vtkSlicerModuleNavigator::AddModuleNameToHistoryList ( const char *name )
{
  if ( name != NULL )
    {
    // if HistoryList is emtpy, then add first entry
    if ( this->ModuleHistoryList == NULL )
      {
      struct ModuleNameEntry *m = new ModuleNameEntry ( name );
      m->Prev = NULL;
      m->Next = NULL;
      this->ModuleHistoryList = m;
      }
    else
      {
      // search the list to see if name already exists...
      int name_exists = 0;
      struct ModuleNameEntry *mp;
      mp = this->ModuleHistoryList;
      while ( mp != NULL )
        {
        if ( strcmp ( mp->ModuleName, name ) == 0 )
          {
          name_exists = 1;
          break;
          }
        mp = mp->Next;
        }

      // if the name doesn't exist yet, find the end of the list...
      if ( name_exists==0 )
        {
        mp = this->ModuleHistoryList;
        while ( mp->Next != NULL )
          {
          mp = mp->Next;
          }
        // add to the end of the list.
        struct ModuleNameEntry *m = new ModuleNameEntry ( name );
        mp->Next = m;
        }
      
      }
    } 
}



//---------------------------------------------------------------------------
void vtkSlicerModuleNavigator::AddModuleNameToNavigationList ( const char *name )
{

  if ( name != NULL )
    {
    // if NavList is empty, then add first module
    if ( this->ModuleNavigationList == NULL )
      {
      struct ModuleNameEntry *nm = new ModuleNameEntry ( name );
      nm->Prev = NULL;
      nm->Next = NULL;
      this->ModuleNavigationList = nm;
      // mark the end of the list
      this->LastModule = this->ModuleNavigationList;
      // and update the current module.
      this->CurrentModule = this->ModuleNavigationList;
      }

    else
      {

      // If we re-load the same module consecutively, don't add
      // it again to the module history or navigation lists.
      if ( strcmp (name, this->CurrentModule->ModuleName ) != 0)
        {
        // if we're at the end of the list, add to the Nav list.
        if ( this->CurrentModule->Next == NULL )
          {
          struct ModuleNameEntry *nm = new ModuleNameEntry ( name );
          this->CurrentModule->Next = nm;
          nm->Prev = this->CurrentModule;
          nm->Next = NULL;
          this->LastModule = nm;
          // and update the current module.
          this->CurrentModule = nm;
          }

        else
          {
          // if we're in the middle of the list, remove all
          // module entries past our current module...
          struct ModuleNameEntry *m = this->LastModule;
          struct ModuleNameEntry *mprev;
          while ( m != this->CurrentModule )
            {
            mprev = m->Prev;
            delete m;
            m = mprev;
            }
          // ...And now add the new module name.
          struct ModuleNameEntry *nm = new ModuleNameEntry ( name );
          this->CurrentModule->Next = nm;
          nm->Prev = this->CurrentModule;
          nm->Next = NULL;
          this->LastModule = nm;
          // and update the current module.
          this->CurrentModule = nm;
          }
        }
      }

    }
}


//---------------------------------------------------------------------------
char * vtkSlicerModuleNavigator::NavigateBack ( )
{

  if ( this->CurrentModule != NULL )
    {
    if ( this->CurrentModule->Prev != NULL )
      {
      this->CurrentModule = this->CurrentModule->Prev;
      return ( this->CurrentModule->ModuleName );
      }
    else
      {
      return ( this->CurrentModule->ModuleName );
//      return ( NULL );
      }
    }
  else
    {
    return ( NULL );
    }
}




//---------------------------------------------------------------------------
char * vtkSlicerModuleNavigator::NavigateForward ( )
{

  if ( this->CurrentModule != NULL )
    {
    if ( this->CurrentModule->Next != NULL )
      {
      this->CurrentModule = this->CurrentModule->Next;
      return ( this->CurrentModule->ModuleName );
      }
    else
      {
      return ( this->CurrentModule->ModuleName );
//      return ( NULL );
      }
    }
  else
    {
    return ( NULL );
    }
}





//---------------------------------------------------------------------------
ModuleNameEntry* vtkSlicerModuleNavigator::GetModuleNavigationList ( )
{
  return ( this->ModuleNavigationList );
}



//---------------------------------------------------------------------------
ModuleNameEntry* vtkSlicerModuleNavigator::GetModuleHistoryList ( )
{
  return ( this->ModuleHistoryList );
}



//---------------------------------------------------------------------------
char* vtkSlicerModuleNavigator::GetFirstModuleName ( )
{

  if ( this->ModuleNavigationList != NULL )
    {
    return ( this->ModuleNavigationList->ModuleName );
    }
  else
    {
    return ( NULL );
    }  
}


//---------------------------------------------------------------------------
char* vtkSlicerModuleNavigator::GetNextModuleName ( )
{
  if ( this->CurrentModule != NULL )
    {
    if ( this->CurrentModule->Next != NULL )
      {
      return ( this->CurrentModule->Next->ModuleName );
      }
    else
      {
      return ( NULL );
      }
    }
  else
    {
    return ( NULL );
    }
}


//---------------------------------------------------------------------------
char* vtkSlicerModuleNavigator::GetPreviousModuleName ( )
{
  if ( this->CurrentModule != NULL )
    {
    if ( this->CurrentModule->Prev != NULL )
      {
      return ( this->CurrentModule->Prev->ModuleName );
      }
    else
      {
      return ( NULL );
      }
    }
  else
    {
    return ( NULL );
    }
}


//---------------------------------------------------------------------------
char* vtkSlicerModuleNavigator::GetLastModuleName ( )
{
  if ( this->LastModule != NULL )
    {
    return ( this->LastModule->ModuleName );
    }
  else
    {
    return ( NULL );
    }
}



//---------------------------------------------------------------------------
char* vtkSlicerModuleNavigator::GetCurrentModuleName ( )
{
  if ( this->CurrentModule != NULL )
    {
    return ( this->CurrentModule->ModuleName );
    }
  else
    {
    return ( NULL );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerModuleNavigator::ShowHistory ( )
{
}



//---------------------------------------------------------------------------
void vtkSlicerModuleNavigator::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->Superclass::PrintSelf ( os, indent );
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerModuleNavigator: " << this->GetClassName ( ) << "\n";

    struct ModuleNameEntry *m = this->ModuleNavigationList ;
    while (m != NULL )
      {
      os << indent << "ModuleNavigationList entry: " << m->ModuleName << "\n";
      m = m->Next;
      }
    m = this->ModuleHistoryList;
    while (m != NULL )
      {
        os << indent << "ModuleHistoryList entry: " << m->ModuleName << "\n";
        m = m->Next;
      }
}
