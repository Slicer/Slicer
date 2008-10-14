#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkTagTable.h"
#include "vtkIdTypeArray.h"
#include "vtkVariant.h"
#include "vtkStringArray.h"

#include <vtksys/SystemTools.hxx>
#include <iostream>
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkTagTable );
vtkCxxRevisionMacro(vtkTagTable, "$Revision: 1.9.12.1 $");


//----------------------------------------------------------------------------
vtkTagTable::vtkTagTable()
{
  this->Name = NULL;
}


//----------------------------------------------------------------------------
vtkTagTable::~vtkTagTable()
{
  if ( this->Name )
    {
    delete [] this->Name;
    }
  this->TagTable.clear();
  this->TagSelectionTable.clear();
}


//----------------------------------------------------------------------------
void vtkTagTable::PrintSelf(ostream& os, vtkIndent indent)
{
  std::map <std::string, std::string>::iterator iter;
  std::map <std::string, int>::iterator iter2;
  
  Superclass::PrintSelf ( os, indent );
  
  os << indent << "Name = " << this->GetName() << "\n";

  iter = this->TagTable.begin();

  for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      iter2 = this->TagSelectionTable.find( iter->first );
      if (iter2 != this->TagSelectionTable.end() )
        {
        os << indent << "TagTable:\n";    
        os << indent << "Attribute = " << iter->first << " Value = " << iter->second << " Selected = " << iter2->second << "\n";
        }
      else
        {
        os << indent << "TagTable:\n";    
        os << indent << "Attribute = " << iter->first << " Value = " << iter->second << " Selected = " << "??" << "\n";
        vtkErrorMacro ( "vtkTagTable::PrintSelf -- TagTable and TagSelectionTable are out of synch." );
        }
      }
}



//----------------------------------------------------------------------------
void vtkTagTable::AddOrUpdateTag ( const char *attribute, const char *value)
{
  //--- add a attribute value pair. If the attribute
  //--- already exists in the table, overwrite its value and parentID.
  if ( attribute != NULL )
    {
    if ( value == NULL )
      {
      value = "NULL";
      }

    //--- replace the tag if already present.
    //--- maintain its selected status.
    if ( this->UpdateTag ( attribute, value ) == 1 )
      {
      return;
      }
    else
      {
      //--- otherwise...
      //--- add tag in a new column and deselect it
      std::string att(attribute);
      std::string val(value );
      this->TagTable.insert (std::make_pair(att, val ));
      this->TagSelectionTable.insert ( std::make_pair(att, 0));
      }
    }
}

//----------------------------------------------------------------------------
void vtkTagTable::AddOrUpdateTag ( const char *attribute, const char *value, int selected)
{
  //--- add a attribute value pair. If the attribute
  //--- already exists in the table, overwrite its value and parentID.

  
  if ( attribute != NULL )
    {
    if ( value == NULL )
      {
      value = "NULL";
      }

    //--- replace the tag if already present.
    //--- maintain its selected status.
    if ( this->UpdateTag ( attribute, value, selected ) == 1 )
      {
      return;
      }
    else
      {
      //--- otherwise...
      //--- add tag in a new column and deselect it
      std::string att(attribute);
      std::string val(value );
      this->TagTable.insert (std::make_pair(att, val ));
      this->TagSelectionTable.insert ( std::make_pair(att, selected));
      }
    }
}




//----------------------------------------------------------------------------
void vtkTagTable::AddUniqueTag ( const char *attribute, const char *value )
{
  //--- add a attribute value pair.
  //--- update the tag if it's present in the table.
  if ( attribute != NULL )
    {
    if ( value == NULL )
      {
      value = "NULL";
      }

    std::string att (attribute);
    std::string val (value);
    std::map<std::string, std::string>::iterator iter;
    int unique=1;
    for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      if (iter->first == att && iter->second == val )
        {
        unique=0;
        break;
        }
      }

    if ( unique == 1 )
      {
      //--- otherwise...
      //--- add tag in a new column and deselect it
      std::string val(value );
      this->TagTable.insert (std::make_pair(att, val ));
      this->TagSelectionTable.insert ( std::make_pair(att, 0));      
      }
    }
}




//----------------------------------------------------------------------------
int vtkTagTable::UpdateTag ( const char *attribute, const char *value )
{

  vtksys_stl::string att = attribute;
  vtksys_stl::string val = value;  

  //--- try to do a case insensitive comparison.
  //--- this was Attribute, ATTRIBUTE, attribute (etc) will match.
  vtksys_stl::string lowAtt;
  vtksys_stl::string lowVal;
  vtksys_stl::string lowTest;
  lowAtt = vtksys::SystemTools::LowerCase (att);
  lowVal = vtksys::SystemTools::LowerCase(val);
  
    std::map<std::string, std::string>::iterator iter;
    for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      lowTest = vtksys::SystemTools::LowerCase(iter->first);
      if (lowTest == lowAtt )
        {
        iter->second = val;
        return 1;
        }
      }
    return 0;
}


