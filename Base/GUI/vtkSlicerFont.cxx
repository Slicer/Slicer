
#include "vtkObjectFactory.h"
#include "vtkSlicerFont.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFont );
vtkCxxRevisionMacro ( vtkSlicerFont, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerFont::vtkSlicerFont ( ) {
    this->AdobeHelvetica12 = "-Adobe-Helvetica-Bold-R-Normal-*-12-*-*-*-*-*-*-*";
    this->AdobeHelvetica10 = "-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*";
    this->AdobeHelvetica8 = "-Adobe-Helvetica-Bold-R-Normal-*-8-*-*-*-*-*-*-*";

    this->JustifyLeft = "left";
    this->JustifyRight = "right";
    this->JustifyCenter = "center";
    this->JustifyFull = "full";
        
}


//---------------------------------------------------------------------------
vtkSlicerFont::~vtkSlicerFont ( ) {
}

//---------------------------------------------------------------------------
char *vtkSlicerFont::GetFontAsFamily(char *fontstring)
{
    // parses out the font string and returns the css version of the family
    return "Adobe,Helvetica,sans-serif";
}


