
#include "vtkObjectFactory.h"
#include "vtkSlicerColor.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerColor );
vtkCxxRevisionMacro ( vtkSlicerColor, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerColor::vtkSlicerColor ( ) {
    
          // colors to experiment with ...
    SetColor(this->White, 1.0, 1.0, 1.0);
    SetColor(this->Black,  0.0, 0.0, 0.0 );
    SetColor(this->MediumGrey,  0.65, 0.65, 0.65 );
    SetColor(this->LightGrey,  0.729, 0.729, 0.729 );

    // warm, soft brown grey
    // light
    SetColor(this->Cornsilk2,  .93, .9098, .804 );
    // medium
    SetColor(this->Cornsilk3,  .804, .784, .694 );
    // darker
    SetColor(this->Cornsilk4,  .545, .5333, .47 );

    // warm, soft brown-pink grey
    // light
    SetColor(this->Seashell2,  .5411, .898, .87 );
    // medium
    SetColor(this->Seashell3,  .803, 7725, .749 );
    // darker
    SetColor(this->Seashell4,  .545, .525, .5098 );
    
    // Possibilities for SliceWindows
    // desaturated grey-green
    SetColor(this->MediumGreen, .5137, .6509, .498);
    // dark saturated green
    SetColor(this->DarkGreen, .149, .78, .149 );
    
    // Possibilities for SliceWindows
    // desaturated grey-red
    SetColor(this->MediumRed,  .7568, .4, .3529) ;
    // dark saturated red
    SetColor(this->DarkRed,  .6, .25, 0.0 );
    
    // Possibilities for SliceWindows
    // desaturated grey-yellow
    SetColor(this->MediumYellow,  .9333, .8392, .5019 );
    // dark saturated yellow
    SetColor(this->DarkYellow,  .8196, .5804, .047);

    // bright yellow (warning message code)
    SetColor(this->BrightYellow, .902, .816, .242);

    // 3D viewer background
    SetColor(this->MediumBlue,  .4588, .5647, .6823 );
    SetColor(this->ViewerBlue,  .9333, .8392, .5019 );
}


//---------------------------------------------------------------------------
vtkSlicerColor::~vtkSlicerColor ( ) {
}

//---------------------------------------------------------------------------
int vtkSlicerColor::SetColor(double * color,
                               double r, double g, double b) {
    
    if ( (r==color[0]) && (g==color[1]) && (b==color[2]) ||
         (r<0.0 || r>1.0) || (g<0.0 || g>1.0) || (b<0.0 || b>1.0)) {
        return 0;
    }
    color [0] = r;
    color [1] = g;
    color [2] = b;

    return 1;
}

//---------------------------------------------------------------------------
char *vtkSlicerColor::GetColorAsHex(double *color)
{
    sprintf(this->HexColor, "%X%X%X",
            color[0]*16.0, color[1]*16.0, color[2]*16.0);
    return this->HexColor;
}

