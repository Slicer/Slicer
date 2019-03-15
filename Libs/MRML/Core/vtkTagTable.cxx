#include "vtkObjectFactory.h"
#include "vtkTagTable.h"

#include <vtksys/SystemTools.hxx>


//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkTagTable );


//----------------------------------------------------------------------------
vtkTagTable::vtkTagTable()
{
  this->Name = nullptr;
  this->RestoreSelectionState = 0;
}


//----------------------------------------------------------------------------
vtkTagTable::~vtkTagTable()
{
  this->RestoreSelectionState = 0;
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

  os << indent << "Name = " << (this->GetName() == nullptr ? "(none)" : this->GetName()) << "\n";

  os << indent << "RestoreSelectionState = " << this->GetRestoreSelectionState() << "\n";

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
  if ( attribute != nullptr )
    {
    if ( value == nullptr )
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


  if ( attribute != nullptr )
    {
    if ( value == nullptr )
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
void vtkTagTable::Copy( vtkTagTable *t)
{
  std::string att;
  std::string val;
  int selected;
  if ( t != nullptr )
    {
    this->ClearTagTable();
    this->SetName ( t->GetName() );
    for ( int i=0; i < t->GetNumberOfTags(); i++ )
      {
      att.clear();
      val.clear();
      att = t->GetTagAttribute(i);
      val = t->GetTagValue(i);
      selected = t->IsTagSelected ( att.c_str() );
      this->AddUniqueTag ( att.c_str(), val.c_str() );
      if ( selected )
        {
        this->SelectTag ( att.c_str() );
        }
      else
        {
        this->DeselectTag ( att.c_str() );
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkTagTable::AddUniqueTag ( const char *attribute, const char *value )
{
  //--- add a attribute value pair.
  //--- update the tag if it's present in the table.
  if ( attribute != nullptr )
    {
    if ( value == nullptr )
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
      std::string val2(value );
      this->TagTable.insert (std::make_pair(att, val2 ));
      this->TagSelectionTable.insert ( std::make_pair(att, 0));
      }
    }
}




//----------------------------------------------------------------------------
int vtkTagTable::UpdateTag ( const char *attribute, const char *value )
{

  std::string att = attribute;
  std::string val = value;

  //--- try to do a case insensitive comparison.
  //--- this was Attribute, ATTRIBUTE, attribute (etc) will match.
  std::string lowAtt;
  std::string lowVal;
  std::string lowTest;
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

  std::string att = attribute;
  std::string val = value;

  //--- try to do a case insensitive comparison.
  //--- this was Attribute, ATTRIBUTE, attribute (etc) will match.
  std::string lowAtt;
  std::string lowVal;
  std::string lowTest;
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
    return ( nullptr );
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
    return ( nullptr );
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
    return ( nullptr );
}


//----------------------------------------------------------------------------
int vtkTagTable::CheckTableForTag ( const char *attribute )
{
  std::string att;
  std::string lowatt;
  std::map<std::string, std::string>::iterator iter;

  att = vtksys::SystemTools::LowerCase(attribute);
  int index = 0;
  for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      lowatt = vtksys::SystemTools::LowerCase(iter->first);
      if ( lowatt == att )
        {
        return ( index );
        }
      index ++;
      }
    return ( -1 );
}



//----------------------------------------------------------------------------
 int vtkTagTable::CheckTableForTag ( const char *attribute, const char *value)
{
  std::string att;
  std::string val;
  std::map<std::string, std::string>::iterator iter;

  att = vtksys::SystemTools::LowerCase(attribute);
  val = vtksys::SystemTools::LowerCase(value);
  std::string lowatt;
  std::string lowval;

  int index = 0;
  for ( iter = this->TagTable.begin();
          iter != this->TagTable.end();
          iter++)
      {
      lowatt = vtksys::SystemTools::LowerCase(iter->first);
      lowval = vtksys::SystemTools::LowerCase(iter->second);
      if ( (lowatt == att ) && (lowval == val) )
        {
        return ( index );
        }
      index ++;
      }
    return ( -1 );
}




//----------------------------------------------------------------------------
int vtkTagTable::GetNumberOfTags ()
{
  return static_cast<int>( this->TagTable.size() );
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

  this->TagTable.erase( att );
  this->TagSelectionTable.erase (att );
}



