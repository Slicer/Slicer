
#include "vtkObjectFactory.h"
#include "vtkSlicerFont.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFont );
vtkCxxRevisionMacro ( vtkSlicerFont, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerFont::vtkSlicerFont ( ) {

  this->FontFamilies.push_back ( "Arial" );
  this->FontFamilies.push_back ( "Helvetica" );
  this->FontFamilies.push_back ( "Verdana" );
    
  this->FontSizes.push_back ("small");
  this->FontSizes.push_back ("medium");
  this->FontSizes.push_back ("large");
  this->FontSizes.push_back("largest");

  //--- GUI uses three font sizes per category above.
  //--- TODO: make this nicer 
  this->FontSizeSmall0 = 5;
  this->FontSizeSmall1 = 6;
  this->FontSizeSmall2 = 8;
  this->FontSizeMedium0 = 7;
  this->FontSizeMedium1 = 8;
  this->FontSizeMedium2 = 10;
  this->FontSizeLarge0 = 9;
  this->FontSizeLarge1 = 10;
  this->FontSizeLarge2 = 12;
  this->FontSizeLargest0 = 11;
  this->FontSizeLargest1 = 12;
  this->FontSizeLargest2 = 14;
}

//---------------------------------------------------------------------------
int vtkSlicerFont::GetFontSize2 ( const char *size )
{
  if ( !( strcmp (size, "small" ) ))
    {
    return ( this->FontSizeSmall2 );
    }
  else if (!( strcmp (size, "medium" ) ))
    {
    return ( this->FontSizeMedium2 );
    }
  else if (!( strcmp (size, "large" ) ))
    {
    return ( this->FontSizeLarge2 );
    }
  else if (!( strcmp (size, "largest" ) ))
    {
    return ( this->FontSizeLargest2 );
    }
  else
    {
    vtkErrorMacro ("invalid font size" << size);
    return ( this->FontSizeMedium2 );
    }
}

//---------------------------------------------------------------------------
int vtkSlicerFont::GetFontSize1 ( const char *size )
{

  if ( !( strcmp (size, "small" ) ))
    {
    return ( this->FontSizeSmall1 );
    }
  else if (!( strcmp (size, "medium" ) ))
    {
    return ( this->FontSizeMedium1 );
    }
  else if (!( strcmp (size, "large" ) ))
    {
    return ( this->FontSizeLarge1 );
    }
  else if (!( strcmp (size, "largest" ) ))
    {
    return ( this->FontSizeLargest1 );
    }
  else
    {
    vtkErrorMacro ("invalid font size" << size);
    return ( this->FontSizeMedium1 );
    }
}

//---------------------------------------------------------------------------
int vtkSlicerFont::GetFontSize0 ( const char *size )
{

  if ( !( strcmp (size, "small" ) ))
    {
    return ( this->FontSizeSmall0 );
    }
  else if (!( strcmp (size, "medium" ) ))
    {
    return ( this->FontSizeMedium0 );
    }
  else if (!( strcmp (size, "large" ) ))
    {
    return ( this->FontSizeLarge0 );
    }
  else if (!( strcmp (size, "largest" ) ))
    {
    return ( this->FontSizeLargest0 );
    }
  else
    {
    vtkErrorMacro ("invalid font size" << size);
    return  ( this->FontSizeMedium0 );
    }
}

//---------------------------------------------------------------------------
vtkSlicerFont::~vtkSlicerFont ( ) {
}

//---------------------------------------------------------------------------
int vtkSlicerFont::IsValidFontFamily( const char *str )
{
  
  for ( int i = 0; i < this->GetNumberOfFontFamilies(); i++ )
    {
    if ( !( strcmp (str, this->FontFamilies[i].c_str() ) ) )
      {
      return 1;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
int vtkSlicerFont::IsValidFontSize( const char *str )
{
  for ( int i = 0; i < this->GetNumberOfFontSizes(); i++ )
    {
    if ( !( strcmp (str, this->FontSizes[i].c_str() ) ) )
      {
      return 1;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
char *vtkSlicerFont::GetFontAsFamily(char *fontstring)
{
  // parses out the font string and returns the css version of the family
  return "Adobe,Helvetica,sans-serif";
}

//---------------------------------------------------------------------------
int vtkSlicerFont::GetNumberOfFontSizes()
{
  return this->FontSizes.size();
}

//---------------------------------------------------------------------------
int vtkSlicerFont::GetNumberOfFontFamilies()
{
  return this->FontFamilies.size();
}


