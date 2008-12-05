
#include "vtkObjectFactory.h"
#include "vtkSlicerFont.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFont );
vtkCxxRevisionMacro ( vtkSlicerFont, "$Revision: 1.1 $" );

//---------------------------------------------------------------------------
vtkSlicerFont::vtkSlicerFont ( ) {

    this->FontFamilies.push_back ( "Arial" );
    this->FontFamilies.push_back ( "Helvetica" );
    this->FontFamilies.push_back ( "Verdana" );
    this->NumberOfFontFamilies = this->FontFamilies.size();;
    
    this->FontSizes.push_back ("small");
    this->FontSizes.push_back ("medium");
    this->FontSizes.push_back ("large");
    this->FontSizes.push_back("largest");
    this->NumberOfFontSizes = this->FontSizes.size();

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

  // legacy...
    this->AdobeHelvetica12 = "-Adobe-Helvetica-Bold-R-Normal-*-12-*-*-*-*-*-*-*";
    this->AdobeHelvetica10 = "-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*";
    this->AdobeHelvetica8 = "-Adobe-Helvetica-Bold-R-Normal-*-8-*-*-*-*-*-*-*";
    this->JustifyLeft = "left";
    this->JustifyRight = "right";
    this->JustifyCenter = "center";
    this->JustifyFull = "full";
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
}


 
//---------------------------------------------------------------------------
vtkSlicerFont::~vtkSlicerFont ( ) {
    this->NumberOfFontFamilies = 0;
    this->NumberOfFontSizes = 0;
}


//---------------------------------------------------------------------------
int vtkSlicerFont::IsValidFontFamily( const char *str )
{
  
  for ( int i = 0; i < this->NumberOfFontFamilies; i++ )
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
  for ( int i = 0; i < this->NumberOfFontSizes; i++ )
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


