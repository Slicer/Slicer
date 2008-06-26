
#include "vtkObjectFactory.h"
#include "vtkSlicerStyle.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerStyle );
vtkCxxRevisionMacro ( vtkSlicerStyle, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerStyle::vtkSlicerStyle ( ) {
  this->Colors = vtkSlicerColor::New();
  this->Fonts = vtkSlicerFont::New();
}


//---------------------------------------------------------------------------
vtkSlicerStyle::~vtkSlicerStyle ( ) {
  if ( this->Colors ) {
  this->Colors->Delete();
  this->Colors = NULL;
  }
  if ( this->Fonts ) {
  this->Fonts->Delete();
  this->Fonts = NULL;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerStyle::ApplyPresentation ( ) {
  /*
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
  double ViewerBlue [3] = { .70196, .70196, .90588};
  */
  // Configure Slicer Brand; just a few things for now.
  // these font settings cause a crash... fix.
  //this->SetFlatRelief ( );
  this->SetBgColor ( this->Colors->White );
  this->SetFgColor ( this->Colors->MediumGrey );
  this->SetPadX ( 2 );
  this->SetPadY ( 2 );

  this->SetColor(SagColor, this->Colors->MediumYellow);
  this->SetColor(AxiColor, this->Colors->MediumRed);
  this->SetColor(CorColor, this->Colors->MediumGreen);
  this->SetColor(WarningTextColor, this->Colors->BrightYellow);
  this->SetColor(GUIBgColor, this->Colors->White);
  this->SetColor(GUIFgColor, this->Colors->Black);
  this->SetColor(ViewerBgColor, this->Colors->ViewerBlue);
  this->SetColor(SliceBgColor, this->Colors->Black);
}

//---------------------------------------------------------------------------
void vtkSlicerStyle::ParseStyleParameters ( ) {
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
int vtkSlicerStyle::SetColor ( double * c1, double * c2)
{
  return SetColor(c1, c2[0], c2[1], c2[2]);
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
int vtkSlicerStyle::SetGUIBgColor ( double *c ) {

  if ( this->SetColor ( this->GUIBgColor, c[0], c[1], c[2] ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetGUIBgColor ( double r, double g, double b ) {

  if ( this->SetColor ( this->GUIBgColor, r, g, b ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetGUIBgColor ( ) {

  return this->GUIBgColor;
}




//---------------------------------------------------------------------------
int vtkSlicerStyle::SetGUIFgColor ( double *c ) {

  if ( this->SetColor ( this->GUIFgColor, c[0], c[1], c[2] ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetGUIFgColor ( double r, double g, double b ) {

  if ( this->SetColor ( this->GUIFgColor, r, g, b ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetGUIFgColor ( ) {

  return this->GUIFgColor;
}




//---------------------------------------------------------------------------
int vtkSlicerStyle::SetSliceBgColor ( double *c ) {

  if ( this->SetColor ( this->SliceBgColor, c[0], c[1], c[2] ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetSliceBgColor ( double r, double g, double b ) {

  if ( this->SetColor ( this->SliceBgColor, r, g, b ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetSliceBgColor ( ) {

  return this->SliceBgColor;
}




//---------------------------------------------------------------------------
int vtkSlicerStyle::SetViewerBgColor ( double *c ) {

  if ( this->SetColor ( this->ViewerBgColor, c[0], c[1], c[2] ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
int vtkSlicerStyle::SetViewerBgColor ( double r, double g, double b ) {

  if ( this->SetColor ( this->ViewerBgColor, r, g, b ) ) {
  return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------
double* vtkSlicerStyle::GetViewerBgColor ( ) {

  return this->ViewerBgColor;
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


//---------------------------------------------------------------------------
// writes out a cascading style sheet based on Slicer colours
// return 1 on success, 0 on failure
int vtkSlicerStyle::WriteCascadingStyleSheet (char * filename)
{
  FILE *fp;
  // open the filename for writing
  fp = fopen(filename, "w");
  if (!fp)
    {
    vtkErrorMacro(<<"WriteCascadingStyleSheet: Unable to open file for writing: " << filename);
    return 0;
    }

  // header comment
  fprintf(fp, "/* Autogenerated from vtkSlicerStyle::WriteCascadingStyleSheet.\n%s\n*/", filename);
    
  // body
  fprintf(fp, "body {background-color:#%s; color:#%s; margin-bottom:5; margin-top:5; margin-left:5; margin-right:5;}\n",
          this->Colors->GetColorAsHex(this->GetBgColor()),
          this->Colors->GetColorAsHex(this->GetFgColor()));

  // headers
  fprintf(fp, "h1,h2,h3,h4 {font-family:%s;}\n",
          this->Fonts->GetFontAsFamily(this->GetBigFont()));

  // horizontal rule
  fprintf(fp, "hr {size:2; color: #%s;}\n",
          this->Colors->GetColorAsHex(this->Colors->MediumBlue));

  // emphasis
  fprintf(fp, "em {color:#%s; font-weight:bold; font-style:normal;}\n",
          this->Colors->GetColorAsHex(this->Colors->MediumBlue)); 

  fprintf(fp, "p,body,ul,pre {font-family:%s;}\n",
          this->Fonts->GetFontAsFamily(this->GetMedFont()));

  fprintf(fp, "table.heading {border:0; cellspacing:0; cellpadding:0; width:100%%;}\n");

  fprintf(fp, "table.separator {border:0; cellspacing:0; width:100%%;}\n");
  fprintf(fp, "table.separatorline {border:0; cellspacing:0; cellpadding:0;}\n");
  fprintf(fp, "td.line1 {background-color:#%s;}\n",
          this->Colors->GetColorAsHex(this->Colors->White));
  fprintf(fp, "td.lineht {height:0.05cm; border:0; cellpadding:0}\n");
  fprintf(fp, "td.line2 {background-color:#%s;}\n",
          this->Colors->GetColorAsHex(this->SagColor));
  fprintf(fp, "td.line3 {background-color:#%s;}\n",
          this->Colors->GetColorAsHex(this->AxiColor));


  fprintf(fp, "th.box {align:center; valign:top; background-color:#%s; color:#%s; font-family:%s; font-weight:bold;}\n",
          this->Colors->GetColorAsHex(this->Colors->ViewerBlue),
          this->Colors->GetColorAsHex(this->Colors->MediumGrey),
          this->Fonts->GetFontAsFamily(this->GetMedFont()));
  fprintf(fp, "td.box0 {valign:top; background-color:#e5e5e5;}\n");
  fprintf(fp, "td.box1 {valign:top; background-color:#f1f1f1;}\n");

  fprintf(fp, "div.q {font-weight:bold; }\n");
  fprintf(fp, "div.a {padding:5}\n");

  fprintf(fp, "img {border-color:#333399; border:2;}\n");


  fprintf(fp, "A,A:Visited  {color:#%s; font-family:%s; font-weight:bold; text-decoration:underline;}\n",
          this->Colors->GetColorAsHex(this->CorColor),
          this->Fonts->GetFontAsFamily(this->GetMedFont()));
  fprintf(fp, "A:Hover,A:Active  {color:#c17350; font-family:Arial,Helvetica,sans-serif; font-weight:bold; text-decoration:underline;}\n");
  fprintf(fp, "A.topic,A:Visited.topic  {color:#%s; font-family:%s; font-weight:normal; text-decoration:none;}\n",
          this->Colors->GetColorAsHex(this->CorColor),
          this->Fonts->GetFontAsFamily(this->GetMedFont()));
  fprintf(fp, "A:Hover.topic,A:Active.topic  {color:#%s; font-family:%s; font-weight:normal; text-decoration:underline;}\n",
          this->Colors->GetColorAsHex(this->AxiColor),
          this->Fonts->GetFontAsFamily(this->GetMedFont()));
  fprintf(fp, "A.folder,A:Visited.folder,A:Hover.folder,A:Active.folder {color:#%s; font-family:%s; font-weight:normal; text-decoration:none;}\n",
          this->Colors->GetColorAsHex(this->Colors->Black),
          this->Fonts->GetFontAsFamily(this->GetMedFont()));
  fprintf(fp, "A:Hover.folder {color:#%s; font-family:%s; font-weight:normal; text-decoration:underline;}\n",
          this->Colors->GetColorAsHex(this->Colors->Black),
          this->Fonts->GetFontAsFamily(this->GetMedFont()));

  fprintf(fp, "P.code {font-family: \"Courier New\" Courier monospace; margin-left:3em; margin-right:3em; padding:5; background-color:#cdc8b1; border:3px ridge #AAAAAA}\n");

  fprintf(fp, "P.warning {margin-left:3em; margin-right:3em; padding:5; background-color:#%s; color:black; border:3px ridge #BB2222}\n",
          this->Colors->GetColorAsHex(this->WarningTextColor));
  fprintf(fp, "P.warning em {font-weight:bold; color:#%s;}\n",
          this->Colors->GetColorAsHex(this->ErrorTextColor));
  fprintf(fp, "P.tidbit {padding:5; background-color:#%s; border:3px ridge #2222BB}\n",
          this->Colors->GetColorAsHex(this->Colors->ViewerBlue));
  fprintf(fp, "P.tidbit em {font-weight:bold; color:#%s;}\n",
          this->Colors->GetColorAsHex(this->AxiColor));
  fprintf(fp, "P.teaser {color:#707070; font-family:%s; font-weight:bold;}\n",
          this->Fonts->GetFontAsFamily(this->GetMedFont()));


  fprintf(fp, "P.question {font-weight:bold; color:black;}\n");
  fprintf(fp, "P.answer {color:black;margin-bottom:10}\n");
  fprintf(fp, "LI.faq {}\n");

  fclose(fp);
    
  return 1;
}
