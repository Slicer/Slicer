#ifndef __vtkSlicerColor_h
#define __vtkSlicerColor_h

#include "vtkObject.h"

// Description:
// Color definitios used in Slicer's style.
//
class vtkSlicerColor : public vtkObject
{
 public:
    static vtkSlicerColor* New ( );
    vtkTypeRevisionMacro ( vtkSlicerColor, vtkObject );

    
    //---
    //--- Slicer semantic color names
    double White [3];
    double Black [3];
    double DarkGrey [3];
    double MediumGrey [3];
    double LightGrey [3];
    double LightestGrey [3];
    
    double SagittalColor[3];
    double AxialColor[3];
    double CoronalColor[3];

    double ActiveTextColor[3];
    double DisabledTextColor[3];
    double FocusTextColor[3];
    double ActiveMenuBackgroundColor[3];
    double HighlightColor[3];
    double HighlightBackground[3];

    double RecessedColor[3];
    double NotificationMessageColor[3];
    double WarningMessageColor[3];
    double ErrorMessageColor[3];

    double GUIBgColor[3];
    double ViewerBgColor[3];
    double SliceBgColor[3];
    
    //---
    //--- colors in the palette
    double Cornsilk2 [3];
    double Cornsilk3 [3];
    double Cornsilk4 [3];
    double Seashell2 [3];
    double Seashell3 [3];
    double Seashell4 [3];   
    double MediumGreen [3];
    double DarkGreen [3];
    double MediumRed [3];
    double DarkRed [3];
    double BrightRed [3];
    double BrightYellow [3];
    double MediumYellow [3];
    double DarkYellow [3];
    double MediumBlue [3];
    double ViewerBlue [3];
    double LightBlue [3];
    double SliceGUIRed [3];
    double SliceGUIYellow [3];
    double SliceGUIGreen [3];

    char *GetColorAsHex(double *color);
    
protected:
    
    vtkSlicerColor ( );
    ~vtkSlicerColor ( );

    void DefineSlicerColorNames ( );
    void DefineDefaultColorPalette ( );
    int SetColor (double *color, double r, double g, double b);
    int SetColor (double *color1, double *color2);

    char HexColor [6];
    
 private:
    vtkSlicerColor ( const vtkSlicerColor&); // Not implemented
    void operator = ( const vtkSlicerColor&); // Not implemented
};

#endif
