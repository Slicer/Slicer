
#include "vtkObjectFactory.h"
#include "vtkSlicerColor.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerColor );
vtkCxxRevisionMacro ( vtkSlicerColor, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerColor::vtkSlicerColor ( ) {
    
    //--- Set Slicer color palette
    this->DefineDefaultColorPalette ( );

    //--- Set Slicer colors using palette
    this->DefineSlicerColorNames ( );
}



//---------------------------------------------------------------------------
vtkSlicerColor::~vtkSlicerColor ( ) {
}




//---------------------------------------------------------------------------
void vtkSlicerColor::DefineDefaultColorPalette ( ) {
    //--
    //--- Slicer main palette color names
    //---
    this->SetColor ( this->Black, 0.0, 0.0, 0.0);
    this->SetColor ( this->White, 1.0, 1.0, 1.0);
    this->SetColor ( this->LightestGrey, 0.898039215686, 0.898039215686, 1.0);
    this->SetColor ( this->LighterGrey, 0.874509803922, 0.866666666667, 0.886274509804);
    this->SetColor ( this->LightGrey, 0.682352941176, 0.682352941176, 0.682352941176);
    this->SetColor ( this->MediumGrey, 0.6, 0.6, 0.6);
    this->SetColor ( this->DarkGrey, 0.321568627451, 0.321568627451, 0.321568627451);
    this->SetColor ( this->DarkOchre, 0.709803921569, 0.392156862745, 0.0823529411765);
    this->SetColor ( this->MediumOchre, 0.894117647059, 0.650980392157, 0.125490196078);
    this->SetColor ( this->BrightOchre, 0.952941176471, 0.674509803922, 0.133333333333);
    this->SetColor ( this->SliceYellow, 0.929411764706, 0.835294117647, 0.298039215686);
    this->SetColor ( this->LightOchre, 0.956862745098, 0.913725490196, 0.498039215686);
    this->SetColor ( this->DarkOrange, 0.760784313725, 0.290196078431, 0.0941176470588);
    this->SetColor ( this->MediumOrange, 0.882352941176, 0.439215686275, 0.0705882352941);
    this->SetColor ( this->BrightOrange, 0.956862745098, 0.509803921569, 0.078431372549);
    this->SetColor ( this->LightOrange, 0.952941176471, 0.721568627451, 0.274509803922);
    this->SetColor ( this->LightestOrange, 0.937254901961, 0.835294117647, 0.501960784314);
    this->SetColor ( this->DarkBrown, 0.513725490196, 0.4, 0.282352941176);
    this->SetColor ( this->MediumBrown, 0.756862745098, 0.450980392157, 0.309803921569);
    this->SetColor ( this->Brown, 0.745098039216, 0.580392156863, 0.38431372549);
    this->SetColor ( this->LightBrown, 0.878431372549, 0.760784313725, 0.619607843137);
    this->SetColor ( this->LightestBrown, 0.945098039216, 0.878431372549, 0.81568627451);
    this->SetColor ( this->DarkRed, 0.764705882353, 0.180392156863, 0.0627450980392);
    this->SetColor ( this->MediumRed, 0.850980392157, 0.145098039216, 0.0705882352941);
    this->SetColor ( this->SliceRed, 0.952941176471, 0.290196078431, 0.2);
    this->SetColor ( this->LightRed, 0.894117647059, 0.41568627451, 0.266666666667);
    this->SetColor ( this->LightestRed, 0.901960784314, 0.517647058824, 0.392156862745);
    this->SetColor ( this->DarkGreyGreen, 0.274509803922, 0.329411764706, 0.188235294118);
    this->SetColor ( this->MediumGreyGreen, 0.403921568627, 0.556862745098, 0.247058823529);
    this->SetColor ( this->SliceGreen, 0.43137254902, 0.690196078431, 0.294117647059);
    this->SetColor ( this->LightGreyGreen, 0.541176470588, 0.647058823529, 0.439215686275);
    this->SetColor ( this->LightestGreyGreen, 0.792156862745, 0.901960784314, 0.709803921569);
    this->SetColor ( this->DarkGreen, 0.145098039216, 0.337254901961, 0.223529411765);
    this->SetColor ( this->MediumGreen, 0.0, 0.498039215686, 0.0352941176471);
    this->SetColor ( this->Green, 0.18431372549, 0.643137254902, 0.18431372549);
    this->SetColor ( this->LightGreen, 0.349019607843, 0.776470588235, 0.478431372549);
    this->SetColor ( this->LightestGreen, 0.807843137255, 0.933333333333, 0.847058823529);
    this->SetColor ( this->DarkGreyBlue, 0.282352941176, 0.250980392157, 0.396078431373);
    this->SetColor ( this->MediumGreyBlue, 0.388235294118, 0.360784313725, 0.521568627451);
    this->SetColor ( this->GreyBlue, 0.454901960784, 0.560784313725, 0.678431372549);
    this->SetColor ( this->SlicerBlue, 0.701960784314, 0.701960784314, 0.905882352941);
    this->SetColor ( this->LightGreyBlue, 0.81568627451, 0.81568627451, 0.945098039216);
    this->SetColor ( this->DarkBlue, 0.149019607843, 0.23137254902, 0.545098039216);
    this->SetColor ( this->MediumBlue, 0.219607843137, 0.372549019608, 0.694117647059);
    this->SetColor ( this->Blue, 0.164705882353, 0.443137254902, 0.733333333333);
    this->SetColor ( this->LightBlue, 0.478431372549, 0.61568627451, 0.760784313725);
    this->SetColor ( this->LightestBlue, 0.596078431373, 0.83137254902, 0.835294117647);

    //---
    //--- message palette color names
    //---
    this->SetColor ( this->ErrorRed, 1.0, 0.0, 0.0);
    this->SetColor ( this->SystemBlue, 0.0588235294118, 0.4, 0.752941176471);
    this->SetColor ( this->WarningYellow, 1.0, 0.886274509804, 0.117647058824);
    
    //---
    //--- accent palette color names
    //---
    this->SetColor ( this->Magenta, 1.0, 0.0, 1.0 );
    this->SetColor ( this->Purple, 0.749019607843, 0.286274509804, 0.745098039216);
    this->SetColor ( this->DarkPurple, 0.545098039216, 0.152941176471, 0.521568627451);
    this->SetColor ( this->LogoGreyBlue, 0.443137254902, 0.498039215686, 0.596078431373);
    this->SetColor ( this->IGTGrey, 0.266666666667, 0.270588235294, 0.356862745098);
    this->SetColor ( this->LogoDarkGrey, 0.188235294118, 0.219607843137, 0.235294117647);
    this->SetColor ( this->LogoLightYellow, 0.941176470588, 0.850980392157, 0.541176470588);
    this->SetColor ( this->LogoMediumYellow, 0.972549019608, 0.749019607843, 0.145098039216);
    this->SetColor ( this->LogoOrange, 0.941176470588, 0.423529411765, 0.180392156863);
    this->SetColor ( this->LogoRed, 0.96862745098, 0.176470588235, 0.117647058824);
    this->SetColor ( this->NAMICBlue, 0.0, 0.388235294118, 0.709803921569);
    this->SetColor ( this->SPLGreen, 0.0, 0.6, 0.4 );


    // old colors -- work on phasing these Out!

    this->SetColor(this->LightestGreyBlue, 0.9, 0.9, 1.0);
    this->SetColor(this->BurntOrange, 0.7, 0.4, 0.0 );
    this->SetColor(this->DarkStone, .811764, 0.8, 0.65413 );
    this->SetColor(this->MediumCoolStone, 0.87627, 0.87039,  0.88843);
    this->SetColor(this->LightCoolStone, 0.94627, 0.94039, 0.97843);

    this->SetColor(this->LightStone,0.95941,0.945098, 0.823529 );
    this->SetColor ( this->LightestStone, 0.996078, 0.988235,0.913725);
    this->SetColor(this->Cornsilk2,  .93, .9098, .804 );
    this->SetColor(this->Cornsilk3,  .804, .784, .694 );
    this->SetColor(this->Cornsilk4,  .545, .5333, .47 );
    this->SetColor(this->Seashell2,  .5411, .898, .87 );
    this->SetColor(this->Seashell3,  .803, 7725, .749 );
    this->SetColor(this->Seashell4,  .545, .525, .5098 );
    this->SetColor (this->BrightRed, .8, 0.0, 0.0 );
    this->SetColor(this->MediumYellow,  .9333, .8392, .5019 );
    this->SetColor(this->DarkYellow,  .8196, .5804, .047);
    this->SetColor(this->BrightYellow, .902, .816, .242);


}


//---------------------------------------------------------------------------
void vtkSlicerColor::DefineSlicerColorNames ( )
{

  this->SetColor(this->SliceGUIRed,  this->SliceRed );
  this->SetColor(this->SliceGUIYellow, this->SliceYellow );
  this->SetColor(this->SliceGUIGreen, this->SliceGreen );
  this->SetColor (this->ViewerBlue, this->SlicerBlue );
  this->SetColor (this->ActiveTextColor, this->Black );
  this->SetColor (this->DisabledTextColor, this->MediumGrey );
  this->SetColor (this->FocusTextColor, this->Black );
  this->SetColor (this->ActiveMenuBackgroundColor, this->LightGrey);
  this->SetColor (this->ActiveMenuBackgroundColor, this->LighterGrey);
  this->SetColor (this->ActiveMenuForegroundColor, this->Black);

  this->SetColor (this->HighlightBackground, this->Black );
  this->SetColor (this->HighlightColor, this->GreyBlue );
  this->SetColor (this->HighlightColor, this->LighterGrey );
    
  this->SetColor (this->NotificationMessageColor, this->SystemBlue );
  this->SetColor (this->WarningMessageColor, this->WarningYellow );
  this->SetColor (this->ErrorMessageColor, this->ErrorRed );

  this->SetColor (this->GUIBgColor, this->White );
  this->SetColor (this->ViewerBgColor, this->ViewerBlue );
  this->SetColor (this->SliceBgColor, this->Black );
  this->SetColor (this->RecessedColor, this->LightGrey );

  this->SetColor ( this->ModuleTopFrame, this->LighterGrey );
  this->SetColor ( this->GUIDropShadow, this->LightestGrey );
}



//---------------------------------------------------------------------------
int vtkSlicerColor::SetColor (double *color1, double *color2 ) {

    int ret = this->SetColor ( color1, color2[0], color2[1], color2[2] );
    return ret;
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

