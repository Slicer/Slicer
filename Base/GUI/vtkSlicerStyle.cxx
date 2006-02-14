
#include "vtkObjectFactory.h"
#include "vtkSlicerStyle.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerStyle );
vtkCxxRevisionMacro ( vtkSlicerStyle, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerStyle::vtkSlicerStyle ( ) {
}


//---------------------------------------------------------------------------
vtkSlicerStyle::~vtkSlicerStyle ( ) {
}

//---------------------------------------------------------------------------
void vtkSlicerStyle::ApplyPresentation ( ) {

    // colors to experiment with ...
    double White [3]  = { 1.0, 1.0, 1.0 };
    double Black [3]  = { 0.0, 0.0, 0.0 };
    double  MediumGrey [3]  = { 0.65, 0.65, 0.65 };
    double  LightGrey [3] = { 0.729, 0.729, 0.729 };

    // warm, soft brown grey
    // light
    double  Cornsilk2 [3] = { .93, .9098, .804 };
    // medium
    double Cornsilk3 [3] = { .804, .784, .694 };
    // darker
    double Cornsilk4 [3] = { .545, .5333, .47 };

    // warm, soft brown-pink grey
    // light
    double  Seashell2 [3] = { .5411, .898, .87 };
    // medium
    double  Seashell3 [3] = { .803, 7725, .749 };
    // darker
    double Seashell4 [3] = { .545, .525, .5098 };
    
    // Possibilities for SliceWindows
    // desaturated grey-green
    double  MediumGreen [3] = { .5137, .6509, .498 } ;
    // dark saturated green
    double DarkGreen [3] = { .149, .78, .149 } ;
    
    // Possibilities for SliceWindows
    // desaturated grey-red
    double  MediumRed [3] = { .7568, .4, .3529 } ;
    // dark saturated red
    double  DarkRed [3] = { .6, .25, 0.0 };
    
    // Possibilities for SliceWindows
    // desaturated grey-yellow
    double  MediumYellow [3] = { .9333, .8392, .5019 } ;
    // dark saturated yellow
    double  DarkYellow [3] = { .8196, .5804, .047} ;

    // 3D viewer background
    double  MediumBlue [3] = { .4588, .5647, .6823 } ;
    double  ViewerBlue [3] = { .9333, .8392, .5019 } ;


    // Configure Slicer Brand; just a few things for now.
    this->SetBigFont ("-Adobe-Helvetica-Bold-R-Normal-*-12-*-*-*-*-*-*-*");
    this->SetMedFont ("-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*");
    this->SetSmallFont ("-Adobe-Helvetica-Bold-R-Normal-*-8-*-*-*-*-*-*-*");
    this->SetJustify("left");
    this->SetFlatRelief ( );
    this->SetBgColor ( White );
    this->SetFgColor ( MediumGrey );
    this->SetPadX ( 2 );
    this->SetPadY ( 2 );
}

//---------------------------------------------------------------------------
void vtkSlicerStyle::ParseStyleParameters ( ) {
}

//---------------------------------------------------------------------------
void vtkSlicerStyle::SetTextLeftJustify (  ) {

    this->SetJustify ( "left" );
}


//---------------------------------------------------------------------------
void vtkSlicerStyle::SetTextRightJustify (  ) {

    this->SetJustify ( "right" );
}


//---------------------------------------------------------------------------
void vtkSlicerStyle::SetTextCenterJustify (  ) {

    this->SetJustify ( "center" );
}






//---------------------------------------------------------------------------
void vtkSlicerStyle::SetFlatRelief ( ) {
    this->SetRelief ( "flat" );
}



//---------------------------------------------------------------------------
void vtkSlicerStyle::SetGrooveRelief ( ) {
    this->SetRelief ( "groove" );
}




//---------------------------------------------------------------------------
int vtkSlicerStyle::SetColor ( double * color,
                               double r, double g, double b ) {

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
int vtkSlicerStyle::SetBgColor ( double *c ) {

    if ( this->SetColor ( this->BgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetBgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->BgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetBgColor ( ) {

    return this->BgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetActiveBgColor ( double *c ) {

    if ( this->SetColor ( this->ActiveBgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetActiveBgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->ActiveBgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetActiveBgColor ( ) {

    return this->ActiveBgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetInsertBgColor ( double *c ) {

    if ( this->SetColor ( this->InsertBgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetInsertBgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->InsertBgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetInsertBgColor ( ) {

    return this->InsertBgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetSelectBgColor ( double *c ) {

    if ( this->SetColor ( this->SelectBgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetSelectBgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->SelectBgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetSelectBgColor ( ) {

    return this->SelectBgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetFgColor ( double *c ) {

    if ( this->SetColor ( this->FgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetFgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->FgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetFgColor ( ) {

    return this->FgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetActiveFgColor ( double *c ) {

    if ( this->SetColor ( this->ActiveFgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetActiveFgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->ActiveFgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetActiveFgColor ( ) {

    return this->ActiveFgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetDisabledFgColor ( double *c ) {

    if ( this->SetColor ( this->DisabledFgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetDisabledFgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->DisabledFgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetDisabledFgColor ( ) {

    return this->DisabledFgColor;
}





//---------------------------------------------------------------------------
int vtkSlicerStyle::SetSelectFgColor ( double *c ) {

    if ( this->SetColor ( this->SelectFgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetSelectFgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->SelectFgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetSelectFgColor ( ) {

    return this->SelectFgColor;
}






//---------------------------------------------------------------------------
int vtkSlicerStyle::SetTroughColor ( double *c ) {

    if ( this->SetColor ( this->TroughColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetTroughColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->TroughColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetTroughColor ( ) {

    return this->TroughColor;
}





//---------------------------------------------------------------------------
int vtkSlicerStyle::SetHighLightColor ( double *c ) {

    if ( this->SetColor ( this->HighLightColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetHighLightColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->HighLightColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetHighLightColor ( ) {

    return this->HighLightColor;
}





//---------------------------------------------------------------------------
int vtkSlicerStyle::SetHighLightBgColor ( double *c ) {

    if ( this->SetColor ( this->HighLightBgColor, c[0], c[1], c[2] ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetHighLightBgColor ( double r, double g, double b ) {

    if ( this->SetColor ( this->HighLightBgColor, r, g, b ) ) {
        return 1;
    }
    return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetHighLightBgColor ( ) {

    return this->HighLightBgColor;
}





//---------------------------------------------------------------------------
void vtkSlicerStyle::SetHighLightThickness ( int thickness ) {

    this->HighLightThickness = thickness;
}



//---------------------------------------------------------------------------
void vtkSlicerStyle::SetBorderWidth ( int width ) {
    this->BorderWidth = width;
}




//---------------------------------------------------------------------------
void vtkSlicerStyle::SetActiveBorderWidth ( int width ) {
    this->ActiveBorderWidth = width;
}



//---------------------------------------------------------------------------
void vtkSlicerStyle::SetSelectBorderWidth ( int width ) {
    this->SelectBorderWidth = width;
}



//---------------------------------------------------------------------------
void vtkSlicerStyle::SetPadX ( int padx ) {
    this->PadX = padx;
}




//---------------------------------------------------------------------------
void vtkSlicerStyle::SetPadY ( int pady ) {
    this->PadY = pady;
}


