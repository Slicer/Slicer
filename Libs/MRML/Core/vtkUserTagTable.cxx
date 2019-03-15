
#include <iostream>
#include <sstream>
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkUserTagTable.h"
#include "vtkIdTypeArray.h"
#include "vtkVariant.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro ( vtkUserTagTable );

//----------------------------------------------------------------------------
vtkUserTagTable::vtkUserTagTable()
{
  this->TagTable = vtkTable::New();
  //--- Tag = keyword + value.
  //--- table looks like:
  //---
  //    kw1   kw2   kw3 ...  kwN
  //    val1   val2   val3 ...  valN
  //---
  //--- where kw is the column name, and
  //--- val is the 0th value in the column.
}


//----------------------------------------------------------------------------
vtkUserTagTable::~vtkUserTagTable()
{
  if ( this->TagTable )
    {
    int numcols = this->TagTable->GetNumberOfColumns();
    for ( int i=numcols-1; i >= 0; i-- )
      {
      this->TagTable->RemoveColumn ( i );
      }

    this->TagTable->Delete();
    this->TagTable = nullptr;
    }
}


//----------------------------------------------------------------------------
void vtkUserTagTable::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}



//----------------------------------------------------------------------------
int vtkUserTagTable::AddKeywordValuePair ( const char *keyword, const char *value)
{

  if ( this->TagTable && keyword != nullptr )
    {
    if ( value == nullptr )
      {
      value = "NULL";
      }

    if ( this->CheckTableForKeyword ( keyword ) == 1 )
      {
      vtkErrorMacro ( "Already a tag specified for keyword " << keyword << ". Not adding the value " << value << "to the user tags." );
      return ( 0 );
      }
    //--- add a new column whose name is
    //--- the new keyword, and whose value
    //--- is given by 'value'.
    vtkStringArray *a = vtkStringArray::New();
    a->SetName (keyword );
    a->SetNumberOfValues ( 1 );
    a->SetValue ( 0, value );
    this->TagTable->AddColumn ( a );
    a->Delete();
    }
  return (1 );
}


//----------------------------------------------------------------------------
int vtkUserTagTable::AddKeywordValuePair ( const char *keyword, void *value)
{

  if ( this->TagTable && keyword != nullptr )
    {
      std::stringstream ss;
    if ( value != nullptr )
      {
      ss << value;
      }
    else
      {
      ss << "NULL";
      }

    if ( this->CheckTableForKeyword ( keyword ) == 1 )
      {
      vtkErrorMacro ( "Already a tag specified for keyword " << keyword << ". Not adding the value " << value << "to the user tags." );
      return ( 0 );
      }
    //--- add a new column whose name is
    //--- the new keyword, and whose value
    //--- is given by 'value'.
    vtkStringArray *a = vtkStringArray::New();
    a->SetName (keyword );
    a->SetNumberOfValues ( 1 );
    a->SetValue ( 0, ss.str().c_str() );
    this->TagTable->AddColumn ( a );
    a->Delete();
    }
  return (1 );
}




//----------------------------------------------------------------------------
const char * vtkUserTagTable::GetUserTagKeyword ( int index)
{
  if ( this->TagTable )
    {
    int numcols = this->TagTable->GetNumberOfColumns();

    if ( index < numcols )
      {
      //--- return the keyword in this keyword-value column
      return ( this->TagTable->GetColumnName( index ) );
      }
    }
  return ( nullptr );
}


//----------------------------------------------------------------------------
int vtkUserTagTable::GetKeywordColumn ( const char* keyword )
{
  if ( this->TagTable )
    {
    int numcols = this->TagTable->GetNumberOfColumns();
    for ( int i=0; i < numcols; i++ )
      {
      if ( !strcmp ( this->TagTable->GetColumnName( i ), keyword ))
        {
        return (i);
        }
      }
    }
    return ( -1 );
}




//----------------------------------------------------------------------------
 const char * vtkUserTagTable::GetUserTagValue ( int index)
{
  char * returnString = nullptr;
  const char *val;
  vtkVariant d;
  std::string s;

  if ( this->TagTable )
    {
    int numcols = this->TagTable->GetNumberOfColumns();
    if ( index < numcols )
      {
      //--- get the value in this keyword-value column.
      d = this->TagTable->GetValue ( 0, index );
      s = d.ToString();
      size_t n = strlen(s.c_str()) + 1;
      val = s.c_str();
      char *cptr = new char[n];
      returnString = cptr;
      do { *cptr++ = *val++; } while ( --n );
      }
    }
  return (returnString);
}


//----------------------------------------------------------------------------
const char * vtkUserTagTable::GetUserTagValue ( const char* keyword)
{
  char * returnString = nullptr;
  const char *val;
  vtkVariant d;
  std::string ss;

  if ( this->TagTable )
    {
    int numcols = this->TagTable->GetNumberOfColumns();
    for ( int i=0; i < numcols; i++ )
      {
      //--- get each column in table and check to see if
      //--- its name matches the keyword.
      vtkStringArray *s =  vtkStringArray::SafeDownCast ( this->TagTable->GetColumn(i) );
      if ( !strcmp (keyword,  s->GetName() ) )
        {
        //--- if we have a match, return the keyword's value
        d = this->TagTable->GetValue ( 0, i);
        ss = d.ToString();
        val = ss.c_str();
        size_t n = strlen(ss.c_str()) + 1;
        val = ss.c_str();
        char *cptr = new char[n];
        returnString = cptr;
        do { *cptr++ = *val++; } while ( --n );
        }
      }
    }
  return  (returnString );
}


//----------------------------------------------------------------------------
 int vtkUserTagTable::CheckTableForKeyword ( const char *keyword)
{
  if ( this->TagTable )
    {
    std::stringstream ss;
    int numcols = this->TagTable->GetNumberOfColumns();
    for ( int i=0; i < numcols; i++ )
      {
      //--- get each column in table and check to see if
      //--- its name matches the keyword.
      vtkStringArray *s =  vtkStringArray::SafeDownCast ( this->TagTable->GetColumn(i) );
      ss << s->GetName ( );
      if ( !strcmp (keyword,  ss.str().c_str() ) )
        {
        //--- if the keyword is here, return 1,
        //--- otherwise return 0
        return ( 1 );
        }
      }
    }
  return ( 0 );
}


//----------------------------------------------------------------------------
 int vtkUserTagTable::DeleteKeywordValuePair ( int index )
{
  if ( this->TagTable )
    {
    int numcols = this->TagTable->GetNumberOfColumns();
    if ( index < numcols )
      {
      this->TagTable->RemoveColumn ( index );
      return ( 1 );
      }
    }
  return ( 0 );
}


//----------------------------------------------------------------------------
int vtkUserTagTable::GetNumberOfColumns ()
{
  if ( this->TagTable )
    {
    return (this->TagTable->GetNumberOfColumns() );
    }
  else
    {
    return 0;
    }
}



//----------------------------------------------------------------------------
int vtkUserTagTable::DeleteKeywordValuePair ( const char *keyword  )
{
  if ( this->TagTable )
    {
    std::stringstream ss;
    int numcols = this->TagTable->GetNumberOfColumns();
    for ( int i=0; i < numcols; i++ )
      {
      //--- find what column keyword is in
      int col = this->GetKeywordColumn ( keyword );
      if ( col >= 0 && col < numcols )
        {
        //--- try to delete it and return 1 if successful.
        if ( this->DeleteKeywordValuePair ( col ) )
          {
          return (1 );
          }
        }
      }
    }
  return ( 0 );
}