//----------------------------------------------------------------------------
int vtkTagTable::UpdateTag ( const char *attribute, const char *value, int selected )
{

  vtksys_stl::string att = attribute;
  vtksys_stl::string val = value;  

  //--- try to do a case insensitive comparison.
  //--- this was Attribute, ATTRIBUTE, attribute (etc) will match.
  vtksys_stl::string lowAtt;
  vtksys_stl::string lowVal;
  vtksys_stl::string lowTest;
  lowAtt = vtksys::SystemTools::LowerCase (att);
  lowVal = vtksys::SystemTools::LowerCase(val);
  
    std::map<std::string, std::string>::iterator iter;
    for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      lowTest = vtksys::SystemTools::LowerCase(iter->first);
      if (lowTest == lowAtt )
        {
        iter->second = val;
        if ( selected )
          {
          this->SelectTag ( attribute );
          }
        else
          {
          this->DeselectTag ( attribute );
          }
        return 1;
        }
      }
    return 0;
}





//----------------------------------------------------------------------------
void vtkTagTable::SelectTag ( const char *attribute )
{
    std::string att (attribute);
     
    std::map<std::string, int>::iterator iter;
    int updated=0;
    for ( iter = this->TagSelectionTable.begin();
          iter != this->TagSelectionTable.end();
          iter++)
      {
      if (iter->first == att )
        {
        iter->second = 1;
        }
      }
}


//----------------------------------------------------------------------------
void vtkTagTable::DeselectTag ( const char *attribute)
{

    std::string att (attribute);
     
    std::map<std::string, int>::iterator iter;
    int updated=0;
    for ( iter = this->TagSelectionTable.begin();
          iter != this->TagSelectionTable.end();
          iter++)
      {
      if (iter->first == att )
        {
        iter->second = 0;
        }
      }
}



//----------------------------------------------------------------------------
int vtkTagTable::IsTagSelected ( const char *attribute )
{

    std::string att (attribute);
     
    std::map<std::string, int>::iterator iter;
    int updated=0;
    for ( iter = this->TagSelectionTable.begin();
          iter != this->TagSelectionTable.end();
          iter++)
      {
      if (iter->first == att )
        {
        if ( iter->second == 0 )
          {
          return 0;
          }
        else if ( iter->second ==1 )
          {
          return 1;
          }
        }
      }
    return -1;
}




//----------------------------------------------------------------------------
const char * vtkTagTable::GetTagAttribute ( int index)
{

    std::map<std::string, std::string>::iterator iter;
    int i=0;
    for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++, i++)
      {
      if (i==index )
        {
        return ( iter->first.c_str() );
        }
      }
    return ( NULL );
}





//----------------------------------------------------------------------------
 const char * vtkTagTable::GetTagValue ( int index)
{

    std::map<std::string, std::string>::iterator iter;
    int i=0;
    for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++, i++)
      {
      if (i==index )
        {
        return ( iter->second.c_str() );
        }
      }
    return ( NULL );
}


//----------------------------------------------------------------------------
const char * vtkTagTable::GetTagValue ( const char* attribute)
{
  std::string att(attribute);
  std::map<std::string, std::string>::iterator iter;
  int i=0;
  
  for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++, i++)
      {
      if (iter->first == att )
        {
        return ( iter->second.c_str() );
        }
      }
    return ( NULL );
}



//----------------------------------------------------------------------------
 int vtkTagTable::CheckTableForTag ( const char *attribute, const char *value)
{
  std::string att(attribute);
  std::string val (value);

  std::map<std::string, std::string>::iterator iter;
  
  for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      if (iter->first == att && iter->second == val )
        {
        return ( 1 );
        }
      }
    return ( 0 );
}




//----------------------------------------------------------------------------
int vtkTagTable::GetNumberOfTags ()
{
  return ( this->TagTable.size() );
}





//----------------------------------------------------------------------------
void vtkTagTable::ClearTagTable ( )
{
  this->TagTable.clear();
  this->TagSelectionTable.clear();
}





//----------------------------------------------------------------------------
void vtkTagTable::DeleteTag ( const char *attribute  )
{

  std::string att(attribute);
  
  int deltag=0;
  this->TagTable.erase( att );
  this->TagSelectionTable.erase (att );
}



